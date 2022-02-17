/* Copyright © 2019 Raheman Vaiya.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include "keyboard.h"
#include "keyd.h"
#include "descriptor.h"
#include "layer.h"

#define MACRO_REPEAT_TIMEOUT 400 /* In ms */
#define MACRO_REPEAT_INTERVAL 20 /* In ms */

static long get_time_ms()
{
	struct timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);
	return (tv.tv_sec*1E3)+tv.tv_nsec/1E6;
}

static void send_mods(uint16_t mods, int press)
{
	size_t i;

	for (i = 0; i < sizeof modifier_table / sizeof(modifier_table[0]); i++) {
		uint16_t code = modifier_table[i].code1;
		uint16_t mask = modifier_table[i].mask;

		if (mask & mods && keystate[code] != press)
			send_key(code, press);
	}
}

/* Intelligently disarm active mods to avoid spurious key press events. */
static void disarm_mods(uint16_t mods)
{
	uint16_t dmods = (MOD_ALT|MOD_SUPER) & mods;
	/*
	 * We interpose a control sequence to prevent '<alt down> <alt up>'
	 * from being interpreted as an alt keypress.
	 */
	if (dmods) {
		if (keystate[KEY_LEFTCTRL])
			send_mods(dmods, 0);
		else {
			send_key(KEY_LEFTCTRL, 1);
			send_mods(dmods, 0);
			send_key(KEY_LEFTCTRL, 0);
		}

		mods &= ~dmods;
	}

	send_mods(mods, 0);
}



static void kbd_lookup_descriptor(struct keyboard *kbd,
				  uint16_t code,
				  int pressed,
				  struct descriptor *descriptor,
				  int *descriptor_layer)
{
	size_t i;
	struct descriptor d;
	int dl;
	int found = 0;

	/* Check if the key is active */
	for (i = 0; i < kbd->nr_active_keys; i++) {
		struct active_key *ak = &kbd->active_keys[i];

		if (ak->code == code) {
			dl = ak->dl;
			d = ak->d;

			found = 1;
		}
	}

	if (!found) {
		struct layer *layer;

		if (!kbd->nr_active_layers)
			dl = kbd->layout;
		else
			dl = kbd->active_layers[kbd->nr_active_layers-1].layer;

		layer = &kbd->config.layers[dl];

		d = layer->keymap[code];

		/*
		 * If the most recently active layer is a modifier layer
		 * and the key is undefined, use the layout definition.
		 * If the key is undefined in a normal layer, treat it
		 * as undefined.
		 */
		if (!d.op && layer->mods) {
			dl = kbd->layout;
			d = kbd->config.layers[dl].keymap[code];
		}
	}

	if (pressed) {
		struct active_key *ak = &kbd->active_keys[kbd->nr_active_keys++];

		ak->code = code;
		ak->d = d;
		ak->dl = dl;
	} else {
		int n = 0;

		for (i = 0; i < kbd->nr_active_keys; i++) {
			if (kbd->active_keys[i].code != code)
				kbd->active_keys[n++] = kbd->active_keys[i];
		}

		kbd->nr_active_keys = n;
	}

	*descriptor = d;
	*descriptor_layer = dl;
}

/* Compute the current modifier set based on the activated layers. */
static uint16_t kbd_compute_mods(struct keyboard *kbd)
{
	size_t i;
	uint16_t mods = 0;

	for (i = 0; i < kbd->nr_active_layers; i++) {
		struct layer *layer = &kbd->config.layers[kbd->active_layers[i].layer];
		mods |= layer->mods;
	}

	return mods;
}

/* Returns the active_layer struct associated with the given layer index. */
static const struct active_layer *kbd_lookup_active_layer(struct keyboard *kbd, int layer)
{
	size_t i;

	for (i = 0; i < kbd->nr_active_layers; i++) {
		struct active_layer *al = &kbd->active_layers[i];

		if (al->layer == layer)
			return al;
	}

	return NULL;
}

static void kbd_deactivate_layer(struct keyboard *kbd, int layer_idx, int disarm_mods_p)
{
	int i;
	int n = kbd->nr_active_layers;
	uint16_t active_mods = 0;
	struct layer *layer = &kbd->config.layers[layer_idx];

	dbg("Deactivating layer %s", layer->name);

	kbd->nr_active_layers = 0;
	for (i = 0; i < n; i++) {
		struct active_layer *al = &kbd->active_layers[i];

		if (al->layer != layer_idx) {
			kbd->active_layers[kbd->nr_active_layers++] = *al;
			active_mods |= kbd->config.layers[al->layer].mods;
		}
	}

	if (disarm_mods_p)
		disarm_mods(layer->mods & ~active_mods);
	else
		send_mods(layer->mods & ~active_mods, 0);
}

