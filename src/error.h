#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>

extern char errstr[1024];
void _die(char *fmt, ...);

#define err(fmt, ...) snprintf(errstr, sizeof(errstr), fmt, ##__VA_ARGS__);
#define die(fmt, ...) _die("%s:%d: "fmt, __FILE__, __LINE__, ## __VA_ARGS__);


#endif
