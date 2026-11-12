#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "file_operations.h"
#include "shell_status.h"

static int passed = 0, failed = 0;

static char test_dir[256];
static int saved_stderr = -1;
static int saved_stdout = -1;

static void check(const char *name, bool ok)
{
    if (ok)
    {
        passed++;
    }
    else
    {
        failed++;
        printf("  [FAIL] %s\n", name);
    }
}

static bool setup_test_dir(void)
{
    snprintf(test_dir, sizeof(test_dir), "/tmp/cshell_test_XXXXXX");
    return mkdtemp(test_dir) != NULL;
}

static void cleanup_test_dir(void)
{
    char command[512];
    snprintf(command, sizeof(command), "rm -rf '%s'", test_dir);
    if (system(command) != 0)
    {
        fprintf(stderr, "Warning: could not remove %s\n", test_dir);
    }
}

static void make_path(char *out, size_t size, const char *name)
{
    snprintf(out, size, "%s/%s", test_dir, name);
}

static bool exists(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

static bool is_directory(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

static char *read_whole_file(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (f == NULL)
    {
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *content = malloc((size_t)size + 1);
    if (content == NULL)
    {
        fclose(f);
        return NULL;
    }

    size_t got = fread(content, 1, (size_t)size, f);
    content[got] = '\0';
    fclose(f);
    return content;
}

static bool file_has_content(const char *path, const char *expected)
{
    char *content = read_whole_file(path);
    bool ok = content != NULL && strcmp(content, expected) == 0;
    free(content);
    return ok;
}

static void write_raw(const char *path, const char *content)
{
    FILE *f = fopen(path, "wb");
    if (f != NULL)
    {
        fputs(content, f);
        fclose(f);
    }
}

static void silence_stderr(void)
{
    fflush(stderr);
    saved_stderr = dup(STDERR_FILENO);
    int null_fd = open("/dev/null", O_WRONLY);
    if (null_fd != -1)
    {
        dup2(null_fd, STDERR_FILENO);
        close(null_fd);
    }
}

static void restore_stderr(void)
{
    if (saved_stderr != -1)
    {
        dup2(saved_stderr, STDERR_FILENO);
        close(saved_stderr);
        saved_stderr = -1;
    }
}

static void silence_stdout(void)
{
    fflush(stdout);
    saved_stdout = dup(STDOUT_FILENO);
    int null_fd = open("/dev/null", O_WRONLY);
    if (null_fd != -1)
    {
        dup2(null_fd, STDOUT_FILENO);
        close(null_fd);
    }
}

static void restore_stdout(void)
{
    if (saved_stdout != -1)
    {
        fflush(stdout);
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
        saved_stdout = -1;
    }
}

static char *read_captured(const char *path, int *result)
{
    char capture_path[300];
    make_path(capture_path, sizeof(capture_path), ".capture");

    fflush(stdout);
    int saved = dup(STDOUT_FILENO);
    int fd = open(capture_path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (saved == -1 || fd == -1)
    {
        *result = SHELL_STATUS_ERROR;
        return NULL;
    }

    dup2(fd, STDOUT_FILENO);
    close(fd);

    *result = file_read(path);

    fflush(stdout);
    dup2(saved, STDOUT_FILENO);
    close(saved);

    return read_whole_file(capture_path);
}

static void test_create(void)
{
    char path[300];

    make_path(path, sizeof(path), "new.txt");
    check("create: neue Datei",
          file_create(path) == SHELL_STATUS_SUCCESS && exists(path));

    silence_stderr();
    int again = file_create(path);
    restore_stderr();
    check("create: existiert schon -> Fehler", again == SHELL_STATUS_ERROR);

    make_path(path, sizeof(path), "a/b/c.txt");
    check("create: legt fehlende Ordner an",
          file_create(path) == SHELL_STATUS_SUCCESS &&
          exists(path));

    char dir_a[300];
    make_path(dir_a, sizeof(dir_a), "a/b");
    check("create: Zwischenordner sind Ordner", is_directory(dir_a));

    silence_stderr();
    int null_result = file_create(NULL);
    int empty_result = file_create("");
    restore_stderr();
    check("create: NULL -> Fehler", null_result == SHELL_STATUS_ERROR);
    check("create: leerer Pfad -> Fehler", empty_result == SHELL_STATUS_ERROR);
}

static void test_write(void)
{
    char path[300];
    make_path(path, sizeof(path), "write.txt");

    check("write: legt Datei an, wenn sie fehlt",
          file_write(path, "alt") == SHELL_STATUS_SUCCESS &&
          file_has_content(path, "alt"));

    check("write: ueberschreibt den Inhalt",
          file_write(path, "neu") == SHELL_STATUS_SUCCESS &&
          file_has_content(path, "neu"));

    check("write: leerer Text leert die Datei",
          file_write(path, "") == SHELL_STATUS_SUCCESS &&
          file_has_content(path, ""));

    char missing_dir[300];
    make_path(missing_dir, sizeof(missing_dir), "fehlt/x.txt");
    silence_stderr();
    int no_dir = file_write(missing_dir, "x");
    int null_content = file_write(path, NULL);
    int null_path = file_write(NULL, "x");
    restore_stderr();
    check("write: Ordner fehlt -> Fehler", no_dir == SHELL_STATUS_ERROR);
    check("write: Inhalt NULL -> Fehler", null_content == SHELL_STATUS_ERROR);
    check("write: Pfad NULL -> Fehler", null_path == SHELL_STATUS_ERROR);
}

static void test_append(void)
{
    char path[300];
    make_path(path, sizeof(path), "append.txt");

    check("append: legt Datei an, wenn sie fehlt",
          file_append(path, "a") == SHELL_STATUS_SUCCESS &&
          file_has_content(path, "a"));

    check("append: haengt an",
          file_append(path, "b") == SHELL_STATUS_SUCCESS &&
          file_has_content(path, "ab"));

    silence_stderr();
    int null_content = file_append(path, NULL);
    int null_path = file_append(NULL, "x");
    restore_stderr();
    check("append: Inhalt NULL -> Fehler", null_content == SHELL_STATUS_ERROR);
    check("append: Pfad NULL -> Fehler", null_path == SHELL_STATUS_ERROR);
    check("append: Datei blieb bei Fehlern unveraendert", file_has_content(path, "ab"));
}

static void test_read(void)
{
    char path[300];
    int result;
    char *output;

    make_path(path, sizeof(path), "read.txt");
    write_raw(path, "hallo\n");
    output = read_captured(path, &result);
    check("read: Inhalt mit Zeilenumbruch unveraendert",
          result == SHELL_STATUS_SUCCESS && output != NULL && strcmp(output, "hallo\n") == 0);
    free(output);

    write_raw(path, "hallo");
    output = read_captured(path, &result);
    check("read: ohne Zeilenumbruch wird einer ergaenzt",
          result == SHELL_STATUS_SUCCESS && output != NULL && strcmp(output, "hallo\n") == 0);
    free(output);

    write_raw(path, "");
    output = read_captured(path, &result);
    check("read: leere Datei -> keine Ausgabe",
          result == SHELL_STATUS_SUCCESS && output != NULL && strcmp(output, "") == 0);
    free(output);

    /* groesser als der 4096-Byte-Puffer */
    enum { BIG = 10000 };
    char *big = malloc(BIG + 2);
    if (big != NULL)
    {
        memset(big, 'x', BIG);
        big[BIG] = '\n';
        big[BIG + 1] = '\0';
        write_raw(path, big);
        output = read_captured(path, &result);
        check("read: Datei groesser als der Puffer",
              result == SHELL_STATUS_SUCCESS && output != NULL && strcmp(output, big) == 0);
        free(output);
        free(big);
    }
    else
    {
        check("read: Speicher fuer Grosstest", false);
    }

    char missing[300];
    make_path(missing, sizeof(missing), "fehlt.txt");
    silence_stderr();
    output = read_captured(missing, &result);
    restore_stderr();
    check("read: Datei fehlt -> Fehler", result == SHELL_STATUS_ERROR);
    free(output);

    silence_stderr();
    output = read_captured(test_dir, &result);
    restore_stderr();
    check("read: Pfad ist ein Ordner -> Fehler", result == SHELL_STATUS_ERROR);
    free(output);

    silence_stderr();
    int null_result = file_read(NULL);
    restore_stderr();
    check("read: NULL -> Fehler", null_result == SHELL_STATUS_ERROR);
}

static void test_delete(void)
{
    char path[300];
    make_path(path, sizeof(path), "delete.txt");
    write_raw(path, "x");

    check("delete: Datei wird geloescht",
          file_delete(path) == SHELL_STATUS_SUCCESS && !exists(path));

    silence_stderr();
    int missing = file_delete(path);
    restore_stderr();
    check("delete: Datei fehlt -> Fehler", missing == SHELL_STATUS_ERROR);

    char dir[300];
    make_path(dir, sizeof(dir), "ordner_fuer_delete");
    mkdir(dir, 0755);
    silence_stderr();
    int on_dir = file_delete(dir);
    restore_stderr();
    check("delete: Ordner wird nicht geloescht",
          on_dir == SHELL_STATUS_ERROR && is_directory(dir));

    silence_stderr();
    int null_result = file_delete(NULL);
    restore_stderr();
    check("delete: NULL -> Fehler", null_result == SHELL_STATUS_ERROR);
}

static void test_delete_directory(void)
{
    char empty_dir[300];
    make_path(empty_dir, sizeof(empty_dir), "leer");
    mkdir(empty_dir, 0755);
    check("delete_directory: leerer Ordner",
          file_delete_directory(empty_dir) == SHELL_STATUS_SUCCESS && !exists(empty_dir));

    char full_dir[300];
    char inner[300];
    make_path(full_dir, sizeof(full_dir), "voll");
    mkdir(full_dir, 0755);
    make_path(inner, sizeof(inner), "voll/datei.txt");
    write_raw(inner, "x");
    silence_stderr();
    int full_result = file_delete_directory(full_dir);
    restore_stderr();
    check("delete_directory: Ordner mit Inhalt bleibt bestehen",
          full_result == SHELL_STATUS_ERROR && is_directory(full_dir) && exists(inner));

    silence_stderr();
    int on_file = file_delete_directory(inner);
    restore_stderr();
    check("delete_directory: Pfad ist eine Datei -> Fehler",
          on_file == SHELL_STATUS_ERROR && exists(inner));

    silence_stderr();
    int null_result = file_delete_directory(NULL);
    restore_stderr();
    check("delete_directory: NULL -> Fehler", null_result == SHELL_STATUS_ERROR);
}

static void test_open_errors(void)
{
    char missing[300];
    make_path(missing, sizeof(missing), "fehlt.txt");

    silence_stderr();
    int null_result = file_open(NULL);
    int empty_result = file_open("");
    int missing_result = file_open(missing);
    restore_stderr();

    check("open: NULL -> Fehler", null_result == SHELL_STATUS_ERROR);
    check("open: leerer Pfad -> Fehler", empty_result == SHELL_STATUS_ERROR);
    check("open: Datei fehlt -> Fehler", missing_result == SHELL_STATUS_ERROR);
}

static void test_handle_file_command(void)
{
    char path[300];
    make_path(path, sizeof(path), "cmd.txt");

    char *no_option[] = {"file", NULL};
    char *unknown[] = {"file", "-x", path, NULL};
    char *create_ok[] = {"file", "-c", path, NULL};
    char *create_many[] = {"file", "-c", path, "extra", NULL};
    char *create_none[] = {"file", "-c", NULL};
    char *write_ok[] = {"file", "-w", path, "hallo", NULL};
    char *write_no_text[] = {"file", "-w", path, NULL};
    char *write_many[] = {"file", "-w", path, "a", "b", NULL};
    char *append_ok[] = {"file", "-a", path, "!", NULL};
    char *append_no_text[] = {"file", "-a", path, NULL};
    char *read_ok[] = {"file", "-r", path, NULL};
    char *delete_ok[] = {"file", "-d", path, NULL};
    char *delete_many[] = {"file", "-d", path, "extra", NULL};
    char *delete_dir_none[] = {"file", "-D", NULL};

    silence_stderr();
    int r_no_option = handle_file_command(1, no_option);
    int r_unknown = handle_file_command(3, unknown);
    int r_create_many = handle_file_command(4, create_many);
    int r_create_none = handle_file_command(2, create_none);
    int r_write_no_text = handle_file_command(3, write_no_text);
    int r_write_many = handle_file_command(5, write_many);
    int r_append_no_text = handle_file_command(3, append_no_text);
    int r_delete_many = handle_file_command(4, delete_many);
    int r_delete_dir_none = handle_file_command(2, delete_dir_none);
    restore_stderr();

    check("file: ohne Option -> Fehler", r_no_option == SHELL_STATUS_ERROR);
    check("file: unbekannte Option -> Fehler", r_unknown == SHELL_STATUS_ERROR);
    check("file -c: zu viele Argumente -> Fehler", r_create_many == SHELL_STATUS_ERROR);
    check("file -c: ohne Pfad -> Fehler", r_create_none == SHELL_STATUS_ERROR);
    check("file -w: ohne Text -> Fehler", r_write_no_text == SHELL_STATUS_ERROR);
    check("file -w: zu viele Argumente -> Fehler", r_write_many == SHELL_STATUS_ERROR);
    check("file -a: ohne Text -> Fehler", r_append_no_text == SHELL_STATUS_ERROR);
    check("file -d: zu viele Argumente -> Fehler", r_delete_many == SHELL_STATUS_ERROR);
    check("file -D: ohne Pfad -> Fehler", r_delete_dir_none == SHELL_STATUS_ERROR);
    check("file: Fehlerfaelle haben keine Datei angelegt", !exists(path));

    check("file -c: legt Datei an",
          handle_file_command(3, create_ok) == SHELL_STATUS_SUCCESS && exists(path));
    check("file -w: schreibt Text",
          handle_file_command(4, write_ok) == SHELL_STATUS_SUCCESS &&
          file_has_content(path, "hallo"));
    check("file -a: haengt Text an",
          handle_file_command(4, append_ok) == SHELL_STATUS_SUCCESS &&
          file_has_content(path, "hallo!"));

    silence_stdout();
    int r_read = handle_file_command(3, read_ok);
    restore_stdout();
    check("file -r: Rueckgabewert", r_read == SHELL_STATUS_SUCCESS);

    check("file -d: loescht Datei",
          handle_file_command(3, delete_ok) == SHELL_STATUS_SUCCESS && !exists(path));
}

int main(void)
{
    if (!setup_test_dir())
    {
        perror("mkdtemp");
        return 1;
    }

    test_create();
    test_write();
    test_append();
    test_read();
    test_delete();
    test_delete_directory();
    test_open_errors();
    test_handle_file_command();

    cleanup_test_dir();

    if (failed == 0)
    {
        printf("\033[1;32m%-14s: %d passed, %d failed\033[0m\n", "File-Tests", passed, failed);
        return 0;
    }

    printf("\033[1;31m%-14s: %d passed, %d failed\033[0m\n", "File-Tests", passed, failed);
    return 1;
}