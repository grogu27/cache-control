#include "../../include/cache_control/file_monitoring.h"
#include <unistd.h>

// инициализация структуры мониторинга
int init_file_monitoring(struct file_monitoring *fm)
{
	if ((fm->epoll_fd = epoll_create1(0)) == -1) {
		logging(LOG_ERR, "<init_file_monitoring> epoll_create1 failed");
		return -1;
	}
	if ((fm->main_timer_fd = timerfd_create(CLOCK_REALTIME, 0)) == -1) {
		logging(LOG_ERR,
			"<init_file_monitoring> timerfd_create failed");
		return -1;
	}
	if ((fm->inotify_fd = inotify_init()) == -1) {
		logging(LOG_ERR, "<init_file_monitoring> inotify_init failed");
		return -1;
	}
	logging(LOG_INFO, "struct file_monitoring initialized successfully");

	return 0;
}

// собирает статистику для файла
int do_file_statistics(struct file *myfile)
{
	if (myfile == NULL) {
		logging(LOG_ERR,
			"<do_file_statistics> struct file* myfile is NULL");
		return -1;
	}
	if (myfile->stat == NULL) {
		logging(LOG_ERR, "<do_file_statistics> myfile->stat is NULL");
		return -1;
	}
	int recv_la =
		get_last_access(myfile->path, &(myfile->stat->last_access));
	if (recv_la == -1) {
		// ! логрование !
		logging(LOG_ERR, "<do_file_statistics> get_last_access failed");
		return -1;
	}

	char log_output[LOG_BUFF];
	sprintf(log_output, "size = %d", (int)(myfile->stat->size));
	logging(LOG_INFO, log_output);
	sprintf(log_output, "last access = %d",
		(int)(myfile->stat->last_access.tv_sec));
	logging(LOG_INFO, log_output);
	if (myfile->vmem_mapping == NULL) {
		if (myfile->politic != EVICT) {
			//! логирование !
			logging(LOG_ERR,
				"<do_file_statistics> myfile->vmem_mapping is NULL and myfile`s politics isn`t EVICT");
			return -1;
		}
		// ! логирование ! файлы evict могут не иметь vmem после выгрузки, так что всё ок
		logging(LOG_WARNING,
			"<do_file_statistics> file with EVICT politics has been already unmapped");
		return 0;
	}
	int ps = get_page_stat(myfile);
	if (ps == -1) {
		// ! логирование !
		logging(LOG_ERR, "get_page_stat failed");
		return -1;
	}
	sprintf(log_output, "number_of_page = %d",
		myfile->stat->number_of_pages);
	logging(LOG_INFO, log_output);
	sprintf(log_output, "number_of_page_cache = %d",
		myfile->stat->number_of_pages_cache);
	logging(LOG_INFO, log_output);
	sprintf(log_output, "percent_cache = %d", myfile->stat->percent_cache);
	logging(LOG_INFO, log_output);

	return 0;
}

// смотрит на политику файла и применяет соотвествующие действия для файла
int do_file_loading(struct file *myfile)
{
	if (myfile == NULL) {
		// ! логирование !
		logging(LOG_ERR,
			"<do_file_loading> struct file* myfile is NULL");
		return -1;
	}
	if (myfile->stat == NULL) {
		// ! логирование !
		logging(LOG_ERR, "<do_file_loading> myfile->stat is NULL");

		return -1;
	}

	switch (myfile->politic) {
	case KEEP:
		logging(LOG_INFO, "file`s politic is KEEP");
		if (myfile->stat->percent_cache < MIN_PERCENT_CACHE) {
			if (load_file(myfile) == -1) {
				// ! логирование !
				logging(LOG_ERR,
					"<do_file_loading> load failed");
				return -1;
			}
			logging(LOG_INFO,
				"KEEP policy applied: file loaded into cache");
		}
		break;
	case EVICT:
		logging(LOG_INFO, "file`s politic is EVICT");
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
				logging(LOG_ERR,
					"<do_file_loading> unmap failed");
				return -1;
			}
			logging(LOG_INFO, "it`s time to unmap evict file");
		} else if ((diff < myfile->age) &&
			   (myfile->vmem_mapping == NULL)) {
			if (map_file(myfile) == -1) {
				return -1;
			}
			logging(LOG_INFO,
				"evict file has been mapped again because someone opened it");
		} else {
			logging(LOG_INFO, "it`s too early to unmap evict file");
		}

		break;
	case MONITOR:
		logging(LOG_INFO, "file`s politic is MONITOR");
	}

	return 0;
}

