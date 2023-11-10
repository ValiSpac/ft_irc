NAME		=	a.out
CFLAGS		=	-Wall -Wextra -Werror -std=c++98
CC			=	g++
MAKE		=	make

HEADERS		=	./Server.hpp ./User.hpp
INCLUDE		=	
OBJS		=	$(patsubst %.cpp,%.o,$(SRCS))
SRCS		=	./main.cpp ./Server.cpp ./User.cpp

all :
	make $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

%.o : %.cpp $(HEADERS) Makefile
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean :
	rm -f $(OBJS)

fclean : clean
	rm -f $(NAME)

re : fclean all

.PHONY : all clean fclean re
