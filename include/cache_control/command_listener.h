#ifndef CACHECONTROL_LISTNER_H
#define CACHECONTROL_LISTNER_H

#include "file_loading.h"
#include "context.h"
#include "../queue/queue.h"
#include "../log/log.h"
#include "file_statistics.h"
#include "file_monitoring.h"
#include "../list/list.h"

/*!\addtogroup
 * Набор функция для обработки команд от mediator
 * @{
 */
#define COMMAND_LEN 10 /*!< Длинна команды которая принимается от mediator*/
#define PATH_FILE 255 /*!< Длинна пути до файла который указан в файле конфигурации*/
/*!
 * Структура в которую будут записыватся информация о файле для передачи mediator
 */
struct file_info {
	char path[PATH_BUFF_SIZE]; /*!< путь до файла который указан в конфигурациооном файле*/

	off_t size; /*!< размер файла*/
	int number_of_pages; /*!< общее количество страниц файлакоторые можно подгрузить в памть*/
	int number_of_pages_cache; /*!< количество страниц файла которые уже загружены в память*/
	int percent_cache; /*!< процент страниц загруженных страниц файла в память*/
	struct timespec last_access; /*!< время последнего обращения к файлу*/

	int politic; /*!< политика для маниторинга файла в памяти*/
	int age; /*!< время через которое файл будет выгружен из памяти*/
};

/*!
 * структура для передачи mediator
 */
struct replay_t {
	int status; /*!< информационное поле для обозначения принята команда в обработку или получена ошибка*/
	int last_msg; /*!< обозначение последнего сообщения в передаче сообщений для mediator*/
	struct file_info f; /*!< структура с информацией о файле*/
};
/* структура для приема сообщения от mediator с командой*/
struct mediator_command {
	char com[COMMAND_LEN]; /*!< команда от mediator*/
	char path_to_file
		[PATH_FILE]; /*!< аргумент для команды. Может быть пустым для некоторых команд*/
};
/*!
 * \brief Функция отправки статуса по всем файлам
 * \details Функция передает информацию по всем файлам из файла конфигурации mediator один файл в отдельном сообщении
 * \param struct node* mynode - структура списка где перечисленны все файлы для мониторинга
 * \param struct replay_t snd_msg структура которая будет заполняться для передачи mediator
 * \param mqd_t snd_queue - идентификатор очереди для отправки сообщений mediator
 * \param struct cache_control *cc - структура содержащая все параметры
 */
int status_all(struct node *mynode, struct replay_t snd_msg, mqd_t snd_queue,
	       struct cache_control *cc);
/*!
 * \brief Функция отправки статуса по определенному файлу
 * \param struct file* myfile - структура списка, где перечислена информация о файле
 * \param struct replay_t snd_msg структура которая будет заполняться для передачи mediator
 * \param mqd_t snd_queue - идентификатор очереди для отправки сообщений mediator
 * \param struct cache_control *cc - структура содержащая все параметры
 */
int status_spec(struct file *myfile, struct replay_t snd_msg, mqd_t snd_queue,
		struct cache_control *cc);
/*!
 * \brief Функция загрузки страниц файла впамять
 * \param struct file* myfile - структура списка где перечислена информация о файле
 * \param struct replay_t snd_msg структура которая будет заполняться для передачи mediator
 * \param mqd_t snd_queue - идентификатор очереди для отправки сообщений mediator
 */
int do_cache(struct file *myfile, struct replay_t snd_msg, mqd_t snd_queue);
/*!
 * \brief Функция выгрузки файла из памяти
 * \details Выгружает файл из памяти, но не удаляет его из файла конфигурации и структуры, так как к нему может повторно обратится другая программа
 * \param struct file* myfile - структура списка, где перечислена информация о файле
 * \param struct replay_t snd_msg структура которая будет заполняться для передачи mediator
 * \param mqd_t snd_queue - идентификатор очереди для отправки сообщений mediator
 * \param struct cache_control *cc - структура содержащая все параметры
 */
int do_evict(struct file *myfile, struct replay_t snd_msg, mqd_t snd_queue,
	     struct cache_control *cc);
/*!
 * \brief Функция перечитывания конфигурационного файла
 * \param struct cache_control *cc - структура содержащая все параметры
 * \param struct replay_t snd_msg структура которая будет заполняться для передачи mediator
 * \param mqd_t snd_queue - идентификатор очереди для отправки сообщений mediator
 */
int do_reload(struct cache_control *cc, struct replay_t snd_msg,
	      mqd_t snd_queue);
/*!
 * \brief Функция корректного завершения потока обработчика команд
 * \param struct cache_control *cc - структура содержащая все параметры
 * \param struct replay_t snd_msg структура которая будет заполняться для передачи mediator
 * \param mqd_t rcv_queue - идентификатор очереди передачи сообщений для закрытия очереди
 * \param mqd_t snd_queue - идентификатор очереди приема сообщений для закрытия очереди
 */
int do_exit(struct cache_control *cc, struct replay_t snd_msg, mqd_t rcv_queue,
	    mqd_t snd_queue);
/*!
 * \brief Функция для запуска в потоке для обработки команд от mediator
 * \param void* args принимает в качестве аргумента структуру cache control содержащую всю информацию о файлах которые нужно мониторить
 */
int command_listner(void *args);
/*! @} */
#endif
