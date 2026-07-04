#pragma once
#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "context.h"

void print_info();
int parse_options(int argc, char **argv, struct cache_control *cc);
int validate_path_config(const char *path);

#endif
