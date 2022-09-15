/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */

#include "keyboard.h"

/*
 * Here be tiny dragons.
 */

static long get_time()
{
	/* Close enough :/. Using a syscall is unnecessary. */
	static long time = 1;
	return time++;
}

static int cache_set(struct keyboard *kbd, uint8_t code,
		     const struct descriptor *d, int dl)
{
	size_t i;
	int slot = -1;

	for (i = 0; i < CACHE_SIZE; i++)
		if (kbd->cache[i].code == code) {
			slot = i;
			break;
		} else if (!kbd->cache[i].code) {
			slot = i;
		}

	if (slot == -1)
		return -1;

	if (d == NULL) {
		kbd->cache[slot].code = 0;
	} else {
		kbd->cache[slot].code = code;
		kbd->cache[slot].d = *d;
		kbd->cache[slot].dl = dl;
	}

	return 0;
}

static int cache_get(struct keyboard *kbd, uint8_t code,
		     struct descriptor *d, int *dl)
{
	size_t i;

	for (i = 0; i < CACHE_SIZE; i++)
		if (kbd->cache[i].code == code) {
			if (d)
				*d = kbd->cache[i].d;
			if (dl)
				*dl = kbd->cache[i].dl;

			return 0;
		}

	return -1;
}

static void reset_keystate(struct keyboard *kbd)
{
	size_t i;

	for (i = 0; i < 256; i++) {
		if (kbd->keystate[i]) {
			kbd->output(i, 0);
			kbd->keystate[i] = 0;
		}
	}

}

static void send_key(struct keyboard *kbd, uint8_t code, uint8_t pressed)
{
	if (code == KEYD_NOOP || code == KEYD_EXTERNAL_MOUSE_BUTTON)
		return;

	if (pressed)
		kbd->last_pressed_output_code = code;

	if (kbd->keystate[code] != pressed) {
		kbd->keystate[code] = pressed;
		kbd->output(code, pressed);
	}
}

static void set_mods(struct keyboard *kbd, uint8_t mods)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(modifier_table); i++) {
		uint8_t code = modifier_table[i].code1;
		uint8_t mask = modifier_table[i].mask;

		if (mask & mods) {
			if (!kbd->keystate[code])
				send_key(kbd, code, 1);
		} else {
			/*
			 * Some modifiers have a special meaning when used in
			 * isolation (e.g meta in Gnome, alt in Firefox).
			 * In order to prevent spurious key presses we
			 * avoid adjacent down/up pairs by interposing
			 * additional control sequences.
			 */
			int guard = ((((kbd->last_pressed_output_code == KEYD_LEFTMETA) && mask == MOD_SUPER) ||
					((kbd->last_pressed_output_code == KEYD_LEFTALT) && mask == MOD_ALT)) &&
				    !kbd->inhibit_modifier_guard);

			if (kbd->keystate[code]) {
				if (guard && !kbd->keystate[KEYD_LEFTCTRL]) {
					send_key(kbd, KEYD_LEFTCTRL, 1);
					send_key(kbd, code, 0);
					send_key(kbd, KEYD_LEFTCTRL, 0);
				} else {
					send_key(kbd, code, 0);
				}
			}
		}
	}
}

static void update_mods(struct keyboard *kbd, int excluded_layer_idx, uint8_t mods)
{
	size_t i;
	struct layer *excluded_layer = excluded_layer_idx == -1 ?
					NULL :
					&kbd->config.layers[excluded_layer_idx];

	for (i = 0; i < kbd->config.nr_layers; i++) {
		struct layer *layer = &kbd->config.layers[i];
		int excluded = 0;

		if (!kbd->layer_state[i].active)
			continue;

		if (layer == excluded_layer) {
			excluded = 1;
		} else if (excluded_layer && excluded_layer->type == LT_COMPOSITE) {
			size_t j;

			for (j = 0; j < excluded_layer->nr_constituents; j++)
				if ((size_t)excluded_layer->constituents[j] == i)
					excluded = 1;
		}

		if (!excluded)
			mods |= layer->mods;
	}

	set_mods(kbd, mods);
}

static void execute_macro(struct keyboard *kbd, int dl, const struct macro *macro)
{
	/* Minimize redundant modifier strokes for simple key sequences. */
	if (macro->sz == 1 && macro->entries[0].type == MACRO_KEYSEQUENCE) {
		uint8_t code = macro->entries[0].data;
		uint8_t mods = macro->entries[0].data >> 8;

		update_mods(kbd, dl, mods);
		send_key(kbd, code, 1);
		send_key(kbd, code, 0);
	} else {
		update_mods(kbd, dl, 0);
		macro_execute(kbd->output, macro, kbd->config.macro_sequence_timeout);
	}
}

