#ifndef nstring_h_
#define nstring_h_

/*	nstring.h
 * Sane string handling for C programs, implemented inline-only.
 * (c) 2018 Sirio Balmelli
 */

#include <nonlibc.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>


/*	nstrcpy()
 * Copy a string of at most 'max-1' characters from 'src' to 'dest',
 * followed by a '\0' (NULL) terminator.
 * Returns number of characters copied, not counting the NULL terminator.
 *
 * Writes NULL terminator to 'dest' if at all possible,
 * regardless of how many characters copied or whether string was truncated:
 * - `max == 0` or `strlen(src) == 0` result in `dest[0] = '\0'`
 * - truncation results in `dest[max-1] = '\0'`
 * If 'zero_pad' is true, pads _all_ trailing bytes with zeroes.
 *
 * Caller should ALWAYS check `errno`:
 * EINVAL	: insane parameters (and therefore could _not_ write to 'dest')
 * E2BIG	: string was truncated
 */
NLC_INLINE size_t nstrcpy(char *dest, const char *src, size_t max, bool zero_pad)
{
	/* sanity */
	if (!dest || !src) {
		errno = EINVAL;
		return 0;
	}

	/* `max == 0` and `ret == 0` are perfectly legitimate */
	size_t ret = strnlen(src, max);

	/* explicitly signal truncation */
	if (ret && ret == max) {
		errno = E2BIG;
		ret--;
	}

	memcpy(dest, src, ret);

	if (zero_pad)
		memset(&dest[ret], 0, max - ret);
	else
		dest[ret] = '\0';

	return ret;
}


/*	nstralloc()
 * Allocate a new string, copy at most 'max-1' size string into it from 'src'.
 * Return allocated string or NULL on error.
 * Write size of allocated string to '*out_len' (if given).
 *
 * Caller should ALWAYS check `errno`:
 * EINVAL	: insane parameters (therefore could not alloc or copy)
 * E2BIG	: string was truncated
 *
 * Caller is responsible for free()ing returned string.
 */
NLC_INLINE char *nstralloc(const char *src, size_t max, size_t *out_len)
{
	char *ret = NULL;
	size_t len = 0;

	/* sanity */
	if (!src) {
		errno = EINVAL;
		goto out;
	}

	len = strnlen(src, max);
	if (!len)
		goto out;

	if (len == max) {
		len--;
		errno = E2BIG;
	}
	if ((ret = malloc(len+1))) {
		memcpy(ret, src, len);
		ret[len] = '\0';
	}

out:
	if (out_len)
		*out_len = len;
	return ret;
}


#endif /* nstring_h_ */
