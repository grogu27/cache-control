#ifndef PARSE_CONFIG_H
#define PARSE_CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fnmatch.h>
#include <dirent.h>
#include <limits.h>
#include "../list/list.h"

#define DEFAULT_INTERVAL 60
#define SIZE_BUFF 4096

int parse_config(const char *config_path, struct list *l, int *interval);

#endif