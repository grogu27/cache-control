#ifndef QUEUE_H
#define QUEUE_H
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mqueue.h>
#include <time.h>
#include <errno.h>
/*!
 * \addtogroup queue
 * Набор функций для взаимодействия с очередями сообщений между command listener и mediator
 * @{
 */
#define SIZE_RCV 1024/*!< размер сообщения для передачи*/
#define SIZE_SND 45/*!< размер сообщения для приема*/
#define MAX_SIZE_MSG 8192 /*!< максимальновозможный размер сообщения для очередей*/
#define CHANGE_SIZE_MSG 8190 /*!< размер сообщения для очередей если был превышено максимальное значение*/
#define MQ_TIMEOUT 5 /*!< Таймаут ожидания сообщения из очереди*/
/*!
 * \brief Функция создания.присоеденения к очереди сообщений
 * \param char *file_desc - уникальное имя идентификатор для очереди
 * \param int flags - принимаемые флаги определяющие права для очереди O_RDONLY/O_WRONLY и флаг для создания новой O_CREAT
 * \param int size_msg - размер сообщения которые будут передаваться в очереди
 * \return возвращает идентификатор очереди
 */
mqd_t create_queue(char *file_desc, int flags, int size_msg);
/*!
 * \brief Функция приема сообщений из очереди
 * \param mqd_t queue - идентификатор очереди откуда будут приниматься сообщений
 * \param char *snd_msg - буфер куда бедет записываться сообщение из очереди
 * \param int size_msg - размер сообщения которое будет вычитываться из очереди
 * \return возвращает буфер с полученным сообщением. В случае ошибки возвращает NULL
 */
char *recive_msg(mqd_t queue, char *snd_msg, int size_msg);
/*!
 * \brief Функция отправки сообщения в очередь
 * \param mqd_t queue - идентификатор очереди откуда будут приниматься сообщений
 * \param char *snd_msg - сообщение которое будет переданно в очередь
 * \param int size_msg - размер сообщения которое будет передаваться в очередь
 */
int send_msg(mqd_t queue, char *snd_msg, int size_msg);
/*!
 * \brief Функция отправки сообщения в очередь для mediator
 * \param mqd_t queue - идентификатор очереди откуда будут приниматься сообщений
 * \param char *snd_msg - сообщение которое будет переданно в очередь
 * \param int size_msg - размер сообщения которое будет передаваться в очередь
 * \warning отличается от функции recive_msg тем что в ней нет таймера для прерывания ожидания сообщения
 */
char *recive_msg_for_mediator(mqd_t queue_fd, char *rcv_msg, int size_msg);
/*! @}*/
#endif
