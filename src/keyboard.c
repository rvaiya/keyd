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

#define MACRO_REPEAT_TIMEOUT 600 /* In ms */
#define MACRO_REPEAT_INTERVAL 50 /* In ms */

static struct macro *active_macro = NULL;
static uint8_t active_macro_mods = 0;
static uint8_t oneshot_latch = 0;

static long get_time()
{
	/* close enough :/. using a syscall is unnecessary. */
	static long time = 1;
	return time++;
}

static void kbd_send_key(struct keyboard *kbd, uint8_t code, uint8_t pressed)
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

/* 
 * refcounted to account for overlapping active mods without adding manual
 * accounting to the calling code, each send_mods(foo, 1) *must* be accompanied
 * by a corresponding send_mods(foo, 0) at some point. Failure to do so is
 * considered a bug (i.e treat this like malloc).
 */

static void send_mods(struct keyboard *kbd, uint8_t mods, int press)
{
	size_t i;

	for (i = 0; i < sizeof modifier_table / sizeof(modifier_table[0]); i++) {
		uint8_t code = modifier_table[i].code1;
		uint8_t mask = modifier_table[i].mask;

		if (mask & mods) {
			kbd->modstate[i] += press ? 1 : -1;

			if (kbd->modstate[i] == 0)
				kbd_send_key(kbd, code, 0);
			else if (kbd->modstate[i] == 1)
				kbd_send_key(kbd, code, 1);
		}
	}
}

/* intelligently disarm active mods to avoid spurious alt/meta keypresses. */
static void disarm_mods(struct keyboard *kbd, uint8_t mods)
{
	uint8_t dmods = (MOD_ALT|MOD_SUPER) & mods;
	/*
	 * We interpose a control sequence to prevent '<alt down> <alt up>'
	 * from being interpreted as an alt keypress.
	 */
	if (dmods && ((kbd->last_pressed_output_code == KEYD_LEFTMETA) || (kbd->last_pressed_output_code == KEYD_LEFTALT))) {
		if (kbd->keystate[KEYD_LEFTCTRL])
			send_mods(kbd, dmods, 0);
		else {
			kbd_send_key(kbd, KEYD_LEFTCTRL, 1);
			send_mods(kbd, dmods, 0);
			kbd_send_key(kbd, KEYD_LEFTCTRL, 0);
		}

		mods &= ~dmods;
	}

	send_mods(kbd, mods, 0);
}


static void execute_macro(struct keyboard *kbd, const struct macro *macro)
{
	size_t i;
	int hold_start = -1;

	for (i = 0; i < macro->sz; i++) {
		const struct macro_entry *ent = &macro->entries[i];

		switch (ent->type) {
		case MACRO_HOLD:
			if (hold_start == -1)
				hold_start = i;

			kbd_send_key(kbd, ent->code, 1);

			break;
		case MACRO_RELEASE:
			if (hold_start != -1) {
				size_t j;

				for (j = hold_start; j < i; j++) {
					const struct macro_entry *ent = &macro->entries[j];
					kbd_send_key(kbd, ent->code, 0);
				}
			}
			break;
		case MACRO_KEYSEQUENCE:
			if (kbd->keystate[ent->code])
				kbd_send_key(kbd, ent->code, 0);

			send_mods(kbd, ent->mods, 1);
			kbd_send_key(kbd, ent->code, 1);
			kbd_send_key(kbd, ent->code, 0);
			send_mods(kbd, ent->mods, 0);

			break;
		case MACRO_TIMEOUT:
			usleep(ent->timeout*1E3);
			break;
		}

	}
}

int kbd_execute_expression(struct keyboard *kbd, const char *exp)
{
	return layer_table_add_entry(&kbd->layer_table, exp);
}

void kbd_reset(struct keyboard *kbd)
{
	uint8_t flags[MAX_LAYERS];
	long at[MAX_LAYERS];
	size_t i;

	/* Preserve layer state to facilitate hotswapping (TODO: make this more robust) */

	for (i = 0; i < kbd->config.layer_table.nr; i++) {
		flags[i] = kbd->layer_table.layers[i].flags;
		at[i] = kbd->layer_table.layers[i].activation_time;
	}

	memcpy(&kbd->layer_table,
		&kbd->config.layer_table,
		sizeof(kbd->layer_table));

	for (i = 0; i < kbd->config.layer_table.nr; i++) {
		kbd->layer_table.layers[i].flags = flags[i];
		kbd->layer_table.layers[i].activation_time = at[i];
	}

}

