#include <nonlibc.h>
#include <ndebug.h>
#include <lifo.h>
#include <time.h> /* clock() */
#include <stdlib.h> /* getenv() */

/*	test_many()
*/
int	test_many(LIFO_MEM_TYPE numiter)
{
	int err_cnt = 0;
	struct lifo *stk = NULL;
	NB_die_if(!(stk = lifo_new()),
		"");

	/* push to stack sequentially */
	for (LIFO_MEM_TYPE i=0; i < numiter; i++) {
		LIFO_MEM_TYPE ret = lifo_push(&stk, i);
		NB_die_if(ret != i,
			"ret %zu != i %zu",
			ret, i);
	}

	/* pop off stack; verify sequence */
	LIFO_MEM_TYPE pop, remain;
	while ( (remain = lifo_pop(stk, &pop)) != LIFO_ERR) {
		NB_die_if(--numiter != remain,
			"--numiter %zu != remain %zu",
			numiter, remain);
		NB_die_if(pop != remain,
			"pop %zu != remain %zu",
			pop, remain);
	}

die:
	lifo_free(stk);
	return err_cnt;
}


/*	main()
*/
int main()
{
	int err_cnt = 0;

	/* do MUCH less work if VALGRIND environment variable is set */
	int shifts = 4;
	if (!(getenv("VALGRIND")))
		shifts = 20;

	LIFO_MEM_TYPE numiter = 128;
	for (int i=0; i < shifts; i++, numiter <<= 1) {
		nlc_timing_start(elapsed);
		err_cnt += test_many(numiter);
		/* TODO: 'errno' is SOMETIMES set here, but ONLY on BSD;
		It is NEVER set just before the 'return'
			statement in test_many() ????
		*/
		nlc_timing_stop(elapsed);
		NB_inf("%zu stack pushes in %fs",
			numiter, nlc_timing_cpu(elapsed));
	}

	return err_cnt;
}
