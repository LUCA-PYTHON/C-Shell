#ifndef REDIRECTIONS_H
#define REDIRECTIONS_H

#include <stdbool.h>

typedef struct
{
    const char *input_file;
    const char *output_file;
    bool append;
} Redirections;

#endif