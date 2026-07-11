#pragma once

#ifndef FILE_MONITORING_H
#define FILE_MONITORING_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <sys/inotify.h>

#include "context.h"
#include "../list/list.h"
#include "file_statistics.h"
#include "file_loading.h"
#include "events_stuff.h"
#include "inotify_stuff.h"

#define INTERVAL 60 // заглушка!! Заменить после реализации парсинга
#define MIN_PERCENT_CACHE 80

struct file_monitoring {
	int epoll_fd;
	int main_timer_fd;
	int inotify_fd;
};

int init_file_monitoring(struct file_monitoring *fm);
void *start_file_monitoring(void *args);
int do_file_statistics(struct file *myfile);
int do_file_loading(struct file *myfile);
int map_all_files(struct list *list_files);
int processing_all_files(struct list *list_files);
int check_type_fd(int fd, struct file_monitoring *fm, struct list *file_list);
int reload_config(struct cache_control *cc, struct file_monitoring *fm);

#endif
