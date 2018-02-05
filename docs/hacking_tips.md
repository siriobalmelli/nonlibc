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

As an example, I use [vim](http://www.vim.org/) with
	[cscope](http://cscope.sourceforge.net/) support,
	and you'll see that [bootstrap.py](./bootstrap.py) generates
	cscope files from the sources.

Then start looking at the example code and related comments in the `test` dir;
	you'll be able to jump to function definitions/implementations
	and read their documentation in context with the usage in the test.

Communication is always welcome, feel free to send a pull request
	or drop me a line at <sirio.bm@gmail.com>.

### You Complete Me

The author uses [YCM](https://github.com/Valloric/YouCompleteMe).

You will notice that the [ycm config file](.ycm_extra_conf.py)
	contains `compilation_database_folder='./build-debug'`.

A successful build by ninja yields the file `build-debug/compile_commands.json`,
	which means YCM should pretty much work out-of-the-box.

## Style

-	tabs!
-	tabs are 8 spaces wide
-	all hail the [Linux kernel coding style](https://01.org/linuxgraphics/gfx-docs/drm/process/coding-style.html)

-	keep things clean by prefixing function groups with 4-letter faux-namespaces
	Example set of functions for a fictional library:

```c
/*	 usls = the Useless Library
*/

void	*usls_new();
int		usls_do(void *usls);
void	usls_free(void *usls);
```

-	handle end-of-function cleanup with GOTOs
-	when in doubt, have a function return a status code as `int` : `0 == success`
-	when allocating, return a pointer: `NULL == fail`
-	when calling libc, probably best to return an FD: `-1 == fail`

-	Comments formatting:

```c
/* A proper comment block:
Lines start on the tab.
Long lines are split
	and indented like code.
*/
```

-	Believe in the 80-character margin. Ignore it when it makes the code clearer.

-	Eschew the '_l' suffix convention found in `libc` and other places.
	Rather, explicitly suffix functions with '32' or '64' when multiple word
		lengths are being used.
-	`.c` files only #include their related `.h` file, which includes anything else.
-	Visibility is important: <https://gcc.gnu.org/wiki/Visibility> - see the
		relevant helper macros in `nonlibc.h`.
	Also, feel free to suffix internal helper functions with an underscore '_'
		as a hint to other coders.
-	use test programs to show intended usage
-	Header files should list functions in the order in which they appear in the
		corresponding `.c` file.
-	Use more thoughtful `<stdint.h>` and `<inttypes.h>` types to declare and
		print variables in this library (portability constraint).
