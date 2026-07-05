#include "../../include/list/list.h"

// инициализация статистики файла
// возращает:	указатель на инициализированную структуру - успех
//		NULL - ошибка
struct file_stat *init_file_stat(off_t s, int np, int npc, int pc,
				 struct timespec *la)
{
	struct file_stat *fs = malloc(sizeof(struct file_stat));
	if (fs == NULL) {
		// ! логирование !
		return NULL;
	}
	if (la == NULL) {
		// ! логирование !
		return NULL;
	}
	fs->size = s;
	fs->number_of_pages = np;
	fs->number_of_pages_cache = npc;
	fs->percent_cache = pc;
	memcpy(&(fs->last_access), la, sizeof(struct timespec));

	return fs;
}

// инициализация пустой статистики файла
// необходима тот случай, если мы хотим безопасно выделить память под структуру, но пока не заполнять поля
// возвращает:	указатель на инициализированную структуру - успех
// 		NULL - ошибка
struct file_stat *init_empty_stat()
{
	struct file_stat *fs = malloc(sizeof(struct file_stat));
	if (fs == NULL) {
		// ! логирование !
		return NULL;
	}

	fs->size = -1;
	fs->number_of_pages = -1;
	fs->number_of_pages_cache = -1;
	fs->percent_cache = -1;
	struct timespec la;
	la.tv_sec = -1;
	la.tv_nsec = -1;
	memcpy(&(fs->last_access), &la, sizeof(struct timespec));
}

// инициализация структуры файла
// возращает: 	указатель на инициализированную структуру - успех
// 		NULL - ошибка
struct file *init_file(char *pb, int p, int a)
{
	struct file *f = malloc(sizeof(struct file));
	if (f == NULL) {
		// ! логирование !
		return NULL;
	}
	int strlen_pb = strlen(pb);
	if (strlen_pb == 0) {
		// ! логирование !
		return NULL;
	}
	if (strlen_pb >= PATH_BUFF_SIZE) {
		strncpy(f->path, pb, (PATH_BUFF_SIZE - 1));
	} else {
		strcpy(f->path, pb);
	}
	f->stat = NULL;
	f->politic = p;
	f->age = a;
	f->fd = -1;
	f->vmem_mapping = NULL;
	return f;
}

// добавление статистики к файлу
// возращает: -1 - ошибка, 0 - успех
int add_file_stat(struct file *f, struct file_stat *fs)
{
	if (fs == NULL) {
		// ! логирование !
		return -1;
	}

	f->stat = fs;
	return 0;
}

// инициализация вершины
// возращает: -1 - ошибка, 0 - успех
struct node *init_node(struct file *f)
{
	if (f == NULL) {
		// ! логирование !
		return NULL;
	}
	struct node *n = malloc(sizeof(struct node));
	if (n == NULL) {
		// ! логирование !
		return NULL;
	}
	n->nd_file = f;
	n->nd_next = NULL;
	return n;
}

// инициализация списка
struct list *init_list()
{
	struct list *l = malloc(sizeof(struct list));
	if (l == NULL) {
		// ! логирование !
		return NULL;
	}
	l->start = NULL;
	l->length = 0;
	l->evict_files = 0;
	return l;
}

// добавление файла в список
// возращает: -1 - ошибка, 0 - успех
int add_file(struct list *l, struct file *f)
{
	if (l == NULL) {
		// ! логирование !
		return -1;
	}
	struct node *new_node = init_node(f);
	if (new_node == NULL) {
		// ! логирование !
		return -1;
	}

	new_node->nd_next = l->start;
	l->start = new_node;
	l->length++;

	if (f->politic == EVICT) {
		l->evict_files++;
	}

	return 0;
}

// поиск файла по его пути в списке
// возвращает:
// 	NULL - в случае ошибки или отсуствия файла
// 	указатель на файл - в случае успеха
struct file *find_file_by_path(struct list *l, char *path)
{
	if (l == NULL) {
		// ! логирование !
		return NULL;
	}
	if (l->start == NULL) {
		// ! логирование !
		return NULL;
	}

	struct node *search_node = l->start;
	while (search_node != NULL) {
		if (strcmp(search_node->nd_file->path, path) == 0) {
			return search_node->nd_file;
		}
		search_node = search_node->nd_next;
	}

	return NULL;
}

// удаляет вершину по пути файла в списке
// возвращает: -1 - ошибка, 0 - успех
int delete_file_by_path(struct list *l, char *path)
{
	if (l == NULL) {
		// ! логирование !
		return -1;
	}
	if (l->start == NULL) {
		// ! логирование !
		return -1;
	}

	struct node *delete_node;
	// если искомая врешина оказалась первой
	if (strcmp(l->start->nd_file->path, path) == 0) {
		delete_node = l->start;
		l->start = l->start->nd_next;
		free(delete_node->nd_file->stat);
		free(delete_node->nd_file);
		free(delete_node);
		l->length--;
	} else {
		struct node *cur_node = l->start;
		while ((cur_node->nd_next != NULL) &&
		       (strcmp(cur_node->nd_next->nd_file->path, path) != 0)) {
			cur_node = cur_node->nd_next;
		}
		delete_node = cur_node->nd_next;
		cur_node->nd_next = cur_node->nd_next->nd_next;
		free(delete_node->nd_file->stat);
		free(delete_node->nd_file);
		free(delete_node);
		l->length--;
	}

	return 0;
}

// удаление списка
// возвращает: -1 - ошибка, 0 - успех
int free_list(struct list *l)
{
	if (l == NULL) {
		// ! логирование !
		return -1;
	}

	struct node *cur_node = l->start;
	while (cur_node != NULL) {
		struct node *delete_node = cur_node;
		cur_node = cur_node->nd_next;
		free(delete_node->nd_file->stat);
		free(delete_node->nd_file);
		free(delete_node);
	}
	free(l);

	return 0;
}
