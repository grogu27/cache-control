#include "../../include/cache_control/command_listener.h"
#include "../../include/cache_control/parse_config.h"

static void fill_file_info(struct file_info *dst, struct file *src,
			   struct cache_control *cc)
{
	memset(dst, 0, sizeof(*dst));
	pthread_mutex_lock(&(cc->mutex));
	if (src->vmem_mapping != NULL) {
		get_page_stat(src);
	} else {
		src->stat->number_of_pages_cache = 0;
		src->stat->percent_cache = 0;
	}
	pthread_mutex_unlock(&(cc->mutex));
	strcpy(dst->path, src->path);

	dst->politic = src->politic;
	dst->age = src->age;

	get_last_access(src->path, &src->stat->last_access);
	if (src->stat != NULL) {
		dst->size = src->stat->size;
		dst->number_of_pages = src->stat->number_of_pages;
		dst->number_of_pages_cache = src->stat->number_of_pages_cache;
		dst->percent_cache = src->stat->percent_cache;
		dst->last_access = src->stat->last_access;
	}
}

int status_all(struct node *mynode, struct replay_t snd_msg, mqd_t snd_queue,
	       struct cache_control *cc)
{
	snd_msg.status = 0;
	int count = 0;
	while (mynode != NULL) {
		count++;
		if (mynode->nd_next != NULL)
			snd_msg.last_msg = 1;
		else
			snd_msg.last_msg = 0;
		fill_file_info(&snd_msg.f, mynode->nd_file, cc);
		printf("[command_listener]Sending file: %s, last_msg=%d, status=%d\n",
		       snd_msg.f.path, snd_msg.last_msg, snd_msg.status);
		if (send_msg(snd_queue, (char *)&snd_msg,
			     sizeof(struct replay_t)) < 0) {
			return -1;
		}
		mynode = mynode->nd_next;
	}
	return 0;
}

int status_spec(struct file *myfile, struct replay_t snd_msg, mqd_t snd_queue,
		struct cache_control *cc)
{
	snd_msg.status = 0;
	snd_msg.last_msg = 0;
	fill_file_info(&snd_msg.f, myfile, cc);
	printf("[command_listener]Sending file: %s, last_msg=%d, status=%d\n",
	       snd_msg.f.path, snd_msg.last_msg, snd_msg.status);
	if (send_msg(snd_queue, (char *)&snd_msg, sizeof(struct replay_t)) <
	    0) {
		return -1;
	}
	return 0;
}

int do_cache(struct file *myfile, struct replay_t snd_msg, mqd_t snd_queue)
{
	if (myfile->vmem_mapping == NULL) {
		if (map_file(myfile) == -1) {
			return -1;
		}
	}
	if (load_file(myfile) < 0) {
		return -1;
	}

	char log_msg[280];
	snprintf(log_msg, sizeof(log_msg), "File %s unload in cache",
		 myfile->path);
	logging(LOG_INFO, log_msg);
	snd_msg.status = 0;
	printf("[command_listener]cache %s\n", myfile->path);
	if (send_msg(snd_queue, (char *)&snd_msg, sizeof(struct replay_t)) <
	    0) {
		return -1;
	}
	return 0;
}

int do_evict(struct file *myfile, struct replay_t snd_msg, mqd_t snd_queue,
	     struct cache_control *cc)
{
	if (myfile->vmem_mapping == NULL) {
		snd_msg.status = 0;
		snd_msg.last_msg = 0;
		if (send_msg(snd_queue, (char *)&snd_msg,
			     sizeof(struct replay_t)) < 0) {
			return -1;
		}
		return 0;
	}

	pthread_mutex_lock(&(cc->mutex));

	if (unmap_file(myfile) < 0) {
		char log_msg[512];
		snprintf(log_msg, sizeof(log_msg),
			 "File %s not unload from cache", myfile->path);
		logging(LOG_ERR, log_msg);
		pthread_mutex_unlock(&(cc->mutex));
		return -1;
	}
	pthread_mutex_unlock(&(cc->mutex));
	int fd = open(myfile->path, O_RDONLY);
	if (fd == -1) {
		return -1;
	}

	int ret_advise = posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);
	if (ret_advise != 0) {
		char buf[128];
		snprintf(buf, sizeof(buf), "posix_fadvise failed: %s",
			 strerror(ret_advise));
		logging(LOG_ERR, buf);
		printf("posix_fadvise failed: %s\n", strerror(ret_advise));
	}
	close(fd);
	printf("posix_fadvise ok\n");

	char log_msg[280];
	snprintf(log_msg, sizeof(log_msg), "File %s unload from cache",
		 myfile->path);
	logging(LOG_INFO, log_msg);

	snd_msg.status = 0;
	snd_msg.last_msg = 0;
	printf("[command_listener]evict %s\n", myfile->path);
	if (send_msg(snd_queue, (char *)&snd_msg, sizeof(struct replay_t)) <
	    0) {
		return -1;
	}
	return 0;
}

int do_reload(struct cache_control *cc, struct replay_t snd_msg,
	      mqd_t snd_queue)
{
	pthread_mutex_lock(&(cc->mutex));
	cc->reload_flag = 1;
	pthread_mutex_unlock(&(cc->mutex));
	logging(LOG_INFO, "File config reread");
	snd_msg.status = 0;
	if (send_msg(snd_queue, (char *)&snd_msg, sizeof(struct replay_t)) <
	    0) {
		return 0;
	}
	return 0;
}

