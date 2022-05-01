/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "keyboard.h"
#include "keyd.h"
#include "vkbd.h"
#include "descriptor.h"
#include "layer.h"

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
		     const struct descriptor *d, struct layer *dl)
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
		     struct descriptor *d, struct layer **dl)
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


static void send_key(struct keyboard *kbd, uint8_t code, uint8_t pressed)
{
	if (code == KEYD_NOOP || code == KEYD_EXTERNAL_MOUSE_BUTTON)
		return;

	if (pressed)
		kbd->last_pressed_output_code = code;

	kbd->keystate[code] = pressed;

	switch (code) {
		case KEYD_LEFT_MOUSE:
			vkbd_send_button(vkbd, 1, pressed);
			break;
		case KEYD_MIDDLE_MOUSE:
			vkbd_send_button(vkbd, 2, pressed);
			break;
		case KEYD_RIGHT_MOUSE:
			vkbd_send_button(vkbd, 3, pressed);
			break;
		default:
			vkbd_send_key(vkbd, code, pressed);
			break;
	}
}

static void set_mods(struct keyboard *kbd, uint8_t mods)
{
	size_t i;

	for (i = 0; i < sizeof modifier_table / sizeof(modifier_table[0]); i++) {
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

static void update_mods(struct keyboard *kbd, struct layer *excluded_layer, uint8_t mods)
{
	int i;

	for (i = 0; i < (int)kbd->layer_table.nr_layers; i++) {
		struct layer *layer = &kbd->layer_table.layers[i];
		int excluded = 0;

		if (!layer->active)
			continue;

		if (layer == excluded_layer) {
			excluded = 1;
		} else if (excluded_layer && excluded_layer->type == LT_COMPOSITE) {
			size_t j;

			for (j = 0; j < excluded_layer->nr_layers; j++)
				if (excluded_layer->layers[j] == i)
					excluded = 1;
		}

		if (!excluded)
			mods |= layer->mods;
	}

	set_mods(kbd, mods);
}

static void execute_macro(struct keyboard *kbd, struct layer *dl, const struct macro *macro)
{
	size_t i;
	int hold_start = -1;

	for (i = 0; i < macro->sz; i++) {
		const struct macro_entry *ent = &macro->entries[i];

		switch (ent->type) {
		size_t j;
		uint16_t n;
		uint8_t code, mods;

		case MACRO_HOLD:
			if (hold_start == -1) {
				update_mods(kbd, dl, 0);
				hold_start = i;
			}

			send_key(kbd, ent->data, 1);

			break;
		case MACRO_RELEASE:
			if (hold_start != -1) {
				size_t j;

				for (j = hold_start; j < i; j++) {
					const struct macro_entry *ent = &macro->entries[j];
					send_key(kbd, ent->data, 0);
				}

				hold_start = -1;
			}
			break;
		case MACRO_UNICODE:
			n = ent->data;

			send_key(kbd, KEYD_LINEFEED, 1);
			send_key(kbd, KEYD_LINEFEED, 0);

			for (j = 10000; j; j /= 10) {
				int digit = n / j;
				code = digit == 0 ? KEYD_0 : digit - 1 + KEYD_1;

				send_key(kbd, code, 1);
				send_key(kbd, code, 0);
				n %= j;
			}
			break;
		case MACRO_KEYSEQUENCE:
			code = ent->data;
			mods = ent->data >> 8;

			if (kbd->keystate[code])
				send_key(kbd, code, 0);

			update_mods(kbd, dl, mods);
			send_key(kbd, code, 1);
			send_key(kbd, code, 0);

			break;
		case MACRO_TIMEOUT:
			usleep(ent->data*1E3);
			break;
		}

	}

	update_mods(kbd, NULL, 0);
}

static void lookup_descriptor(struct keyboard *kbd, uint8_t code,
			      struct descriptor *d, struct layer **dl)
{
	size_t max;
	size_t i;
	uint8_t active[MAX_LAYERS];
	struct layer_table *lt = &kbd->layer_table;

	d->op = OP_UNDEFINED;

	long maxts = 0;

	for (i = 0; i < lt->nr_layers; i++) {
		struct layer *layer = &lt->layers[i];

		if (layer->active) {
			active[i] = 1;
			if (layer->keymap[code].op && layer->activation_time >= maxts) {
				maxts = layer->activation_time;
				*d = layer->keymap[code];
				*dl = layer;
			}
		} else {
			active[i] = 0;
		}
	}

	max = 0;
	/* Scan for any composite matches (which take precedence). */
	for (i = 0; i < lt->nr_layers; i++) {
		struct layer *layer = &lt->layers[i];

		if (layer->type == LT_COMPOSITE) {
			size_t j;
			int match = 1;
			uint8_t mods = 0;

			for (j = 0; j < layer->nr_layers; j++) {
				if (active[layer->layers[j]])
					mods |= lt->layers[layer->layers[j]].mods;
				else
					match = 0;
			}

			if (match && layer->keymap[code].op && (layer->nr_layers > max)) {
				*dl = layer;
				*d = layer->keymap[code];

				max = layer->nr_layers;
			}
		}
	}
}

static void update_leds(struct keyboard *kbd)
{
	struct layer_table *lt = &kbd->layer_table;

	if (!kbd->config.layer_indicator)
		return;

	size_t i;
	int active = 0;

	for (i = 0; i < lt->nr_layers; i++) {
		if (lt->layers[i].active && lt->layers[i].mods)
			active = 1;
	}

	device_set_led(kbd->dev, 1, active);
}

static void deactivate_layer(struct keyboard *kbd, struct layer *layer)
{
	if (layer->active > 0)
		layer->active--;
}

/*
 * NOTE: Every activation call *must* be paired with a
 * corresponding deactivation call.
 */

static void activate_layer(struct keyboard *kbd, uint8_t code, struct layer *layer)
{
	layer->active++;
	layer->activation_time = get_time();

	kbd->last_layer_code = code;
}

static void execute_command(const char *cmd)
{
	int fd;

	dbg("Executing command: %s", cmd);

	if (fork())
		return;

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
	struct layer *layers = kbd->layer_table.layers;
	size_t nr_layers = kbd->layer_table.nr_layers;

	for (i = 0; i < nr_layers; i++)
		if (layers[i].flags & LF_ONESHOT) {
			layers[i].flags &= ~LF_ONESHOT;
			deactivate_layer(kbd, &layers[i]);
		}

	kbd->oneshot_latch = 0;
}

static long process_descriptor(struct keyboard *kbd, uint8_t code,
			       struct descriptor *d, struct layer *dl,
			       int pressed)
{
	int timeout = 0;

	struct macro *macros = kbd->layer_table.macros;
	struct timeout *timeouts = kbd->layer_table.timeouts;
	struct layer *layers = kbd->layer_table.layers;
	struct command *commands = kbd->layer_table.commands;

	switch (d->op) {
		struct macro *macro;
		struct layer *layer;
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
			update_mods(kbd, NULL, 0);
		}

		break;
	case OP_LAYER:
		layer = &layers[d->args[0].idx];

		if (pressed) {
			activate_layer(kbd, code, layer);
		} else {
			deactivate_layer(kbd, layer);
		}

		if (kbd->last_pressed_code == code) {
			kbd->inhibit_modifier_guard = 1;
			update_mods(kbd, NULL, 0);
			kbd->inhibit_modifier_guard = 0;
		} else {
			update_mods(kbd, NULL, 0);
		}

		break;
	case OP_OVERLOAD:
		layer = &layers[d->args[0].idx];
		macro = &macros[d->args[1].idx];

		if (pressed) {
			activate_layer(kbd, code, layer);
			update_mods(kbd, NULL, 0);
		} else {
			deactivate_layer(kbd, layer);

			if (kbd->last_pressed_code == code) {
				clear_oneshot(kbd);
				update_mods(kbd, dl, 0);
				execute_macro(kbd, dl, macro);
			}

			update_mods(kbd, NULL, 0);
		}

		break;
	case OP_ONESHOT:
		layer = &layers[d->args[0].idx];

		if (pressed) {
			if (layer->active) {
				/* Neutralize the upstroke. */
				cache_set(kbd, code, NULL, NULL);
			} else {
				activate_layer(kbd, code, layer);
				update_mods(kbd, dl, 0);
				kbd->oneshot_latch = 1;
			}
		} else {
			if (kbd->oneshot_latch) {
				layer->flags |= LF_ONESHOT;
			} else {
				deactivate_layer(kbd, layer);
				update_mods(kbd, NULL, 0);
			}
		}

		break;
	case OP_MACRO:
		macro = &macros[d->args[0].idx];

		if (pressed) {
			clear_oneshot(kbd);

			execute_macro(kbd, dl, macro);
			kbd->active_macro = macro;
			kbd->active_macro_layer = dl;

			timeout = macro->timeout == -1 ?
					kbd->config.macro_timeout :
					macro->timeout;
		}

		break;
	case OP_TOGGLE:
		layer = &layers[d->args[0].idx];

		if (!pressed) {
			layer->flags ^= LF_TOGGLED;

			if (layer->flags & LF_TOGGLED)
				activate_layer(kbd, code, layer);
			else
				deactivate_layer(kbd, layer);
		}

		break;
	case OP_TIMEOUT:
		if (pressed) {
			kbd->pending_timeout.t = timeouts[d->args[0].idx];
			kbd->pending_timeout.code = code;
			kbd->pending_timeout.dl = dl;
			kbd->pending_timeout.active = 1;

			timeout = kbd->pending_timeout.t.timeout;
		}
		break;
	case OP_COMMAND:
		if (pressed)
			execute_command(commands[d->args[0].idx].cmd);
		break;
	case OP_SWAP:
		layer = &layers[d->args[0].idx];
		macro = d->args[1].idx == -1 ? NULL : &macros[d->args[1].idx];


		if (pressed) {
			struct descriptor od;
			struct layer *odl;

			if (!cache_get(kbd, kbd->last_layer_code, &od, &odl)) {
				struct layer *oldlayer = &layers[od.args[0].idx];
				od.args[0].idx = d->args[0].idx;

				cache_set(kbd, kbd->last_layer_code, &od, odl);

				deactivate_layer(kbd, oldlayer);
				activate_layer(kbd, kbd->last_layer_code, layer);

				if (macro) {
					/*
					 * If we are dealing with a simple macro of the form <mod>-<key>
					 * keep the corresponding key sequence depressed for the duration
					 * of the swapping key stroke. This is necessary to account for
					 * pathological input systems (e.g Gnome) which expect a human-scale
					 * interval between key down/up events.
					 */
					if (macro->sz == 1 &&
					    macro->entries[0].type == MACRO_KEYSEQUENCE) {
						uint8_t code = macro->entries[0].data;
						uint8_t mods = macro->entries[0].data >> 8;

						update_mods(kbd, layer, mods);
						send_key(kbd, code, 1);
					} else {
						execute_macro(kbd, layer, macro);
					}
				} else {
					update_mods(kbd, NULL, 0);
				}
			}
		} else {
			if (macro && macro->sz == 1 &&
			    macro->entries[0].type == MACRO_KEYSEQUENCE) {
				uint8_t code = macro->entries[0].data;
				uint8_t mods = macro->entries[0].data >> 8;

				send_key(kbd, code, 0);
				update_mods(kbd, NULL, 0);
			}
		}

		break;
	case OP_UNDEFINED:
		break;
	}

	update_leds(kbd);

	if (pressed)
		kbd->last_pressed_code = code;

	return timeout;
}


/*
 * `code` may be 0 in the event of a timeout.
 *
 * The return value corresponds to a timeout before which the next invocation
 * of kbd_process_key_event must take place. A return value of 0 permits the
 * main loop to call at liberty.
 */
long kbd_process_key_event(struct keyboard *kbd,
			   uint8_t code, int pressed)
{
	struct layer *dl = NULL;
	struct descriptor d;

	/* timeout */
	if (!code) {
		if (kbd->active_macro) {
			execute_macro(kbd, kbd->active_macro_layer, kbd->active_macro);
			return kbd->active_macro->repeat_timeout == -1 ?
					kbd->config.macro_repeat_timeout :
					kbd->active_macro->repeat_timeout;
		}

		if (kbd->pending_timeout.active) {
			struct layer *dl = kbd->pending_timeout.dl;
			uint8_t code = kbd->pending_timeout.code;
			struct descriptor *d = &kbd->pending_timeout.t.d2;

			cache_set(kbd, code, d, dl);

			kbd->pending_timeout.active = 0;
			return process_descriptor(kbd, code, d, dl, 1);
		}
	} else {
		if (kbd->pending_timeout.active) {
			struct layer *dl = kbd->pending_timeout.dl;
			uint8_t code = kbd->pending_timeout.code;
			struct descriptor *d = &kbd->pending_timeout.t.d1;

			cache_set(kbd, code, d, dl);
			process_descriptor(kbd, code, d, dl, 1);
		}

		if (kbd->active_macro) {
			kbd->active_macro = NULL;
			update_mods(kbd, NULL, 0);
		}

		kbd->pending_timeout.active = 0;
	}


	if (pressed) {
		lookup_descriptor(kbd, code, &d, &dl);

		if (cache_set(kbd, code, &d, dl) < 0)
			return 0;
	} else {
		if (cache_get(kbd, code, &d, &dl) < 0)
			return 0;

		cache_set(kbd, code, NULL, NULL);
	}

	return process_descriptor(kbd, code, &d, dl, pressed);
}

int kbd_execute_expression(struct keyboard *kbd, const char *exp)
{
	return layer_table_add_entry(&kbd->layer_table, exp);
}

void kbd_reset(struct keyboard *kbd)
{
	uint8_t ad[MAX_LAYERS];
	long at[MAX_LAYERS];
	uint8_t flags[MAX_LAYERS];

	size_t i;

	/* Preserve layer state to facilitate hotswapping (TODO: make this more robust) */

	for (i = 0; i < kbd->config.layer_table.nr_layers; i++) {
		struct layer *layer = &kbd->layer_table.layers[i];

		ad[i] = layer->active;
		flags[i] = layer->flags;
		at[i] = layer->activation_time;
	}

	memcpy(&kbd->layer_table,
		&kbd->config.layer_table,
		sizeof(kbd->layer_table));

	for (i = 0; i < kbd->config.layer_table.nr_layers; i++) {
		struct layer *layer = &kbd->layer_table.layers[i];

		layer->active = ad[i];
		layer->flags = flags[i];
		layer->activation_time = at[i];
	}
}

