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
#include "nmxptool_getoptlong.h"

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
#define MAXADDRLEN      80      /* Length of NaqsServer hostname/address  */

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

time_t timeNow;
time_t timeLastBeat = 0;

void nmxptool_ew_attach() {
    /* Attach to Output transport ring */
    tport_attach (&regionOut, ringKey);
    logit ("t", "%s version %s\n", PACKAGE_NAME, PACKAGE_VERSION);
}

void nmxptool_ew_detach() {
    tport_detach(&regionOut);
    logit("t","%s terminated\n", PACKAGE_NAME);
}


int nmxptool_ew_pd2ewring (NMXP_DATA_PROCESS *pd, SHM_INFO *pregionOut, MSG_LOGO *pwaveLogo) {
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

    if ( ! forcetracebuf ) {
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

    if ( tracebuf2 != 2 ) {
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
    if ( tracebuf2 == 2 ) {
	if ( typeWaveform2 == 0 ) {
	    logit("et", "nmxptool: Error - created TRACE2_HEADER but TYPE_TRACEBUF2 is unknown\n");
	    return EW_FAILURE;
	} else {
	    pwaveLogo->type = typeWaveform2;
	}
    } else {
	pwaveLogo->type = typeWaveform;
    }

    if ( tport_putmsg( pregionOut, pwaveLogo, len, (char*)&tbuf ) != PUT_OK ) {
	logit("et", "nmxptool: Error sending message via transport.\n");
	return EW_FAILURE;
    }

    return EW_SUCCESS;
}				/* End of nmxptool_ew_pd2ewring() */


int nmxptool_ew_nmx2ew(NMXP_DATA_PROCESS *pd) {
    int ret;
    ret = nmxptool_ew_pd2ewring (pd, &regionOut, &waveLogo);
    return ret;
}



/***************************************************************************
 * nmxptoo_ew_configure():
 * Process configuration parameters.
 *
 ***************************************************************************/
void nmxptool_ew_configure (char ** argvec, NMXPTOOL_PARAMS *params) {

    /* Initialize name of log-file & open it */
    logit_init (argvec[1], 0, 512, 1);

    /* Read module config file */
    if ( nmxptool_ew_proc_configfile (argvec[1], params) == EW_FAILURE ) {
	fprintf (stderr, "%s: configure() failed \n", argvec[0]);
	exit (EW_FAILURE);
    }

    params->stc=-1;

    /* Read node configuration info */
    if ( GetLocalInst( &myInstId) != 0 ) {
	fprintf(stderr, "%s: Error getting myInstId.\n", PACKAGE_NAME );
	exit (EW_FAILURE);
    }

    /* Lookup the ring key */
    if ((ringKey = GetKey (ringName) ) == -1) {
	fprintf (stderr,
		"%s:  Invalid ring name <%s>; exitting!\n", PACKAGE_NAME, ringName);
	exit (EW_FAILURE);
    }

    /* Look up message types of interest */
    if (GetType ("TYPE_HEARTBEAT", &typeHeartbeat) != 0) {
	fprintf (stderr, 
		"%s: Invalid message type <TYPE_HEARTBEAT>; exitting!\n", PACKAGE_NAME);
	exit (EW_FAILURE);
    }
    if (GetType ("TYPE_ERROR", &typeError) != 0) {
	fprintf (stderr, 
		"%s: Invalid message type <TYPE_ERROR>; exitting!\n", PACKAGE_NAME);
	exit (EW_FAILURE);
    }

    if (GetType ("TYPE_TRACEBUF", &typeWaveform) != 0) {
	fprintf (stderr, 
		"%s: Invalid message type <TYPE_TRACEBUF>; exitting!\n", PACKAGE_NAME);
	exit (EW_FAILURE);
    }

    /* No GetType error checking as this type will not exist in all versions */
    GetType ("TYPE_TRACEBUF2", &typeWaveform2);

    /* Set up logos for outgoing messages */
    hrtLogo.instid = myInstId;
    hrtLogo.mod    = myModId;
    hrtLogo.type   = typeHeartbeat;

    errLogo.instid = myInstId;
    errLogo.mod    = myModId;
    errLogo.type   = typeError;

    waveLogo.instid = myInstId;
    waveLogo.mod    = myModId;
    waveLogo.type   = 0;  /* This gets set to the appropriate type later */

    /* Get my process ID so I can let statmgr restart me */
    myPid = getpid();

    logit ("et" , "%s(%s): Read command file <%s>\n",
	    argvec[0], myModName, argvec[1]);

    /* Initialize libslink logging facility */
    //m sl_loginit (verbose,
    //m 	      &logit_msg, "",
    //m 	      &logit_err, "error: ");

    /* Attempt to recover sequence numbers from state file */
    //m if ( statefile ) {
    //m if (sl_recoverstate (slconn, statefile) < 0) {
    //m sl_log (1, 0, "state recovery failed\n");
    //m }
    //m }

    /* Reinitialize the logging level */
    logit_init (argvec[1], 0, 512, logSwitch);

}				/* End of nmxptool_ew_configure() */


/***************************************************************************
 * nmxptool_ew_proc_configfile():
 * Process the module configuration parameters.
 *
 ***************************************************************************/
int nmxptool_ew_proc_configfile (char * configfile, NMXPTOOL_PARAMS *params) {
    char    		*com;
    char    		*str;
    int      		nfiles;
    int      		success;

    /* Some important initial values or defaults */
    ringName[0]   = '\0';
    myModName[0] = '\0';
    heartbeatInt = -1;
    logSwitch    = -1;

    /* Open the main configuration file */
    nfiles = k_open (configfile);
    if (nfiles == 0) {
	fprintf (stderr,
		"%s: Error opening command file <%s>; exiting!\n", PACKAGE_NAME,
		configfile);
	return EW_FAILURE;
    }

    /* Process all command files */
    while (nfiles > 0) {   /* While there are command files open */
	while (k_rd ()) {       /* Read next line from active file  */
	    com = k_str ();         /* Get the first token from line */

	    /* Ignore blank lines & comments */
	    if (!com)
		continue;
	    if (com[0] == '#')
		continue;

	    /* Open a nested configuration file */
	    if (com[0] == '@') {
		success = nfiles + 1;
		nfiles  = k_open (&com[1]);
		if (nfiles != success) {
		    fprintf (stderr, 
			    "%s: Error opening command file <%s>; exiting!\n", PACKAGE_NAME,
			    &com[1]);
		    return EW_FAILURE;
		}
		continue;
	    }

	    /* Process anything else as a command */
	    if (k_its ("MyModuleId")) {
		if ( (str = k_str ()) ) {
		    if (strlen(str) >= MAXMODNAMELEN) {
			fprintf(stderr, "MyModId too long; max is %d\n", MAXMODNAMELEN -1);
			return EW_FAILURE;
		    }

		    strcpy (myModName, str);

		    /* Lookup module ID */
		    if ( GetModId( myModName, &myModId) != 0 ) {
			fprintf( stderr, "%s: Error getting myModId.\n", PACKAGE_NAME );
			exit (EW_FAILURE);
		    }
		}
	    }

	    else if (k_its ("RingName")) {
		if ( (str = k_str ()) ) {
		    if (strlen(str) >= MAXRINGNAMELEN) {
			fprintf(stderr, "OutRing name too long; max is %d\n", 
				MAXRINGNAMELEN - 1);
			return EW_FAILURE;
		    }

		    strcpy (ringName, str);
		}
	    }

	    else if (k_its ("HeartBeatInterval")) {
		heartbeatInt = k_long ();
	    }

	    else if (k_its ("LogFile")) {
		logSwitch = k_int();
	    }

	    else if (k_its ("Verbosity")) {
		verbose = k_int();
		if(verbose != 0) {
		    params->flag_verbose = 1;
		} else {
		    params->flag_verbose = 0;
		}
	    }

	    else if (k_its ("NmxpHost")) {
		if ( (str = k_str ()) ) {
		    if (strlen(str) >= MAXADDRLEN) {
			fprintf(stderr, "nmxphost too long; max is %d characters\n",
				MAXADDRLEN);
			return EW_FAILURE;
		    }
		    params->hostname = strdup(str);
		}
	    }

	    else if ( k_its ("NmxpPortPDS")) {
		params->portnumberpds = k_int();
	    }

	    else if ( k_its ("NmxpPortDAP")) {
		params->portnumberdap = k_int();
	    }

	    else if (k_its ("ForceTraceBuf1")) {
		forcetracebuf = k_int();
	    }

	    else if (k_its ("TimeoutRecv")) {
		params->timeoutrecv = k_int();
	    }

	    else if (k_its ("MaxTolerableLatency")) {
		params->max_tolerable_latency = k_int();
	    }


	    else if (k_its ("DefaultNetworkCode")) {
		if ( (str = k_str ()) ) {
		    if(params->network) {
			fprintf(stderr, "DefaultNetworkCode has been replicated!\n");
			return EW_FAILURE;
		    } else {
			params->network = strdup(str);
		    }
		}
	    }

	    else if (k_its ("Channel")) {
		if ( (str = k_str ()) ) {
		    if(!params->channels) {
#define MAXSIZECHANNELSTRING 8000
			params->channels = (char *) malloc (MAXSIZECHANNELSTRING);
			strncpy(params->channels, str, MAXSIZECHANNELSTRING);
		    } else {
			strncat(params->channels, ",", MAXSIZECHANNELSTRING);
			strncat(params->channels, str, MAXSIZECHANNELSTRING);
		    }
		}
	    }


	    /* Unknown command */ 
	    else {
		fprintf (stderr, "%s: <%s> Unknown command in <%s>.\n", PACKAGE_NAME,
			com, configfile);
		continue;
	    }

	    /* See if there were any errors processing the command */
	    if (k_err ()) {
		fprintf (stderr, 
			"%s: Bad command in <%s>; exiting!\n\t%s\n", PACKAGE_NAME,
			configfile, k_com());
		return EW_FAILURE;
	    }

	} /** while k_rd() **/

	nfiles = k_close ();

    } /** while nfiles **/

    /* Check for required parameters */
    if ( myModName[0] == '\0' ) {
	fprintf (stderr, "%s: No MyModId parameter found in %s\n", PACKAGE_NAME,
		configfile);
	return EW_FAILURE;
    }
    if ( ringName[0] == '\0' ) {
	fprintf (stderr, "%s: No OutRing parameter found in %s\n", PACKAGE_NAME,
		configfile);
	return EW_FAILURE;
    }
    if ( heartbeatInt == -1 ) {
	fprintf (stderr, "%s: No HeartBeatInterval parameter found in %s\n", PACKAGE_NAME,
		configfile);
	return EW_FAILURE;
    }
    if ( logSwitch == -1 ) {
	fprintf (stderr, "%s: No LogFile parameter found in %s\n", PACKAGE_NAME,
		configfile);
	return EW_FAILURE;
    }

    return EW_SUCCESS;
}				/* End of nmxptool_ew_proc_configfile() */

/***************************************************************************
 * nmxptoole_ew_report_status():
 * Send error and hearbeat messages to transport ring.
 *
 ***************************************************************************/
void nmxptool_ew_report_status( MSG_LOGO * pLogo, short code, char * message ) {
    char          outMsg[MAXMESSAGELEN];  /* The outgoing message.        */
    time_t        msgTime;        /* Time of the message.                 */

    /*  Get the time of the message                                       */
    time( &msgTime );

    /* Build & process the message based on the type */
    if ( pLogo->type == typeHeartbeat ) {
	sprintf( outMsg, "%ld %ld\n\0", (long) msgTime, (long) myPid );

	/* Write the message to the output region */
	if ( tport_putmsg( &regionOut, &hrtLogo, (long) strlen( outMsg ),
		    outMsg ) != PUT_OK ) {
	    /* Log an error message */
	    logit( "et", "nmxptool: Failed to send a heartbeat message (%d).\n",
		    code );
	}
    } else {
	if ( message ) {
	    sprintf( outMsg, "%ld %hd %s\n\0", (long) msgTime, code, message );
	    logit("t","Error:%d (%s)\n", code, message );
	} else {
	    sprintf( outMsg, "%ld %hd\n\0", (long) msgTime, code );
	    logit("t","Error:%d (No description)\n", code );
	}

	/* Write the message to the output region  */
	if ( tport_putmsg( &regionOut, &errLogo, (long) strlen( outMsg ),
		    outMsg ) != PUT_OK ) {
	    /*     Log an error message                                    */
	    logit( "et", "nmxptool: Failed to send an error message (%d).\n",
		    code );
	}

    }
}				/* End of nmxptool_ew_report_status() */


int nmxptool_ew_check_flag_terminate() {
    /* Check if we are being asked to terminate */
    return (tport_getflag (&regionOut) == TERMINATE || tport_getflag (&regionOut) == myPid );
}

void nmxptool_ew_send_heartbeat_if_needed() {
    /* Check if we need to send heartbeat message */
    if ( time( &timeNow ) - timeLastBeat >= heartbeatInt )
    {
	timeLastBeat = timeNow;
	nmxptool_ew_report_status ( &hrtLogo, 0, "" ); 
    }
}

/***************************************************************************
 * nmxptool_ew_logit_msg() and nmxptool_ew_logit_err():
 * 
 * Hooks for Earthworm logging facility.
 ***************************************************************************/
int nmxptool_ew_logit_msg (char *msg) {
  logit ("t",  msg);
  return 0;
}

int nmxptool_ew_logit_err (char *msg) {
  logit ("et",  msg);
  return 0;
}


#endif

