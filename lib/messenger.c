#include <ndebug.h>
#include <nonlibc.h>
#include <messenger.h>

/* our 'size' variable on this platform *must* be large enough
 * to express the largest atomic write size for a pipe (PIPE_BUF)
 */
NLC_ASSERT(mg_size_sanity, INT_FAST16_MAX > PIPE_BUF);

/* macrofy it so that we don't rewrite this for mgrp_broadcast()
 * or (heaven forbid) do a new "assembly" for each target of the broadcast.
 */
#define MG_ASSEMBLE  \
	if (len > MG_MAX) \
		return -1; \
	struct message mg = { .len = len }; \
	memcpy(mg.data, data, len);


/* mg_send()
 * Write 'len' bytes from 'data' into 'to_fd'.
 * Return behavior and semantics are same as for write()
 */
ssize_t mg_send(int to_fd, void *data, size_t len)
{
	MG_ASSEMBLE
	ssize_t ret = write(to_fd, &mg, PIPE_BUF);
	/* hide our header size from the caller */
	if (ret > 0)
		return len;
	return ret; /* because there is a different between 0 and -1 */
}

/* mg_recv()
 * Get a message in 'from_fd' and write it to 'data_out'.
 * Return behavior and semantics are same as for read().
 */
ssize_t mg_recv(int from_fd, void *data_out)
{
	/* NOTE: we assume that since we are writing *atomically*
	 * we will *always* read either PIPE_BUF or 0 or -1
	 */
	struct message mg;
	ssize_t rd = read(from_fd, &mg, PIPE_BUF);
	if (rd != PIPE_BUF)
		return rd;

	ssize_t ret = (ssize_t)mg.len;
	memcpy(data_out, mg.data, ret);
	return ret;
}


/* mgrp_free()
 */
void mgrp_free(struct mg_group *grp)
{
	if (!grp)
		return;
	/* TODO: free RCU */
	free(grp);
}

/* mgrp_new()
 * Create a new messaging group, return pointer,
 * which must be freed with mgrp_free().
 */
struct mg_group *mgrp_new()
{
	struct mg_group *grp = NULL;
	NB_die_if(!(
		grp = malloc(sizeof *grp)
		), "");
	/* TODO: init rcu linked list */
	return grp;
die:
	mgrp_free(grp);
	return NULL;
}

/* mgrp_subscribe()
 * Subscribe 'my_fd' to receive notifications by 'grp'.
 * Returns 0 on success.
 */
int mgrp_subscribe(struct mg_group *grp, int my_fd)
{
	/* TODO: add 'my_fd' to RCU linked list */
}

/* mgrp_unsubscribe()
 * Remove 'my_fd' from 'grp'.
 * Returns 0 on success.
 */
int mgrp_unsubscribe(struct mg_group *grp, int my_fd)
{
	/* TODO: remove from RCU linked list */
}

/* mgrp_broadcast()
 * Send 'len' bytes from 'data' to each member of 'grp'.
 * Same semantics as write().
 */
int mgrp_broadcast(struct mg_group *grp, void *data, size_t len)
{
	int err_cnt = 0;
	int curr_fd = 0;
	MG_ASSEMBLE
	/* TODO: walk linked list
	while (curr_fd = rcu_next(grp->fd_rcu))
	*/
		NB_err_if(write(curr_fd, &mg, PIPE_BUF) != PIPE_BUF,
			"curr_fd %d", curr_fd);
	if (err_cnt)
		return -1;
	return len;
}
