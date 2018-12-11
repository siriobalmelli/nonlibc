/*	ndebug_test.c
 * Show usage of NB macros in ndebug.h
 * (c) 2018 Sirio Balmelli
 */

#include <ndebug.h>
#include <stdlib.h>


/* Tell sanitizers not to freak out when we deliberately fail a malloc()
 * as part of the tests below.
 */
const char *__asan_default_options()
{
	return "allocator_may_return_null=1";
}
const char *__tsan_default_options()
{
	return "allocator_may_return_null=1";
}


/*	fail()
 * Always fail.
*/
int fail()
{
	int err_cnt = 0;
	NB_die("should always fail");
die:
	return err_cnt;
}


/*	basic_function()
 * In the majority of cases, a function returns an int,
 * which should be 0 to indicate success.
 * We follow this convention by declaring a local 'err_cnt' variable
 * and using the NB macros to test for errors and print them if they occur;
 * then just return 'err_cnt'.
 * Caller can test return value and act accordingly.
*/
int basic_function()
{
	int err_cnt = 0;

	/* test for an error condition;
	 * print a message and increment error count if test evaluates to true.
	 */
	NB_err_if(1 != 1,
		"this will never error; err_cnt is %d",
		err_cnt);

	/* print an error (and increase err_cnt) but continue execution */
	NB_err("this is an expected error, continue");

	/* test true and jump to 'die' */
	NB_die_if(1, "this is an expected but unrecoverable error, die");

	/* It would be an error to ever arrive here - the preceding,
	 * error should have jumped to 'out'.
	 */
	exit(1);

die:
	return err_cnt;
}


/* Struct to illustrate usage in malloc_error()
*/
struct	malloc_err_example {
	void	*ptr_a;
	void	*ptr_b;
};

/*	malloc_error()
 * Show use of NB macros in a function which allocates memory.
 * Such a function usually returns a pointer to the allocated memory, NULL on error.
 * The goal is to graciously handle malloc() or other failures without risking
 * a memory leak.
 */
struct malloc_err_example *malloc_error()
{
	/* Declare any values which will be used below 'die:'
	 * BEFORE any NB_die_if() calls.
	 */
	struct malloc_err_example *ret;

	/* Assign AND test in the same construct; give an error on fail */
	NB_die_if(!(
		ret = malloc(sizeof(struct malloc_err_example))
		), "could not malloc() %zu bytes", sizeof(struct malloc_err_example));

	/* Assign inner pointers - the second of these allocs should fail
	 * because it's an insane amount of memory.
	 */
	NB_die_if(!(
		ret->ptr_a = malloc(sizeof(uint64_t))
		), "could not malloc() %zu bytes", sizeof(uint64_t));
	NB_die_if(!(
		ret->ptr_b = malloc(1UL << (sizeof(long) * 8 - 1))
		), "could not malloc() %lu (insane amount) bytes",
		1UL << (sizeof(long) * 8 - 1));

	/* successful return */
	return ret;

die:
	/* Bad things happened.
	 * Clean up any possible dirty resources (allocs, FDs, etc),
	 * then return failure value.
	 * NOTE: passing NULL to free() is perfectly permissible.
	 * By explicitly free()ing all pointers, we are certain never to leak.
	 */
	if (ret) {
		free(ret->ptr_a);
		free(ret->ptr_b);
	}
	free(ret);
	return NULL;
}


/*	main()
*/
int main()
{
	/* This will be incremented by any calls to
	 * NB_err(), NB_err_if(), NB_die(), NB_die_if()
	 */
	int err_cnt = 0;

	/* Print: the current function name as an "info" loglevel;
	 * 'NB_inf' 'NB_wrn' and 'NB_err' levels are always enabled by default.
	 */
	NB_inf("current function: %s()", __FUNCTION__);

	/* Declare all variables which might be used in 'die:'
	 * BEFORE any NB_die_if() calls.
	 * Always initialize them to their "failure values".
	 */
	int res = 0;
	void *a_pointer = NULL;
	void *a_malloc = NULL;

	/* Show error handling when calling a "basic" function
	 * (which in the UNIX tradition returns 0 on success.
	 * NOTE we assign a return value AND test it at the same time
	 * and we leave the assignment on it's own line to make this obvious.
	 */
	NB_die_if((
		res = basic_function()
		) != 2, "sum ting willy wong heah");

	/* standard printing facility */
	NB_prn( "res is %d", res);
	NB_line();

	NB_wrn("this warning visible only when NDEBUG is not defined");
	NB_wrn_if(1, "generic warning should always warn");

	/* Show error handling for a function which allocates
	 * (potentially multiple) memory regions.
	 */
	NB_die_if((
		a_pointer = malloc_error()
		) != NULL, "we expected this to fail");

	/* malloc (with safety check); then print a buffer */
	const size_t len = 125;
	NB_die_if(!(
		a_malloc = malloc(len)
		), "len = %zu", len);
	NB_dump(a_malloc, len,
		"printing buffer len %zu", len);

	/* test control logic */
	NB_die_if(!fail(), "should NEVER succeed");

die:
	/* Regardless of whether function succeeded or not,
		make sure all things are cleaned up.
	*/
	free(a_pointer); /* free(NULL) is perfectly OK */
	free(a_malloc);
	return err_cnt;;
}
