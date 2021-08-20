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

#ifndef __H_CONFIG_
#define __H_CONFIG_

#include <linux/input-event-codes.h>
#include <stdint.h>
#include <stddef.h>
#include "keys.h"

#define MAX_LAYERS 32
#define CONFIG_DIR "/etc/keyd"
#define MAX_LAYER_NAME_LEN 256
#define MAX_MACRO_SIZE 256
#define MAX_MACROS 256

enum action {
	ACTION_UNDEFINED,
	ACTION_OVERLOAD,
	ACTION_MACRO,
	ACTION_LAYOUT,
	ACTION_KEYSEQ,
	ACTION_LAYER,
	ACTION_LAYER_TOGGLE,
	ACTION_ONESHOT,
};

struct key_descriptor
{
	enum action action;
	union {
		uint32_t keyseq;
		uint16_t mods;
		int8_t layer;
		uint32_t *macro;
		size_t sz;
	} arg, arg2;
};




//A layer may optionally have modifiers. If it does it is expected to behave as
//a normal modifier in all instances except when a key is explicitly defined in
//the keymap.

struct layer {
	uint16_t mods;
	struct key_descriptor *keymap;

	//State
	uint8_t active;
	uint64_t timestamp;
};

struct keyboard_config {
	char name[256];

	struct layer *layers[MAX_LAYERS];
	size_t nlayers;

	int default_modlayout;
	int default_layout;

	struct keyboard_config *next;
};

extern struct keyboard_config *configs;

void config_generate();
void config_free();
const char *keyseq_to_string(uint32_t keyseq);

#endif
