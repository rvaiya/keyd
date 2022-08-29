/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#ifndef UNICODE_H
#define UNICODE_H

#include <stdint.h>

/*
 * Overview
 *
 * Unicode input is accomplished using one of several 'input methods' or
 * 'IMEs'. The X input method (xim) is the name of the default input method
 * which ships with X, and currently seems to be the most widely supported one.
 * An emerging competitor called 'ibus' exists, but seems to be less
 * ubiquitous, a notable advantage being that it allows codepoints to be input
 * directly by their hex values (C-S-u).
 *
 * xim, by contrast, works by requiring the user to explicitly specify a
 * mapping for each codepoint of interest in an XCompose(5) file, which maps a
 * sequence of keysyms (usually beginning with a dedicated 'compose' key
 * (<Multi_key>)) into a valid utf8 output string.
 *
 * Unfortunately xim doesn't provide a mechanism by which arbitrary unicode
 * points can be input, so we have to construct an XCompose file containing
 * explicit mappings between each sequence and the desired utf8 output.
 *
 * ~/.XCompose Constraints:
 *
 * To compound matters, every program/toolkit seems to have its own XCompose
 * parser and consequently only supports a subset of the full spec. The
 * following real-world constraints have been arrived at empirically:
 *
 * 1. Each sequence should be less than 6 characters since some programs (e.g
 * chrome) seem to have a maximum sequence length.
 *
 * 2. No sequence should be a subset of another sequence since some programs
 * don't handle this properly (e.g kitty)
 *
 * 3. Sequences should confine themselves to keysyms available on all layouts
 * (e.g no a-f (hex)).
 *
 * Approach
 *
 * In order to satisfy the above constraints, we create an XCompose file
 * mapping each codepoint's index in a lookup table to the desired utf8
 * sequence. The use of a table index instead of the codepoint value ensures
 * all codepoints consist of a maximum of 3 base36 encoded digits (since there are
 * <35k of them).  Each codepoint is zero-left padded to 3 characters to avoid
 * the subset issue.
 *
 * Finally, we use cancel as our compose prefix so the user doesn't have to
 * faff about with XkbOptions. This technically introduces the possibility of a
 * conflict, but I haven't found any evidence which suggests that Linefeed is
 * anything other than a vestigial keypendage from a more glorious era.
 *
 * </end_verbiage>
 */


int unicode_lookup_index(uint32_t codepoint);
void unicode_get_sequence(int idx, uint8_t codes[4]);

#endif
