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

#include <stdlib.h>
#include <string.h>

#include "layer.h"
#include "keys.h"
#include "descriptor.h"

struct descriptor *layer_get_descriptor(const struct layer *layer, uint16_t code)
{
	struct keymap_entry *ent;

	for (ent = layer->_keymap; ent; ent = ent->next)
		if (ent->code == code)
			return &ent->descriptor;

	return NULL;
}

void layer_set_descriptor(struct layer *layer,
			  uint16_t code,
			  const struct descriptor *descriptor)
{
	struct keymap_entry *ent = layer->_keymap;

	while (ent) {
		if (ent->code == code) {
			ent->descriptor = *descriptor;
			return;
		}

		ent = ent->next;
	}

	ent = malloc(sizeof(struct keymap_entry));

	ent->code = code;
	ent->descriptor = *descriptor;

	ent->next = layer->_keymap;

	layer->_keymap = ent;
}

struct layer *create_layer(const char *name, uint16_t mods, int is_layout)
{
	uint16_t code;
	struct layer *layer;

	layer = calloc(1, sizeof(struct layer));
	layer->mods = mods;
	layer->is_layout = is_layout;
	strcpy(layer->name, name);

	if (is_layout) {
		for (code = 0; code < KEY_MAX; code++) {
			struct descriptor d;

			d.op = OP_KEYSEQ;
			d.args[0].sequence.code = code;
			d.args[0].sequence.mods = 0;

			layer_set_descriptor(layer, code, &d);
		}
	}

	return layer;
}

void free_layer(struct layer *layer)
{
	struct keymap_entry *ent = layer->_keymap;

	while (ent) {
		struct keymap_entry *tmp = ent;
		ent = ent->next;
		free(tmp);
	}

	free(layer);
}
