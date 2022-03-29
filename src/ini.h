/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#ifndef INI_H
#define INI_H

#include <stdint.h>

#define MAX_SECTIONS 32
#define MAX_SECTION_ENTRIES 128

struct ini_entry {
	char *line;
	size_t lnum;		// The line number in the original source file.
};

struct ini_section {
	char name[256];

	size_t nr_entries;
	size_t lnum;

	struct ini_entry entries[MAX_SECTION_ENTRIES];
};

struct ini {
	size_t nr_sections;

	struct ini_section sections[MAX_SECTIONS];
};

/*
 * Reads a file of the form:
 *
 *  [section]
 *
 *  # Comment
 *
 *  entry1
 *  entry2
 *
 *  [section2]
 *  ...
 *
 *  Stripping comments and empty lines along the way.
 *  Each entry is a non comment, non empty line
 *  sripped of leading whitespace. If a default
 *  section name is supplied then entries not
 *  listed under an explicit heading will be
 *  returned under the named section.
 *
 *  The returned result is statically allocated and only
 *  valid until the next invocation. It should not be
 *  freed.
 */

struct ini	*ini_parse_file(const char *path, const char *default_section_name);

int		parse_kvp(char *s, char **key, char **value);

#endif
