#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

int handle_file_command(int argc, char **args);
int file_create(const char *path);
int file_write(const char *path, const char *content);
int file_append(const char *path, const char *content);
int file_delete(const char *path);
int file_delete_directory(const char *path);
int file_read(const char *path);

#endif