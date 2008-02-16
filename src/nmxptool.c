/*! \file
 *
 * \brief Nanometrics Protocol Tool
 *
 * Author:
 * 	Matteo Quintiliani
 * 	Istituto Nazionale di Geofisica e Vulcanologia - Italy
 *	quintiliani@ingv.it
 *
 * $Id: nmxptool.c,v 1.126 2008-02-16 15:32:23 mtheo Exp $
 *
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <nmxp.h>

#ifndef HAVE_WINDOWS_H
#include <signal.h>
#endif

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

#include "nmxptool_getoptlong.h"

#ifdef HAVE_EARTHWORMOBJS
#include "nmxptool_ew.h"
#endif

#ifdef HAVE_LIBMSEED
#include <libmseed.h>
#endif

#ifdef HAVE___SRC_SEEDLINK_PLUGIN_H
#include "seedlink_plugin.h"
#endif

#define TIMES_FLOW_EXIT 100

#define DAP_CONDITION(params_struct) ( params_struct.start_time != 0.0 || params_struct.delay > 0 )

#define CURRENT_NETWORK ( (params.network)? params.network : DEFAULT_NETWORK )
#define NETCODE_OR_CURRENT_NETWORK ( (network_code[0] != 0)? network_code : CURRENT_NETWORK )

#define GAP_TOLLERANCE 0.001

typedef struct {
    int significant;
    double last_time;
    time_t last_time_call_raw_stream;
    int32_t x_1;
    double after_start_time;
    NMXP_RAW_STREAM_DATA raw_stream_buffer;
} NMXPTOOL_CHAN_SEQ;


#ifndef HAVE_WINDOWS_H
static void clientShutdown(int sig);
static void clientDummyHandler(int sig);
#endif

static void save_channel_states(NMXP_CHAN_LIST_NET *chan_list, NMXPTOOL_CHAN_SEQ *chan_list_seq);
void load_channel_states(NMXP_CHAN_LIST_NET *chan_list, NMXPTOOL_CHAN_SEQ *chan_list_seq);
static void flushing_raw_data_stream();

#ifdef HAVE_LIBMSEED
int nmxptool_write_miniseed(NMXP_DATA_PROCESS *pd);
#endif

#ifdef HAVE___SRC_SEEDLINK_PLUGIN_C
int nmxptool_send_raw_depoch(NMXP_DATA_PROCESS *pd);
#endif

int nmxptool_print_seq_no(NMXP_DATA_PROCESS *pd);

int nmxptool_check_and_log_gap(double time1, double time2, const double gap_tollerance, const char *station, const char *channel);
void nmxptool_str_time_to_filename(char *str_time);


/* Global variable for main program and handling terminitation program */
NMXPTOOL_PARAMS params;
int naqssock = 0;
FILE *outfile = NULL;
NMXP_CHAN_LIST *channelList = NULL;
NMXP_CHAN_LIST_NET *channelList_subset = NULL;
NMXP_CHAN_LIST_NET *channelList_subset_waste = NULL;
NMXPTOOL_CHAN_SEQ *channelList_Seq = NULL;
int n_func_pd = 0;
int (*p_func_pd[NMXP_MAX_FUNC_PD]) (NMXP_DATA_PROCESS *);


#ifdef HAVE_LIBMSEED
/* Mini-SEED variables */
NMXP_DATA_SEED data_seed;
MSRecord *msr_list_chan[MAX_N_CHAN];
#endif


