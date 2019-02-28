#ifndef nonlibc_h_
#define nonlibc_h_

/*	nonlibc.h	nonlibc's generic header file
 *
 * This header file exports defines and macros to clean up ugly code and abstract
 * away compiler- or platform-dependent functionality.
 *
 * (c) 2017 Sirio Balmelli; https://b-ad.ch
 */


/* TODO: not true on all systems, how to test this at compile-time? */
#define NLC_CACHE_LINE 64


/*	compile-time checks (size, etc)
 * A macro to allow for compile-time checks where the CPP is missing info,
 * such as sizeof().
 * Exploit the fact that a '0' bitfield throws a compiler error.
 */
#define NLC_ASSERT(name, expr) \
	struct name { unsigned int bitfield : (expr); }


/*	compile-time computation for number of array members
 */
#define NLC_ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))


/*	inlining!
 * Use this in header files when defining inline functions for use library callers.
 * The effect is the same as a macro, except these are actually legibile ;)
 */
#define	NLC_INLINE static inline __attribute__((always_inline))



/*	visibility!
 * Visibility is important for motives of cleanliness, performance and bug-catching.
 * The recommended approach is:
 * - Declare ALL functions (even static helper functions not meant for use by caller)
 *   in header files.
 * - Prefix each declaration with a PUBLIC/LOCAL macro as defined below.
 *
 * And that's it.
 * You'll keep track of all your functions (and not lose "private/local" functions
 * buried somewhere in a .c file), BUT the exported symbols of your library
 * will be exactly and only that which you intend to export.
 */
#if (__GNUC__ >= 4) || defined(__clang__)
	#define NLC_PUBLIC __attribute__ ((visibility ("default")))
	#define NLC_LOCAL  __attribute__ ((visibility ("hidden")))
#else
	#define NLC_PUBLIC
	#define NLC_LOCAL
#endif


/*	branch hints
 */
#define NLC_UNLIKELY(test) (__builtin_expect(test, 0))


/*	benchmarking!
 * Use these macros to time segments of code, without:
 * - being tricked by compiler or CPU reordering
 * - dealing with a bunch of kludge
 */
#ifdef __APPLE__
/* OS X <10.12 doesn't have clock_gettime() ...
	and I can't find how to check for version :P
Fall back on less accurate methods.
*/
#include <sys/time.h>
#define nlc_timing_2u64(tp) \
	((uint64_t)tp.tv_usec + ((uint64_t)tp.tv_sec * 1000000))

