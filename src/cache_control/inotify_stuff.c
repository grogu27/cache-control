#include "../../include/cache_control/inotify_stuff.h"

int add_all_files_to_inotify(int inotify_fd, struct list *file_list)
{
	if (inotify_fd < 0) {
		return -1;
	}
	if (file_list == NULL) {
		return -1;
	}
	if (file_list->start == NULL) {
		return -1;
	}

	struct node *cur_node = file_list->start;
	while (cur_node != NULL) {
		int recv_add_watch =
			inotify_add_watch(inotify_fd, cur_node->nd_file->path,
					  IN_ATTRIB | IN_MODIFY | IN_MOVE_SELF);
		if (recv_add_watch == -1) {
			return -1;
		}
		cur_node->nd_file->wd = recv_add_watch;
		cur_node = cur_node->nd_next;
	}
	return 0;
}

int remove_all_files_from_inotify(int inotify_fd, struct list *file_list)
{
	if (inotify_fd < 0) {
		return -1;
	}
	if (file_list == NULL) {
		return -1;
	}
	if (file_list->start == NULL) {
		return -1;
	}

	struct node *cur_node = file_list->start;
	while (cur_node != NULL) {
		int recv_rm_watch =
			inotify_rm_watch(inotify_fd, cur_node->nd_file->wd);
		if (recv_rm_watch == -1) {
			return -1;
		}
		cur_node = cur_node->nd_next;
	}
	return 0;
}

int file_in_attrib(int wd, struct list *file_list)
{
	struct file *changed_file = find_file_by_wd(file_list, wd);
	if (changed_file == NULL) {
		// ! логирование !
		return -1;
	}
	if ((unmap_file(changed_file) == -1) &&
	    (changed_file->politic != EVICT)) {
		return -1;
	}
	off_t new_size = get_file_size(changed_file->path);
	if (new_size == -1) {
		if (delete_file_by_wd(file_list, wd) == -1) {
			// ! логирование !
			return -1;
		}
		return 1;
	}
	changed_file->stat->size = new_size;
	if (map_file(changed_file) == -1) {
		return -1;
	}

	return 0;
}

int file_in_modify(int wd, struct list *file_list)
{
	struct file *changed_file = find_file_by_wd(file_list, wd);
	if (changed_file == NULL) {
		// ! логирование !
		return -1;
	}
	// изымаем виртуальный адрес
	if ((unmap_file(changed_file) == -1) &&
	    (changed_file->politic == EVICT)) {
		// ! логирование !
		return -1;
	}
	// пересчитываем размер файла
	off_t new_size = get_file_size(changed_file->path);
	if (new_size == -1) {
		// ! логирование !
		return -1;
	}
	changed_file->stat->size = new_size;
	// назначили новый адрес
	if (map_file(changed_file) == -1) {
		// ! логирование !
		return -1;
	}
	return 0;
}

int read_inotify(int inotify_fd, struct list *file_list)
{
	size_t buff_size = sizeof(struct inotify_event) + PATH_BUFF_SIZE + 1;
	struct inotify_event *event = malloc(buff_size);
	if (event == NULL) {
		// ! логирование !
		return -1;
	}

	if (read(inotify_fd, event, buff_size) == -1) {
		return -1;
	}

	if (event->mask & IN_ATTRIB) {
		if (file_in_attrib(event->wd, file_list) == -1) {
			return -1;
		}
	} else if (event->mask & IN_MOVE_SELF) {
		if (delete_file_by_wd(file_list, event->wd) == -1) {
			// ! логирование !
			return -1;
		}
	} else if (event->mask & (IN_MODIFY)) {
		if (file_in_modify(event->wd, file_list) == -1) {
			return -1;
		}
	}
	free(event);
	return 0;
}