int main (int argc, char **argv) {
    int32_t connection_time;
    int request_SOCKET_OK;
    int i_chan, cur_chan = 0;
    int to_cur_chan = 0;
    int exitpdscondition;
    int exitdapcondition;
    time_t timeout_for_channel;

    int span_interval = 10;
    int time_to_sleep = 0;

    char str_start_time[200];
    char str_end_time[200];
    char str_pd_time[200];

    NMXP_MSG_SERVER type;
    void *buffer;
    int32_t length;
    int ret;

    int pd_null_count = 0;
    int timeoutrecv_warning = 300; /* 5 minutes */

    int recv_errno = 0;

    char filename[500];
    char station_code[20], channel_code[20], network_code[20];

    char cur_after_start_time_str[1024];
    double cur_after_start_time = DEFAULT_BUFFERED_TIME;
    int skip_current_packet = 0;

    int times_flow = 0;
    double default_start_time = 0.0;
    char start_time_str[30], end_time_str[30], default_start_time_str[30];

    NMXP_DATA_PROCESS *pd;

#ifdef HAVE_LIBMSEED
    /* Init mini-SEED variables */
    nmxp_data_seed_init(&data_seed);
#endif

#ifndef HAVE_WINDOWS_H
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
    nmxp_log(NMXP_LOG_SET, NMXP_LOG_D_NULL);

    /* Initialize params from argument values */
    if(nmxptool_getopt_long(argc, argv, &params) != 0) {
	return 1;
    }

    if(params.ew_configuration_file) {

#ifdef HAVE_EARTHWORMOBJS

	nmxp_log_init(nmxptool_ew_logit_msg, nmxptool_ew_logit_err);

	nmxptool_ew_configure(argv, &params);

	/* Check consistency of params */
	if(nmxptool_check_params(&params) != 0) {
	    return 1;
	}

#endif

    } else {

	nmxp_log_init(nmxp_log_stdout, nmxp_log_stderr);

	/* Check consistency of params */
	if(nmxptool_check_params(&params) != 0) {
	    return 1;
	}

	/* List available channels on server */
	if(params.flag_listchannels) {

	    nmxp_meta_chan_print(nmxp_getMetaChannelList(params.hostname, params.portnumberdap, NMXP_DATA_TIMESERIES, params.flag_request_channelinfo, params.datas_username, params.datas_password, &channelList));

	    return 1;

	} else if(params.flag_listchannelsnaqs) {

	    channelList = nmxp_getAvailableChannelList(params.hostname, params.portnumberpds, NMXP_DATA_TIMESERIES);
	    nmxp_chan_print_channelList(channelList);
	    return 1;

	}
    }

    nmxp_log(NMXP_LOG_SET, params.verbose_level);
    nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_ANY, "verbose_level %d\n", params.verbose_level);

    nmxptool_log_params(&params);

    /* Get list of available channels and get a subset list of params.channels */
    if( DAP_CONDITION(params) ) {
	/* From DataServer */
	nmxp_getMetaChannelList(params.hostname, params.portnumberdap, NMXP_DATA_TIMESERIES, params.flag_request_channelinfo, params.datas_username, params.datas_password, &channelList);
    } else {
	/* From NaqsServer */
	channelList = nmxp_getAvailableChannelList(params.hostname, params.portnumberpds, NMXP_DATA_TIMESERIES);
    }

    channelList_subset = nmxp_chan_subset(channelList, NMXP_DATA_TIMESERIES, params.channels, CURRENT_NETWORK);

    /* Check if some channel already exists */
    if(channelList_subset->number <= 0) {
	nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_CHANNEL, "Channels not found!\n");
	return 1;
    } else {
	nmxp_chan_print_netchannelList(channelList_subset);

	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_CHANNEL, "Init channelList_Seq.\n");

	/* init channelList_Seq */
	channelList_Seq = (NMXPTOOL_CHAN_SEQ *) malloc(sizeof(NMXPTOOL_CHAN_SEQ) * channelList_subset->number);
	for(i_chan = 0; i_chan < channelList_subset->number; i_chan++) {
	    channelList_Seq[i_chan].significant = 0;
	    channelList_Seq[i_chan].last_time = 0.0;
	    channelList_Seq[i_chan].last_time_call_raw_stream = 0;
	    channelList_Seq[i_chan].x_1 = 0;
	    channelList_Seq[i_chan].after_start_time = DEFAULT_BUFFERED_TIME;
	    nmxp_raw_stream_init(&(channelList_Seq[i_chan].raw_stream_buffer), params.max_tolerable_latency, params.timeoutrecv);
	}

#ifdef HAVE_LIBMSEED
	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_EXTRA, "Init mini-SEED record list.\n");

	/* Init mini-SEED record list */
	for(i_chan = 0; i_chan < channelList_subset->number; i_chan++) {

	    nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_EXTRA, "Init mini-SEED record for %s\n", channelList_subset->channel[i_chan].name);

	    msr_list_chan[i_chan] = msr_init(NULL);

	    /* Separate station_code and channel_code */
	    if(nmxp_chan_cpy_sta_chan(channelList_subset->channel[i_chan].name, station_code, channel_code, network_code)) {

		nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_EXTRA, "%s.%s.%s\n", NETCODE_OR_CURRENT_NETWORK, station_code, channel_code);

		strcpy(msr_list_chan[i_chan]->network, NETCODE_OR_CURRENT_NETWORK);
		strcpy(msr_list_chan[i_chan]->station, station_code);
		strcpy(msr_list_chan[i_chan]->channel, channel_code);

		msr_list_chan[i_chan]->reclen = 512;         /* byte record length */
		msr_list_chan[i_chan]->encoding = DE_STEIM1;  /* Steim 1 compression */

	    } else {
		nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_CHANNEL, "Channels %s error in format!\n");
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

#ifdef HAVE_EARTHWORMOBJS
	if(params.ew_configuration_file) {
	    nmxptool_ew_attach();
	}
