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
 
/*! \addtogroup file_monitoring
 *  Функции, связанные с мониторингом файлов
 *  @{
 */

#define MIN_PERCENT_CACHE 80 /* минимальный процент кэша для политики EVICT */

/*! Содержит набор файловых дескрипторов, необходимых для мониторинга файлов */
struct file_monitoring {
	int epoll_fd; /* файловый дескриптор epoll */
	int main_timer_fd; /* файловый дескриптор таймера */
	int inotify_fd; /* файловый дескриптор inotify */
};

/*!
 * \brief Инициализирует структуру мониторинга файлов
 * \param struct file_monitoring *fm - указатель на структуру, которую нужно инициализировать
 * \return Возвращает в случае успеха вернётся 0, в случае провала -1.
 * \details Во время инициализации выделяет файловый дескриптор для epoll, таймера и inotify.
 */
int init_file_monitoring(struct file_monitoring *fm);

/*!
 * \brief Запускает процесс мониторинга файлов, используется в соотвествующем потоке
 * \param void *args - указатель на struct cache_control*
 * \details Ждёт, пока epoll обнаружит какое-нибудь событие и вызывает check_type_fd()
 */
void *start_file_monitoring(void *args);

/*!
 * \brief Собирает статистику о файле
 * \param struct file *myfile - файл, о котором будет собираться статистика
 * \return Возвращает в случае успеха вернётся 0, в случае провала -1.
 * \details Сначала собирает информацию о последнем доступе, так как она может понадобиться
 * для подгрузка evict файла. Затем проверяет наличие виртуального адреса. Если указатель
 * на виртуальный адрес равен NULL, функция возвращается. В ином случае дополнительно
 * собирает информация о страницах.
 */
int do_file_statistics(struct file *myfile);

/*!
 * \brief Применяет соотвествующую политику к файлу
 * \param struct file *myfile - файл, по отнощении к которому будет применяться политика
 * \return Возвращает в случае успеха вернётся 0, в случае провала -1.
 * \details Применяемые политики:
 *  - KEEP - проверяет процент занятых страниц. Если их меньше 80, то подгружает.
 *  - EVICT - проверяет сколько секунд назад использовался файл. Если секунд больше значения age, выгружает файл. В случае, если выгруженный файл использовался вновь, обратно вгружает.
 *  - MONITOR - просто выводит лог.
 */
int do_file_loading(struct file *myfile);

/*!
 * \brief Выделяет всем файлам в списке свой виртуальный адрес.
 * \param struct list *list_files - список файлов
 * \return Возвращает в случае успеха вернётся 0, в случае провала -1.
 */
int map_all_files(struct list *list_files);

/*!
 * \brief Обрабатывает все файлы: собирает статистику и применяет политику.
 * \param struct list *list_files - список файлов
 * \return Возвращает в случае успеха вернётся 0, в случае провала -1.
 */
int processing_all_files(struct list *list_files);

/*!
 * \brief Проверяет тип принятого файлового дескриптора
 * \param int fd - принятый файловый дескриптор
 * \param struct file_monitoring *fm - структура мониторинга файлов
 * \param struct list *file_list - список файлов
 * \return Возвращает в случае успеха вернётся 0, в случае провала -1.
 * \details В случае если был принят файмер вызовает processing_all_files().
 * В случае если это оказался inotify вызовет read_inotify().
 */
int check_type_fd(int fd, struct file_monitoring *fm, struct list *file_list);
int reload_config(struct cache_control *cc, struct file_monitoring *fm);

/*!
 * \brief Перезагружает систему
 * \param struct cache_control *cc - структура управления cache_contorl
 * \param struct file_monitoring *fm - структура файла мониторинга
 * \return Возвращает в случае успеха вернётся 0, в случае провала -1.
 * \details Функция выполняет следующее:
 *  - Перечитывает файл конфигурации
 *  - Выделяет для файлов новый ударес в виртуальной памяти
 *  - Пересоздаёт файловые дескрипторы
 */
int reload_config(struct cache_control *cc, struct file_monitoring *fm);

/*! @} */

#endif
