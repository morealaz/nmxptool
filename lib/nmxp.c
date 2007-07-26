/*! \file
 *
 * \brief Nanometrics Protocol Libray
 *
 * Author:
 * 	Matteo Quintiliani
 * 	Istituto Nazionale di Geofisica e Vulcanologia - Italy
 *	quintiliani@ingv.it
 *
 */

#include "nmxp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int nmxp_sendConnect(int isock) {
    return nmxp_sendMessage(isock, NMXPMSG_CONNECT, NULL, 0);
}

int nmxp_sendTerminateSubscription(int isock, NMXP_REASON_SHUTDOWN reason, char *message) {
    return nmxp_sendMessage(isock, NMXPMSG_TERMINATESUBSCRIPTION, message, ((message)? strlen(message)-1 : 0));
}

int nmxp_receiveChannelList(int isock, NMXP_CHAN_LIST **pchannelList) {
    int ret;
    int i;

    NMXP_MSG_SERVER type;
    void *buffer;
    uint32_t length;

    *pchannelList = NULL;

    ret = nmxp_receiveMessage(isock, &type, &buffer, &length);

    if(type != NMXPMSG_CHANNELLIST) {
	nmxp_log(1, 0, "Type %d is not NMXPMSG_CHANNELLIST!\n", type);
    } else {

	*pchannelList = buffer;
	(*pchannelList)->number = ntohl((*pchannelList)->number);

	nmxp_log(0, 1, "number of channels %d\n", (*pchannelList)->number);
	
	// TODO check

	for(i=0; i < (*pchannelList)->number; i++) {
	    (*pchannelList)->channel[i].key = ntohl((*pchannelList)->channel[i].key);
	    nmxp_log(0, 1, "%d %s\n", (*pchannelList)->channel[i].key, (*pchannelList)->channel[i].name);
	}

    }

    return ret;
}


int nmxp_sendAddTimeSeriesChannel(int isock, NMXP_CHAN_LIST *channelList, uint32_t shortTermCompletion, uint32_t out_format, NMXP_BUFFER_FLAG buffer_flag) {
    int ret;
    uint32_t buffer_length = 16 + (4 * channelList->number); 
    char *buffer = malloc(buffer_length);
    uint32_t app, i, disp;

    disp=0;

    app = htonl(channelList->number);
    memcpy(&buffer[disp], &app, 4);
    disp+=4;

    for(i=0; i < channelList->number; i++) {
	app = htonl(channelList->channel[i].key);
	memcpy(&buffer[disp], &app, 4);
	disp+=4;
    }
    
    app = htonl(shortTermCompletion);
    memcpy(&buffer[disp], &app, 4);
    disp+=4;

    app = htonl(out_format);
    memcpy(&buffer[disp], &app, 4);
    disp+=4;

    app = htonl(buffer_flag);
    memcpy(&buffer[disp], &app, 4);
    disp+=4;

    ret = nmxp_sendMessage(isock, NMXPMSG_ADDTIMESERIESCHANNELS, buffer, buffer_length);

    if(buffer) {
	free(buffer);
    }
    return ret;
}


int nmxp_receiveCompressedData(int isock, NMXP_CHAN_LIST *channelList) {
    int ret;

    NMXP_MSG_SERVER type;
    void *buffer;
    uint32_t length;

    ret = nmxp_receiveMessage(isock, &type, &buffer, &length);

    if(type != NMXPMSG_COMPRESSED) {
	nmxp_log(1, 0, "Type %d is not NMXPMSG_COMPRESSED!\n", type);
    } else {

	nmxp_processCompressedData(buffer, length, channelList);
    }

    return ret;
}


int nmxp_receiveDecompressedData(int isock, NMXP_CHAN_LIST *channelList) {
    int ret;

    NMXP_MSG_SERVER type;
    void *buffer;
    uint32_t length;

    ret = nmxp_receiveMessage(isock, &type, &buffer, &length);

    if(type != NMXPMSG_DECOMPRESSED) {
	nmxp_log(1, 0, "Type %d is not NMXPMSG_DECOMPRESSED!\n", type);
    } else {

	nmxp_processDecompressedData(buffer, length, channelList);
    }

    return ret;
}

