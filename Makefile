CC         = gcc
CFLAGS     = -Wall -Wextra -Iinclude -std=c17 -llibssl -libcrypto -g
CWARNFLAGS = -Wno-unused-variable -Wno-pointer-sign

SRC_DIR = src
OBJ_DIR = obj

TARGET  = server

SRCS    = $(wildcard $(SRC_DIR)/*.c)
OBJS    = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(CWARNFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)
