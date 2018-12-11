#ifndef ndebug_h_
#define ndebug_h_

/*	ndebug.h
 * The nonlibc debug/info print toolkit.
 * Choice of prefix "NB" is a nod to Latin "Nota Bene" but can also stand for
 * "Nonlibc Buffer" or just "NdeBug" (your pick).
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
 * (c) 2018 Sirio Balmelli
 */

#include <errno.h>
#include <stdio.h>
#include <libgen.h>	/* basename() */
#include <string.h>	/* strerror() */
#include <inttypes.h>	/* PRIu64 etc. for reliable printf across platforms */


/* "global" (per-translation-unit) variables.
 * These can be ignored for basic functionality,
 * or overidden on a per-function basis.
 */
static int err_cnt = 0;
static int fdout;
static int fderr;

/* output function is overridden at compile-time only */
#define NB_PRN dprintf


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
#define NB_LOG(FD, PREFIX, MESSAGE, ...) \
	NB_PRN(FD, "[%s] %10s:%03d +%-15s\t:: " MESSAGE "\n", \
		PREFIX, basename(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__); \
	if (errno) { \
		NB_PRN(FD, "\t!errno: 0x%02x|%s!\n", errno, strerror(errno)); \
		errno = 0; \
	}

#else
/* If NDEBUG (no debugging) is set:
 * - ignore PREFIX
 * - elide file, line, function and errno info
 * - avoid logging at all if message is null
 */
#define NB_LOG(FD, PREFIX, MESSAGE, ...) \
	NB_PRN(FD, MESSAGE "\n", ##__VA_ARGS__);
#endif


/*	NB_inf()
 * Print INFO with "INF" prefix, becomes NOP if NDEBUG is defined.
 */
#ifndef NDEBUG
#define NB_inf(INFO, ...) \
	NB_LOG(fdout, "INF", INFO, ##__VA_ARGS__)
#else
#define NB_inf(INFO, ...) \
	;
#endif

/*	NB_dump()
 * Dump BUF_LEN bytes from BUF, prefixed by INFO message;
 * becomes a NOP if NDEBUG is defined.
 */
#ifndef NDEBUG
#define NB_dump(BUF, BUF_LEN, INFO, ...) \
	NB_LOG(fdout, "INF", INFO, ##__VA_ARGS__); \
	for (size_t nb_i = 0; nb_i < BUF_LEN; nb_i++) { \
		/* new line every 8 bytes (aka: nb_i % 8) */ \
		if (!(nb_i & 0x7)) \
			NB_PRN(fdout, "\n"); \
		NB_PRN(fdout, "0x%hhx\t", ((const char*)BUF)[nb_i]); \
	} \
	NB_PRN(fdout, "\n"); \

#else
#define NB_dump(BUF, BUF_LEN, INFO, ...) \
	;
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
#define NB_wrn(WARNING, ...) \
	;
#endif

/*	NB_wrn_if()
 * Execute NB_wrn() if CONDITION is true.
 * Tells compiler *not* to expect 'true' (puts warning in cold path).
 * Test is NOT performed in release (NDEBUG) builds.
 */
#ifndef NDEBUG
#define NB_wrn_if(CONDITION, WARNING, ...) \
	if (__builtin_expect(CONDITION, 0)) { \
		NB_wrn(WARNING, ##__VA_ARGS__); \
	}
#else
#define NB_wrn_if(CONDITION, WARNING, ...) \
	;
#endif


/*	NB_err()
 * Print ERROR as "ERR"; increment 'err_cnt'.
 * Is a full memory barrier.
 */
#define NB_err(ERROR, ...) \
	do { \
		NB_LOG(fderr, "ERR", ERROR, ##__VA_ARGS__); \
		__atomic_add_fetch(&err_cnt, 1, __ATOMIC_SEQ_CST); \
	} while (0)

/*	NB_err_if()
 * Test CONDITION and call NB_err() if true.
 */
#define NB_err_if(CONDITION, ERROR, ...) \
	if (__builtin_expect(CONDITION, 0)) { \
		NB_err(ERROR, ##__VA_ARGS__); \
	}


/*	NB_die()
 * Print DEATH as "DIE"; increment 'err_cnt'; jump (goto) 'die'.
 * Is a full memory barrier.
 */
#define NB_die(DEATH, ...) \
	do { \
		NB_LOG(fderr, "DIE", DEATH, ##__VA_ARGS__); \
		__atomic_add_fetch(&err_cnt, 1, __ATOMIC_SEQ_CST); \
		goto die; \
	} while (0)

/*	NB_die_if()
 * Test CONDITION and call NB_die() if true.
 */
#define NB_die_if(CONDITION, DEATH, ...) \
	if (__builtin_expect(CONDITION, 0)) { \
		NB_die(DEATH, ##__VA_ARGS__); \
	}


#endif /* ndebug_h_ */
