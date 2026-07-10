#include "../../include/cache_control/events_stuff.h"

int set_timer(int timer_fd, time_t seconds)
{
	if (timer_fd < 0) {
		logging(LOG_ERR, "<set_timer> timer_fd is negative");
		return -1;
	}
	if (seconds < 0) {
		logging(LOG_ERR, "<set_timer> seconds value is negative");
		return -1;
	}

	struct itimerspec spec;
	spec.it_value.tv_sec = seconds;
	spec.it_value.tv_nsec = 0;
	spec.it_interval.tv_sec = seconds;
	spec.it_interval.tv_nsec = 0;
	if (timerfd_settime(timer_fd, 0, &spec, NULL) == -1) {
		// ! логирование !
		logging(LOG_ERR, "<set_timer> timerfd_settime failed");
		return -1;
	}

	char log_output[LOG_BUFF];
	sprintf(log_output, "timer has been set, interval=%d", (int)seconds);
	logging(LOG_INFO, log_output);
	return 0;
}

int add_fd_to_epoll(int epoll_fd, int fd)
{
	if (epoll_fd < 0) {
		logging(LOG_ERR, "<add_fd_to_epoll> epoll_fd is negative");
		return -1;
	}
	if (fd < 0) {
		logging(LOG_ERR, "<add_fd_to_epoll> fd is negative");
		return -1;
	}

	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN;
	int recv_epoll_ctl =
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, event.data.fd, &event);
	if (recv_epoll_ctl == -1) {
		// ! логирование !
		logging(LOG_ERR, "<add_fd_to_epoll> epoll_ctl failed");
		return -1;
	}

	char log_output[LOG_BUFF];
	sprintf(log_output, "fd=%d was added to epoll", fd);
	logging(LOG_INFO, log_output);
	return 0;
}
