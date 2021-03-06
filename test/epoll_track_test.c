#include <stdint.h>
#include <unistd.h>

#include <ndebug.h>
#include <epoll_track.h>

#define NUMITER 4096
#define MSG_LEN 32
#define PIPE_COUNT 4

/*	rx_callback()
 */
int rx_callback(int fd, uint32_t events, void *context)
{
	int err_cnt = 0;
	NB_die_if(!events, "events mask not propagated to callback");

	/* accumulator */
	unsigned int *acc = context;

	unsigned int buf;
	int ret;
	NB_die_if((
		ret = read(fd, &buf, sizeof(buf))
		) != sizeof(buf), "read returns %d", ret);
	/* accumulate received values - will be checked by main */
	*acc += buf;

die:
	return err_cnt;
}


/*	main()
 * Returns number of errors encountered; 0 on successful test execution.
 */
int main()
{
	/* rely on global err_cnt created by zed_dbg, so that errors registered
	 * in rx_callback() are properly returned as a test failure.
	 * int err_cnt = 0;
	 */
	struct epoll_track *tk = NULL;
	int pvc[PIPE_COUNT * 2];
	unsigned int counters[sizeof(unsigned int) * PIPE_COUNT] = { 0 };
	for (int i=0; i < PIPE_COUNT; i++) {
		pvc[i] = -1;
		counters[i] = 0;
	}

	/* create an epoll tracker; register read-end of the pipe for tracking */
	NB_die_if(!(
		tk = eptk_new()
		), "");

	/* set up each (pipe, callback, counter) tuple */
	for (int i=0; i < PIPE_COUNT; i++) {
		int *curr_pipe = &pvc[i*2];
		NB_die_if(pipe(curr_pipe), "");
		NB_die_if(eptk_register(tk, curr_pipe[0], EPOLLIN,
				rx_callback, &counters[i], NULL)
			, "");
	}

	/* try and register the last one again: should modify the existing one only */
	NB_die_if(eptk_register(tk, pvc[(PIPE_COUNT-1)*2], EPOLLIN,
			rx_callback, &counters[(PIPE_COUNT-1)], NULL)
		, "");
	NB_die_if(eptk_count(tk) != PIPE_COUNT,
		"count %zu expected PIPE_COUNT of %d", eptk_count(tk), PIPE_COUNT);


	/* test approach:
	 * push data into the pipe; rely on epoll_tracking infrastructure to
	 * correctly invoke rx_callback()
	 */
	unsigned int total = 0;
	for (unsigned int i=0; i <  NUMITER; i++) {
		/* write to all pipes */
		for (int j=0; j < PIPE_COUNT; j++) {
			NB_die_if((
				write(pvc[j*2+1], &i, sizeof(i))
				) != sizeof(i), "fail write size %zu", sizeof(i));
			total += i;
		}

		int ret;
		/* 1ms timedie: there should always BE data ready for reading;
		 * any timeout at all is an indication of failure somewhere.
		 */
		NB_die_if((
			ret = eptk_pwait_exec(tk, 1, NULL)
			) != PIPE_COUNT,
			"pwait %d != PIPE_COUNT %d", ret, PIPE_COUNT);
	}

	/* rx_callback() should have accumulated identical data to this loop */
	unsigned int check = 0;
	for (int i=0; i < PIPE_COUNT; i++)
		check += counters[i];
	NB_die_if(total != check,
		"total %u != check %u", total, check);

	/* delete should return 1 */
	int ret;
	NB_die_if((
		ret = eptk_remove(tk, pvc[0])
		) != 1, "removed %d instead of 1", ret);

die:
	eptk_free(tk);
	for (int i=0; i < NLC_ARRAY_LEN(pvc); i++) {
		if (pvc[i] != -1)
			close(pvc[i]);
	}
	return err_cnt;
}
