#ifndef nmem_h_
#define nmem_h_

/*	nmem.h		the Nonlibc Memory library

Library of functions to handle zero-copy I/O
*/

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <fcntl.h> /* splice() */
#include <unistd.h> /* pipe() */
#include <sys/uio.h>
#include <sys/mman.h>
#include <sys/stat.h> /* umask() */

#include <stdint.h> /* uint{x}_t */
#include <nonlibc.h>

#define NMEM_PERMS (mode_t)(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define NMEM_SPLICE_FLAGS ( SPLICE_F_GIFT | SPLICE_F_MOVE )


#ifdef __linux__
#include <nlc_linuxversion.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,17,0)
#include <sys/syscall.h>
#include <linux/memfd.h>
#else
#include <stdlib.h>
#endif
#endif


struct nmem {
	int32_t		fd;
	uint32_t	o_flags; /* only open() flags valid here */
union {
	struct {
		void	*mem;
		size_t	len;
	};
	struct iovec	iov;
};
#ifdef __linux__
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,11,0)
	char		*tempfile;
#endif
#endif
};


NLC_PUBLIC int		nmem_file(const char *path, struct nmem *out);
NLC_PUBLIC int		nmem_alloc(size_t len, const char *tmp_dir, struct nmem *out);
NLC_PUBLIC void		nmem_free(struct nmem *nm, const char *deliver_path);

NLC_PUBLIC ssize_t	nmem_in_splice(struct nmem	*nm,
					size_t		offset,
					size_t		len,
					int		fd_pipe_from);

NLC_PUBLIC ssize_t	nmem_out_splice(struct nmem	*nm,
					size_t		offset,
					size_t		len,
					int		fd_pipe_to);

NLC_PUBLIC size_t	nmem_cp(struct nmem	*src,
				size_t		src_offt,
				size_t		len,
				struct nmem	*dst,
				size_t		dst_offt);

#endif /* nmem_h_ */
