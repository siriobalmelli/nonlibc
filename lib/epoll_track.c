#include <epoll_track.h>
#include <unistd.h>
#include <ndebug.h>

/*	eptk_free()
 * @close_children	exec close() on all tracked fds
 */
void eptk_free(struct epoll_track *tk)
{
	if (!tk)
		return;
	if (tk->epfd != -1)
		close(tk->epfd);

	struct epoll_track_cb *curr = NULL, *e = NULL;
	/* use _safe_ version: freeing while walking */
	cds_hlist_for_each_entry_safe_2(curr, e, &tk->cb_list, node) {
		cds_hlist_del(&curr->node);
		if (curr->destructor)
			curr->destructor(curr->context);
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
	NB_die_if(!(
		tk = calloc(sizeof(struct epoll_track), 1)
		), "alloc sz %zu", sizeof(struct epoll_track));

	/* setup epoll loop */
	NB_die_if((
		tk->epfd = epoll_create1(0)
		) < 0, "fail to create epoll");

	return tk;
die:
	eptk_free(tk);
	return NULL;
}

/*	eptk_register()
 * Register a new fd for tracking with epoll.
 * @fd		fd to be tracked using epoll
 * @events	events to be tracked, see 'man epoll_ctl'
 * @callback	called when 'events' trigger epoll
 * @context	optional opaque value to be passed to callback
 * @destructor	(optional) executed on node(s) by eptk_free() and eptk_destroy()
 * Returns 0 on success
 */
int eptk_register(struct epoll_track *tk, int fd, uint32_t events,
		eptk_callback_t callback, eptk_context_t context,
		void (*destructor)(eptk_context_t))
{
	int err_cnt = 0;
	int e_flag = EPOLL_CTL_ADD;
	struct epoll_track_cb *new_cb = NULL;
	NB_die_if(!tk || fd < 0 || !events || !callback, "");

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
		NB_die_if(!(
			tk->report = realloc(tk->report, alloc_sz)
			), "fail alloc sz %zu", alloc_sz);

		/* add new callback to list */
		NB_die_if(!(
			new_cb = malloc(sizeof(*new_cb))
			), "fail alloc sz %zu", sizeof(*new_cb));
	}

	/* register connection */
	new_cb->fd = fd;
	new_cb->events = events;
	new_cb->callback = callback;
	new_cb->context = context;
	new_cb->destructor = destructor;
	struct epoll_event ep_in = {
		.data.ptr = new_cb,
		.events = new_cb->events
	};
	NB_die_if(
		epoll_ctl(tk->epfd, e_flag, new_cb->fd, &ep_in)
		, "epfd %d; register fail for fd %d", tk->epfd, new_cb->fd);

	/* don't add to list until epoll succeeded */
	if (e_flag == EPOLL_CTL_ADD)
		cds_hlist_add_head(&new_cb->node, &tk->cb_list);
	//NB_inf("register " EPTK_CB_PRN(new_cb));

	return err_cnt;
die:
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
		NB_err_if(
			epoll_ctl(tk->epfd, EPOLL_CTL_DEL, curr->fd, NULL)
			, "epfd %d remove fail for fd %d", tk->epfd, curr->fd);
		removed++;
		if (curr->destructor)
			curr->destructor(curr->context);
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
		cb->callback(cb->fd, tk->report[i].events, cb->context, tk);
	}
	return ret;
}