static void kbd_clear_oneshots(struct keyboard *kbd)
{
	size_t i, n;
	uint16_t cleared_mods = 0;

	n = 0;
	for (i = 0; i < kbd->nr_active_layers; i++) {
		struct active_layer *al = &kbd->active_layers[i];

		if (!al->oneshot) {
			kbd->active_layers[n++] = *al;
		} else {
			cleared_mods |= kbd->config.layers[al->layer].mods;
			dbg("Clearing oneshot layer %s", kbd->config.layers[al->layer].name);
		}
	}

	kbd->nr_active_layers = n;

	send_mods(cleared_mods & ~kbd_compute_mods(kbd), 0);
}

static void kbd_activate_layer(struct keyboard *kbd, int layer_idx, int oneshot)
{
	struct active_layer *al;
	struct layer *layer = &kbd->config.layers[layer_idx];
	size_t i;

	dbg("Activating layer %s", kbd->config.layers[layer_idx].name);

	for (i = 0; i < kbd->nr_active_layers; i++) {
		al = &kbd->active_layers[i];
		if(al->layer == layer_idx) {
			al->oneshot = oneshot;
			return;
		}
	}

	al = &kbd->active_layers[kbd->nr_active_layers];

	al->layer = layer_idx;
	al->oneshot = oneshot;

	kbd->nr_active_layers++;

	send_mods(layer->mods, 1);
}

static void kbd_swap_layer(struct keyboard *kbd,
			   int dl,
			   int replacment_layer,
			   struct descriptor new_descriptor)
{
	size_t i;

	/*
	 * Find the key activating dl and swap out its descriptor with the
	 * current one to deactivate the replacement layer.
	 */
	for (i = 0; i < kbd->nr_active_keys; i++) {
		struct active_key *ak = &kbd->active_keys[i];

		if((ak->d.op == OP_LAYER || ak->d.op == OP_OVERLOAD || ak->d.op == OP_SWAP) && ak->d.args[0].idx == dl) {
			kbd_deactivate_layer(kbd, dl, 1);
			kbd_activate_layer(kbd, replacment_layer, 0);

			ak->d = new_descriptor;

			return;
		}
	}
}

static void kbd_execute_macro(struct keyboard *kbd,
			      const struct macro *macro)
{
	size_t i;
	int hold_start = -1;
	uint16_t active_mods = kbd_compute_mods(kbd);

	send_mods(active_mods, 0);

	for (i = 0; i < macro->sz; i++) {
		const struct macro_entry *ent = &macro->entries[i];

		switch (ent->type) {
		case MACRO_HOLD:
			if (hold_start == -1) {
				hold_start = i;
				set_mods(0);
			}

			send_key(ent->data.sequence.code, 1);

			break;
		case MACRO_RELEASE:
			if (hold_start != -1) {
				size_t j;

				for (j = hold_start; j < i; j++) {
					const struct macro_entry *ent = &macro->entries[j];
					send_key(ent->data.sequence.code, 0);
				}
			}
			break;
		case MACRO_KEYSEQUENCE:
			set_mods(ent->data.sequence.mods);

			send_key(ent->data.sequence.code, 1);
			send_key(ent->data.sequence.code, 0);
			break;
		case MACRO_TIMEOUT:
			usleep(ent->data.timeout*1E3);
			break;
		}

	}

	send_mods(active_mods, 1);
}

int kbd_execute_expression(struct keyboard *kbd, const char *exp)
{
	return config_execute_expression(&kbd->config, exp);
}

void kbd_reset(struct keyboard *kbd)
{
	/* TODO optimize */

	kbd->config = kbd->original_config;
}

static void kbd_start_sequence(struct keyboard *kbd, const struct key_sequence *sequence, int literally)
{
	uint16_t active_mods = kbd_compute_mods(kbd);

	if (!sequence->code && !sequence->mods)
		return;

	if (literally)
		disarm_mods(active_mods);

	send_mods(sequence->mods, 1);
	send_key(sequence->code, 1);
}

static void kbd_end_sequence(struct keyboard *kbd, const struct key_sequence *sequence)
{
	uint16_t active_mods = kbd_compute_mods(kbd);

	send_key(sequence->code, 0);
	send_mods(sequence->mods, 0);
	send_mods(active_mods, 1);
}