#define nlc_timing_start(timer_name)						\
	struct timeval tp_##timer_name = { 0 };					\
	gettimeofday(&tp_##timer_name, NULL);					\
	clock_t cpu_##timer_name = clock();					\
	__atomic_thread_fence(__ATOMIC_SEQ_CST);

#define nlc_timing_stop(timer_name)						\
	__atomic_thread_fence(__ATOMIC_SEQ_CST);				\
	cpu_##timer_name = clock() - cpu_##timer_name;				\
	uint64_t wall_##timer_name = nlc_timing_2u64(tp_##timer_name);		\
	gettimeofday(&tp_##timer_name, NULL);					\
	wall_##timer_name = nlc_timing_2u64(tp_##timer_name) - wall_##timer_name;

/* wall clock time */
#define nlc_timing_wall(timer_name) \
	((double)wall_##timer_name / 1000000)

#else
#include <time.h>
#define nlc_timing_2u64(tp) \
	((uint64_t)(tp).tv_nsec + ((uint64_t)(tp).tv_sec * 1000000000))

#define nlc_timing_start(timer_name)						\
	struct timespec tp_##timer_name = { 0 };				\
	clock_gettime(CLOCK_MONOTONIC, &tp_##timer_name);			\
	clock_t cpu_##timer_name = clock();					\
	__atomic_thread_fence(__ATOMIC_SEQ_CST);

#define nlc_timing_stop(timer_name)						\
	__atomic_thread_fence(__ATOMIC_SEQ_CST);				\
	cpu_##timer_name = clock() - cpu_##timer_name;				\
	uint64_t wall_##timer_name = nlc_timing_2u64(tp_##timer_name);		\
	clock_gettime(CLOCK_MONOTONIC, &tp_##timer_name);			\
	wall_##timer_name = nlc_timing_2u64(tp_##timer_name) - wall_##timer_name;

/* wall clock time */
#define nlc_timing_wall(timer_name) \
	((double)wall_##timer_name / 1000000000)
#endif


/* CPU time in fractional seconds */
#define nlc_timing_cpu(timer_name) \
	((double)cpu_##timer_name / CLOCKS_PER_SEC) /* e.g.: with %.2lf printf() format */


/*	NLC_SWAP_EXEC
 *
 * Atomically exchange the value in 'var' with 'swap';
 * if the exchanged value is different from 'swap', pass it to 'exec'.
 * Can be used e.g.: in check()->free() logic looking at shared variables.
 *
 * var	:	variable name
 * swap	:	value e.g. '0' or 'NULL'
 * exec	:	function pointer e.g. 'free'
 */
#define NLC_SWAP_EXEC(var, swap, exec) {					\
	typeof(var) bits_temp_;							\
	bits_temp_ = (typeof(var))__atomic_exchange_n(&(var),			\
						(typeof(var))(swap),		\
						__ATOMIC_ACQUIRE);		\
	if (bits_temp_ != ((typeof(var))(swap))) {				\
		exec(bits_temp_);						\
		bits_temp_ = (typeof(var))(swap);				\
	}									\
}


/* Explicitly provide endianness macros, since 'endian.h' isn't everywhere
 * (I'm looking at you, Darwin BSD!).
 * Bonus: the implementation is GCC builtins :)
 * // #include <endian.h>
 * NOTE: of course, it's not that simple. Certain systems define _some_ but not
 * _all_ of these macros, so we need to explicitly check for and undef every one.
 */
#ifdef le16toh
#undef le16toh
#endif
#ifdef le32toh
#undef le32toh
#endif
#ifdef le64toh
#undef le64toh
#endif
#ifdef be16toh
#undef be16toh
#endif
#ifdef be32toh
#undef be32toh
#endif
#ifdef be64toh
#undef be64toh
#endif
#ifdef h16tole
#undef h16tole
#endif
#ifdef h32tole
#undef h32tole
#endif
#ifdef h64tole
#undef h64tole
#endif
#ifdef h16tobe
#undef h16tobe
#endif
#ifdef h32tobe
#undef h32tobe
#endif
#ifdef h64tobe
#undef h64tobe
#endif

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define le16toh(x) (x)
#define le32toh(x) (x)
#define le64toh(x) (x)
#define be16toh(x) __builtin_bswap16(x)
#define be32toh(x) __builtin_bswap32(x)
#define be64toh(x) __builtin_bswap64(x)
#define h16tole(x) (x)
#define h32tole(x) (x)
#define h64tole(x) (x)
#define h16tobe(x) __builtin_bswap16(x)
#define h32tobe(x) __builtin_bswap32(x)
#define h64tobe(x) __builtin_bswap64(x)

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define le16toh(x) __builtin_bswap16(x)
#define le32toh(x) __builtin_bswap32(x)
#define le64toh(x) __builtin_bswap64(x)
#define be16toh(x) (x)
#define be32toh(x) (x)
#define be64toh(x) (x)
#define h16tole(x) __builtin_bswap16(x)
#define h32tole(x) __builtin_bswap32(x)
#define h64tole(x) __builtin_bswap64(x)
#define h16tobe(x) (x)
#define h32tobe(x) (x)
#define h64tobe(x) (x)

#else
#error "Architecture endinanness not supported. Send espresso to maintainer"
#endif


#endif /* nonlibc_h_ */
