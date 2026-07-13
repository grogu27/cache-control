#include "../../include/cache_control/options.h"

void print_info()
{
	printf("\t\t\tPre-launch information\n");
	printf("\n1) cachecontrol -d -> run in the background with default config\n");
	printf("2) cachecontrol -c <config> -> run in the foreground with your path to the config\n");
	printf("3) cachecontrol -d -c <config> -> run in the background with your path to the config\n");
	printf("4) cachecontrol -> run in the foreground with default config\n");
	printf("5) cachecontrol -h -> show help menu\n");
}

int validate_path_config(char *path)
{
	struct stat st;

	if (stat(path, &st) != 0) {
		logging(LOG_ERR, "<validate_path_config> stat failed");
		printf("Something is wrong file: %s\n", path);
		return -1;
	}

	if (S_ISREG(st.st_mode)) {
		logging(LOG_INFO,
			"<validate_path_config> path is a regular file");
		return 0;
	} else {
		if (S_ISDIR(st.st_mode)) {
			logging(LOG_ERR,
				"<validate_path_config> path is a directory");
			printf("Config path is a directory, not a file: %s\n",
			       path);
		} else {
			logging(LOG_ERR,
				"<validate_path_config> path is not a regular file");
			printf("Config path is not a regular file: %s\n", path);
		}
		return -1;
	}
}

int parse_options(int argc, char **argv, struct cache_control *cc)
{
	int opt;
	char path_buf[PATH_SIZE] = { 0 };

	while ((opt = getopt(argc, argv, "dc:h")) != -1) {
		switch (opt) {
		case 'd':
			logging(LOG_INFO,
				"<parse_options> start with -d flag (daemon mode)");
			cc->daemon_flag = 1;
			break;

		case 'c':
			if (strlen(optarg) >= PATH_SIZE) {
				logging(LOG_ERR,
					"<parse_options> config path is too long");
				printf("Config path is too long\n");
				return -1;
			}
			if (strlen(optarg) == 0) {
				logging(LOG_ERR,
					"<parse_options> config path is empty");
				printf("Config path is empty\n");
				return -1;
			}
			strcpy(cc->config_path, optarg);
			if (realpath(cc->config_path, path_buf) == NULL) {
				logging(LOG_ERR,
					"<parse_options> realpath failed");
				return -1;
			}
			strcpy(cc->config_path, path_buf);
			logging(LOG_INFO, "<parse_options> start with -c flag");
			break;

		case 'h':
			logging(LOG_INFO,
				"<parse_options> start with -h flag (help)");
			print_info();
			return -2;

		case '?':
			logging(LOG_ERR,
				"<parse_options> unknow option, please try run with option -h for help");
			printf("Uknown option, try run with option -h for help\n");
			return -1;

		default:
			logging(LOG_ERR,
				"<parse_options> getopt returned invalid value");
			return -1;
		}
	}

	if (cc->config_path[0] == '\0') {
		if (realpath("config/cachecontrol.conf", path_buf) == NULL) {
			logging(LOG_ERR,
				"<parse_options> realpath with default path failed");
			return -1;
		}
		strcpy(cc->config_path, path_buf);
	}

	if (validate_path_config(cc->config_path) != 0) {
		return -1;
	}

	logging(LOG_INFO, "<validate_path_config> validation successful");

	logging(LOG_INFO, "<parse_options> options parsed successfully");
	return 0;
}
