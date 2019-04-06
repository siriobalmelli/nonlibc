/*	mgrp_test.c
 *
 * Test messenger group broadcast/receive functionality and concurrency.
 * Each thread broadcasts a counter from 0 to ITERS, to all other threads
 * (but not itself).
 * Every receiving thread should sum received integers into a per-thread
 * rx_sum variable.
 * Every thread should accumulate the same rx_sum, which is:
 * (ITERS -1) * ITERS * (THREAD_CNT -1) * 0.5
 * (see memorywell/test/well_test.c for more details).
 *
 * (c) 2018 Sirio Balmelli and Anthony Soenen
 */

#include <ndebug.h>
#include <posigs.h> /* use atomic PSG kill flag as a global error flag */
#include <pcg_rand.h>
#include <messenger.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>

#define THREAD_CNT 2
#define ITERS 2 /* How many messages each thread should send.
		 * TODO: increase once registration/rcu issue is resolved.
		 */



/*	thread()
 */
void *thread(void* arg)
{
	struct mgrp *group = arg;
	int pvc[2] = { 0, 0 };
	size_t rx_sum = 0, rx_i = 0;
	size_t message;

	NB_die_if(
		pipe(pvc)
		|| fcntl(pvc[0], F_SETFL, fcntl(pvc[0], F_GETFL) | O_NONBLOCK)
		, "");
	NB_die_if(
		mgrp_subscribe(group, pvc[1])
		, "");
	/* Don't start broadcasting until everyone is subscribed.
	 * We could use pthread_barrier_t but that's not implemented on macOS,
	 * and anyways messenger()'s mgrp_count uses acquire/release semantics.
	 */
	while (mgrp_count(group) != THREAD_CNT && !psg_kill_check())
		sched_yield();

	/* generate ITERS broadcasts; receive others' broadcasts */
	for (size_t i = 0; i < ITERS && !psg_kill_check(); i++) {
		NB_die_if(
			mgrp_broadcast(group, pvc[1], &i, sizeof(i))
			, "");

		while ((mg_recv(pvc[0], &message) > 0) && !psg_kill_check()) {
			rx_sum += message;
			rx_i++;
			sched_yield(); /* prevent deadlock: give other threads a chance to write */
		}
		errno = 0; /* _should_ be EINVAL: don't pollute later prints */
	}

	/* wait for all other senders */
	while (rx_i < ITERS * (THREAD_CNT -1) && !psg_kill_check()) {
		if ((mg_recv(pvc[0], &message) > 0)) {
			rx_sum += message;
			rx_i++;
		} else {
			sched_yield();
		}
	}

die:
	NB_err_if(
		mgrp_unsubscribe(group, pvc[1])
		, "fd %d unsubscribe fail", pvc[1]);
	if (err_cnt)
		psg_kill();
	if (pvc[0]) {
		close(pvc[1]);
		close(pvc[0]);
	}

	return (void *)rx_sum;
}


/*	main()
 */
int main()
{
	int err_cnt = 0;
	struct mgrp *group = NULL;
	pthread_t threads[THREAD_CNT];

	NB_die_if(!(
		group = mgrp_new()
		), "");

	/* run all threads */
	for (unsigned int i=0; i < THREAD_CNT; i++) {
		NB_die_if(pthread_create(&threads[i], NULL, thread, group), "");
	}

	/* test results */
	size_t expected = (ITERS -1) * ITERS * (THREAD_CNT -1) * 0.5;
	for (unsigned int i=0; i < THREAD_CNT; i++) {
		size_t rx_sum;
		NB_die_if(
			pthread_join(threads[i], (void **)&rx_sum)
			, "");
		NB_err_if(rx_sum != expected,
			"thread %zu != expected %zu", rx_sum, expected);
	}

die:
	mgrp_free(group);
	err_cnt += psg_kill_check();  /* return any error from any thread */
	return err_cnt;
}
