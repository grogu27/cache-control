#include "../../include/cache_control/threads.h"

static pthread_t monitoring_id;
static pthread_t listener_id;
static struct cache_control *ctx = NULL;

void *file_monitoring_thread(void *arg)
{
	if (arg == NULL) {
		//логирование, у потока для мониторинга входной параметр NULL
		return NULL;
	}

	struct cache_control *cc = (struct cache_control *)arg;
	start_file_monitoring(
		cc); //еще там нужно поменять условия цикла while, чтобы завершался при флаге shutdown
	// логирование, поток для мониторинга стартанул

	//логирование, поток для мониторинга завершился
	return NULL;
}

void *command_listener_thread(void *arg)
{
	if (arg == NULL) {
		//логирование, у потока для обработчика fifo входной параметр NULL
		return NULL;
	}

	struct cache_control *cc = (struct cache_control *)arg;
	// логирование, поток для обработчика fifo стартанул
	/*while (!cc->shutdown_flag) {
		//читать команды из fifo
	}*/

	command_listner(cc);

	//логирование, поток для обработчика fifo завершился
	return NULL;
}

int start_threads(struct cache_control *cc)
{
	if (cc == NULL) {
		//логирование, у start_threads входной параметр NULL
		return -1;
	}

	ctx = cc;

	if (pthread_create(&monitoring_id, NULL, file_monitoring_thread, ctx) !=
	    0) {
		//логирование, не создался поток для мониторинга
		return -1;
	}
	//логирование, поток для мониторинга создался

	if (pthread_create(&listener_id, NULL, command_listener_thread, ctx) !=
	    0) {
		//логирование, не создался поток для обработчика fifo
		return -1;
	}
	//логирование, поток для обработчика fifo создался

	return 0;
}

int stop_threads()
{
	if (ctx == NULL) {
		//логирование, у stop_threads ctx NULL
		return -1;
	}

	ctx->shutdown_flag = 1;

	pthread_join(monitoring_id, NULL);
	//лоигрование, pthread_join для потока мониторинга прошел

	pthread_join(listener_id, NULL);
	//лоигрование, pthread_join для потока обработчика fifo прошел

	return 0;
}