#endif

    nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_CONNFLOW, "Starting comunication.\n");

    times_flow = 0;

    while(times_flow < 2) {

	if(params.statefile) {
	    load_channel_states(channelList_subset, channelList_Seq);
	}

	if(times_flow == 0) {
	    if(params.statefile) {
		params.interval = DEFAULT_INTERVAL_INFINITE;
	    }
	} else if(times_flow == 1) {
	    params.start_time = 0.0;
	    params.end_time = 0.0;
	    params.interval = DEFAULT_INTERVAL_NO_VALUE;

	}

    /* TODO condition starting DAP or PDS */
    if( DAP_CONDITION(params) || (times_flow == 0  &&  params.statefile  && params.interval == DEFAULT_INTERVAL_INFINITE) ) {

	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_CONNFLOW, "Begin DAP Flow.\n");

	if(params.interval > 0  ||  params.interval == DEFAULT_INTERVAL_INFINITE) {
	    if(params.interval > 0) {
		params.end_time = params.start_time + params.interval;
	    } else {
		params.end_time = nmxp_data_gmtime_now();
	    }
	} else if(params.delay > 0) {
	    params.start_time = ((double) (time(NULL) - params.delay - span_interval) / 10.0) * 10.0;
	    params.end_time = params.start_time + span_interval;
	}


	/* ************************************************************** */
	/* Start subscription protocol "DATA ACCESS PROTOCOL" version 1.0 */
	/* ************************************************************** */

	/* DAP Step 1: Open a socket */
	if( (naqssock = nmxp_openSocket(params.hostname, params.portnumberdap)) == NMXP_SOCKET_ERROR) {
	    nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_CONNFLOW, "Error opening socket!\n");
	    return 1;
	}

	/* DAP Step 2: Read connection time */
	if(nmxp_readConnectionTime(naqssock, &connection_time) != NMXP_SOCKET_OK) {
	    nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_CONNFLOW, "Error reading connection time from server!\n");
	    return 1;
	}

	/* DAP Step 3: Send a ConnectRequest */
	if(nmxp_sendConnectRequest(naqssock, params.datas_username, params.datas_password, connection_time) != NMXP_SOCKET_OK) {
	    nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_CONNFLOW, "Error sending connect request!\n");
	    return 1;
	}

	/* DAP Step 4: Wait for a Ready message */
	if(nmxp_waitReady(naqssock) != NMXP_SOCKET_OK) {
	    nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_CONNFLOW, "Error waiting Ready message!\n");
	    return 1;
	}

	exitdapcondition = 1;

	default_start_time = (params.start_time > 0.0)? params.start_time : nmxp_data_gmtime_now() - params.max_data_to_retrieve;

	while(exitdapcondition) {

	    /* Start loop for sending requests */
	    i_chan=0;
	    request_SOCKET_OK = NMXP_SOCKET_OK;

	    while(request_SOCKET_OK == NMXP_SOCKET_OK  &&  i_chan < channelList_subset->number) {

		if(params.statefile) {
		    if(channelList_Seq[i_chan].after_start_time > 0) {
			params.start_time = channelList_Seq[i_chan].after_start_time;
			if(params.end_time - params.start_time > params.max_data_to_retrieve) {
			    nmxp_data_to_str(start_time_str, params.start_time);
			    nmxp_data_to_str(default_start_time_str, params.end_time - params.max_data_to_retrieve);
			    nmxp_log(NMXP_LOG_WARN, NMXP_LOG_D_ANY, "%s start_time changed from %s to %s\n",
				    channelList_subset->channel[i_chan].name, start_time_str, default_start_time_str);
			    params.start_time = params.end_time - params.max_data_to_retrieve;
			}
		    } else {
			params.start_time = default_start_time;
		    }
		    channelList_Seq[i_chan].last_time = params.start_time;
		    channelList_Seq[i_chan].significant = 1;

		}

		nmxp_data_to_str(start_time_str, params.start_time);
		nmxp_data_to_str(end_time_str, params.end_time);
		nmxp_data_to_str(default_start_time_str, default_start_time);
		nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_EXTRA, "%s start_time = %s - end_time = %s - (default_start_time = %s)\n",
			channelList_subset->channel[i_chan].name, start_time_str, end_time_str, default_start_time_str);

		/* DAP Step 5: Send Data Request */
		request_SOCKET_OK = nmxp_sendDataRequest(naqssock, channelList_subset->channel[i_chan].key, (int32_t) params.start_time, (int32_t) (params.end_time + 1.0));

		if(request_SOCKET_OK == NMXP_SOCKET_OK) {

		    nmxp_data_to_str(str_start_time, params.start_time);
		    nmxp_data_to_str(str_end_time, params.end_time);
		    nmxptool_str_time_to_filename(str_start_time);
		    nmxptool_str_time_to_filename(str_end_time);

		    if(params.flag_writefile) {
			/* Open output file */
			if(nmxp_chan_cpy_sta_chan(channelList_subset->channel[i_chan].name, station_code, channel_code, network_code)) {
			    sprintf(filename, "%s.%s.%s_%s_%s.nmx",
				    NETCODE_OR_CURRENT_NETWORK,
				    station_code,
				    channel_code,
				    str_start_time,
				    str_end_time);
			} else {
			    sprintf(filename, "%s_%s_%s.nmx",
				    channelList_subset->channel[i_chan].name,
				    str_start_time,
				    str_end_time);
			}

			outfile = fopen(filename, "w");
			if(!outfile) {
			    nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_EXTRA, "Can not to open file %s!", filename);
			}
		    }

#ifdef HAVE_LIBMSEED
		    if(params.flag_writeseed) {
			/* Open output Mini-SEED file */
			if(nmxp_chan_cpy_sta_chan(channelList_subset->channel[i_chan].name, station_code, channel_code, network_code)) {
			    sprintf(data_seed.filename_mseed, "%s.%s.%s_%s_%s.miniseed",
				    NETCODE_OR_CURRENT_NETWORK,
				    station_code,
				    channel_code,
				    str_start_time,
				    str_end_time);
			} else {
			    sprintf(filename, "%s_%s_%s.miniseed",
				    channelList_subset->channel[i_chan].name,
				    str_start_time,
				    str_end_time);
			}

			data_seed.outfile_mseed = fopen(data_seed.filename_mseed, "w");
			if(!data_seed.outfile_mseed) {
			    nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_EXTRA, "Can not to open file %s!", data_seed.filename_mseed);
			}
		    }
