/*! \file
 *
 * \brief Computing a 32 bit CRC.
 *
 * $Id: nmxp_crc32.c,v 1.6 2008-02-24 15:10:52 mtheo Exp $
 *
 *
 */

#include "nmxp_crc32.h"

static uint32_t crc32_tab[256];

void crc32_init_table () {
    unsigned int i, j;
    uint32_t h = 1;
    crc32_tab[0] = 0;
    for (i = 128; i; i >>= 1) {
	h = (h >> 1) ^ ((h & 1) ? POLYNOMIAL : 0);
	/* h is now crc_table[i]*/
	for (j = 0; j < 256; j += 2 * i) {
	    crc32_tab[i + j] = crc32_tab[j] ^ h;
	}
    }
}

uint32_t crc32(uint32_t crc32val, const char *s, uint32_t len)
{
	uint32_t i;

	crc32_init_table();

	crc32val ^= 0xffffffff;
	for (i = 0;  i < len;  i ++) {
		crc32val = crc32_tab[(crc32val ^ s[i]) & 0xff] ^ (crc32val >> 8);
	}

	return crc32val ^ 0xffffffff;
}

