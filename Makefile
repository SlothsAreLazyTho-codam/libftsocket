# **************************************************************************** #
#                                                                              #
#                                                         ::::::::             #
#    Makefile                                           :+:    :+:             #
#                                                      +:+                     #
#    By: macbook <macbook@student.codam.nl>           +#+                      #
#                                                    +#+                       #
#    Created: 2024/04/18 03:23:25 by macbook       #+#    #+#                  #
#    Updated: 2024/08/13 14:54:42 by cbijman       ########   odam.nl          #
#                                                                              #
# **************************************************************************** #

# Compiler Rules
NAME=libftsocket.a
CXX=clang++ --std=c++11
CXX_FLAGS=-I./include/

# Colours
RESET		= \033[m
DARK_GRAY	= \033[1;30m
GREEN		= \033[0;92m
BOLD_RED	= \033[1;31m

# Commands
AR = ar -rcs
RM = rm -rf

# Folders
SRC_FOLDER=./src
BIN_FOLDER=./bin
DEP_FOLDER=$(BIN_FOLDER)/deps

# File structure
FILES =	Socket \
		TcpClient \
		TcpServer 

vpath %.cpp $(SRC_FOLDER) \
			$(SRC_FOLDER)/tcp \

SRC =	${addsuffix .cpp, $(FILES)}
OBJS =	${patsubst %.cpp, $(BIN_FOLDER)/%.o, $(SRC)}
DEPS =	${patsubst %.cpp, $(DEP_FOLDER)/%.d, $(SRC)}

.SILENT:
.QUIET:

all: $(NAME)

-include $(DEPS)

$(BIN_FOLDER)/%.o: %.cpp | $(DEP_FOLDER)
	$(CXX) $(CXX_FLAGS) -MMD -MF $(DEP_FOLDER)/$*.d -c $< -o $@
	echo "$(GREEN)Building $(DARK_GRAY)$<$(RESET)"

$(NAME): $(OBJS)
	$(AR) $(NAME) $(OBJS)
	@echo "Socket library compiled!"

$(DEP_FOLDER):
	mkdir -p $(DEP_FOLDER)

clean:
	$(RM) $(OBJS) $(BIN_FOLDER)
	
fclean: clean
	$(RM) $(NAME)

re: clean all

debug: CXX += -fsanitize=address -DDEBUG -DBOLD_MAGENTA="\"$(BOLD_RED)\""
debug: fclean $(OBJS)
	$(CXX) $(CXX_FLAGS) ./tests/server.cpp $(OBJS)
	echo "Debug build"

.PHONY: all clean fclean re build debug
