NAME		=	topy

SRC		=	find_process.c	\
			get_info.c	\
			main.c		\
			string.c

OBJ		=	$(SRC:.c=.o)

CFLAGS		+=	-Wall -W -Werror -g

CC		=	gcc

$(NAME)		:	$(OBJ)
			$(CC) $(CFLAGS) $(OBJ) -o $(NAME) -lncurses

all		:	$(NAME)

clean		:
			rm -rf $(OBJ)

distclean	:	clean
			rm -rf $(NAME)

rebuild		:	distclean all

