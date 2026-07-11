#pragma once

#ifndef EVENTS_STUFF_H
#define EVENTS_STUFF_H

#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdio.h>
#include "../log/log.h"

/*! \addtogroup event_stuff
 *  event_stuff - функции, связанные с событиями
 *  @{
 */

#define TIMEOUT 1000  /* время, через которое epoll перестанет ждать сигнал */

#define MAX_NUM_OF_EVENTS 2 /* количество событий, которое может произойти одновремено */

/*!
 * \brief Задёт время для таймера и запускает его.
 * \param int timer_fd - файловый дескриптор таймера
 * \param time_t seconds - колличество секунд
 * \return Возвращает в случае успеха вернётся 0, в случае провала -1.
 */
int set_timer(int timer_fd, time_t seconds);

/*!
 * \brief Добавляет файловый дескриптор в epoll список.
 * \param int epoll_fd - фаловый дескриптор epoll
 * \param int fd - файловый дескриптор, который будет добавлен
 * \return Возвращает в случае успеха 0, в случае провала -1.
 */
int add_fd_to_epoll(int epoll_fd, int fd);

/*! @} */

#endif
