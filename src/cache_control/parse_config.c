#include "../../include/cache_control/parse_config.h"

static int is_number(const char *s)
{
	if (!s || !*s)
		return 1;
	if (*s == '0')
		return 1;

	for (int i = 0; s[i]; i++)
		if (s[i] < '0' || s[i] > '9')
			return 1;

	return 0;
}

static int split_tokens(char *str, char *tokens[], const char *delim)
{
	int count = 0;
	char *tok = strtok(str, delim);

	while (tok) {
		tokens[count++] = tok;
		tok = strtok(NULL, delim);

		if (count > 3)
			return -1;
	}

	return count;
}

static int split_eq(char *str, char **left, char **right)
{
	char *eq = strchr(str, '=');
	if (!eq)
		return 1;

	*eq = '\0';
	*left = str;
	*right = eq + 1;

	return 0;
}

static int parse_interval(char *line, int *interval)
{
	char *tok[2];

	if (split_eq(line, &tok[0], &tok[1]))
		return -1;

	if (strcmp(tok[0], "interval") != 0)
		return -1;

	if (is_number(tok[1]))
		return -1;
	*interval = atoi(tok[1]);

	return 0;
}

static int parse_policy(const char *str)
{
	if (!strcmp(str, "keep"))
		return KEEP;
	if (!strcmp(str, "evict"))
		return EVICT;
	if (!strcmp(str, "monitor"))
		return MONITOR;

	return -1;
}

static int parse_age(char *s)
{
	char *a[2];

	if (split_eq(s, &a[0], &a[1]))
		return -1;

	if (strcmp(a[0], "age") != 0)
		return -1;

	if (is_number(a[1]))
		return -1;

	return atoi(a[1]);
}

static int check_pattern(const char *path)
{
	const char *slash = strrchr(path, '/');
	if (slash == NULL || slash == path)
		return 1;
	for (const char *p = path; p < slash; p++)
		if (*p == '*' || *p == '?')
			return 1;

	return 0;
}

static int parse_dir(const char *dir, const char *pattern, struct list *list,
		      int policy, int age)
{
	DIR *fd = opendir(dir);
	if (!fd)
		return 1; /* Каталог отсутствует или недоступен — пропускаем правило. Можно написать лог, но это не является ошибкой, и парсер продолжает работу*/

	struct dirent *entry;
	char path[PATH_MAX];

	while ((entry = readdir(fd))) {
		if (entry->d_name[0] == '.')
			continue;

		if (fnmatch(pattern, entry->d_name, 0) != 0)
			continue;

		snprintf(path, PATH_MAX, "%s/%s", dir, entry->d_name);

		if (find_file_by_path(list, path) != NULL){ // семантическая ошибка в конфигурационном файле (к одному и тому же файлу применяются разные политики).
			closedir(fd);
			return 2;
		}  
			
		struct file *f = init_file(path, policy, age);

		add_file(list, f);
	}

	closedir(fd);
	return 0;
}

/* ---------- main ----------
 * Разбирает конфиг.
 *
 * Параметры:
 *   config_path — путь к конфигурационному файлу;
 *   l           — список, в который добавляются файлы,
 *                 соответствующие правилам конфигурации;
 *   interval    — сюда записывается значение параметра interval
 *                 (если в конфигурации он отсутствует,
 *                 используется DEFAULT_INTERVAL).
 *
 * Возвращает:
 *   0 — успех;
 *   1 — ошибка открытия конфигурационного файла;
 *   2 — синтаксическая ошибка в конфигурационном файле.
 * 	 3 - семантическая ошибка в конфигурационном файле (к одному и тому же файлу применяются разные политики).
 */
int parse_config(const char *config_path, struct list *l, int *interval)
{
	FILE *fd = fopen(config_path, "r");
	if (!fd)
		return 1;
	char buff[SIZE_BUFF];
	char *ptr_buff = NULL;
	int policy;

	while (fgets(buff, sizeof(buff), fd)) {
		buff[strcspn(buff, "\n")] = 0;
		ptr_buff = buff;
		while (*ptr_buff == ' ' || *ptr_buff == '\t') {
			ptr_buff++;
		}

		if (*ptr_buff == '\0' || *ptr_buff == '#')
			continue;

		if (strchr(ptr_buff, '=') && *ptr_buff != '/') {
			if (parse_interval(ptr_buff, interval) != 0) {
				fclose(fd);
				return 2;
			}
			continue;
		}

		char *tokens[3];
		int count = split_tokens(ptr_buff, tokens, " ");

		if (count < 2 || count > 3) {
			fclose(fd);
			return 2;
		}
		if (tokens[0][0] != '/') {
			fclose(fd);
			return 2;
		}
		if (check_pattern(tokens[0])) {
			fclose(fd);
			return 2;
		}

		char *slash = strrchr(tokens[0], '/');
		if (!slash) {
			fclose(fd);
			return 2;
		}

		char dir[PATH_MAX];
		char pattern[PATH_BUFF_SIZE];

		size_t len = slash - tokens[0];

		strncpy(dir, tokens[0], len);
		dir[len] = '\0';

		strcpy(pattern, slash + 1);

		if ((policy = parse_policy(tokens[1])) == -1) {
			fclose(fd);
			return 2;
		}
		int age = 0;

		if (count == 3) {
			age = parse_age(tokens[2]);
			if (age < 0) {
				fclose(fd);
				return 2;
			}
		}

		int res_parse_dir = parse_dir(dir, pattern, l, policy, age);
		if(res_parse_dir == 2)
			return 3;
	}

	fclose(fd);
	return 0;
}
