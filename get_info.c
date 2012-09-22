#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char	get_state(const char* pid)
{
	int		fd;
	char*		path;
	char		state;
	int		nread;
	unsigned int	nb_space;

	path = malloc((unsigned int)strlen("/proc//stat") + (unsigned int)strlen(pid) + 1);

	strcpy(path, "/proc/");
	strcat(path, pid);
	strcat(path, "/stat");

	fd = open(path, O_RDONLY);
	free(path);

	if (fd < 0)
		return 0;
	nb_space = 0;
	while (nb_space < 2)
	{
		nread = read(fd, &state, 1);
		if (nread != 1)
			return 0;
		if (state == ' ')
			nb_space = nb_space + 1;
	}
	nread = read(fd, &state, 1);
	if (nread != 1)
		return 0;
	close(fd);
	return state;
}

unsigned int	get_proc_usage(const char* pid)
{
	int		fd;
	char*		path;
	unsigned int	proc_usage;

	path = malloc(strlen("/proc//stat") + strlen(pid) + 1);

	strcpy(path, "/proc/");
	strcat(path, pid);
	strcat(path, "/stat");

	fd = open(path, O_RDONLY);
	free(path);

	if (fd < 0)
		return 0;
	proc_usage = 0;
	close(fd);
	return proc_usage;
}

unsigned int	get_mem(const char* pid)
{
	int		fd;
	char*		path;
	unsigned int	mem_usage;

	path = malloc(strlen("/proc//meminfo") + strlen(pid) + 1);

	strcpy(path, "/proc/");
	strcat(path, pid);
	strcat(path, "/meminfo");

	fd = open(path, O_RDONLY);
	free(path);

	if (fd < 0)
		return 0;
	mem_usage = 0;
	close(fd);
	return mem_usage;
}

