#pragma once

#ifndef FILE_LOADING_H
#define FILE_LOADING_H

#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include "../list/list.h"
#include "../log/log.h"

#define READ_BUFF 4096

int map_file(struct file *myfile);
int load_file(struct file *myfile);
int unmap_file(struct file *myfile);

#endif
