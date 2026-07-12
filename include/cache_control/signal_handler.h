#pragma once

#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include "context.h"
#include "../log/log.h"

/*! \addtogroup signal_handler
 *  Функции для обработки системных сигналов
 *  @{
 */

/*!
 * \brief Обработчик сигнала SIGINT
 * \param int sig - номер принятого сигнала
 * \details Устанавливает флаг shutdown_flag в контексте программы(cache_control)
 */
void sigint_handler(int sig);

/*!
 * \brief Обработчик сигнала SIGTERM
 * \param int sig - номер принятого сигнала
 * \details Устанавливает флаг shutdown_flag в контексте программы (cache_control)
 */
void sigterm_handler(int sig);

/*!
 * \brief Обработчик сигнала SIGHUP
 * \param int sig - номер принятого сигнала
 * \details Устанавливает флаг reload_flag в контексте программы (cache_control)
 */
void sighup_handler(int sig);

/*!
 * \brief Обработчик сигнала SIGUSR1
 * \param int sig - номер принятого сигнала
 * \details Устанавливает флаг stats_flag в контексте программы (cache_control)
 */
void sigusr1_handler(int sig);

/*!
 * \brief Регистрирует обработчик для указанного сигнала
 * \param int signum - номер сигнала (например, SIGINT, SIGTERM)
 * \param void (*handler)(int) - указатель на функцию-обработчик
 * \return Возвращает 0 в случае успеха или -1 в случае ошибки
 * \details Использует sigaction() для установки обработчика сигнала
 */
int register_handler(int signum, void (*handler)(int));

/*!
 * \brief Инициализирует все обработчики сигналов программы
 * \details Регистрирует обработчики для сигналов:
 *  - SIGINT и SIGTERM — завершение работы (shutdown_flag)
 *  - SIGHUP — перезагрузка конфигурации (reload_flag)
 *  - SIGUSR1 — запрос статистики (stats_flag)
 */
void init_signal_handler();

/*!
 * \brief Устанавливает контекст программы (cache_control) для обработчиков сигналов
 * \param struct cache_control *cc - указатель на контекст программы
 * \details Сохраняет указатель на контекст, чтобы обработчики сигналов
 * могли изменять соответствующие флаги
 */
void set_context(struct cache_control *cc);

/*! @} */

#endif
