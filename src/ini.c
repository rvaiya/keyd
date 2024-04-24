/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>

#include "ini.h"
#define MAX_INI_SIZE 65536

/*
 * Parse a value of the form 'key = value'. The value may contain =
 * and the key may itself be = as a special case. value may be NULL
 * if '=' is not present in the original string.
 */

void parse_kvp(char *s, char **key, char **value)
{
	char *last_space = NULL;
	char *c = s;

	/* Allow the first character to be = as a special case. */
	if (*c == '=')
		c++;

	*key = s;
	*value = NULL;
	while (*c) {
		switch (*c) {
		case '=':
			if (last_space)
				*last_space = 0;
			else
				*c = 0;

			while (*++c == ' ' || *c == '\t');

			*value = c;
			return;
		case ' ':
		case '\t':
			if (!last_space)
				last_space = c;
			break;
		default:
			last_space = NULL;
			break;
		}

		c++;
	}
}

/*
 * The result is statically allocated and should not be freed by the caller.
 * The returned struct is only valid until the next invocation. The
 * input string may be modified and should only be freed after the
 * returned ini struct is no longer required.
 */

struct ini *ini_parse_string(char *s, const char *default_section_name)
{
	static struct ini ini;

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

				section = &ini.sections[n++];

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
				section = &ini.sections[n++];
				strcpy(section->name, default_section_name);

				section->nr_entries = 0;
				section->lnum = 0;
			} else
				return NULL;
		}

		assert(section->nr_entries < MAX_SECTION_ENTRIES);

		ent = &section->entries[section->nr_entries++];
		parse_kvp(line, &ent->key, &ent->val);
		ent->lnum = ln;
	}

	ini.nr_sections = n;
	return &ini;
}
