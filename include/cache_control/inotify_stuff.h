#pragma once

#ifndef INOTIFY_STUFF_H
#define INOTIFY_STUFF_H

#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "../list/list.h"
#include "file_loading.h"
#include "file_statistics.h"

/*! \addtogroup inotify_stuff
 *  Набор функций для работы с inotify
 *  @{
 */

/*!
 * \brief Добавляет все файлы в inotify список, чтобы тот мог их отслеживать
 * \param int inotify_fd - файловый дескриптор inotify
 * \param struct list *file_list - список файлов
 * \return Возвращает в случае успеха 0, в случае ошибки -1
 */
int add_all_files_to_inotify(int inotify_fd, struct list *file_list);

/*!
 * \brief Обрабатывает событие принятое inotify
 * \param int inotify_fd - файловый дескриптор inotify
 * \param struct file *file_list - список файлов
 * \details Основные события:
 *  - IN_ATTRIB - у файла изменились линки. Вызывается file_in_attrib().
 *  - IN_MOVE_SELF - файл был перемещён. Файл удаляется из списка, так как больше у программы нету способов отслеживать файл.
 *  - IN_MODIFY - файл был изменён. Вызывается file_in_modify().
 */
int read_inotify(int inotify_fd, struct list *file_list);

/*!
 * \brief Убирает все файлы из inotify списка
 * \param int inotify_fd - файловый дескриптор inotify
 * \param struct list *file_list - список файлов
 * \return  Возвращает в случае успеха 0, в случае ошибки -1
 */
int remove_all_files_from_inotify(int inotify_fd, struct list *file_list);

/*!
 * \brief Применяет действия в соотвествии с событием IN_ATTRIB
 * \param int wd - дескриптор просмотра (watch descriptor)
 * \param struct list *file_list - список файлов
 * \return Возвращает случае успеха 0, в случае ошибки -1
 * \details Если у файла есть виртуальный адрес, пытается получить его размер. Затем если get_file_size()
 * вернёт ошибку, то это значит, что файла больше не существует и он удаляется из списка. В ином случае файлу
 * выделяется новый виртуальный адрес в соотвествии с новым размером.
 */
int file_in_attrib(int wd, struct list *file_list);

/*!
 * \brief Применяет действия в соотвествии с событием IN_MODIFY
 * \param int wd - дескриптор просмотра (watch descriptor)
 * \param struct list* *file_list - список файлов
 * \return Возвращает в случае успеха 0, в случае ошибки -1
 * \details Если у файла есть виртуальный адрес удаляет его и выделяет новый в соотвествии новым размером.
 */
int file_in_modify(int wd, struct list *file_list);

/*! @} */
#endif
