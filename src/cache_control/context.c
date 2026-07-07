#include "../../include/cache_control/context.h"

//инициализация структуры cache_control
int init_context(struct cache_control *cc)
{
	if (cc == NULL) {
		//логирование, передали нулевой указатель ;(
		return -1;
	}

	cc->file_list = NULL;
	cc->shutdown_flag = 0;
	cc->daemon_flag = 0;
	cc->config_path[0] = '\0';
	cc->interval = DEFAULT_INTERVAL;

	//логирование, структура инициализирована ;)
	return 0;
}

//Очистка структуры cache_control
int remove_context(struct cache_control *cc)
{
	if (cc == NULL) {
		//логирование, передали нулевой указатель ;(
		return -1;
	}

	if (cc->file_list != NULL) {
		free_list(cc->file_list);
		cc->file_list = NULL;
	}
	//логирование, структура очищена
	return 0;
}
