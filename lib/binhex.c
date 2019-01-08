#include <binhex.h>

/* index into this for b2hx conversions */
static const char *syms = "0123456789abcdef";


/*	b2hx()
 * Writes byte_cnt bytes as (byte_cnt *2 +1) ascii hex digits to the mem in `*out`.
 * (+1 because trailing '\0').
 * Does NOT check that enough mem in `out` exists.
 * 
 * Returns number of CHARACTERS written (should be `byte_cnt *2 +1`)
 * 
 * NOTE that we consider '*bin' to be a SEQUENTIAL FIELD of bytes,
 * with LSB at the EARLIEST memory address (little-endian).
 * If '*bin' would be an array of e.g.: uint32_t, then this function
 * would output the WRONG result; use the uXX_2_hex() functions instead.
 * If '*bin' is big-endian, use b2hx_BE() instead.
 *
 * We output a single hex string, which is in "human" notation: MSB in front.
 *
 * NOTE: 'bin' MUST be unsigned, else array indexing with bitshift yields
 * negative values :O
*/
size_t b2hx(const unsigned char *bin, char *hex, size_t byte_cnt)
{
	if (!bin || !hex || !byte_cnt)
		return 0;

	size_t hex_pos = 0;
	for (int i=byte_cnt-1; i >= 0; i--) {
		hex[hex_pos++] = syms[bin[i] >> 4];
		hex[hex_pos++] = syms[bin[i] & 0xf];
	}
	hex[hex_pos++] = '\0'; /* end of string */

	return hex_pos; /* return number of hex CHARACTERS written */
}

/*	b2hx_be()
 * big-endian version of b2hx() above.
 */
size_t	b2hx_BE	(const unsigned char *bin, char *hex, size_t byte_cnt)
{
	if (!bin || !hex || !byte_cnt)
		return 0;

	size_t hex_pos = 0;
	for (int i=0; i < byte_cnt; i++) {
		hex[hex_pos++] = syms[bin[i] >> 4];
		hex[hex_pos++] = syms[bin[i] & 0xf];
	}
	hex[hex_pos++] = '\0'; /* end of string */

	return hex_pos; /* return number of hex CHARACTERS written */
}


/*	b2hx_u16()
 */
size_t	b2hx_u16(const uint16_t *u16, size_t u_cnt, char *hex)
{
	int j=0;
	union bits__ {
		uint16_t	u16;
		uint8_t		u8[2];
	};
	union bits__ tmp;
	for (int i=0; i < u_cnt; i++) {
		tmp.u16 = __builtin_bswap16(u16[i]);
		hex[j++]= syms[tmp.u8[0] >>4];
		hex[j++]= syms[tmp.u8[0] & 0xf];
		hex[j++]= syms[tmp.u8[1] >>4];
		hex[j++]= syms[tmp.u8[1] & 0xf];
	}
	hex[j++] = '\0';
	return j;
}


/*	b2hx_u32()
 */
size_t	b2hx_u32(const uint32_t *u32, size_t u_cnt, char *hex)
{
	int j = 0;
	union bits__ {
		uint32_t	u32;
		uint8_t		u8[4];
	};
	union bits__ tmp;
	for (int i=0; i < u_cnt; i++) {
		tmp.u32 = __builtin_bswap32(u32[i]);
		hex[j++]= syms[tmp.u8[0] >>4];
		hex[j++]= syms[tmp.u8[0] & 0xf];
		hex[j++]= syms[tmp.u8[1] >>4];
		hex[j++]= syms[tmp.u8[1] & 0xf];
		hex[j++]= syms[tmp.u8[2] >>4];
		hex[j++]= syms[tmp.u8[2] & 0xf];
		hex[j++]= syms[tmp.u8[3] >>4];
		hex[j++]= syms[tmp.u8[3] & 0xf];
	}
	hex[j++] = '\0';
	return j;
}


/*	b2hx_u64()
 */
