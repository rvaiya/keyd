#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>

extern char errstr[1024];
#define err(fmt, ...) snprintf(errstr, sizeof(errstr), fmt, ##__VA_ARGS__);

#endif
