/*! \file
 *
 * \brief Nanometrics Protocol Library
 *
 * Author:
 * 	Matteo Quintiliani
 * 	Istituto Nazionale di Geofisica e Vulcanologia - Italy
 *	quintiliani@ingv.it
 *
 * $Id: nmxp.c,v 1.45 2007-09-17 13:00:05 mtheo Exp $
 *
 */

#include "nmxp.h"

#include "config.h"

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
    int32_t length;

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


int nmxp_sendAddTimeSeriesChannel(int isock, NMXP_CHAN_LIST *channelList, int32_t shortTermCompletion, int32_t out_format, NMXP_BUFFER_FLAG buffer_flag) {
    int ret;
    int32_t buffer_length = 16 + (4 * channelList->number); 
    char *buffer = malloc(buffer_length);
    int32_t app, i, disp;

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


NMXP_DATA_PROCESS *nmxp_receiveData(int isock, NMXP_CHAN_LIST *channelList, const char *network_code) {
    NMXP_MSG_SERVER type;
    void *buffer = NULL;
    int32_t length;
    NMXP_DATA_PROCESS *pd = NULL;

    if(nmxp_receiveMessage(isock, &type, &buffer, &length) == NMXP_SOCKET_OK) {
	if(type == NMXP_MSG_COMPRESSED) {
	    nmxp_log(0, 1, "Type %d is NMXP_MSG_COMPRESSED!\n", type);
	    pd = nmxp_processCompressedData(buffer, length, channelList, network_code);
	} else if(type == NMXP_MSG_DECOMPRESSED) {
	    nmxp_log(0, 1, "Type %d is NMXP_MSG_DECOMPRESSED!\n", type);
	    pd = nmxp_processDecompressedData(buffer, length, channelList, network_code);
	} else {
	    nmxp_log(1, 0, "Type %d is not NMXP_MSG_COMPRESSED or NMXP_MSG_DECOMPRESSED!\n", type);
	}
    }

    return pd;
}


int nmxp_sendConnectRequest(int isock, char *naqs_username, char *naqs_password, int32_t connection_time) {
    int ret;
    char crc32buf[100];
    NMXP_CONNECT_REQUEST connectRequest;
    int naqs_username_length, naqs_password_length;

    naqs_username_length = (naqs_username)? strlen(naqs_username) : 0;
    naqs_password_length = (naqs_password)? strlen(naqs_password) : 0;

    if(naqs_username_length == 0) {
	connectRequest.username[0] = 0;
    } else {
	strcpy(connectRequest.username, naqs_username);
    }
    connectRequest.version = htonl(0);
    connectRequest.connection_time = htonl(connection_time);

    if(naqs_username_length == 0  &&  naqs_password_length == 0 ) {
	sprintf(crc32buf, "%d%d", connectRequest.version, connection_time);
    } else if(naqs_username_length != 0  &&  naqs_password_length != 0 ) {
	sprintf(crc32buf, "%s%d%d%s", naqs_username, connectRequest.version,
		connection_time, naqs_password);
    } else if(naqs_username_length != 0 ) {
	sprintf(crc32buf, "%s%d%d", naqs_username, connectRequest.version, connection_time);
    } else if(naqs_password_length != 0 ) {
	sprintf(crc32buf, "%d%d%s", connectRequest.version, connection_time, naqs_password);
    }
    connectRequest.crc32 = htonl(crc32((unsigned char *) crc32buf, strlen(crc32buf)));

    ret = nmxp_sendMessage(isock, NMXP_MSG_CONNECTREQUEST, &connectRequest, sizeof(NMXP_CONNECT_REQUEST));

    if(ret == NMXP_SOCKET_OK) {
	nmxp_log(0, 1, "Send a ConnectRequest crc32buf = (%s), crc32 = %d\n", crc32buf, connectRequest.crc32);
    } else {
	nmxp_log(1, 0, "Send a ConnectRequest.\n");
    }

    return ret;
}


int nmxp_readConnectionTime(int isock, int32_t *connection_time) {
    int ret;
    ret = nmxp_recv_ctrl(isock, connection_time, sizeof(int32_t));
    *connection_time = ntohl(*connection_time);
    nmxp_log(0, 1, "Read connection time from socket %d.\n", *connection_time);
    if(ret != NMXP_SOCKET_OK) {
	nmxp_log(1, 0, "Read connection time from socket.\n");
    }
    return ret;
}


int nmxp_waitReady(int isock) {
    int times = 0;
    int rc = NMXP_SOCKET_OK;
    int32_t signature;
    int32_t type = 0;
    int32_t length;

    while(rc == NMXP_SOCKET_OK  &&  type != NMXP_MSG_READY) {
	rc = nmxp_recv_ctrl(isock, &signature, sizeof(signature));
	if(rc != NMXP_SOCKET_OK) return rc;
	signature = ntohl(signature);
	if(signature == 0) {
	    nmxp_log(0, 1, "signature is equal to zero. receive again.\n");
	    rc = nmxp_recv_ctrl(isock, &signature, sizeof(signature));
	    signature = ntohl(signature);
	}
	if(signature != NMX_SIGNATURE) {
	    nmxp_log(1, 0, "signature is not valid. signature = %d\n", signature);
	    if(signature == 200) {
		    int32_t err_length;
		    int32_t err_reason;
		    char err_buff[200];
		    rc = nmxp_recv_ctrl(isock, &err_length, sizeof(err_length));
		    err_length = ntohl(err_length);
		    rc = nmxp_recv_ctrl(isock, &err_reason, sizeof(err_reason));
		    err_reason = ntohl(err_reason);
		    if(err_length > 4) {
			    rc = nmxp_recv_ctrl(isock, err_buff, err_length-4);
			    err_buff[err_length] = 0;
		    }
		    nmxp_log(1, 0, "TerminateMessage from Server: %s (%d).\n", err_buff, err_reason);
	    }
	    return NMXP_SOCKET_ERROR;
	}

	rc = nmxp_recv_ctrl(isock, &type, sizeof(type));
	if(rc != NMXP_SOCKET_OK) return rc;
	type = ntohl(type);
	if(type != NMXP_MSG_READY) {
	    nmxp_log(0, 1, "type is not READY. type = %d\n", type);
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


int nmxp_sendDataRequest(int isock, int32_t key, int32_t start_time, int32_t end_time) {
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
		 nmxp_log(1, 0, "Error on receiveChannelList()\n");
	     }
	} else {
	    nmxp_log(1, 0, "Error on sendConnect()\n");
	}

	// 8. Close the socket
	nmxp_closeSocket(naqssock);
    }

    if(channelList) {
	free(channelList);
    }

    return channelList_subset;
}


NMXP_META_CHAN_LIST *nmxp_getMetaChannelList(char * hostname, int portnum, NMXP_DATATYPE datatype, int flag_request_channelinfo) {
    int naqssock;
    NMXP_CHAN_PRECISLIST *precisChannelList = NULL;
    NMXP_CHAN_LIST *channelList = NULL;
    NMXP_META_CHAN_LIST *chan_list = NULL;
    NMXP_META_CHAN_LIST *iter = NULL;
    int i = 0;
    int32_t connection_time;
    char *datas_username = NULL, *datas_password = NULL;
    int ret_sock;

    
    NMXP_MSG_SERVER type;
    void *buffer = NULL;
    int32_t length;
    NMXP_MSGBODY_PRECISLISTREQUEST precisListRequestBody;
    NMXP_MSGBODY_CHANNELINFOREQUEST channelInfoRequestBody;
    NMXP_MSGBODY_CHANNELINFORESPONSE *channelInfo;

    char str_start[200], str_end[200];
    str_start[0] = 0;
    str_end[0] = 0;
    

    /* DAP Step 1: Open a socket */
    if( (naqssock = nmxp_openSocket(hostname, portnum)) == NMXP_SOCKET_ERROR) {
	nmxp_log(1, 0, "Error opening socket!\n");
	return NULL;
    }

    /* DAP Step 2: Read connection time */
    if(nmxp_readConnectionTime(naqssock, &connection_time) != NMXP_SOCKET_OK) {
	nmxp_log(1, 0, "Error reading connection time from server!\n");
	return NULL;
    }

    /* DAP Step 3: Send a ConnectRequest */
    if(nmxp_sendConnectRequest(naqssock, datas_username, datas_password, connection_time) != NMXP_SOCKET_OK) {
	nmxp_log(1, 0, "Error sending connect request!\n");
	return NULL;
    }

    /* DAP Step 4: Wait for a Ready message */
    if(nmxp_waitReady(naqssock) != NMXP_SOCKET_OK) {
	nmxp_log(1, 0, "Error waiting Ready message!\n");
	return NULL;
    }




    /* DAP Step 5: Send Data Request */
    nmxp_sendHeader(naqssock, NMXP_MSG_CHANNELLISTREQUEST, 0);
    /* DAP Step 6: Receive Data until receiving a Ready message */
    ret_sock = nmxp_receiveMessage(naqssock, &type, &buffer, &length);
    nmxp_log(0, 1, "ret_sock = %d, type = %d, length = %d\n", ret_sock, type, length);

    while(ret_sock == NMXP_SOCKET_OK   &&    type != NMXP_MSG_READY) {
	channelList = buffer;

	channelList->number = ntohl(channelList->number);

	for(i = 0; i < channelList->number; i++) {
	    channelList->channel[i].key = ntohl(channelList->channel[i].key);
	    if(getDataTypeFromKey(channelList->channel[i].key) == datatype) {
		nmxp_meta_chan_add(&chan_list, channelList->channel[i].key, channelList->channel[i].name, 0, 0, NULL, NMXP_META_SORT_NAME);
	    }
	}

	/* Receive Message */
	ret_sock = nmxp_receiveMessage(naqssock, &type, &buffer, &length);
	nmxp_log(0, 1, "ret_sock = %d, type = %d, length = %d\n", ret_sock, type, length);
    }


    /* DAP Step 5: Send Data Request */
    precisListRequestBody.instr_id = htonl(-1);
    precisListRequestBody.datatype = htonl(NMXP_DATA_TIMESERIES);
    precisListRequestBody.type_of_channel = htonl(-1);
    nmxp_sendMessage(naqssock, NMXP_MSG_PRECISLISTREQUEST, &precisListRequestBody, sizeof(NMXP_MSGBODY_PRECISLISTREQUEST));


    /* DAP Step 6: Receive Data until receiving a Ready message */
    ret_sock = nmxp_receiveMessage(naqssock, &type, &buffer, &length);
    nmxp_log(0, 1, "ret_sock = %d, type = %d, length = %d\n", ret_sock, type, length);

    while(ret_sock == NMXP_SOCKET_OK   &&    type != NMXP_MSG_READY) {
	precisChannelList = buffer;

	precisChannelList->number = ntohl(precisChannelList->number);
	for(i = 0; i < precisChannelList->number; i++) {
	    precisChannelList->channel[i].key = ntohl(precisChannelList->channel[i].key);
	    precisChannelList->channel[i].start_time = ntohl(precisChannelList->channel[i].start_time);
	    precisChannelList->channel[i].end_time = ntohl(precisChannelList->channel[i].end_time);

	    nmxp_data_to_str(str_start, precisChannelList->channel[i].start_time);
	    nmxp_data_to_str(str_end, precisChannelList->channel[i].end_time);

	    if(!nmxp_meta_chan_set_times(chan_list, precisChannelList->channel[i].key, precisChannelList->channel[i].start_time, precisChannelList->channel[i].end_time)) {
		nmxp_log(1, 0, "Key %d not found for %s!\n", precisChannelList->channel[i].key, precisChannelList->channel[i].name);
	    }

	    /*
	    nmxp_log(0, 0, "%12d %12s %10d %10d %20s %20s\n",
		    precisChannelList->channel[i].key, precisChannelList->channel[i].name,
		    precisChannelList->channel[i].start_time, precisChannelList->channel[i].end_time,
		    str_start, str_end);
		    */
	}

	/* Receive Message */
	ret_sock = nmxp_receiveMessage(naqssock, &type, &buffer, &length);
	nmxp_log(0, 1, "ret_sock = %d, type = %d, length = %d\n", ret_sock, type, length);
    }


    if(flag_request_channelinfo) {
	for(iter = chan_list; iter != NULL; iter = iter->next) {

	    if(getChannelNumberFromKey(iter->key) == 0) {
		/* DAP Step 5: Send Data Request */
		channelInfoRequestBody.key = htonl(iter->key);
		channelInfoRequestBody.ignored = htonl(0);
		nmxp_sendMessage(naqssock, NMXP_MSG_CHANNELINFOREQUEST, &channelInfoRequestBody, sizeof(NMXP_MSGBODY_CHANNELINFOREQUEST));

		/* DAP Step 6: Receive Data until receiving a Ready message */
		ret_sock = nmxp_receiveMessage(naqssock, &type, &buffer, &length);
		nmxp_log(0, 1, "ret_sock = %d, type = %d, length = %d\n", ret_sock, type, length);

		while(ret_sock == NMXP_SOCKET_OK   &&    type != NMXP_MSG_READY) {
		    channelInfo = buffer;
		    channelInfo->key = ntohl(channelInfo->key);

		    if(!nmxp_meta_chan_set_network(chan_list, channelInfo->key, channelInfo->network)) {
			nmxp_log(1, 0, "Key %d (%d) not found for %s!\n", iter->key, channelInfo->key, iter->name);
		    }
		    /* Receive Message */
		    ret_sock = nmxp_receiveMessage(naqssock, &type, &buffer, &length);
		    nmxp_log(0, 1, "ret_sock = %d, type = %d, length = %d\n", ret_sock, type, length);
		}
	    }
	}
    }



    /* DAP Step 7: Repeat steps 5 and 6 for each data request */

    /* DAP Step 8: Send a Terminate message (optional) */
    nmxp_sendTerminateSubscription(naqssock, NMXP_SHUTDOWN_NORMAL, "Bye!");

    /* DAP Step 9: Close the socket */
    nmxp_closeSocket(naqssock);

    nmxp_meta_chan_print(chan_list);

    return chan_list;
}


int nmxp_raw_stream_seq_no_compare(const void *a, const void *b)
{       
    int ret = 0;
    NMXP_DATA_PROCESS **ppa = (NMXP_DATA_PROCESS **) a;
    NMXP_DATA_PROCESS **ppb = (NMXP_DATA_PROCESS **) b;
    NMXP_DATA_PROCESS *pa = *ppa;
    NMXP_DATA_PROCESS *pb = *ppb;

    if(pa && pb) {
	if(pa->seq_no > pb->seq_no) {
	    ret = 1;
	} else if (pa->seq_no < pb->seq_no) {
	    ret = -1;
	}
    } else {
	printf("Error pa and/or pb are NULL!\n");
    }

    return ret;
}

void nmxp_raw_stream_init(NMXP_RAW_STREAM_DATA *raw_stream_buffer, int32_t max_pdlist_items) {
    int j;

    raw_stream_buffer->last_seq_no_sent = -1;
    raw_stream_buffer->last_sample_time = -1.0;
    raw_stream_buffer->max_pdlist_items = max_pdlist_items;
    raw_stream_buffer->n_pdlist = 0;
    raw_stream_buffer->pdlist = (NMXP_DATA_PROCESS **) malloc (raw_stream_buffer->max_pdlist_items * sizeof(NMXP_DATA_PROCESS *));
    for(j=0; j<raw_stream_buffer->max_pdlist_items; j++) {
	raw_stream_buffer->pdlist[j] = NULL;
    }

}


void nmxp_raw_stream_free(NMXP_RAW_STREAM_DATA *raw_stream_buffer) {
    int j;
    if(raw_stream_buffer->pdlist) {
	for(j=0; j<raw_stream_buffer->max_pdlist_items; j++) {
	    if(raw_stream_buffer->pdlist[j]) {
		free(raw_stream_buffer->pdlist[j]);
	    }
	}
	free(raw_stream_buffer->pdlist);
    }
}


int nmxp_raw_stream_manage(NMXP_RAW_STREAM_DATA *p, NMXP_DATA_PROCESS *a_pd, int (*p_func_pd[NMXP_MAX_FUNC_PD]) (NMXP_DATA_PROCESS *), int n_func_pd) {
    int ret = 0;
    int send_again = 1;
    int seq_no_diff;
    double time_diff;
    double latency = 0.0;
    int j=0, k=0;
    int i_func_pd;
    char str_time[200];
    NMXP_DATA_PROCESS *pd = NULL;

    if(a_pd->packet_type == 33) {
	nmxp_data_log(a_pd);
    }

    /* Allocate memory for pd and copy a_pd */
    pd = (NMXP_DATA_PROCESS *) malloc (sizeof(NMXP_DATA_PROCESS));
    memcpy(pd, a_pd, sizeof(NMXP_DATA_PROCESS));
    if(a_pd->length > 0) {
	pd->buffer = malloc(pd->length);
	memcpy(pd->buffer, a_pd->buffer, a_pd->length);
    } else {
	pd->buffer = NULL;
    }
    if(a_pd->nSamp *  sizeof(int) > 0) {
	pd->pDataPtr = (int *) malloc(a_pd->nSamp * sizeof(int));
	memcpy(pd->pDataPtr, a_pd->pDataPtr, a_pd->nSamp * sizeof(int));
    } else {
	pd->pDataPtr = NULL;
    }

    /* First time */
    if(p->last_seq_no_sent == -1) {
	p->last_seq_no_sent = pd->seq_no - 1;
	p->last_sample_time = pd->time;
	nmxp_log(0, 1, "First time nmxp_raw_stream_manage().\n");
    }

    /* Add pd and sort array */
    if(p->n_pdlist >= p->max_pdlist_items) {
	/* Supposing p->pdlist is ordered,
	 * handle the first item and over write it.
	 */
	seq_no_diff = p->pdlist[0]->seq_no - p->last_seq_no_sent;
	time_diff = p->pdlist[0]->time - p->last_sample_time;
	latency = nmxp_data_latency(p->pdlist[0]);
	nmxp_data_to_str(str_time, p->pdlist[0]->time);
	if( seq_no_diff > 0) {
	    nmxp_log(NMXP_LOG_WARN, 0, "Force handling packet %s.%s.%d.%d (%s - %.2f sec.)  time_diff %.2fs  lat. %.1fs!\n",
		    p->pdlist[0]->station, p->pdlist[0]->channel, p->pdlist[0]->seq_no, p->pdlist[0]->packet_type, str_time,
		    (double) p->pdlist[0]->nSamp / (double) p->pdlist[0]->sampRate, time_diff, latency);
	    for(i_func_pd=0; i_func_pd<n_func_pd; i_func_pd++) {
		(*p_func_pd[i_func_pd])(p->pdlist[0]);
	    }
	    p->last_seq_no_sent = (p->pdlist[0]->seq_no);
	    p->last_sample_time = (p->pdlist[0]->time + ((double) p->pdlist[0]->nSamp / (double) p->pdlist[0]->sampRate ));
	} else {
	    /* It should not occur */
	    nmxp_log(NMXP_LOG_WARN, 0, "NOT OCCUR! Packets %s.%s.%d.%d (%s - %.2f sec.) discarded, seq_no_diff=%d time_diff %.2fs  lat. %.1fs\n",
		    p->pdlist[0]->station, p->pdlist[0]->channel, p->pdlist[0]->seq_no, p->pdlist[0]->packet_type, str_time,
		    (double) p->pdlist[0]->nSamp / (double) p->pdlist[0]->sampRate, seq_no_diff, time_diff, latency);
	}

	/* Free handled packet */
	if(p->pdlist[0]->buffer) {
	    free(p->pdlist[0]->buffer);
	    p->pdlist[0]->buffer = NULL;
	}
	if(p->pdlist[0]->pDataPtr) {
	    free(p->pdlist[0]->pDataPtr);
	    p->pdlist[0]->pDataPtr = NULL;
	}
	if(p->pdlist[0]) {
	    free(p->pdlist[0]);
	    p->pdlist[0] = NULL;
	}

	p->pdlist[0] = pd;
    } else {
	p->pdlist[p->n_pdlist++] = pd;
    }
    qsort(p->pdlist, p->n_pdlist, sizeof(NMXP_DATA_PROCESS *), nmxp_raw_stream_seq_no_compare);

    // TODO Check for packet duplication in pd->pdlist

    /* Print array, only for debugging */
    if(p->n_pdlist > 1) {
	int y = 0;
	for(y=0; y < p->n_pdlist; y++) {
	    nmxp_log(0, 1, "%02d pkt %d\n", y, p->pdlist[y]->seq_no);
	}
    }

    // TODO Condition for max tollerable latency

    /* Manage array and execute func_pd() */
    j=0;
    send_again = 1;
    while(send_again  &&  j < p->n_pdlist) {
	send_again = 0;
	seq_no_diff = p->pdlist[j]->seq_no - p->last_seq_no_sent;
	time_diff = p->pdlist[j]->time - p->last_sample_time;
	latency = nmxp_data_latency(p->pdlist[j]);
	nmxp_data_to_str(str_time, p->pdlist[j]->time);
	if(seq_no_diff <= 0) {
	    // Duplicated packets: Discarded
	    nmxp_log(NMXP_LOG_WARN, 0, "Packets %s.%s.%d.%d (%s - %f sec.) discarded, seq_no_diff=%d  time_diff=%.2fs  lat %.1fs\n",
		    p->pdlist[j]->station, p->pdlist[j]->channel, p->pdlist[j]->seq_no, p->pdlist[j]->packet_type, str_time,
		    (double) p->pdlist[j]->nSamp /  (double) p->pdlist[j]->sampRate, seq_no_diff, time_diff, latency);
	    send_again = 1;
	    j++;
	} else if(seq_no_diff == 1) {
	    for(i_func_pd=0; i_func_pd<n_func_pd; i_func_pd++) {
		(*p_func_pd[i_func_pd])(p->pdlist[j]);
	    }
	    if(time_diff > TIME_TOLLERANCE || time_diff < -TIME_TOLLERANCE) {
		nmxp_log(NMXP_LOG_WARN, 0, "%s.%s time is not correct seq_no_diff=%d time_diff=%.2fs  ([%d] %d-%d)  (%s - %.2f sec.) lat. %.1fs\n",
		    p->pdlist[j]->station, p->pdlist[j]->channel, 
		    seq_no_diff, time_diff, p->pdlist[j]->packet_type, p->pdlist[j]->seq_no, p->last_seq_no_sent,
		    str_time, (double) p->pdlist[j]->nSamp /  (double) p->pdlist[j]->sampRate, latency);
	    }
	    p->last_seq_no_sent = p->pdlist[j]->seq_no;
	    p->last_sample_time = (p->pdlist[j]->time + ((double) p->pdlist[j]->nSamp / (double) p->pdlist[j]->sampRate ));
	    send_again = 1;
	    j++;
	} else {
	    nmxp_log(NMXP_LOG_WARN, 0, "%s.%s seq_no_diff=%d ([%d] %d-%d)  j=%2d  p->n_pdlist=%2d (%s - %.2f sec.) time_diff=%.2fs  lat. %.1fs\n",
		    p->pdlist[j]->station, p->pdlist[j]->channel, 
		    seq_no_diff, p->pdlist[j]->packet_type, p->pdlist[j]->seq_no, p->last_seq_no_sent, j, p->n_pdlist,
		    str_time, (double) p->pdlist[j]->nSamp /  (double) p->pdlist[j]->sampRate, time_diff, latency);
	}
    }

    /* Shift and free j handled elements */
    if(j > 0) {
	for(k=0; k < p->n_pdlist; k++) {
	    if(k < j) {
		if(p->pdlist[k]->buffer) {
		    free(p->pdlist[k]->buffer);
		    p->pdlist[k]->buffer = NULL;
		}
		if(p->pdlist[k]->pDataPtr) {
		    free(p->pdlist[k]->pDataPtr);
		    p->pdlist[k]->pDataPtr = NULL;
		}
		if(p->pdlist[k]) {
		    free(p->pdlist[k]);
		    p->pdlist[k] = NULL;
		}
	    }
	    if(k + j < p->n_pdlist) {
		p->pdlist[k] = p->pdlist[k+j];
	    } else {
		p->pdlist[k] = NULL;
	    }
	}
	p->n_pdlist = p->n_pdlist - j;
    }

    nmxp_log(0, 1, "j=%d  p->n_pdlist=%d FINAL\n", j, p->n_pdlist);

    return ret;
}

