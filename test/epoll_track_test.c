#include <stdint.h>
#include <unistd.h>

#include <epoll_track.h>
#include <zed_dbg.h>

#define NUMITER 4096
#define MSG_LEN 32

static unsigned int rx_count = 0;
static const uint64_t rx_context = 42;


/*	rx_callback()
 */
void rx_callback(int fd, uint32_t events, epoll_data_t context)
{
	Z_die_if(context.u64 != rx_context,
		"callback expected context %"PRIu64" but received %"PRIu64,
		rx_context, context.u64);
	Z_die_if(!events, "events mask not propagated to callback");

	unsigned int buf;
	int ret;
	Z_die_if((
		ret = read(fd, &buf, sizeof(buf))
		) != sizeof(buf), "read returns %d", ret);
	/* accumulate received values - will be checked by main */
	rx_count += buf;
out:
	return;
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
	int pvc[2] = { -1, -1 };
	struct epoll_track *tk = NULL;

	/* vanilla pipe */
	Z_die_if(pipe(pvc), "");

	/* create an epoll tracker; register read-end of the pipe for tracking */
	Z_die_if(!(
		tk = eptk_new()
		), "");
	struct epoll_track_cb cb = {
		.fd = pvc[0],
		.events = EPOLLIN, /* not edge-triggered, simply "there is data" */
		.context.u64 = rx_context,
		.callback = rx_callback
	};
	Z_die_if(eptk_register(tk, &cb), "");

	/* test approach:
	 * push data into the pipe; rely on epoll_tracking infrastructure to
	 * correctly invoke rx_callback()
	 */
	unsigned int total = 0;
	for (unsigned int i=0; i <  NUMITER; i++) {
		Z_die_if((
			write(pvc[1], &i, sizeof(i))
			) != sizeof(i), "fail write size %zu", sizeof(i));
		total += i;
		int ret;
		/* 1ms timeout: there should always BE data ready for reading;
		 * any timeout at all is an indication of failure somewhere.
		 */
		Z_die_if((
			ret = eptk_pwait_exec(tk, 1, NULL)
			) != 1, "pwait returns %d", ret);
	}

	/* rx_callback() should have accumulated identical data to this loop */
	Z_die_if(total != rx_count,
		"total %u != rx_count %u", total, rx_count);

out:
	eptk_free(tk, false);
	if (pvc[0] != -1)
		close(pvc[0]);
	if (pvc[1] != -1)
		close(pvc[1]);
	return err_cnt;
}
