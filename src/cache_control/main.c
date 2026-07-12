#include "../../include/cache_control/options.h"
#include "../../include/cache_control/daemonize.h"
#include "../../include/cache_control/signal_handler.h"
#include "../../include/cache_control/context.h"
#include "../../include/cache_control/parse_config.h"
#include "../../include/cache_control/threads.h"
#include "../../include/log/log.h"

int main(int argc, char **argv)
{
	struct cache_control cc;
	init_context(&cc);

	int result = parse_options(argc, argv, &cc);

	if (result == -2) {
		logging(LOG_INFO, "<main> help option was displayed");
		return EXIT_SUCCESS;
	}

	if (result != 0) {
		logging(LOG_ERR, "<main> options parsing failed");
		printf("Options parsing is failed\n");
		return EXIT_FAILURE;
	}

	logging(LOG_INFO, "<main> options parsed successfully");
	printf("Options parsing is success\n");

	printf("PID = %d\n", getpid());

	if (cc.daemon_flag == 1) {
		logging(LOG_INFO, "<main> daemon flag enabled");
		if (daemonize() != 0) {
			logging(LOG_ERR, "<main> daemonize failed");
			return EXIT_FAILURE;
		}
		logging(LOG_INFO, "<main> daemon started with config");
	} else {
		logging(LOG_INFO, "<main> foreground mode");
		printf("Program run in foreground\n");
		printf("Config path: %s\n", cc.config_path);
	}

	set_context(&cc);
	logging(LOG_INFO, "<main> context set for signal handlers");

	init_signal_handler();
	cc.file_list = init_list();
	if (cc.file_list == NULL) {
		logging(LOG_ERR, "<main> failed to allocate list");
		return EXIT_FAILURE;
	}
	int res_parse_config =
		parse_config(cc.config_path, cc.file_list, &cc.interval);
	if (res_parse_config == 1) {
		logging(LOG_ERR, "<main> failed to open config file");
		return EXIT_FAILURE;
	} else if (res_parse_config == 2) {
		logging(LOG_ERR, "<main> syntax error in config file");
		return EXIT_FAILURE;
	} else if (res_parse_config == 3) {
		logging(LOG_ERR, "<main> semantic error in config file");
		return EXIT_FAILURE;
	}
	logging(LOG_INFO, "<main> config parsed");

	if (start_threads(&cc) != 0) {
		logging(LOG_ERR, "<main> failed to start threads");
		remove_context(&cc);
		return EXIT_FAILURE;
	}

	logging(LOG_INFO, "<main> entering main loop");
	char log_output[LOG_BUFF];
	while (!cc.shutdown_flag) {
		if (cc.stats_flag == 1) {
			cc.stats_flag = 0;
			logging(LOG_INFO, "STATISTICS");
			sprintf(log_output, "files: %d", cc.file_list->length);
			logging(LOG_INFO, log_output);

			struct node *cur_node = cc.file_list->start;
			int count = 0;
			while (cur_node != NULL) {
				count++;
				sprintf(log_output,
					"%d) %s | %d | size=%ld | number_of_pages=%d | number_of_pages_cache=%d | percent_cache=%d",
					count, cur_node->nd_file->path,
					cur_node->nd_file->politic,
					cur_node->nd_file->stat->size,
					cur_node->nd_file->stat->number_of_pages,
					cur_node->nd_file->stat
						->number_of_pages_cache,
					cur_node->nd_file->stat->percent_cache);
				logging(LOG_INFO, log_output);
				cur_node = cur_node->nd_next;
			}
		}
	}

	logging(LOG_INFO, "<main> exited main loop, shutting down");
	if (stop_threads() != 0) {
		logging(LOG_ERR, "<main> failed to stop threads");
	} else {
		logging(LOG_INFO, "<main> threads stopped successfully");
	}
	if (remove_context(&cc) != 0) {
		logging(LOG_ERR, "<main> failed to remove context");
	} else {
		logging(LOG_INFO, "<main> context removed successfully");
	}
	logging(LOG_INFO, "<main> program finished");
	return EXIT_SUCCESS;
}
