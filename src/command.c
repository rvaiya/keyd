/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#include <string.h>

#include "command.h"
#include "string.h"

int parse_command(const char *s, struct command *command)
{
	int len = strlen(s);

	if (len == 0 || strstr(s, "command(") != s || s[len-1] != ')')
		return -1;

	if (len > MAX_COMMAND_LEN)
		return 1;

	strcpy(command->cmd, s+8);
	command->cmd[len-9] = 0;
	str_escape(command->cmd);

	return 0;
}
