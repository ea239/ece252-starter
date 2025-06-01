# Makefile - ECE252 project for paster

CC       = gcc
CFLAGS   = -Wall -g -std=c99 -Iinclude
LD       = gcc
LDFLAGS  = -g
LDLIBS   = -lz -lcurl -lpthread

SRC_DIR  = src
INC_DIR  = include
OBJ_DIR  = obj
OUTPUT_DIR = output

# Source files
PASTER_SRC = $(SRC_DIR)/paster.c
COMMON_SRCS = $(filter-out $(PASTER_SRC), $(wildcard $(SRC_DIR)/*.c))

# Object files
PASTER_OBJ = $(OBJ_DIR)/paster.o
COMMON_OBJS = $(COMMON_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

TARGET = paster

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(PASTER_OBJ) $(COMMON_OBJS)
	$(LD) -o $@ $^ $(LDLIBS) $(LDFLAGS)

# Create obj directory if not exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Compile .c to .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

# Include dependency files
-include $(COMMON_OBJS:.o=.d)
-include $(PASTER_OBJ:.o=.d)

clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(OUTPUT_DIR) all.png
