/*! \file cache_control_mediator.h
 *  \brief Заголовочный файл модуля управления кэшем через очередь сообщений
 *  \ingroup cache_control_mediator
 *  \version 1.0
 */

/*!  \addtogroup  cache control mediator
*	 \brief Модуль управления кэшем через очередь сообщений
*    \details Предоставляет интерфейс для взаимодействия с демоном кэширования
*    через очереди сообщений POSIX. Позволяет отправлять команды управления
*    кэшем и получать статистику по файлам.
* @{
*/

#ifndef CACHE_CONTROL_MEDIATOR_H
#define CACHE_CONTROL_MEDIATOR_H

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "../queue/queue.h"
#include <sys/stat.h>
#include "../list/list.h"


#define SEND_FILE "/snd_queue"  						  /*!< Путь к очереди для отправки сообщений демону */
#define RECV_FILE "/rcv_queue" 							  /*!< Путь к очереди для приёма сообщений от демона */
#define CLEAR_TERMINAL "\033[2J\033[H" 					  /*!< Управляющая последовательность для очистки терминала */
#define RED "\033[31m" 									  /*!< Цвет текста (красный) для вывода ошибок */
#define DASHES "---------------------------\n" 
#define N 255 											  /*!< Максимальный размер буфера для ввода команд */
#define MAX_TOK 2 										  /*!< Максимальное количество токенов при разборе командной строки */
#define PATH_LEN 255 									  /*!< Максимальная длина пути к файлу */
#define COMMAND_LEN 10 									  /*!< Максимальная длина имени команды */
#define PATH_BUFF_SIZE 255 								  /*!< Размер буфера для хранения пути */
#define PATH_LOG_FILE "~/logs/cache_control_mediator.log" /*!< Путь к файлу для логирования */
#define POLITIC_LEN 15 									  /*!< Максимальная длина названия политики кэширования */

/*!
 * \brief Структура, представляющая команду управления кэшем
 * \details Содержит имя команды и путь к файлу (если требуется)
 */
typedef struct {
	char command[COMMAND_LEN];   /*!< Имя команды (status, cache, evict и т.д.) */
	char path_to_file[PATH_LEN]; /*!< Путь к файлу (для команд, требующих аргумент) */
} command_t;

/*! \brief Структура, которая содержит статистику о файле*/
struct file_info {
	char path[PATH_BUFF_SIZE];    /*!< Путь к файлу*/
	off_t size; 		          /*! Рамер файла в байтах*/				 
	int number_of_pages;     	  /*! Общее количество страниц */  
	int number_of_pages_cache; 	  /*! Количество занятых страниц */ 
	int percent_cache; 			  /*! Процентное соотношение занятых страниц к общему количеству */ 		 
	struct timespec last_access;  /*! Время последнего доступа к файлу */ 
	int politic; 				  /* Политика кэширования */	 
	int age;     				  /*! Время через, которое файл будет выгружен при политике EVICT */ 
};

/*! \brief Структура, которая содержит ответ от демона на запрос медиатора
 *  \details Содержит статус выполнения и статистику по файлу
*/
typedef struct {
	int status; 		/* Статус выполнения: 0 - успешно, 1 - файл не найден */
	int last_msg; 		/* Флаг последнего сообщения: 0 - последнее, 1 - есть ещё данные */
	struct file_info f; /* Статистика по файлу (заполняется при status) */
} reply_t;


/*!
 * \brief Политики кэширования файлов
 * \details Определяют поведение демона при управлении кэшем файлов
 */
enum pol { 
	P_KEEP = 1,   /*!< Постоянно держать файл в кэше */
	P_EVICT,  	  /*!< Выгружать файл, если им не пользовались age секунд */
	P_MONITOR 	  /*!< Только собирать статистику, не влияя на кэш */	
};

/*!
 * \brief Структура для хранения дескрипторов очередей сообщений
 * \details Содержит два дескриптора: для отправки и приёма сообщений
 */
