/*	fnvsum.c
 * A command-line application, on the pattern of md5sum,
 * which applies the 64-bit FNV1a algorithm to the named filed
 * (or standard input if blank).
 *
 * (c) 2017 Sirio Balmelli; https://b-ad.ch
 */

#include <ndebug.h>
#include <fnv.h>
#include <getopt.h>
#include <limits.h> /* PIPE_BUF */

#include <sys/types.h> /* stat() */
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h> /* open() */
#include <sys/mman.h> /* mmap() */
#include <stdlib.h> /* strtol */


/*	do_stdin()
 * Hash the contents of stdin; until EOF or errno do us part.
 */
int	do_stdin(size_t bitlength)
{
	int err_cnt = 0;
	/* The smallest unit of atomic FD I/O
	 * (in this case, the fd underlying fread())
	 * This is as good a constant as any.
	 */
	uint8_t buf[PIPE_BUF];
	size_t size;

	if (bitlength == 64) {
		uint64_t hash = fnv_hash64(NULL, NULL, 0);
		/* full buffer blocks */
		while ((size = fread(buf, sizeof(buf[0]), PIPE_BUF, stdin)) == PIPE_BUF)
			hash = fnv_hash64(&hash, buf, PIPE_BUF);
		/* Hash trailing blocks.
		 * NOTE that '0' is a valid size to hash; it does nothing.
		 */
		hash = fnv_hash64(&hash, buf, size);
		printf("%"PRIx64"  -\n", hash);

	} else if (bitlength == 32) {
		uint32_t hash = fnv_hash32(NULL, NULL, 0);
		while ((size = fread(buf, sizeof(buf[0]), PIPE_BUF, stdin)) == PIPE_BUF)
			hash = fnv_hash32(&hash, buf, PIPE_BUF);
		hash = fnv_hash32(&hash, buf, size);
		printf("%"PRIx32"  -\n", hash);

	} else if (bitlength == 16) {
		uint16_t hash = fnv_hash16(NULL, NULL, 0);
		while ((size = fread(buf, sizeof(buf[0]), PIPE_BUF, stdin)) == PIPE_BUF)
			hash = fnv_hash16(&hash, buf, PIPE_BUF);
		hash = fnv_hash16(&hash, buf, size);
		printf("%"PRIx16"  -\n", hash);

	} else {
		NB_err("bitlength %zu invalid", bitlength);
	}
	return err_cnt;
}


/*	do_file()
 * Hash a file, print the results;
 */
int do_file(const char *file, size_t bitlength)
{
	int err_cnt = 0;
	int fd = -1;
	void *map = NULL;

	/* a '-' file is handled as stdin */
	if (file[0] == '-')
		return do_stdin(bitlength);

	/* stat file; sanity */
	struct stat st;
	NB_die_if( stat(file, &st),
		"error stat()ing '%s'", file);
	NB_die_if(!S_ISREG(st.st_mode),
		"'%s' not a regular file", file);

	if (st.st_size) {
		/* open file */
		NB_die_if((
			fd = open(file, O_RDONLY)
			) == -1, "open() '%s'", file);
		/* map it */
		NB_die_if((
			map = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0)
			) == MAP_FAILED, "mmap() '%s'", file);
	}

	if (bitlength == 64) {
		uint64_t hash = fnv_hash64(NULL, map, st.st_size);
		printf("%"PRIx64"  %s\n", hash, file);

	} else if (bitlength == 32) {
		uint32_t hash = fnv_hash32(NULL, map, st.st_size);
		printf("%"PRIx32"  %s\n", hash, file);

	} else if (bitlength == 16) {
		uint16_t hash = fnv_hash16(NULL, map, st.st_size);
		printf("%"PRIx16"  %s\n", hash, file);

	} else {
		NB_err("bitlength %zu invalid", bitlength);
	}

die:
	if (map && map != MAP_FAILED)
		munmap(map, st.st_size);
	if (fd != -1)
		close(fd);
	return err_cnt;
}


/* Use as a printf prototype.
 * Expects 'program_name' as a string variable.
 */
static const char *usage =
"usage:\n"
"\t%s [OPTIONS] [FILE]\n"
"\n"
"Calculate the 64-bit FNV1a hash of FILE.\n"
"Read from standard input if no FILE or when FILE is '-'\n"
"\n"
"Options:\n"
"\t-l, --length LENGTH	: return a hash of LENGTH bits (currently supported: 64, 32, 16)\n"
"\t-h, --help		: print usage and exit\n";


/*	main()
 */
int main(int argc, char **argv)
{
	int err_cnt = 0;
	size_t bitlength = 64;

	{
		int opt;
		static struct option long_options[] = {
			{ "length",	required_argument,	0,	'l'},
			{ "help",	no_argument,		0,	'h'},
			{0, 0, 0, 0}
		};
		while ((opt = getopt_long(argc, argv, "l:h", long_options, NULL)) != -1) {
			switch(opt) {
			case 'l':
				NB_die_if(!optarg, "optarg not provided");
				bitlength = strtol(optarg, NULL, 10);
				break;
			case 'h':
				fprintf(stderr, usage, argv[0]);
				goto die;
				break;
			default:
				NB_die(""); /* libc will already complain about invalid option */
			}
		}
	}

	/* no args means "hash from stdint" */
	if (optind == argc)
		return do_stdin(bitlength);

	/* otherwise, look for files */
	while (optind < argc)
		err_cnt += do_file(argv[optind++], bitlength);

die:
	return err_cnt;
}
