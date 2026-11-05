#ifndef INPUT_H
#define INPUT_H
#include <stddef.h>

#define INPUT_STATUS_SUCCESS 0
#define INPUT_STATUS_EOF 1
#define INPUT_STATUS_ERROR 2
#define INPUT_STATUS_TOO_LONG 3
int read_input(char **buffer, size_t prompt_column);
int enable_raw_mode(void);
int disable_raw_mode(void);
int read_key(char *key);
int history_navigation(void);
int get_terminal_width(void);

#endif