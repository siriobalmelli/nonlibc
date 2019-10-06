#include <epoll_kqueue.h>
#include <sys/event.h>
#include <fcntl.h>
#include <ndebug.h>
#include <stdlib.h>
#include <unistd.h>


/*	epoll_create1()
 */
int epoll_create1(int flags)
{
	if (flags & EPOLL_CLOEXEC) {
		; /* man(2) kqueue: "the queue is not inherited by a child created with fork(2). */
	} else if (flags) {
		NB_die("flag %d not implemented", flags);
	}
	return kqueue();
die:
	return -1;
}

/*	epoll_ctl()
 */
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
	struct kevent kev;
	typeof(EV_ADD) flags;

	/* NOTE:
	 * - it is legal for 'event' to be NULL on EPOLL_CTL_DEL
	 * - we must give a valid 'filter' because a kevent is a unique (fd, filter) tuple
	 */
	typeof(EVFILT_READ) filter = 0;
	if (event) {
		/* TODO: implement all */
		if (event->events == EPOLLIN)
			filter = EVFILT_READ;
		else
			filter = EVFILT_WRITE;

	/* guess by removing *both* events for the fd */
	} else if (op == EPOLL_CTL_DEL) {
		struct epoll_event event = {
			.events = EPOLLIN,
			.data.ptr = NULL
		};
		int ret_a = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &event);
		event.events = EPOLLOUT;
		int ret_b = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &event);
		if (ret_a == -1 && ret_b == -1)
			return -1;
		return 0;

	/* because sanity */
	} else {
		errno = EINVAL;
		return -1;
	}


	switch (op) {
	case EPOLL_CTL_DEL:
		flags = EV_DELETE;
		break;

	/* EV_ADD adds new or modifies existing */
	case EPOLL_CTL_ADD:
	case EPOLL_CTL_MOD:
		flags = EV_ADD;
		break;

	default:
		NB_err("op 0x%x not implemented", op);
		errno = EINVAL;
		return -1;
	}

	EV_SET(&kev, fd, filter, flags, 0, 0, event->data.ptr);

	/* kevent returns _number_ of events, with -1 for error.
	 * We return 0 == success; -1 == error;
	 * and interpret anything not -1 as success.
	 */
	errno = 0;
	if (kevent(epfd, &kev, 1, NULL, 0, NULL) == -1) {
		return -1;
	}
	return 0;
}

/*	epoll_wait()
 */
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	struct kevent *evlist;
	NB_die_if(!(
		evlist = malloc(sizeof(struct kevent)*maxevents)
		), "fail malloc size %zu", sizeof(struct kevent)*maxevents);

	struct timespec to = {0, 0};
	if (timeout > 0) {
		to.tv_sec = timeout / 1000;
		to.tv_nsec = (timeout % 1000) * 1000 * 1000;
	}

	int ret = kevent(epfd, NULL, 0, evlist, maxevents, timeout == -1 ? NULL : &to);
	if (ret > 0) {
		for (int i = 0; i < ret; ++i) {
			events[i].events = (evlist[i].filter == EVFILT_READ) ? EPOLLIN : EPOLLOUT;
			events[i].data.ptr = evlist[i].udata;
		}
	}

	free(evlist);
	return ret;
die:
	return -1;
}

/*	epoll_pwait()
 * TODO: this is _not_ atomic, but good enough for experimental use, maybe.
 * Use at your own risk.
 */
int epoll_pwait(int epfd, struct epoll_event *events, int maxevents, int timeout, const sigset_t *sigmask)
{
	sigset_t origmask;
	int ready;

	pthread_sigmask(SIG_SETMASK, sigmask, &origmask);
	ready = epoll_wait(epfd, events, maxevents, timeout);
	pthread_sigmask(SIG_SETMASK, &origmask, NULL);

	return ready;
}
