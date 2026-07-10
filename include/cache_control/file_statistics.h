#pragma once

#ifndef FILE_STATISTICS_H
#define FILE_STATISTICS_H

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include "../list/list.h"
#include "../log/log.h"

off_t get_file_size(char *path);
int get_number_of_pages(off_t file_size);
int get_number_of_pages_cache(void *vmem_mapping, off_t size,
			      int number_of_pages);
int get_percent_cache(int number_of_pages, int number_of_pages_cache);
int get_last_access(char *path, struct timespec *last_access);
int get_page_stat(struct file *myfile);

#endif
