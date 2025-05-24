#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

long count_lines_in_file(const char *filepath) {
    FILE *f = fopen(filepath, "r");
    if (!f) return 0;

    long lines = 0;
    int ch;
    while ((ch = fgetc(f)) != EOF) {
        if (ch == '\n') lines++;
    }
    fclose(f);
    return lines;
}


int should_count_file(const char *filename) {
    size_t len = strlen(filename);
    return (
        (len > 2 && strcmp(filename + len - 2, ".c") == 0) ||
        (len > 2 && strcmp(filename + len - 2, ".h") == 0)
    );
}


int walk_directory(const char *path, long *total_lines) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror(path);  // optional
        return -1;
    }

    struct dirent *entry;
    char fullpath[PATH_MAX];

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        struct stat st;
        if (stat(fullpath, &st) == -1) continue;

        if (S_ISDIR(st.st_mode)) {
            walk_directory(fullpath, total_lines);
        } else if (S_ISREG(st.st_mode)) {
            if (should_count_file(entry->d_name)) {
                *total_lines += count_lines_in_file(fullpath);
            }
        }
    }

    closedir(dir);
    return 0;
}


int main(void) {
    long total_lines = 0;
    walk_directory(".", &total_lines);
    printf("Total lines: %ld\n", total_lines);
    return 0;
}