// выделяет виртуальный адрес для всех файлов
int map_all_files(struct list *list_files)
{
	if (list_files == NULL) {
		logging(LOG_ERR,
			"<map_all_files> struct list* list_files is NULL");
		return -1;
	}
	if (list_files->start == NULL) {
		logging(LOG_ERR, "<map_all_files> list_files->start is NULL");

		return -1;
	}

	struct node *cur_node = list_files->start;
	while (cur_node != NULL) {
		char log_output[LOG_BUFF];
		sprintf(log_output, "Mapping file: %s",
			cur_node->nd_file->path);
		logging(LOG_INFO, log_output);
		if (cur_node->nd_file->stat != NULL) {
			logging(LOG_WARNING,
				"<map_all_files> curent file already has statistics");
			continue;
		}
		cur_node->nd_file->stat = init_empty_stat();
		off_t fs = get_file_size(cur_node->nd_file->path);
		if (fs == -1) {
			// ! логирование !
			logging(LOG_ERR,
				"<map_all_files> get_file_size failed");
			return -1;
		}
		cur_node->nd_file->stat->size = fs;
		if (map_file(cur_node->nd_file) == -1) {
			// ! логирование !
			logging(LOG_ERR, "<map_all_files> map_file failed");
			return -1;
		}

		cur_node = cur_node->nd_next;
	}

	logging(LOG_INFO, "Mapping is done");
	return 0;
}

// обрабатывает все файлы: сначала собирает статистику, затем применяет политику
int processing_all_files(struct list *list_files)
{
	struct node *cur_node = list_files->start;
	while (cur_node != NULL) {
		char log_output[LOG_BUFF];
		sprintf(log_output, "current file: %s",
			cur_node->nd_file->path);
		logging(LOG_INFO, log_output);
		logging(LOG_INFO, "try to get file statistics");
		if (do_file_statistics(cur_node->nd_file) == -1) {
			// ! логирование !
			logging(LOG_ERR,
				"<processing_all_files> do_file_statistics failed");
			return -1;
		}

		logging(LOG_INFO, "apply file`s politics");
		if (do_file_loading(cur_node->nd_file) == -1) {
			// ! логирование !
			logging(LOG_ERR,
				"<processing_all_files> do_file_loading failed");
			return -1;
		}
		cur_node = cur_node->nd_next;
	}

	logging(LOG_INFO, "all files processed successfully");

	return 0;
}

// проверяет какой тип дискриптора принял epoll
// 	main_timer_fd - сработвал таймер
// 	inotify_fd - сработал inotify
int check_type_fd(int fd, struct file_monitoring *fm, struct list *file_list)
{
	if (fd < 0) {
		logging(LOG_ERR, "<check_type_fd> fd is negative");
		return -1;
	}
	if (fm == NULL) {
		logging(LOG_ERR,
			"<check_type_fd> struct file_monitoring* fm is NULL");
		return -1;
	}
	if (file_list == NULL) {
		logging(LOG_ERR,
			"<check_type_fd> struct list* file_list is NULL");
		return -1;
	}

	if (fd == fm->main_timer_fd) { // это таймер
		// 1) собирам статистику о файле
		// 2) проверяем политику:
		// 	monitor - пропускаем
		// 	keep - проверяем percent_cache и если мало подгружаем
		// 	evict - проверяем разницу last_access и текущего времени
		// и если много выгружаем

		logging(LOG_INFO, "this is timer");
		if (processing_all_files(file_list) == -1) {
			// ! логирование !
			logging(LOG_ERR,
				"<check_type_fd> proccessing_all_files failed");
			return -1;
		}
		uint64_t read_buff;
		if (read(fm->main_timer_fd, &read_buff, sizeof(read_buff)) ==
		    -1) {
			logging(LOG_ERR,
				"<check_type_fd> read by main_timer_fd failed");

			return -1;
		}
	} else if (fd == fm->inotify_fd) { // это inotify
		logging(LOG_INFO, "this is inotify");
		// проверяем тип изменений:
		// 	IN_DELETE - удаляем файл из списка
		// 	IN_MODIFY - меняем размер файла в stat, заново задаём fd
		// и vmem_mapping 	IN_MOVE - меняем путь файла
		if (read_inotify(fm->inotify_fd, file_list) == -1) {
			// ! логирование !
			logging(LOG_ERR, "<check_type_fd> read_inotify failed");

			return -1;
		}
	}
	return 0;
}

