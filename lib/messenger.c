/*	messenger.c
 * (c) 2018 Sirio Balmelli and Anthony Soenen
 */

#include <unistd.h>
#include <sys/uio.h>

#include <ndebug.h>
#include <nonlibc.h>
#include <messenger.h>


/* our 'size' variable on this platform *must* be large enough
 * to express the largest atomic write size for a pipe (PIPE_BUF)
 */
NLC_ASSERT(mg_size_sanity, INT_FAST16_MAX > PIPE_BUF);


ssize_t mg_send(int to_fd, void *data, size_t len)
{
	if (len > MG_MAX) \
		return -1;
	uint_fast16_t sz = len;

	struct iovec gather[2] = {
		{ .iov_base = &sz, .iov_len = sizeof(sz) },
		{ .iov_base = data, .iov_len = sz }
	};
	ssize_t ret = writev(to_fd, gather, 2);

	/* hide our header size from the caller */
	if (ret > 1)
		return len;
	return ret; /* because there is a difference between 0 and -1 */
}

/*	mg_recv()
 * Get a message in 'from_fd' and write it to 'data_out'.
 * Return behavior and semantics are same as for libc read().
 */
ssize_t mg_recv(int from_fd, void *data_out)
{
	uint_fast16_t sz = 0;
	ssize_t ret;
	ret = read(from_fd, &sz, sizeof(sz));
	if (ret > 0) {
		ret = read(from_fd, data_out, sz);
		if (ret > 0)
			return sz;
	}

	return ret;
}


/*	mgrp_free()
 */
void mgrp_free(struct mgrp *grp)
{
	if (!grp)
		return;

	struct mgrp_membership *curr = NULL;
	rcu_read_lock();
	cds_hlist_for_each_entry_rcu_2(curr, &grp->members, node) {
		NB_wrn("free fd %d (bad)", curr->in_fd);
		cds_hlist_del_rcu(&curr->node);
		free(curr);
	}
	rcu_read_unlock();

	free(grp);
}

/*	mgrp_new()
 * Create a new messaging group, return pointer,
 * which must be freed with mgrp_free().
 */
struct mgrp *mgrp_new()
{
	struct mgrp *grp = NULL;
	NB_die_if(!(
		grp = calloc(1, sizeof *grp)
		), "fail alloc size %zu", sizeof(*grp));
	return grp;
die:
	mgrp_free(grp);
	return NULL;
}


/*	mgrp_subscribe()
 * Subscribe 'my_fd' to receive notifications by 'grp'.
 * Returns 0 on success.
 */
int mgrp_subscribe(struct mgrp *grp, int my_fd)
{
	int err_cnt = 0;
	struct mgrp_membership *mem = NULL;
	NB_die_if(!(
		mem = calloc(1, sizeof(*mem))
		), "fail alloc size %zu", sizeof(*mem));
	mem->in_fd = my_fd;

	rcu_read_lock();
	cds_hlist_add_head_rcu(&mem->node, &grp->members);
	rcu_read_unlock();
	NB_inf("++ fd %d", mem->in_fd);

	__atomic_add_fetch(&grp->count, 1, __ATOMIC_RELEASE);
	return err_cnt;

die:
	free(mem);
	return err_cnt;
}

/*	mgrp_unsubscribe()
 * Remove 'my_fd' from 'grp'.
 * Returns 0 if membership removed.
 */
int mgrp_unsubscribe(struct mgrp *grp, int my_fd)
{
	int ret = 1;

	struct mgrp_membership *curr = NULL;
	rcu_read_lock();
	cds_hlist_for_each_entry_rcu_2(curr, &grp->members, node) {
		if (curr->in_fd == my_fd) {
			cds_hlist_del_rcu(&curr->node);
			free(curr);
			__atomic_sub_fetch(&grp->count, 1, __ATOMIC_RELEASE);
			ret = 0;
			break;
		}
	}
	rcu_read_unlock();

	return ret;
}

/*	mgrp_broadcast()
 * Send 'len' bytes from 'data' to each member of 'grp' except for caller.
 * Same semantics as write().
 * Returns number of failed sends.
 */
int mgrp_broadcast(struct mgrp *grp, int my_fd, void *data, size_t len)
{
	int err_cnt = 0;

	struct mgrp_membership *curr = NULL;
	rcu_read_lock();
	cds_hlist_for_each_entry_rcu_2(curr, &grp->members, node) {
		/* don't send to self */
		if (curr->in_fd == my_fd)
			continue;

		if (mg_send(curr->in_fd, data, len) != len)
			err_cnt++;
	}
	rcu_read_unlock();

	return err_cnt;
}
