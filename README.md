# line_counter — Recursive Line Counter for C Projects

`line_counter` is a lightweight, fast command-line tool written in C that recursively traverses directories and counts the lines in `.c` and `.h` source files. It's designed for C developers who want a simple alternative to heavier tools or scripting languages.

## Features
* Counts lines in `.c` and `.h` files
* Recursively walks through directories
* Skips irrelevant directories like `.git`, `build`, `bin`, `etc`.
* Ignores empty files
* Correctly handles files without trailing newlines (unlike `wc -l`)
* Designed for Linux and other POSIX-compliant systems

## Build Instructions
You need a POSIX-compatible system (Linux, macOS) and GCC or Clang installed.

### Compile
```bash
gcc -Wall -Wextra -o line_counter line_counter.c
```

### Run
To count all `.c` and `.h` lines in the current directory:

```bash
./line_counter
```

Output includes line counts per file and a total at the end.

### Example Output
```   23 lines  ./src/main.c
   12 lines  ./include/util.h
    9 lines  ./src/helpers.c

=============================
Total lines: 44 
```

## Directory Filtering
The tool automatically skips the following directories:
* `.git`
* `.svn`
* `bin`
* `build`
* `obj`  

You can customize this in `should_ignore_dir()` in the source code.

## License
MIT License (see bottom of `line_counter.c`)

Author: Zülfü Serhat Kük  
Year: 2025

## Contributions
If you’d like to contribute improvements (e.g. extension filtering, blank line exclusion, JSON output), feel free to fork and open a pull request!
