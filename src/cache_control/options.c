#include "../../include/cache_control/options.h"

//выводит описание всех доступных опций
void print_info()
{
	printf("\t\t\tPre-launch information\n");
	printf("\n1) cachecontrol -d -> run in the background with default config\n");
	printf("2) cachecontrol -c <config> -> run in the foreground with your path to the config\n");
	printf("3) cachecontrol -d -c <config> -> run in the background with your path to the config\n");
	printf("4) cachecontrol -> run in the foreground with default config\n");
}

//использует stat() для получения информации о файле и проверяет существование файла
// возвращает: -1 если ошибка (файла нет, это директория), 0 если путь ведет к обычному файлу
int validate_path_config(const char *path)
{
	struct stat st;

	if (stat(path, &st) != 0) {
		//логирование, проблемы с файлом не может записать в st
		printf("Something is wrong file: %s\n", path);
		return -1;
	}

	if (S_ISREG(st.st_mode)) {
		//логирование, данный путь ведет к файлу
		return 0;
	} else {
		//логирвоание, данный путь ведет к директории или что-то другое
		if (S_ISDIR(st.st_mode)) {
			printf("Config path is a directory, not a file: %s\n",
			       path);
		} else {
			printf("Config path is not a regular file: %s\n", path);
		}
		return -1;
	}
}

//парсинг аргументов командной строки
//возвращает: 0 если успех, -1 если ошибка, -2 если показано описание
int parse_options(int argc, char **argv, struct program_options *po)
{
	po->daemon_flag = 0;
	po->config_path[0] = '\0';

	int opt;
	while ((opt = getopt(argc, argv, "dc:h")) != -1) {
		switch (opt) {
		case 'd':
			//логирование то, что запустился с -d и у нас используется дефолтный конфиг ;)
			po->daemon_flag = 1;
			break;

		case 'c':
			//логирование то, что запустился с -c и со своим путем ;)
			if (strlen(optarg) >= PATH_SIZE) {
				//логирование, попытались запустить с большим путем ;(
				printf("Config path is too long\n");
				return -1;
			}
			if (strlen(optarg) == 0) {
				//логирование, строка пуста ;(
				printf("Config path is empty\n");
				return -1;
			}
			strcpy(po->config_path, optarg);
			break;

		case 'h':
			//логирование то, что запустили с -h для помощи ;)
			print_info();
			return -2;

		case '?':
			//логирование, ввели не правильные опции ;(
			printf("Uknown option, try run with option -h for help\n");
			return -1;

		default:
			//логирование, getopt завершился с ошибкой ;(
			return -1;
		}
	}

	if (po->config_path[0] == '\0') {
		strcpy(po->config_path,
		       "config/cachecontrol.conf"); // /etc/cachecontrol.conf
	}

	if (validate_path_config(po->config_path) != 0) {
		return -1;
	}

	return 0;
}
