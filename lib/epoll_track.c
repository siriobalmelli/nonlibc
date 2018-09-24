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

	struct epoll_track_cb *curr = NULL, *e = NULL;
	/* use _safe_ version: freeing while walking */
	cds_hlist_for_each_entry_safe_2(curr, e, &tk->cb_list, node) {
		if (close_children)
			close(curr->fd);
		cds_hlist_del(&curr->node);
		free(curr);
	}

	free(tk->report);
	free(tk);
}

/*	eptk_new()
 * Initial allocation of tracking structure for an epoll group/fd;
 * to track a fd, register it with eptk_register()
 * Returns NULL on error
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
 * @fd		fd to be tracked using epoll
 * @events	events to be tracked, see 'man epoll_ctl'
 * @context	optional opaque value to be passed to callback
 * @callback
 * Returns 0 on success
 */
int eptk_register(struct epoll_track *tk, int fd, uint32_t events,
		void (*callback)(int fd, uint32_t events, epoll_data_t context),
		epoll_data_t context)
{
	int err_cnt = 0;
	int e_flag = EPOLL_CTL_ADD;
	struct epoll_track_cb *new_cb = NULL;
	Z_die_if(!tk || fd < 0 || !events || !callback, "");

	/* look for existing callbacks on the same fd and modify instead */
	struct epoll_track_cb *curr;
	cds_hlist_for_each_entry_2(curr, &tk->cb_list, node) {
		if (curr->fd == fd) {
			new_cb = curr;
			e_flag = EPOLL_CTL_MOD;
			break;
		}
	}

	/* if no existing callback found, alloc for a new one */
	if (!new_cb) {
		/* extend report array */
		size_t alloc_sz = ++tk->rcnt * sizeof(struct epoll_event);
		Z_die_if(!(
			tk->report = realloc(tk->report, alloc_sz)
			), "fail alloc sz %zu", alloc_sz);

		/* add new callback to list */
		Z_die_if(!(
			new_cb = malloc(sizeof(*new_cb))
			), "fail alloc sz %zu", sizeof(*new_cb));
	}

	/* register connection */
	new_cb->fd = fd;
	new_cb->events = events;
	new_cb->callback = callback;
	new_cb->context = context;
	struct epoll_event ep_in = {
		.data.ptr = new_cb,
		.events = new_cb->events
	};
	Z_die_if(
		epoll_ctl(tk->epfd, e_flag, new_cb->fd, &ep_in)
		, "epfd %d; register fail for fd %d", tk->epfd, new_cb->fd);

	/* don't add to list until epoll succeeded */
	if (e_flag == EPOLL_CTL_ADD)
		cds_hlist_add_head(&new_cb->node, &tk->cb_list);
	Z_log(Z_in2, "register " EPTK_CB_PRN(new_cb));

	return err_cnt;
out:
	/* failed modify must not leave list in an inconsistent state */
	if (e_flag == EPOLL_CTL_MOD)
		cds_hlist_del(&new_cb->node);
	free(new_cb);
	return err_cnt;
}

/*	eptk_remove()
 * Remove 'fd' from 'tk'.
 * Returns number of records removed.
 */
int eptk_remove(struct epoll_track *tk, int fd)
{
	if (!tk || fd < 0)
		return 0;

	/* walk le list */
	int removed = 0;
	struct epoll_track_cb *curr = NULL, *e = NULL;
	cds_hlist_for_each_entry_safe_2(curr, e, &tk->cb_list, node) {
		if (curr->fd != fd)
			continue;
		cds_hlist_del(&curr->node);
		Z_err_if(
			epoll_ctl(tk->epfd, EPOLL_CTL_DEL, curr->fd, NULL)
			, "epfd %d remove fail for fd %d", tk->epfd, curr->fd);
		removed++;
		free(curr);
	}

	return removed;
}

/*	eptk_pwait_exec()
 * Execute an epoll_pwait, passing it 'timeout' and 'sigmask' directly.
 * If events are returned, execute respective callback on each.
 * Return original return value of epoll_wait(), with errno intact;
 * allow caller to correctly handle EINTR, etc.
 */
int eptk_pwait_exec(struct epoll_track *tk, int timeout, const sigset_t *sigmask)
{
	/* epoll demands that "maxevents argument must be greater than zero" */
	if (!tk->rcnt)
		return 0;

	int ret = epoll_pwait(tk->epfd, tk->report, tk->rcnt, timeout, sigmask);
	/* -1 is less than 0 ;) */
	for (int i=0; i < ret; i++) {
		struct epoll_track_cb *cb = tk->report[i].data.ptr;
		cb->callback(cb->fd, tk->report[i].events, cb->context);
	}
	return ret;
}
