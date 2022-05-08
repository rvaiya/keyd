/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#ifndef KEYD_H_
#define KEYD_H_

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

void set_led(int led, int state);

#endif
