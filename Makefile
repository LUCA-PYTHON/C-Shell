
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Wstrict-prototypes -g -Iinclude

TARGET = c-shell
BUILD_DIR = build

SOURCES = src/cshell.c src/input.c src/builtins.c src/executor.c src/parser.c
HEADERS = $(wildcard include/*.h)

TEST_PARSER = $(BUILD_DIR)/test_parser
TEST_BUILTINS = $(BUILD_DIR)/test_builtins
TEST_EXECUTOR = $(BUILD_DIR)/test_executor

.PHONY: all clean test test-unit test-integration

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TEST_PARSER): tests/unit/test_parser.c src/parser.c $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) tests/unit/test_parser.c src/parser.c -o $(TEST_PARSER)

$(TEST_BUILTINS): tests/unit/test_builtins.c src/builtins.c $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) tests/unit/test_builtins.c src/builtins.c -o $(TEST_BUILTINS)

$(TEST_EXECUTOR): tests/unit/test_executor.c src/executor.c $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) tests/unit/test_executor.c src/executor.c -o $(TEST_EXECUTOR)

test: all test-unit test-integration

test-unit: $(TEST_PARSER) $(TEST_BUILTINS) $(TEST_EXECUTOR)
	./$(TEST_PARSER)
	./$(TEST_BUILTINS)
	./$(TEST_EXECUTOR)

test-integration: all
	bash tests/integration/test_shell.sh

clean:
	rm -f $(TARGET)
	rm -rf $(BUILD_DIR)