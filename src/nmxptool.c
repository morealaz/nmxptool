/*! \file
 *
 * \brief Nanometrics Protocol Tool
 *
 * Author:
 * 	Matteo Quintiliani
 * 	Istituto Nazionale di Geofisica e Vulcanologia - Italy
 *	quintiliani@ingv.it
 *
 * $Id: nmxptool.c,v 1.64 2007-09-20 16:32:44 mtheo Exp $
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nmxp.h>

#ifndef WIN32
#include <signal.h>
#endif

#include "config.h"
#include "nmxptool_getoptlong.h"

#ifdef HAVE_LIBMSEED
#include <libmseed.h>
#endif

#ifdef HAVE___SRC_SEEDLINK_PLUGIN_H
#include "seedlink_plugin.h"
#endif

#define CURRENT_NETWORK (params.network)? params.network : DEFAULT_NETWORK
#define GAP_TOLLERANCE 0.001

typedef struct {
    int significant;
    double last_time;
    int32_t x_1;
    NMXP_RAW_STREAM_DATA raw_stream_buffer;
} NMXPTOOL_CHAN_SEQ;

static void clientShutdown(int sig);
static void clientDummyHandler(int sig);

int nmxptool_write_miniseed(NMXP_DATA_PROCESS *pd);
int nmxptool_send_raw_depoch(NMXP_DATA_PROCESS *pd);
int nmxptool_print_seq_no(NMXP_DATA_PROCESS *pd);

int nmxptool_check_and_log_gap(double time1, double time2, const double gap_tollerance, const char *station, const char *channel);
void nmxptool_str_time_to_filename(char *str_time);


/* Global variable for main program and handling terminitation program */
NMXPTOOL_PARAMS params;
int naqssock = 0;
FILE *outfile = NULL;
NMXP_CHAN_LIST *channelList = NULL;
NMXP_CHAN_LIST *channelList_subset = NULL;
NMXPTOOL_CHAN_SEQ *channelListSeq = NULL;

#ifdef HAVE_LIBMSEED
/* Mini-SEED variables */
NMXP_DATA_SEED data_seed;
MSRecord *msr_list_chan[MAX_N_CHAN];
#endif


