#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <termcap.h>
#include <termios.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "string.h"
#include "types.h"

int		run;
struct termios	old_term;

void	signal_interrup(int a)
{
	(void)a;

	run = 0;
	write(1, "\n", 1);
}

int	inspect_proccess(t_arg* arg);

void	print_help()
{
	id_print_str("./topy\t[-d, --delay DELAY]\n");
	id_print_str("      \t[-h / --help]\n");
}

int	get_arg(int argc, char** argv, t_arg* arg)
{
	int	i;

	arg->name_process = 0;
	arg->delay = 200;

	if (argc < 2)
	{
		id_print_str(":: Invalid argument\n   Use -h or --help\n");
		return 1;
	}
	if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
	{
		print_help();
		return 0;
	}
	else
		arg->name_process = argv[1];

	i = 2;
	while (argv[i] != 0)
	{
		if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--delay") == 0)
		{
			if (argv[i + 1] == 0)
				id_print_str(":: The option 'Delay' is not complete, delay will be set at '1000'\n");
			else
			{
				i = i + 1;
				arg->delay = atoi(argv[i]);
			}
		}
		else
		{
			id_print_str(":: Invalid argument : '");
			id_print_str(argv[i]);
			id_print_str("'\n   Use -h or --help\n");
			return 1;
		}
		i = i + 1;
	}
	return 0;
}

int	init_tty()
{
	int		ret;
	char*		term;
	struct termios	new_term;

	term = getenv("TERM");
	if (term == 0)
		return fatal(2, "TERM is not defined in env", 1);

	ret = tgetent(0, term);
	if (ret == 0)
		return fatal(2, "tgtent fail", 1);

	tcgetattr(0, &old_term);
	new_term = old_term;
	new_term.c_lflag &= ~(ECHO | ICANON);

	tcsetattr(0, TCSANOW, &new_term);
	tputs(tgetstr("vi", 0), 1, id_print_char);

	if (signal(SIGINT, signal_interrup) == SIG_ERR)
		return fatal(2, "Can't configure the signal SIGINT", 1);

	return 0;
}

void	restore_tty()
{
	tputs(tgetstr("cl", 0), 1, id_print_char);
	tputs(tgetstr("ve", 0), 1, id_print_char);
	tcsetattr(0, TCSANOW, &old_term);
}

int	main(int argc, char** argv)
{
	int	ret;
	t_arg	arg;

	if (isatty(STDIN_FILENO) == 0)
		return fatal(2, strerror(errno), 1);

	if (get_arg(argc, argv, &arg))
		ret = 1;

	if (arg.name_process != 0)
	{
		if (init_tty())
			return 1;
		ret = inspect_proccess(&arg);
		restore_tty();
	}
	return ret;
}

