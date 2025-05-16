# Makefile - ECE252 project

CC       = gcc
CFLAGS   = -Wall -g -std=c99 -Iinclude
LD       = gcc
LDFLAGS  = -g
LDLIBS   = -lz

SRC_DIR  = src
INC_DIR  = include
OBJ_DIR  = obj
BIN_DIR  = bin

SRCS     = $(wildcard $(SRC_DIR)/*.c)
OBJS     = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS     = $(OBJS:.o=.d)

TARGET   = $(BIN_DIR)/main.out

all: $(TARGET)

$(TARGET): | $(OBJ_DIR) $(BIN_DIR)
$(TARGET): $(OBJS)
	$(LD) -o $@ $^ $(LDLIBS) $(LDFLAGS)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.d: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) -MM $(CFLAGS) $< -MF $@


-include $(DEPS)

.PHONY: all clean

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
