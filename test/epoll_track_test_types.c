/*	epoll_track_test_types.c
 * Test correct data/type handling with epoll_track;
 * we pass an 'unsigned int' context value and there should be:
 * - correct behavior
 * - no compiler warnings or other typecasting necessary
 *
 * (c) 2018 Sirio Balmelli
 */

#include <stdint.h>
#include <unistd.h>
#include <limits.h> /* PIPE_BUF */
#include <string.h>

#include <ndebug.h>
#include <epoll_track.h>

const uintptr_t test_context = 42;
const char *nonsense = "hello world";


/*	generic_callback()
 */
int generic_callback(int fd, uint32_t events, uintptr_t context)
{
	int err_cnt = 0;

	NB_err_if(context != test_context,
		"type handling broken: %"PRIuPTR" != %"PRIuPTR, context, test_context);

	char buf[PIPE_BUF];
	ssize_t ret = read(fd, buf, PIPE_BUF);
	NB_err_if(ret < 1, "");

	NB_err_if(strcmp(buf, nonsense),
		"%s != %s", buf, nonsense);

	return err_cnt;
}


/*	main()
 */
int main()
{
	/* rely on ndebug.h global err_cnt: allow callbacks to incremement it!
	 * int err_cnt = 0;
	 */
	struct epoll_track *tk = NULL;
	int pvc[2] = { -1 };

	/* pipe */
	NB_die_if(pipe(pvc), "");
	/* epoll tracker */
	NB_die_if(!(
		tk = eptk_new()
		), "");
	/* register reader */
	NB_die_if(
		eptk_register(tk, pvc[0], EPOLLIN,
			generic_callback, test_context, NULL)
		, "");

	/* write nonsense */
	size_t ret = strlen(nonsense);
	NB_die_if(write(pvc[1], nonsense, ret) != ret, "");

	/* execute callback */
	NB_die_if(eptk_pwait_exec(tk, 1, NULL) != 1, "");

die:
	eptk_free(tk);
	for (int i=0; i < NLC_ARRAY_LEN(pvc); i++) {
		if (pvc[i] != -1)
			close(pvc[i]);
	}
	return err_cnt;
}
