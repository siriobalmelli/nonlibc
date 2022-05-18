#ifndef posigs_h_
#define posigs_h_

/**
 *	posigs.h	POSix SIGnaling utilities
 *
 * Utility funtions for common POSIX signal handling patterns.
 * Originally appeared as the 'mtsig' library in other code repos.
 *
 * (c) 2015-2022 Sirio Balmelli; https://b-ad.ch
 */


#include <nonlibc.h>
#include <signal.h>


/**
 * "kill flag"
 *
 * Mechanism for looping one or many threads while checking a global "kill flag".
 *
 * The actual variable 'psg_kill_' should never be accessed directly.
 * It resides in 'posigs.c' - library callers should only ever have an extern of it.
 *
 * Async-safe, multithread-safe.
 * kill() and kill_check() have an acquire/release relationship.
 * Do not rely on psg_kill_check() to provide a full memory barrier.
 */
extern sig_atomic_t psg_kill_;

NLC_INLINE void psg_kill()
{
	__atomic_store_n(&psg_kill_, 1, __ATOMIC_RELEASE);
}

NLC_INLINE sig_atomic_t psg_kill_check()
{
	return __atomic_load_n(&psg_kill_, __ATOMIC_ACQUIRE);
}


/**
 * Set a handler function for common termination signals
 * encountered by a generic POSIX application:
 *
 * - SIGHUP
 * - SIGINT
 * - SIGQUIT
 * - SIGTERM
 *
 * Will not install handler on any of these signals if it is already being ignored.
 *
 * If 'handler' is NULL, installs a default handler which simply calls psg_kill().
 * As psg_kill() is async-safe and multithread-safe, it is not decessary
 * to disable the default handler when spawning additional threads.
 *
 * Returns 0 on success.
 */
NLC_PUBLIC int __attribute__((warn_unused_result)) psg_sigsetup(void (*handler)(int signum));


#endif /* posigs_h_ */
