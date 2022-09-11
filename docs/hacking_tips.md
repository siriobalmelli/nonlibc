---
title: Hacking Tips
order: 3
---

# Hacking Tips

A quick index of useful concepts for would-be contributors
(and myself every few months).

## Grokking the codebase

Set yourself up so you can look up and jump to/from functions/symbols
inside your text editor or IDE.
This is [my personal setup with vim](https://github.com/siriobalmelli/toolbench).

Then start looking at the example code and related comments in the [test](../test) dir;
you'll be able to jump to function definitions/implementations
and read their documentation in context with the usage in the test.

Communication is always welcome, feel free to send a pull request
or drop me a line at <sirio.bm@gmail.com>.

## Style

1. Use [clang-format](https://clang.llvm.org/docs/ClangFormat.html),
    see [.clang-format](../.clang-format) for formatting info.

1. All hail the [Linux kernel coding style](https://01.org/linuxgraphics/gfx-docs/drm/process/coding-style.html)
    - tabs!

1. Keep things clean by prefixing function groups with 4-letter faux-namespaces.

    Example set of functions for a fictional library:

    ```c
    /*	 usls = the Useless Library
    */

    void	*usls_new();
    int		usls_do(void *usls);
    void	usls_free(void *usls);
    ```

1. Handle end-of-function cleanup with GOTOs:
    - when in doubt, have a function return a status code as `int` : `0 == success`
    - when allocating, return a pointer: `NULL == fail`
    - when calling libc, probably best to return an FD: `-1 == fail`

1. Believe in the 80-character margin. Ignore it when it makes the code clearer.

1. Eschew the '_l' suffix convention found in `libc` and other places.
    Rather, explicitly suffix functions with '32' or '64' when multiple word
    lengths are being used.

1. Visibility is important: <https://gcc.gnu.org/wiki/Visibility>.

    See the relevant helper macros in [nonlibc.h](../include/nonlibc.h)

1. Use test programs to show intended usage.

1. Header files should list functions in the order in which they appear in the
    corresponding `.c` file.

1. Use more thoughtful `<stdint.h>` and `<inttypes.h>` types to declare and
    print variables in this library (portability).
