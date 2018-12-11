/*
	nmem_bsd.c	BSD-specific implementation for nmem
*/

#include <nmem.h>
#include <ndebug.h>
#include <npath.h>
#include <fcntl.h>
#include <sys/param.h> /* MAXPATHLEN */


/*	nmem_alloc()
*/
int nmem_alloc(size_t len, const char *tmp_dir, struct nmem *out)
{
	int err_cnt = 0;
	char *tmpfile = NULL;
	NB_die_if(!len || !out, "args");
	out->len = len;

	if (!tmp_dir)
		tmp_dir = "/tmp";
	tmpfile = n_join(tmp_dir, ".temp_XXXXXX");
	

	/* open an fd */
	out->o_flags = 0;
	NB_die_if((
		out->fd = mkstemp(tmpfile)
		) == -1, "failed to open temp file in %s", tmp_dir);
	/* TODO: must figure out how to unlink from filesystem for security
		... while still being able to link INTO filesystem at 'nmem_free'
		below. !
	NB_die_if((
		unlink(tmpfile)
		) == -1, "unlink %s", tmpfile);
	*/

	/* size and map */
	NB_die_if(ftruncate(out->fd, out->len), "len=%ld", out->len);
	NB_die_if((
		out->mem = mmap(NULL, out->len, PROT_READ | PROT_WRITE, MAP_SHARED, out->fd, 0)
		) == MAP_FAILED, "map sz %zu fd %"PRId32, out->len, out->fd);

	return 0;
die:
	free(tmpfile);
	nmem_free(out, NULL);
	return err_cnt;
}


/*	nmem_free()
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
	if (deliver_path) {
		char path[MAXPATHLEN];
		NB_die_if((
			fcntl(nm->fd, F_GETPATH, path)
			) == -1, "");		
		NB_die_if((
			rename(path, deliver_path)
			) == -1, "%s -> %s", path, deliver_path);
	}

die:
	/* close */
	if (nm->fd != -1)
		close(nm->fd);
	nm->fd = -1;
}


/*	nmem_in_splice()
*/
ssize_t nmem_in_splice(struct nmem	*nm,
     			size_t		offset,
     			size_t		len,
     			int		fd_pipe_from)
{
	ssize_t ret = -1;
	NB_die_if(!nm || fd_pipe_from < 1, "args");

	ret = read(fd_pipe_from, nm->mem + offset, len);
	NB_die_if(ret < 0, "len %zu", len);
die:
	return ret;
}


/*	nmem_out_splice()
*/
ssize_t nmem_out_splice(struct nmem	*nm,
			size_t		offset,
			size_t		len,
			int		fd_pipe_to)
{
	ssize_t ret = -1;
	NB_die_if(!nm || fd_pipe_to < 1, "args");

	/* Owing to DARWIN's inevitable, demoralizing behavior of BLOCKING when
		write()ing to a pipe,
		regardless of a previously successful fcntl() setting O_NONBLOCK!

	In case you're wondering, this is however STILL better than
		a poke in the eye with a sharp memcpy().
	*/
	if (len > PIPE_BUF)
		len = PIPE_BUF;
	//NB_inf("len %zu; offt %zu", len, offset);

	ret = write(fd_pipe_to, nm->mem + offset, len);
	NB_die_if(ret < 0, "len %zu", len);
die:
	return ret;
}


/*	nmem_cp()
Copy 'len' bytes between 'src' and 'dst' at their respective offsets.
Returns bytes copied; may be less than requested.
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

	NB_die_if((
		lseek(src->fd, src_offt, SEEK_SET)
		) < 0, "seek to %zu fail", src_offt);

	while (done < len)
		done += nmem_in_splice(dst, dst_offt, len-done, src->fd);

die:
	return done;
}
