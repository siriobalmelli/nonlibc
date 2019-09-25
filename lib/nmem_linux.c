/*
	nmem_linux.c	linux-specific nmem implementation
*/

#include <nmem.h>
#include <ndebug.h>
#include <limits.h> /* PIPE_BUF, PATH_MAX */

/*	nmem_alloc()
Map 'len' bytes of memory.
If 'tmp_dir' is given, map this as a temp file on disk;
	so that it may be "delivered" on close (nmem_free).
Otherwise map anonymous memory.

In both cases a valid fd is obtained, so that splice()
	calls will succeed into and out of this memory region.
*/
int nmem_alloc(size_t len, const char *tmp_dir, struct nmem *out)
{
	int err_cnt = 0;
	NB_die_if(!len || !out, "args");
	out->len = len;

	#if LINUX_VERSION_CODE < KERNEL_VERSION(3,11,0)
	out->tempfile = NULL;
	#endif

	if (!tmp_dir) {
		/* memfd_create() as a syscall.
		The advantage of memfd is that data is never written back to disk,
			yet we can splice into it as if it were a file.
		If not available, we fall back on a plain mmap()ed file in "/tmp".
		*/
		#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,17,0)
		out->o_flags = O_NONBLOCK;
		char name[16];
		snprintf(name, 16, "nmem_%zu", out->len);
		NB_die_if((
			out->fd = syscall(__NR_memfd_create, name, out->o_flags)
			) == -1, "");
		/* fallback: create a temp file on disk */
		#else
		tmp_dir = "/tmp";
		#endif
	}

	/* open an fd */
	if (tmp_dir) {
		out->o_flags = O_RDWR | O_NONBLOCK;

		/* previous kernels don't implement O_TMPFILE */
		#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,11,0)
		out->o_flags |= O_TMPFILE;
		NB_die_if((
			out->fd = open(tmp_dir, out->o_flags, NMEM_PERMS)
			) == -1, "failed to open temp file in %s", tmp_dir);

		#else
		/* If your kernel is this old, you probably care more about
 		 * pedantic safety than speed.
 		 */
		const char *temp_template = "ncp_tempXXXXXX";
		size_t alloc_len = strnlen(temp_template, PATH_MAX)
			+ strnlen(tmp_dir, PATH_MAX)
			+ 2; /* allow for middle '/' and trailing '\0' */
		NB_die_if(alloc_len > PATH_MAX, "tmp_dir path too long: '%s'", tmp_dir);
		NB_die_if(!(
			out->tempfile = malloc(alloc_len)
			), "failed alloc zs %zu", alloc_len);
		snprintf(out->tempfile, alloc_len, "%s/%s", tmp_dir, temp_template);

		NB_die_if((
			out->fd = mkostemp(out->tempfile, out->o_flags)
			) == -1, "failed to open temp file '%s'", out->tempfile);
		#endif
	}

	/* size and map */
	NB_die_if(ftruncate(out->fd, out->len), "len=%zu", out->len);
	NB_die_if((
		out->mem = mmap(NULL, out->len, PROT_READ | PROT_WRITE, MAP_SHARED, out->fd, 0)
		) == MAP_FAILED, "map sz %zu fd %"PRId32, out->len, out->fd);

	return 0;
die:
	nmem_free(out, NULL);
	return err_cnt;
}


