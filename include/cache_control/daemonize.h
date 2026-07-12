#pragma once

#ifndef DAEMONIZE_H
#define DAEMONIZE_H

#include <stdio.h>
#include <unistd.h>
#include "../log/log.h"

/*! \addtogroup daemonize
 *  Функция для демонизации процесса
 *  @{
 */

/*!
 * \brief Запускает программу в режиме демона
 * \return Возвращает 0 в случае успеха, -1 в случае ошибки
 * \details Функция использует системный вызов daemon(0,0), который отсоединяет процесс от терминала,
 * меняет текущую рабочую директорию на корневую и запускает работу в фоновом режиме
 */
int daemonize();

/*! @} */

#endif