static void lookup_descriptor(struct keyboard *kbd, uint8_t code,
			      struct descriptor *d, int *dl)
{
	size_t max;
	size_t i;

	d->op = 0;

	long maxts = 0;

	for (i = 0; i < kbd->config.nr_layers; i++) {
		struct layer *layer = &kbd->config.layers[i];

		if (kbd->layer_state[i].active) {
			long activation_time = kbd->layer_state[i].activation_time;

			if (layer->keymap[code].op && activation_time >= maxts) {
				maxts = activation_time;
				*d = layer->keymap[code];
				*dl = i;
			}
		}
	}

	max = 0;
	/* Scan for any composite matches (which take precedence). */
	for (i = 0; i < kbd->config.nr_layers; i++) {
		struct layer *layer = &kbd->config.layers[i];

		if (layer->type == LT_COMPOSITE) {
			size_t j;
			int match = 1;
			uint8_t mods = 0;

			for (j = 0; j < layer->nr_constituents; j++) {
				if (kbd->layer_state[layer->constituents[j]].active)
					mods |= kbd->config.layers[layer->constituents[j]].mods;
				else
					match = 0;
			}

			if (match && layer->keymap[code].op && (layer->nr_constituents > max)) {
				*d = layer->keymap[code];
				*dl = i;

				max = layer->nr_constituents;
			}
		}
	}

	if (!d->op) {
		d->op = OP_KEYSEQUENCE;
		d->args[0].code = code;
		d->args[1].mods = 0;
	}
}

static void deactivate_layer(struct keyboard *kbd, int idx)
{
	dbg("Deactivating layer %s", kbd->config.layers[idx].name);

	assert(kbd->layer_state[idx].active > 0);
	kbd->layer_state[idx].active--;

	if (kbd->layer_observer)
		kbd->layer_observer(kbd, kbd->config.layers[idx].name, 0);
}

/*
 * NOTE: Every activation call *must* be paired with a
 * corresponding deactivation call.
 */

static void activate_layer(struct keyboard *kbd, uint8_t code, int idx)
{
	dbg("Activating layer %s", kbd->config.layers[idx].name);

	kbd->layer_state[idx].activation_time = get_time();
	kbd->layer_state[idx].active++;
	kbd->last_layer_code = code;

	if (kbd->layer_observer)
		kbd->layer_observer(kbd, kbd->config.layers[idx].name, 1);
}

static void execute_command(const char *cmd)
{
	int fd;

	dbg("executing command: %s", cmd);

	if (fork())
		return;
	if (fork())
		exit(0);

	fd = open("/dev/null", O_RDWR);

	if (fd < 0) {
		perror("open");
		exit(-1);
	}

	close(0);
	close(1);
	close(2);

	dup2(fd, 0);
	dup2(fd, 1);
	dup2(fd, 2);

	execl("/bin/sh", "/bin/sh", "-c", cmd, NULL);
}

static void clear_oneshot(struct keyboard *kbd)
{
	size_t i = 0;

	for (i = 0; i < kbd->config.nr_layers; i++)
		if (kbd->layer_state[i].oneshot) {
			kbd->layer_state[i].oneshot = 0;
			deactivate_layer(kbd, i);
		}

	kbd->oneshot_latch = 0;
}

static void clear(struct keyboard *kbd)
{
	size_t i;
	for (i = 1; i < kbd->config.nr_layers; i++) {
		struct layer *layer = &kbd->config.layers[i];

		if (layer->type != LT_LAYOUT) {
			if (kbd->layer_state[i].active && kbd->layer_observer)
				kbd->layer_observer(kbd, kbd->config.layers[i].name, 0);

			memset(&kbd->layer_state[i], 0, sizeof kbd->layer_state[0]);
		}
	}

	/* Neutralize upstroke for active keys. */
	memset(kbd->cache, 0, sizeof(kbd->cache));

	kbd->oneshot_latch = 0;
	kbd->active_macro = NULL;

	reset_keystate(kbd);
}

static void setlayout(struct keyboard *kbd, uint8_t idx)
{
	clear(kbd);
	/* Only only layout may be active at a time */
	size_t i;
	for (i = 0; i < kbd->config.nr_layers; i++) {
		struct layer *layer = &kbd->config.layers[i];

		if (layer->type == LT_LAYOUT)
			kbd->layer_state[i].active = 0;
	}

	kbd->layer_state[idx].activation_time = 1;
	kbd->layer_state[idx].active = 1;
}


