NAME     := tcp_client
ROOT_DIR := ../..
SRC_DIR  := src

# Include shared config
include ../../config.mk

.PHONY: all help

all: $(NAME)

$(NAME): $(BUILD_DIR)/$(NAME)

help:
	@printf "$(MAGENTA)Available targets:$(RESET)\n"
	@printf "  $(GREEN)make$(RESET)            : Build Debug version\n"
	@printf "  $(GREEN)make release=1$(RESET)  : Build Release version\n"
