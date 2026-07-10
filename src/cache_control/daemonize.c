#include "../../include/cache_control/daemonize.h"

//Демонизация
// Возвращает 0 если успех и -1 если системный вызов создания демона завершился неудачно
int daemonize()
{
	if (daemon(0, 0) != 0) {
		//логирование, системный вызов закончился неудачно ;(
		logging(LOG_ERR, "<daemonize> daemon system call failed");
		perror("daemon failed");
		return -1;
	}

	//логирование, демон запустился ;)
	logging(LOG_INFO, "daemon system call successfully started");
	return 0;
}
