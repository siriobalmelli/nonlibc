#include <ndebug.h>
#include <posigs.h>


sig_atomic_t psg_kill_ = 0; /* access only with atomics */


/**
 * The most basic of functional signal handling for an application.
 */
static void psg_handler_default_(int signum)
{
	/* WARNING: printf is not signal-handler safe; don't use it here. */

	/* signal program to terminate */
	psg_kill();
}


int psg_sigsetup(void (*handler)(int signum))
{
	int err_cnt = 0;

	if (!handler)
		handler = psg_handler_default_;
	struct sigaction old, action = { .sa_handler = handler };

	/* all the (handleable) glibc termination signals */
	int signals[] = { SIGTERM, SIGINT, SIGQUIT, SIGHUP };

	for (int i = 0; i < NLC_ARRAY_LEN(signals); i++) {
		NB_err_if(sigaction(signals[i], NULL, &old), "SIG %d", signals[i]);
		/* avoid installing handler if signal is being ignored */
		if (old.sa_handler != SIG_IGN)
			NB_err_if(sigaction(signals[i], &action, NULL), "SIG %d", signals[i]);
	}

	return err_cnt;
}