// Перезагрузка конфига: чтение нового файла, пересоздание epoll/inotify, замена списка файлов
int reload_config(struct cache_control *cc, struct file_monitoring *fm){
	logging(LOG_INFO, "reloading configuration...");
	struct list *new_list = init_list();
	if (new_list == NULL){
		logging(LOG_ERR, "Failed to allocate new list");
		return -1;
	}
	int new_interval = cc->interval;
	int res_parse_config = parse_config(cc->config_path, new_list, &new_interval);
	if (res_parse_config == 1) {
		//логирование, не открывается конфиг файл
		logging(LOG_ERR, "<reload_config> failed to open config file");
		free_list(new_list);
		return -1;
	} else if (res_parse_config == 2) {
		//логирование, синтаксическая ошибка в конфиг файле
		logging(LOG_ERR, "<reload_config> syntax error in config file");
		free_list(new_list);
		return -1;
	} else if (res_parse_config == 3){
		//логирование, семантическая ошибка в конфиг файле
		logging(LOG_ERR, "<reload_config> semantic error in config file");
		free_list(new_list);
		return -1;
	}
	logging(LOG_INFO, "<reload_config> config parsed");

	if (map_all_files(new_list) == -1){
		logging(LOG_ERR, "<reload_config> map_all_files failed");
		free_list(new_list);
		return -1;
	}
	struct list *old_list = cc->file_list;
	if (fm->inotify_fd != -1){
		close(fm->inotify_fd);
		fm->inotify_fd = -1;
		logging(LOG_INFO, "<reload_config> closed old inotify fd");
	}

	if (fm->epoll_fd != -1){
		close(fm->epoll_fd);
		fm->epoll_fd = -1;
		logging(LOG_INFO, "<reload_config> closed old epoll fd");
	}

	if (fm->main_timer_fd != -1){
		close(fm->main_timer_fd);
		fm->main_timer_fd = -1;
		logging(LOG_INFO, "<reload_config> closed main_timer_fd");
	}

	if (init_file_monitoring(fm) == -1){
		logging(LOG_ERR, "<reload_config> init_file_monitoring failed");
		free_list(new_list);
		return -1;
	}
	logging(LOG_INFO, "<reload_config> init_file_monitoring success");

	if (add_fd_to_epoll(fm->epoll_fd, fm->main_timer_fd) == -1){
		logging(LOG_ERR, "<reload_config> add_fd_to_epoll failed (main_timer_fd)");
		free_list(new_list);
		return -1;
	}
	if (add_all_files_to_inotify(fm->inotify_fd, new_list) == -1){
		logging(LOG_ERR, "<reload_config> add_all_files_to_inotify failed");
		free_list(new_list);
		return -1;
	}
	if (add_fd_to_epoll(fm->epoll_fd, fm->inotify_fd) == -1){
		logging(LOG_ERR, "<reload_config> add_fd_to_epoll failed (inotify_fd)");
		free_list(new_list);
		return -1;
	}

	cc->file_list = new_list;
	cc->interval = new_interval;
	if (old_list != NULL){
		free_list(old_list);
	}
	set_timer(fm->main_timer_fd, cc->interval);
	logging(LOG_INFO, "<reload_config> config reload");
	return 0;
}

// запускает цикл обрабоки файлов, сбора статистики и т.д.
void *start_file_monitoring(void *args)
{
	if (args == NULL) {
		// ! логирование !
		logging(LOG_ERR, "<start_file_monitoring> args is NULL");

		return NULL;
	}
	struct cache_control *cc = (struct cache_control *)args;

	// инициализация структуры file_monitoring
	struct file_monitoring fm;
	if (init_file_monitoring(&fm) == -1) {
		// ! лоигрование !
		logging(LOG_INFO,
			"<start_file_monitoring> init_file_monitoring failed");
		return NULL;
	}

	// выделили виртуальный адрес для всех файлов
	if (map_all_files(cc->file_list) == -1) {
		// ! логирование, каких-то файлов не существует !
		logging(LOG_ERR,
			"<start_file_monitoring> map_all_files failed");
		return NULL;
	}

	// задаём время для таймера и запускаем его
	if (set_timer(fm.main_timer_fd, cc->interval) == -1) {
		logging(LOG_ERR, "<start_file_monitoring> set_timer failed");
		return NULL;
	}
	if (add_fd_to_epoll(fm.epoll_fd, fm.main_timer_fd) == -1) {
		logging(LOG_ERR,
			"<start_file_monitoring> add_fd_to_epoll (timer) failed");
		return NULL;
	}

	// подписываем файлы на события IN_MODIFY, IN_DELETE, IN_MOVE и
	// запускаем прослушку
	if (add_all_files_to_inotify(fm.inotify_fd, cc->file_list) == -1) {
		logging(LOG_ERR,
			"<start_file_monitoring> add_all_files_to_inotify failed");
		return NULL;
	}
	if (add_fd_to_epoll(fm.epoll_fd, fm.inotify_fd) == -1) {
		logging(LOG_ERR,
			"<start_file_monitoring> add_fd_to_epoll (inotify) failed");
		return NULL;
	}

	struct epoll_event events[MAX_NUM_OF_EVENTS];
	while (!cc->shutdown_flag) {
		if (cc->reload_flag == 1){
			cc->reload_flag = 0;
			logging(LOG_INFO, "<start_file_monitoring> SIGHUP received, reloading config");
			if (reload_config(cc, &fm) == -1){
				logging(LOG_WARNING, "<start_file_monitoring> reload_config failed");
				return NULL;
			}
		}
		// проверка сработал ли таймер, если нет - continue
		int num_ready = epoll_wait(fm.epoll_fd, events,
					   MAX_NUM_OF_EVENTS, TIMEOUT);
		if (num_ready == 0) {
			continue;
		}

		// таймер сработал
		logging(LOG_INFO, "epoll caught event");

		for (int i = 0; i < MAX_NUM_OF_EVENTS; i++) {
			if (events[i].events & EPOLLIN) {
				if ((check_type_fd(events[i].data.fd, &fm,
						   cc->file_list)) == -1) {
					// ! логирование !
					logging(LOG_ERR,
						"<start_file_monitoring> check_type_fd failed");
					return NULL;
				}
			}
		}
	}

	logging(LOG_INFO, "got shutdown_flag, file_monitroing ended work");
	return NULL;
}