static void lookup_descriptor(struct keyboard *kbd, uint8_t code, uint8_t *layer_mods, struct descriptor *d)
{
	size_t i;
	uint8_t active[MAX_LAYERS];
	struct layer_table *lt = &kbd->layer_table;
	d->op = OP_UNDEFINED;

	*layer_mods = 0;
	long maxts = 0;

	for (i = 0; i < lt->nr; i++) {
		struct layer *layer = &lt->layers[i];

		if (layer->flags) {
			active[i] = 1;
			if (layer->keymap[code].op && layer->activation_time >= maxts) {
				maxts = layer->activation_time;
				*layer_mods = layer->mods;
				*d = layer->keymap[code];
			}
		} else {
			active[i] = 0;
		}
	}

	/* Scan for any composite matches (which take precedence). */
	for (i = 0; i < lt->nr; i++) {
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

			if (match && layer->keymap[code].op) {
				*layer_mods = mods;
				*d = layer->keymap[code];
			}
		}
	}
}

static int cache_set(struct keyboard *kbd, uint8_t code, const struct descriptor *d, uint8_t mods)
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
		kbd->cache[slot].layermods = mods;
	}

	return 0;
}

static int cache_get(struct keyboard *kbd, uint8_t code, struct descriptor *d, uint8_t *mods)
{
	size_t i;

	for (i = 0; i < CACHE_SIZE; i++)
		if (kbd->cache[i].code == code) {
			if (d)
				*d = kbd->cache[i].d;
			if (mods)
				*mods = kbd->cache[i].layermods;

			return 0;
		}

	return -1;
}

static void activate_layer(struct keyboard *kbd, struct layer *layer)
{
	layer->flags |= LF_ACTIVE;
	send_mods(kbd, layer->mods, 1);
	layer->activation_time = get_time();
}

static void deactivate_layer(struct keyboard *kbd, struct layer *layer, int disarm_p)
{
	layer->flags &= ~LF_ACTIVE;

	if (disarm_p)
		disarm_mods(kbd, layer->mods);
	else
		send_mods(kbd, layer->mods, 0);
}

static long process_descriptor(struct keyboard *kbd, uint8_t code, struct descriptor *d, int descriptor_layer_mods, int pressed)
{
	int timeout = 0;
	uint8_t clear_oneshot = 0;

	struct macro *macros = kbd->layer_table.macros;
	struct timeout *timeouts = kbd->layer_table.timeouts;
	struct layer *layers = kbd->layer_table.layers;
	size_t nr_layers = kbd->layer_table.nr;

	switch (d->op) {
		struct macro *macro;
		struct layer *layer;

	case OP_MACRO:
		macro = &macros[d->args[0].idx];

		if (pressed) {
			disarm_mods(kbd, descriptor_layer_mods);

			execute_macro(kbd, macro);

			active_macro = macro;
			active_macro_mods = descriptor_layer_mods;

			timeout = MACRO_REPEAT_TIMEOUT;
		}

		oneshot_latch = 0;
		clear_oneshot = 1;
		break;
	case OP_ONESHOT:
		layer = &layers[d->args[0].idx];

		if (pressed) {
			if (layer->flags & LF_ONESHOT_HELD) {
				/* Neutralize key up */
				cache_set(kbd, code, NULL, 0);
			} else {
				if (layer->flags & LF_ONESHOT) {
					disarm_mods(kbd, layer->mods);
					layer->flags &= ~LF_ONESHOT;
				} 

				send_mods(kbd, layer->mods, 1);

				oneshot_latch = 1;
				layer->flags |= LF_ONESHOT_HELD;
				layer->activation_time = get_time();
			}
		} else if (oneshot_latch) {
			if (layer->flags & LF_ONESHOT) {
				/* 
				 * If oneshot is already set for the layer we can't
				 * rely on the clear logic to mirror our send_mod()
				 * call.
				 */
				disarm_mods(kbd, layer->mods);
			} else {
				layer->flags |= LF_ONESHOT;
				layer->flags &= ~LF_ONESHOT_HELD;
			}
		} else {
			send_mods(kbd, layer->mods, 0);

			layer->flags &= ~LF_ONESHOT_HELD;
		}

		break;
	case OP_TOGGLE:
		layer = &layers[d->args[0].idx];

		if (!pressed) {
			layer->flags ^= LF_TOGGLE;

			if (layer->flags & LF_TOGGLE)
				activate_layer(kbd, layer);
			else
				deactivate_layer(kbd, layer, 0);
		}

		clear_oneshot = 1;
		break;
	case OP_KEYCODE:
		if (pressed) {
			disarm_mods(kbd, descriptor_layer_mods);
			kbd_send_key(kbd, d->args[0].code, 1);
		} else {
			kbd_send_key(kbd, d->args[0].code, 0);
			send_mods(kbd, descriptor_layer_mods, 1);
			clear_oneshot = 1;
		}

		oneshot_latch = 0;
		break;
	case OP_LAYER:
		layer = &layers[d->args[0].idx];

		if (pressed) {
			activate_layer(kbd, layer);
			kbd->last_layer_code = code;
		} else {
			deactivate_layer(kbd, layer, kbd->last_pressed_keycode != code);
		}
		break;
	case OP_SWAP:
		layer = &layers[d->args[0].idx];
		macro = d->args[1].idx == -1 ? NULL : &macros[d->args[1].idx];

		if (pressed) {
			struct descriptor od;
			if (macro) {
				disarm_mods(kbd, descriptor_layer_mods);
				execute_macro(kbd, macro);
				send_mods(kbd, descriptor_layer_mods, 1);
			}

			if (!cache_get(kbd, kbd->last_layer_code, &od, NULL)) {
				struct layer *oldlayer = &layers[od.args[0].idx];

				cache_set(kbd, kbd->last_layer_code, d, descriptor_layer_mods);
				cache_set(kbd, code, NULL, 0);

				activate_layer(kbd, layer);
				deactivate_layer(kbd, oldlayer, 1);
			}
		} else
			deactivate_layer(kbd, layer, 1);

		break;
	case OP_OVERLOAD:
		layer = &layers[d->args[0].idx];
		macro = &macros[d->args[1].idx];

		if (pressed) {
			activate_layer(kbd, layer);
			kbd->last_layer_code = code;
		} else {
			deactivate_layer(kbd, layer, 1);

			if (kbd->last_pressed_keycode == code) {
				disarm_mods(kbd, descriptor_layer_mods);
				execute_macro(kbd, macro);
				send_mods(kbd, descriptor_layer_mods, 1);

				oneshot_latch = 0;
				clear_oneshot = 1;
			}
		}

		break;
	case OP_TIMEOUT:
		if (pressed) {
			kbd->pending_timeout.t = timeouts[d->args[0].idx];
			kbd->pending_timeout.code = code;
			kbd->pending_timeout.mods = descriptor_layer_mods;

			timeout = kbd->pending_timeout.t.timeout;
		}
		break;
	case OP_UNDEFINED:
		if (pressed)
			clear_oneshot = 1;
		break;
	}

	if (clear_oneshot) {
		size_t i = 0;

		for (i = 0; i < nr_layers; i++) {
			struct layer *layer = &layers[i];

			if (layer->flags & LF_ONESHOT) {
				layer->flags &= ~LF_ONESHOT;

				send_mods(kbd, layer->mods, 0);
			}
		}

		oneshot_latch = 0;
	}

	if (pressed)
		kbd->last_pressed_keycode = code;

	return timeout;
}


