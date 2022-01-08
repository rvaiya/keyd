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
	struct layer_entry *ent;

	for (ent = layer->entries; ent; ent = ent->next)
		if (ent->code == code)
			return &ent->descriptor;

	return NULL;
}

void layer_set_descriptor(struct layer *layer,
			  uint16_t code,
			  const struct descriptor *descriptor)
{
	struct layer_entry **ent, *new;

	if (!code)
		return;

	ent = &layer->entries;
	while (*ent) {
		if ((*ent)->code == code) {
			struct layer_entry *tmp = *ent;
			*ent = (*ent)->next;
			free(tmp);
			continue;
		}

		ent = &(*ent)->next;
	}

	new = malloc(sizeof(struct layer_entry));

	new->code = code;
	new->descriptor = *descriptor;

	new->next = layer->entries;

	layer->entries = new;
}

struct layer *create_layer(const char *name, uint16_t mods)
{
	uint16_t code;
	struct layer *layer;

	layer = calloc(1, sizeof(struct layer));
	layer->mods = mods;
	strcpy(layer->name, name);


	return layer;
}

struct layer *layer_copy(const struct layer *layer)
{
	struct layer_entry *ent;
	struct layer *nl;

	nl = malloc(sizeof(struct layer));
	*nl = *layer;

	nl->entries = NULL;

	for (ent = layer->entries; ent; ent = ent->next) {
		struct layer_entry *ne = malloc(sizeof(struct layer_entry));

		ne->code = ent->code;
		ne->descriptor = ent->descriptor;

		ne->next = nl->entries;
		nl->entries = ne;
	}

	return nl;
}

void free_layer(struct layer *layer)
{
	struct layer_entry *ent = layer->entries;

	while (ent) {
		struct layer_entry *tmp = ent;
		ent = ent->next;
		free(tmp);
	}

	free(layer);
}
