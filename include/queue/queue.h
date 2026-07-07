#ifndef QUEUE_H
#define QUEUE_H
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mqueue.h>

#define SIZE_RCV 1024
#define SIZE_SND 45
#define MAX_SIZE_MSG 8192
#define CHANGE_SIZE_MSG 8190

mqd_t create_queue(char *file_desc, int flags, int size_msg);
char* recive_msg(mqd_t queue, char *snd_msg, int size_msg);
int send_msg(mqd_t queue, char *snd_msg, int size_msg);

#endif
