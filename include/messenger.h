#ifndef messenger_h_
#define messenger_h_

/* messenger - the atomic pipe-based messaging library
 *
 * Leverage the fact that writes up to PIPE_BUF size are guaranteed atomic
 * to pass simple messages between threads which are listening
 * (e.g. with epoll) to a pipe.
 *
 * Useful when avoiding global state by passing messages between threads/coroutines,
 * or when the program has run-to-sleep semantics (wakelocks on Android).
 *
 * Provides a simple "group" registration and message broadcast mechanism.
 *
 * Thread-safe for multiple writers and a single reader.
 * Not safe for multiple readers on the same fd.
 *
 * (c) 2018 Sirio Balmelli and Anthony Soenen
 */

#include <nonlibc.h>
#include <stdint.h>
#include <limits.h> /* PIPE_BUF */
#include <stdlib.h>

#define _LGPL_SOURCE
#define URCU_INLINE_SMALL_FUNCTIONS
#include <urcu-bp.h>
#include <urcu/hlist.h>


/*	struct message
 */
struct message {
union{
struct {
	uint_fast16_t	len;
	uint8_t		data[];
};
	uint8_t		bytes[PIPE_BUF];
};
};

/* pipe() only guarantees atomicity for PIPE_BUF bytes */
#define MG_MAX (PIPE_BUF - (sizeof(uint_fast16_t)))


ssize_t mg_send		(int to_fd, void *data, size_t len);
ssize_t mg_recv		(int from_fd, void *data_out);


/*	struct mgrp
 * A messenger group is the head of a linked list of memberships.
 */
struct mgrp {
	struct cds_hlist_head	members;
	size_t			count;
};

/*	struct mgrp_membership
 * Group membership is the input (read-end of the pipe) fd, where
 * notifiers can write messages.
 */
struct mgrp_membership {
	struct cds_hlist_node	node;
	int			in_fd;
};


void mgrp_free		(struct mgrp *grp);
struct mgrp *mgrp_new	();

int	mgrp_subscribe	(struct mgrp *grp, int my_fd);
int	mgrp_unsubscribe(struct mgrp *grp, int my_fd);
int	mgrp_broadcast	(struct mgrp *grp, int my_fd, void *data, size_t len);

NLC_INLINE
size_t	mgrp_count	(struct mgrp *grp)
{
	return __atomic_load_n(&grp->count, __ATOMIC_ACQUIRE);
}


#endif /* messenger_h_ */
