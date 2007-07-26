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
    return nmxp_sendMessage(isock, NMXP_MSG_CONNECT, NULL, 0);
}

int nmxp_sendTerminateSubscription(int isock, NMXP_SHUTDOWN_REASON reason, char *message) {
    return nmxp_sendMessage(isock, NMXP_MSG_TERMINATESUBSCRIPTION, message, ((message)? strlen(message)-1 : 0));
}

int nmxp_receiveChannelList(int isock, NMXP_CHAN_LIST **pchannelList) {
    int ret;
    int i;

    NMXP_MSG_SERVER type;
    void *buffer;
    uint32_t length;

    *pchannelList = NULL;

    ret = nmxp_receiveMessage(isock, &type, &buffer, &length);

    if(type != NMXP_MSG_CHANNELLIST) {
	nmxp_log(1, 0, "Type %d is not NMXP_MSG_CHANNELLIST!\n", type);
    } else {

	*pchannelList = buffer;
	(*pchannelList)->number = ntohl((*pchannelList)->number);

	nmxp_log(0, 1, "number of channels %d\n", (*pchannelList)->number);
	
	// TODO check

	for(i=0; i < (*pchannelList)->number; i++) {
	    (*pchannelList)->channel[i].key = ntohl((*pchannelList)->channel[i].key);
	    nmxp_log(0, 1, "%12d %s\n", (*pchannelList)->channel[i].key, (*pchannelList)->channel[i].name);
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

    ret = nmxp_sendMessage(isock, NMXP_MSG_ADDTIMESERIESCHANNELS, buffer, buffer_length);

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

    if(type != NMXP_MSG_COMPRESSED) {
	nmxp_log(1, 0, "Type %d is not NMXP_MSG_COMPRESSED!\n", type);
    } else {
	nmxp_processCompressedData(buffer, length, channelList);
    }

    if(buffer) {
	free(buffer);
    }

    return ret;
}


int nmxp_receiveDecompressedData(int isock, NMXP_CHAN_LIST *channelList) {
    int ret;

    NMXP_MSG_SERVER type;
    void *buffer;
    uint32_t length;

    ret = nmxp_receiveMessage(isock, &type, &buffer, &length);

    if(type != NMXP_MSG_DECOMPRESSED) {
	nmxp_log(1, 0, "Type %d is not NMXP_MSG_DECOMPRESSED!\n", type);
    } else {

	nmxp_processDecompressedData(buffer, length, channelList);
    }

    if(buffer) {
	free(buffer);
    }

    return ret;
}


int nmxp_sendConnectRequest(int isock, char *naqs_username, char *naqs_password, uint32_t connection_time) {
    int ret;
    char crc32buf[100];
    NMXP_CONNECT_REQUEST connectRequest;

    strcpy(connectRequest.username, naqs_username);
    connectRequest.version = htonl(0);
    connectRequest.connection_time = htonl(connection_time);

    if(strlen(naqs_username) == 0  &&  strlen(naqs_password) == 0 ) {
	sprintf(crc32buf, "%d%d", connectRequest.version, connection_time);
    } else if(strlen(naqs_username) != 0  &&  strlen(naqs_password) != 0 ) {
	sprintf(crc32buf, "%s%d%d%s", naqs_username, connectRequest.version,
		connection_time, naqs_password);
    } else if(strlen(naqs_username) != 0 ) {
	sprintf(crc32buf, "%s%d%d", naqs_username, connectRequest.version, connection_time);
    } else if(strlen(naqs_password) != 0 ) {
	sprintf(crc32buf, "%d%d%s", connectRequest.version, connection_time, naqs_password);
    }
    connectRequest.crc32 = htonl(crc32((unsigned char *) crc32buf, strlen(crc32buf)));

    ret = nmxp_sendMessage(isock, NMXP_MSG_CONNECTREQUEST, &connectRequest, sizeof(NMXP_CONNECT_REQUEST));

    if(ret == NMXP_SOCKET_OK) {
	nmxp_log(0,0, "Send a ConnectRequest crc32buf = (%s), crc32 = %d\n", crc32buf, crc32);
    } else {
	nmxp_log(1,0, "Send a ConnectRequest.\n");
    }

    return ret;
}


int nmxp_readConnectionTime(int isock, uint32_t *connection_time) {
    int ret;
    ret = nmxp_recv_ctrl(isock, connection_time, sizeof(connection_time));
    *connection_time = ntohl(*connection_time);
    nmxp_log(0,0, "Read connection time from socket %d.\n", *connection_time);
    if(ret != NMXP_SOCKET_OK) {
	nmxp_log(1,0, "Read connection time from socket.\n");
    }
    return ret;
}


int nmpx_waitReady(int isock) {
    int times = 0;
    int rc = NMXP_SOCKET_OK;
    unsigned long signature;
    unsigned long type = 0;
    unsigned long length;

    while(rc == NMXP_SOCKET_OK  &&  type != NMXP_MSG_READY) {
	rc = nmxp_recv_ctrl(isock, &signature, sizeof(signature));
	if(rc != NMXP_SOCKET_OK) return rc;
	signature = ntohl(signature);
	if(signature == 0) {
	    nmxp_log(0, 0, "signature is equal to zero. receive again.\n");
	    rc = nmxp_recv_ctrl(isock, &signature, sizeof(signature));
	    signature = ntohl(signature);
	}
	if(signature != NMX_SIGNATURE) {
	    nmxp_log(1, 0, "signature is not valid. signature = %d\n", signature);
	    return NMXP_SOCKET_ERROR;
	}

	rc = nmxp_recv_ctrl(isock, &type, sizeof(type));
	if(rc != NMXP_SOCKET_OK) return rc;
	type = ntohl(type);
	if(type != NMXP_MSG_READY) {
	    nmxp_log(0, 0, "type is not READY. type = %d\n", type);
	    rc = nmxp_recv_ctrl(isock, &length, sizeof(length));
	    if(rc != NMXP_SOCKET_OK) return rc;
	    length = ntohl(length);
	    if(length > 0) {
		if(length == 4) {
		    int32_t app;
		    rc = nmxp_recv_ctrl(isock, &app, length);
		    if(rc != NMXP_SOCKET_OK) return rc;
		    app = ntohl(app);
		    nmxp_log(0, 1, "value = %d\n", app);
		} else {
		    char *buf_app = (char *) malloc(sizeof(char) * length);
		    rc = nmxp_recv_ctrl(isock, buf_app, length);
		    if(buf_app) {
			free(buf_app);
		    }
		}
	    }
	} else {
	    rc = nmxp_recv_ctrl(isock, &length, sizeof(length));
	    if(rc != NMXP_SOCKET_OK) return rc;
	    length = ntohl(length);
	    if(length != 0) {
		nmxp_log(1, 0, "length is not equal to zero. length = %d\n", length);
		return NMXP_SOCKET_ERROR;
	    }
	}

	times++;
	if(times > 10) {
	    nmxp_log(1, 0, "waiting_ready_message. times > 10\n");
	    rc = NMXP_SOCKET_ERROR;
	}

    }

    return rc;
}


int nmxp_sendDataRequest(int isock, uint32_t key, uint32_t start_time, uint32_t end_time) {
    int ret;
    NMXP_DATA_REQUEST dataRequest;

    dataRequest.chan_key = htonl(key);
    dataRequest.start_time = htonl(start_time);
    dataRequest.end_time = htonl(end_time);

    ret = nmxp_sendMessage(isock, NMXP_MSG_DATAREQUEST, &dataRequest, sizeof(dataRequest));

    if(ret != NMXP_SOCKET_OK) {
	nmxp_log(1,0, "Send a Request message\n");
    }

    return ret;
}


NMXP_CHAN_LIST *nmxp_getAvailableChannelList(char * hostname, int portnum, NMXP_DATATYPE datatype) {
    int naqssock;
    NMXP_CHAN_LIST *channelList = NULL, *channelList_subset = NULL;
    int i;

    // 1. Open a socket
    naqssock = nmxp_openSocket(hostname, portnum);

    if(naqssock != NMXP_SOCKET_ERROR) {

	// 2. Send a Connect
	if(nmxp_sendConnect(naqssock) == NMXP_SOCKET_OK) {

	    // 3. Receive ChannelList
	     if(nmxp_receiveChannelList(naqssock, &channelList) == NMXP_SOCKET_OK) {

		 channelList_subset = nmxp_chan_getType(channelList, datatype);
		 nmxp_log(0, 1, "%d / %d\n", channelList_subset->number, channelList->number);

		 // nmxp_chan_sortByKey(channelList_subset);
		 nmxp_chan_sortByName(channelList_subset);

		 for(i=0; i < channelList_subset->number; i++) {
		     nmxp_log(0, 1, "%12d %s\n", channelList_subset->channel[i].key, channelList_subset->channel[i].name);
		 }

		 // 4. Send a Request Pending (optional)

		 // 5. Send AddChannels

		 // 6. Repeat until finished: receive and handle packets

		 // 7. Send Terminate Subscription
		 nmxp_sendTerminateSubscription(naqssock, NMXP_SHUTDOWN_NORMAL, "Good Bye!");

	     } else {
		 printf("Error on receiveChannelList()\n");
	     }
	} else {
	    printf("Error on sendConnect()\n");
	}

	// 8. Close the socket
	nmxp_closeSocket(naqssock);
    }

    if(channelList) {
	free(channelList);
    }

    return channelList_subset;
}

