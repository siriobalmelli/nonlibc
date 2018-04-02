#include <zed_dbg.h>
#include <posigs.h>


sig_atomic_t psg_kill_ = 0;


/*	psg_handler_default()

The most basic of functional signal handling
	for an application.
*/
static void psg_handler_default_(int signum)
{
	Z_log_line();
	switch (signum) {
	case SIGTERM:
		Z_log(Z_inf, "SIGTERM");
		break;
	case SIGINT:
		Z_log(Z_inf, "SIGINT");
		break;
	default:
		Z_log(Z_err, "don't know what to do with SIG %d", signum);
	}

	/* signal program to terminate */
	psg_kill();
}


/*	psg_sigsetup()

Boilerplate to set a common-sense function handler for
	termination signals encountered by a generic POSIX application.

If 'handler' is NULL, installs 'psg_handler_default_()'
	which will just log signal and set the kill flag.

Returns 0 on success.
*/
int psg_sigsetup(void (*handler)(int signum))
{
	int err_cnt = 0;

	if (!handler)
		handler = psg_handler_default_;
	struct sigaction old, action = { .sa_handler = handler };

	/* all the (handleable) glibc termination signals */
	int signals[] = { SIGTERM, SIGINT, SIGQUIT, SIGHUP };

	for (int i=0; i < NLC_ARRAY_LEN(signals); i++) {
		Z_err_if(sigaction(signals[i], NULL, &old)
			, "SIG %d", signals[i]);
		/* avoid installing handler if signal is being ignored */
		if (old.sa_handler != SIG_IGN) {
			Z_err_if(sigaction(signals[i], &action, NULL)
				, "SIG %d", signals[i]);
		}
	}

	return err_cnt;
}
