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
#include <fcntl.h>

#define THREAD_CNT 2
#define ITERS 500 /* how many messages each thread should send */

static pthread_barrier_t start_barrier;

/*	thread()
 */
void *thread(void* arg)
{
	struct mgrp *group = arg;
	bool taken_barrier = false;
	int pvc[2] = { 0, 0 };
	size_t rx_sum = 0, rx_i = 0;
	union {
		size_t message;
		uint8_t buf[MG_MAX];
	} rx;

	NB_die_if(
		pipe(pvc)
		|| fcntl(pvc[0], F_SETFL, fcntl(pvc[0], F_GETFL) | O_NONBLOCK)
		, "");
	NB_die_if(
		mgrp_subscribe(group, pvc[1])
		, "");
	pthread_barrier_wait(&start_barrier);
	taken_barrier = true;

	/* generate ITERS broadcasts; receive others' broadcasts */
	for (size_t i = 0; i < ITERS && !psg_kill_check(); i++) {
		NB_die_if(
			mgrp_broadcast(group, pvc[1], &i, sizeof(i))
			, "");
		//NB_wrn("write");

		while ((mg_recv(pvc[0], rx.buf) > 0) && !psg_kill_check()) {
			//NB_wrn("read");
			rx_sum += rx.message;
			rx_i++;
		}
	}

	/* wait on other senders */
	while (rx_i < ITERS * (THREAD_CNT -1) && !psg_kill_check()) {
		if ((mg_recv(pvc[0], rx.buf) > 0)) {
			rx_sum += rx.message;
			rx_i++;
		}
	}
die:
	/* don't leave other threads hanging if we died before taking barrier */
	if (!taken_barrier) {
		NB_err_if(
			pthread_barrier_wait(&start_barrier)
			, "");
	}
	NB_err_if(
		mgrp_unsubscribe(group, pvc[1])
		!= 1, "should have unsubscribed exactly 1 membership");
	if (err_cnt)
		psg_kill();

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
	NB_die_if(
		pthread_barrier_init(&start_barrier, NULL, THREAD_CNT)
		, "");

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

	pthread_barrier_destroy(&start_barrier);
die:
	mgrp_free(group);
	err_cnt += psg_kill_check();  /* return any error from any thread */
	return err_cnt;
}
