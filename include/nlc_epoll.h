#ifndef nlc_epoll_h_
#define nlc_epoll_h_

/*	nlc_epoll.h
 *
 * Provide cross-platform epoll() functionality.
 *
 * (c) 2019 Sirio Balmelli
 */
#ifdef __linux__
#include <sys/epoll.h>
#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <epoll_kqueue.h>
#else
#error "epoll compatibility implemented for this OS"
#endif

#endif /* nlc_epoll_h_ */
