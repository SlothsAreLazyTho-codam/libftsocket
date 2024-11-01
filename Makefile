# **************************************************************************** #
#                                                                              #
#                                                         ::::::::             #
#    Makefile                                           :+:    :+:             #
#                                                      +:+                     #
#    By: macbook <macbook@student.codam.nl>           +#+                      #
#                                                    +#+                       #
#    Created: 2024/04/18 03:23:25 by macbook       #+#    #+#                  #
#    Updated: 2024/11/01 16:14:52 by cbijman       ########   odam.nl          #
#                                                                              #
# **************************************************************************** #

# Compiler Rules
NAME=libsocket.a
CXX=clang++ -g --std=c++20
CXX_FLAGS=-I./include/

# Commands
AR = ar -rcs
RM = rm -rf

# Folders
SRC_FOLDER=./src
BIN_FOLDER=./bin
DEP_FOLDER=$(BIN_FOLDER)/deps

# File structure
FILES =	Socket \
		TcpClient 

vpath %.cpp $(SRC_FOLDER) \
			$(SRC_FOLDER)/tcp \

SRC =	${addsuffix .cpp, $(FILES)}
OBJS =	${patsubst %.cpp, $(BIN_FOLDER)/%.o, $(SRC)}
DEPS =	${patsubst %.cpp, $(DEP_FOLDER)/%.d, $(SRC)}

# Colors
ifneq (,$(findstring xterm,${TERM}))
	BLACK		:= $(shell tput -Txterm setaf 0)
	RED			:= $(shell tput -Txterm setaf 1)
	GREEN		:= $(shell tput -Txterm setaf 2)
	YELLOW		:= $(shell tput -Txterm setaf 3)
	LIGHTPURPLE	:= $(shell tput -Txterm setaf 4)
	PURPLE		:= $(shell tput -Txterm setaf 5)
	BLUE		:= $(shell tput -Txterm setaf 6)
	WHITE		:= $(shell tput -Txterm setaf 7)
	RESET		:= $(shell tput -Txterm sgr0)
endif

P				:= printf "%-25.25s%s\n"
BUILD_CHECK		= && $(P) "Build${GREEN}" "$@$(RESET)" || $(P) "Build${RED}" "$@$(RESET)"
BUILD_FATAL		= && $(P) "Build${GREEN}" "$@$(RESET)" || ($(P) "Build${RED}" "$@$(RESET)" && exit 1)
GIT_FATAL		= && $(P) "Git pull${GREEN}" "$@$(RESET)" || ($(P) "Git pull failed${RED}" "$@$(RESET)" && exit 1)
EXEC_FATAL		= && $(P) "Executable${GREEN}" "$@$(RESET)" || ($(P) "Executable${RED}" "$@$(RESET)" && exit 1)
FINAL			= && $(P) "🙏$(PURPLE) All Done" "$(RESET)" || $(P) "🤬 $(RED)Norm Error" "$(RESET)"

# Rules
.SILENT:
.QUIET:

all: $(NAME)

-include $(DEPS)

$(BIN_FOLDER)/%.o: %.cpp | $(DEP_FOLDER)
	$(CXX) $(CXX_FLAGS) -MMD -MF $(DEP_FOLDER)/$*.d -c $< -o $@ $(BUILD_CHECK)

$(NAME): $(OBJS)
	$(AR) $(NAME) $(OBJS) $(EXEC_FATAL)

$(DEP_FOLDER):
	mkdir -p $(DEP_FOLDER)

clean:
	$(RM) $(OBJS) $(BIN_FOLDER)
	
fclean: clean
	$(RM) $(NAME) ./a.out

re: clean all

debug: CXX += -DDEBUG
debug: fclean all
	echo "Library build with debug enabled"

.PHONY: all clean fclean re