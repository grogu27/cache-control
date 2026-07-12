#pragma once

#ifndef CONTEXT_H
#define CONTEXT_H

#include <pthread.h>
#include "../list/list.h"
#include "parse_config.h"
#include "../log/log.h"

#define PATH_SIZE 255

struct cache_control {
	int daemon_flag;
	char config_path[PATH_SIZE];
	struct list *file_list;
	int shutdown_flag;
	int interval;
	int reload_flag;
	int stats_flag;
	pthread_mutex_t mutex;
};

int init_context(struct cache_control *cc);
int remove_context(struct cache_control *cc);

#endif
