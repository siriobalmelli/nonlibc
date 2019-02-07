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
	size_t len;
	char *alloc = NULL;

	/* identical copy */
	char identical[sizeof(source)];
	len = nstrcpy(identical, source, sizeof(identical), false);
	NB_die_if(len != strlen(source) || len != strlen(identical), "");
	NB_die_if(strcmp(source, identical), "");

	/* truncated copy */
	char trunc[4];
	len = nstrcpy(trunc, source, sizeof(trunc), false);
	NB_die_if(errno != E2BIG, "");			errno=0;
	NB_die_if(len != strlen(trunc), "");
	NB_die_if(strncmp(source, trunc, len), "");

	/* padded copy */
	char padded[64];
	len = nstrcpy(padded, source, sizeof(padded), true);
	NB_die_if(len != strlen(padded), "");
	NB_die_if(strncmp(source, padded, len), "");
	for (int i=len; i < sizeof(padded); i++) {
		NB_die_if(padded[i] != '\0', "");
	}

	/* zero-size copy */
	char zero[1];
	len = nstrcpy(zero, source, 0, false);
	NB_die_if(len != 0, "");
	NB_die_if(zero[0] != '\0', "");

	/* invalid copy */
	len = nstrcpy(NULL, source, -1, false);
	NB_die_if(errno != EINVAL, "");			errno = 0;
	NB_die_if(len != 0, "");

	/* alloc */
	NB_die_if(!(
		alloc = nstralloc(source, sizeof(source), &len)
		), "");
	NB_die_if(len != strlen(source) || len != strlen(alloc), "");
	NB_die_if(strcmp(source, alloc), "");
	free(alloc);					alloc = NULL;

	/* truncated alloc */
	NB_die_if(!(
		alloc = nstralloc(source, 6, &len)
		), "");
	NB_die_if(errno != E2BIG, "");			errno = 0;
	NB_die_if(len != strlen(alloc), "");
	NB_die_if(strncmp(source, alloc, len), "");
	free(alloc);					alloc = NULL;

	/* zero alloc */
	NB_die_if(!(
		alloc = nstralloc(source, 0, &len)
		), "");
	NB_die_if(len, "");
	free(alloc);					alloc = NULL;

die:
	free(alloc);
	return err_cnt;
}
