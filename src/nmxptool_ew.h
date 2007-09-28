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

void nmxptool_ew_attach();
void nmxptool_ew_detach();

int nmxptool_ew_pd2ewring (NMXP_DATA_PROCESS *pd, SHM_INFO *pregionOut, MSG_LOGO *pwaveLogo);

int nmxptool_nxm2ew(NMXP_DATA_PROCESS *pd);

void nmxptool_ew_configure (char ** argvec);

int nmxptool_ew_proc_configfile (char * configfile);

void nmxptool_ew_report_status ( MSG_LOGO *pLogo, short code, char * message );

void nmxptool_ew_logit_msg ( const char *msg );
void nmxptool_ew_logit_err ( const char *msg );

#endif

