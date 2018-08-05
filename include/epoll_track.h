#ifndef epoll_track_h_
#define epoll_track_h_

/*	epoll_track.h	the Nonlibc epoll wrapper/tracker

Simplifies common use cases for Linux epoll.
*/
#include <stdlib.h>
#include <sys/epoll.h>
#include <nonlibc.h>
#include <stdbool.h>
#include <zed_dbg.h>

#include <urcu/hlist.h>

/*	struct epoll_track_cb
 * metadata to admin epoll an epoll entry
 * @fd		file descriptor to be polled
 * @events	e.g. (EPOLLIN | EPOLLOUT) see 'man epoll_ctl'
 * @callback	function for eptk_pwait_exec() to call when event received
 * @context	passed to callback on every invocation
 */
struct epoll_track_cb {
	struct cds_hlist_node	node;
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
	struct cds_hlist_head	cb_list;
	int			epfd;
	size_t			rcnt;
	struct epoll_event	*report;
};


NLC_PUBLIC void			eptk_free(struct epoll_track *tk, bool close_children);
NLC_PUBLIC struct epoll_track	*eptk_new();

NLC_PUBLIC int			eptk_register(struct epoll_track *tk,
						int fd,
						uint32_t events,
						void (*callback)(int fd, uint32_t events, epoll_data_t context),
						epoll_data_t context);
NLC_INLINE size_t		eptk_count(struct epoll_track *tk)
{
	return tk->rcnt;
}
NLC_PUBLIC int			eptk_remove(struct epoll_track *tk, int fd);

NLC_PUBLIC int			eptk_pwait_exec(struct epoll_track *tk,
						int timeout, const sigset_t *sigmask);


#define EPTK_CB_PRN(cb) "@%p: fd %d events %d ctx %p callback %p", \
	cb, cb->fd, cb->events, cb->context.ptr, cb->callback

/*	eptk_debug_dump()
 * Print all callbacks structures for debug purposes.
 */
NLC_INLINE void			eptk_debug_dump(struct epoll_track *tk)
{
	struct epoll_track_cb *curr = NULL;
	cds_hlist_for_each_entry_2(curr, &tk->cb_list, node)
		Z_log(Z_inf, "dump "EPTK_CB_PRN(curr));
}

#endif /* epoll_track_h_ */