#endif

		    if(params.flag_writefile  &&  outfile) {
			/* Compute SNCL line */

			/* Separate station_code_old_way and channel_code_old_way */
			if(nmxp_chan_cpy_sta_chan(channelList_subset->channel[i_chan].name, station_code, channel_code, network_code)) {
			    /* Write SNCL line */
			    fprintf(outfile, "%s.%s.%s.%s\n",
				    station_code,
				    NETCODE_OR_CURRENT_NETWORK,
				    channel_code,
				    (params.location)? params.location : "");
			}

		    }

		    /* DAP Step 6: Receive Data until receiving a Ready message */
		    ret = nmxp_receiveMessage(naqssock, &type, &buffer, &length, 0, &recv_errno);
		    /* nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_CONNFLOW, "ret = %d, type = %d\n", ret, type); */

		    while(ret == NMXP_SOCKET_OK   &&    type != NMXP_MSG_READY) {

			/* Process a packet and return value in NMXP_DATA_PROCESS structure */
			pd = nmxp_processCompressedData(buffer, length, channelList_subset, NETCODE_OR_CURRENT_NETWORK);
			nmxp_data_trim(pd, params.start_time, params.end_time, 0);

			/* To prevent to manage a packet with zero sample after nmxp_data_trim() */
			if(pd->nSamp > 0) {

			/* Log contents of last packet */
			if(params.flag_logdata) {
			    nmxp_data_log(pd);
			}

			/* Set cur_chan */
			cur_chan = nmxp_chan_lookupKeyIndex(pd->key, channelList_subset);
			if(i_chan != cur_chan) {
			    nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_ANY, "i_chan != cur_chan  %d != %d!\n", i_chan, cur_chan);
			}

			/* Management of gaps */
			if(!channelList_Seq[cur_chan].significant && pd->nSamp > 0) {
			    channelList_Seq[cur_chan].significant = 1;
			} else {
			    if(channelList_Seq[cur_chan].significant && pd->nSamp > 0) {
				if(nmxptool_check_and_log_gap(pd->time, channelList_Seq[cur_chan].last_time, GAP_TOLLERANCE, pd->station, pd->channel)) {
				    channelList_Seq[cur_chan].x_1 = 0;
				    nmxp_data_to_str(str_pd_time, pd->time);
				    nmxp_log(NMXP_LOG_WARN, NMXP_LOG_D_EXTRA, "%s.%s x0 set to zero at %s!\n", pd->station, pd->channel, str_pd_time);
				}
			    }
			}
			if(channelList_Seq[cur_chan].significant && pd->nSamp > 0) {
			    channelList_Seq[cur_chan].last_time = pd->time + ((double) pd->nSamp / (double) pd->sampRate);
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

#ifdef HAVE_EARTHWORMOBJS
			if(params.ew_configuration_file) {
			    nmxptool_ew_nmx2ew(pd);
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

			}

			/* Store x_1 */
			if(pd->nSamp > 0) {
			    channelList_Seq[cur_chan].x_1 = pd->pDataPtr[pd->nSamp-1];
			}

			/* Free pd->buffer */
			if(pd->buffer) {
			    free(pd->buffer);
			    pd->buffer = NULL;
			}

			/* Receive Data */
			ret = nmxp_receiveMessage(naqssock, &type, &buffer, &length, 0, &recv_errno);
			/* nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_CONNFLOW, "ret = %d, type = %d\n", ret, type); */
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
		    nmxp_sleep(time_to_sleep);
		} else {
		    nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_CONNFLOW, "time to sleep %d sec.\n", time_to_sleep);
		    nmxp_sleep(3);
		}
		params.start_time = params.end_time;
		params.end_time = params.start_time + span_interval;
	    } else {
		exitdapcondition = 0;
	    }


#ifdef HAVE_EARTHWORMOBJS
	    if(params.ew_configuration_file) {

		/* Check if we are being asked to terminate */
		if( nmxptool_ew_check_flag_terminate() ) {
		    logit ("t", "nmxptool terminating on request\n");
		    nmxptool_ew_send_error(NMXPTOOL_EW_ERR_TERMREQ);
		    exitdapcondition = 0;
		    times_flow = TIMES_FLOW_EXIT;
		}

		/* Check if we need to send heartbeat message */
		nmxptool_ew_send_heartbeat_if_needed();

	    }
#endif

	} /* END while(exitdapcondition) */

	/* DAP Step 8: Send a Terminate message (optional) */
	nmxp_sendTerminateSubscription(naqssock, NMXP_SHUTDOWN_NORMAL, "Bye!");

	/* DAP Step 9: Close the socket */
	nmxp_closeSocket(naqssock);

	/* ************************************************************ */
	/* End subscription protocol "DATA ACCESS PROTOCOL" version 1.0 */
	/* ************************************************************ */

	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_CONNFLOW, "End DAP Flow.\n");

    } else {

	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_CONNFLOW, "Begin PDS Flow.\n");

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

