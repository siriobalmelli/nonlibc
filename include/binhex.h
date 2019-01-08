#ifndef binhex_h_
#define binhex_h_

/*	binhex.h
 * The library of binary -> hex and hex -> binary functions.
 *
 * (c) Sirio Balmelli
 */
#include <nonlibc.h>
#include <stdint.h>
#include <string.h> /* memset() */
#include <endian.h>


NLC_PUBLIC	size_t b2hx(const unsigned char *bin, char *hex, size_t byte_cnt);
/* Big Endian */
NLC_PUBLIC	size_t b2hx_BE(const unsigned char *bin, char *hex, size_t byte_cnt);


NLC_PUBLIC	size_t b2hx_u16(const uint16_t *u16, size_t u_cnt, char *hex);
NLC_PUBLIC	size_t b2hx_u32(const uint32_t *u32, size_t u_cnt, char *hex);
NLC_PUBLIC	size_t b2hx_u64(const uint64_t *u64, size_t u_cnt, char *hex);


/* internal things - expose them because they may be useful elsewhere */
NLC_PUBLIC	const char *hex_burn_leading(const char *hex);
NLC_INLINE	int hex_parse_nibble(const char *hex, uint8_t *out)
{
	/* We take advantage of the fact that sets of characters appear in
	 * the ASCII table in sequence ;)
	 */
	switch (*hex) {
		case '0'...'9':
			*out = (*hex) & 0xf;
			return 0;
		case 'A'...'F':
		case 'a'...'f':
			*out = 9 + ((*hex) & 0xf);
			return 0;
		default:
			return 1;
	}
}


/* use this one for single bytes and long sequences */
NLC_PUBLIC	size_t hx2b(const char *hex, uint8_t *out, size_t bytes);
/* Big Endian */
NLC_PUBLIC	size_t hx2b_BE(const char *hex, uint8_t *out, size_t bytes);

NLC_INLINE uint8_t	hx2b_u8		(const char *hex)
{
	#pragma GCC diagnostic ignored "-Wuninitialized"
	uint8_t ret;
	hx2b(hex, &ret, sizeof(ret));
	return ret;
}

NLC_INLINE uint16_t	hx2b_u16	(const char *hex)
{
	#pragma GCC diagnostic ignored "-Wuninitialized"
	uint16_t ret;
	hx2b(hex, (uint8_t *)&ret, sizeof(uint16_t));
	return le16toh(ret);
}

NLC_INLINE uint32_t	hx2b_u32	(const char *hex)
{
	#pragma GCC diagnostic ignored "-Wuninitialized"
	uint32_t ret;
	hx2b(hex, (uint8_t *)&ret, sizeof(uint32_t));
	return le32toh(ret);
}

NLC_INLINE uint64_t	hx2b_u64	(const char *hex)
{
	#pragma GCC diagnostic ignored "-Wuninitialized"
	uint64_t ret;
	hx2b(hex, (uint8_t *)&ret, sizeof(uint64_t));
	return le64toh(ret);
}
#endif /* binhex_h_ */
