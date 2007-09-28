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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "config.h"
#include "nmxp.h"

#ifdef HAVE_EARTHWORMOBJS
/* Earthworm includes */
#include <earthworm.h>
#include <kom.h>
#include <transport.h>
#include <trace_buf.h>

#include "nmxptool_ew.h"


#define MAXMESSAGELEN   160     /* Maximum length of a status or error  */
                                /*   message.                           */
#define MAXRINGNAMELEN  28      /* Maximum length of a ring name.       */
                                /* Should be defined by kom.h           */
#define MAXMODNAMELEN   30      /* Maximum length of a module name      */
                                /* Should be defined by kom.h           */
#define MAXADDRLEN      80      /* Length of SeedLink hostname/address  */

SHM_INFO      regionOut;         /* Shared memory region                 */
pid_t         myPid;             /* Process ID                           */

char  ringName[MAXRINGNAMELEN];  /* Name of destination ring for data    */
long  ringKey;                   /* Key to output shared memory region   */
char  myModName[MAXMODNAMELEN];  /* Name of module instance              */
int   forcetracebuf = 0;         /* Switch to force TRACEBUF             */

unsigned char myModId;           /* ID of this module                    */
unsigned char myInstId;          /* Installation running this module     */
unsigned char typeError;         /* Error message type                   */
unsigned char typeHeartbeat;     /* Heartbeat message type               */
unsigned char typeWaveform;      /* Waveform message type TRACEBUF       */
unsigned char typeWaveform2 = 0; /* Waveform message type TRACEBUF2      */

MSG_LOGO      hrtLogo;           /* Heartbeat message logo               */
MSG_LOGO      waveLogo;          /* Waveform message logo                */
MSG_LOGO      errLogo;           /* Error message logo                   */

int           heartbeatInt;      /* Heartbeat interval (seconds)         */
int           logSwitch;         /* 1 -> write log, 0 -> no log          */
                          /* 2 -> write module log but not stderr/stdout */

int nmxptool_nxm2ew(NMXP_DATA_PROCESS *pd) {
    int ret = 0;

    return ret;
}

#endif

