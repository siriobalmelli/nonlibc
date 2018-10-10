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

/*	write_thread()
 *	Generate blobs of random bytes, hash them, add all hashes and send
 *	each blob over pipe.
 *	'ptr' is the pipe fd.
 */
void* write_thread(void* ptr)
{
	Z_log(Z_inf, "write_thread");
	int fd = *((int*)ptr);
	/* Generate a number of random bytes between 1 and MG_MAX 
		and generate an fnv hash.
	Do this x number of times and add all hashes together. 
	 Return the final hash to the caller. */	

	uint64_t ret_hash = 0;
	void* data = NULL;
	/* Generate a random len for the number of bytes to be generated
		between 0 - MG_MAX */	
	struct pcg_state rnd;
	pcg_seed_static(&rnd);
	Z_die_if(!(
		data = malloc(PIPE_BUF)
		), "");

	/* Generate blobs and send */
	for (int x = 0; x < 5; x++) {
		uint32_t len = pcg_rand_bound(&rnd, MG_MAX);
		Z_log(Z_inf, "len %d", len);

		uint64_t seed1 = time(NULL);
		uint64_t seed2 = time(NULL);	

		pcg_randset(data, len, seed1, seed2); 

		/* Zero out everything after len in data */
		memset(data + len, 0, PIPE_BUF - len - 1);

		/* Fnv hash*/
		uint64_t hash = 0;
		hash = fnv_hash64(&hash, data, PIPE_BUF); 

		ret_hash += hash;

		printf("hash %lx\r\n", hash);
		/* Send the data */
		mg_send(fd, data, len);
		
		/* Zero out the buffer */
		memset(data, 0, PIPE_BUF);
	}
	printf("Write hash %lx\r\n", ret_hash);
	Z_log(Z_inf, "end write_thread");

	if (data)
		free(data);
	return (void*)ret_hash;
out:
	if (data)
		free(data);
	Z_log(Z_inf, "error write_thread");
	return NULL;
}

/*	messenger_callback()
 *	Callback from epoll tracker to read blobs coming into pipe defined 
 *	by 'fd'.
 *	Read each blob and hash, accumulate all hashes.
*/
void messenger_callback(int fd, uint32_t events, epoll_data_t context)
{
	Z_log(Z_inf, "read_thread");
	static uint64_t ret_hash = 0;

	void* data_out = NULL;

	/* Create PIP_BUF size memory area for reading data into. */
	Z_die_if(!(
		data_out = malloc(PIPE_BUF)
		), "");

	mg_recv(fd, data_out);

	uint64_t hash = 0;
	/* Fnv hash */
	hash = fnv_hash64(&hash, data_out, PIPE_BUF); 
	printf("hash %lx\r\n", hash);

	if (data_out) {
		free(data_out);
		data_out = NULL;
	}
	ret_hash += hash;
	printf("Read hash %lx\r\n", ret_hash);
	Z_log(Z_inf, "end read_thread");

	return;
out:
	if (data_out)
		free(data_out);
	Z_log(Z_inf, "error read_thread");
}

int main()
{
	pthread_t wthread1, wthread2;
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

	/* Register with epoll tracker */
	struct epoll_track_cb cb = {
		.fd = pvc[0],
		.events = EPOLLIN,
		.context.ptr = NULL,
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
		pthread_create(&wthread1, NULL, write_thread, &pvc[1])
		), "");
#if 0
	Z_die_if((
		pthread_create(&wthread2, NULL, write_thread, &pvc2[1])
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
	return 1;
}
