#ifndef epoll_kqueue_h_
#define epoll_kqueue_h_

/*	epoll_kqueue.h
 * Implement epoll primitives using kqueue on BSDs
 */
#include <stdint.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <nonlibc.h>


#define EPOLLIN		0x01
#define EPOLLOUT	0x04
#define EPOLLERR	0x08
#define EPOLLHUP	0x10
#define EPOLLMSG	0x20

#define EPOLL_CLOEXEC	0x01

#define EPOLL_CTL_ADD	1
#define EPOLL_CTL_DEL	2
#define EPOLL_CTL_MOD	3


typedef union epoll_data {
	void		*ptr;
	int		fd;
	uint32_t	u32;
	uint64_t	u64;
} epoll_data_t;

struct epoll_event {
	uint32_t	events;
	epoll_data_t	data;
};


int epoll_create1(int flags);
NLC_INLINE
int epoll_create(int size)
{
	return epoll_create1(0);
}

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);

int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);

int epoll_pwait(int epfd, struct epoll_event *events, int maxevents, int timeout,
		const sigset_t *sigmask);


#endif /* epoll_kqueue_h_ */