static void kbd_execute_sequence(struct keyboard *kbd, const struct key_sequence *sequence, int literally)
{
	kbd_start_sequence(kbd, sequence, literally);
	kbd_end_sequence(kbd, sequence);
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
			   uint16_t code,
			   int pressed)
{
	int dl;
	struct descriptor d;
	int timeout = 0;
	int disarm = 1;
	static struct key_sequence active_sequence = {0};

	static int oneshot_latch = 0;
	static const struct macro *active_macro = NULL;

	static struct {
		int layer;
		struct key_sequence sequence;
		int timeout;

		long start_time;
		uint8_t is_active;
	} pending_overload = {0};

	if (active_sequence.code) {
		kbd_end_sequence(kbd, &active_sequence);
		active_sequence.code = 0;
	}

	if (active_macro) {
		if (!code) {
			kbd_execute_macro(kbd, active_macro);
			return MACRO_REPEAT_INTERVAL;
		} else
			active_macro = NULL;
	}

	if (pending_overload.is_active) {
		if ((get_time_ms() - pending_overload.start_time) >= (long)pending_overload.timeout) {
			kbd_activate_layer(kbd, pending_overload.layer, 0);
		} else {
			kbd_execute_sequence(kbd, &pending_overload.sequence, 1);
		}

		pending_overload.is_active = 0;
	}

	if (!code)
		return 0;

	kbd_lookup_descriptor(kbd, code, pressed, &d, &dl);

	if(!d.op) {
		kbd_clear_oneshots(kbd);
		return 0;
	}

	switch (d.op) {
	int verbatim;
	int is_sequence;
	const struct key_sequence *sequence;
	int layer;
	const struct macro *macro;

	case OP_KEYSEQ:
		/* TODO: distinguish between key codes and key sequences at the config level. */
		sequence = &d.args[0].sequence;
		is_sequence = sequence->mods || dl != kbd->layout;

		if (is_sequence) {
			if (pressed) {
				active_sequence = *sequence;
				kbd_start_sequence(kbd, sequence, 1);
			}
		} else {
			disarm = 0;
			send_key(sequence->code, pressed);
		}

		break;
	case OP_RESET:
		if (!pressed) {
			send_mods(kbd_compute_mods(kbd), 0);
			kbd->nr_active_layers = 0;
		}

		break;
	case OP_TOGGLE:
		layer = d.args[0].idx;

		if (!pressed) {
			const struct active_layer *al = kbd_lookup_active_layer(kbd, layer);
			if (al) {
				if (al->oneshot) /* Allow oneshot layers to toggle themselves. */
					kbd_activate_layer(kbd, layer, 0);
				else
					kbd_deactivate_layer(kbd, layer, 1);
			} else
				kbd_activate_layer(kbd, layer, 0);
		}

		break;
	case OP_LAYOUT:
		if (!pressed)
			kbd->layout = d.args[0].idx;

		break;
	case OP_LAYER:
		layer = d.args[0].idx;

		if (pressed)
			kbd_activate_layer(kbd, layer, 0);
		else {
			int disarm = kbd->disarm_flag;
			if (kbd->last_pressed_keycode == code)
				disarm = 0;

			kbd_deactivate_layer(kbd, layer, disarm);
		}

		break;
	case OP_ONESHOT:
		layer = d.args[0].idx;

		if (pressed) {
			oneshot_latch++;
			kbd_activate_layer(kbd, layer, 0);
		} else if (oneshot_latch) /* No interposed KEYSEQ since key down (other modifiers don't interfere with this).  */
			kbd_activate_layer(kbd, layer, 1);
		else
			kbd_deactivate_layer(kbd, layer, 1);

		break;
	case OP_SWAP:
		layer = d.args[0].idx;
		sequence = &d.args[1].sequence;

		if (pressed) {
			kbd_execute_sequence(kbd, sequence, 1);
			kbd_swap_layer(kbd, dl, layer, d);
		} else if (kbd->last_pressed_keycode != code) {  /* We only reach this from the remapped dl activate key. */
			kbd_deactivate_layer(kbd, layer, 1);
		}

		break;
	case OP_OVERLOAD:
		layer = d.args[0].idx;
		sequence = &d.args[1].sequence;

		if (pressed) {
			kbd_activate_layer(kbd, layer, 0);
		} else if (kbd->last_pressed_keycode == code) {
			kbd_deactivate_layer(kbd, layer, 1);

			/* TODO: enforce this as a code in the config. */
			kbd_execute_sequence(kbd, sequence, dl != kbd->layout);
		} else {
			kbd_deactivate_layer(kbd, layer, 1);
		}

		break;
	case OP_OVERLOAD_TIMEOUT:
		layer = d.args[0].idx;
		sequence = &d.args[1].sequence;

		if (pressed) {
			pending_overload.layer = d.args[0].idx;
			pending_overload.sequence = d.args[1].sequence;
			pending_overload.timeout = d.args[2].timeout;
			pending_overload.start_time = get_time_ms();

			pending_overload.is_active = 1;
		} else {
			kbd_deactivate_layer(kbd, layer, 0);
		}

		break;
	case OP_MACRO:
		macro = &kbd->config.macros[d.args[0].idx];

		if (pressed) {
			active_macro = macro;
			kbd_execute_macro(kbd, macro);
			timeout = MACRO_REPEAT_TIMEOUT;
		}

		break;
	default:
		printf("Unrecognized op: %d, ignoring...\n", d.op);
		break;
	}

	if (!d.op ||
		(pressed &&
		 (d.op == OP_KEYSEQ ||
		  d.op == OP_MACRO  ||
		  d.op == OP_OVERLOAD))) {
		oneshot_latch = 0;
		kbd_clear_oneshots(kbd);
	}

	if (pressed) {
		kbd->last_pressed_keycode = code;
		kbd->disarm_flag = disarm;
	}


	return timeout;
}
