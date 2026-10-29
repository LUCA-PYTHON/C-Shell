#ifndef INPUT_H
#define INPUT_H
#include <stddef.h>

#define INPUT_STATUS_SUCCESS 0
#define INPUT_STATUS_EOF 1
#define INPUT_STATUS_ERROR 2
#define INPUT_STATUS_TOO_LONG 3

int read_input(char *buffer, size_t size);

#endif