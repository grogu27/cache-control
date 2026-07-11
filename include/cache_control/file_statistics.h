#pragma once

#ifndef FILE_STATISTICS_H
#define FILE_STATISTICS_H

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include "../list/list.h"
#include "../log/log.h"

/*! \addtogroup file_statistics
 *  Набор функций, отвечающих за сбор статистики о файле
 *  @{
 */

/*!
 * \brief Позволяет узнать текущий размер файла
 * \param char *path - путь до файла
 * \return Возвращает в случае успеха текущий размер файла, в случае ошибки -1
 */
off_t get_file_size(char *path);

/*!
 * \brief Позволяет узнать бощее количество страниц, выделенных под файл
 * \param off_t file_size - размер файла
 * \return Возвращает в случае успеха общее количество страниц, в случае ошибки -1
 */
int get_number_of_pages(off_t file_size);

/*!
 * \brief Позволяет узнать текущее количество занятых страниц в кэше, выделенных под файл
 * \param void *vmem_mapping - указатель на виртуальный адрес
 * \param off_t size - размер файла
 * \param int number_of_pages - общее количество страниц, выделеных под файл
 * \return Возвращает в случае успеха текущее количество занятых страниц, в случае ошибки -1
 */
int get_number_of_pages_cache(void *vmem_mapping, off_t size,
			      int number_of_pages);

/*!
 * \brief Позволяет узнать процентное соотношение занятых страниц по отношению к общему количеству
 * \param int number_of_pages - общее количество страниц
 * \param int number_of_pages_cache - количество занятых страниц
 * \return Возвращает в случае успеха процент занятых страниц по отношению в общему количетсву
 */
int get_percent_cache(int number_of_pages, int number_of_pages_cache);

/*!
 * \brief Позволяет узнать время последнего доступа к файлу
 * \param char *path - путь до файла
 * \param struct timespec *last_access - указатель на структуру времени, куда будет записан результат 
 * \return Возвращает в случае успеха 0, в случае ошибки -1
 */
int get_last_access(char *path, struct timespec *last_access);

/*!
 * \brief Позволяет узнать общую информацию о страницах в кэше
 * \param struct file *myfile - указатель на структуру файла, куда будет записан результат
 * \return Возвращает в случае успеха 0, в случае ошибки -1
 * \details Получает общее количество страниц, количество занятых страниц и их процентное соотношение
 */
int get_page_stat(struct file *myfile);

/*! @} */

#endif
