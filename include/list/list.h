#pragma once

#ifndef LIST_H
#define LIST_H

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define PATH_BUFF_SIZE 255

struct file_stat {
	off_t size;
	int number_of_pages;
	int number_of_pages_cache;
	int percent_cache;
	struct timespec last_access;
};

struct file {
	char path[PATH_BUFF_SIZE];
	struct file_stat* stat;
	int politic;
	int age;
	int fd;
	void* vmem_mapping;
};

struct node {
	struct file* nd_file;
	struct node* nd_next;
};

struct list {
	struct node* start;
	int length;
};

struct file_stat* init_file_stat(off_t s, int np, int npc, int pc, struct timespec* la);
struct file* init_file(char* pb, int p, int a);
int add_file_stat(struct file* f, struct file_stat* fs);
struct node* init_node(struct file* f);
struct list* init_list();
int add_file(struct list* l, struct file* f);
struct file* find_file_by_path(struct list* l, char* path);
int delete_file_by_path(struct list* l, char* path);
int free_list(struct list* l);

#endif
