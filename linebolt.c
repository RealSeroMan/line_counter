/*
 * linebolt.c — A high-performance source line counter
 *
 * Traverses the current directory and all subdirectories (non-recursively),
 * counts the number of lines in `.c` and `.h` source files, and prints the
 * result along with per-file line counts. It skips common build or VCS
 * directories like `.git`, `bin`, and `build`.
 *
 * Implements a non-recursive depth-first search using a global stack.
 * Designed for POSIX-compliant systems (Linux, macOS).
 *
 * Uses only standard C system functions: `opendir`, `readdir`, `stat`, etc.
 *
 * Author: Zülfü Serhat Kük
 * Github: https://github.com/RealSeroMan
 * License: MIT
 * Year: 2025
 */


// Standard I/O library for printf(), fopen(), etc.
#include <stdio.h>

// For malloc(), free(), exit(), etc.
#include <stdlib.h>

// For string manipulation functions like strlen(), strcmp()
#include <string.h>

// For working with directories: DIR, struct dirent, opendir(), readdir(), closedir()
#include <dirent.h>

// For file metadata and file type macros like stat(), S_ISDIR(), S_ISREG()
#include <sys/stat.h>

// For system-defined path length limits (e.g., PATH_MAX)
#include <limits.h>

// For error reporting (used with perror())
#include <errno.h>

// Required for clock_gettime()
#include <time.h>

#define MAX_PATH_SIZE PATH_MAX
#define STACK_SIZE 200000

typedef struct {
    char path[MAX_PATH_SIZE];
} StackEntry;

// Declare time structs to capture start and end timestamps
struct timespec start, end;

static StackEntry stack[STACK_SIZE];
static int top = 0;


// Determines whether the file should be counted based on its extension
// Only .c and .h files are considered valid source files here
int should_count_file(const char *filename) {
    size_t len = strlen(filename);
    if (len > 2 && strcmp(filename + len - 2, ".c") == 0) return 1;
    if (len > 2 && strcmp(filename + len - 2, ".h") == 0) return 1;
    return 0;
}


// Determines whether a directory should be skipped during traversal
// Common build or version-control directories are excluded
int should_ignore_dir(const char *dirname) {
    return (
        strcmp(dirname, ".git") == 0 ||    // Git metadata directory
        strcmp(dirname, ".svn") == 0 ||    // Subversion metadata directory
        strcmp(dirname, "build") == 0 ||   // Common build output folder
        strcmp(dirname, "bin") == 0 ||     // Common binary output folder
        strcmp(dirname, ".vscode") == 0 || // VSCode config folder
        strcmp(dirname, "obj") == 0        // Common object file folder
    );
}


// Opens a text file and counts how many newline characters it contains
// This is used to determine the number of lines in a .c or .h file
long count_lines_in_file(const char *filepath) {
    FILE *f = fopen(filepath, "r"); // Open the file in read mode
    if (!f) {
        perror(filepath);   // Print an error message if opening fails
        return 0;             // Return 0 lines if file couldn't be opened
    }

    long lines = 0;
    int ch;
    int has_content = 0;            // Whether file has at least 1 non-empty char
    int last_char_was_newline = 0;  // Track if last char is a newline

    while ((ch = fgetc(f)) != EOF) {
        has_content = 1;            // File is not empty
        if (ch == '\n') {
            lines++;
            last_char_was_newline = 1;
        } else {
            last_char_was_newline = 0;
        }
    }

    fclose(f);  // MUST close the file

    // If file has content but does not end in newline, count the last line
    if (has_content && !last_char_was_newline)
        lines++;

    return lines;
}


// Performs a non-recursive depth-first traversal starting at 'start_path'
// Counts the total number of lines in all `.c` and `.h` files encountered
// Accumulates the result in the variable pointed to by 'total_lines'
int walk_directory(const char *start_path, long *total_lines) {
    // Ensure we have space in the traversal stack before starting
    if (top >= STACK_SIZE) {
        fprintf(stderr, "Initial stack overflow\n");
        return -1;
    }

    // Push the initial path onto the global stack
    snprintf(stack[top].path, MAX_PATH_SIZE, "%s", start_path);
    top++;

    // Loop while there are directories left to process
    while (top > 0) {
        top--;

        // Make a local copy of the current path (avoid pointer reuse issues)
        char path[MAX_PATH_SIZE];
        snprintf(path, sizeof(path), "%s", stack[top].path);

        // Attempt to open the directory
        DIR *dir = opendir(path);
        if (!dir) {
            perror(path);  // Print error and skip if directory can't be opened
            continue;
        }

        struct dirent *entry;
        char fullpath[MAX_PATH_SIZE];  // Buffer to hold full path to each entry

        // Iterate over entries in the current directory
        while ((entry = readdir(dir)) != NULL) {
            // Skip "." and ".." entries to avoid infinite recursion
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            // Build full path to the file or subdirectory
            snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

            // Retrieve file information (type, size, etc.)
            struct stat st;
            if (stat(fullpath, &st) == -1) {
                perror(fullpath);  // Print error if stat fails
                continue;
            }

            // If it's a directory, push it onto the stack to process later
            if (S_ISDIR(st.st_mode)) {
                if (should_ignore_dir(entry->d_name)) continue;

                if (top >= STACK_SIZE) {
                    fprintf(stderr, "Stack overflow\n");
                    continue;
                }

                snprintf(stack[top].path, MAX_PATH_SIZE, "%s", fullpath);
                top++;
            }

            // If it's a regular file and a .c or .h file, count its lines
            else if (S_ISREG(st.st_mode)) {
                if (should_count_file(entry->d_name)) {
                    long file_lines = count_lines_in_file(fullpath);
                    printf("%6ld lines  %s\n", file_lines, fullpath);
                    *total_lines += file_lines;
                }
            }
        }

        closedir(dir);
    }

    return 0;
}


// Entry point of the program
int main(void) {
    // Record the start time using a monotonic (non-wall) clock
    clock_gettime(CLOCK_MONOTONIC, &start);
    long total_lines = 0;

    // Start recursive directory traversal from current directory (".")
    // Accumulate total line count in total_lines
    if (walk_directory(".", &total_lines) == 0) {
        // If directory traversal succeeded, print final result
        printf("\n=============================\n");
        printf("Total lines: %ld\n", total_lines);
    } else {
        // If an error occurred, report it to stderr
        fprintf(stderr, "Error walking the directory tree.\n");
    }

    // Record the end time after processing completes
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Compute elapsed time in milliseconds
    double elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0 + \
                    (end.tv_nsec - start.tv_nsec) / 1e6;

    printf("\nExecution time: %.2f ms\n", elapsed_ms);

    return 0;  // Exit with success
}