#include "keyd.h"

static size_t dbg_print_device_bits(int fd, const char *name, int type, int max, char *out, size_t out_sz)
{
	size_t i;
	size_t j;
	size_t sz = 0;
	uint8_t arr[1024] = {0};
	size_t n = (max+7)/8;
	int has_set_bits = 0;

	if (ioctl(fd, EVIOCGBIT(type, n), arr) == -1) {
		perror("ioctl");
		exit(-1);
	}

	sz = snprintf(out, out_sz, "\t%s: ", name);
	for (i = 0; i < n; i++)
		for (j = 0; j < 8; j++)
			if ((arr[i] >> j) & 0x01) {
				size_t val = 8*i + j;

				int nw = snprintf(out + sz, out_sz - sz, "%zu ", val);
				assert (nw > 0 && ((sz + nw) < out_sz));
				sz += nw;

				has_set_bits = 1;
			}

	if (!has_set_bits) {
		out[0] = 0;
		return 0;
	}

	sz += snprintf(out + sz, out_sz - sz, "\n");

	return sz;
}

void dbg_print_evdev_details(const char *path)
{
	size_t sz = 0;
	char out[4096] = {0};
	char name[128];

	if (log_level < 2)
		return;

	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		dbg2("Failed to open %s", path);
		return;
	}

	if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) == -1) {
		perror("ioctl");
		return;
	}

	sz = snprintf(out, sizeof out, "(%s) (%s):\n", path, name);
	sz += dbg_print_device_bits(fd, "EV", 0, EV_MAX, out + sz, sizeof(out) - sz);
	sz += dbg_print_device_bits(fd, "EV_KEY", EV_KEY, KEY_MAX, out + sz, sizeof(out) - sz);
	sz += dbg_print_device_bits(fd, "EV_REL", EV_REL, REL_MAX, out + sz, sizeof(out) - sz);
	sz += dbg_print_device_bits(fd, "EV_ABS", EV_ABS, ABS_MAX, out + sz, sizeof(out) - sz);
	sz += dbg_print_device_bits(fd, "EV_MSC", EV_MSC, MSC_MAX, out + sz, sizeof(out) - sz);
	sz += dbg_print_device_bits(fd, "EV_SW", EV_SW, SW_MAX, out + sz, sizeof(out) - sz);
	sz += dbg_print_device_bits(fd, "EV_LED", EV_LED, LED_MAX, out + sz, sizeof(out) - sz);

	_keyd_log(2, "r{DEBUG}: %s", out);
	close(fd);
}
