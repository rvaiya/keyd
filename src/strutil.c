/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#include <assert.h>

#include "strutil.h"

int utf8_read_char(const char *_s, uint32_t *code)
{
	const unsigned char *s = (const unsigned char*)_s;

	if (!s[0])
		return 0;

	if (s[0] >= 0xF0) {
		assert(s[1]);
		assert(s[2]);
		assert(s[3]);
		*code = (s[0] & 0x07) << 18 | (s[1] & 0x3F) << 12 | (s[2] & 0x3F) << 6 | (s[3] & 0x3F);
		return 4;
	} else if (s[0] >= 0xE0) {
		assert(s[1]);
		assert(s[2]);
		*code = (s[0] & 0x0F) << 12 | (s[1] & 0x3F) << 6 | (s[2] & 0x3F);
		return 3;
	} else if (s[0] >= 0xC0) {
		assert(s[1]);
		*code = (s[0] & 0x1F) << 6 | (s[1] & 0x3F);
		return 2;
	} else {
		*code = s[0] & 0x7F;
		return 1;
	}
}

int utf8_strlen(const char *s)
{
	uint32_t code;
	int csz;
	int n = 0;

	while ((csz = utf8_read_char(s, &code))) {
		n++;
		s+=csz;
	}

	return n;
}

int is_timeval(const char *s)
{
	if (s[0] < '0' || s[0] > '9')
		return 0;

	while(*s && *s >= '0' && *s <= '9')
		s++;

	return s[0] == 'm' && s[1] == 's' && (s[2] == 0 || s[2] == '\n');
}

/*
 * Returns the character size in bytes, or 0 in the case of the empty string.
 */
size_t str_escape(char *s)
{
	int i = 0;
	int n = 0;

	for (i = 0; s[i]; i++) {
		if (s[i] == '\\') {
			switch (s[i+1]) {
			case 'n':
				s[n++] = '\n';
				break;
			case 't':
				s[n++] = '\t';
				break;
			case '\\':
				s[n++] = '\\';
				break;
			case ')':
				s[n++] = ')';
				break;
			case '(':
				s[n++] = '(';
				break;
			case 0:
				s[n] = 0;
				return n;
			default:
				s[n++] = '\\';
				s[n++] = s[i+1];
				break;
			}

			i++;
		} else {
			s[n++] = s[i];
		}
	}

	s[n] = 0;

	return n;
}

