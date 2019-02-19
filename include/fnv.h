#ifndef fnv_h_
#define fnv_h_

/*	fnv.h		FNV1a hash algorithm
 *
 * The algorithm is not invented here (gasp!) and is public domain.
 * See <https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function>
 *
 * TODO: check efficiency against <http://burtleburtle.net/bob/c/lookup3.c>
 *
 *(c) 2016 Sirio Balmelli and Anthony Soenen; https://b-ad.ch
 */

#include <nonlibc.h>
#include <stddef.h> /* size_t */
#include <stdint.h>


NLC_PUBLIC uint64_t	fnv_hash64(uint64_t *hash, const void *data, size_t data_len);
NLC_PUBLIC uint32_t	fnv_hash32(uint32_t *hash, const void *data, size_t data_len);
NLC_PUBLIC uint16_t	fnv_hash16(uint16_t *hash, const void *data, size_t data_len);


#endif /* fnv_h_ */
