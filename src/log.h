/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#ifndef KEYD_LOG_H
#define KEYD_LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#define keyd_log(fmt, ...) _keyd_log(0, fmt, ##__VA_ARGS__);

#define dbg(fmt, ...) _keyd_log(1, "r{DEBUG:} b{%s:%d:} "fmt"\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define dbg2(fmt, ...) _keyd_log(2, "r{DEBUG:} b{%s:%d:} "fmt"\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define err(fmt, ...) snprintf(errstr, sizeof(errstr), fmt, ##__VA_ARGS__);

void _keyd_log(int level, const char *fmt, ...);
void die(const char *fmt, ...);

extern int log_level;
extern int suppress_colours;
extern char errstr[2048];

#endif