static long process_descriptor(struct keyboard *kbd, uint8_t code,
			       struct descriptor *d, int dl,
			       int pressed)
{
	int timeout = 0;

	if (pressed) {
		struct macro *macro;

		switch (d->op) {
		case OP_LAYERM:
		case OP_ONESHOTM:
		case OP_TOGGLEM:
			macro = &kbd->config.macros[d->args[1].idx];
			execute_macro(kbd, dl, macro);
			break;
		default:
			break;
		}
	}

	switch (d->op) {
		int idx;
		struct macro *macro;
		struct descriptor *action;
		uint8_t mods;

	case OP_KEYSEQUENCE:
		code = d->args[0].code;
		mods = d->args[1].mods;

		if (pressed) {
			/*
			 * Permit variations of the same key
			 * to be actuated next to each other
			 * E.G [/{
			 */
			if (kbd->keystate[code])
				send_key(kbd, code, 0);

			update_mods(kbd, dl, mods);

			send_key(kbd, code, 1);
			clear_oneshot(kbd);
		} else {
			send_key(kbd, code, 0);
			update_mods(kbd, -1, 0);
		}

		break;
	case OP_OVERLOAD_TIMEOUT_TAP:
	case OP_OVERLOAD_TIMEOUT:
		if (pressed) {
			uint8_t layer = d->args[0].idx;
			struct descriptor *action = &kbd->config.descriptors[d->args[1].idx];
			timeout = d->args[2].idx;

			kbd->overload2.active = 1;
			kbd->overload2.resolve_on_tap = d->op == OP_OVERLOAD_TIMEOUT_TAP ? 1 : 0;
			kbd->overload2.code = code;
			kbd->overload2.layer = layer;
			kbd->overload2.action = action;
			kbd->overload2.dl = dl;
			kbd->overload2.n = 0;
		} else {
			deactivate_layer(kbd, d->args[0].idx);
			update_mods(kbd, -1, 0);
		}


		break;
	case OP_LAYOUT:
		if (pressed)
			setlayout(kbd, d->args[0].idx);

		break;
	case OP_LAYERM:
	case OP_LAYER:
		idx = d->args[0].idx;

		if (pressed) {
			activate_layer(kbd, code, idx);
		} else {
			deactivate_layer(kbd, idx);
		}

		if (kbd->last_pressed_code == code) {
			kbd->inhibit_modifier_guard = 1;
			update_mods(kbd, -1, 0);
			kbd->inhibit_modifier_guard = 0;
		} else {
			update_mods(kbd, -1, 0);
		}

		break;
	case OP_CLEAR:
		if(pressed)
			clear(kbd);
		break;
	case OP_OVERLOAD:
		idx = d->args[0].idx;
		action = &kbd->config.descriptors[d->args[1].idx];

		if (pressed) {
			activate_layer(kbd, code, idx);
			update_mods(kbd, -1, 0);
		} else {
			deactivate_layer(kbd, idx);

			if (kbd->last_pressed_code == code) {
				clear_oneshot(kbd);

				process_descriptor(kbd, code, action, dl, 1);
				process_descriptor(kbd, code, action, dl, 0);
			}

			update_mods(kbd, -1, 0);
		}

		break;
	case OP_ONESHOTM:
	case OP_ONESHOT:
		idx = d->args[0].idx;

		if (pressed) {
			if (kbd->layer_state[idx].active) {
				/* Neutralize the upstroke. */
				cache_set(kbd, code, NULL, -1);
			} else {
				activate_layer(kbd, code, idx);
				update_mods(kbd, dl, 0);
				kbd->oneshot_latch = 1;
			}
		} else {
			if (kbd->oneshot_latch) {
				kbd->layer_state[idx].oneshot = 1;
			} else {
				deactivate_layer(kbd, idx);
				update_mods(kbd, -1, 0);
			}
		}

		break;
	case OP_MACRO2:
	case OP_MACRO:
		if (pressed) {
			if (d->op == OP_MACRO2) {
				macro = &kbd->config.macros[d->args[2].idx];

				timeout = d->args[0].timeout;
				kbd->macro_repeat_timeout = d->args[1].timeout;
			} else {
				macro = &kbd->config.macros[d->args[0].idx];

				timeout = kbd->config.macro_timeout;
				kbd->macro_repeat_timeout = kbd->config.macro_repeat_timeout;
			}

			clear_oneshot(kbd);

			execute_macro(kbd, dl, macro);
			kbd->active_macro = macro;
			kbd->active_macro_layer = dl;
		}

		break;
	case OP_TOGGLEM:
	case OP_TOGGLE:
		idx = d->args[0].idx;

		if (!pressed) {
			kbd->layer_state[idx].toggled = !kbd->layer_state[idx].toggled;

			if (kbd->layer_state[idx].toggled)
				activate_layer(kbd, code, idx);
			else
				deactivate_layer(kbd, idx);

			update_mods(kbd, -1, 0);
		} else {
			clear_oneshot(kbd);
		}

		break;
	case OP_TIMEOUT:
		if (pressed) {
			kbd->pending_timeout.d1 = kbd->config.descriptors[d->args[0].idx];
			kbd->pending_timeout.d2 = kbd->config.descriptors[d->args[2].idx];

			kbd->pending_timeout.code = code;
			kbd->pending_timeout.dl = dl;

			kbd->pending_timeout.active = 1;

			timeout = d->args[1].timeout;
		}
		break;
	case OP_COMMAND:
		if (pressed)
			execute_command(kbd->config.commands[d->args[0].idx].cmd);
		break;
	case OP_SWAP:
	case OP_SWAPM:
		idx = d->args[0].idx;
		macro = d->op == OP_SWAPM ?  &kbd->config.macros[d->args[1].idx] : NULL;

		if (pressed) {
			struct descriptor od;
			int odl;

			if (kbd->last_layer_code &&
				!cache_get(kbd, kbd->last_layer_code, &od, &odl)) {
				int oldlayer = od.args[0].idx;
				od.args[0].idx = d->args[0].idx;

				cache_set(kbd, kbd->last_layer_code, &od, odl);

				deactivate_layer(kbd, oldlayer);
				activate_layer(kbd, kbd->last_layer_code, idx);
				update_mods(kbd, -1, 0);

				if (macro)
					execute_macro(kbd, dl, macro);
			}
		} else {
			if (macro &&
			    macro->sz == 1 &&
			    macro->entries[0].type == MACRO_KEYSEQUENCE) {
				uint8_t code = macro->entries[0].data;

				send_key(kbd, code, 0);
				update_mods(kbd, -1, 0);
			}
		}

		break;
	}

	if (pressed)
		kbd->last_pressed_code = code;

	return timeout;
}

