#ifndef CACHECONTROL_LISTNER_H
#define CACHECONTROL_LISTNER_H

#include "file_loading.h"
#include "context.h"
#include "../queue/queue.h"
#include "../log/log.h"
#include "file_statistics.h"
#include "file_monitoring.h"
#include "../list/list.h"

#define COMMAND_LEN 10
#define PATH_FILE 255

struct file_info {
    char path[PATH_BUFF_SIZE];

    off_t size;
    int number_of_pages;
    int number_of_pages_cache;
    int percent_cache;
    struct timespec last_access;

    int politic;
    int age;
};
struct replay_t {
    int status;
    int last_msg;
    struct file_info f;
};

struct mediator_command{
	char com[COMMAND_LEN];
	char path_to_file[PATH_FILE];
};

int status_all(struct node* mynode, struct replay_t snd_msg, mqd_t snd_queue, struct cache_control *cc);
int status_spec(struct file* myfile, struct replay_t snd_msg, mqd_t snd_queue, struct cache_control *cc);
int do_cache(struct file* myfile, struct replay_t snd_msg, mqd_t snd_queue);
int do_evict(struct file* myfile, struct replay_t snd_msg, mqd_t snd_queue, struct cache_control* cc);
int do_reload(struct cache_control* cc, struct replay_t snd_msg, mqd_t snd_queue);
int do_exit(struct cache_control* cc, struct replay_t snd_msg, mqd_t rcv_queue, mqd_t snd_queue);
int command_listner(void* args);

#endif