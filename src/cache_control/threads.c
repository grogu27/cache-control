#include "../../include/cache_control/threads.h"

static pthread_t monitoring_id;
static pthread_t listener_id;
static struct cache_control *ctx = NULL;

void *file_monitoring_thread(void *arg)
{
	if (arg == NULL) {
		logging(LOG_ERR, "<file_monitoring_thread> arg is NULL");
		return NULL;
	}

	struct cache_control *cc = (struct cache_control *)arg;
	logging(LOG_INFO, "<file_monitoring_thread> thread started");
	start_file_monitoring(cc);
	logging(LOG_INFO, "<file_monitoring_thread> thread finished");
	return NULL;
}

void *command_listener_thread(void *arg)
{
	if (arg == NULL) {
		logging(LOG_ERR, "<command_listener_thread> arg is NULL");
		return NULL;
	}

	struct cache_control *cc = (struct cache_control *)arg;
	logging(LOG_INFO, "<command_listener_thread> thread started");
	command_listner(cc);
	logging(LOG_INFO, "<command_listener_thread> thread finished");
	return NULL;
}

int start_threads(struct cache_control *cc)
{
	if (cc == NULL) {
		logging(LOG_ERR, "<start_threads> cc is NULL");
		return -1;
	}

	ctx = cc;

	if (pthread_create(&monitoring_id, NULL, file_monitoring_thread, ctx) !=
	    0) {
		logging(LOG_ERR,
			"<start_threads> pthread_create failed (file_monitoring_thread)");
		return -1;
	}
	logging(LOG_INFO, "<start_threads> monitoring thread created");

	if (pthread_create(&listener_id, NULL, command_listener_thread, ctx) !=
	    0) {
		logging(LOG_ERR,
			"<start_threads> pthread_create failed (command_listener_thread)");
		return -1;
	}
	logging(LOG_INFO, "<start_threads> command listener thread created");

	return 0;
}

int stop_threads()
{
	if (ctx == NULL) {
		logging(LOG_ERR, "<stop_threads> ctx is NULL");
		return -1;
	}

	pthread_join(monitoring_id, NULL);
	logging(LOG_INFO, "<stop_threads> monitoring thread joined");

	pthread_join(listener_id, NULL);
	logging(LOG_INFO, "<stop_threads> command listener thread joined");

	return 0;
}
