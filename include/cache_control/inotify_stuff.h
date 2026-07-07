#pragma once

#ifndef INOTIFY_STUFF_H
#define INOTIFY_STUFF_H

#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "../list/list.h"
#include "file_loading.h"
#include "file_statistics.h"

int add_all_files_to_inotify(int inotify_fd, struct list *file_list);
int read_inotify(int inotify_fd, struct list *file_list);
int remove_all_files_from_inotify(int inotify_fd, struct list *file_list);
int file_in_attrib(int wd, struct list *file_list);
int file_in_modify(int wd, struct list *file_list);

#endif
