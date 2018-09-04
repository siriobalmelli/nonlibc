#ifndef binhex_h_
#define binhex_h_

/*	binhex.h
 * 
 * The library of binary -> hex and hex -> binary functions.
 *
 * (c) Sirio Balmelli
 */
#include <nonlibc.h>
#include <stdint.h>
#include <string.h> /* memset() */


NLC_PUBLIC	size_t b2hx(const unsigned char *bin, char *hex, size_t byte_cnt);
/* Big Endian */
NLC_PUBLIC	size_t b2hx_BE(const unsigned char *bin, char *hex, size_t byte_cnt);


NLC_PUBLIC	size_t b2hx_u16(const uint16_t *u16, size_t u_cnt, char *hex);
NLC_PUBLIC	size_t b2hx_u32(const uint32_t *u32, size_t u_cnt, char *hex);
NLC_PUBLIC	size_t b2hx_u64(const uint64_t *u64, size_t u_cnt, char *hex);


/* internal things - expose them because they may be useful elsewhere */
NLC_PUBLIC	const char *hex_burn_leading(const char *hex);
NLC_PUBLIC	uint8_t hex_parse_nibble(const char *hex);


/* use this one for single bytes and long sequences */
NLC_PUBLIC	size_t hx2b(const char *hex, uint8_t *out, size_t bytes);

NLC_INLINE uint8_t	hx2b_u8		(const char *hex)
{
	#pragma GCC diagnostic ignored "-Wuninitialized"
		uint8_t ret;
	hx2b(hex, &ret, sizeof(ret));
	return ret;
}

NLC_INLINE uint16_t	hx2b_u16	(const char *hex)
{
	union bits__ {
		uint16_t	u16;
		uint8_t		u8[2];
	};
	#pragma GCC diagnostic ignored "-Wuninitialized"
		union bits__ ret;
	hx2b(hex, ret.u8, sizeof(uint16_t));
	return ret.u16;
}

NLC_INLINE uint32_t	hx2b_u32	(const char *hex)
{
	union bits__ {
		uint32_t	u32;
		uint8_t		u8[4];
	};
	#pragma GCC diagnostic ignored "-Wuninitialized"
		union bits__ ret;
	hx2b(hex, ret.u8, sizeof(uint32_t));
	return ret.u32;
}

NLC_INLINE uint64_t	hx2b_u64	(const char *hex)
{
	union bits__ {
		uint64_t	u64;
		uint8_t		u8[8];
	};
	#pragma GCC diagnostic ignored "-Wuninitialized"
		union bits__ ret;
	hx2b(hex, ret.u8, sizeof(uint64_t));
	return ret.u64;
}
#endif /* binhex_h_ */
