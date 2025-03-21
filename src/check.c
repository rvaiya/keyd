#include "keyd.h"

static int rc = 0;

static void validate(const char *path)
{
	int ret;
	struct config config;

	keyd_log("Parsing b{%s}\n", path);

	ret = config_parse(&config, path);
	if (ret != 0) {
		if (ret < 0)
			keyd_log("\tr{FAILED} (file does not exist?)\n");

		rc = -1;
	}
}

int check(int argc, char *argv[])
{
	int i;

	if (argc > 1) {
		for (i = 1; i < argc; i++)
			validate(argv[i]);
	} else {
		DIR *dh;
		int ret;
		struct dirent *ent;

		dh = opendir(CONFIG_DIR);
		if (!dh) {
			perror("chdir");
			return -1;
		}

		while ((ent = readdir(dh))) {
			char path[PATH_MAX];

			snprintf(path, sizeof path, "%s/%s", CONFIG_DIR, ent->d_name);

			size_t len = strlen(path);
			if (len > 5 && !strcmp(path + len - 5, ".conf"))
				validate(path);
		}
		closedir(dh);
	}

	if (rc == 0)
		keyd_log("\nNo errors found.\n");

	return rc;
}
