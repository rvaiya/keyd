/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#include "log.h"
#include <time.h>

char errstr[2048];

int log_level = 0;
int suppress_colours = 0;

static const char *colorize(const char *s)
{
	int i;

	static char buf[1024];
	size_t n  = 0;
	int inside_escape = 0;

	for (i = 0; s[i] != 0 && n < sizeof(buf); i++) {
		if (s[i+1] == '{') {
			int escape_num = 0;

			switch (s[i]) {
				case 'r': escape_num = 1; break;
				case 'g': escape_num = 2; break;
				case 'y': escape_num = 3; break;
				case 'b': escape_num = 4; break;
				case 'm': escape_num = 5; break;
				case 'c': escape_num = 6; break;
				case 'w': escape_num = 7; break;
				default: break;
			}

			if (escape_num) {
				if (!suppress_colours && (sizeof(buf)-n > 5)) {
					buf[n++] = '\033';
					buf[n++] = '[';
					buf[n++] = '3';
					buf[n++] = '0' + escape_num;
					buf[n++] = 'm';
				}

				inside_escape = 1;

				i++;
				continue;
			}
		}

		if (s[i] == '}' && inside_escape) {
			if (!suppress_colours && (sizeof(buf)-n > 4)) {
				memcpy(buf+n, "\033[0m", 4);
				n += 4;
			}

			inside_escape = 0;
			continue;
		}

		buf[n++] = s[i];
	}

	buf[n] = 0;

	return buf;
}

void die(const char *fmt, ...) {
	fprintf(stderr, "%s", colorize("r{FATAL ERROR:} "));

	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, colorize(fmt), ap);
	va_end(ap);
	fprintf(stderr, "\n");

	exit(-1);
}

void _keyd_log(int level, const char *fmt, ...)
{
	if (level > log_level)
		return;

	va_list ap;
	va_start(ap, fmt);
	vprintf(colorize(fmt), ap);
	va_end(ap);
}
