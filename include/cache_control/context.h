#pragma once

#ifndef CONTEXT_H
#define CONTEXT_H

#include <pthread.h>
#include "../list/list.h"
#include "parse_config.h"
#include "../log/log.h"

/*! \addtogroup contex
 *  Функции, которые отвечаю за инициализацию и удаление контекста программы
 *  @{
 */

#define PATH_SIZE 255 /* Максимальный размер пути к конфигурационному файлу */

/*! Содержит главную информацию о состоянии программы cache_control*/
struct cache_control {
	int daemon_flag; /* флаг запуска в режиме демона */
	char config_path[PATH_SIZE]; /* путь к конфигурационному файлу */
	struct list *file_list; /* список отслеживаемых файлов */
	int shutdown_flag; /* флаг завершения работы программы */
	int interval; /* интервал мониторинга в секундах */
	int reload_flag; /* флаг перезагрузки конфигурации */
	int stats_flag; /* флаг запроса статистики */
	pthread_mutex_t mutex; /* мьютекс для синхронизации доступа к контексту*/
};

/*!
 * \brief Инициализирует структуру контекста программы (cache_control)
 * \param struct cache_control *cc - указатель на структуру
 * \return Возвращает 0 в случае успеха или -1 в случае ошибки
 * \details Устанавливает значения полей по умолчанию, инициализирует мьютекс
 */
int init_context(struct cache_control *cc);

/*!
 * \brief Освобождает ресурсы, связанные со структурой cache_control
 * \param struct cache_control *cc - указатель на структуру
 * \return Возвращает 0 в случае успеха или -1 в случае ошибки
 * \details Освобождает список файлов, если он был выделен
 */
int remove_context(struct cache_control *cc);

/*! @} */

#endif
