CC := gcc
CFLAGS := -Wall -Werror -MMD -MP
LDFLAGS := 

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
INCLUDE_DIR := include
SRC_DIR_LIST := $(SRC_DIR)/list
SRC_DIR_QUEUE := $(SRC_DIR)/queue
SRC_DIR_CACHE_CONTROL := $(SRC_DIR)/cache_control
SRC_DIR_CACHE_CONTROL_MEDIATOR := $(SRC_DIR)/cache_control_mediator
SRC_DIR_LOG := $(SRC_DIR)/LOG

OBJ_DIR_LIST := $(OBJ_DIR)/list
OBJ_DIR_QUEUE := $(OBJ_DIR)/queue
OBJ_DIR_CACHE_CONTROL := $(OBJ_DIR)/cache_control
OBJ_DIR_CACHE_CONTROL_MEDIATOR := $(OBJ_DIR)/cache_control_mediator
OBJ_DIR_LOG := $(OBJ_DIR)/LOG

INCLUDE_DIR_LIST := $(INCLUDE_DIR)/list
INCLUDE_DIR_QUEUE := $(INCLUDE_DIR)/queue
INCLUDE_DIR_CACHE_CONTROL := $(INCLUDE_DIR)/cache_control
INCLUDE_DIR_CACHE_CONTROL_MEDIATOR := $(INCLUDE_DIR)/cache_control_mediator
INCLUDE_DIR_LOG := $(INCLUDE_DIR)/LOG

DEBUG := 0

TARGET_CACHE_CONTROL := $(BIN_DIR)/cache_control
TARGET_CACHE_CONTROL_MEDIATOR := $(BIN_DIR)/cache_control_mediator

ifeq ($(DEBUG), 1)
CFLAGS+=-g
endif

SRC_CACHE_CONTROL := $(wildcard $(SRC_DIR_CACHE_CONTROL)/*.c)
SRC_CACHE_CONTROL_MEDIATOR := $(wildcard $(SRC_DIR_CACHE_CONTROL_MEDIATOR)/*.c)
SRC_LIST := $(wildcard $(SRC_DIR_LIST)/*.c)
SRC_QUEUE := $(wildcard $(SRC_DIR_QUEUE)/*.c)
SRC_LOG := $(wildcard $(SRC_DIR_LOG)/*.c)


OBJ_CACHE_CONTROL := $(patsubst $(SRC_DIR_CACHE_CONTROL)/%.c, $(OBJ_DIR_CACHE_CONTROL)/%.o, $(SRC_CACHE_CONTROL))
OBJ_CACHE_CONTROL_MEDIATOR := $(patsubst $(SRC_DIR_CACHE_CONTROL_MEDIATOR)/%.c, $(OBJ_DIR_CACHE_CONTROL_MEDIATOR)/%.o, $(SRC_CACHE_CONTROL_MEDIATOR))
OBJ_LIST := $(patsubst $(SRC_DIR_LIST)/%.c, $(OBJ_DIR_LIST)/%.o, $(SRC_LIST))
OBJ_QUEUE := $(patsubst $(SRC_DIR_QUEUE)/%.c, $(OBJ_DIR_QUEUE)/%.o, $(SRC_QUEUE))
OBJ_LOG := $(patsubst $(SRC_DIR_LOG)/%.c, $(OBJ_DIR_LOG)/%.o, $(SRC_LOG))


OBJ := $(OBJ_CACHE_CONTROL) $(OBJ_CACHE_CONTROL_MEDIATOR) $(OBJ_LIST) $(OBJ_QUEUE) $(OBJ_LOG)

DEP := $(OBJ:.o=.d)

.PHONY: all clean print_vars docs docs_clean help clang_format

all:  $(TARGET_CACHE_CONTROL) $(TARGET_CACHE_CONTROL_MEDIATOR)


$(TARGET_CACHE_CONTROL): $(OBJ_CACHE_CONTROL) $(OBJ_LIST) $(OBJ_QUEUE) $(OBJ_LOG) | $(BIN_DIR)
	$(CC) $^ -o $@ $(LDFLAGS)

$(TARGET_CACHE_CONTROL_MEDIATOR): $(OBJ_CACHE_CONTROL_MEDIATOR) $(OBJ_QUEUE) | $(BIN_DIR)
	$(CC) $^ -o $@ $(LDFLAGS)

$(BIN_DIR):
	mkdir -p $@

$(OBJ_DIR_CACHE_CONTROL)/%.o: $(SRC_DIR_CACHE_CONTROL)/%.c | $(OBJ_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR_CACHE_CONTROL_MEDIATOR)/%.o: $(SRC_DIR_CACHE_CONTROL_MEDIATOR)/%.c | $(OBJ_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR_LIST)/%.o: $(SRC_DIR_LIST)/%.c | $(OBJ_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR_QUEUE)/%.o: $(SRC_DIR_QUEUE)/%.c | $(OBJ_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR):
	mkdir -p $@ $(OBJ_DIR_CACHE_CONTROL) $(OBJ_DIR_CACHE_CONTROL_MEDIATOR) $(OBJ_DIR_LIST) $(OBJ_DIR_QUEUE)


print_vars:
	@echo "SRC_CACHE_CONTROL = $(SRC_CACHE_CONTROL)"
	@echo "SRC_CACHE_CONTROL_MEDIATOR = $(SRC_CACHE_CONTROL_MEDIATOR)"
	@echo "SRC_LIST = $(SRC_LIST)"
	@echo "SRC_QUEUE = $(SRC_QUEUE)"
	@echo "SRC_LOG = $(SRC_LOG)"
	@echo "OBJ_CACHE_CONTROL = $(OBJ_CACHE_CONTROL)"
	@echo "OBJ_CACHE_CONTROL_MEDIATOR = $(OBJ_CACHE_CONTROL_MEDIATOR)"
	@echo "OBJ_LIST = $(OBJ_LIST)"
	@echo "OBJ_QUEUE = $(OBJ_QUEUE)"
	@echo "OBJ_LOG = $(OBJ_LOG)"
	@echo "DEP = $(DEP)"

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

docs:
	doxygen Doxyfile
	make -C docs/latex/
	@echo "Documentation generated in $(DOC_DIR)/html/index.html и в $(DOC_DIR)/latex/refman.pdf"

docs_clean:
	rm -rf docs/
	@echo "Documentation removed"

clang_format:
	find $(INCLUDE_DIR) $(SRC_DIR) -type f \( -name "*.c" -o -name "*.h" \) -exec clang-format -i --style=file {} +
	@echo "The files are formatted"

help:
	@echo "Available targets:"
	@echo "  make all             - build all binaries"
	@echo "  make clean           - remove build files"
	@echo "  make clang_format    - format source code with clang-format"
	@echo "  make docs            - generate documentation"
	@echo "  make docs_clean      - remove documentation"
	@echo "  make print_vars      - print Makefile variables"
	@echo ""
	@echo "Binaries:"
	@echo "  $(TARGET_CACHE_CONTROL)"
	@echo "  $(TARGET_CACHE_CONTROL_MEDIATOR)"

-include $(DEP)