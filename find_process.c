#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termcap.h>
#include <unistd.h>

#include "get_info.h"
#include "string.h"
#include "types.h"

extern int	gl_run;

typedef struct	s_list_proc
{
	char*			pid;
	struct s_list_proc*	next;
}t_list_proc;

typedef struct	s_process
{
	char	pid[6];
	char*	name;
}t_process;

void	remplace_char(char* str, char src, char dst)
{
	int	count;

	count = 0;
	while (str[count] != '\0')
	{
		if (str[count] == src)
			str[count] = dst;
		count = count + 1;
	}
}

int	check_process(char* name_dir, const char* name_process)
{
	char*	path;
	char*	name_process_in_dir;
	int	ret;
	int	fd;

	ret = 0;
	path = malloc(sizeof(char) * (strlen("/proc//status") + strlen(name_dir) + 1));
	name_process_in_dir = malloc(sizeof(char) * (strlen(name_process) + 3)); //+ 3

	if (path == 0 || name_process_in_dir == 0)
		ret = fatal(2, "Can't allocate memory (malloc fail)\n", -1);
	else
	{
		strcpy(path, "/proc/");
		strcat(path, name_dir);
		strcat(path, "/status");
//		printf("path = %s\n", path);

		fd = open(path, O_RDONLY);
		if (fd == -1)
		{
			if (errno == ENOENT)
				ret = 1;
			else
				ret = fatal(2, "Bob", -1);///strerror(errno), -1);
		}
		else
		{
			if (lseek(fd, strlen("Name:\t"), SEEK_SET) == -1)
				ret = fatal(2, strerror(errno), -1);
			else if (read(fd, name_process_in_dir, strlen(name_process) + 3) == -1) // +3
				ret = fatal(2, strerror(errno), -1);
			else
			{
				name_process_in_dir[strlen(name_process)] = '\0';
				if (strcmp(name_process_in_dir, name_process) != 0)
					ret = 1;
			}
			if (close(fd))
				ret = fatal(2, strerror(errno), -1);
		}
	}
	free(path);
	free(name_process_in_dir);
	return ret;
}

int	add_proc_to_list(t_list_proc** list, char* pid)
{
	t_list_proc*	ptr_list;
	t_list_proc*	new_proc;

	if (*list == 0)
	{
		*list = malloc(sizeof(t_list_proc));
		if (*list == 0)
			return fatal(1, "Malloc fail", 1);
		(*list)->pid = malloc(strlen(pid) + 1);
		if ((*list)->pid == 0)
			return fatal(1, "Malloc fail", 1);
		strcpy((*list)->pid, pid);
		(*list)->next = 0;
	}
	else
	{
		new_proc = malloc(sizeof(t_list_proc));
		if (new_proc == 0)
			return fatal(1, "Malloc fail", 1);
		new_proc->pid = malloc(strlen(pid) + 1);
		if (new_proc->pid == 0)
			return fatal(1, "Malloc fail", 1);
		strcpy(new_proc->pid, pid);
		new_proc->next = 0;

		ptr_list = *list;
		while (ptr_list->next != 0)
			ptr_list = ptr_list->next;
		ptr_list->next = new_proc;
	}
	return 0;
}

void	delete_list(t_list_proc* list)
{
	t_list_proc* ptr_list;

	ptr_list = list;
	while (ptr_list != 0)
	{
		list = ptr_list;
		ptr_list = ptr_list->next;
		free(list->pid);
		free(list);
	}
}

int	get_list(t_list_proc** list, const char* process_name)
{
	DIR*		dir;
	struct dirent*	dirent;
	int		ret;

	delete_list(*list);
	*list = 0;

	dir = opendir("/proc/");
	if (dir == 0)
		return fatal(2, strerror(errno), 1);

	dirent = (struct dirent*)1;
	while (dirent != 0)
	{
		dirent = readdir(dir);
		if (dirent != 0 && (dirent->d_name[0] >= '0' && dirent->d_name[0] <= '9'))
		{
			ret = check_process(dirent->d_name, process_name);
			if (ret == -1)
				break;
			else if (ret == 0)
			{
				if (add_proc_to_list(list, dirent->d_name))
				{
					ret = 1;
					break;
				}
			}
		}
		else if (errno != 0 && errno != ENOENT)
			ret = fatal(2, strerror(errno), -1);
		else
			ret = 0;
	}
	if (closedir(dir))
		ret = fatal(2, strerror(errno), -1);
	return ret;
}