int main (int argc, char **argv) {
    int32_t connection_time;
    int request_SOCKET_OK;
    int i_chan, cur_chan;
    int exitpdscondition;
    int exitdapcondition;

    int span_interval = 10;
    int time_to_sleep = 0;

    char str_start_time[200];
    char str_end_time[200];

    NMXP_MSG_SERVER type;
    void *buffer;
    int32_t length;
    int ret;

    char filename[500];
    char station_code[20], channel_code[20];

    NMXP_DATA_PROCESS *pd;

#ifdef HAVE_LIBMSEED
    /* Init mini-SEED variables */
    nmxp_data_seed_init(&data_seed);
#endif

#ifndef WIN32
    /* Signal handling, use POSIX calls with standardized semantics */
    struct sigaction sa;

    sa.sa_handler = clientDummyHandler;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);

    sa.sa_handler = clientShutdown;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL); 
    sigaction(SIGTERM, &sa, NULL);

    sa.sa_handler = SIG_IGN;
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGPIPE, &sa, NULL); 
#endif


    /* Default is normal output */
    nmxp_log(-1, 0);

    /* Initialize params from argument values */
    if(nmxptool_getopt_long(argc, argv, &params) != 0) {
	return 1;
    }

    /* Check consistency of params */
    if(nmxptool_check_params(&params) != 0) {
	return 1;
    }

    if(params.flag_verbose) {
	nmxp_log(-1, 2);
    }

    /* List available channels on server */
    if(params.flag_listchannels) {

	// TOREMOVE
	// channelList = nmxp_getAvailableChannelList(params.hostname, params.portnumberpds, NMXP_DATA_TIMESERIES);
	// TOREMOVE
	// nmxp_chan_print_channelList(channelList);

	nmxp_getMetaChannelList(params.hostname, params.portnumberdap, NMXP_DATA_TIMESERIES, params.flag_request_channelinfo);

	return 1;
    }

    /* Get list of available channels and get a subset list of params.channels */
    channelList = nmxp_getAvailableChannelList(params.hostname, params.portnumberpds, NMXP_DATA_TIMESERIES);
    channelList_subset = nmxp_chan_subset(channelList, NMXP_DATA_TIMESERIES, params.channels);

    /* Check if some channel already exists */
    if(channelList_subset->number <= 0) {
	nmxp_log(1, 0, "Channels not found!\n");
	return 1;
    } else {
	nmxp_chan_print_channelList(channelList_subset);

	nmxp_log(0, 1, "Init channelListSeq.\n");

	/* init channelListSeq */
	channelListSeq = (NMXPTOOL_CHAN_SEQ *) malloc(sizeof(NMXPTOOL_CHAN_SEQ) * channelList_subset->number);
	for(i_chan = 0; i_chan < channelList_subset->number; i_chan++) {
	    channelListSeq[i_chan].significant = 0;
	    channelListSeq[i_chan].last_time = 0.0;
	    channelListSeq[i_chan].x_1 = 0;
	    nmxp_raw_stream_init(&(channelListSeq[i_chan].raw_stream_buffer), params.max_tollerable_latency);
	}

#ifdef HAVE_LIBMSEED
	nmxp_log(0, 1, "Init mini-SEED record list.\n");

	/* Init mini-SEED record list */
	for(i_chan = 0; i_chan < channelList_subset->number; i_chan++) {

	    nmxp_log(0, 1, "Init mini-SEED record for %s\n", channelList_subset->channel[i_chan].name);

	    msr_list_chan[i_chan] = msr_init(NULL);

	    /* Separate station_code and channel_code */
	    if(nmxp_chan_cpy_sta_chan(channelList_subset->channel[i_chan].name, station_code, channel_code)) {

		nmxp_log(0, 1, "%s.%s.%s\n", CURRENT_NETWORK, station_code, channel_code);

		strcpy(msr_list_chan[i_chan]->network, CURRENT_NETWORK);
		strcpy(msr_list_chan[i_chan]->station, station_code);
		strcpy(msr_list_chan[i_chan]->channel, channel_code);

		msr_list_chan[i_chan]->reclen = 512;         /* byte record length */
		msr_list_chan[i_chan]->encoding = DE_STEIM1;  /* Steim 1 compression */

	    } else {
		nmxp_log(1, 0, "Channels %s error in format!\n");
		return 1;
	    }

	}
#endif

    }

    /* Free the complete channel list */
    if(channelList) {
	free(channelList);
	channelList = NULL;
    }

    nmxp_log(0, 1, "Starting comunication.\n");

    /* TODO condition starting DAP or PDS */
    if( (params.start_time != 0   &&   params.end_time != 0)
	    || params.delay > 0
	    ) {

	if(params.delay > 0) {
	    params.start_time = ((time(NULL) - params.delay - span_interval) / 10) * 10;
	    params.end_time = params.start_time + span_interval;
	}


	/* ************************************************************** */
	/* Start subscription protocol "DATA ACCESS PROTOCOL" version 1.0 */
	/* ************************************************************** */

	/* DAP Step 1: Open a socket */
	if( (naqssock = nmxp_openSocket(params.hostname, params.portnumberdap)) == NMXP_SOCKET_ERROR) {
	    nmxp_log(1, 0, "Error opening socket!\n");
	    return 1;
	}

	/* DAP Step 2: Read connection time */
	if(nmxp_readConnectionTime(naqssock, &connection_time) != NMXP_SOCKET_OK) {
	    nmxp_log(1, 0, "Error reading connection time from server!\n");
	    return 1;
	}

	/* DAP Step 3: Send a ConnectRequest */
	if(nmxp_sendConnectRequest(naqssock, params.datas_username, params.datas_password, connection_time) != NMXP_SOCKET_OK) {
	    nmxp_log(1, 0, "Error sending connect request!\n");
	    return 1;
	}

	/* DAP Step 4: Wait for a Ready message */
	if(nmxp_waitReady(naqssock) != NMXP_SOCKET_OK) {
	    nmxp_log(1, 0, "Error waiting Ready message!\n");
	    return 1;
	}

	exitdapcondition = 1;

	while(exitdapcondition) {

	nmxp_log(0, 1, "start_time = %d - end_time = %d\n", params.start_time, params.end_time);

	/* Start loop for sending requests */
	i_chan=0;
	request_SOCKET_OK = NMXP_SOCKET_OK;

	while(request_SOCKET_OK == NMXP_SOCKET_OK  &&  i_chan < channelList_subset->number) {

	    /* DAP Step 5: Send Data Request */
	    request_SOCKET_OK = nmxp_sendDataRequest(naqssock, channelList_subset->channel[i_chan].key, (double) params.start_time, (double) params.end_time);

	    if(request_SOCKET_OK == NMXP_SOCKET_OK) {

		nmxp_data_to_str(str_start_time, params.start_time);
		nmxp_data_to_str(str_end_time, params.end_time);
		nmxptool_str_time_to_filename(str_start_time);
		nmxptool_str_time_to_filename(str_end_time);

		if(params.flag_writefile) {
		    /* Open output file */
		    sprintf(filename, "%s.%s_%s_%s.nmx",
			    CURRENT_NETWORK,
			    channelList_subset->channel[i_chan].name,
			    str_start_time,
			    str_end_time);

		    outfile = fopen(filename, "w");
		    if(!outfile) {
			nmxp_log(1, 0, "Can not to open file %s!", filename);
		    }
		}

#ifdef HAVE_LIBMSEED
		if(params.flag_writeseed) {
		    /* Open output Mini-SEED file */
		    sprintf(data_seed.filename_mseed, "%s.%s_%s_%s.miniseed",
			    CURRENT_NETWORK,
			    channelList_subset->channel[i_chan].name,
			    str_start_time,
			    str_end_time);

		    data_seed.outfile_mseed = fopen(data_seed.filename_mseed, "w");
		    if(!data_seed.outfile_mseed) {
			nmxp_log(1, 0, "Can not to open file %s!", data_seed.filename_mseed);
		    }
		}
#endif

		if(params.flag_writefile  &&  outfile) {
		    /* Compute SNCL line */

		    /* Separate station_code_old_way and channel_code_old_way */
		    if(nmxp_chan_cpy_sta_chan(channelList_subset->channel[i_chan].name, station_code, channel_code)) {
			/* Write SNCL line */
			fprintf(outfile, "%s.%s.%s.%s\n",
				station_code,
				CURRENT_NETWORK,
				channel_code,
				(params.location)? params.location : "");
		    }

		}

		/* DAP Step 6: Receive Data until receiving a Ready message */
		ret = nmxp_receiveMessage(naqssock, &type, &buffer, &length);
		nmxp_log(0, 1, "ret = %d, type = %d\n", ret, type);

		while(ret == NMXP_SOCKET_OK   &&    type != NMXP_MSG_READY) {

		    /* Process a packet and return value in NMXP_DATA_PROCESS structure */
		    pd = nmxp_processCompressedData(buffer, length, channelList_subset, CURRENT_NETWORK);
		    nmxp_data_trim(pd, params.start_time, params.end_time, 0);

		    /* Log contents of last packet */
		    if(params.flag_logdata) {
			nmxp_data_log(pd);
		    }

		    /* Set cur_chan */
		    cur_chan = nmxp_chan_lookupKeyIndex(pd->key, channelList_subset);

		    /* Management of gaps */
		    if(!channelListSeq[cur_chan].significant && pd->nSamp > 0) {
			channelListSeq[cur_chan].significant = 1;
		    } else {
			if(channelListSeq[cur_chan].significant) {
			    if(nmxptool_check_and_log_gap(pd->time, channelListSeq[cur_chan].last_time, GAP_TOLLERANCE, pd->station, pd->channel)) {
				channelListSeq[cur_chan].x_1 = 0;
				nmxp_log(NMXP_LOG_WARN, 0, "%s.%s x0 set to zero!\n", pd->station, pd->channel);
			    }
			}
		    }
		    if(channelListSeq[cur_chan].significant && pd->nSamp > 0) {
			channelListSeq[cur_chan].last_time = pd->time + ((double) pd->nSamp / (double) pd->sampRate);
		    }

#ifdef HAVE_LIBMSEED
		    /* Write Mini-SEED record */
		    if(params.flag_writeseed) {
			nmxptool_write_miniseed(pd);
		    }
#endif

#ifdef HAVE___SRC_SEEDLINK_PLUGIN_C
		    /* Send data to SeedLink Server */
		    if(params.flag_slink) {
			nmxptool_send_raw_depoch(pd);
		    }
#endif

		    if(params.flag_writefile  &&  outfile) {
			/* Write buffer to the output file */
			if(outfile && buffer && length > 0) {
			    int32_t length_int = length;
			    nmxp_data_swap_4b((int32_t *) &length_int);
			    fwrite(&length_int, sizeof(length_int), 1, outfile);
			    fwrite(buffer, length, 1, outfile);
			}
		    }

		    /* Store x_1 */
		    if(pd->nSamp > 0) {
			channelListSeq[cur_chan].x_1 = pd->pDataPtr[pd->nSamp-1];
		    }
		    /* Free pd->buffer */
		    if(pd->buffer) {
			free(pd->buffer);
			pd->buffer = NULL;
		    }

		    /* Receive Data */
		    ret = nmxp_receiveMessage(naqssock, &type, &buffer, &length);
		    nmxp_log(0, 1, "ret = %d, type = %d\n", ret, type);
		}

		if(params.flag_writefile  &&  outfile) {
		    /* Close output file */
		    fclose(outfile);
		    outfile = NULL;
		}

#ifdef HAVE_LIBMSEED
		if(params.flag_writeseed  &&  data_seed.outfile_mseed) {
		    /* Close output Mini-SEED file */
		    fclose(data_seed.outfile_mseed);
		    data_seed.outfile_mseed = NULL;
		}
#endif

	    }
	    i_chan++;
	}
	/* DAP Step 7: Repeat steps 5 and 6 for each data request */

	if(params.delay > 0) {
	    time_to_sleep = (params.end_time - params.start_time) - (time(NULL) - (params.start_time + params.delay + span_interval));
	    if(time_to_sleep >= 0) {
		sleep(time_to_sleep);
	    } else {
		nmxp_log(1, 0, "time to sleep %dsec.\n", time_to_sleep);
		sleep(3);
	    }
	    params.start_time = params.end_time;
	    params.end_time = params.start_time + span_interval;
	} else {
	    exitdapcondition = 0;
	}


    } /* END while(exitdapcondition) */

	/* DAP Step 8: Send a Terminate message (optional) */
	nmxp_sendTerminateSubscription(naqssock, NMXP_SHUTDOWN_NORMAL, "Bye!");

	/* DAP Step 9: Close the socket */
	nmxp_closeSocket(naqssock);

	/* ************************************************************ */
	/* End subscription protocol "DATA ACCESS PROTOCOL" version 1.0 */
	/* ************************************************************ */



    } else {

	int n_func_pd = 0;
	int (*p_func_pd[NMXP_MAX_FUNC_PD]) (NMXP_DATA_PROCESS *);

	if(params.stc == -1) {


	    if(params.flag_logdata) {
		p_func_pd[n_func_pd++] = nmxptool_print_seq_no;
	    }

#ifdef HAVE_LIBMSEED
	    /* Write Mini-SEED record */
	    if(params.flag_writeseed) {
		p_func_pd[n_func_pd++] = nmxptool_write_miniseed;
	    }
#endif

#ifdef HAVE___SRC_SEEDLINK_PLUGIN_C
	    /* Send data to SeedLink Server */
	    if(params.flag_slink) {
		p_func_pd[n_func_pd++] = nmxptool_send_raw_depoch;
	    }
#endif
	}

	/* ************************************************************* */
	/* Start subscription protocol "PRIVATE DATA STREAM" version 1.4 */
	/* ************************************************************* */

	/* PDS Step 1: Open a socket */
	naqssock = nmxp_openSocket(params.hostname, params.portnumberpds);

	if(naqssock == NMXP_SOCKET_ERROR) {
	    return 1;
	}

	/* PDS Step 2: Send a Connect */
	if(nmxp_sendConnect(naqssock) != NMXP_SOCKET_OK) {
	    printf("Error on sendConnect()\n");
	    return 1;
	}

	/* PDS Step 3: Receive ChannelList */
	if(nmxp_receiveChannelList(naqssock, &channelList) != NMXP_SOCKET_OK) {
	    printf("Error on receiveChannelList()\n");
	    return 1;
	}

	/* Get a subset of channel from arguments */
	channelList_subset = nmxp_chan_subset(channelList, NMXP_DATA_TIMESERIES, params.channels);


	/* PDS Step 4: Send a Request Pending (optional) */


	/* PDS Step 5: Send AddChannels */
	/* Request Data */
	nmxp_sendAddTimeSeriesChannel(naqssock, channelList_subset, params.stc, params.rate, (params.flag_buffered)? NMXP_BUFFER_YES : NMXP_BUFFER_NO);

	/* PDS Step 6: Repeat until finished: receive and handle packets */

#ifdef HAVE_LIBMSEED
	if(params.flag_writeseed) {
	    /* Open output Mini-SEED file */
	    sprintf(data_seed.filename_mseed, "%s.realtime.miniseed",
		    CURRENT_NETWORK);

	    data_seed.outfile_mseed = fopen(data_seed.filename_mseed, "w");
	    if(!data_seed.outfile_mseed) {
		nmxp_log(1, 0, "Can not to open file %s!", data_seed.filename_mseed);
	    } else {
		nmxp_log(0, 1, "Opened file %s!\n", data_seed.filename_mseed);
	    }
	}
#endif

	// TODO
	exitpdscondition = 1;

	while(exitpdscondition) {
	    /* Process Compressed or Decompressed Data */
	    pd = nmxp_receiveData(naqssock, channelList_subset, CURRENT_NETWORK);

	    /* Log contents of last packet */
	    if(params.flag_logdata) {
		nmxp_data_log(pd);
	    }

	    /* Set cur_chan */
	    cur_chan = nmxp_chan_lookupKeyIndex(pd->key, channelList_subset);

	    /* Manage Raw Stream */
	    if(params.stc == -1) {
		nmxp_raw_stream_manage(&(channelListSeq[cur_chan].raw_stream_buffer), pd, p_func_pd, n_func_pd);
	    } else {

		/* Management of gaps */
		if(!channelListSeq[cur_chan].significant && pd->nSamp > 0) {
		    channelListSeq[cur_chan].significant = 1;
		} else {
		    if(channelListSeq[cur_chan].significant) {
			if(nmxptool_check_and_log_gap(pd->time, channelListSeq[cur_chan].last_time, GAP_TOLLERANCE, pd->station, pd->channel)) {
			    channelListSeq[cur_chan].x_1 = 0;
			    nmxp_log(NMXP_LOG_WARN, 0, "%s.%s x0 set to zero!\n", pd->station, pd->channel);
			}
		    }
		}
		if(channelListSeq[cur_chan].significant && pd->nSamp > 0) {
		    channelListSeq[cur_chan].last_time = pd->time + ((double) pd->nSamp / (double) pd->sampRate);
		}


#ifdef HAVE_LIBMSEED
		/* Write Mini-SEED record */
		if(params.flag_writeseed) {
		    nmxptool_write_miniseed(pd);
		}
#endif

#ifdef HAVE___SRC_SEEDLINK_PLUGIN_C
		/* Send data to SeedLink Server */
		if(params.flag_slink) {
		    nmxptool_send_raw_depoch(pd);
		}
#endif
	    }

	    /* Store x_1 */
	    if(pd->nSamp > 0) {
		channelListSeq[cur_chan].x_1 = pd->pDataPtr[pd->nSamp-1];
	    }
	    /* Free pd->buffer */
	    if(pd->buffer) {
		free(pd->buffer);
		pd->buffer = NULL;
	    }

	    // TODO
	    exitpdscondition = 1;
	}

#ifdef HAVE_LIBMSEED
	if(params.flag_writeseed  &&  data_seed.outfile_mseed) {
	    /* Close output Mini-SEED file */
	    fclose(data_seed.outfile_mseed);
	}
#endif


	/* PDS Step 7: Send Terminate Subscription */
	nmxp_sendTerminateSubscription(naqssock, NMXP_SHUTDOWN_NORMAL, "Good Bye!");

	/* PDS Step 8: Close the socket */
	nmxp_closeSocket(naqssock);

	/* *********************************************************** */
	/* End subscription protocol "PRIVATE DATA STREAM" version 1.4 */
	/* *********************************************************** */




    }

