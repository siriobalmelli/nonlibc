#ifndef epoll_track_h_
#define epoll_track_h_

/*	epoll_track.h	the Nonlibc epoll wrapper/tracker

Simplifies common use cases for Linux epoll.
*/
#include <stdlib.h>
#include <sys/epoll.h>
#include <nonlibc.h>
#include <stdbool.h>


/*	struct epoll_track_cb
 * metadata to admin epoll an epoll entry
 * @fd		file descriptor to be polled
 * @events	e.g. (EPOLLIN | EPOLLOUT) see 'man epoll_ctl'
 * @callback	function for eptk_pwait_exec() to call when event received
 * @context	passed to callback on every invocation
 */
struct epoll_track_cb {
	int		fd;
	uint32_t	events;
	epoll_data_t	context;
	void		(*callback)(int fd, uint32_t events, epoll_data_t context);
};

/*	struct epoll_track
 * @epfd	epoll fd
 * @rcnt	number of children (each has a 'cb' and 'report')
 * @report	epoll writes events here
 */
struct epoll_track {
	int			epfd;
	size_t			rcnt;
	struct epoll_track_cb	*cb;
	struct epoll_event	*report;
};


NLC_PUBLIC void			eptk_free(struct epoll_track *tk, bool close_children);
NLC_PUBLIC struct epoll_track	*eptk_new();

NLC_PUBLIC int			eptk_register(struct epoll_track *tk,
						const struct epoll_track_cb *cb);
NLC_INLINE size_t		eptk_count(struct epoll_track *tk)
{
	return tk->rcnt;
}
NLC_PUBLIC int			eptk_remove(int fd);

NLC_PUBLIC int			eptk_pwait_exec(struct epoll_track *tk,
						int timeout, const sigset_t *sigmask);
#endif /* epoll_track_h_ */
