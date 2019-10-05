#ifndef nlc_endian_h_
#define nlc_endian_h_

/* Explicitly provide endianness macros, since 'endian.h' isn't everywhere
 * (I'm looking at you, Darwin BSD!).
 * Bonus: the implementation is GCC builtins :)
 * // #include <endian.h>
 * NOTE: of course, it's not that simple. Certain systems define _some_ but not
 * _all_ of these macros, so we need to explicitly check for and undef every one.
 */
#ifdef le16toh
#undef le16toh
#endif
#ifdef le32toh
#undef le32toh
#endif
#ifdef le64toh
#undef le64toh
#endif
#ifdef be16toh
#undef be16toh
#endif
#ifdef be32toh
#undef be32toh
#endif
#ifdef be64toh
#undef be64toh
#endif
#ifdef h16tole
#undef h16tole
#endif
#ifdef h32tole
#undef h32tole
#endif
#ifdef h64tole
#undef h64tole
#endif
#ifdef h16tobe
#undef h16tobe
#endif
#ifdef h32tobe
#undef h32tobe
#endif
#ifdef h64tobe
#undef h64tobe
#endif

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define le16toh(x) (x)
#define le32toh(x) (x)
#define le64toh(x) (x)
#define be16toh(x) __builtin_bswap16(x)
#define be32toh(x) __builtin_bswap32(x)
#define be64toh(x) __builtin_bswap64(x)
#define h16tole(x) (x)
#define h32tole(x) (x)
#define h64tole(x) (x)
#define h16tobe(x) __builtin_bswap16(x)
#define h32tobe(x) __builtin_bswap32(x)
#define h64tobe(x) __builtin_bswap64(x)

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define le16toh(x) __builtin_bswap16(x)
#define le32toh(x) __builtin_bswap32(x)
#define le64toh(x) __builtin_bswap64(x)
#define be16toh(x) (x)
#define be32toh(x) (x)
#define be64toh(x) (x)
#define h16tole(x) __builtin_bswap16(x)
#define h32tole(x) __builtin_bswap32(x)
#define h64tole(x) __builtin_bswap64(x)
#define h16tobe(x) (x)
#define h32tobe(x) (x)
#define h64tobe(x) (x)

#else
#error "Architecture endinanness not supported. Send espresso to maintainer"
#endif

#endif /* nlc_endian_h_ */