#ifdef HAVE_LIBMSEED
	if(*msr_list_chan) {
	    for(i_chan = 0; i_chan < channelList_subset->number; i_chan++) {
		if(msr_list_chan[i_chan]) {
		    msr_free(&(msr_list_chan[i_chan]));
		}
	    }
	}
#endif

	for(i_chan = 0; i_chan < channelList_subset->number; i_chan++) {
	    nmxp_raw_stream_free(&(channelListSeq[i_chan].raw_stream_buffer));
	}

	if(channelListSeq) {
	    free(channelListSeq);
	}

	/* This has to be tha last */
	if(channelList_subset) {
	    free(channelList_subset);
	}


    return 0;
} /* End MAIN */





/* Do any needed cleanup and exit */
static void clientShutdown(int sig) {

    nmxp_log(0, 0, "Program interrupted!\n");

    if(params.flag_writefile  &&  outfile) {
	/* Close output file */
	fclose(outfile);
    }

#ifdef HAVE_LIBMSEED
    if(params.flag_writeseed  &&  data_seed.outfile_mseed) {
	/* Close output Mini-SEED file */
	fclose(data_seed.outfile_mseed);
    }
#endif


    /* PDS Step 7: Send Terminate Subscription */
    nmxp_sendTerminateSubscription(naqssock, NMXP_SHUTDOWN_NORMAL, "Good Bye!");

    /* PDS Step 8: Close the socket */
    nmxp_closeSocket(naqssock);


    if(channelList == NULL) {
	free(channelList);
    }

#ifdef HAVE_LIBMSEED
    int i_chan;
    if(*msr_list_chan) {
	for(i_chan = 0; i_chan < channelList_subset->number; i_chan++) {
	    if(msr_list_chan[i_chan]) {
		msr_free(&(msr_list_chan[i_chan]));
	    }
	}
    }
#endif

    for(i_chan = 0; i_chan < channelList_subset->number; i_chan++) {
	nmxp_raw_stream_free(&(channelListSeq[i_chan].raw_stream_buffer));
    }

    if(channelListSeq) {
	free(channelListSeq);
    }

    /* This has to be the last */
    if(channelList_subset == NULL) {
	free(channelList_subset);
    }

    exit( sig );
} /* End of clientShutdown() */


