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

#include <stdint.h>
#include "keys.h"

uint16_t keycode_to_mod(uint16_t code)
{
	switch (code) {
	case KEY_LEFTSHIFT:
	case KEY_RIGHTSHIFT:
		return MOD_SHIFT;
	case KEY_LEFTALT:
		return MOD_ALT;
	case KEY_RIGHTALT:
		return MOD_ALT_GR;
	case KEY_RIGHTCTRL:
		return MOD_CTRL;
	case KEY_LEFTMETA:
	case KEY_RIGHTMETA:
		return MOD_SUPER;
	}

	return 0;
}

int parse_modset(const char *s, uint16_t *mods)
{
	*mods = 0;

	while (*s) {
		switch (*s) {
		case 'C':
			*mods |= MOD_CTRL;
			break;
		case 'M':
			*mods |= MOD_SUPER;
			break;
		case 'A':
			*mods |= MOD_ALT;
			break;
		case 'S':
			*mods |= MOD_SHIFT;
			break;
		case 'G':
			*mods |= MOD_ALT_GR;
			break;
		default:
			return -1;
			break;
		}

		if (s[1] == 0)
			return 0;
		else if (s[1] != '-')
			return -1;

		s += 2;
	}

	return 0;
}

