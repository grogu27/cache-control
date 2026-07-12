#include "../../include/cache_control/context.h"

int init_context(struct cache_control *cc)
{
	if (cc == NULL) {
		logging(LOG_ERR,
			"<init_context> struct cache_control *cc is NULL");
		return -1;
	}

	cc->file_list = NULL;
	cc->shutdown_flag = 0;
	cc->reload_flag = 0;
	cc->stats_flag = 0;
	cc->daemon_flag = 0;
	cc->config_path[0] = '\0';
	cc->interval = DEFAULT_INTERVAL;
	pthread_mutex_init(&(cc->mutex), NULL);

	logging(LOG_INFO, "struct cache_control* cc was initialized");
	return 0;
}

int remove_context(struct cache_control *cc)
{
	if (cc == NULL) {
		logging(LOG_ERR,
			"<remove_context> struct cache_control *cc is NULL");
		return -1;
	}

	if (cc->file_list != NULL) {
		free_list(cc->file_list);
		cc->file_list = NULL;
	}
	logging(LOG_INFO, "struct cache_control* cc was removed");
	return 0;
}
