#include "../../include/cache_control/file_monitoring.h"

// инициализация структуры мониторинга
int init_file_monitoring(struct file_monitoring *fm)
{
	if ((fm->epoll_fd = epoll_create1(0)) == -1) {
		return -1;
	}
	if ((fm->main_timer_fd = timerfd_create(CLOCK_REALTIME, 0)) == -1) {
		return -1;
	}
	if ((fm->inotify_fd = inotify_init()) == -1) {
		return -1;
	}
	return 0;
}

// собирает статистику для файла
int do_file_statistics(struct file *myfile)
{
	if (myfile == NULL) {
		return -1;
	}
	if (myfile->stat == NULL) {
		return -1;
	}
	int recv_la =
		get_last_access(myfile->path, &(myfile->stat->last_access));
	if (recv_la == -1) {
		// ! логрование !
		return -1;
	}
	if (myfile->vmem_mapping == NULL) {
		if (myfile->politic != EVICT) {
			//! логирование ! файлы evict могут не иметь vmem после выгрузки, так что всё ок
			return -1;
		}
		// ! логирование !
		return 0;
	}
	int ps = get_page_stat(myfile);
	if (ps == -1) {
		// ! логирование !
		return -1;
	}

	return 0;
}

// смотрит на политику файла и применяет соотвествующие действия для файла
int do_file_loading(struct file *myfile)
{
	if (myfile == NULL) {
		// ! логирование !
		return -1;
	}
	if (myfile->stat == NULL) {
		// ! логирование !
		return -1;
	}

	switch (myfile->politic) {
	case KEEP:
		if (myfile->stat->percent_cache < MIN_PERCENT_CACHE) {
			if (load_file(myfile) == -1) {
				// ! логирование !
				return -1;
			}
		}
		break;
	case EVICT:
		// берём текущее время
		struct timespec curr_time;
		clock_gettime(CLOCK_REALTIME, &curr_time);
		// разница между текущим временем и временем последнего
		// доступа
		int diff = (int)curr_time.tv_sec -
			   (int)myfile->stat->last_access.tv_sec;
		if ((diff >= myfile->age) && (myfile->vmem_mapping != NULL)) {
			if (unmap_file(myfile) == -1) {
				// ! логирование !
				return -1;
			}
		} else if ((diff < myfile->age) &&
			   (myfile->vmem_mapping == NULL)) {
			if (map_file(myfile) == -1) {
				return -1;
			}
		}

		break;
	}
	return 0;
}

// выделяет виртуальный адрес для всех файлов
int map_all_files(struct list *list_files)
{
	if (list_files == NULL) {
		return -1;
	}
	if (list_files->start == NULL) {
		return -1;
	}

	struct node *cur_node = list_files->start;
	while (cur_node != NULL) {
		if (cur_node->nd_file->stat != NULL) {
			continue;
		}
		cur_node->nd_file->stat = init_empty_stat();
		off_t fs = get_file_size(cur_node->nd_file->path);
		if (fs == -1) {
			// ! логирование !
			return -1;
		}
		cur_node->nd_file->stat->size = fs;
		if (map_file(cur_node->nd_file) == -1) {
			// ! логирование !
			return -1;
		}
		cur_node = cur_node->nd_next;
	}

	return 0;
}

// обрабатывает все файлы: сначала собирает статистику, затем применяет политику
int processing_all_files(struct list *list_files)
{
	struct node *cur_node = list_files->start;
	while (cur_node != NULL) {
		if (do_file_statistics(cur_node->nd_file) == -1) {
			// ! логирование !
			return -1;
		}
		if (do_file_loading(cur_node->nd_file) == -1) {
			// ! логирование !
			return -1;
		}
		cur_node = cur_node->nd_next;
	}

	return 0;
}

// проверяет какой тип дискриптора принял epoll
// 	main_timer_fd - сработвал таймер
// 	inotify_fd - сработал inotify
int check_type_fd(int fd, struct file_monitoring *fm, struct list *file_list)
{
	if (fd < 0) {
		return -1;
	}
	if (fm == NULL) {
		return -1;
	}
	if (file_list == NULL) {
		return -1;
	}

	if (fd == fm->main_timer_fd) { // это таймер
		// 1) собирам статистику о файле
		// 2) проверяем политику:
		// 	monitor - пропускаем
		// 	keep - проверяем percent_cache и если мало подгружаем
		// 	evict - проверяем разницу last_access и текущего времени
		// и если много выгружаем
		if (processing_all_files(file_list) == -1) {
			// ! логирование !
			return -1;
		}
		uint64_t read_buff;
		if (read(fm->main_timer_fd, &read_buff, sizeof(read_buff)) ==
		    -1) {
			return -1;
		}
	} else if (fd == fm->inotify_fd) { // это inotify
		// проверяем тип изменений:
		// 	IN_DELETE - удаляем файл из списка
		// 	IN_MODIFY - меняем размер файла в stat, заново задаём fd
		// и vmem_mapping 	IN_MOVE - меняем путь файла
		if (read_inotify(fm->inotify_fd, file_list) == -1) {
			// ! логирование !
			return -1;
		}
	}
	return 0;
}

// запускает цикл обрабоки файлов, сбора статистики и т.д.
void *start_file_monitoring(void *args)
{
	if (args == NULL) {
		// ! логирование !
		return NULL;
	}
	struct cache_control *cc = (struct cache_control *)args;

	// инициализация структуры file_monitoring
	struct file_monitoring fm;
	if (init_file_monitoring(&fm) == -1) {
		// ! лоигрование !
		return NULL;
	}

	// выделили виртуальный адрес для всех файлов
	if (map_all_files(cc->file_list) == -1) {
		// ! логирование, каких-то файлов не существует !
		return NULL; // если файл не имеет прав, чтобы его читать , то запуск потом зависнет и не завершится
	}

	// задаём время для таймера и запускаем его
	if (set_timer(fm.main_timer_fd, cc->interval) == -1) {
		return NULL;
	}
	if (add_fd_to_epoll(fm.epoll_fd, fm.main_timer_fd) == -1) {
		return NULL;
	}

	// подписываем файлы на события IN_MODIFY, IN_DELETE, IN_MOVE и
	// запускаем прослушку
	if (add_all_files_to_inotify(fm.inotify_fd, cc->file_list) == -1) {
		return NULL;
	}
	if (add_fd_to_epoll(fm.epoll_fd, fm.inotify_fd) == -1) {
		return NULL;
	}

	struct epoll_event events[MAX_NUM_OF_EVENTS];
	while (!cc->shutdown_flag) {
		// проверка сработал ли таймер, если нет - continue
		int num_ready = epoll_wait(fm.epoll_fd, events,
					   MAX_NUM_OF_EVENTS, TIMEOUT);
		if (num_ready == 0) {
			continue;
		}

		// таймер сработал
		for (int i = 0; i < MAX_NUM_OF_EVENTS; i++) {
			if (events[i].events & EPOLLIN) {
				if ((check_type_fd(events[i].data.fd, &fm,
						   cc->file_list)) == -1) {
					// ! логирование !
					return NULL;
				}
			}
		}
	}
	return NULL;
}
