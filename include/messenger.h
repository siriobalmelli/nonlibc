#ifndef messenger_h_
#define messenger_h_

/* messenger - the atomic pipe-based messaging library
 *
 * Leverage the fact that PIPE_BUF is guaranteed atomic to pass simple messages
 * between threads which are listening (e.g. with epoll) to a pipe.
 *
 * Useful when avoiding global state by passing messages between threads/coroutines,
 * or when the program has run-to-sleep semantics (wakelocks on Android).
 *
 * Provides a simple "group" registration and message broadcast mechanism
 * using RCU to track membership.
 *
 * Eminently thread-safe.
 *
 * (c) 2018 Sirio Balmelli and Anthony Soenen
 */
#include <stdint.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <limits.h> /* PIPE_BUF */
#include <stdlib.h>
#include <stddef.h>
/*
 *	message passing functions
 * NOTE: these are orthogonal to mg_subscribe/unsubscribe/alert (messenger)
 * semantics below/
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
/* TODO: doing '- sizeof(struct message)' makes MG_MAX always equal to 0 */
#define MG_MAX (PIPE_BUF - (sizeof(uint_fast16_t)))

ssize_t mg_send(int to_fd, void *data, size_t len);
ssize_t mg_recv(int from_fd, void *data_out);


/*
 *	message passing groups
 */
struct mg_group {
	/* TODO: proper type for RCU */
	void	*fd_rcu;
};

void mgrp_free(struct mg_group *grp);
struct mg_group *grprp_new();

int mgrp_subscribe(struct mg_group *grp, int my_fd);
int mgrp_unsubscribe(struct mg_group *grp, int my_fd);
int mgrp_broadcast(struct mg_group *grp, void *data, size_t len);

#endif /* messenger_h_ */
