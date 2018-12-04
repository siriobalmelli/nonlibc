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

#include <zed_dbg.h>
#include <epoll_track.h>

const uint64_t test_context = 42;
const char *nonsense = "hello world";


/*	generic_callback()
 */
void generic_callback(int fd, uint32_t events, uint64_t context)
{
	Z_err_if((uint64_t)context != test_context,
		"type handling broken: %lu != %lu",
		(uint64_t)context, test_context);

	char buf[PIPE_BUF];
	ssize_t ret = read(fd, buf, PIPE_BUF);
	Z_err_if(ret < 1, "");

	Z_err_if(strcmp(buf, nonsense),
		"%s != %s", buf, nonsense);
}


/*	main()
 */
int main()
{
	/* rely on zed_dbg.h global err_cnt: allow callbacks to incremement it!
	 * int err_cnt = 0;
	 */
	struct epoll_track *tk = NULL;
	int pvc[2] = { -1 };

	/* pipe */
	Z_die_if(pipe(pvc), "");
	/* epoll tracker */
	Z_die_if(!(
		tk = eptk_new()
		), "");
	/* register reader */
	Z_die_if(eptk_register(tk, pvc[0], EPOLLIN, generic_callback, test_context), "");

	/* write nonsense */
	size_t ret = strlen(nonsense);
	Z_die_if(write(pvc[1], nonsense, ret) != ret, "");

	/* execute callback */
	Z_die_if(eptk_pwait_exec(tk, 1, NULL) != 1, "");

out:
	eptk_free(tk, false);
	for (int i=0; i < NLC_ARRAY_LEN(pvc); i++) {
		if (pvc[i] != -1)
			close(pvc[i]);
	}
	return err_cnt;
}
