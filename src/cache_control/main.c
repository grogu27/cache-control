#include "../../include/cache_control/options.h"

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

	return 0;
}