int do_exit(struct cache_control *cc, struct replay_t snd_msg, mqd_t rcv_queue,
	    mqd_t snd_queue)
{
	logging(LOG_INFO, "Start stoping cache control");
	snd_msg.status = 0;
	printf("[command_listener]exit\n");
	if (send_msg(snd_queue, (char *)&snd_msg, sizeof(struct replay_t)) <
	    0) {
		return -1;
	}
	mq_close(rcv_queue);
	mq_close(snd_queue);
	pthread_mutex_lock(&(cc->mutex));
	cc->shutdown_flag = 1;
	pthread_mutex_unlock(&(cc->mutex));
	return 0;
}

int command_listner(void *args)
{
	struct cache_control *cc = (struct cache_control *)args;
	mqd_t rcv_queue, snd_queue;
	mq_unlink("/snd_queue");
	mq_unlink("/rcv_queue");
	struct mediator_command *command =
		malloc(sizeof(struct mediator_command));
	struct node *mynode;
	struct file *myfile;
	struct replay_t snd_msg;
	rcv_queue = create_queue("/snd_queue", O_CREAT | O_RDONLY,
				 sizeof(struct mediator_command));
	if (rcv_queue == -1) {
		return -1;
	} else {
		logging(LOG_INFO, "Open recive queue from mediator");
	}
	snd_queue = create_queue("/rcv_queue", O_CREAT | O_WRONLY,
				 sizeof(struct replay_t));
	if (snd_queue == -1) {
		return -1;
	} else {
		logging(LOG_INFO, "Open send queue to mediator");
	}
	while (!cc->shutdown_flag) {
		memset(command, 0, sizeof(struct mediator_command));
		if (recive_msg(rcv_queue, (char *)command,
			       sizeof(struct mediator_command)) < 0) {
			return -1;
		}
		if (command->com[0] == '\0') {
			continue;
		}

		memset(&snd_msg, 0, sizeof(struct replay_t));
		mynode = cc->file_list->start;
		if (strcmp(command->com, "status") == 0 &&
		    strcmp(command->path_to_file, "all") == 0) {
			char buf[512];
			snprintf(buf, sizeof(buf), "Command: %s, path_file: %s",
				 command->com, command->path_to_file);
			logging(LOG_INFO, buf);
			if (status_all(mynode, snd_msg, snd_queue, cc) == -1) {
				char buf[1024];
				snprintf(buf, sizeof(buf),
					 "Command: %s not complete",
					 command->com);
				logging(LOG_ERR, buf);
			}
			continue;
		}
		if (strcmp(command->com, "reload") == 0) {
			char buf[512];
			snprintf(buf, sizeof(buf), "Command: %s, path_file: %s",
				 command->com, command->path_to_file);
			logging(LOG_INFO, buf);
			if (do_reload(cc, snd_msg, snd_queue) == -1) {
				char buf[1024];
				snprintf(buf, sizeof(buf),
					 "Command: %s not complete",
					 command->com);
				logging(LOG_ERR, buf);
			}
			continue;
		}
		if (strcmp(command->com, "exit") == 0) {
			char buf[512];
			snprintf(buf, sizeof(buf), "Command: %s, path_file: %s",
				 command->com, command->path_to_file);
			logging(LOG_INFO, buf);
			if (do_exit(cc, snd_msg, rcv_queue, snd_queue) == -1) {
				char buf[1024];
				snprintf(buf, sizeof(buf),
					 "Command: %s not complete",
					 command->com);
				logging(LOG_ERR, buf);
			}
			free(command);
			break;
		}
		if (command->path_to_file[0] == '\0') {
			snd_msg.status = 1;
			if (send_msg(snd_queue, (char *)&snd_msg,
				     sizeof(struct replay_t)) < 0) {
				return -1;
			}
		}
		myfile =
			find_file_by_path(cc->file_list, command->path_to_file);
		if (myfile == NULL) {
			snd_msg.status = 1;
			if (send_msg(snd_queue, (char *)&snd_msg,
				     sizeof(struct replay_t)) < 0) {
				return -1;
			}
		}
		if (strcmp(command->com, "status") == 0) {
			char buf[1024];
			snprintf(buf, sizeof(buf), "Command: %s, path_file: %s",
				 command->com, command->path_to_file);
			logging(LOG_INFO, buf);
			if (status_spec(myfile, snd_msg, snd_queue, cc) == -1) {
				char buf[1024];
				snprintf(buf, sizeof(buf),
					 "Command: %s not complete",
					 command->com);
				logging(LOG_ERR, buf);
			}
			continue;
		} else if (strcmp(command->com, "cache") == 0) {
			char buf[1024];
			snprintf(buf, sizeof(buf), "Command: %s, path_file: %s",
				 command->com, command->path_to_file);
			logging(LOG_INFO, buf);
			if (do_cache(myfile, snd_msg, snd_queue) == -1) {
				char buf[1024];
				snprintf(buf, sizeof(buf),
					 "Command: %s not complete",
					 command->com);
				logging(LOG_ERR, buf);
			}
			continue;
		} else if (strcmp(command->com, "evict") == 0) {
			char buf[1024];
			snprintf(buf, sizeof(buf), "Command: %s, path_file: %s",
				 command->com, command->path_to_file);
			logging(LOG_INFO, buf);
			if (do_evict(myfile, snd_msg, snd_queue, cc) == -1) {
				char buf[1024];
				snprintf(buf, sizeof(buf),
					 "Command: %s not complete",
					 command->com);
				logging(LOG_ERR, buf);
			}
			continue;
		}
	}
	return 0;
}
