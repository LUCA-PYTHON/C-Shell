CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Iinclude
TARGET = c-shell

SOURCES = src/cshell.c \
          src/input.c \
          src/builtins.c \
          src/executor.c \
          src/parser.c

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: clean