/*
 * Here be tiny dragons.
 *
 * `code` may be 0 in the event of a timeout.
 *
 * The return value corresponds to a timeout before which the next invocation
 * of kbd_process_key_event must take place. A return value of 0 permits the
 * main loop to call at liberty.
 */
long kbd_process_key_event(struct keyboard *kbd,
			   uint8_t code,
			   int pressed)
{
	uint8_t descriptor_layer_mods;
	struct descriptor d;

	/* timeout */
	if (!code) {
		if (active_macro) {
			execute_macro(kbd, active_macro);
			return MACRO_REPEAT_INTERVAL;
		} else if (kbd->pending_timeout.code) {
			uint8_t mods = kbd->pending_timeout.mods;
			uint8_t code = kbd->pending_timeout.code;
			struct descriptor *d = &kbd->pending_timeout.t.d2;

			cache_set(kbd, code, d, mods);

			kbd->pending_timeout.code = 0;
			return process_descriptor(kbd, code, d, mods, 1);
		}
	}

	if (kbd->pending_timeout.code) {
		uint8_t mods = kbd->pending_timeout.mods;
		uint8_t code = kbd->pending_timeout.code;
		struct descriptor *d = &kbd->pending_timeout.t.d1;

		cache_set(kbd, code, d, mods);
		process_descriptor(kbd, code, d, mods, 1);

		kbd->pending_timeout.code = 0;
	}

	if (active_macro) {
		active_macro = NULL;
		send_mods(kbd, active_macro_mods, 1);
	}

	if (pressed) {
		lookup_descriptor(kbd, code, &descriptor_layer_mods, &d);

		if (cache_set(kbd, code, &d, descriptor_layer_mods) < 0)
			return 0;
	} else {
		if (cache_get(kbd, code, &d, &descriptor_layer_mods) < 0)
			return 0;

		cache_set(kbd, code, NULL, 0);
	}

	return process_descriptor(kbd, code, &d, descriptor_layer_mods, pressed);
}
