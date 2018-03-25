#ifndef posigs_h_
#define posigs_h_

/*	posigs.h	Linux signaling utilities

Utility funtions for comming POSIX signal handling patterns.
Originally appeared as the 'mtsig' library in other code repos.

(c) 2015 Sirio Balmelli; https://b-ad.ch
*/


#include <signal.h>
#include <nonlibc.h>


/*	kill flag

Mechanism for looping one or many threads while checking a global "kill flag".

The actual variable 'psg_kill_' should never be accessed directly.
It resides in 'posigs.c' - library callers should only ever have an extern of it.

Async-safe, multithread-safe.
kill() and kill_check() have an acquire/release relationship.
Do not rely on psg_kill_check() to provide a full memory barrier.
*/
extern sig_atomic_t psg_kill_;

NLC_INLINE void		psg_kill()
{
	__atomic_store_n(&psg_kill_, 1, __ATOMIC_RELEASE);
}

NLC_INLINE sig_atomic_t	psg_kill_check()
{
	return __atomic_load_n(&psg_kill_, __ATOMIC_ACQUIRE);
}


/*	sig setup
*/
NLC_PUBLIC int psg_sigsetup(void (*handler)(int signum));


#endif /* posigs_h_ */
