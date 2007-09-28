/*! \file
 *
 * \brief Earthworm support for Nanometrics Protocol Tool
 *
 * Author:
 * 	Matteo Quintiliani
 * 	Istituto Nazionale di Geofisica e Vulcanologia - Italy
 *	quintiliani@ingv.it
 *
 * $Id $
 *
 */

#ifndef NMXPTOOL_EW_H
#define NMXPTOOL_EW_H 1

#include "nmxp.h"

/* Earthworm includes */
#include <earthworm.h>
#include <kom.h>
#include <transport.h>
#include <trace_buf.h>

extern SHM_INFO      regionOut;         /* Shared memory region                 */

int nmxptool_nxm2ew(NMXP_DATA_PROCESS *pd);

#endif

