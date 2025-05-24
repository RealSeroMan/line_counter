<p align="center">
  <img src="linebolt.png" alt="linebolt logo" width="200"/>
</p>

<p align="center"><b>The lightning-fast line counter for source projects.</b></p>

# linebolt — Blazing-Fast Line Counter for Source Projects

**linebolt** is a lightweight, high-performance command-line tool written in C that traverses directories and counts the total number of lines in source code files — fast. Originally designed for `.c` and `.h` files, it's engineered to scale to massive codebases like the Linux kernel, and flexible enough to support any extension (`.py`, `.cpp`, `.yaml`, etc.).

* 10× faster than `cloc` on large codebases  
* Handles massive trees like the Linux kernel in under 10 seconds  
* Accurate even for files without a trailing newline  
* POSIX-compatible (Linux, macOS)

---

## Features
* Counts lines in all `.c` and `.h` files by default
* Non-recursive traversal using an internal stack (no malloc, no crashes)
* Skips irrelevant directories (`.git`, `build`, `bin`, etc.)
* Correctly counts files without final newline (unlike `wc -l`)
* Ignores empty files (zero-character files)
* Designed for Linux and other POSIX systems
* Ultra fast — C standard library only

Future roadmap:
* [ ] Custom extension filtering (`--ext py,cpp`)
* [ ] Blank/comment line exclusion
* [ ] JSON or CSV output mode
* [ ] Per-directory summaries

## Build Instructions
You need a POSIX-compatible system (Linux, macOS) and GCC or Clang installed.

### Compile
```bash
gcc -Wall -Wextra -o linebolt linebolt.c
```

### Run
To count all `.c` and `.h` lines in the current directory:

```bash
./linebolt
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
By default, `linebolt` skips common non-source folders:
* `.git`
* `.svn`
* `bin`
* `build`
* `obj`  

You can customize this in `should_ignore_dir()` in the source code.

## License
MIT License (see [LICENSE](LICENSE))

Author: [Zülfü Serhat Kük](https://github.com/RealSeroMan)  
Year: 2025

## Contributions
Want to extend it to support other languages or formats? Want blazing-fast CSV or JSON output? Open a pull request or submit an issue.