/* Empty signal handler routine */
static void clientDummyHandler(int sig) {
}




int nmxptool_write_miniseed(NMXP_DATA_PROCESS *pd) {
    int cur_chan;
    int ret = 0;
    if( (cur_chan = nmxp_chan_lookupKeyIndex(pd->key, channelList_subset)) != -1) {

	ret = nmxp_data_msr_pack(pd, &data_seed, msr_list_chan[cur_chan], channelListSeq[cur_chan].x_1);

    } else {
	nmxp_log(1, 0, "Key %d not found in channelList_subset!\n", pd->key);
    }
    return ret;
}

int nmxptool_print_seq_no(NMXP_DATA_PROCESS *pd) {
    int ret = 0;

    nmxp_log(NMXP_LOG_NORM_NO, 0, "%s.%s %2d %d %d  lat. %.1fs\n",
	    pd->station,
	    pd->channel,
	    pd->packet_type,
	    pd->seq_no,
	    pd->oldest_seq_no,
	    nmxp_data_latency(pd)
	    );

    return ret;
}


int nmxptool_send_raw_depoch(NMXP_DATA_PROCESS *pd) {
    /* TODO Set values */
    const int usec_correction = 0;
    const int timing_quality = 100;

    return send_raw_depoch(pd->station, pd->channel, pd->time, usec_correction, timing_quality,
	    pd->pDataPtr, pd->nSamp);
}



