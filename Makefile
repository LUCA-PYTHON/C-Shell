
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Wstrict-prototypes -g -Iinclude

TARGET = c-shell
BUILD_DIR = build

SOURCES = src/cshell.c src/input.c src/builtins.c src/executor.c src/parser.c
HEADERS = $(wildcard include/*.h)

TEST_PARSER = $(BUILD_DIR)/test_parser

.PHONY: all clean test test-unit test-integration

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TEST_PARSER): tests/unit/test_parser.c src/parser.c $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) tests/unit/test_parser.c src/parser.c -o $(TEST_PARSER)

test: all $(TEST_PARSER)
	bash tests/run_tests.sh

test-unit: $(TEST_PARSER)
	./$(TEST_PARSER)

test-integration: all
	bash tests/integration/test_shell.sh

clean:
	rm -f $(TARGET)
	rm -rf $(BUILD_DIR)
