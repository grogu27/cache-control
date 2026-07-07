#ifndef CACHE_CONTROL_MEDIATOR_H
#define CACHE_CONTROL_MEDIATOR_H

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "../queue/queue.h"
#include <sys/stat.h>
#include "../list/list.h"

#define SEND_FILE "/snd_queue"
#define RECV_FILE "/rcv_queue"
#define CLEAR_TERMINAL "\033[2J\033[H"
#define RED "\033[31m"
#define DASHES "---------------------------\n"
#define N 255
#define MAX_TOK 2
#define PATH_LEN 255
#define COMMAND_LEN 10
#define PATH_BUFF_SIZE 255
#define PATH_LOG_FILE "/var/cache_control_mediator.log"
#define POLITIC_LEN 15

typedef struct{
    char command[COMMAND_LEN];
    char path_to_file[PATH_LEN];
} command_t;

typedef struct {
    int status; // 0 - ok, 1 - not found
    int last_msg; //0-last 1 have more
    struct file f; // для операций status, status <file>
} reply_t;

typedef struct{
    mqd_t fds[2];
} mqfd_t;

enum pol { P_KEEP = 1, P_EVICT, P_MONITOR };

typedef enum {
    CMD_EXIT,
    CMD_RELOAD,
    CMD_EVICT,
    CMD_CACHE,
    CMD_STATUS,
    CMD_CLEAR,
    CMD_UNKNOWN
} command_type_t;

int read_string(char *str);
void clear_stdin();
void print_menu();
int get_command(char *str, command_t *command, command_type_t *op);
int print_answer(const char *const answer, const command_t *const ptr_command, const command_type_t op);
int split_tokens(char *str, char *tokens[], const char *delim, size_t *count_tok);
mqfd_t open_queue(const char *send_path, const char *recv_path, const int send_flags, const int recv_flags);
int set_command(const char *str, command_t *command, const char *path, const command_type_t op);
command_type_t get_command_type(const char *cmd);
int get_politic_str(reply_t *replay, char *str);
#endif