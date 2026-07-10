#include "../../include/cache_control/file_statistics.h"

// возвращает: 	размер файла - успех
// 		-1 - ошибка
off_t get_file_size(char *path)
{
	struct stat st;
	if (path == NULL) {
		// ! логирование, указатель равен NULL !
		logging(LOG_ERR, "<get_file_size> path is NULL");
		return -1;
	}
	if (strlen(path) == 0) {
		// ! логирование, строка пуста !
		logging(LOG_ERR, "<get_file_size> path is empty");

		return -1;
	}
	if (stat(path, &st) == -1) {
		// ! логирование, вывести значение errno !
		logging(LOG_ERR, "<get_file_size> stat failed");
		return -1;
	}
	return st.st_size;
}

// возвращает: 	общаее колличетсво страниц, выделенных под файл - успех
// 		-1 - ошибка
int get_number_of_pages(off_t file_size)
{
	if (file_size <= 0) {
		logging(LOG_ERR,
			"<get_number_of_pages> file size is not positive");
		return -1;
	}
	size_t file_size_gnu = (size_t)file_size;
	int page_size = getpagesize();
	int number_of_pages = (file_size_gnu + page_size - 1) / page_size;
	return number_of_pages;
}

// возвращает: 	количество занытых страниц в памяти - успех
// 		-1 - ошибка
int get_number_of_pages_cache(void *vmem_mapping, off_t size,
			      int number_of_pages)
{
	if (number_of_pages <= 0) {
		// ! логирование !
		logging(LOG_ERR,
			"<get_number_of_pages_cache> number_of_pages is not positive");
		return -1;
	}
	if (vmem_mapping == NULL) {
		// ! логирование !
		logging(LOG_ERR,
			"<get_number_of_pages_cache> vmem_mapping is NULL");
		return -1;
	}
	unsigned char *vec = malloc(sizeof(unsigned char) * number_of_pages);
	if (vec == NULL) {
		// ! логирование, вывести значение errno!
		logging(LOG_ERR, "<get_number_of_pages_cache> malloc failed");
		return -1;
	}
	if (size <= 0) {
		// ! логирование !
		logging(LOG_ERR,
			"<get_number_of_pages_cache> size is not positive");
		free(vec);
		return -1;
	}
	int recv_mincore = mincore(vmem_mapping, (size_t)size, vec);
	if (recv_mincore == -1) {
		// ! логирование, вывести значение errno !
		logging(LOG_ERR, "<get_number_of_pages_cache> mincore failed");
		free(vec);
		return -1;
	}

	int number_of_pages_cache = 0;
	// проверяем, что последний бит каждой байта равен 1
	// если не равен страница не занята
	for (int i = 0; i < number_of_pages; i++) {
		uint8_t byte = vec[i];
		uint8_t last_bit = byte & 0x1;
		number_of_pages_cache += last_bit;
	}

	free(vec);
	return number_of_pages_cache;
}

// возвращает: 	процентное соотношение занятых страниц к общему количеству -
// успех 		-1 - ошибка
int get_percent_cache(int number_of_pages, int number_of_pages_cache)
{
	if (number_of_pages <= 0) {
		// ! логирование !
		logging(LOG_ERR,
			"<get_percent_cache> number_of_pages is not positive");
		return -1;
	}
	if (number_of_pages_cache < 0) {
		// ! логирование !
		logging(LOG_ERR,
			"<get_percent_cache> number_of_pages_cache is negative");
		return -1;
	}
	int percent_cache = (number_of_pages_cache * 100 / number_of_pages);
	return percent_cache;
}

// записывает в переменную last_access время последнего доступа
// возвращает: 0 - успех, -1 - ошибка
int get_last_access(char *path, struct timespec *last_access)
{
	if (path == NULL) {
		// ! логирование !
		logging(LOG_ERR, "<get_last_access> path is NULL");
		return -1;
	}
	if (strlen(path) == 0) {
		// ! логирование !
		logging(LOG_ERR, "<get_last_access> path is empty");
		return -1;
	}

	struct stat st;
	if (stat(path, &st) == -1) {
		// ! логирование, вывести значение errno!
		logging(LOG_ERR, "<get_last_access> stat failed");
		return -1;
	}
	memcpy(last_access, &st.st_atim, sizeof(struct timespec));
	return 0;
}

// собирает статистику о страницах в памяти
// возвращает: 0 - успех, -1 - ошибка
int get_page_stat(struct file *myfile)
{
	if (myfile == NULL) {
		// ! логирование !
		logging(LOG_ERR, "<get_page_stat> struct file* myfile is NULL");
		return -1;
	}
	if (myfile->stat == NULL) {
		// ! логирование !
		logging(LOG_ERR, "<get_page_stat> myfile->stat is NULL");
		return -1;
	}

	if (myfile->stat->size <= 0) {
		// ! логирование !
		logging(LOG_ERR, "<get_page_stat> file size is not positive");
		return -1;
	}
	int nop = get_number_of_pages(myfile->stat->size);
	if (nop == -1) {
		// ! логирование !
		logging(LOG_ERR, "<get_page_stat> get_number_of_pages failed");
		return -1;
	}
	int nopc = get_number_of_pages_cache(myfile->vmem_mapping,
					     myfile->stat->size, nop);
	if (nopc == -1) {
		// ! логирование !
		logging(LOG_ERR,
			"<get_page_stat> get_number_of_pages_cache failed");
		return -1;
	}
	int pc = get_percent_cache(nop, nopc);
	if (pc == -1) {
		// ! логирование !
		logging(LOG_ERR, "<get_page_stat> get_percent_cache failed");
		return -1;
	}

	myfile->stat->number_of_pages = nop;
	myfile->stat->number_of_pages_cache = nopc;
	myfile->stat->percent_cache = pc;

	return 0;
}
