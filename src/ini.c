/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
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
		size_t len;
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

		len = strlen(line);

		while(len > 0 && isspace(line[len-1]))
			len--;

		if (line[0] == 0)
			continue;

		line[len] = 0;

		switch (line[0]) {
		case '[':
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