#ifdef HAVE_EARTHWORMOBJS
	    if(params.ew_configuration_file) {
		p_func_pd[n_func_pd++] = nmxptool_ew_nmx2ew;
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
	channelList_subset_waste = nmxp_chan_subset(channelList, NMXP_DATA_TIMESERIES, params.channels, CURRENT_NETWORK);


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
		nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_EXTRA, "Can not to open file %s!", data_seed.filename_mseed);
	    } else {
		nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_EXTRA, "Opened file %s!\n", data_seed.filename_mseed);
	    }
	}
#endif

	// TODO
	exitpdscondition = 1;

	skip_current_packet = 0;

	while(exitpdscondition) {

	    /* Process Compressed or Decompressed Data */
	    pd = nmxp_receiveData(naqssock, channelList_subset, NETCODE_OR_CURRENT_NETWORK, params.timeoutrecv, &recv_errno);

	    if(!pd) {
		pd_null_count++;
		if((pd_null_count * params.timeoutrecv) >= timeoutrecv_warning) {
		    nmxp_log(NMXP_LOG_WARN, NMXP_LOG_D_ANY, "Received %d times a null packet. (%d sec.)\n",
			    pd_null_count, pd_null_count * params.timeoutrecv);
		    pd_null_count = 0;
		}
	    } else {
		pd_null_count = 0;
	    }

	    if(recv_errno == 0) {
		// TODO
		exitpdscondition = 1;
	    } else {
		if(recv_errno == EWOULDBLOCK) {
		    // TODO
		    exitpdscondition = 1;
		} else {
		    nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_CONNFLOW, "Error receiving data. pd=%p recv_errno=%d\n", pd, recv_errno);

#ifdef HAVE_EARTHWORMOBJS
		    if(params.ew_configuration_file) {
			nmxptool_ew_send_error(NMXPTOOL_EW_ERR_RECVDATA);
		    }
#endif
		    exitpdscondition = 0;
		}
	    }

	    if(pd) {
		/* Set cur_chan */
		cur_chan = nmxp_chan_lookupKeyIndex(pd->key, channelList_subset);
	    }

	    /* Log contents of last packet */
	    if(params.flag_logdata) {
		nmxp_data_log(pd);
	    }

	    skip_current_packet = 0;
	    if(pd &&
		    (params.statefile  ||  params.buffered_time) &&
		    ( params.timeoutrecv <= 0 )
	      )	{
		if(params.statefile && channelList_Seq[cur_chan].after_start_time > 0.0) {
		    cur_after_start_time = channelList_Seq[cur_chan].after_start_time;
		} else if(params.buffered_time) {
		    cur_after_start_time = params.buffered_time;
		} else {
		    cur_after_start_time = DEFAULT_BUFFERED_TIME;
		}
		nmxp_data_to_str(cur_after_start_time_str, cur_after_start_time);
		nmxp_log(NMXP_LOG_WARN, NMXP_LOG_D_PACKETMAN, "cur_chan %d, cur_after_start_time %f, cur_after_start_time_str %s\n", cur_chan, cur_after_start_time, cur_after_start_time_str);
		if(pd->time + ((double) pd->nSamp / (double) pd->sampRate) >= cur_after_start_time) {
		    if(pd->time < cur_after_start_time) {
			int first_nsample_to_remove = (cur_after_start_time - pd->time) * (double) pd->sampRate;
			/* Remove the first sample in order avoiding overlap  */
			first_nsample_to_remove++;
			if(pd->nSamp > first_nsample_to_remove) {
			    pd->nSamp -= first_nsample_to_remove;
			    pd->time = cur_after_start_time;
			    pd->pDataPtr += first_nsample_to_remove;
			    pd->x0 = pd->pDataPtr[0];
			} else {
			    skip_current_packet = 1;
			}
		    }
		} else {
		    skip_current_packet = 1;
		}
	    }

	    if(!skip_current_packet) {

		/* Manage Raw Stream */
		if(params.stc == -1) {

		    /* cur_char is computed only for pd != NULL */
		    if(pd) {
			nmxp_raw_stream_manage(&(channelList_Seq[cur_chan].raw_stream_buffer), pd, p_func_pd, n_func_pd);
			channelList_Seq[cur_chan].last_time_call_raw_stream = nmxp_data_gmtime_now();
		    }

		    /* Check timeout for other channels */
		    if(params.timeoutrecv > 0) {
			exitpdscondition = 1;
			to_cur_chan = 0;
			while(to_cur_chan < channelList_subset->number) {
			    timeout_for_channel = nmxp_data_gmtime_now() - channelList_Seq[to_cur_chan].last_time_call_raw_stream;
			    if(channelList_Seq[to_cur_chan].last_time_call_raw_stream != 0
				    && timeout_for_channel >= params.timeoutrecv) {
				nmxp_log(NMXP_LOG_WARN, NMXP_LOG_D_DOD, "Timeout for channel %s (%d sec.)\n",
					channelList_subset->channel[to_cur_chan].name, timeout_for_channel);
				nmxp_raw_stream_manage(&(channelList_Seq[to_cur_chan].raw_stream_buffer), NULL, p_func_pd, n_func_pd);
				channelList_Seq[to_cur_chan].last_time_call_raw_stream = nmxp_data_gmtime_now();
			    }
			    to_cur_chan++;
			}
		    }

		} else {

		    if(pd) {
			/* Management of gaps */
			if(!channelList_Seq[cur_chan].significant && pd->nSamp > 0) {
			    channelList_Seq[cur_chan].significant = 1;
			} else {
			    if(channelList_Seq[cur_chan].significant && pd->nSamp > 0) {
				if(nmxptool_check_and_log_gap(pd->time, channelList_Seq[cur_chan].last_time, GAP_TOLLERANCE, pd->station, pd->channel)) {
				    channelList_Seq[cur_chan].x_1 = 0;
				    nmxp_data_to_str(str_pd_time, pd->time);
				    nmxp_log(NMXP_LOG_WARN, NMXP_LOG_D_EXTRA, "%s.%s x0 set to zero at %s!\n", pd->station, pd->channel, str_pd_time);
				}
			    }
			}
			if(channelList_Seq[cur_chan].significant && pd->nSamp > 0) {
			    channelList_Seq[cur_chan].last_time = pd->time + ((double) pd->nSamp / (double) pd->sampRate);
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
		}
	    } /* End skip_current_packet condition */

	    if(pd) {
		/* Store x_1 */
		if(pd->nSamp > 0) {
		    channelList_Seq[cur_chan].x_1 = pd->pDataPtr[pd->nSamp-1];
		}
		/* Free pd->buffer */
		if(pd->buffer) {
		    free(pd->buffer);
		    pd->buffer = NULL;
		}
	    }

#ifdef HAVE_EARTHWORMOBJS
	    if(params.ew_configuration_file) {

		/* Check if we are being asked to terminate */
		if( nmxptool_ew_check_flag_terminate() ) {
		    logit ("t", "nmxptool terminating on request\n");
		    nmxptool_ew_send_error(NMXPTOOL_EW_ERR_TERMREQ);
		    exitpdscondition = 0;
		}

		/* Check if we need to send heartbeat message */
		nmxptool_ew_send_heartbeat_if_needed();

	    }
#endif

	} /* End main PDS loop */

	/* Flush raw data stream for each channel */
	flushing_raw_data_stream();

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

	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_CONNFLOW, "End PDS Flow.\n");

    }

    if(params.interval == DEFAULT_INTERVAL_INFINITE) {
	times_flow++;
    } else {
	times_flow = TIMES_FLOW_EXIT;
    }

    if(params.statefile) {
	save_channel_states(channelList_subset, channelList_Seq);
    }

    }

