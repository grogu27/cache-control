#include "../../include/cache_control/signal_handler.h"

static struct cache_control *ctx = NULL;

//установка структуры для обработки сигналов
void set_context(struct cache_control *cc)
{
	ctx = cc;
}

//обработчик для сигнала sigint
void sigint_handler(int sig)
{
	(void)sig;
	//логирование, пришел сигнал sigint
	if (ctx != NULL) {
		ctx->shutdown_flag = 1;
	}
}

//обработчик для сигнала sigterm
void sigterm_handler(int sig)
{
	(void)sig;
	//логирование, пришел сигнал sigterm
	if (ctx != NULL) {
		ctx->shutdown_flag = 1;
	}
}

// void sighup_handler(int sig){
// 	(void)sig;
// 	поднимется флаг для перезагрузки конфига
// }

// void sigusr1_handler(int sig){
// 	(void)sig;
// поднимется флаг для вывода текущей статистики в syslog
// }

//регистрация обработчика для сигнала
int register_handler(int signum, void (*handler)(int))
{
	struct sigaction sa = { 0 };
	sa.sa_handler = handler;

	if (sigaction(signum, &sa, NULL) == -1) {
		perror("sigaction failed");
		return -1;
	}

	return 0;
}

//инициализация всех обработчиков сигналов
void init_signal_handler()
{
	register_handler(SIGINT, sigint_handler);
	register_handler(SIGTERM, sigterm_handler);
	//логирование, все сигналы зареганы
}