size_t	b2hx_u64(const uint64_t *u64, size_t u_cnt, char *hex)
{
	int j = 0;
	union bits__ {
		uint64_t	u64;
		uint8_t		u8[8];
	};
	union bits__ tmp;
	for (int i=0; i < u_cnt; i++) {
		tmp.u64 = __builtin_bswap64(u64[i]);
		hex[j++]= syms[tmp.u8[0] >>4];
		hex[j++]= syms[tmp.u8[0] & 0xf];
		hex[j++]= syms[tmp.u8[1] >>4];
		hex[j++]= syms[tmp.u8[1] & 0xf];
		hex[j++]= syms[tmp.u8[2] >>4];
		hex[j++]= syms[tmp.u8[2] & 0xf];
		hex[j++]= syms[tmp.u8[3] >>4];
		hex[j++]= syms[tmp.u8[3] & 0xf];
		hex[j++]= syms[tmp.u8[4] >>4];
		hex[j++]= syms[tmp.u8[4] & 0xf];
		hex[j++]= syms[tmp.u8[5] >>4];
		hex[j++]= syms[tmp.u8[5] & 0xf];
		hex[j++]= syms[tmp.u8[6] >>4];
		hex[j++]= syms[tmp.u8[6] & 0xf];
		hex[j++]= syms[tmp.u8[7] >>4];
		hex[j++]= syms[tmp.u8[7] & 0xf];
	}
	hex[j++] = '\0';
	return j;
}


/*	hex_burn_leading()
 * Ignore any leading '0{0,1}[xh]' sequences in a hex string.
 * Be careful - leading '0' may be '0xaf' or may just be '0a' (!)
 *
 * Returns (possibly shifted) beginning of string.
 */
const char *hex_burn_leading(const char *hex)
{
	if (hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'h') )
		hex += 2;
	else if (hex[0] == 'x' || hex[0] == 'h')
		hex++;
	return hex;
}


/*	COUNT_NIBBLES_COMMON
 * Dedup some code in the following functions.
 * Unless you are a maintainer, this is safe to ignore.
 */
#define COUNT_NIBBLES_COMMON							\
	/* How many nibbles are there ACTUALLY?					\
	 * We may be parsing a 64-bit int expressed in 1 character (LSnibble).	\
	 * Therefore, we must parse backwards.					\
	 */									\
	int nibble_cnt = 0;							\
	while (nibble_cnt < bytes * 2) {					\
		switch (*hex) {							\
			/* valid hex: check next nibble */			\
			case '0'...'9':						\
			case 'A'...'F':						\
			case 'a'...'f':						\
				nibble_cnt++;					\
				hex++;						\
				break;						\
			default:						\
				goto out_while;					\
		}								\
	}									\
out_while:									\
	/* last symbol was either bad or out of bounds. backtrack */		\
	hex--;


/*	hx2b()
 * Parse any quantity of hex digits into (at most) 'bytes' bytes.
 *
 * Expected hex format: '0?[hx]?[0-9,a-f,A-F]+'
 *
 * Ignores leading '0?[hx]?' sequence.
 * Parses in little-endian order; ignores TRAILING superfluous hex digits
 * (e.g. for 'hex -> 0x11223344' and 'bytes = 2'  :  out[0] = 0x22; out[1] = 0x11)
 *
 * memset()s 'bytes' output bytes before writing.
 *
 * Returns numbers of characters/nibbles parsed (excluding ignored leading sequence);
 * 0 is a valid value if nothing parsed.
 */
size_t hx2b(const char *hex, uint8_t *out, size_t bytes)
{
	/* Burn any leading characters */
	hex = hex_burn_leading(hex);
	COUNT_NIBBLES_COMMON

	/* first clean. then parse */
	memset(out, 0x0, bytes);

	/* parse all the nibbles, from the BACK (LSn of LSB == i=0) */
	for (int i=0; i < nibble_cnt; i++, hex--) {
		uint8_t res;
		if (hex_parse_nibble(hex, &res))
			break;
		/* odd nibble == MSn; requires shift */
		if (!(i & 0x1))
			*out = res;
		else
			*(out++) |= res << 4;
	}

	return nibble_cnt;
}

/*	hx2b_BE()
 * big-endian version of hx2b() above.
 *
 * WARNING: one would think that, as human representation is *also* big-endian,
 * this function is faster than hx2b(), since it wouldn't need to pre-scan
 * for the number of nibbles to parse.
 * This is foolish, because of humans' elision of leading 0 digits, and no
 * guarantee that the number of digits/characters/nibbles is _even_.
*/
size_t hx2b_BE(const char *hex, uint8_t *out, size_t bytes)
{
	/* Burn any leading characters */
	hex = hex_burn_leading(hex);
	COUNT_NIBBLES_COMMON

	/* first clean. then parse */
	memset(out, 0x0, bytes);

	/* parse all the nibbles, from the BACK (LSn of LSB == i=0) */
	out += bytes -1;
	for (int i=0; i < nibble_cnt; i++, hex--) {
		uint8_t res;
		if (hex_parse_nibble(hex, &res))
			break;
		/* odd nibble == MSn; requires shift */
		if (!(i & 0x1))
			*out = res;
		else
			*(out--) |= (res << 4);
	}

	return nibble_cnt;
}
