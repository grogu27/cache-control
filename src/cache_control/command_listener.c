#include "../../include/cache_control/command_listener.h"

int status_all(struct node* mynode, struct replay_t snd_msg, mqd_t snd_queue)
{
	snd_msg.status=0;
	while(mynode!=NULL)
	{
		if(mynode->nd_next!=NULL)
			snd_msg.last_msg=1;
		else
			snd_msg.last_msg=0;
		memcpy(&snd_msg.f, mynode->nd_file, sizeof(struct file));
		if(send_msg(snd_queue, (char *)&snd_msg, sizeof(struct replay_t))<0){
			return 0;}
		mynode=mynode->nd_next;
	}
	return 0;
}

int status_spec(struct file* myfile, struct replay_t snd_msg, mqd_t snd_queue)
{
	snd_msg.status=0;
	snd_msg.last_msg=0;
	snd_msg.f=*myfile;
	if(send_msg(snd_queue, (char *)&snd_msg, sizeof(struct replay_t))<0){
		return 0;}
	return 0;
}

int do_cache(struct file* myfile, struct replay_t snd_msg, mqd_t snd_queue)
{
	if (myfile->vmem_mapping == NULL){
                if (map_file(myfile) == -1){
                        return 0;
                }
        }
	if(load_file(myfile)<0){
		return 0;}
	char log_msg[LOG_BUFF];
        snprintf(log_msg, sizeof(log_msg), "File %s load in cache", myfile->path);
	logging(LOG_INFO, log_msg);
	snd_msg.status=0;
	if(send_msg(snd_queue, (char *)&snd_msg, sizeof(struct replay_t))<0){
		return 0;}
	return 0;
}

int do_evict(struct file* myfile, struct replay_t snd_msg, mqd_t snd_queue)
{
	if(myfile->vmem_mapping == NULL){
                snd_msg.status=0;
                if(send_msg(snd_queue, (char *)&snd_msg, sizeof(struct replay_t))<0){
                        return 0;}
                return 0;
        }
	if(unmap_file(myfile)<0)
		return 0;
	char log_msg[LOG_BUFF];
	snprintf(log_msg, sizeof(log_msg), "File %s unload from cache", myfile->path);
	logging(LOG_INFO, log_msg);
	snd_msg.status=0;
	if(send_msg(snd_queue, (char *)&snd_msg, sizeof(struct replay_t))<0){
		return 0;}
	return 0;
}
int do_reload(struct cache_control* cc, struct replay_t snd_msg, mqd_t snd_queue)
{
	cc->reload_flag = 1;
	logging(LOG_INFO, "File config reread");
	snd_msg.status=0;
	if(send_msg(snd_queue, (char *)&snd_msg, sizeof(struct replay_t))<0){
		return 0;}
	return 0;
}

int do_exit(struct cache_control* cc, struct replay_t snd_msg, mqd_t rcv_queue, mqd_t snd_queue)
{
	logging(LOG_INFO, "Start stoping cache control");
	snd_msg.status=0;
	if(send_msg(snd_queue, (char *)&snd_msg, sizeof(struct replay_t))<0){
		return 0;}
        mq_close(rcv_queue);
        mq_close(snd_queue);
        cc->shutdown_flag=1;
	return 0;
}

int command_listner(void* args)
{
	struct cache_control* cc = (struct cache_control*)args;
	mqd_t rcv_queue, snd_queue;
	struct mediator_command* command = malloc(sizeof(struct mediator_command));
	struct node* mynode;
	struct file* myfile;
	struct replay_t snd_msg;
	rcv_queue = create_queue("/snd_queue", O_CREAT|O_RDONLY, sizeof(struct mediator_command));
	if(rcv_queue==-1)
	{
		return -1;
	}
	else
	{
		logging(LOG_INFO, "Open recive queue from mediator");
	}
	snd_queue = create_queue("/rcv_queue", O_CREAT|O_WRONLY, sizeof(struct replay_t));
	if(snd_queue==-1)
	{
		return -1;
	}
	else
	{
		logging(LOG_INFO, "Open send queue to mediator");
	}
	while(1)
	{
		if(recive_msg(rcv_queue, (char *)command, sizeof(struct mediator_command))<0)
		{
			return -1;
		}
		memset(&snd_msg, 0, sizeof(struct replay_t));
		mynode=cc->file_list->start;
		if(strcmp(command->com, "status")==0 && strcmp(command->path_to_file, "all")==0)
                {
                        status_all(mynode, snd_msg, snd_queue);
                        continue;
                }
		if(strcmp(command->com, "reload")==0)
                {
                        do_reload(cc, snd_msg, snd_queue);
                        continue;
                }
		else if(strcmp(command->com, "exit")==0)
                {
                        do_exit(cc, snd_msg, rcv_queue, snd_queue);
                        free(command);
                        break;
                }
		if(command->path_to_file[0] == '\0')
		{
			snd_msg.status=1;
			if(send_msg(snd_queue, (char *)&snd_msg, sizeof(struct replay_t))<0){
				return -1;}
		}
		myfile=find_file_by_path(cc->file_list, command->path_to_file);
		if(myfile==NULL)
		{
			snd_msg.status=1;
                        if(send_msg(snd_queue, (char *)&snd_msg, sizeof(struct replay_t))<0){
                        	return -1;}
                }
		if(strcmp(command->com, "status")==0)
		{
			status_spec(myfile, snd_msg, snd_queue);
			continue;
		}
		if(strcmp(command->com, "cache")==0)
		{
			do_cache(myfile, snd_msg, snd_queue);
			continue;
		}
		if(strcmp(command->com, "evict")==0)
		{
			do_evict(myfile, snd_msg, snd_queue);
			continue;
		}
	}
	return 0;
}
