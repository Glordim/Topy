#include <string.h>
#include <unistd.h>

int	id_print_char(int c)
{
	return write(1, &c, 1);
}

int	id_print_str(const char* str)
{
	return write(1, str, strlen(str));
}

int	fatal(int level, const char* str, int ret)
{
	(void)level;
	id_print_str(str);
	write(1, "\n", 1);
	return ret;
}

