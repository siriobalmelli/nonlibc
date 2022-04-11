#ifndef ndebug_h_
#define ndebug_h_

/*	ndebug.h
 * The nonlibc debug/info print toolkit.
 * Choice of prefix "NB" is a nod to Latin "Nota Bene" but can also stand for
 * "Nonlibc Buffer" or just "NdeBug" (your pick).
 *
 *
 * Table of statements:
 * | PREFIX  | FD     | NDEBUG* | STATEMENTS            | SIDE EFFECTS         |
 * | ------- | ------ | ------- | --------------------- | -------------------- |
 * | INF     | fdout  | NOP     | NB_inf(), NB_dump()   |                      |
 * | PRN     | fdout  |         | NB_prn(), NB_line()   |                      |
 * | WRN     | fderr  | NOP     | NB_wrn(), NB_wrn_if() |                      |
 * | ERR     | fderr  |         | NB_err(), NB_err_if() | err_cnt++;           |
 * | DIE     | fderr  |         | NB_die(), NB_die_if() | err_cnt++; goto die; |
 * * = if NDEBUG is defined, certain statements are NOPs (discarded by cpp).
 *
 *
 * ON THE PHILOSOPHY OF GOTOs
 *
 * This library provides NB_die() and NB_die_if() statements which:
 * - output an error message
 * - increment an 'err_cnt' variable
 * - `goto out`
 *
 * If you haven't spent a fair amount in the trenches writing C,
 * you may still have jammed in your head the mantra of
 * "You must never goto, Simba".
 * This point of view is understandable, but will get you in trouble
 * when it comes to error handling.
 *
 * Somewhere in the innards of a function, you may encounter an error
 * (you DO check the return values of the functions you call, right?),
 * or you may yourself decide something isn't kosher.
 * I wish to posit that THE right thing to do here is a goto.
 *
 * Are you looking at your screen in horror? Good - I have your attention.
 *
 * Let me explain:
 * You see, the function you're in the middle of has likely caused at least a couple
 * state changes: you may have allocated memory, opened a file descriptor,
 * written data somewhere, etc.
 * If there's multiple places in your function where you check a value for error;
 * it's very tricky and kludgy to write bulletproof error handling code
 * in each one of these locations.
 *
 * So how about this instead:
 * - put ONE block of error-handling instructions at the bottom
 *   of your function (usually under the return statement)
 * - label that block `die:`
 * - jump there any time something bad happens
 *
 * "Hey! that sounds like a Try/Catch block in [favorite language]!"
 *
 * Precisely.
 * (c) 2018 Sirio Balmelli
 */

#include <errno.h>
#include <stdio.h>
#include <libgen.h>	/* basename() */
#include <string.h>	/* strerror() */
#include <inttypes.h>	/* PRIu64 etc. for reliable printf across platforms */
#include "nonlibc.h"


/* "global" (per-translation-unit) variables.
 * These can be ignored for basic functionality,
 * or overidden on a per-function basis.
 */
static __attribute__((unused)) int err_cnt = 0;
static int fdout;
static int fderr;

/* output function can be overridden, but at compile-time only */
#ifndef NB_PRN
#define NB_PRN dprintf
#endif


/*	ND_start()
 * Initializer to set runtime variables.
*/
static void __attribute__ ((constructor)) ND_start()
{
	fdout = fileno(stdout);
	fderr = fileno(stderr);
}


/*	NB_LOG()
 * Raw call to NB_PRN, with extra instrumentation if program is in DEBUG mode.
 * Not usually called directly.
 * All other functions (with the exception of NB_dump())
 * eventually call this one.
 * NOTE that a newline '\n' is always appended when printing
 * (you don't have to include a newline in MESSAGE).
 */
#ifndef NDEBUG
/* Default (DEBUG) mode:
 * - prefix output lines with PREFIX
 * - print instrumentation
 * - 'errno' is printed if set, AND THEN RESET.
 */
