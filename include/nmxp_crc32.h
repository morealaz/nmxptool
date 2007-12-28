/*! \file
 *
 * \brief Computing a 32 bit CRC.
 *
 * $Id: nmxp_crc32.h,v 1.5 2007-12-28 10:36:07 mtheo Exp $
 *
 * author: tatu ylonen <ylo@cs.hut.fi>
 *
 * copyright (c) 1992 tatu ylonen, espoo, finland
 *                    all rights reserved
 *
 *                    created: tue feb 11 14:37:27 1992 ylo
 *
 *                    functions for computing 32-bit crc.
 *
 * The implementation here was originally done by Gary S. Brown.  I have
 *    borrowed the tables directly, and made some minor changes to the
 *       crc32-function (including changing the interface). //ylo
 *
 *
 * ====================================================================
 *
 *  COPYRIGHT (C) 1986 Gary S. Brown.  You may use this program, or
 *  code or tables extracted from it, as desired without restriction.
 *
 *  First, the polynomial itself and its table of feedback terms.  The
 *  polynomial is
 *  
 *  X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0
 *
 *  Note that we take it "backwards" and put the highest-order term in
 *  the lowest-order bit.  The X^32 term is "implied"; the LSB is the
 *  X^31 term, etc.  The X^0 term (usually shown as "+1") results in
 *  the MSB being 1.
 *
 *  Note that the usual hardware shift register implementation, which
 *  is what we're using (we're merely optimizing it by doing eight-bit
 *  chunks at a time) shifts bits into the lowest-order term.  In our
 *  implementation, that means shifting towards the right.  Why do we
 *  do it this way?  Because the calculated CRC must be transmitted in
 *  order from highest-order term to lowest-order term.  UARTs transmit
 *  characters in order from LSB to MSB.  By storing the CRC this way,
 *  we hand it to the UART in the order low-byte to high-byte; the UART
 *  sends each low-bit to hight-bit; and the result is transmission bit
 *  by bit from highest- to lowest-order term without requiring any bit
 *  shuffling on our part.  Reception works similarly.
 *
 *  The feedback terms table consists of 256, 32-bit entries.  Notes:
 *
 *      The table can be generated at runtime if desired; code to do so
 *      is shown later.  It might not be obvious, but the feedback
 *      terms simply represent the results of eight shift/xor opera-
 *      tions for all combinations of data and CRC register values.
 *
 *      The values must be right-shifted by eight bits by the "updcrc"
 *      logic; the shift must be unsigned (bring in zeroes).  On some
 *      hardware you could probably optimize the shift in assembler by
 *      using byte-swap instructions.
 *      polynomial $edb88320
 *
 * ====================================================================
 *
 */

#ifndef NMXP_CRC32_H
#define NMXP_CRC32_H 1

#include <stdint.h>

/*! \brief Computes a 32 bit crc of the data in the buffer,
 * and returns the crc.  the polynomial used is 0xedb88320. */
uint32_t crc32(uint32_t crc32val, const char *buf, uint32_t len);

#endif /* CRC32_H */


