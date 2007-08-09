#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nmxp.h>

#include "nmxptool_getoptlong.h"

#include "config.h"

#ifdef HAVE___SRC_SEEDLINK_PLUGIN_H
#include "seedlink_plugin.h"
#endif


int main (int argc, char **argv) {
    int naqssock;
    NMXP_CHAN_LIST *channelList;
    NMXP_CHAN_LIST *channelList_subset;
    uint32_t connection_time;
    int request_SOCKET_OK;
    int i_chan;

    NMXP_MSG_SERVER type;
    void *buffer;
    uint32_t length;
    int ret;

    FILE *outfile = NULL;
    char filename[500];
    char *station_code = NULL, *channel_code = NULL;

    NMXP_DATA_PROCESS *pd;

    NMXPTOOL_PARAMS params;

#ifdef HAVE_LIBMSEED
    /* Mini-SEED variables */
    NMXP_DATA_SEED data_seed;

    /* Init mini-SEED variables */
    nmxp_data_seed_init(&data_seed);
#endif


    nmxp_log(-1, 0);

    /* Initialize params from argument values */
    if(nmxptool_getopt_long(argc, argv, &params) != 0) {
	return 1;
    }

    /* Check consistency of params */
    if(nmxptool_check_params(&params) != 0) {
	return 1;
    }

    /* List available channels on server */
    if(params.flag_listchannels) {

	channelList = nmxp_getAvailableChannelList(params.hostname, params.portnumberpds, NMXP_DATA_TIMESERIES);
	nmxp_chan_print_channelList(channelList);

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
    }

    /* Free the complete channel list */
    if(channelList) {
	free(channelList);
	channelList = NULL;
    }


    /* TODO condition starting DAP */
    if(params.start_time != 0   &&   params.end_time != 0) {

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

	/* Start loop for sending requests */
	i_chan=0;
	request_SOCKET_OK = NMXP_SOCKET_OK;

	while(request_SOCKET_OK == NMXP_SOCKET_OK  &&  i_chan < channelList_subset->number) {

	    /* DAP Step 5: Send Data Request */
	    request_SOCKET_OK = nmxp_sendDataRequest(naqssock, channelList_subset->channel[i_chan].key, params.start_time, params.end_time);

	    if(request_SOCKET_OK == NMXP_SOCKET_OK) {

		if(params.flag_writefile) {
		    /* Open output file */
		    sprintf(filename, "%s.%s.%d.%d.%d.nmx", (params.network)? params.network : DEFAULT_NETWORK, channelList_subset->channel[i_chan].name, channelList_subset->channel[i_chan].key, params.start_time, params.end_time);

		    outfile = fopen(filename, "w");

		    if(!outfile) {
			nmxp_log(1, 0, "Can not to open file %s!", filename);
		    }
		}

#ifdef HAVE_LIBMSEED
		if(params.flag_writeseed) {
		    /* Open output Mini-SEED file */
		    sprintf(data_seed.filename_mseed, "%s.%s.%d.%d.%d.miniseed", (params.network)? params.network : DEFAULT_NETWORK, channelList_subset->channel[i_chan].name, channelList_subset->channel[i_chan].key, params.start_time, params.end_time);

		    data_seed.outfile_mseed = fopen(data_seed.filename_mseed, "w");

		    if(!data_seed.outfile_mseed) {
			nmxp_log(1, 0, "Can not to open file %s!", data_seed.filename_mseed);
		    }
		}
#endif

		if(params.flag_writefile  &&  outfile) {
		    /* Compute SNCL line */

		    /* Separate station_code and channel_code */
		    station_code = NULL;
		    channel_code = NULL;

		    station_code = strdup(channelList_subset->channel[i_chan].name);
		    if ( (channel_code = strchr(station_code, '.')) == NULL ) {
			nmxp_log(1,0, "Channel name not in STA.CHAN format: %s\n", station_code);
		    }     
		    if(channel_code) {
			*channel_code++ = '\0'; 
		    }     

		    if(station_code) {
			free(station_code);
		    }

		    /* Write SNCL line */
		    fprintf(outfile, "%s.%s.%s.%s\n", station_code, (params.network)? params.network : DEFAULT_NETWORK, channel_code, (params.location)? params.location : "");
		}

		/* DAP Step 6: Receive Data until receiving a Ready message */
		ret = nmxp_receiveMessage(naqssock, &type, &buffer, &length);
		nmxp_log(0, 1, "ret = %d, type = %d\n", ret, type);

		while(ret == NMXP_SOCKET_OK   &&    type != NMXP_MSG_READY) {

		    /* Process a packet and return value in NMXP_DATA_PROCESS structure */
		    pd = nmxp_processCompressedDataFunc(buffer, length, channelList_subset);

#ifdef HAVE_LIBMSEED
		    /* Write Mini-SEED record */
		    if(params.flag_writeseed) {

			nmxp_data_msr_pack(pd, &data_seed);

		    }
#endif

#ifdef HAVE___SRC_SEEDLINK_PLUGIN_C
		    /* Send data to SeedLink Server */
		    if(params.flag_writeseedlink) {

			/* TODO Set values */
			const int usec_correction = 0;
			const int timing_quality = 100;

			send_raw_depoch(pd->station, pd->channel, pd->time, usec_correction, timing_quality,
				pd->pDataPtr, pd->nSamp);

		    }
#endif

		    /* Log contents of last packet */
		    nmxp_data_log(pd);

		    if(params.flag_writefile  &&  outfile) {
			/* Write buffer to the output file */
			if(outfile && buffer && length > 0) {
			    int length_int = length;
			    nmxp_data_swap_4b((int32_t *) &length_int);
			    fwrite(&length_int, sizeof(length_int), 1, outfile);
			    fwrite(buffer, length, 1, outfile);
			}
		    }

		    if(buffer) {
			free(buffer);
		    }

		    /* Receive Data */
		    ret = nmxp_receiveMessage(naqssock, &type, &buffer, &length);
		    nmxp_log(0, 1, "ret = %d, type = %d\n", ret, type);
		}

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

	    }
	    i_chan++;
	}
	/* DAP Step 7: Repeat steps 5 and 6 for each data request */

	/* DAP Step 8: Send a Terminate message (optional) */
	nmxp_sendTerminateSubscription(naqssock, NMXP_SHUTDOWN_NORMAL, "Bye!");

	/* DAP Step 9: Close the socket */
	nmxp_closeSocket(naqssock);

	/* ************************************************************ */
	/* End subscription protocol "DATA ACCESS PROTOCOL" version 1.0 */
	/* ************************************************************ */

    } else {

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

	while(1) {
	    /* Process Compressed or Decompressed Data */
	    nmxp_receiveData(naqssock, channelList_subset, &nmxp_data_log);
	}

	/* PDS Step 7: Send Terminate Subscription */
	nmxp_sendTerminateSubscription(naqssock, NMXP_SHUTDOWN_NORMAL, "Good Bye!");

	/* PDS Step 8: Close the socket */
	nmxp_closeSocket(naqssock);

	/* *********************************************************** */
	/* End subscription protocol "PRIVATE DATA STREAM" version 1.4 */
	/* *********************************************************** */

    }


    return 0;
}






