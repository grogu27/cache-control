#include "../../include/cache_control/context.h"

//инициализация структуры cache_control
int init_context(struct cache_control *cc)
{
	if (cc == NULL) {
		//логирование, передали нулевой указатель ;(
		logging(LOG_ERR,
			"<init_context> struct cache_control *cc is NULL");
		return -1;
	}

	cc->file_list = NULL;
	cc->shutdown_flag = 0;
	cc->daemon_flag = 0;
	cc->config_path[0] = '\0';
	cc->interval = DEFAULT_INTERVAL;

	//логирование, структура инициализирована ;)
	logging(LOG_INFO, "struct cache_control* cc was initialized");
	return 0;
}

//Очистка структуры cache_control
int remove_context(struct cache_control *cc)
{
	if (cc == NULL) {
		//логирование, передали нулевой указатель ;(
		logging(LOG_ERR,
			"<remove_context> struct cache_control *cc is NULL");
		return -1;
	}

	if (cc->file_list != NULL) {
		free_list(cc->file_list);
		cc->file_list = NULL;
	}
	//логирование, структура очищена
	logging(LOG_INFO, "struct cache_control* cc was removed");
	return 0;
}
