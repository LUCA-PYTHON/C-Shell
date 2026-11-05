CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Wstrict-prototypes -g -Iinclude

TARGET = c-shell
BUILD_DIR = build

SOURCES = src/cshell.c src/input.c src/builtins.c src/executor.c src/parser.c src/file_operations.c
OBJECTS = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SOURCES))
HEADERS = $(wildcard include/*.h)

TEST_PARSER   = $(BUILD_DIR)/test_parser
TEST_BUILTINS = $(BUILD_DIR)/test_builtins
TEST_EXECUTOR = $(BUILD_DIR)/test_executor
TEST_INPUT    = $(BUILD_DIR)/test_input
UNIT_TESTS    = $(TEST_PARSER) $(TEST_BUILTINS) $(TEST_EXECUTOR) $(TEST_INPUT)

.PHONY: all clean test test-unit test-integration test-asan

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(OBJECTS:.o=.d)

$(BUILD_DIR)/test_%: tests/unit/test_%.c $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(filter %.c %.o,$^) -o $@

$(TEST_PARSER):   $(BUILD_DIR)/parser.o
$(TEST_BUILTINS): $(BUILD_DIR)/builtins.o
$(TEST_EXECUTOR): $(BUILD_DIR)/executor.o
$(TEST_INPUT):    $(BUILD_DIR)/input.o

test: all test-unit test-integration

test-unit: $(UNIT_TESTS)
	@for t in $(UNIT_TESTS); do echo "== $$t"; ./$$t || exit 1; done

test-integration: all
	bash tests/integration/test_shell.sh

test-asan:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(CFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer" test-unit

clean:
	rm -f $(TARGET)
	rm -rf $(BUILD_DIR)