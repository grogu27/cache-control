#include "../../include/log/log.h"

//Функция открытия файл лога для записи в syslog
void open_log()
{
	openlog(NULL, LOG_CONS | LOG_PERROR, LOG_USER);
}

/*
Функция принимает 2 параметра уровень логирования и сообщение которое нужно записать в syslog
Уровни логировани которые используются LOG_INFO LOG_WARNING LOG_ERR
Сообщения которые записываются в syslog указываются при вызове функции или берутся при помощи strerror(errno)
*/
void logging(int log_level, char *msg)
{
	if (msg == NULL) {
		syslog(LOG_INFO, "General: Empty call logging");
	}
	if (log_level == LOG_INFO || log_level == LOG_WARNING) {
		if (log_level == LOG_INFO)
			syslog(log_level, "[INFO]: %s", msg);
		if (log_level == LOG_WARNING)
			syslog(log_level, "[WARNING]: %s", msg);
	} else if (log_level == LOG_ERR) {
		syslog(log_level, "[ERROR] %s: %s", msg, strerror(errno));
	}
}

//Функция закрытия файл лога для записи в syslog
void close_log()
{
	closelog();
}
