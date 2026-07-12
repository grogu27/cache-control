#include "../../include/cache_control/signal_handler.h"

static struct cache_control *ctx = NULL;

void set_context(struct cache_control *cc)
{
	ctx = cc;
}

void sigint_handler(int sig)
{
	(void)sig;
	logging(LOG_INFO, "<sigint_handler> SIGINT signal was received");
	if (ctx != NULL) {
		ctx->shutdown_flag = 1;
	}
}

void sigterm_handler(int sig)
{
	(void)sig;
	logging(LOG_INFO, "<sigterm_handler> SIGTERM signal was received");
	if (ctx != NULL) {
		ctx->shutdown_flag = 1;
	}
}

void sighup_handler(int sig){
	(void)sig;
	logging(LOG_INFO, "<sighup_handler> SIGHUP signal was received");
	if (ctx != NULL) {
		ctx->reload_flag = 1;
	}
}

void sigusr1_handler(int sig){
	(void)sig;
	logging(LOG_INFO, "<sigusr1_handler> SIGUSR1 signal was received");
	if (ctx != NULL) {
		ctx->stats_flag = 1;
	}
}

int register_handler(int signum, void (*handler)(int))
{
	struct sigaction sa = { 0 };
	sa.sa_handler = handler;

	if (sigaction(signum, &sa, NULL) == -1) {
		perror("sigaction failed");
		logging(LOG_ERR, "<register_handler> sigaction failed");
		return -1;
	}
	logging(LOG_INFO, "<register_handler> handler registered");
	return 0;
}

void init_signal_handler()
{
	register_handler(SIGINT, sigint_handler);
	register_handler(SIGTERM, sigterm_handler);
	register_handler(SIGHUP, sighup_handler);
	register_handler(SIGUSR1, sigusr1_handler);
	logging(LOG_INFO, "<init_signal_handler> all signal handlers initialized");
}
