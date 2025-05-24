#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>

int should_count_file(const char *filename) {
    size_t len = strlen(filename);
    if (len > 2 && strcmp(filename + len - 2, ".c") == 0) return 1;
    if (len > 2 && strcmp(filename + len - 2, ".h") == 0) return 1;
    return 0;
}

int should_ignore_dir(const char *dirname) {
    return (
        strcmp(dirname, ".git") == 0 ||
        strcmp(dirname, ".svn") == 0 ||
        strcmp(dirname, "build") == 0 ||
        strcmp(dirname, "bin") == 0 ||
        strcmp(dirname, "obj") == 0
    );
}

long count_lines_in_file(const char *filepath) {
    FILE *f = fopen(filepath, "r");
    if (!f) {
        perror(filepath);
        return 0;
    }

    long lines = 0;
    int ch;
    while ((ch = fgetc(f)) != EOF) {
        if (ch == '\n') lines++;
    }

    fclose(f);
    return lines;
}

int walk_directory(const char *path, long *total_lines) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror(path);
        return -1;
    }

    struct dirent *entry;
    char fullpath[PATH_MAX];

    while ((entry = readdir(dir)) != NULL) {
        // Skip current and parent directories
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        struct stat st;
        if (stat(fullpath, &st) == -1) {
            perror(fullpath);
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            if (should_ignore_dir(entry->d_name)) continue;
            walk_directory(fullpath, total_lines);
        } else if (S_ISREG(st.st_mode)) {
            if (should_count_file(entry->d_name)) {
                long file_lines = count_lines_in_file(fullpath);
                printf("%6ld lines  %s\n", file_lines, fullpath);
                *total_lines += file_lines;
            }
        }
    }

    closedir(dir);
    return 0;
}

int main(void) {
    long total_lines = 0;
    if (walk_directory(".", &total_lines) == 0) {
        printf("\n=============================\n");
        printf("Total lines: %ld\n", total_lines);
    } else {
        fprintf(stderr, "Error walking the directory tree.\n");
    }

    return 0;
}
