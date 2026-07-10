#pragma once

#ifndef EVENTS_STUFF_H
#define EVENTS_STUFF_H

#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdio.h>
#include "../log/log.h"

#define TIMEOUT 1000
#define MAX_NUM_OF_EVENTS 2

int set_timer(int timer_fd, time_t seconds);
int add_fd_to_epoll(int epoll_fd, int fd);

#endif
