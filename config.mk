CXX      := g++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98
LDFLAGS  :=
ARGS     ?=

# Colors
GREEN    := \033[1;32m
YELLOW   := \033[1;33m
BLUE     := \033[1;34m
MAGENTA  := \033[1;35m
RESET    := \033[0m

# Build Configuration
ifdef release
    BUILD_TYPE := release
    CXXFLAGS   += -O3 -DNDEBUG
else
    BUILD_TYPE := debug
    CXXFLAGS   += -g3 -O0 -DDEBUG
endif

# Verbose Mode
ifndef V
    Q := @
else
    Q :=
endif

ifdef sanitize
    CXXFLAGS   += -fsanitize=address -fno-omit-frame-pointer
    LDFLAGS    += -fsanitize=address
endif

# Note: SRC_DIR and ROOT_DIR must be defined by the Makefile that includes this!
OBJ_DIR    := obj/$(BUILD_TYPE)
BUILD_DIR  := build/$(BUILD_TYPE)
INC_DIR    := $(ROOT_DIR)/include
CPPFLAGS   := -I$(SRC_DIR) -I$(INC_DIR) -MMD -MP

SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

.DELETE_ON_ERROR:

.PHONY: release run clean fclean re

$(BUILD_DIR)/$(NAME): $(OBJS)
	@mkdir -p $(dir $@)
	@printf "$(BLUE)Linking $(NAME) ($(BUILD_TYPE))...$(RESET)\n"
	$(Q)$(CXX) $(OBJS) -o $@ $(LDFLAGS)
	@printf "$(GREEN)Build successful! Binary: $@$(RESET)\n"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@printf "$(YELLOW)Compiling $<...$(RESET)\n"
	$(Q)$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

release::
	@$(MAKE) all release=1

run:: $(BUILD_DIR)/$(NAME)
	@printf "$(MAGENTA)Running $(NAME)...$(RESET)\n"
	./$(BUILD_DIR)/$(NAME) $(ARGS)

clean::
	@printf "$(YELLOW)Cleaning objects...$(RESET)\n"
	$(Q)rm -rf $(firstword $(subst /, ,$(OBJ_DIR)))

fclean:: clean
	@printf "$(YELLOW)Removing executables...$(RESET)\n"
	$(Q)rm -rf $(firstword $(subst /, ,$(BUILD_DIR)))

re:: fclean all

-include $(DEPS)
