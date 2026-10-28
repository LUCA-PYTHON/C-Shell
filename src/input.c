#include <stdio.h>
#include <string.h>

#include "input.h"

int read_input(char *buffer, size_t size)
{
    if (fgets(buffer, size, stdin) == NULL)
    {
        return -1;
    }

    buffer[strcspn(buffer, "\n")] = '\0';

    return 0;
}