#define NB_LOG(FD, PREFIX, MESSAGE, ...)					\
	NB_PRN(FD, "[%s] %10s:%03d +%-15s\t:: " MESSAGE "\n",			\
		PREFIX, basename(__FILE__),					\
		__LINE__, __FUNCTION__,						\
		##__VA_ARGS__);							\
	if (errno) {								\
		NB_PRN(FD, "\t!errno: 0x%02x|%s!\n", errno, strerror(errno));	\
		errno = 0;							\
	}

#else
/* If NDEBUG (no debugging) is set:
 * - ignore PREFIX
 * - elide file, line, function and errno info
 * - avoid logging at all if message is null
 */
#define NB_LOG(FD, PREFIX, MESSAGE, ...)					\
	if (MESSAGE[0] != '\0') {						\
		NB_PRN(FD, MESSAGE "\n", ##__VA_ARGS__);			\
	}
#endif


/*	NB_inf()
 * Print INFO with "INF" prefix, becomes NOP if NDEBUG is defined.
 */
#ifndef NDEBUG
#define NB_inf(INFO, ...) \
	NB_LOG(fdout, "INF", INFO, ##__VA_ARGS__)
#else
#define NB_inf(INFO, ...) ;
#endif

/*	NB_dump()
 * Dump BUF_LEN bytes from BUF, prefixed by INFO message;
 * becomes a NOP if NDEBUG is defined.
 * Dump is (losely speaking) in xxd format.
 */
#ifndef NDEBUG
#define NB_dump(BUF, BUF_LEN, INFO, ...)					\
	do {									\
		NB_LOG(fdout, "INF", INFO, ##__VA_ARGS__);			\
		size_t nb_i = 0;						\
		for (; nb_i < BUF_LEN; nb_i++) {				\
			/* starting address at the head of each line */		\
			if (!(nb_i & 0xf))					\
				NB_PRN(fdout, "%08zx:", nb_i);			\
			/* space between groups of 2 bytes */			\
			if (!(nb_i & 0x1))					\
				NB_PRN(fdout, " ");				\
			/* pad bytes to 2 hex digits always */			\
			NB_PRN(fdout, "%02hhx", ((const char*)BUF)[nb_i]);	\
			/* new line after 16 bytes */				\
			if ((nb_i & 0xf) == 0xf)				\
				NB_PRN(fdout, "\n");				\
		}								\
		/* corner case: avoid dual-newline when dumping an even multiple\
		 * of 16.							\
		 */								\
		if ((nb_i & 0xf) != 0xf)					\
			NB_PRN(fdout, "\n");					\
	} while(0);

#else
#define NB_dump(BUF, BUF_LEN, INFO, ...) ;
#endif


/*	NB_prn()
 * Regular program output.
 * Use this one for runtime messages which should be present (minus instrumentation)
 * in the NDEBUG (aka release) version.
 */
#define NB_prn(OUTPUT, ...) \
	NB_LOG(fdout, "PRN", OUTPUT, ##__VA_ARGS__)

/*	NB_line()
 * Prints a literal separator line.
 */
#define NB_line() \
	NB_LOG(fdout, "PRN", "------------")


/*	NB_wrn()
 * Print WARNING as "WRN"; is a NOP in release (when NDEBUG is defined).
 */
#ifndef NDEBUG
#define NB_wrn(WARNING, ...) \
	NB_LOG(fderr, "WRN", WARNING, ##__VA_ARGS__)
#else
#define NB_wrn(WARNING, ...) ;
#endif

/*	NB_wrn_if()
 * Execute NB_wrn() if CONDITION is true.
 * Tells compiler *not* to expect 'true' (puts warning in cold path).
 * Test is NOT performed in release (NDEBUG) builds.
 */
#ifndef NDEBUG
#define NB_wrn_if(CONDITION, WARNING, ...)					\
	if (NLC_UNLIKELY(CONDITION)) {					\
		NB_wrn(WARNING, ##__VA_ARGS__);					\
	}
#else
#define NB_wrn_if(CONDITION, WARNING, ...) ;
#endif


/*	NB_err()
 * Print ERROR as "ERR"; increment 'err_cnt'.
 * Is a full memory barrier.
 */
#define NB_err(ERROR, ...)							\
	do {									\
		NB_LOG(fderr, "ERR", ERROR, ##__VA_ARGS__);			\
		__atomic_add_fetch(&err_cnt, 1, __ATOMIC_SEQ_CST);		\
	} while (0)

/*	NB_err_if()
 * Test CONDITION and call NB_err() if true.
 */
#define NB_err_if(CONDITION, ERROR, ...)					\
	if (NLC_UNLIKELY(CONDITION)) {					\
		NB_err(ERROR, ##__VA_ARGS__);					\
	}


/*	NB_die()
 * Print DEATH as "DIE"; increment 'err_cnt'; jump (goto) 'die'.
 * Is a full memory barrier.
 */
#define NB_die(DEATH, ...)							\
	do {									\
		NB_LOG(fderr, "DIE", DEATH, ##__VA_ARGS__);			\
		__atomic_add_fetch(&err_cnt, 1, __ATOMIC_SEQ_CST);		\
		goto die;							\
	} while (0)

/*	NB_die_if()
 * Test CONDITION and call NB_die() if true.
 */
#define NB_die_if(CONDITION, DEATH, ...)					\
	if (NLC_UNLIKELY(CONDITION)) {					\
		NB_die(DEATH, ##__VA_ARGS__);					\
	}


#endif /* ndebug_h_ */
