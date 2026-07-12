#ifndef LOG_H
#define LOG_H
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
/*! addtogroup logging
 * Набор функций для логирования
 * @{
 */

#define LOG_BUFF 512 /*размер буфера для лог сообщений */
/*!
 * \brief функция открытия syslog файла
 */
void open_log();
/*
 * \brief Функция записи сообщения в syslog
 * \details функция записывает сообщение в syslog  с определенным уровнем логирования
 * \param int log_level - указывает уровани логирования для сообщения используются только LOG_INFO, LOG_WARNING, LOG_ERR
 * \param char *msg - сообщение которое будет записано в syslog
 * \warning при использовании LOG_ERR сообщение будет дополненно информацией из errno
 */
void logging(int log_level, char *msg);
/*!
 * \brief Функция закрытия syslog файла
 */
void close_log();
/*! @}*/
#endif
