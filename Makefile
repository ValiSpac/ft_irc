# Nom de votre executable
NAME = ircserv

# Compilateur et flags de compilation
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I includes -g3

# Dossiers
SRCDIR = srcs
OBJDIR = objs
INCDIR = includes

# Fichiers sources et objets
SRC = $(wildcard $(SRCDIR)/*.cpp)
OBJ = $(SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Règles
all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(OBJ) -o $(NAME)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
