/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#ifndef COMMNAD_H
#define COMMNAD_H

#define MAX_COMMAND_LEN	256

struct command {
	char cmd[MAX_COMMAND_LEN+1];
};

int parse_command(const char *s, struct command *command);
#endif
