#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <zed_dbg.h>
#include <epoll_track.h>
#include <posigs.h>
#include <time.h>
#include <fnv.h>
#include <pcg_rand.h>
#include <messenger.h>

/* Sirio:
TODO:	-  this test doesn't exit as its an epoll loop
	-  you'll see that I had to do funky stuff to add pthreads as 
		a dependency for the pkgconfig not to barf when meson 
		tries to build.
		The dependency 'pthread' defined in 'meson.build' cannot be added
		in 'lib/meson.build' to the pkgconfig.generate, you have to provide
		the raw '-pthreads' string there.
*/

uint64_t *data[2] = { NULL, NULL };

struct thread_data {
	int idx;	/* Index into the 'data' array */
	int write_fd;
};
/*	write_thread()
 *	Generate blobs of random bytes, hash them, add all hashes and send
 *	each blob over pipe.
 *	'ptr' is the pipe fd.
 */
void* write_thread(void* ptr)
{
	Z_log(Z_inf, "write_thread");
	struct thread_data *td = ptr;
	int fd = td->write_fd;
	/* Generate a number of random bytes between 1 and MG_MAX 
		and generate an fnv hash.
	Do this x number of times and add all hashes together. 
	 Return the final hash to the caller. */	

	uint64_t ret_hash = 0;
	/* Generate a random len for the number of bytes to be generated
		between 0 - MG_MAX */	
	struct pcg_state rnd;
	pcg_seed_static(&rnd);
	printf("idx %d\r\n", td->idx);
	Z_die_if(!(
		data[td->idx] = malloc(PIPE_BUF)
		), "");

	/* Generate blobs and send */
	for (int x = 0; x < 5; x++) {
		uint32_t len = pcg_rand_bound(&rnd, MG_MAX);
		Z_log(Z_inf, "len %d", len);

		uint64_t seed1 = time(NULL);
		uint64_t seed2 = time(NULL);	

		pcg_randset(data[td->idx], len, seed1, seed2); 

		/* Zero out everything after len in data */
		memset(data[td->idx] + len, 0, PIPE_BUF - len - 1);

		/* Fnv hash*/
		uint64_t hash = 0;
		hash = fnv_hash64(&hash, data[td->idx], PIPE_BUF); 

		ret_hash += hash;

	//	printf("hash %lx\r\n", hash);
		/* Send the data */
		mg_send(fd, data[td->idx], len);
		
		/* Zero out the buffer */
		memset(data[td->idx], 0, PIPE_BUF);
	}
	printf("Write hash %lx\r\n", ret_hash);
	Z_log(Z_inf, "end write_thread");

	printf("idx %d\r\n", td->idx);
	if (data[td->idx] != NULL) {
		free(data[td->idx]);
		data[td->idx] = NULL;
	}
	return (void*)ret_hash;
out:
	if (data[td->idx]) {
		free(data[td->idx]);
		data[td->idx] = NULL;
	}
	Z_log(Z_inf, "error write_thread");
	return NULL;
}

uint64_t *data_out[2] = { NULL, NULL };
/*	messenger_callback()
 *	Callback from epoll tracker to read blobs coming into pipe defined 
 *	by 'fd'.
 *	Read each blob and hash, accumulate all hashes.
*/
void messenger_callback(int fd, uint32_t events, epoll_data_t context)
{
	struct thread_data *td = context.ptr;
	Z_log(Z_inf, "read_thread");
	static uint64_t ret_hash = 0;

	/* Create PIP_BUF size memory area for reading data into. */
	Z_die_if(!(
		data_out[td->idx] = malloc(PIPE_BUF)
		), "");

	/* Zero out 'data_out' so that any unused space is zero as in the
		writer threads. */
	
	memset(data_out[td->idx], 0, PIPE_BUF);

	int len = mg_recv(fd, data_out[td->idx]);

	uint64_t hash = 0;
	/* Fnv hash */
	hash = fnv_hash64(&hash, data_out[td->idx], PIPE_BUF); 
//	printf("hash %lx\r\n", hash);

	if (data_out[td->idx]) {
		free(data_out[td->idx]);
		data_out[td->idx] = NULL;
	}
	ret_hash += hash;
	printf("Read hash %lx\r\n", ret_hash);
	Z_log(Z_inf, "end read_thread");

	return;
out:
	if (data_out[td->idx] != NULL) {
		free(data_out[td->idx]);
		data_out[td->idx] = NULL;
	}
	Z_log(Z_inf, "error read_thread");
}

int main()
{
	pthread_t wthread1, wthread2;
	struct thread_data *td1 = NULL;
	struct epoll_track *tk = NULL;
	int pvc[2] = {-1};
	int pvc2[2] = {-1};

	Z_die_if((
		psg_sigsetup(NULL)
		), "");

	Z_die_if(!(
		tk = eptk_new()
		), "");	

	Z_die_if((
		pipe(pvc) != 0 
		), "");	

	Z_die_if((
		pipe(pvc2) != 0 
		), "");	

	Z_die_if(!(
		td1 = malloc(sizeof(struct thread_data))
		), "");
	td1->idx = 0;
	td1->write_fd = pvc[1];

	/* Register with epoll tracker */
	struct epoll_track_cb cb = {
		.fd = pvc[0],
		.events = EPOLLIN,
		.context.ptr = td1,
		.callback = messenger_callback
	};

	Z_die_if(eptk_register(tk, cb.fd, cb.events, cb.callback, cb.context), "");

	/* Register with epoll tracker */
	struct epoll_track_cb cb1 = {
		.fd = pvc2[0],
		.events = EPOLLIN,
		.context.ptr = NULL,
		.callback = messenger_callback
	};

	Z_die_if(eptk_register(tk, cb1.fd, cb1.events, cb1.callback, cb1.context), "");

	Z_die_if((
		pthread_create(&wthread1, NULL, write_thread, (void*)td1)
		), "");
#if 0
	struct thread_data *td2 = NULL;
	Z_die_if(!(
		td2 = malloc(sizeof(struct thread_data))
		), "");
	td2->idx = 0;
	td2->write_fd = pvc2[1];
	Z_die_if((
		pthread_create(&wthread2, NULL, write_thread, (void*)td2)
		), "");
#endif
#if 1 
	while (!psg_kill_check()) {
		int res = eptk_pwait_exec(tk, -1, NULL);

		if (res == -1) {
			switch (errno) {
				case EINTR:
					continue;
				default:
					Z_die("eptk_pwait_exec()");
			}
		}
	}
#endif
	close(pvc[0]);
	close(pvc[1]);
	close(pvc2[0]);
	close(pvc2[1]);
	eptk_free(tk, false);
	free(td1);
	return 0;
out:
	if (pvc[0] != -1)
		close(pvc[0]);
	if (pvc[1] != -1)
		close(pvc[1]);
	if (pvc2[0] != -1)
		close(pvc2[0]);
	if (pvc2[1] != -1)
		close(pvc2[1]);
	if (tk)
		eptk_free(tk, false);
	if (td1)
		free(td1);
	return 1;
}
