/*	nstring_test.c
 *
 * (c) 2018 Sirio Balmelli
 */

#include <nstring.h>
#include <ndebug.h>


static const char source[] = "a_string";


/*	main()
 */
int main()
{
	int err_cnt = 0;
	size_t ret;
	char *alloc = NULL;

	/* identical copy */
	char identical[sizeof(source)];
	ret = nstrcpy(identical, source, sizeof(identical), false);
	NB_die_if(ret != strlen(source), "");
	NB_die_if(strcmp(source, identical), "");

	/* truncated copy */
	char trunc[4];
	ret = nstrcpy(trunc, source, sizeof(trunc), false);
	NB_die_if(errno != E2BIG, "");			errno=0;
	NB_die_if(ret != strlen(trunc), "");
	NB_die_if(strncmp(source, trunc, ret), "");

	/* padded copy */
	char padded[64];
	ret = nstrcpy(padded, source, sizeof(padded), true);
	NB_die_if(ret != strlen(padded), "");
	NB_die_if(strncmp(source, padded, ret), "");
	for (int i=ret; i < sizeof(padded); i++) {
		NB_die_if(padded[i] != '\0', "");
	}

	/* zero-size copy */
	char zero[1];
	ret = nstrcpy(zero, source, 0, false);
	NB_die_if(ret != 0, "");
	NB_die_if(zero[0] != '\0', "");

	/* invalid copy */
	ret = nstrcpy(NULL, source, -1, false);
	NB_die_if(errno != EINVAL, "");			errno = 0;
	NB_die_if(ret != 0, "");

	/* alloc */
	NB_die_if(!(
		alloc = nstralloc(source, sizeof(source))
		), "");
	NB_die_if(strlen(alloc) != sizeof(source) -1, "");
	NB_die_if(strcmp(source, alloc), "");
	free(alloc);					alloc = NULL;

	/* truncated alloc */
	size_t size = 6;
	NB_die_if(!(
		alloc = nstralloc(source, size)
		), "");
	NB_die_if(errno != E2BIG, "");			errno = 0;
	NB_die_if(strlen(alloc) != size -1, "");
	NB_die_if(strncmp(source, alloc, size-1), "");
	free(alloc);					alloc = NULL;

	/* zero alloc */
	NB_die_if((
		alloc = nstralloc(source, 0)
		) != NULL, "");

die:
	free(alloc);
	return err_cnt;
}
