#include <epoll_track.h>
#include <unistd.h>
#include <zed_dbg.h>

/*	eptk_free()
 * @close_children	exec close() on all tracked fds
 */
void eptk_free(struct epoll_track *tk, bool close_children)
{
	if (!tk)
		return;
	if (tk->epfd != -1)
		close(tk->epfd);
	for (int i=0; close_children && i < tk->rcnt; i++) {
		if (tk->cb[i].fd != -1)
			close(tk->cb[i].fd);
	}
	free(tk->cb);
	free(tk->report);
	free(tk);
}

/*	eptk_new()
 * Initial allocation of tracking structure for an epoll group/fd;
 * to track a fd, register it with eptk_register()
 */
struct epoll_track *eptk_new()
{
	struct epoll_track *tk = NULL;
	Z_die_if(!(
		tk = calloc(sizeof(struct epoll_track), 1)
		), "alloc sz %zu", sizeof(struct epoll_track));

	/* setup epoll loop */
	Z_die_if((
		tk->epfd = epoll_create1(0)
		) < 0, "fail to create epoll");

	return tk;
out:
	eptk_free(tk, false);
	return NULL;
}

/*	eptk_register()
 * Register a new fd for tracking with epoll.
 * @cb		caller-populated structure containing:
 * @cb->fd	fd to be tracked using epoll
 * @cb->events	events to be tracked, see 'man epoll_ctl'
 * @cb->context	optional opaque value to be passed to callback
 * @cb->callback
 */
int eptk_register(struct epoll_track *tk, const struct epoll_track_cb *cb)
{
	int err_cnt = 0;
	Z_die_if(!tk || !cb, "");
	Z_die_if(!cb->callback, "");

	/* TODO: look for existing callbacks on the same fd and modify instead */

	/* extend arrays */
	size_t alloc_sz = ++tk->rcnt * sizeof(struct epoll_event);
	Z_die_if(!(
		tk->report = realloc(tk->report, alloc_sz)
		), "fail alloc sz %zu", alloc_sz);
	alloc_sz = tk->rcnt * sizeof(struct epoll_track_cb);
	Z_die_if(!(
		tk->cb = realloc(tk->cb, alloc_sz)
		), "fail alloc sz %zu", alloc_sz);

	/* copy user args, operate on copy */
	memcpy(&tk->cb[tk->rcnt-1], cb, sizeof(*cb));
	struct epoll_track_cb *new_cb = &tk->cb[tk->rcnt-1];

	/* register connection */
	struct epoll_event ep_in = {
		.data.ptr = new_cb,
		.events = new_cb->events
	};
	Z_die_if(
		epoll_ctl(tk->epfd, EPOLL_CTL_ADD, new_cb->fd, &ep_in)
		, "epfd %d; register fail for fd %d", tk->epfd, new_cb->fd);

out:
	return err_cnt;
}

/*	eptk_remove()
TODO: fuuuu needs RCU linked-list
 */
int eptk_remove(int fd)
{
	int err_cnt = 0;
	Z_die("removal not implemented");
out:
	return err_cnt;
}

/*	eptk_pwait_exec()
 * Execute an epoll_pwait, passing it 'timeout' and 'sigmask' directly.
 * If events are returned, execute respective callback on each.
 * Return original return value of epoll_wait(), with errno intact;
 * allow caller to correctly handle EINTR, etc.
 */
int eptk_pwait_exec(struct epoll_track *tk, int timeout, const sigset_t *sigmask)
{
	int ret = epoll_pwait(tk->epfd, tk->report, tk->rcnt, timeout, sigmask);
	for (int i=0; ret > 0 && i < ret; i++) {
		struct epoll_track_cb *cb = tk->report[i].data.ptr;
		cb->callback(cb->fd, cb->events, cb->context);
	}
	return ret;
}
