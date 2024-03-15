#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../src/keyd.h"

#define MAX_EVENTS 1024

struct key_event output[MAX_EVENTS];
size_t noutput = 0;

static uint64_t get_time_ns()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (uint64_t)(ts.tv_sec*1E9)+(uint64_t)ts.tv_nsec;
}

static uint8_t lookup_code(const char *name)
{
	size_t i;

	if (!strcmp(name, "control"))
		return KEYD_LEFTCTRL;
	if (!strcmp(name, "shift"))
		return KEYD_LEFTSHIFT;
	if (!strcmp(name, "meta"))
		return KEYD_LEFTMETA;
	if (!strcmp(name, "alt"))
		return KEYD_LEFTALT;

	for (i = 0; i < ARRAY_SIZE(keycode_table); i++)
		if (keycode_table[i].name && !strcmp(keycode_table[i].name, name))
			return i;
	return 0;
}

static void send_key(uint8_t code, uint8_t pressed)
{
	output[noutput].code = code;
	output[noutput].pressed = pressed;
	noutput++;
}

static char *read_file(const char *path)
{
	int fd = open(path, O_RDONLY);
	static char buf[4096];
	size_t sz = 0;
	ssize_t n;

	if (fd < 0) {
		perror("open");
		exit(-1);
	}

	while ((n = read(fd, buf, sizeof(buf) - sz)) > 0) {
		sz += n;
		assert(sz < sizeof buf);
	}

	buf[sz] = 0;
	return buf;
}

static int cmp_events(struct key_event *input, size_t nin,
		      struct key_event *output, size_t nout)
{
	size_t i;

	if (nin != nout)
		return -1;

	for (i = 0; i < nin; i++) {
		if (input[i].code != output[i].code
		    || input[i].pressed != output[i].pressed)
			return -1;
	}

	return 0;
}

static int print_diff(struct key_event *expected, size_t nexp,
		      struct key_event *output, size_t nout)
{
	int i;
	int n = nout > nexp ? nout : nexp;
	int ret = 0;

	printf("\n%-30s%s\n\n", "Expected", "Output");

	for (i = 0; i < n; i++) {
		int np = 0;
		int match = i < nexp &&
		    i < nout &&
		    output[i].code == expected[i].code &&
		    output[i].pressed == expected[i].pressed;

		if (!match)
			ret = -1;

		if (!match)
			printf("\033[32;1m");

		np = 0;
		if (i < nexp)
			np = printf("%s %s",
				    keycode_table[expected[i].code].name,
				    expected[i].pressed ? "down" : "up");

		while (np++ < 30)
			printf(" ");

		if (!match)
			printf("\033[0m\033[31;1m");

		if (i < nout)
			printf("%s %s",
			       keycode_table[output[i].code].name,
			       output[i].pressed ? "down" : "up");

		if (!match)
			printf("\033[0m");

		printf("\n");
	}

	return ret;
}

static int parse_events(char *s, struct key_event in[MAX_EVENTS], size_t *nin,
			struct key_event out[MAX_EVENTS], size_t *nout)
{
	int ret;
	int time = 0;
	int ln = 0;
	int n = 0;
	struct key_event *events = in;

	char *line = s;
	*nin = 0;
	*nout = 0;

	while (1) {
		int len;
		char *end = strchr(line, '\n');

		if (!end)
			break;
		*end = 0;

		len = strlen(line);

		ln++;

		if (line[0] == '#')
			goto next;

		while (line[0] == ' ')
			line++;

		if (!line[0]) {
			*nin = n;
			events = out;
			n = 0;

			goto next;
		}

		if (len >= 2 && line[len - 1] == 's' && line[len - 2] == 'm') {
			time += atoi(line);
		} else {
			uint8_t code;
			char *k = strtok(line, " ");
			char *v = strtok(NULL, " \n");

			if (!v || (strcmp(v, "up") && strcmp(v, "down"))) {
				printf("%d: Invalid line\n", ln);
				goto next;
			}

			if (!(code = lookup_code(k))) {
				printf("%d: %s is not a valid key\n", ln,
				       k);
				goto next;
			}

			assert(n < MAX_EVENTS);
			events[n].code = code;
			events[n].pressed = !strcmp(v, "down");
			events[n].timestamp = time;
			n++;
		}

	      next:
		line = end + 1;
	}

	*nout = n;
	return 0;
}

uint64_t run_test(struct keyboard *kbd, const char *path)
{
	uint64_t time;
	char *data = read_file(path);

	struct key_event input[MAX_EVENTS];
	size_t ninput;

	struct key_event expected[MAX_EVENTS];
	size_t nexpected;

	if (parse_events(data, input, &ninput, expected, &nexpected) < 0) {
		fprintf(stderr, "Failed to parse input\n");
		exit(-1);
	}

	noutput = 0;

	time = get_time_ns();
	kbd_process_events(kbd, input, ninput);
	time = get_time_ns()-time;

	if (cmp_events(output, noutput, expected, nexpected)) {
		printf("%s \033[31;1mFAILED\033[0m\n", path);
		print_diff(expected, nexpected, output, noutput);
		exit(-1);
	} else {
		printf("%s \033[32;1mPASSED\033[0m (%zu us)\n", path, time/1000);
	}

	return time;
}

static void on_layer_change(const struct keyboard *kbd, const struct layer *layer, uint8_t active)
{
}

int main(int argc, char *argv[])
{
	size_t i;
	struct config config;
	uint64_t total_time;

	struct keyboard *kbd;

	struct output output = {
		.send_key = send_key,
		.on_layer_change = on_layer_change,
	};

	if (argc < 2) {
		printf
		    ("usage: %s <test config> <test file> [<test file>...]\n",
		     argv[0]);
		return -1;
	}

	if (config_parse(&config, argv[1])) {
		printf("Failed to parse config %s\n", argv[1]);
		return -1;
	}

	kbd = new_keyboard(&config, &output);

	for (i = 2; i < argc; i++)
		total_time += run_test(kbd, argv[i]);

	printf("\nTotal time spent in the main loop: %zu us\n", total_time/1000);
	return 0;
}
