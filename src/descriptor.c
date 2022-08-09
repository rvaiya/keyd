/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "unicode.h"
#include "macro.h"
#include "layer.h"
#include "descriptor.h"
#include "ini.h"
#include "command.h"
#include "config.h"
#include "error.h"
#include "keys.h"
#include "string.h"

static struct {
	const char *name;
	uint8_t op;
	enum {
		ARG_EMPTY,

		ARG_MACRO,
		ARG_LAYER,
		ARG_LAYOUT,
		ARG_TIMEOUT,
		ARG_DESCRIPTOR,
	} args[MAX_DESCRIPTOR_ARGS];
} actions[] =  {
	{ "swap",	OP_SWAP,	{ ARG_LAYER } },
	{ "swap2",	OP_SWAP2,	{ ARG_LAYER, ARG_MACRO } },
	{ "oneshot",	OP_ONESHOT,	{ ARG_LAYER } },
	{ "toggle",	OP_TOGGLE,	{ ARG_LAYER } },
	{ "toggle2",	OP_TOGGLE2,	{ ARG_LAYER, ARG_MACRO } },
	{ "layer",	OP_LAYER,	{ ARG_LAYER } },
	{ "overload",	OP_OVERLOAD,	{ ARG_LAYER, ARG_DESCRIPTOR } },
	{ "timeout",	OP_TIMEOUT,	{ ARG_DESCRIPTOR, ARG_TIMEOUT, ARG_DESCRIPTOR } },
	{ "macro2",	OP_MACRO2,	{ ARG_TIMEOUT, ARG_TIMEOUT, ARG_MACRO } },
	{ "setlayout",	OP_LAYOUT,	{ ARG_LAYOUT } },
};

/* Modifies the input string */
static int parse_fn(char *s,
		    char **name,
		    char *args[MAX_DESCRIPTOR_ARGS],
		    size_t *nargs)
{
	char *c, *arg;

	c = s;
	while (*c && *c != '(')
		c++;

	if (!*c)
		return -1;

	*name = s;
	*c++ = 0;

	while (*c == ' ')
		c++;

	*nargs = 0;
	arg = c;
	while (1) {
		int plvl = 0;

		while (*c) {
			switch (*c) {
			case '\\':
				if (*(c+1)) {
					c+=2;
					continue;
				}
				break;
			case '(':
				plvl++;
				break;
			case ')':
				plvl--;

				if (plvl == -1)
					goto exit;
				break;
			case ',':
				if (plvl == 0)
					goto exit;
				break;
			}

			c++;
		}
exit:

		if (!*c)
			return -1;

		assert(*nargs < MAX_DESCRIPTOR_ARGS);
		args[(*nargs)++] = arg;

		if (*c == ')') {
			*c = 0;
			return 0;
		}

		*c++ = 0;
		while (*c == ' ')
			c++;
		arg = c;
	}
}

int parse_descriptor(const char *descstr,
		     struct descriptor *d,
		     struct config *config)
{
	char *fn = NULL;
	char *args[MAX_DESCRIPTOR_ARGS];
	size_t nargs = 0;
	uint8_t code, mods;
	int ret;
	struct macro macro;
	struct command cmd;

	char fnstr[MAX_EXP_LEN+1];

	if (strlen(descstr) > MAX_EXP_LEN) {
		err("maximum descriptor length exceeded");
		return -1;
	}

	strcpy(fnstr, descstr);

	if (!parse_key_sequence(descstr, &code, &mods)) {
		d->op = OP_KEYSEQUENCE;
		d->args[0].code = code;
		d->args[1].mods = mods;

		/* TODO: fixme. */
		if (keycode_to_mod(code))
			fprintf(stderr,
				"WARNING: mapping modifier keycodes directly may produce unintended results, you probably want layer(<modifier name>) instead\n");

		return 0;
	} else if ((ret=parse_command(descstr, &cmd)) >= 0) {
		if (ret) {
			err("max command length (%d) exceeded\n", MAX_COMMAND_LEN);
			return -1;
		}

		if (config->nr_commands >= MAX_COMMANDS) {
			err("max commands (%d), exceeded", MAX_COMMANDS);
			return -1;
		}


		d->op = OP_COMMAND;
		d->args[0].idx = config->nr_commands;

		config->commands[config->nr_commands++] = cmd;

		return 0;
	} else if ((ret=parse_macro(descstr, &macro)) >= 0) {
		if (ret) {
			return -1;
		}

		if (config->nr_macros >= MAX_MACROS) {
			err("max macros (%d), exceeded", MAX_MACROS);
			return -1;
		}

		d->op = OP_MACRO;
		d->args[0].idx = config->nr_macros;

		config->macros[config->nr_macros++] = macro;

		return 0;
	} else if (!parse_fn(fnstr, &fn, args, &nargs)) {
		int i;

		for (i = 0; i < (int)(sizeof(actions)/sizeof(actions[0])); i++) {
			if (!strcmp(actions[i].name, fn)) {
				int j;

				d->op = actions[i].op;

				for (j = 0; j < MAX_DESCRIPTOR_ARGS; j++) {
					if (!actions[i].args[j])
						break;
				}

				if ((int)nargs != j) {
					err("%s requires %d %s", actions[i].name, j, j == 1 ? "argument" : "arguments");
					return -1;
				}

				while (j--) {
					int type = actions[i].args[j];
					descriptor_arg_t *arg = &d->args[j];
					const char *argstr = args[j];
					struct descriptor desc;

					switch (type) {
					case ARG_LAYER:
						arg->idx = config_get_layer_index(config, argstr);
						if (arg->idx == -1 || config->layers[arg->idx].type == LT_LAYOUT) {
							err("%s is not a valid layer", argstr);
							return -1;
						}

						break;
					case ARG_LAYOUT:
						arg->idx = config_get_layer_index(config, argstr);
						if (arg->idx == -1 || config->layers[arg->idx].type != LT_LAYOUT) {
							err("%s is not a valid layout", argstr);
							return -1;
						}

						break;
					case ARG_DESCRIPTOR:
						if (parse_descriptor(argstr, &desc, config))
							return -1;

						if (config->nr_descriptors >= MAX_AUX_DESCRIPTORS) {
							err("maximum descriptors exceeded");
							return -1;
						}

						config->descriptors[config->nr_descriptors] = desc;
						arg->idx = config->nr_descriptors++;
						break;
					case ARG_TIMEOUT:
						arg->timeout = atoi(argstr);
						break;
					case ARG_MACRO:
						if (config->nr_macros >= MAX_MACROS) {
							err("max macros (%d), exceeded", MAX_MACROS);
							return -1;
						}

						if (parse_macro(argstr, &config->macros[config->nr_macros])) {
							return -1;
						}

						arg->idx = config->nr_macros;
						config->nr_macros++;

						break;
					default:
						assert(0);
						break;
					}
				}

				return 0;
			}
		}
	}

	err("invalid key or action");
	return -1;
}
