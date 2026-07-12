#include "../../include/cache_control_mediator/cache_control_mediator.h"
#include <time.h>
// Создать очередь сообщений.
// Демон должен уметь выполнять из него команды:
// status [file] – вывести статистику по файлу или по всем отслеживаемым.
// cache <file> – принудительно подгрузить указанный файл в кэш (игнорируя политику).
// evict <file> – принудительно выгрузить файл.
// reload – перечитать конфигурацию.
// exit – корректно завершить демон.
// clear - очистить терминал.
int main()
{
	char str[PATH_LEN + COMMAND_LEN] = { 0 };
	command_t command;
	command_t *ptr_command = &command;
	char *recv_msg = NULL;
	command_type_t op;
	mqfd_t fds = open_queue(SEND_FILE, RECV_FILE, O_WRONLY, O_RDONLY);
	reply_t *ptr_recv = NULL;

	while (1) {
		print_menu();
		int res = read_string(str);
		if (res == 1) {
			printf("Input error\n");
			clear_stdin();
			sleep(2);
			printf(CLEAR_TERMINAL);
			continue;
		}
		if (res == 2) {
			printf("The string is too long\n");
			clear_stdin();
			sleep(2);
			printf(CLEAR_TERMINAL);
			continue;
		}
		memset(&command, 0, sizeof(command));
		int res_get_command = get_command(str, &command, &op);
		if (res_get_command == 1) {
			clear_stdin();
			sleep(2);
			printf(CLEAR_TERMINAL);
			continue;
		} else if (res_get_command == 2) {
			printf(CLEAR_TERMINAL);
			continue;
		}
		printf("%s:%s\n", command.command, command.path_to_file);
		int res_send = send_msg(fds.fds[1], (char *)ptr_command,
					sizeof(command_t));
		if (res_send == -1) {
			printf("Error send_msg\n");
			exit(EXIT_FAILURE);
		}

		do {
			recv_msg = malloc(sizeof(reply_t));
			if (recv_msg == NULL) {
				perror("malloc");
				exit(EXIT_FAILURE);
			}

			if (recive_msg_for_mediator(fds.fds[0], recv_msg,
						    sizeof(reply_t)) == NULL) {
				printf("Error recive_msg\n");
				free(recv_msg);
				exit(EXIT_FAILURE);
			}

			int res_print_answer =
				print_answer(recv_msg, ptr_command, op);

			if (res_print_answer == 1) {
				perror("fopen");
				free(recv_msg);
				exit(EXIT_FAILURE);
			}

			if (res_print_answer == 2) {
				fprintf(stderr, "File not found\n");
				free(recv_msg);
				exit(EXIT_FAILURE);
			}
			if (res_print_answer == 3) {
				fprintf(stderr, "Get politic error\n");
				free(recv_msg);
				exit(EXIT_FAILURE);
			}

			ptr_recv = (reply_t *)recv_msg;

			int last_msg = ptr_recv->last_msg;

			free(recv_msg);

			if (last_msg == 0)
				break;

		} while (1);
	}
	mq_close(fds.fds[0]);
	mq_close(fds.fds[1]);
	return 0;
}

int print_answer(const char *const answer, const command_t *const ptr_command,
		 const command_type_t op)
{
	reply_t *rep = (reply_t *)answer;
	char politic[POLITIC_LEN];
	/* демон сообщил, что файл не найден */
	if (rep->status == 1) {
		printf("-------------------------------------------------------\n");
		printf("File \"%s\" not found\n", ptr_command->path_to_file);
		printf("-------------------------------------------------------\n");

		return 2;
	}

	switch (op) {
	case CMD_EXIT:
	case CMD_RELOAD:
	case CMD_CACHE:
	case CMD_EVICT:

		printf("-------------------------------------------------------\n");
		printf("%s %s: %s\n", ptr_command->command,
		       ptr_command->path_to_file, "ok");
		printf("-------------------------------------------------------\n");

		break;

	case CMD_STATUS: {
		if (get_politic_str(rep, politic)) {
			return 3;
		}
		printf("-------------------------------------------------------\n");
		printf("File: %s\n", rep->f.path);
		printf("Policy: %s\n", politic);
		printf("Age: %d\n", rep->f.age);

		printf("Size: %ld\n", (long)rep->f.size);
		printf("Pages: %d\n", rep->f.number_of_pages);
		printf("Cached pages: %d\n", rep->f.number_of_pages_cache);
		printf("Cache: %d\n", rep->f.percent_cache);
		struct tm tm_result;
		char time_buf[64];
		localtime_r(&(rep->f.last_access.tv_sec), &tm_result);
		strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S",
			 &tm_result);
		printf("Last access: %s.%09ld\n", time_buf,
		       rep->f.last_access.tv_nsec);

		printf("-------------------------------------------------------\n");

		break;
	}

	default:
		return 1;
	}

	return 0;
}

command_type_t get_command_type(const char *cmd)
{
	if (strcmp(cmd, "exit") == 0)
		return CMD_EXIT;
	if (strcmp(cmd, "reload") == 0)
		return CMD_RELOAD;
	if (strcmp(cmd, "evict") == 0)
		return CMD_EVICT;
	if (strcmp(cmd, "cache") == 0)
		return CMD_CACHE;
	if (strcmp(cmd, "status") == 0)
		return CMD_STATUS;
	if (strcmp(cmd, "clear") == 0)
		return CMD_CLEAR;
	return CMD_UNKNOWN;
}

