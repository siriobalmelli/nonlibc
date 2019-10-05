#include <epoll_kqueue.h>

/*	epoll_create1()
 */
int epoll_create1(int flags)
{
	return 0;
}

/*	epoll_ctl()
 */
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
	return 0;
}

/*	epoll_wait()
 */
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	return 0;
}

/*	epoll_pwait()
 */
int epoll_pwait(int epfd, struct epoll_event *events, int maxevents, int timeout, const sigset_t *sigmask)
{
	return 0;
}