struct keyboard *new_keyboard(struct config *config,
			      void (*sink) (uint8_t code, uint8_t pressed),
			      void (*layer_observer)(struct keyboard *kbd, const char *name, int state))
{
	size_t i;
	struct keyboard *kbd;

	kbd = calloc(1, sizeof(struct keyboard));

	kbd->original_config = config;
	memcpy(&kbd->config, kbd->original_config, sizeof(struct config));

	kbd->layer_state[0].active = 1;
	kbd->layer_state[0].activation_time = 0;

	if (kbd->config.default_layout[0]) {
		int found = 0;
		for (i = 0; i < kbd->config.nr_layers; i++) {
			struct layer *layer = &kbd->config.layers[i];

			if (layer->type == LT_LAYOUT &&
			    !strcmp(layer->name,
				    kbd->config.default_layout)) {
				kbd->layer_state[i].active = 1;
				kbd->layer_state[i].activation_time = 1;
				found = 1;
				break;
			}
		}

		if (!found)
			fprintf(stderr,
				"\tWARNING: could not find default layout %s.\n",
				kbd->config.default_layout);
	}

	kbd->output = sink;
	kbd->layer_observer = layer_observer;

	return kbd;
}

static int resolve_overload2(struct keyboard *kbd, int time, int hold)
{
	int timeout = 0;
	int timeout_ts;

	struct key_event queue[ARRAY_SIZE(kbd->overload2.queued)];
	int n = kbd->overload2.n;

	kbd->overload2.active = 0;

	memcpy(queue, kbd->overload2.queued, sizeof(struct key_event)*n);

	if (hold) {
		activate_layer(kbd, kbd->overload2.code, kbd->overload2.layer);
		update_mods(kbd, -1, 0);
	} else {
		process_descriptor(kbd,
				   kbd->overload2.code,
				   kbd->overload2.action,
				   kbd->overload2.dl, 1);

		process_descriptor(kbd,
				   kbd->overload2.code,
				   kbd->overload2.action,
				   kbd->overload2.dl, 0);

		cache_set(kbd, kbd->overload2.code, NULL, -1);
	}

	if (!n)
		return 0;

	timeout = kbd_process_events(kbd, queue, n);
	timeout_ts = queue[n-1].timestamp + timeout;

	if (timeout_ts <= time) {
		struct key_event ev = {.code = 0, .timestamp = timeout_ts};
		return kbd_process_events(kbd, &ev, 1);
	}

	return timeout_ts - time;
}

