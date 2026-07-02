#include "../../include/cache_control/options.h"
#include "../../include/cache_control/daemonize.h"

int main(int argc, char **argv)
{
	struct program_options options;

	int result = parse_options(argc, argv, &options);

	if (result == -2) {
		//логирование, вывелось описание опций ;)
		return EXIT_SUCCESS;
	}

	if (result != 0) {
		//логирование, парсинг прошел не успешно ;(
		printf("Options parsing is failed\n");
		return EXIT_FAILURE;
	}

	//логирование, парсинг прошел успешно ;)
	printf("Options parsing is success\n");

	if (options.daemon_flag == 1) {
		if (daemonize() != 0) {
			return EXIT_FAILURE;
		}
		//логирование, демон запустил с конфигом ;)
	} else {
		printf("Program run in foreground\n");
		printf("Config path: %s\n", options.config_path);
		//логирование, запуск произошел, но не фоне ;)
	}

	return 0;
}
