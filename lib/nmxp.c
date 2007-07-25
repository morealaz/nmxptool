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
#include <string.h>


int sendConnect(int isock) {
    return sendMessage(isock, NMXPMSG_CONNECT, NULL, 0);
}

int sendTerminateSubscription(int isock, enum NMXP_REASON_SHUTDOWN reason, char *message) {
    return sendMessage(isock, NMXPMSG_TERMINATESUBSCRIPTION, message, ((message)? strlen(message)-1 : 0));
}

int receiveChannelList(int isock, ChannelList **pchannelList) {
    int ret;
    int i;

    enum NMXP_MSG_SERVER type;
    void *buffer;
    uint32_t length;

    *pchannelList = NULL;

    ret = receiveMessage(isock, &type, &buffer, &length);

    if(type != NMXPMSG_CHANNELLIST) {
	gen_log(1, 0, "Type %d is not NMXPMSG_CHANNELLIST!\n", type);
    } else {

	*pchannelList = buffer;
	(*pchannelList)->number = ntohl((*pchannelList)->number);

	gen_log(0, 1, "number of channels %d\n", (*pchannelList)->number);
	
	// TODO check

	for(i=0; i < (*pchannelList)->number; i++) {
	    (*pchannelList)->channel[i].key = ntohl((*pchannelList)->channel[i].key);
	    gen_log(0, 1, "%d %s\n", (*pchannelList)->channel[i].key, (*pchannelList)->channel[i].name);
	}

    }

    return ret;
}


