/*	hex2bin2hex_test.c

Test series for the hx2b hex-to-binary and b2hx binary-to-hex functions.
These functions can be used to parse and generate hexadecimal strings,
	such as hashes or configuration file values.

(c) 2017 Sirio Balmelli; https://b-ad.ch
*/

#include <binhex.h>
#include "ndebug.h"


/*	test_union_behavior()

Prove unions behave predictably on this platform.
returns 0 on success
*/
int test_union_behavior()
{
	int err_cnt = 0;

	union bits__ {
		uint64_t	u64;
		uint32_t	u32[2];
		uint16_t	u16[4];
		uint8_t		u8[8];
	};

	union bits__ thing = { .u64 = 0x0 };
	const uint8_t val = 42;

	/* test array indexing behavior */
	thing.u8[0] = val;
	NB_err_if(thing.u8[0] != thing.u16[0],
		"%"PRIx8" != %"PRIx16,
		thing.u8[0], thing.u16[0]);
	NB_err_if(thing.u8[0] != thing.u32[0],
		"%"PRIx8" != %"PRIx32,
		thing.u8[0], thing.u32[0]);
	NB_err_if(thing.u8[0] != thing.u64,
		"%"PRIx8" != %"PRIx64,
		thing.u8[0], thing.u64);

	/* test bitshift behavior */
	thing.u64 <<= 8;
	NB_err_if(thing.u8[1] != val, "");

	return err_cnt;
}


/*	test_hx2b()

Convert strings expressing hex values into unsigned integers.
returns 0 on success
*/
int test_hx2b()
{
	int err_cnt = 0;

	/* 8-bit parsing */
	char *check8[] = {
		"0xff", "xFF", "ff", "A", "h05", "h1", "07"
	};
	uint8_t ans8[] = {
		0xff, 0xff, 0xff, 0xA, 0x5, 0x1, 0x7
	};
	for (int i=0; i < sizeof(ans8); i++) {
		NB_err_if(hx2b_u8(check8[i]) != ans8[i],
			"@i=%d  :  %hhx != %s",
			i, hx2b_u8(check8[i]), check8[i]);
	}

	/* 16-bit parsing */
	char *check16[] = {
		"0xff", "a", "1234", "h555", "004"
	};
	uint16_t ans16[] = {
		0xff, 0xa, 0x1234, 0x555, 0x004
	};
	for (int i=0; i < sizeof(ans16) / sizeof(ans16[0]); i++) {
		NB_err_if(hx2b_u16(check16[i]) != ans16[i],
			"@i=%d  :  %hx != %s",
			i, hx2b_u16(check16[i]), check16[i]);
	}

	return err_cnt;
}


/*	test_b2hx()

Convert unsigned integers and bytefields into hex value strings;
	convert them back into binary and verify equivalence.
returns 0 on success
*/
int test_b2hx()
{
	int err_cnt = 0;

	const uint16_t n_16 = 0x1234;
	const char *hex_16 = "1234";
	const uint32_t n_32 = 0x12345678;
	const char *hex_32 = "12345678";
	const uint64_t n_64 = 0x1234567890123456;
	const char *hex_64 = "1234567890123456";

	char hex[33];

	NB_err_if(b2hx_u16(&n_16, 1, hex)
		!= 5, "");
	NB_err_if(strcmp(hex, hex_16)
		, "'%s' not expected '%s'", hex, hex_16);
	NB_err_if(hx2b_u16(hex_16)
		!= n_16,
		"%"PRIu16" != %"PRIu16, hx2b_u16(hex_16), n_16);

	NB_err_if(b2hx_u32(&n_32, 1, hex)
		!= 9, "");
	NB_err_if(strcmp(hex, hex_32)
		, "'%s' not expected '%s'", hex, hex_32);
	NB_err_if(hx2b_u32(hex_32)
		!= n_32,
		"%"PRIu32" != %"PRIu32, hx2b_u32(hex_32), n_32);

	NB_err_if(b2hx_u64(&n_64, 1, hex)
		!= 17, "");
	NB_err_if(strcmp(hex, hex_64)
		, "'%s' not expected '%s'", hex, hex_64);
	NB_err_if(hx2b_u64(hex_64)
		!= n_64,
		"%"PRIu64" != %"PRIu64, hx2b_u64(hex_64), n_64);

	const unsigned char byte_field[] = {
		0xa0, 0xb1, 0xc2, 0xd3, 0xe4, 0xf5, 0x06, 0x17,
		0x28, 0x39, 0x4a, 0x5b, 0x6c, 0x7d, 0x8e, 0x9f
	};
	NB_dump(byte_field, 16, "byte_field");
	unsigned char bytes_parse[16];

	NB_err_if(b2hx(byte_field, hex, 16)
		!= 33, "");
	NB_err_if(hx2b(hex, bytes_parse, sizeof(bytes_parse))
		!= 32, "");
	NB_err_if(memcmp(byte_field, bytes_parse, 16)
		, "");
	NB_wrn("hex (le):\n%s", hex);
	NB_dump(bytes_parse, 16, "bytes_parse (le)");

	/* repeat for big-endian */
	NB_err_if(b2hx_BE(byte_field, hex, 16)
		!= 33, "");
	NB_err_if(hx2b_BE(hex, bytes_parse, sizeof(bytes_parse))
		!= 32, "");
	NB_err_if(memcmp(byte_field, bytes_parse, 16)
		, "");
	NB_wrn("hex (BE):\n%s", hex);
	NB_dump(bytes_parse, 16, "bytes_parse (BE)");

	return err_cnt;
}


/*	main()
returns 0 on success
*/
int main()
{
	int err_cnt = 0;

	NB_die_if(test_union_behavior(),
		"Broken on this platform! Don't use!");
	err_cnt += test_hx2b();
	err_cnt += test_b2hx();

die:
	return err_cnt;
}