int nmxptool_check_and_log_gap(double time1, double time2, const double gap_tollerance, const char *station, const char *channel) {
    char str_time1[200];
    char str_time2[200];
    int ret = 0;
    double gap = time1 - time2 ;
    nmxp_data_to_str(str_time1, time1);
    nmxp_data_to_str(str_time2, time2);
    if(gap > gap_tollerance) {
	nmxp_log(1, 0, "Gap %.2f sec. for %s.%s from %s to %s!\n", gap, station, channel, str_time1, str_time2);
	ret = 1;
    } else if (gap < -gap_tollerance) {
	nmxp_log(1, 0, "Overlap %.2f sec. for %s.%s from %s to %s!\n", gap, station, channel, str_time2, str_time1);
	ret = 1;
    }
    return ret;
}

void nmxptool_str_time_to_filename(char *str_time) {
    int i;
    for(i=0; i<strlen(str_time); i++) {
	if(   (str_time[i] >= 'A'  &&  str_time[i] <= 'Z')
		|| (str_time[i] >= 'a'  &&  str_time[i] <= 'z')
		|| (str_time[i] >= '0'  &&  str_time[i] <= '9')
		|| (str_time[i] == '_')
	  ) {
	    /* Do nothing */
	} else {
	    str_time[i] = '.';
	}
    }
}