#ifdef HAVE_EARTHWORMOBJS
	if(params.ew_configuration_file) {
	    nmxptool_ew_detach();
	}
#endif

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
	nmxp_raw_stream_free(&(channelList_Seq[i_chan].raw_stream_buffer));
    }

    if(channelList_Seq) {
	free(channelList_Seq);
    }

    /* This has to be tha last */
    if(channelList_subset) {
	free(channelList_subset);
    }

    return 0;
} /* End MAIN */


#define MAX_LEN_FILENAME 4096
#define NMXP_STR_STATE_EXT ".nmxpstate"

static void save_channel_states(NMXP_CHAN_LIST_NET *chan_list, NMXPTOOL_CHAN_SEQ *chan_list_seq) {
    int to_cur_chan;
    char last_time_str[30];
    char raw_last_sample_time_str[30];
    char state_line_str[1000];
    FILE *fstatefile = NULL;
    char statefilefilename[MAX_LEN_FILENAME] = "";

    if(params.statefile) {
	strncpy(statefilefilename, params.statefile, MAX_LEN_FILENAME);
	strncat(statefilefilename, NMXP_STR_STATE_EXT, MAX_LEN_FILENAME);
	fstatefile = fopen(statefilefilename, "w");
	if(fstatefile == NULL) {
	    nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_ANY, "Unable to write channel states into %s!\n", statefilefilename);
	} else {
	    nmxp_log(NMXP_LOG_WARN, NMXP_LOG_D_ANY, "Writing channel states into %s!\n", statefilefilename);
	}

	/* Save state for each channel */
	// if(params.stc == -1)
	to_cur_chan = 0;
	while(to_cur_chan < chan_list->number) {
	    nmxp_data_to_str(last_time_str, chan_list_seq[to_cur_chan].last_time);
	    nmxp_data_to_str(raw_last_sample_time_str, chan_list_seq[to_cur_chan].raw_stream_buffer.last_sample_time);
	    sprintf(state_line_str, "%d %s %s %s",
		    chan_list->channel[to_cur_chan].key,
		    chan_list->channel[to_cur_chan].name,
		    last_time_str,
		    raw_last_sample_time_str
		   );
	    nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_CHANNEL, "%s\n", state_line_str);
	    if(fstatefile) {
		fprintf(fstatefile, "%s\n", state_line_str);
		if( (chan_list_seq[to_cur_chan].last_time != 0) || (chan_list_seq[to_cur_chan].raw_stream_buffer.last_sample_time != -1.0) ) {
		    nmxp_log(NMXP_LOG_WARN, NMXP_LOG_D_ANY, "%s %d %d %f %f\n", state_line_str, to_cur_chan, chan_list->channel[to_cur_chan].key,
			    chan_list_seq[to_cur_chan].last_time, chan_list_seq[to_cur_chan].raw_stream_buffer.last_sample_time);
		} else {
		    /* Do nothing */
		}
	    }
	    to_cur_chan++;
	}
	if(fstatefile) {
	    fclose(fstatefile);
	}
    }
}

