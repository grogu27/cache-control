#pragma once

#ifndef THREAD_H
#define THREAD_H

#include "context.h"
#include <pthread.h>
#include "command_listener.h"
#include "file_monitoring.h"

/*! \addtogroup threads
 *  Функции для управления рабочими потоками программы
 *  @{
 */

/*!
 * \brief Запускает рабочие потоки программы
 * \param struct cache_control *cc - указатель на контекст программы
 * \return Возвращает 0 в случае успеха или -1 в случае ошибки
 * \details Создаёт два потока:
 *  - поток мониторинга файлов (file_monitoring_thread)
 *  - поток обработки команд из FIFO (command_listener_thread)
 */
int start_threads(struct cache_control *cc);

/*!
 * \brief Останавливает все рабочие потоки программы
 * \return Возвращает 0 в случае успеха или -1 в случае ошибки
 * \details Оожидает завершения потоков мониторинга и обработчика команд через pthread_join().
 */
int stop_threads();

/*! @} */

#endif
