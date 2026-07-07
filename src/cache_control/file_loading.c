#include "../../include/cache_control/file_loading.h"

// получает файловый дескриптор и указатель на адрес файла в виртуальной памяти
// полученные значения записывает в структуру file
// возвращает: -1 - ошибка, 0 - успех
int map_file(struct file *myfile)
{
	if (myfile == NULL) {
		// ! логирование !
		return -1;
	}
	if (myfile->stat == NULL) {
		// ! лоигрование !
		return -1;
	}

	int fd = open(myfile->path, O_RDONLY);
	if (fd == -1) {
		int err = errno;
		// ! логирование, вывести значение errno!
		return -1;
	}
	myfile->fd = fd;

	if (myfile->stat->size <= 0) {
		// ! логирование !
		close(myfile->fd);
		return -1;
	}
	size_t file_size = (size_t)(myfile->stat->size);

	myfile->vmem_mapping =
		mmap(NULL, file_size, PROT_READ, MAP_SHARED, fd, 0);
	if (myfile->vmem_mapping == MAP_FAILED) {
		int err = errno;
		// ! логирование, вывести значение errno!
		close(myfile->fd);
		return -1;
	}

	if (close(myfile->fd) == -1) {
		int err = errno;
		// ! логирование, вывести значение errno !
		return -1;
	}
	myfile->fd = -1; // !!!

	return 0;
}

// загружает файл из виртуальной памяти в опреативную посредством чтения
// если файл слишком большой, читает его по кускам
// возвращает: -1 - ошибка, 0 - успех
// * ПРЕДУПРЕЖДЕНИЕ * : прежде чем загружать файл его необходимо замапать
// (вызвать map_file())
int load_file(struct file *myfile)
{
	if (myfile == NULL) {
		// ! логирование !
		return -1;
	}
	if (myfile->stat == NULL) {
		// ! лоигрование !
		return -1;
	}

	if (myfile->vmem_mapping == NULL) {
		// ! логирование !
		return -1;
	}

	int page_size = getpagesize();
	if (myfile->stat->size <= 0) {
		// ! логирование !
		return -1;
	}
	size_t file_size = (size_t)myfile->stat->size;
	for (size_t i = 0; i < file_size; i += page_size) {
		volatile char dummy = ((char *)(myfile->vmem_mapping))[i];
	}

	return 0;
}

// выгружает файл из виртуальной памяти,
// тем самым высвобождая его из оперативной памяти
// возвращает: -1 - ошибка, 0 - успех
int unmap_file(struct file *myfile)
{
	if (myfile == NULL) {
		// ! логирование !
		return -1;
	}
	if (myfile->stat == NULL) {
		// ! лоигрование !
		return -1;
	}

	if (myfile->vmem_mapping == NULL) {
		// ! логирование !
		return -1;
	}
	if (myfile->stat->size <= 0) {
		// ! логирование !
		return -1;
	}
	size_t size = (size_t)(myfile->stat->size);
	int recv = munmap(myfile->vmem_mapping, size);
	if (recv == -1) {
		int err = errno;
		// ! логирование, вывести знаение errno!
		return -1;
	}
	myfile->vmem_mapping = NULL; // !!!

	return 0;
}

