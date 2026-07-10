#include "../../include/cache_control/options.h"
#include "../../include/cache_control/daemonize.h"
#include "../../include/cache_control/signal_handler.h"
#include "../../include/cache_control/context.h"
#include "../../include/cache_control/parse_config.h"
#include "../../include/cache_control/threads.h"
#include "../../include/log/log.h"

int main(int argc, char **argv)
{
	struct cache_control cc;
	init_context(&cc);

	int result = parse_options(argc, argv, &cc);

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

	if (cc.daemon_flag == 1) {
		if (daemonize() != 0) {
			return EXIT_FAILURE;
		}
		//логирование, демон запустил с конфигом ;)
	} else {
		printf("Program run in foreground\n");
		printf("Config path: %s\n", cc.config_path);
		//логирование, запуск произошел, но не фоне ;)
	}

	set_context(&cc);
	//логирование, структура передалась к орбаботчикам сигналов

	init_signal_handler();
	//логирование, обработчики сигналов инициализированы
	cc.file_list = init_list();
	if (cc.file_list == NULL) {
		// ошибка выделения памяти
		return EXIT_FAILURE;
	}
	int res_parse_config =
		parse_config(cc.config_path, cc.file_list, &cc.interval);
	if (res_parse_config == 1) {
		//логирование, не открывается конфиг файл
		return EXIT_FAILURE;
	} else if (res_parse_config == 2) {
		//логирование, синтаксическая ошибка в конфиг файле
		return EXIT_FAILURE;
	} else if (res_parse_config == 3){
		//логирование, семантическая ошибка в конфиг файле
		return EXIT_FAILURE;
	}


	//логирование, запуск потоков начался
	if (start_threads(&cc) != 0) {
		//логирование, запуск поток завершился с ошибкой
		remove_context(&cc);
		return EXIT_FAILURE;
	}
	//логирование, запуск прошел успешно

	//логирование вхождение в бесконечный цикл
	while (!cc.shutdown_flag) {
	}

	//логирвоание, вышли из бесконечного цикла
	if (stop_threads() != 0) {
		//логирование, остановка потоков завершилось неудачно
	} else {
		//логирование, остановка потоков завершилось удачно
	}
	if (remove_context(&cc) != 0) {
		//логирование, удаление структуры завершилось неудачно
	} else {
		//логирование, удаление структуры завершилось успешно
	}
	free_list(cc.file_list);
	return EXIT_SUCCESS;
}