/*	nmem_free()
Free the memory pointed to by '*nm'; clear '*nm'
*/
void nmem_free(struct nmem *nm, const char *deliver_path)
{
	if (!nm)
		return;

	/* unmap */
	if (nm->mem && nm->mem != MAP_FAILED)
		munmap(nm->mem, nm->len);
	nm->mem = NULL;

	/* deliver if requested */
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,11,0)
	if (deliver_path && (nm->o_flags & O_TMPFILE)) {
		char src[32];
		snprintf(src, 32, "/proc/self/fd/%d", nm->fd);
		NB_err_if(
			linkat(AT_FDCWD, src, AT_FDCWD, deliver_path, AT_SYMLINK_FOLLOW)
			, "%s -> %s", src, deliver_path);
	}
	#else
	if (deliver_path) {
		NB_err_if(
			link(nm->tempfile, deliver_path)
			, "failed to link '%s' -> '%s'", nm->tempfile, deliver_path);
		NB_err_if(
			unlink(nm->tempfile)
			, "failed to unlink tempfile '%s'", nm->tempfile);
	}
	#endif

	/* close */
	if (nm->fd != -1)
		close(nm->fd);
	nm->o_flags = 0;
	nm->fd = -1;

	#if LINUX_VERSION_CODE < KERNEL_VERSION(3,11,0)
	free(nm->tempfile);
	#endif
}


/*	nmem_in_splice()
Splice 'len' bytes from 'fd_pipe_from' into 'nm' at 'offset'.
Returns number of bytes pushed; which may be less than requested.
*/
ssize_t	nmem_in_splice(struct nmem	*nm,
			size_t		offset,
			size_t		len,
			int		fd_pipe_from)
{
	ssize_t ret = -1;
	NB_die_if(!nm || fd_pipe_from < 1, "args");

	ret = splice(fd_pipe_from, NULL, nm->fd, (loff_t*)&offset,
				len, NMEM_SPLICE_FLAGS);

	/* some systems (ARMv7 that I know of) get finicky - provide a sane fallback */
	if NLC_UNLIKELY(ret == -1) {
		if (len > PIPE_BUF)
			len = PIPE_BUF;
		ret = read(fd_pipe_from, nm->mem + offset, len);
	}

	NB_die_if(ret < 0, "len %zu @%zu offt; fd_pipe_from %d -> nm->fd %d",
			len, offset, fd_pipe_from, nm->fd);
die:
	return ret;
}


/*	nmem_out_splice()
Splice 'len' bytes from 'nm' into 'fd_pipe_to' at 'offset' (if applicable).
Returns number of bytes pushed; which may be less than requested.
*/
ssize_t nmem_out_splice(struct nmem	*nm,
			size_t		offset,
			size_t		len,
			int		fd_pipe_to)
{
	ssize_t ret = -1;
	NB_die_if(!nm || fd_pipe_to < 1, "args");

	ret = splice(nm->fd, (loff_t*)&offset, fd_pipe_to, NULL,
				len, NMEM_SPLICE_FLAGS);

	/* some systems (ARMv7 that I know of) get finicky - provide a sane fallback */
	if NLC_UNLIKELY(ret == -1) {
		if (len > PIPE_BUF)
			len = PIPE_BUF;
		ret = write(fd_pipe_to, nm->mem + offset, len);
	}

	NB_die_if(ret < 0, "len %zu @%zu offt; nm->fd %d -> fd_pipe_to %d",
			len, offset, nm->fd, fd_pipe_to);
die:
	return ret;
}


/*	nmem_cp()
Returns number of bytes copied, may be less than requested.
*/
size_t nmem_cp(struct nmem	*src,
		size_t		src_offt,
		size_t		len,
		struct nmem	*dst,
		size_t		dst_offt)
{
	size_t done = 0;

	/* sanity */
	if (len > src->len - src_offt)
		len = src->len - src_offt;
	if (len > dst->len - dst_offt)
		len = dst->len - dst_offt;

	int piping[2] = { -1, -1 };
	NB_die_if(pipe2(piping, O_NONBLOCK), "");

	/* splicings */
	for (size_t fd_sz=0; done < src->len; ) {
		NB_die_if((
			fd_sz = nmem_out_splice(src, done, src->len-done, piping[1])
			) == -1, "");
		for (size_t temp=0; fd_sz > 0; ) {
			NB_die_if((
				temp = nmem_in_splice(dst, done, fd_sz, piping[0])
				) == -1, "");
			done += temp;
			fd_sz -= temp;
		}
	}

die:
	if (piping[0] != -1) {
		close(piping[0]);
		close(piping[1]);
	}
	return done;
}
