#include "../../include/cache_control/inotify_stuff.h"

int add_all_files_to_inotify(int inotify_fd, struct list *file_list)
{
	if (inotify_fd < 0) {
		logging(LOG_ERR,
			"<add_all_files_to_inotify> inotify_fd is negative");
		return -1;
	}
	if (file_list == NULL) {
		logging(LOG_ERR,
			"<add_all_files_to_inotify> file_list is NULL");
		return -1;
	}
	if (file_list->start == NULL) {
		logging(LOG_ERR,
			"<add_all_files_to_inotify> file_list->start is NULL");
		return -1;
	}

	struct node *cur_node = file_list->start;
	while (cur_node != NULL) {
		char log_output[LOG_BUFF];
		sprintf(log_output, "Adding file %s to inotify",
			cur_node->nd_file->path);
		logging(LOG_INFO, log_output);
		int recv_add_watch =
			inotify_add_watch(inotify_fd, cur_node->nd_file->path,
					  IN_ATTRIB | IN_MODIFY | IN_MOVE_SELF);
		if (recv_add_watch == -1) {
			logging(LOG_ERR,
				"<add_all_files_to_inotify> inotify_add_watch failed");
			return -1;
		}
		cur_node->nd_file->wd = recv_add_watch;
		cur_node = cur_node->nd_next;
	}

	logging(LOG_INFO, "all files successfully added to inotify");
	return 0;
}

int remove_all_files_from_inotify(int inotify_fd, struct list *file_list)
{
	if (inotify_fd < 0) {
		logging(LOG_ERR,
			"<remove_all_files_from_inotify> inotify_fd is negative");
		return -1;
	}
	if (file_list == NULL) {
		logging(LOG_ERR,
			"<remove_all_files_from_inotify> file_list is NULL");
		return -1;
	}
	if (file_list->start == NULL) {
		logging(LOG_ERR,
			"<remove_all_files_from_inotify> file_list->start is NULL");
		return -1;
	}

	struct node *cur_node = file_list->start;
	while (cur_node != NULL) {
		char log_output[LOG_BUFF];
		sprintf(log_output, "Removing file %s from inotify",
			cur_node->nd_file->path);
		logging(LOG_INFO, log_output);

		int recv_rm_watch =
			inotify_rm_watch(inotify_fd, cur_node->nd_file->wd);
		if (recv_rm_watch == -1) {
			logging(LOG_ERR,
				"<remove_all_files_from_inotify> inotify_rm_watch failed");
			return -1;
		}
		cur_node = cur_node->nd_next;
	}
	logging(LOG_INFO, "all file saccessfully removed from inotify");

	return 0;
}

int file_in_attrib(int wd, struct list *file_list)
{
	struct file *changed_file = find_file_by_wd(file_list, wd);
	if (changed_file == NULL) {
		logging(LOG_ERR, "<file_in_attrib> find_file_by_wd failed");
		return -1;
	}
	if ((unmap_file(changed_file) == -1) &&
	    (changed_file->politic != EVICT)) {
		logging(LOG_ERR,
			"<file_in_attrib> KEEP or MONITOR file doesn`t have virtual address");
		return 1;
	}
	off_t new_size = get_file_size(changed_file->path);
	if (new_size == -1) {
		if (delete_file_by_wd(file_list, wd) == -1) {
			logging(LOG_ERR,
				"<file_in_attrib> delete_file_by_wd failed");
			return -1;
		}
		logging(LOG_WARNING, "<file_in_attrib> file has been deleted");

		return 1;
	}
	changed_file->stat->size = new_size;
	if (map_file(changed_file) == -1) {
		logging(LOG_ERR, "<file_in_attrib> map_file failed");

		return -1;
	}

	logging(LOG_WARNING,
		"<file_in_attrib> file has changed its size and got new virtual address");
	return 0;
}

int file_in_modify(int wd, struct list *file_list)
{
	struct file *changed_file = find_file_by_wd(file_list, wd);
	if (changed_file == NULL) {
		logging(LOG_ERR, "<file_in_modify> find_file_by_wd failed");
		return -1;
	}
	// изымаем виртуальный адрес
	if ((unmap_file(changed_file) == -1) &&
	    (changed_file->politic == EVICT)) {
		logging(LOG_ERR,
			"<file_in_modify> KEEP or MONITOR file doesn`t have virtual address");
		return 1;
	}
	// пересчитываем размер файла
	off_t new_size = get_file_size(changed_file->path);
	if (new_size == -1) {
		logging(LOG_ERR, "<file_in_modify> get_file_size failed");
		return -1;
	}
	changed_file->stat->size = new_size;
	// назначили новый адрес
	if (map_file(changed_file) == -1) {
		logging(LOG_ERR, "<file_in_modify> map_file failed");
		return -1;
	}

	logging(LOG_WARNING,
		"<file_in_modify> file has changed its size and got new virtual address");

	return 0;
}

int read_inotify(int inotify_fd, struct list *file_list)
{
	size_t buff_size = sizeof(struct inotify_event) + PATH_BUFF_SIZE + 1;
	struct inotify_event *event = malloc(buff_size);
	if (event == NULL) {
		logging(LOG_ERR, "<read_inotify> malloc failed");
		return -1;
	}

	if (read(inotify_fd, event, buff_size) == -1) {
		logging(LOG_ERR, "<read_inotify> read by inotify_fd failed");
		free(event);
		return -1;
	}

	if (event->mask & IN_ATTRIB) {
		logging(LOG_INFO, "inotify caught IN_ATTRIB");
		if (file_in_attrib(event->wd, file_list) == -1) {
			logging(LOG_ERR,
				"<read_inotify> file_in_attrib failed");
			free(event);
			return -1;
		}
	} else if (event->mask & IN_MOVE_SELF) {
		logging(LOG_INFO, "inotify caught IN_MOVE_SELF");
		if (delete_file_by_wd(file_list, event->wd) == -1) {
			logging(LOG_ERR,
				"<read_inotify> delete_file_by_wd failed");
			free(event);
			return -1;
		}
	} else if (event->mask & (IN_MODIFY)) {
		logging(LOG_INFO, "inotify caught IN_MODIFY");
		if (file_in_modify(event->wd, file_list) == -1) {
			logging(LOG_ERR,
				"<read_inotify> file_in_modify failed");
			free(event);
			return -1;
		}
	}
	free(event);
	return 0;
}
