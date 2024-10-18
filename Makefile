# SRCS = $(wildcard srcs/*.cpp)
SRCS = $(wildcard srcs/*.c)
OBJS = $(SRCS:.c=.o)
# Replace with absolute path if needed
MINGW_CC_PATH = x86_64-w64-mingw32-gcc
# 32 bits since voicemeeter is 32 bits

# SPEEDCHANGE_DLL_SRC = srcs/speedchange/speedchange_dll.c
# SPEEDCHANGE_DLL_NAME = speedchange.dll

NAME = VoicemeeterKeygen.exe

# CC = gcc -Wall -Wextra -Werror

# https://stackoverflow.com/questions/4702732/the-program-cant-start-because-libgcc-s-dw2-1-dll-is-missing
MINGW_CC = $(MINGW_CC_PATH) -Wall -Wextra -Werror -static -static-libgcc -static-libstdc++

SHELL = zsh

AQUA = \033[0;96m
AQUA_BOLD = \033[1;96m

PURPLE = \033[0;95m
PURPLE_BOLD = \033[1;95m

GREEN = \033[0;92m
GREEN_BOLD = \033[1;92m
GREEN_UNDRLINE = \033[4;32m

RED = \033[0;91m
IRED = \033[0;31m
RED_BOLD = \033[1;91m

SAME_LINE = \033[0G\033[2K

RESET = \033[0m

%.o: %.c
	@$(MINGW_CC) -c $< -o $@
	@echo -n "$(SAME_LINE)$(AQUA)Compiling $(AQUA_BOLD)$<$(RESET)"


$(NAME):	$(OBJS)
	@echo
	@echo "$(PURPLE)Linking $(PURPLE)*.o into $(PURPLE_BOLD)$(NAME)$(RESET)"
	@$(MINGW_CC) $(OBJS) -o $(NAME)
	@echo "$(GREEN_BOLD)Done compiling $(GREEN_UNDERLINE)$(NAME)"

# TODO remove this (only for dev), also copy file to windows susbsystem instead of using wsl
	@cp $(NAME) /mnt/e/Dev/voicemeeter_bypass_output/$(NAME)

# $(SPEEDCHANGE_DLL_NAME): $(SPEEDCHANGE_DLL_SRC):
# 	gcc -shared -o speedChange.dll speedChange.dll.c -I./minhook-master/include -L./minhook-master/lib -lMinHook
# 	$(CC) $(SPEEDCHANGE_DLL_SRC) -shared -o $(SPEEDCHANGE_DLL_NAME) -I./minhook-master/include -L./minhook-master/lib -lMinHook 

all:		$(NAME)

clean:
	@rm -f $(OBJS)
	@echo "$(RED)Removing $(IRED)*.o$(RESET)"

fclean:		clean
	@rm -f $(NAME)
	@echo "$(RED)Removing $(IRED)$(NAME)$(RESET)"

re:			fclean all

.PHONY:	all clean fclean re