typedef struct {
	mqd_t fds[2]; /*!< Массив дескрипторов: [0] - приём, [1] - отправка */
} mqfd_t;

/*!
 * \brief Команды, поддерживаемые медиатором
 * \details Используется для идентификации команды при обработке
 */
typedef enum {
	CMD_EXIT, 		/*! Команда завершения работы демона */
	CMD_RELOAD,     /*! Команда перезагрузки конфигурации */
	CMD_EVICT, 		/*! Команда принудительной выгрузки файла */ 
	CMD_CACHE, 		/*! Команда принудительной загрузки файла в кэш */ 	
	CMD_STATUS,		/*! Команда получения статистики по файлу */	
	CMD_CLEAR, 		/*! Команда очистки терминала */ 
	CMD_UNKNOWN 	/*! Неизвестная команда */
} command_type_t;

/*!
 * \brief Считывает строку из стандартного ввода
 * \param[out] str - Буфер для хранения считанной строки
 * \return 0 - успех, 1 - ошибка ввода, 2 - строка слишком длинная
 */
int read_string(char *str);

/*!
 * \brief Очищает буфер стандартного ввода
 * \details Считывает и отбрасывает символы до конца строки или EOF
 */
void clear_stdin();

/*!
 * \brief Выводит меню доступных команд
 * \details Отображает список всех поддерживаемых команд и их назначение
 */
void print_menu();

/*!
 * \brief Разбирает введённую строку на команду и её аргументы
 * \param[in] str Входная строка для разбора
 * \param[out] command Структура для заполнения командой и путём
 * \param[out] op Тип команды 
 * \return 0 - успех, 1 - ошибка синтаксиса, 2 - команда clear
 */
int get_command(char *str, command_t *command, command_type_t *op);

/*!
 * \brief Выводит ответ от демона в удобочитаемом формате
 * \param[in] answer Буфер с ответом от демона
 * \param[in] ptr_command Указатель на отправленную команду
 * \param[in] op Тип выполненной команды
 * \return 0 - успех, 1 - ошибка открытия файла, 2 - файл не найден, 3 - ошибка определения политики
 */
int print_answer(const char *const answer, const command_t *const ptr_command,
		 const command_type_t op);

/*!
 * \brief Разбивает строку на токены по разделителю
 * \param[in] str Входная строка для разбиения
 * \param[out] tokens Массив указателей на токены
 * \param[in] delim Строка-разделитель
 * \param[out] count_tok Количество полученных токенов
 * \return 0 - успех, 1 - превышено максимальное количество токенов
 */
int split_tokens(char *str, char *tokens[], const char *delim,
		 size_t *count_tok);

/*!
 * \brief Открывает очереди сообщений для обмена с демоном
 * \param[in] send_path Путь к очереди для отправки сообщений
 * \param[in] recv_path Путь к очереди для приёма сообщений
 * \param[in] send_flags Флаги открытия очереди отправки
 * \param[in] recv_flags Флаги открытия очереди приёма
 * \return Структура с дескрипторами открытых очередей
 */
mqfd_t open_queue(const char *send_path, const char *recv_path,
		  const int send_flags, const int recv_flags);

/*!
 * \brief Формирует структуру команды из строки
 * \param[in] str Имя команды
 * \param[out] command Структура для заполнения
 * \param[in] path Путь к файлу (или пустая строка)
 * \param[in] op Тип команды
 * \return 0 - успех, 1 - ошибка
 */
int set_command(const char *str, command_t *command, const char *path,
		const command_type_t op);

/**
 * \brief Определяет тип команды по её имени
 * \param[in] cmd Строка с именем команды
 * \return Тип команды (command_type_t)
 */
command_type_t get_command_type(const char *cmd);

/**
 * \brief Преобразует числовой код политики в строку
 * \param[in] replay Указатель на структуру ответа с политикой
 * \param[out] str Буфер для записи названия политики
 * \return 0 - успех, 1 - неизвестная политика
 */
int get_politic_str(reply_t *replay, char *str);

#endif

/*! @}*/