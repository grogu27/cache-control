#pragma once

#ifndef CONTEXT_H
#define CONTEXT_H

#include "../list/list.h"
#include "parse_config.h"

#define PATH_SIZE 255

struct cache_control {
	int daemon_flag;
	char config_path[PATH_SIZE];
	struct list *file_list;
	int shutdown_flag;
	int interval;
};

int init_context(struct cache_control *cc);
int remove_context(struct cache_control *cc);

#endif
