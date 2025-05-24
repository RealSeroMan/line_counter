/*
 * Line Counter for C Projects
 * Recursively walks through the current directory and subdirectories,
 * counts the lines in .c and .h source files, and skips common irrelevant folders.
 *
 * Designed for POSIX systems (Linux, macOS).
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

    // Read the file character by character
    // Count each newline character as a line
    while ((ch = fgetc(f)) != EOF) {
        if (ch == '\n') lines++;
    }

    fclose(f);  // Always close the file when done
    return lines;
}



// Recursively traverses the directory at 'path'
// Counts the total number of lines in all .c and .h files
// Adds the result to the value pointed to by 'total_lines'
int walk_directory(const char *path, long *total_lines) {
    // Open the directory for reading
    // DIR is an opaque type representing a directory stream
    DIR *dir = opendir(path);
    if (!dir) {
        // Print an error if the directory can't be opened (e.g., permission denied)
        perror(path);
        return -1;    // Signal failure
    }

    struct dirent *entry;    // Used to hold info about each entry in the directory
    char fullpath[PATH_MAX]; // Buffer to store the full path of each entry

    // Iterate through entries in the directory one by one
    while ((entry = readdir(dir)) != NULL) {
        // Skip special entries "." (current dir) and ".." (parent dir) to prevent infinite recursion
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Construct the full path to the file or directory
        // Example: path = "src", entry->d_name = "main.c" → fullpath = "src/main.c"
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        struct stat st;
        // Retrieve file metadata into 'st'
        // This tells us whether the entry is a regular file, directory, symlink, etc.
        if (stat(fullpath, &st) == -1) {
            // If stat fails (e.g., broken symlink), print an error and skip it
            perror(fullpath);
            continue;
        }

        // If the entry is a directory, recurse into it
        if (S_ISDIR(st.st_mode)) {
            // Skip unwanted directories like .git/, bin/, etc.
            if (should_ignore_dir(entry->d_name)) continue;

            // Recursive call to process the subdirectory
            walk_directory(fullpath, total_lines);
        }

        // If the entry is a regular file (not a directory, symlink, etc.)
        else if (S_ISREG(st.st_mode)) {
            // Check if it's a source file we care about (.c or .h)
            if (should_count_file(entry->d_name)) {
                // Count the number of lines in the file
                long file_lines = count_lines_in_file(fullpath);

                // Print the line count and file path
                printf("%6ld lines  %s\n", file_lines, fullpath);

                // Add to the running total
                *total_lines += file_lines;
            }
        }
        // Other file types (symlinks, sockets, etc.) are silently skipped
    }

    // Close the directory stream to free resources
    closedir(dir);

    // Return success
    return 0;
}



// Entry point of the program
int main(void) {
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

    return 0;  // Exit with success
}
