#include <ndebug.h>
#include <posigs.h> /* use atomic PSG kill flag as inter-thread err_cnt */
#include <fnv.h>
#include <pcg_rand.h>
#include <sched.h> /* sched_yield() */
#include <messenger.h>
#include <pthread.h>

#define THREAD_CNT 4
#define ITERS 5000 /* how many messages each thread should send */

static uint64_t tx_hash[THREAD_CNT] = { 0 };
static uint64_t rx_hash[THREAD_CNT] = { 0 };
static int contended[2] = { -1 };

/* send this over the wire */
struct blob {
	unsigned int thread_id;
	size_t len;
	uint8_t bytes[];
};

/* tx_thread()
 * Generate messages containing variable amounts of random bytes,
 * send them over the fd,
 * add their hash to our tx_hash.
 *
 * @arg : main() should provide us with our thread ID
 */
void *tx_thread(void* arg)
{
	int err_cnt = 0;
	struct blob *header = NULL;
	NB_die_if(!(
		header = malloc(MG_MAX)
		), "MG_MAX %zu", MG_MAX);
	header->thread_id = (unsigned int)(uintptr_t)arg;
	NB_inf("tx %u", header->thread_id);

	/* Generate variable-sized messages,
	 * up to the maximum number of bytes that
	 * this platform guarantees will be sent atomically.
	 */
	size_t max_len = MG_MAX - sizeof(struct blob);

	/* initialize fnv1a hash */
	uint64_t *hash = &tx_hash[header->thread_id];
	*hash = fnv_hash64(NULL, NULL, 0);

	/* seed RNG */
	struct pcg_state rnd;
	pcg_seed_static(&rnd);

	/* generate ITERS messages, of random length each,
	 * hashing each one, then sending
	 */
	for (int x = 0; x < ITERS; x++) {
		header->len = pcg_rand_bound(&rnd, max_len);
		pcg_set(&rnd, header->bytes, header->len);
		*hash = fnv_hash64(hash, header->bytes, header->len);
		/* Zero out everything after len in data */
		//memset(&header->bytes[header->len], 0, max_len - header->len);

		/* Send the data */
		size_t send_len = sizeof(struct blob) + header->len;
		ssize_t ret = mg_send(contended[1], header, send_len);
		NB_die_if(ret != send_len, "mg_send %zd != len %zu", ret, send_len);
	}
die:
	free(header);
	if (err_cnt)
		psg_kill();
	return NULL;
}


/* rx_thread()
 * Receive messages containing variable amounts of random bytes,
 * hash them into the correct rx_hash for that sender.
 */
void *rx_thread(void* arg)
{
	int err_cnt = 0;

	/* Receive variable-sized messages,
	 * up to the maximum number of bytes that
	 * this platform guarantees will be sent atomically.
	 */
	struct blob *header = NULL;
	NB_die_if(!(
		header = malloc(MG_MAX)
		), "MG_MAX %zu", MG_MAX);

	/* init rx hashes */
	for (int i=0; i < THREAD_CNT; i++)
		rx_hash[i] = fnv_hash64(NULL, NULL, 0);

	int x = 0;
	for (; x < ITERS * THREAD_CNT && !psg_kill_check(); x++) {
		/* may return no bytes or an error (-1) */
		while ((mg_recv(contended[0], header) < 1))
			sched_yield();
		uint64_t *hash = &rx_hash[header->thread_id];
		*hash = fnv_hash64(hash, header->bytes, header->len);
	}
	NB_err_if(x != ITERS * THREAD_CNT, "");
die:
	free(header);
	if (err_cnt)
		psg_kill();
	return NULL;
}


/*	main()
 */
int main()
{
	int err_cnt = 0;

	pthread_t txes[THREAD_CNT];
	pthread_t rx;

	NB_die_if(pipe(contended), "");

	/* run all threads */
	for (uintptr_t i=0; i < THREAD_CNT; i++) {
		NB_die_if(pthread_create(&txes[i], NULL, tx_thread, (void *)i), "");
	}
	NB_die_if(pthread_create(&rx, NULL, rx_thread, NULL), "");

	for (int i=0; i < THREAD_CNT; i++)
		NB_die_if(pthread_join(txes[i], NULL), "");
	NB_die_if(pthread_join(rx, NULL), "");

	/* test results */
	for (int i=0; i < THREAD_CNT; i++)
		NB_err_if(tx_hash[i] != rx_hash[i], "thread %d", i);

die:
	if (contended[0] != -1)
		close(contended[0]);
	if (contended[1] != -1)
		close(contended[1]);

	/* return any error from any thread */
	err_cnt += psg_kill_check();
	return err_cnt;
}