/*
 * `code` may be 0 in the event of a timeout.
 *
 * The return value corresponds to a timeout before which the next invocation
 * of kbd_process_key_event must take place. A return value of 0 permits the
 * main loop to call at liberty.
 */
static long process_event(struct keyboard *kbd, uint8_t code, int pressed, int time)
{
	int dl = -1;
	struct descriptor d;

	int timeleft = kbd->timeout - (time - kbd->last_event_ts);
	kbd->last_event_ts = time;

	/* timeout */
	if (!code) {
		if (kbd->overload2.active)
			return resolve_overload2(kbd, time, 1);

		if (kbd->active_macro) {
			execute_macro(kbd, kbd->active_macro_layer, kbd->active_macro);
			return kbd->macro_repeat_timeout;
		}

		if (kbd->pending_timeout.active) {
			int dl = kbd->pending_timeout.dl;
			uint8_t code = kbd->pending_timeout.code;
			struct descriptor *d = &kbd->pending_timeout.d2;

			cache_set(kbd, code, d, dl);

			kbd->pending_timeout.active = 0;
			return process_descriptor(kbd, code, d, dl, 1);
		}
	} else {
		if (kbd->overload2.active) {
			if (kbd->overload2.code == code) {
				return resolve_overload2(kbd, time, 0);
			} else {
				assert(kbd->overload2.n < ARRAY_SIZE(kbd->overload2.queued));
				struct key_event *ev = &kbd->overload2.queued[kbd->overload2.n++];

				ev->code = code;
				ev->pressed = pressed;
				ev->timestamp = time;

				//TODO: make this optional (overload3)
				if (kbd->overload2.resolve_on_tap && !pressed) {
					size_t i;
					for (i = 0; i < kbd->overload2.n; i++)
						if (kbd->overload2.queued[i].code == code &&
						    kbd->overload2.queued[i].pressed) {
							return resolve_overload2(kbd, time, 1);
						}
				}

				return timeleft;
			}

		}

		if (kbd->pending_timeout.active) {
			int dl = kbd->pending_timeout.dl;
			uint8_t code = kbd->pending_timeout.code;
			struct descriptor *d = &kbd->pending_timeout.d1;

			cache_set(kbd, code, d, dl);
			process_descriptor(kbd, code, d, dl, 1);
		}

		if (kbd->active_macro) {
			kbd->active_macro = NULL;
			update_mods(kbd, -1, 0);
		}

		kbd->pending_timeout.active = 0;
	}


	if (pressed) {
		/*
		 * Guard against successive key down events
		 * of the same key code. This can be caused
		 * by unorthodox hardware or by different
		 * devices mapped to the same config.
		 */
		if (cache_get(kbd, code, &d, &dl) == 0)
			return 0;

		lookup_descriptor(kbd, code, &d, &dl);

		if (cache_set(kbd, code, &d, dl) < 0)
			return 0;
	} else {
		if (cache_get(kbd, code, &d, &dl) < 0)
			return 0;

		cache_set(kbd, code, NULL, -1);
	}

	kbd->timeout = process_descriptor(kbd, code, &d, dl, pressed);
	return kbd->timeout;
}


long kbd_process_events(struct keyboard *kbd, const struct key_event *events, size_t n)
{
	size_t i = 0;
	int timeout = 0;
	int timeout_ts = 0;

	while (i != n) {
		const struct key_event *ev = &events[i];

		if (timeout > 0 && timeout_ts < ev->timestamp) {
			timeout = process_event(kbd, 0, 0, timeout_ts);
		} else {
			timeout = process_event(kbd, ev->code, ev->pressed, ev->timestamp);
			i++;
		}

		timeout_ts = ev->timestamp + timeout;
	}

	return timeout;
}

int kbd_eval(struct keyboard *kbd, const char *exp)
{
	if (!strcmp(exp, "reset")) {
		memcpy(&kbd->config, kbd->original_config, sizeof(struct config));
		return 0;
	} else {
		return config_add_entry(&kbd->config, exp);
	}
}
