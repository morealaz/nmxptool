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
	    logit("et", "nmxp2ew: Error - created TRACE2_HEADER but TYPE_TRACEBUF2 is unknown\n");
	    return EW_FAILURE;
	} else {
	    pwaveLogo->type = typeWaveform2;
	}
    } else {
	pwaveLogo->type = typeWaveform;
    }

    if ( tport_putmsg( pregionOut, pwaveLogo, len, (char*)&tbuf ) != PUT_OK ) {
	logit("et", "nmxp2ew: Error sending message via transport.\n");
	return EW_FAILURE;
    }

    return EW_SUCCESS;
}				/* End of nmxptool_ew_pd2ewring() */

int nmxptool_nxm2ew(NMXP_DATA_PROCESS *pd) {
    int ret = 0;
    ret = nmxptool_ew_pd2ewring (pd, &regionOut, &waveLogo);
    return ret;
}



/***************************************************************************
 * configure():
 * Process configuration parameters.
 *
 ***************************************************************************/
void nmxptool_ew_configure (char ** argvec) {

    /* Initialize name of log-file & open it */
    logit_init (argvec[1], 0, 512, 1);

    /* Read module config file */
    if ( nmxptool_ew_proc_configfile (argvec[1]) == EW_FAILURE ) {
	fprintf (stderr, "%s: configure() failed \n", argvec[0]);
	exit (EW_FAILURE);
    }

    /* Read node configuration info */
    if ( GetLocalInst( &myInstId) != 0 ) {
	fprintf(stderr, "nmxp2ew: Error getting myInstId.\n" );
	exit (EW_FAILURE);
    }

    /* Lookup the ring key */
    if ((ringKey = GetKey (ringName) ) == -1) {
	fprintf (stderr,
		"nmxp2ew:  Invalid ring name <%s>; exitting!\n", ringName);
	exit (EW_FAILURE);
    }

    /* Look up message types of interest */
    if (GetType ("TYPE_HEARTBEAT", &typeHeartbeat) != 0) {
	fprintf (stderr, 
		"nmxp2ew: Invalid message type <TYPE_HEARTBEAT>; exitting!\n");
	exit (EW_FAILURE);
    }
    if (GetType ("TYPE_ERROR", &typeError) != 0) {
	fprintf (stderr, 
		"nmxp2ew: Invalid message type <TYPE_ERROR>; exitting!\n");
	exit (EW_FAILURE);
    }

    if (GetType ("TYPE_TRACEBUF", &typeWaveform) != 0) {
	fprintf (stderr, 
		"nmxp2ew: Invalid message type <TYPE_TRACEBUF>; exitting!\n");
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
int nmxptool_ew_proc_configfile (char * configfile) {
    char    		*com;
    char    		*str;
    int      		nfiles;
    int      		success;

    //m int    slport = 0;
    //m char  *slhost = 0;
    //m char   sladdr[100];
    //m char  *selectors = 0;
    //m char  *paramdir = 0;

    /* Some important initial values or defaults */
    ringName[0]   = '\0';
    myModName[0] = '\0';
    heartbeatInt = -1;
    logSwitch    = -1;

    /* Open the main configuration file */
    nfiles = k_open (configfile);
    if (nfiles == 0) {
	fprintf (stderr,
		"nmxp2ew: Error opening command file <%s>; exiting!\n", 
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
			    "nmxp2ew: Error opening command file <%s>; exiting!\n", 
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
			fprintf( stderr, "nmxp2ew: Error getting myModId.\n" );
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

	    else if ( k_its("Verbosity") ) {
		if ( (str = k_str ()) ) {
		    //m verbose = atoi (str);
		}
		if ( !str ) {
		    fprintf(stderr, "Verbosity is unspecified, quiting\n");
		    return EW_FAILURE;
		}
	    }

	    else if (k_its ("SLhost")) {
		if ( (str = k_str ()) ) {
		    if (strlen(str) >= MAXADDRLEN) {
			fprintf(stderr, "SLaddr too long; max is %d characters\n",
				MAXADDRLEN);
			return EW_FAILURE;
		    }
		    //m slhost = strdup(str);
		}
	    }

	    else if ( k_its ("SLport")) {
		//m slport = k_int();
		//m if (slport < 1) {
		    //m fprintf(stderr, "SLport is 0 or junk, quiting.\n");
		    //m return EW_FAILURE;
		//m }
	    }

	    else if (k_its ("StateFile")) {
		/* Build a state file name of the form 'slink<modid>.state' and
		   prepend the parameter directory */
		//m paramdir = getenv ( "EW_PARAMS" );
		//m statefile = (char *) malloc (strlen(paramdir) + 16);

#ifdef _INTEL
		//m sprintf (statefile, "%s\\slink%d.state", paramdir, myModId);
#else  /* *nix brand */
		//m sprintf (statefile, "%s/slink%d.state", paramdir, myModId);
#endif
	    }

	    else if ( k_its ("StateFileInt")) {
		if ( (str = k_str ()) ) {
		    //m stateint = atoi (str);
		}
		//m if ( !str || stateint < 0 ) {
		    //m fprintf(stderr, "StateFileInt is unspecified or negative, quiting\n");
		    //m return EW_FAILURE;
		//m }
	    }

	    else if ( k_its ("NetworkTimeout")) {
		if ( (str = k_str ()) ) {
		    //m slconn->netto = atoi (str);
		}
		//m if ( !str || slconn->netto < 0 ) {
		    //m fprintf(stderr, "NetworkTimeout is unspecified or negative, quiting\n");
		    //m return EW_FAILURE;
		//m }
	    }

	    else if ( k_its ("NetworkDelay")) {
		if ( (str = k_str ()) ) {
		    //m slconn->netdly = atoi (str);
		}
		//m if ( !str || slconn->netdly < 0 ) {
		    //m fprintf(stderr, "NetworkDelay is unspecified or negative, quiting\n");
		    //m return EW_FAILURE;
		//m }
	    }

	    else if ( k_its ("KeepAlive")) {
		if ( (str = k_str ()) ) {
		    //m slconn->keepalive = atoi (str);
		}
		//m if ( !str || slconn->keepalive < 0 ) {
		    //m fprintf(stderr, "KeepAlive is unspecified or negative, quiting\n");
		    //m return EW_FAILURE;
		//m }
	    }

	    else if (k_its ("ForceTraceBuf1")) {
		forcetracebuf = k_int();
	    }

	    else if ( k_its ("Selectors")) {
		if ( (str = k_str ()) ) {
		    if (strlen(str) >= 100) {
			fprintf(stderr, "Selectors too long; max is 100 characters\n");
			return EW_FAILURE;
		    }
		    //m selectors = strdup(str);
		}
	    }

	    else if (k_its ("Stream")) {
		if ( (str = k_str ()) ) {
		    char *net;
		    char *sta;
		    char netsta[20];
		    char streamselect[100];

		    streamselect[0] = '\0';

		    /* Collect the stream key (the NET_STA specifier) */
		    strncpy (netsta, str, sizeof(netsta));
		    net = netsta;
		    if ( (sta = strchr (netsta, '_')) == NULL ) {
			fprintf(stderr, "Could not parse stream key: %s\n", str);
			return EW_FAILURE;
		    } else {
			*sta++ = '\0';
		    }

		    /* Build a selector list from an optional 3rd value */
		    if ( (str = k_str ()) ) {
			strncpy (streamselect, str, sizeof(streamselect));
		    }
		    else
			k_err();  /* Clear the error if there was no selectors */

		    if ( streamselect[0] != '\0' ) {
			//m sl_addstream (slconn, net, sta, streamselect, -1, NULL);
		    } else {
			//m sl_addstream (slconn, net, sta, selectors, -1, NULL);
		    }
		}
	    }

	    /* Unknown command */ 
	    else {
		fprintf (stderr, "nmxp2ew: <%s> Unknown command in <%s>.\n", 
			com, configfile);
		continue;
	    }

	    /* See if there were any errors processing the command */
	    if (k_err ()) {
		fprintf (stderr, 
			"nmxp2ew: Bad command in <%s>; exiting!\n\t%s\n",
			configfile, k_com());
		return EW_FAILURE;
	    }

	} /** while k_rd() **/

	nfiles = k_close ();

    } /** while nfiles **/

    /* Configure uni-station mode if no streams were specified */
    //m if ( slconn->streams == NULL )
	//m sl_setuniparams (slconn, selectors, -1, 0);

    /* Check for required parameters */
    if ( myModName[0] == '\0' ) {
	fprintf (stderr, "nmxp2ew: No MyModId parameter found in %s\n",
		configfile);
	return EW_FAILURE;
    }
    if ( ringName[0] == '\0' ) {
	fprintf (stderr, "nmxp2ew: No OutRing parameter found in %s\n",
		configfile);
	return EW_FAILURE;
    }
    if ( heartbeatInt == -1 ) {
	fprintf (stderr, "nmxp2ew: No HeartBeatInterval parameter found in %s\n",
		configfile);
	return EW_FAILURE;
    }
    if ( logSwitch == -1 ) {
	fprintf (stderr, "nmxp2ew: No LogFile parameter found in %s\n",
		configfile);
	return EW_FAILURE;
    }
    //m if ( !slhost ) {
	//m fprintf (stderr, "nmxp2ew: No SLhost parameter found in %s\n",
		//m configfile);
	//m return EW_FAILURE;
    //m }
    //m if ( !slport ) {
	//m fprintf (stderr, "nmxp2ew: No SLport parameter found in %s\n",
		//m configfile);
	//m return EW_FAILURE;
    //m }

    /* Configure the SeedLink connection description thing */
    //m snprintf (sladdr, sizeof(sladdr), "%s:%d", slhost, slport);
    //m slconn->sladdr = strdup(sladdr);

    return EW_SUCCESS;
}				/* End of nmxptool_ew_proc_configfile() */

/***************************************************************************
 * nmxptoole_ew_report_status():
 * Send error and hearbeat messages to transport ring.
 *
 ***************************************************************************/
void nmxptoole_ew_report_status( MSG_LOGO * pLogo, short code, char * message ) {
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
	    logit( "et", "nmxp2ew: Failed to send a heartbeat message (%d).\n",
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
	    logit( "et", "nmxp2ew: Failed to send an error message (%d).\n",
		    code );
	}

    }
}				/* End of nmxptool_ew_report_status() */


/***************************************************************************
 * nmxptool_ew_logit_msg() and nmxptool_ew_logit_err():
 * 
 * Hooks for Earthworm logging facility.
 ***************************************************************************/
void nmxptool_ew_logit_msg (const char *msg) {
  logit ("t", (char *) msg);
}

void nmxptool_ew_logit_err (const char *msg) {
  logit ("et", (char *) msg);
}


#endif