int get_politic_str(reply_t *replay, char *str)
{
	if (replay->f.politic == P_KEEP) {
		strncpy(str, "keep", POLITIC_LEN);
		return 0;
	}
	if (replay->f.politic == P_EVICT) {
		strncpy(str, "evict", POLITIC_LEN);
		return 0;
	}
	if (replay->f.politic == P_MONITOR) {
		strncpy(str, "monitor", POLITIC_LEN);
		return 0;
	}
	return 1;
}

int read_string(char *str)
{
	if (fgets(str, PATH_LEN + COMMAND_LEN, stdin) == NULL) {
		perror("fgets");
		return 1;
	}
	char *ptr_newline = strchr(str, '\n');
	if (ptr_newline != NULL) {
		*ptr_newline = '\0';
		return 0;
	}
	if (strlen(str) == (PATH_LEN + COMMAND_LEN))
		return 0;

	clear_stdin();
	return 2;
}

void clear_stdin()
{
	int c;
	while ((c = getchar()) != '\n' && c != EOF) {
	}
}

void print_menu()
{
	printf("----------------------------------------------\n");
	printf("Cache control mediator\n\n");
	printf("status [file] – вывести статистику по файлу или по всем отслеживаемым.\n");
	printf("cache <file> – принудительно подгрузить указанный файл в кэш (игнорируя политику).\n");
	printf("evict <file> – принудительно выгрузить файл.\n");
	printf("reload – перечитать конфигурацию.\n");
	printf("exit – корректно завершить демон.\n");
	printf("clear – очитска терминала.\n");
	printf("----------------------------------------------\n");
}

int get_command(char *str, command_t *command, command_type_t *op)
{
	char *tokens[MAX_TOK];
	size_t count;
	struct stat st;
	if (split_tokens(str, tokens, " ", &count)) {
		fprintf(stderr, "Syntax error\n");
		return 1;
	}
	if (count != 1 && count != 2)
		return 1;
	//exit, reload
	if (count == 1) {
		*op = get_command_type(tokens[0]);
		if (*op == CMD_UNKNOWN)
			return 1;
		if (*op == CMD_CLEAR)
			return 2;
		if (set_command(tokens[0], command, "\0", *op)) {
			fprintf(stderr, "Syntax error\n");
			return 1;
		}

	} else if (count == 2) {
		*op = get_command_type(tokens[0]);
		if (*op == CMD_STATUS && strcmp(tokens[1], "all") == 0) {
			if (set_command(tokens[0], command, tokens[1], *op)) {
				return 1;
			}
		} else {
			if (stat(tokens[1], &st) == -1) {
				perror("Something is wrong file");
				return 1;
			}
			if (!S_ISREG(st.st_mode)) {
				fprintf(stderr, "Is not a regular file\n");
				return 1;
			}
			if (*op == CMD_UNKNOWN)
				return 1;
			if (set_command(tokens[0], command, tokens[1], *op)) {
				return 1;
			}
		}
	}

	return 0;
}

int set_command(const char *str, command_t *command, const char *path,
		const command_type_t op)
{
	if (op == CMD_EXIT) {
		strncpy(command->command, str, COMMAND_LEN);
		memcpy(command->path_to_file, path, PATH_LEN - 1);
		return 0;
	} else if (op == CMD_RELOAD) {
		strncpy(command->command, str, COMMAND_LEN);
		memcpy(command->path_to_file, path, PATH_LEN - 1);
		return 0;
	} else if (op == CMD_STATUS) {
		strncpy(command->command, str, COMMAND_LEN);
		memcpy(command->path_to_file, path, PATH_LEN - 1);
		return 0;
	} else if (op == CMD_EVICT) {
		strncpy(command->command, str, COMMAND_LEN);
		memcpy(command->path_to_file, path, PATH_LEN - 1);
		return 0;
	} else if (op == CMD_CACHE) {
		strncpy(command->command, str, COMMAND_LEN);
		memcpy(command->path_to_file, path, PATH_LEN - 1);
		return 0;
	}
	return 1;
}

mqfd_t open_queue(const char *send_path, const char *recv_path,
		  const int send_flags, const int recv_flags)
{
	mqfd_t fds;
	mqd_t send_fd = mq_open(send_path, send_flags);
	if (send_fd < 0) {
		perror("mq_open send");
		exit(EXIT_FAILURE);
	}
	mqd_t recv_fd = mq_open(recv_path, recv_flags);
	if (recv_fd < 0) {
		perror("mq_open recv");
		exit(EXIT_FAILURE);
	}
	fds.fds[0] = recv_fd;
	fds.fds[1] = send_fd;
	return fds;
}

int split_tokens(char *str, char *tokens[], const char *delim,
		 size_t *count_tok)
{
	*count_tok = 0;
	char *tok = strtok(str, delim);

	while (tok) {
		tokens[(*count_tok)++] = tok;
		tok = strtok(NULL, delim);
		if (*count_tok > MAX_TOK)
			return 1;
	}
	return 0;
}