#ifndef epoll_track_h_
#define epoll_track_h_

/*	epoll_track.h	the Nonlibc epoll wrapper/tracker

Simplifies common use cases for Linux epoll.
*/
#include <nonlibc.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <ndebug.h>

#include <urcu/hlist.h>


/*	eptk_context_t
 * Make user calls to epoll_track more legible by removing typing garbage.
 */
typedef union {
	void		*pointer;
	int64_t		integer;
	uint64_t	unsignd;
	epoll_data_t	classic;
} eptk_context_t __attribute__((__transparent_union__));

/*	eptk_callback_t
 * just for legibility
 */
struct epoll_track; /* forward-declare only, see below */
typedef void (*eptk_callback_t) (int fd,
				uint32_t events,
				eptk_context_t context,
				struct epoll_track *tk);
typedef void (*eptk_destructor_t)(eptk_context_t context);


/*	epoll_track_cb
 * Metadata to admin epoll an epoll entry.
 * @fd		file descriptor to be polled
 * @events	e.g. (EPOLLIN | EPOLLOUT) see 'man epoll_ctl'
 * @callback	function for eptk_pwait_exec() to call when event received
 * @context	passed to callback on every invocation
 * @destructor	(optional) executed on node(s) by eptk_free() and eptk_remove()
 */
struct epoll_track_cb {
	struct cds_hlist_node	node;

	int			fd;
	uint32_t		events;
	eptk_callback_t		callback;
	eptk_context_t		context;
	void			(*destructor)(eptk_context_t);
};

/*	epoll_track
 * @epfd	epoll fd
 * @rcnt	number of children (each has a 'cb' and 'report')
 */
struct epoll_track {
	struct cds_hlist_head	cb_list;
	int			epfd;
	size_t			rcnt;
};


NLC_PUBLIC void			eptk_free(struct epoll_track *tk);
NLC_PUBLIC struct epoll_track	*eptk_new();


NLC_PUBLIC int			eptk_register(struct epoll_track *tk,
						int fd,
						uint32_t events,
						eptk_callback_t callback,
						eptk_context_t context,
						eptk_destructor_t destructor);

NLC_INLINE size_t		eptk_count(struct epoll_track *tk)
{
	return tk->rcnt;
}

NLC_PUBLIC int			eptk_remove(struct epoll_track *tk, int fd);

NLC_PUBLIC int			eptk_pwait_exec(struct epoll_track *tk,
						int timeout, const sigset_t *sigmask);


#define EPTK_CB_PRN(cb) "@%p: fd %d events %d ctx %p callback %p", \
	cb, cb->fd, cb->events, cb->context.pointer, cb->callback

/*	eptk_debug_dump()
 * Print all callbacks structures for debug purposes.
 */
NLC_INLINE void			eptk_debug_dump(struct epoll_track *tk)
{
	struct epoll_track_cb *curr = NULL;
	cds_hlist_for_each_entry_2(curr, &tk->cb_list, node)
		NB_inf("dump "EPTK_CB_PRN(curr));
}

#endif /* epoll_track_h_ */
