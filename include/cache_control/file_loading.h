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

/*! \addtogroup file_loading
 *  Функции, связанные с загрузкой и выгрузкой файла в оперативной памяти
 *  @{
 */

/*!
 * \brief Выделяет виртуальный адрес для файла
 * \param struct file *myfile - файл, для которого будет выделяться память
 * \return Возвращает в случае успеха вернётся 0, в случае провала -1.
 */
int map_file(struct file *myfile);

/*!
 * \brief Загружает файл из виртуальной памяти в оперативную
 * \param struct file *myfile - файл, который будет загружен в память
 * \return Возвращает в случае успеха вернётся 0, в случае провала -1.
 */
int load_file(struct file *myfile);

/*!
 * \brief Выгружает файл из виртуальной памяти, тем самым высвобождая его из опреативной памяти
 * \param struct file *myfile - файл, который будет загружен в память
 * \return Возвращает в случае успеха вернётся 0, в случае провала -1.
 */
int unmap_file(struct file *myfile);

/*! @} */
#endif
