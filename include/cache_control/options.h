#pragma once

#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#define PATH_SIZE 255

struct program_options {
	int daemon_flag;
	char config_path[PATH_SIZE];
};

void print_info();
int parse_options(int argc, char **argv, struct program_options *po);
int validate_path_config(const char *path);

#endif
