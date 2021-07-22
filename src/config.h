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

#define MAX_LAYERS 8
#define CONFIG_DIR "/etc/keyd"
#define MAX_LAYER_NAME_LEN 256

enum action {
	ACTION_DEFAULT,
	ACTION_DOUBLE_MODIFIER,
	ACTION_DOUBLE_LAYER,
	ACTION_KEYSEQ,
	ACTION_LAYER_TOGGLE,
	ACTION_LAYER_ONESHOT,
	ACTION_LAYER,
	ACTION_ONESHOT
};

struct key_descriptor
{
	enum action action;
	union {
		uint32_t keyseq;
		uint8_t mods;
		uint8_t layer;
	} arg, arg2;
};

struct keyboard_config {
	char name[256];

	struct key_descriptor layers[MAX_LAYERS][KEY_CNT];
	struct keyboard_config *next;
};

extern struct keyboard_config *configs;

void config_generate();
void config_free();

#endif
