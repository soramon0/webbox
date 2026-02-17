# Define Root and config before including config
NAME       := webserve
ROOT_DIR   := .
SRC_DIR    := src
CLIENT_DIR := tools/client

# Include shared config
include config.mk

.PHONY: all help client

all: $(NAME)

$(NAME): $(BUILD_DIR)/$(NAME)

client:
	@$(MAKE) -C $(CLIENT_DIR)

client-%:
	@$(MAKE) -C $(CLIENT_DIR) $*

# Triggers for cleaning the client
clean::
	@$(MAKE) -C $(CLIENT_DIR) clean

fclean::
	@$(MAKE) -C $(CLIENT_DIR) fclean

help:
	@printf "$(MAGENTA)Available targets:$(RESET)\n"
	@printf "  $(GREEN)make$(RESET)            : Build Debug version\n"
	@printf "  $(GREEN)make release=1$(RESET)  : Build Release version\n"
	@printf "  $(GREEN)make client$(RESET)     : Build Client\n"