void load_channel_states(NMXP_CHAN_LIST_NET *chan_list, NMXPTOOL_CHAN_SEQ *chan_list_seq) {
    FILE *fstatefile = NULL;
    FILE *fstatefileINPUT = NULL;
#define MAXSIZE_LINE 2048
    char line[MAXSIZE_LINE];
    char s_chan[128];
    char s_noraw_time_s[128];
    char s_rawtime_s[128];
    double s_noraw_time_f_calc, s_rawtime_f_calc;
    int cur_chan;
    int n_scanf;
    int32_t key_chan;
    NMXP_TM_T tmp_tmt;
    char statefilefilename[MAX_LEN_FILENAME] = "";

    if(params.statefile) {
	strncpy(statefilefilename, params.statefile, MAX_LEN_FILENAME);
	strncat(statefilefilename, NMXP_STR_STATE_EXT, MAX_LEN_FILENAME);
	fstatefile = fopen(statefilefilename, "r");
	if(fstatefile == NULL) {
	    fstatefileINPUT = fopen(params.statefile, "r");
	    if(fstatefileINPUT == NULL) {
		nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_ANY, "Unable to read channel states from %s!\n", params.statefile);
	    } else {
		fstatefile = fopen(statefilefilename, "w");
		if(fstatefile == NULL) {
		    nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_ANY, "Unable to write channel states into %s!\n", statefilefilename);
		} else {
		    /*
		    while(fgets(line, MAXSIZE_LINE, fstatefileINPUT) != NULL) {
			fputs(line, fstatefile);
		    }
		    */
		    fclose(fstatefile);
		}
		fclose(fstatefileINPUT);
	    }
	}
    }

    if(params.statefile) {
	strncpy(statefilefilename, params.statefile, MAX_LEN_FILENAME);
	strncat(statefilefilename, NMXP_STR_STATE_EXT, MAX_LEN_FILENAME);
	fstatefile = fopen(statefilefilename, "r");
	if(fstatefile == NULL) {
	    nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_ANY, "Unable to read channel states from %s!\n", statefilefilename);
	} else {
	    nmxp_log(NMXP_LOG_WARN, NMXP_LOG_D_ANY, "Loading channel states from %s!\n", statefilefilename);
	    while(fgets(line, MAXSIZE_LINE, fstatefile) != NULL) {
		s_chan[0] = 0;
		s_noraw_time_s[0] = 0;
		s_rawtime_s[0] = 0;
		n_scanf = sscanf(line, "%d %s %s %s", &key_chan, s_chan, s_noraw_time_s, s_rawtime_s); 

		s_noraw_time_f_calc = DEFAULT_BUFFERED_TIME;
		s_rawtime_f_calc = DEFAULT_BUFFERED_TIME;
		if(n_scanf == 4) {
		    if(nmxp_data_parse_date(s_noraw_time_s, &tmp_tmt) == -1) {
			nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_ANY, "Parsing time '%s'\n", s_noraw_time_s); 
		    } else {
			s_noraw_time_f_calc = nmxp_data_tm_to_time(&tmp_tmt);
		    }
		    if(nmxp_data_parse_date(s_rawtime_s, &tmp_tmt) == -1) {
			nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_ANY, "Parsing time '%s'\n", s_rawtime_s); 
		    } else {
			s_rawtime_f_calc = nmxp_data_tm_to_time(&tmp_tmt);
		    }
		}

		nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_EXTRA, "%d %12d %-14s %16.4f %s %16.4f %s\n", n_scanf, key_chan, s_chan, s_noraw_time_f_calc, s_noraw_time_s, s_rawtime_f_calc, s_rawtime_s); 

		cur_chan = 0;
		while(cur_chan < chan_list->number  &&  strcasecmp(s_chan, chan_list->channel[cur_chan].name) != 0) {
		    cur_chan++;
		}
		if(cur_chan < chan_list->number) {
		    if( s_rawtime_f_calc != DEFAULT_BUFFERED_TIME  && s_rawtime_f_calc != 0.0 ) {
			chan_list_seq[cur_chan].after_start_time = s_rawtime_f_calc;
			nmxp_log(NMXP_LOG_WARN, NMXP_LOG_D_ANY, "For channel %s (%d %d) starting from %s. %f.\n", s_chan, cur_chan, chan_list->channel[cur_chan].key, s_rawtime_s, s_rawtime_f_calc); 
		    } else if( s_noraw_time_f_calc != DEFAULT_BUFFERED_TIME ) {
			chan_list_seq[cur_chan].after_start_time = s_noraw_time_f_calc;
			nmxp_log(NMXP_LOG_WARN, NMXP_LOG_D_ANY, "For channel %s (%d %d) starting from %s. %f.\n", s_chan, cur_chan, chan_list->channel[cur_chan].key, s_noraw_time_s, s_noraw_time_f_calc); 
		    } else {
			nmxp_log(NMXP_LOG_WARN, NMXP_LOG_D_ANY, "For channel %s there is not valid start_time.\n", s_chan); 
		    }
		} else {
		    nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_ANY, "Channel %s not found! (%d %s)\n", s_chan, strlen(line), line); 
		}
	    }
	    fclose(fstatefile);
	}
    }
    errno = 0;
}


