/*! \file
 *
 * \brief Computing a 32 bit CRC.
 *
 * $Id: nmxp_crc32.h,v 1.6 2007-12-28 11:21:51 mtheo Exp $
 *
 */

#ifndef NMXP_CRC32_H
#define NMXP_CRC32_H 1

#include <stdint.h>

#define POLYNOMIAL (uint32_t)0xedb88320

/*! \brief Computes a 32 bit crc of the data in the buffer,
 * and returns the crc.  the polynomial used is 0xedb88320. */
uint32_t crc32(uint32_t crc32val, const char *buf, uint32_t len);

#endif /* CRC32_H */


