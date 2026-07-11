#pragma once

#ifndef LIST_H
#define LIST_H

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include "../log/log.h"

/*! \addtogroup list
 *  Набор функций для списка файлов
 *  @{
 */

#define PATH_BUFF_SIZE 255 /* размер буфера пути файла */

/*! Основные политики файла:
 *   - KEEP - постоянно держит файл в кэше
 *   - EVICT - выгружает файл, если им не пользовались age секунд
 *   - MONITOR - собирает статистику
 */
enum politics { KEEP = 1, EVICT, MONITOR };

/*!
 * Хранит основную информацию о статистики файла
 */
struct file_stat {
	off_t size; /* рамер файла */
	int number_of_pages; /* общее количество страниц */
	int number_of_pages_cache; /* количество занятых страниц */
	int percent_cache; /* процентное соотщение занятых страниц к общему количеству */
	struct timespec last_access; /* время последнего доступа к файлу */
};

/*!
 * Хранит основную информацию о файле
 */
struct file {
	char path[PATH_BUFF_SIZE]; /* путь файла */ 
	struct file_stat *stat; /* статистика файла */
	int politic; /* политика */
	int age; /* время через, которое файл будет выгружен при политике EVICT */
	int wd; /* дескриптор просмотра (watch descriptor) */
	int fd; /* файловый дескриптор */
	void *vmem_mapping; /* указтель на виртуальную память*/
};

/*!
 * Хранит основную информацию о вершине списка
 */
struct node {
	struct file *nd_file; /* указатель на файл */
	struct node *nd_next; /* указатель на следующую вершину в списке */
};

/*!
 * Хранит основную информацию о списке
 */
struct list {
	struct node *start; /* указатель на первую вершину в списке*/
	int length; /* размер списка */
	int evict_files; /* количество evict файлов*/
};

/*!
 * \brief Функция инициализации структуры статистики файла
 * \param off_t s - размер файла
 * \param int np - общее количество страниц
 * \param int npc - количество занятых страниц
 * \param int pc - процентное соотщение занятых страниц к общему количеству
 * \param struct timespec *la - время последнего доступа к файлу
 * \retrun Возвращает указатель на структуру файла
 * \warning Память для структуры статистики выделяется динамически, поэтому после использования
 * рекомендуется вызвать функцию free()
 */
struct file_stat *init_file_stat(off_t s, int np, int npc, int pc,
				 struct timespec *la);

/*!
 * \brief Функция инициализации пустой структуры статистики файла
 * \retrun Возвращает казатель на структуру файла
 * \warning Память для статистики выделяется денамически, поэтому после использования
 * рекомендуется вызвать функцию free()

 */
struct file_stat *init_empty_stat();

/*!
 * \brief Функция инициализации структуры файла
 * \param char *pb - путь файла
 * \param int p - политика
 * \param int a - время через, которое файл будет выгружен при политике EVICT
 * \return Возвращает указатель на структуру файла
 * \warning Память для структуры файла выделяется денамически, поэтому после использования
 * рекомендуется вызвать функцию free()
 */
struct file *init_file(char *pb, int p, int a);

/*!
 * \brief Добавляет стутс к списку
 * \param struct file *f - структура файла
 * \param struct file_stat *fs - структура статистики файла
 * \return В случае успеха 0, в случае ошибки -1
 */
int add_file_stat(struct file *f, struct file_stat *fs);

/*!
 * \brief Функция инициализации вершины
 * \param struct file *f - файл, который будет добавлен в вершину
 * \warning Память для вершины выделяется денамически, поэтому после использования
 * рекомендуется вызвать функцию free()
 */
struct node *init_node(struct file *f);

/*!
 * \brief Функция инициализации списка
 * \return Возвращает указатель на список
 * \warning Память для списка выделяется денамически, поэтому после использования
 * рекомендуется вызвать функцию free_list();
 */
struct list *init_list();

/*!
 * \brief Добавляет файл в список
 * \param struct list *l - список, куда добавится файл
 * \param struct file *f - файл, который добавится в список
 * \return В случае успеха 0, в случае ошибки -1
 */
int add_file(struct list *l, struct file *f);

/*!
 * \brief Находит файл по его пути
 * \param struct list *l - список, где будет производится поиск
 * \param char *path - путь до файла
 * \return Возвращает в случае успеха указатель на файл, в случае ошибки -1
 */
struct file *find_file_by_path(struct list *l, char *path);

/*!
 * \brief Находит файл в списке по его дескриптору просмотра (watch descriptor)
 * \param struct list *l - список, где будет производится поиск
 * \param int wd - дескриптор просмотра
 * \return Возвращает в случае успеха указатель на файл, в случае ошибки -1
 */
struct file *find_file_by_wd(struct list *l, int wd);

/*!
 * \brief Удаляет файл из списка по его пути
 * \param struct list *l - список, откуда будет удалён файл
 * \param char *path - путь до файла
 * \return В случае успеха 0, в случае ошибки -1
 */
int delete_file_by_path(struct list *l, char *path);

/*!
 * \brief Удаляет файл из списка по его дескриптору просмотра (watch descriptor)
 * \param struct list *l - список, откуда будет удалён файл
 * \param int wd - дескриптор просмотра
 * \return В случае успеха 0, в случае ошибки -1

 */
int delete_file_by_wd(struct list *l, int wd);

/*!
 * \brief Высвобождает память, выделенную под список
 * \param struct list *l - список, который будет удалён
 * \return В случае успеха 0, в случае ошибки -1
 */
int free_list(struct list *l);

/*! @} */

#endif