void	print_list_proc(unsigned int choice, t_list_proc* list)
{
	while (list != 0)
	{
		printf("\tPID = %s | %u%% CPU | %u ko | state '%c'\n", list->pid, get_proc_usage(list->pid), get_mem(list->pid), get_state(list->pid));
		list = list->next;
	}
	tputs(tgoto(tgetstr("cm", 0), 1, choice + 1), 1, id_print_char);
	id_print_char('>');
}

int	apply_input(unsigned int* choice, unsigned int size_list, t_list_proc* list, char** path)
{
	int		ret;
	fd_set		fds;
	struct timeval	timeout;

	char		c;
	unsigned int	i;

	FD_ZERO(&fds);
	FD_SET(0, &fds);

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	ret = select(1, &fds, 0, 0, &timeout);
	if (ret == -1)
		return fatal(2, strerror(errno), 1);
	else if (ret != 0 && FD_ISSET(0, &fds))
	{
		if (read(0, &c, 1) == -1)
			return fatal(2, strerror(errno), 1);

		if (c == 'A')
		{
			tputs(tgoto(tgetstr("cm", 0), 1, *choice + 1), 1, id_print_char);
			id_print_char(' ');
			*choice = *choice - 1;
			if (*choice < 1)
				*choice = size_list;
		}
		else if (c == 'B')
		{
			tputs(tgoto(tgetstr("cm", 0), 1, *choice + 1), 1, id_print_char);
			id_print_char(' ');
			*choice = *choice + 1;
			if (*choice > size_list)
				*choice = 1;
		}
		else if (c == '\n')
		{
			i = 1;
			while (i < *choice)
			{
				list = list->next;
				i = i + 1;
			}
			*path = malloc(sizeof(char) * (strlen("/proc//") + strlen(list->pid) + 1));
			if (*path == 0)
				return fatal(2, "Malloc fail", 1);
			strcpy(*path, "/proc/");
			strcat(*path, list->pid);
			strcat(*path, "/");
		}
	}
	return 0;
}

int	get_proc_path(char** path, t_arg* arg)
{
	unsigned int	size_list;
	unsigned int	old_size;
	unsigned int	choice;
	t_list_proc*	list;
	t_list_proc*	ptr_list;

	if (*path != 0)
		free(*path);

	*path = 0;
	list = 0;
	choice = 0;
	old_size = 0;
	tputs(tgetstr("cl", 0), 1, id_print_char);
	while (*path == 0 && gl_run == 1)
	{
		size_list = 0;

	//	tputs(tgetstr("cl", 0), 1, id_print_char);
		tputs(tgoto(tgetstr("cm", 0), 0, 0), 1, id_print_char);
		if (get_list(&list, arg->name_process))
			return 1;

		ptr_list = list;
		while (ptr_list != 0)
		{
			ptr_list = ptr_list->next;
			size_list = size_list + 1;
		}
		if (size_list == 1)
		{
			*path = malloc(strlen("/proc//") + strlen(list->pid) + 1);
			if (*path == 0)
			{
				delete_list(list);
				return fatal(2, "Malloc fail", 1);
			}
			strcpy(*path, "/proc/");
			strcat(*path, list->pid);
			strcat(*path, "/");
			break;
		}
		if (size_list < old_size)
			tputs(tgetstr("cl", 0), 1, id_print_char);

		if (choice == 0 && size_list != 0)
			choice = 1;
		else if (size_list == 0)
			choice = 0;
		else if (choice > size_list)
			choice = size_list;

		if (size_list == 0)
			printf(":: Processus not found... Waiting '%s'\n", arg->name_process);
		else
		{
			printf(":: %d process found\n\n", size_list);
			print_list_proc(choice, list);
			if (apply_input(&choice, size_list, list, path))
			{
				delete_list(list);
				return 1;
			}
		}

		old_size = size_list;

		usleep(arg->delay * 1000);
	}
	delete_list(list);
	return 0;
}

int	inspect_proccess(t_arg* arg)
{
	char*		path;
	int		fd;
	int		ret;

	path = 0;

	gl_run = 1;
	if (get_proc_path(&path, arg))
		return 1;

	ret = 0;
	while (gl_run)
	{
		fd = open(path, O_RDONLY);
		if (fd < 0)
		{
			if (errno == ENOENT)
			{
				ret = get_proc_path(&path, arg);
				if (ret == -1)
				{
					ret = 1;
					gl_run = 0;
				}
			}
			else
			{
				ret = fatal(2, strerror(errno), 1);
				gl_run = 0;
			}
		}
		else
		{
			id_print_str("OK\n");
		//	read();
			if (close(fd) == -1)
				ret = fatal(2, strerror(errno), 1);
		}
		usleep(arg->delay * 1000);
	}
	free(path);
	return ret;
}

