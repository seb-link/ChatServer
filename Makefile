CC         = gcc
CFLAGS     = -Wall -Wextra -Wpedantic -Iinclude -std=c17 -ggdb
LDFLAGS    = -lssl -lcrypto
CWARNFLAGS = -Wno-unused-variable -Wno-pointer-sign -Wno-unused-command-line-argument

TST_DIR = tests/src
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = tests/bin

TARGET = server

# Main program
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Test handling
TEST_SRCS = $(wildcard $(TST_DIR)/*.c)
TEST_BINS = $(patsubst $(TST_DIR)/%.c, $(BIN_DIR)/%, $(TEST_SRCS))

.PHONY: all clean tests

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CWARNFLAGS) -c $< -o $@

# Fixed test rule with explicit path handling
$(TEST_BINS): $(BIN_DIR)/%: $(TST_DIR)/%.c $(filter-out $(OBJ_DIR)/main.o, $(OBJS))
	@mkdir -p $(@D)
	$(CC) -Itests $(CFLAGS) $(CWARNFLAGS) $^ -o $@ $(LDFLAGS)

tests: $(TEST_BINS)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(TARGET) core*
