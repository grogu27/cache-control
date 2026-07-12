/*! \file parse_config.h
 *  \brief Заголовочный файл парсера конфигурации кэширования
 *  \ingroup parse_config
 *  \version 1.0
 */


/*! \addtogroup parse_config
 *  \brief Модуль парсинга конфигурационного файла кэширования
 *  \details Предоставляет функции для разбора конфигурационного файла,
 *  содержащего правила управления кэшем для файлов и каталогов.
 *  Поддерживает три политики: keep, evict, monitor.
 *  @{
 */

#ifndef PARSE_CONFIG_H
#define PARSE_CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fnmatch.h>
#include <dirent.h>
#include <limits.h>
#include "../list/list.h"

#define DEFAULT_INTERVAL 60 /*!< Значение интервала обновления по умолчанию (в секундах) */
#define SIZE_BUFF 4096      /*!< Размер буфера для чтения строк конфигурационного файла */


/*!
 * \brief Разбирает конфигурационный файл и заполняет список файлов
 * \details Читает конфигурационный файл построчно, парсит правила для файлов
 * и каталогов. Поддерживаются три политики: keep (постоянно держать в кэше),
 * evict (выгружать через age секунд), monitor (только статистика).
 * 
 * Формат строки конфигурации:
 *   <путь_с_маской> <политика> [age=<секунды>]
 * 
 * Примеры:
 * \verbatim
   /var/lib/mysql/* keep
   /var/log/*.log evict age=3600
   /tmp/* evict age=300
   /opt/data/*.dat monitor
   \endverbatim
 * 
 * \param[in] config_path Путь к конфигурационному файлу
 * \param[out] l Список, в который добавляются структуры file
 * \param[out] interval Значение интервала обновления (в секундах)
 * \return 
 * - 0 - успех
 * - 1 - ошибка открытия файла
 * - 2 - синтаксическая ошибка
 * - 3 - семантическая ошибка (разные политики для одного файла)
 */
int parse_config(const char *config_path, struct list *l, int *interval);

#endif

/*! @}*/