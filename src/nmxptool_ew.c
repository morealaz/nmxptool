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

void nmxptool_ew_attach() {
    /* Attach to Output transport ring */
    tport_attach (&regionOut, ringKey);
    logit ("t", "nmxp2ew version %s\n", PACKAGE_VERSION);
}

void nmxptool_ew_detach() {
    tport_detach(&regionOut);
    logit("t","%s terminated\n", PACKAGE_NAME);
}

int nmx2ewring (NMXP_DATA_PROCESS *pd, SHM_INFO *pregionOut, MSG_LOGO *pwaveLogo)
{
  //m static MSrecord * msr = NULL;
  TracePacket tbuf;
  int tracebuf2 = 0;   /* TRACEBUF2 => 0: none, 1: available, 2: populated */
  int len;
  int32_t *samples;
  int i;
  
  //m msr_parse (slconn->log, msrecord, &msr, 1, 1);
  
  /* TRACE_HEADER and TRACE2_HEADER are the same size */
  memset (&tbuf, 0, sizeof(TRACE_HEADER));
  
  /* Log packet details if the verbosity is high */
  //m if ( verbose > 1 )
  //m  msr_print (slconn->log, msr, verbose - 1);
  
  /* Create a TRACEBUF2 message if supported */
#ifdef TRACE2_STA_LEN
  tracebuf2 = 1;
  
  if ( ! forcetracebuf )
    {
      tbuf.trh2.pinno = 0;
      //m tbuf.trh2.nsamp = msr->numsamples;
      tbuf.trh2.nsamp = pd->nSamp;
      
      //m tbuf.trh2.starttime = msr_depochstime(msr);
      tbuf.trh2.starttime = pd->time;
      //m msr_dsamprate (msr, &tbuf.trh2.samprate);
      tbuf.trh2.samprate = pd->sampRate;
      tbuf.trh2.endtime = (tbuf.trh2.starttime +
			   ((tbuf.trh2.nsamp - 1) / tbuf.trh2.samprate));
      
      //m strncpclean(tbuf.trh2.net, msr->fsdh.network, 2);
      //m strncpclean(tbuf.trh2.sta, msr->fsdh.station, 5);
      //m strncpclean(tbuf.trh2.chan, msr->fsdh.channel, 3);
      strcpy(tbuf.trh2.net, pd->network);
      strcpy(tbuf.trh2.sta, pd->station);
      strcpy(tbuf.trh2.chan, pd->channel);
      
      //m if ( strncmp(msr->fsdh.location, "  ", 2) == 0 )
      //m strncpclean(tbuf.trh2.loc, LOC_NULL_STRING, 2);
      //m else
      //m strncpclean(tbuf.trh2.loc, msr->fsdh.location, 2);
      strncpy(tbuf.trh2.loc, LOC_NULL_STRING, 2);
      
      tbuf.trh2.version[0] = TRACE2_VERSION0;
      tbuf.trh2.version[1] = TRACE2_VERSION1;
      
      /* The decoding always produces 32-bit integers in host byte order */
#ifdef _INTEL
      strcpy(tbuf.trh2.datatype, "i4");
#endif
#ifdef _SPARC
      strcpy(tbuf.trh2.datatype, "s4");
#endif
      
      //m tbuf.trh2.quality[0] = msr->fsdh.dq_flags;
      tbuf.trh2.quality[0] = 100; /* TODO */
      tbuf.trh2.quality[1] = 0;
      
      tracebuf2 = 2;
    }
#endif
  
  if ( tracebuf2 != 2 )
    {
      /* Create a TRACEBUF message otherwise */
      tbuf.trh.pinno = 0;
      //m tbuf.trh.nsamp = msr->numsamples;
      tbuf.trh.nsamp = pd->nSamp;
      
      //m tbuf.trh.starttime = msr_depochstime(msr);
      tbuf.trh.starttime = pd->time;
      //m msr_dsamprate (msr, &tbuf.trh.samprate);
      tbuf.trh.samprate = pd->sampRate;
      tbuf.trh.endtime = (tbuf.trh.starttime +
			  ((tbuf.trh.nsamp - 1) / tbuf.trh.samprate));
      
      //m strncpclean(tbuf.trh.net, msr->fsdh.network, 2);
      //m strncpclean(tbuf.trh.sta, msr->fsdh.station, 5);
      //m strncpclean(tbuf.trh.chan, msr->fsdh.channel, 3);
      strcpy(tbuf.trh.net, pd->network);
      strcpy(tbuf.trh.sta, pd->station);
      strcpy(tbuf.trh.chan, pd->channel);
      
      /* The decoding always produces 32-bit integers in host byte order */
#ifdef _INTEL
      strcpy(tbuf.trh.datatype, "i4");
#endif
#ifdef _SPARC
      strcpy(tbuf.trh.datatype, "s4");
#endif
      
      //m tbuf.trh.quality[0] = msr->fsdh.dq_flags;
      tbuf.trh.quality[0] = 100; /* TODO */
      tbuf.trh.quality[1] = 0;
    }


  /* SeedLink always uses 512-byte Mini-SEED records, all of the samples
     should always fit into a single TracePacket if MAX_TRACEBUF_SIZ
     remains defined in Trace_buf.h as 4096 or greater */
  
  samples = (int32_t *) ((char *)&tbuf + sizeof(TRACE_HEADER));
  //m memcpy (samples, msr->datasamples, msr->numsamples * sizeof(int32_t));
  for(i=0; i < pd->nSamp; i++) {
      samples[i] = pd->pDataPtr[i];
  }
  
  //m len = (msr->numsamples * sizeof(int32_t)) + sizeof(TRACE_HEADER);
  len = (pd->nSamp * sizeof(int32_t)) + sizeof(TRACE_HEADER);
  
  /* Set the approriate TRACE type in the logo */
  if ( tracebuf2 == 2 )
    {
      if ( typeWaveform2 == 0 )
	{
	  logit("et", "nmxp2ew: Error - created TRACE2_HEADER but TYPE_TRACEBUF2 is unknown\n");
	  return EW_FAILURE;
	}
      else
	{
	  pwaveLogo->type = typeWaveform2;
	}
    }
  else
    {
      pwaveLogo->type = typeWaveform;
    }
  
  if ( tport_putmsg( pregionOut, pwaveLogo, len, (char*)&tbuf )
       != PUT_OK )
    {
      logit("et", "nmxp2ew: Error sending message via transport.\n");
      return EW_FAILURE;
    }
  
  return EW_SUCCESS;
}				/* End of nmx2ewring() */

int nmxptool_nxm2ew(NMXP_DATA_PROCESS *pd) {
    int ret = 0;
    ret = nmx2ewring (pd, &regionOut, &waveLogo);
    return ret;
}

#endif

