#include "../../include/queue/queue.h"
#include "../../include/log/log.h"

/*
Функция создает очередь сообщений с параметрами которые передаются через вызов
file_desc пааметр имени очереди указывающийся через /
flags параметр для создания O_CREAT или подключения к очереди O_RDONLY, O_WRONLY
size_msg размер сообщений, есть 2 заранее созданных параметра SIZE_RCV для приема сообщений от медиатора и SIZE_SND для отправки сообщений медиатору
*/
mqd_t create_queue(char *file_desc, int flags, int size_msg)
{
	if(size_msg>MAX_SIZE_MSG)
	{
		logging(LOG_WARNING, "Error size message for queue very high. This parametr will be set 8190", NULL);
		size_msg=CHANGE_SIZE_MSG;
	}
	struct mq_attr *queue;
	mqd_t msg_queue;
	queue = malloc(sizeof(struct mq_attr));
	if(queue==NULL)
	{
		logging(LOG_ERR, "Error src queue", strerror(errno));
		return -1;;
	}
	queue->mq_maxmsg = 10;
	queue->mq_msgsize = size_msg;
	msg_queue = mq_open(file_desc, flags, 0660, queue);
	if(msg_queue == (mqd_t) -1)
	{
		logging(LOG_ERR, "Error open queue file descriptor: ", strerror(errno));
		free(queue);
		return -1;
	}
	logging(LOG_INFO, "Create message queue!", NULL);
	free(queue);
	return msg_queue;
}

/*
Функция приема команд из очереди от медиатора
Сделал универсальной, если потребуется создавать дополнительные очереи
*/
char* recive_msg(mqd_t queue_fd, char* rcv_msg, int size_msg)
{
	if(size_msg>MAX_SIZE_MSG)
	{
		logging(LOG_WARNING, "Error size message for recive from queue very high. This parametr will be set 8190", NULL);
		size_msg=CHANGE_SIZE_MSG;
	}
	if(mq_receive(queue_fd, rcv_msg, size_msg, 0)<0)
	{
		logging(LOG_ERR, "Error recive message: ", strerror(errno));
		return NULL;
	}
	return rcv_msg;
}

/*
Функция отправки сообщения в очередь для медиатора
Сделал универсальной, если потребуется создавать дополнительные очереи
*/
int send_msg(mqd_t queue_fd, char *snd_msg, int size_msg)
{
	if(size_msg>MAX_SIZE_MSG)
	{
		logging(LOG_WARNING, "Error size message for send in queue very high. This parametr will be set 8190", NULL);
		size_msg=CHANGE_SIZE_MSG;
	}
	if(mq_send(queue_fd, snd_msg, size_msg, 0)<0)
	{
		logging(LOG_ERR, "Error send message: ", strerror(errno));
		return -1;
	}
	return 0;
}