static void flushing_raw_data_stream() {
    int to_cur_chan;

    /* Flush raw data stream for each channel */
    if(params.stc == -1) {
	to_cur_chan = 0;
	while(to_cur_chan < channelList_subset->number) {
	    nmxp_log(NMXP_LOG_WARN, NMXP_LOG_D_ANY, "Flushing data for channel %s\n",
		    channelList_subset->channel[to_cur_chan].name);
	    nmxp_raw_stream_manage(&(channelList_Seq[to_cur_chan].raw_stream_buffer), NULL, p_func_pd, n_func_pd);
	    to_cur_chan++;
	}
    }
}

#ifndef HAVE_WINDOWS_H
/* Do any needed cleanup and exit */
static void clientShutdown(int sig) {

    nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_ANY, "Program interrupted!\n");

    flushing_raw_data_stream();

    if(params.statefile) {
	save_channel_states(channelList_subset, channelList_Seq);
    }

    if(params.flag_writefile  &&  outfile) {
	/* Close output file */
	fclose(outfile);
    }

#ifdef HAVE_EARTHWORMOBJS
    if(params.ew_configuration_file) {
	nmxptool_ew_detach();
    }
#endif

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



    /* Free the complete channel list */
    if(channelList) {
	free(channelList);
	channelList = NULL;
    }

    int i_chan = 0;

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
	nmxp_raw_stream_free(&(channelList_Seq[i_chan].raw_stream_buffer));
    }

    if(channelList_Seq) {
	free(channelList_Seq);
    }

    /* This has to be the last */
    if(channelList_subset == NULL) {
	free(channelList_subset);
    }

    exit( sig );
} /* End of clientShutdown() */


/* Empty signal handler routine */
static void clientDummyHandler(int sig) {
    int chan_index;
    for(chan_index = 0; chan_index < channelList_subset->number; chan_index++) {
	nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY,
		"i %2d s %d lt %f ltrs %d x_1 %8d after_st %d ",
	chan_index,
	channelList_Seq[chan_index].significant,
	channelList_Seq[chan_index].last_time,
	channelList_Seq[chan_index].last_time_call_raw_stream,
	channelList_Seq[chan_index].x_1,
	channelList_Seq[chan_index].after_start_time
		);

	nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY,
	    "sqno %d lst %.4f maxit %d maxlat %.2f torv %d nit %d\n",
	channelList_Seq[chan_index].raw_stream_buffer.last_seq_no_sent,
	channelList_Seq[chan_index].raw_stream_buffer.last_sample_time,
	channelList_Seq[chan_index].raw_stream_buffer.max_pdlist_items,
	channelList_Seq[chan_index].raw_stream_buffer.max_tolerable_latency,
	channelList_Seq[chan_index].raw_stream_buffer.timeoutrecv,
	channelList_Seq[chan_index].raw_stream_buffer.n_pdlist
		);
    }
}

#endif /* HAVE_WINDOWS_H */




#ifdef HAVE_LIBMSEED
int nmxptool_write_miniseed(NMXP_DATA_PROCESS *pd) {
    int cur_chan;
    int ret = 0;
    if( (cur_chan = nmxp_chan_lookupKeyIndex(pd->key, channelList_subset)) != -1) {

	ret = nmxp_data_msr_pack(pd, &data_seed, msr_list_chan[cur_chan]);

    } else {
	nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_CHANNEL, "Key %d not found in channelList_subset!\n", pd->key);
    }
    return ret;
}
#endif

int nmxptool_print_seq_no(NMXP_DATA_PROCESS *pd) {
    int ret = 0;
    char str_time[200];
    nmxp_data_to_str(str_time, pd->time);

    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "Process %s.%s.%s %2d %d %d %s %dpts lat. %.1fs\n",
	    pd->network,
	    pd->station,
	    pd->channel,
	    pd->packet_type,
	    pd->seq_no,
	    pd->oldest_seq_no,
	    str_time,
	    pd->nSamp,
	    nmxp_data_latency(pd)
	    );

    return ret;
}


#ifdef HAVE___SRC_SEEDLINK_PLUGIN_C
int nmxptool_send_raw_depoch(NMXP_DATA_PROCESS *pd) {
    /* TODO Set values */
    const int usec_correction = 0;
    const int timing_quality = 100;

    return send_raw_depoch(pd->station, pd->channel, pd->time, usec_correction, timing_quality,
	    pd->pDataPtr, pd->nSamp);
}
#endif



int nmxptool_check_and_log_gap(double time1, double time2, const double gap_tollerance, const char *station, const char *channel) {
    char str_time1[200];
    char str_time2[200];
    int ret = 0;
    double gap = time1 - time2 ;
    nmxp_data_to_str(str_time1, time1);
    nmxp_data_to_str(str_time2, time2);
    if(gap > gap_tollerance) {
	nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_GAP, "Gap %.2f sec. for %s.%s from %s to %s!\n", gap, station, channel, str_time2, str_time1);
	ret = 1;
    } else if (gap < -gap_tollerance) {
	nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_GAP, "Overlap %.2f sec. for %s.%s from %s to %s!\n", gap, station, channel, str_time1, str_time2);
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
