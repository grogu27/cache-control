#include "../../include/cache_control/daemonize.h"

int daemonize()
{
	if (daemon(0, 0) != 0) {
		logging(LOG_ERR, "<daemonize> daemon system call failed");
		perror("daemon failed");
		return -1;
	}

	logging(LOG_INFO, "daemon system call successfully started");
	return 0;
}
