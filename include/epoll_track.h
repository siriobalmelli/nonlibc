#ifndef epoll_track_h_
#define epoll_track_h_

/*	epoll_track.h
 * The Nonlibc epoll wrapper/tracker.
 * Simplifies common use cases for Linux epoll (and in future maybe BSD's kqueue)
 * by tracking/manipulating file descriptors registered with an instance of epoll.
 *
 * (c) 2018 Sirio Balmelli
 */

#include <nonlibc.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <ndebug.h>
#include <urcu/hlist.h>


struct epoll_track; /* forward declaration only, see below */

/*	eptk_context_t
 * Make user calls to epoll_track more legible by removing type warnings warnings.
 */
typedef union {
	void		*pointer;
	uintptr_t	integer;
	epoll_data_t	classic;
} eptk_context_t __attribute__((__transparent_union__));

/*	eptk_callback_t
 * Executed when epoll returns 'fd':
 * @fd		: as returned by epoll()
 * @events	: events that triggered epoll()
 * @context	: opaque value given to eptk_register()
 *
 * If callback returns non-0, eptk_remove() will be called on 'fd'.
 */
typedef int (*eptk_callback_t) (int fd,
				uint32_t events,
				eptk_context_t context);

/*	eptk_destructor_t
 * Executed when an fd is removed from an instance of epoll_track:
 * - when eptk_remove() is called on 'fd'
 * - when 'callback' executed on 'fd' returns non-0
 * - when eptk_free() is called
 *
 * @context	: opaque value that was passed to eptk_register().
 *
 * NOTE:
 * - the function signature is such that libc's free() is a valid destructor
 * - if 'destructor' was not given to eptk_register() then close() is called
 *   on 'fd' instead.
 */
typedef void (*eptk_destructor_t)(eptk_context_t context);



/*	epoll_track_cb
 * Metadata to admin epoll an epoll entry.
 * @fd		: file descriptor to be polled.
 * @events	: events on which epoll should trigger
 *		e.g. (EPOLLIN | EPOLLOUT), see 'man epoll_ctl'.
 * @callback	: function for eptk_pwait_exec() to call when event received
 * @context	: passed to callback on every invocation.
 * @destructor	: executed (if present) on node when removed.
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
 * @cb_list	: rcu linked list of 'struct epoll_track_cb' tracked fds.
 * @rcnt	: number of cbs in cb_list.
 * @epfd	: epoll fd
 */
struct epoll_track {
	struct cds_hlist_head	cb_list;
	size_t			rcnt;
	int			epfd;
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
						int timeout,
						const sigset_t *sigmask);


#define EPTK_CB_PRN(cb) "@%p: fd %d events %d ctx %p callback %p destructpr %p", \
	cb, cb->fd, cb->events, cb->context.pointer, cb->callback, cb->destructor

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
