/*	epoll_track_test_destructor.c
 * Test the use of destructors with epoll_track;
 * verify expected behavior and that no leaks develop.
 * (c) 2018 Sirio Balmelli
 */

#include <epoll_track.h>
#include <ndebug.h>
#include <unistd.h>


/* A stereotypical "subsystem", which has:
 * - a struct for resource tracking (including allocation)
 * - one or more fd's for polled I/O
 * - specific new() and free() functions which must be called to manage resources
 */
struct subsys {
	int	infd;
	int	outfd;
	size_t	resource_sz;
	char	*resource;
	const char *name;
};

/*	subsys_free()
 * NOTE the use of a 'void *' 'arg' as opposed to explicitly typed function pointer.
 * This is 99% semantics, and 1% avoiding having to choose between a cast
 * and a compiler warning when passing &subsys_free to eptk_register().
 */
void subsys_free(void *arg)
{
	if (!arg)
		return;
	struct subsys *sub = arg;

	if (sub->infd > 0)
		close(sub->infd);
	if (sub->outfd > 0)
		close(sub->outfd);
	free(sub->resource);
	free(sub);
}

/*	subsys_new()
 */
struct subsys *subsys_new(int infd, int outfd, const char *name)
{
	struct subsys *ret = NULL;
	NB_die_if(!(
		ret = calloc(1, sizeof(*ret))
		), "fail alloc size %zu", sizeof(*ret));

	ret->infd = infd;
	ret->outfd = outfd;
	ret->name = name;

	ret->resource_sz = 42;
	NB_die_if(!(
		ret->resource = malloc(ret->resource_sz)
		), "fail alloc size %zu", ret->resource_sz);

	return ret;
die:
	subsys_free(ret);
	return NULL;
}

/*	subsys_callback()
 */
void subsys_callback(int fd, uint32_t events, void *context, struct epoll_track *tk)
{
	struct subsys *sub = context;
	ssize_t res = read(fd, sub->resource, sub->resource_sz);

	/* Error or closure means we should close and deallocate;
	 * this is done when eptk_remove() calls our destructor.
	 */
	if (res <= 0) {
		eptk_remove(tk, fd);
		return;
	}

	ssize_t check = write(sub->outfd, sub->resource, res);
	NB_err_if(res != check, "I/O fail");

	sub->resource[res] = '\0'; /* force string termination */
	NB_inf("%s received '%s'", sub->name, sub->resource);
}


/*	main()
 */
int main()
{
	/* use global err_cnt, so that callbacks can easily report failure
	 * int err_cnt = 0;
	 */

	struct epoll_track *tk = NULL;
	struct subsys *suba = NULL;
	int us2a[2] = { -1, -1 };
	int a2us[2] = { -1, -1 };

	NB_die_if(!(
		tk = eptk_new()
		), "");
	/* plumbing */
	NB_die_if(pipe(us2a) || pipe(a2us)
		, "pipe2() failed");
	NB_die_if(!(
		suba = subsys_new(us2a[0], a2us[1], "sub-a")
		), "");

	NB_die_if(
		eptk_register(tk, us2a[0], EPOLLIN, &subsys_callback, suba, &subsys_free)
		, "");

	/* write data to pipe and run subsystem */
	char buf[] = "hello";
	NB_die_if((
		write(us2a[1], buf, sizeof(buf))
		) != sizeof(buf), "fail write size %zu", sizeof(buf));
	eptk_pwait_exec(tk, 1, NULL);

	/* verify subsystem ran */
	char check[sizeof(buf)] = { '\0' };
	ssize_t ret = read(a2us[0], check, sizeof(buf));
	NB_die_if(ret != sizeof(buf), "read %zu but expect %zu", ret, sizeof(buf));
	NB_die_if(strcmp(buf, check), "buf '%s' != check '%s'", buf, check);
	
	/* trigger closures and run epoll subsystems */
	close(us2a[1]);
	eptk_pwait_exec(tk, 1, NULL);

	/* verify subsystem deregistered itself */
	int subsys_cnt = eptk_count(tk);
	NB_die_if(subsys_cnt, "%d subsystems have not exited", subsys_cnt);

die:
	eptk_free(tk);
	return err_cnt;
}
