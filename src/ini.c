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

#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "ini.h"

int ini_parse(char *s, struct ini *ini, const char *default_section_name)
{
	int ln = 0;
	size_t n = 0;

	struct ini_section *section = NULL;

	while (s) {
		struct ini_entry *ent;
		char *line;

		ln++;

		line = s;
		s = strchr(s, '\n');

		if (s) {
			*s = 0;
			s++;
		}


		while (isspace(line[0]))
			line++;

		if (line[0] == 0)
			continue;

		switch (line[0]) {
		size_t len;
		case '[':
			len = strlen(line);

			if (line[len-1] == ']') {
				assert(n < MAX_SECTIONS);

				section = &ini->sections[n++];

				line[len-1] = 0;

				strncpy(section->name, line+1, sizeof(section->name));
				section->nr_entries = 0;
				section->lnum = ln;

				continue;
			}

			break;
		case '#':
			continue;
		}

		if (!section) {
			if(default_section_name) {
				section = &ini->sections[n++];
				strcpy(section->name, default_section_name);

				section->nr_entries = 0;
				section->lnum = 0;
			} else
				return -1;
		}

		assert(section->nr_entries < MAX_SECTION_ENTRIES);

		ent = &section->entries[section->nr_entries++];

		ent->line = line;
		ent->lnum = ln;
	}

	ini->nr_sections = n;
	return 0;
}
