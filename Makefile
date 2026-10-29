CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Wstrict-prototypes -g -Iinclude

TARGET = c-shell
SOURCES = src/cshell.c src/input.c src/builtins.c src/executor.c src/parser.c
HEADERS = $(wildcard include/*.h)

.PHONY: all clean test test_parser

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET) tests/test_parser

test_parser: tests/test_parser.c src/parser.c $(HEADERS)
	$(CC) $(CFLAGS) tests/test_parser.c src/parser.c -o tests/test_parser

test: $(TARGET) test_parser
	./tests/test_parser
	bash tests/test_shell.sh