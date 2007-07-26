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

#include <qlib2.h>

/* private variables */
int my_order = -1;


int nmxp_sendConnect(int isock) {
    return nmxp_sendMessage(isock, NMXPMSG_CONNECT, NULL, 0);
}

int nmxp_sendTerminateSubscription(int isock, enum NMXP_REASON_SHUTDOWN reason, char *message) {
    return nmxp_sendMessage(isock, NMXPMSG_TERMINATESUBSCRIPTION, message, ((message)? strlen(message)-1 : 0));
}

int nmxp_receiveChannelList(int isock, nmxp_ChannelList **pchannelList) {
    int ret;
    int i;

    enum NMXP_MSG_SERVER type;
    void *buffer;
    uint32_t length;

    *pchannelList = NULL;

    ret = nmxp_receiveMessage(isock, &type, &buffer, &length);

    if(type != NMXPMSG_CHANNELLIST) {
	nmxp_gen_log(1, 0, "Type %d is not NMXPMSG_CHANNELLIST!\n", type);
    } else {

	*pchannelList = buffer;
	(*pchannelList)->number = ntohl((*pchannelList)->number);

	nmxp_gen_log(0, 1, "number of channels %d\n", (*pchannelList)->number);
	
	// TODO check

	for(i=0; i < (*pchannelList)->number; i++) {
	    (*pchannelList)->channel[i].key = ntohl((*pchannelList)->channel[i].key);
	    nmxp_gen_log(0, 1, "%d %s\n", (*pchannelList)->channel[i].key, (*pchannelList)->channel[i].name);
	}

    }

    return ret;
}


int nmxp_sendAddTimeSeriesChannel(int isock, nmxp_ChannelList *channelList, uint32_t shortTermCompletion, uint32_t out_format, enum NMXP_BUFFER_FLAG buffer_flag) {
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


/***********************************************************************
 *  unpack_bundle:                                                     
 *      Unpack a 17-byte Nanometrics compressed data bundle.           
 *      Return the number of unpacked data samples, -1 if null bundle. 
 *
 * Author:  Doug Neuhauser
 *          UC Berkeley Seismological Laboratory
 *          doug@seismo.berkeley.edu
 ************************************************************************/
int unpack_bundle (int *outdata, unsigned char *indata, int *prev)
{         
	int nsamples = 0;
	int d4[4];
	short int d2[2];
	int cb[4];  
	int i, j, k;
	unsigned char cbits;

	cbits = (unsigned char)indata[0];
	if (cbits == 9) return (-1);
	++indata;

	/* Get the compression bits for the bundle. */
	for (i=0,j=6; j>=0; i++,j-=2) {
		cb[i] = (cbits>>j) & 3;
	}       

	for (j=0; j<4; j++) {
		/*
		nmxp_gen_log (0,0, "cb[%d]=%d\n", j, cb[j]);
		*/
		switch (cb[j]) 
		{   
			case 0:       /* not used     */
				k=0;
				break;       
			case 1:       /* 4 byte diffs */
				d4[0] = (signed char)indata[0];
				d4[1] = (signed char)indata[1];
				d4[2] = (signed char)indata[2];
				d4[3] = (signed char)indata[3];
				k=4;
				break;
			case 2:       /* 2 16-bit diffs */
				memcpy (&d2[0],indata,2);
				memcpy (&d2[1],indata+2,2);
				if (my_order != SEED_LITTLE_ENDIAN) {
					swab2 (&d2[0]);
					swab2 (&d2[1]);
				}
				d4[0] = d2[0];
				d4[1] = d2[1];
				k=2;
				break;
			case 3:       /* 1 32-bit diff */
				memcpy (&d4[0],indata,4);
				if (my_order != SEED_LITTLE_ENDIAN) {
					swab4 (&d4[0]);
				}
				k=1;
				break;
		}
		indata += 4;

		for (i=0; i<k; i++) {
			*outdata = *prev + d4[i];
			/*
				nmxp_gen_log(0,0, "val = %d, diff[%d] = %d, *prev = %d\n",
						*outdata, i, d4[i], *prev);
						*/
			*prev = *outdata;

			outdata++;
			++nsamples;
		}
	}
	return (nsamples);
}

/**************************************************************************
 *
 *   Function:  lookupnmxp_ChannelKey
 *
 *     Purpose:  looks up a channel key in the nmxp_ChannelList using the name
 *
 *      ------------------------------------------------------------------------*/
int lookupnmxp_ChannelKey(char* name, nmxp_ChannelList *channelList)
{
    int length = channelList->number;
    int ich = 0;

    for (ich = 0; ich < length; ich++)
    {
	if (strcasecmp(name, channelList->channel[ich].name) == 0)
	    return channelList->channel[ich].key;
    }

    return -1;
}

/**************************************************************************
 *
 *   Function:  lookupChannelName
 *
 *     Purpose:  looks up a channel name in the nmxp_ChannelList using a key
 *
 *      ------------------------------------------------------------------------*/
char *lookupChannelName(int key, nmxp_ChannelList *channelList)
{
    int length = channelList->number;
    int ich = 0;

    for (ich = 0; ich < length; ich++)
    {
	if ( key == channelList->channel[ich].key )
	    return &channelList->channel[ich].name[0];
    }

    return NULL;
}

/**************************************************************************

  Function:  processData

  Purpose:  processes compressed data from the server and sends them
            to the controlling SeedLink server.

 ------------------------------------------------------------------------*/
void nmxp_processDecompressedData(char* buffer, int length, nmxp_ChannelList *channelList)
{
  int32_t   netInt    = 0;
  int32_t   pKey      = 0;
  double    pTime     = 0.0;
  int32_t   pNSamp    = 0;
  int32_t   pSampRate = 0;
  int32_t  *pDataPtr  = 0;
  int       swap      = 0;
  int       idx;

  char *sta = 0;      /* The station code */
  char *chan = 0;     /* The channel code */

  /* copy the header contents into local fields and swap */
  memcpy(&netInt, &buffer[0], 4);
  pKey = ntohl(netInt);
  if ( pKey != netInt ) { swap = 1; }

  memcpy(&pTime, &buffer[4], 8);
  if ( swap ) { swab8(&pTime); }

  memcpy(&netInt, &buffer[12], 4);
  pNSamp = ntohl(netInt);
  memcpy(&netInt, &buffer[16], 4);
  pSampRate = ntohl(netInt);

  /* There should be (length - 20) bytes of data as 32-bit ints here */
  pDataPtr = (int32_t *) &buffer[20];

  /* Swap the data samples to host order */
  for ( idx=0; idx < pNSamp; idx++ ) {
      netInt = ntohl(pDataPtr[idx]);
      pDataPtr[idx] = netInt;
  }

  /* Lookup the station and channel code */
  sta = strdup(lookupChannelName(pKey, channelList));
  if ( (chan = strchr(sta, '.')) == NULL ) {
    nmxp_gen_log(1,0, "Channel name not in STN.CHAN format: %s\n", sta);
    free(sta);
    return;
  }
  *chan++ = '\0';
  
  /* Send it off to the controlling SeedLink server */
  /*
  if ( send_raw_depoch(sta, chan, pTime, 0, 100, pDataPtr, pNSamp) < 0 ) {
    nmxp_gen_log(1,0, "cannot send data to seedlink: %s", strerror(errno));
    exit(1);
  }
  */

  nmxp_gen_log(0, 0, "%10d %5s_%3s %10.4f, %04d, %04d, %03d\n", pKey, sta, chan, pTime, length, pNSamp, pSampRate);
  
  /* print out header and/or data for different packet types */
  nmxp_gen_log(0,1, "Received uncompressed data for stream %ld (%s_%s)\n",
	  pKey, sta, chan);
  nmxp_gen_log(0,1, "  length: %d, nsamp: %d, samprate: %d, time: %f\n",
	  length, pNSamp, pSampRate, pTime);
  
  free(sta);
}

void nmxp_processCompressedData(char* buffer_data, int length_data, nmxp_ChannelList *channelList)
{
    int32_t   pKey      = 0;
    double    pTime     = 0.0;
    int32_t   pNSamp    = 0;
    int32_t   pSampRate = 0;
    int32_t  *pDataPtr  = 0;

    char *sta = 0;      /* The station code */
    char *chan = 0;     /* The channel code */

    const int nmx_rate_code_to_sample_rate[32] = {
	0,1,2,5,10,20,40,50,
	80,100,125,200,250,500,1000,25,
	120,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0};

	int nmx_oldest_sequence_number;
	char nmx_hdr[25];
	unsigned char nmx_ptype;
	unsigned int nmx_seconds;
	double nmx_seconds_double;
	short int nmx_ticks, nmx_instr_id;
	int nmx_seqno;
	unsigned char nmx_sample_rate;
	int nmx_x0;
	int rate_code, chan_code, this_sample_rate;

	int comp_bytecount;
	unsigned char *indata;
#define MAX_OUTDATA 4096
	int32_t outdata[MAX_OUTDATA];
	int nout, i, k;
	int prev_xn;

	my_order = get_my_wordorder();
	nmxp_gen_log(0, 1, "my_order is %d\n", my_order);

	memcpy(&nmx_oldest_sequence_number, buffer_data, 4);
	nmxp_gen_log(0, 1, "Oldest sequence number = %d\n", nmx_oldest_sequence_number);

	memcpy(nmx_hdr, buffer_data+4, 17);
	/* Decode the Nanometrics packet header bundle. */
	memcpy (&nmx_ptype, nmx_hdr+0, 1);
	if ( (nmx_ptype & 0xf) == 9) {
	    /* Filler packet.  Discard entire packet.   */
	    nmxp_gen_log (1,0, "Filler packet - discarding\n");
	    //m continue;
	    exit(0);
	}

	nmx_x0 = 0;
	memcpy (&nmx_seconds, nmx_hdr+1, 4);
	memcpy (&nmx_ticks, nmx_hdr+5, 2);
	memcpy (&nmx_instr_id, nmx_hdr+7, 2);
	memcpy (&nmx_seqno, nmx_hdr+9, 4);
	memcpy (&nmx_sample_rate, nmx_hdr+13, 1);
	memcpy (&nmx_x0, nmx_hdr+14, 3);

	const unsigned int high_scale = 4096 * 2048;
	const unsigned int high_scale_p = 4096 * 4096;
	/* check if nmx_x0 is negative like as signed 3-byte int */
	if( (nmx_x0 & high_scale) ==  high_scale) {
	    // nmxp_gen_log(0, 0, "WARNING: changed nmx_x0, old value = %d\n",  nmx_x0);
	    nmx_x0 -= high_scale_p;
	}
	if (my_order != SEED_LITTLE_ENDIAN) {
	    swab4 ((int *)&nmx_seconds);
	    swab2 (&nmx_ticks);
	    swab2 (&nmx_instr_id);
	    swab4 (&nmx_seqno);
	    nmx_x0 = nmx_x0 >> 8;
	    swab4 (&nmx_x0);
	    nmx_x0 = nmx_x0 >> 8;
	}
	nmx_seconds_double = (double) nmx_seconds + ( (double) nmx_ticks / 10000.0 );
	rate_code = nmx_sample_rate>>3;
	chan_code = nmx_sample_rate&7;
	this_sample_rate = nmx_rate_code_to_sample_rate[rate_code];

	nmxp_gen_log(0, 1, "nmx_ptype          = %d\n", nmx_ptype);
	nmxp_gen_log(0, 1, "nmx_seconds        = %d\n", nmx_seconds);
	nmxp_gen_log(0, 1, "nmx_ticks          = %d\n", nmx_ticks);

	nmxp_gen_log(0, 1, "nmx_seconds_double = %f\n", nmx_seconds_double);
	nmxp_gen_log(0, 1, "nmx_x0             = %d\n", nmx_x0);

	nmxp_gen_log(0, 1, "nmx_instr_id       = %d\n", nmx_instr_id);
	nmxp_gen_log(0, 1, "nmx_seqno          = %d\n", nmx_seqno);
	nmxp_gen_log(0, 1, "nmx_sample_rate    = %d\n", nmx_sample_rate);
	nmxp_gen_log(0, 1, "this_sample_rate    = %d\n", this_sample_rate);

	comp_bytecount = length_data-21;
	indata = (unsigned char *) buffer_data + 21;

	/* Unpack the data bundles, each 17 bytes long. */
	prev_xn = nmx_x0;
	outdata[0] = nmx_x0;
	nout = 1;
	for (i=0; i<comp_bytecount; i+=17) {
	    if (i+17>comp_bytecount) {
		nmxp_gen_log (1,0, "comp_bytecount = %d, i+17 = %d\n",
			comp_bytecount, i+17);
		exit(1);
	    }
	    if (nout+16 > MAX_OUTDATA)  {
		nmxp_gen_log (1,0, "Output buffer size too small\n");
		exit(1);
	    }
	    k = unpack_bundle (outdata+nout,indata+i,&prev_xn);
	    if (k < 0) nmxp_gen_log (1,0, "Break: %d\n", __LINE__);
	    if (k < 0) break;
	    nout += k;
	    /* prev_xn = outdata[nout-1]; */
	}
	nout--;

	nmxp_gen_log(0, 1, "Unpacked %d samples.\n", nout);

	pKey = (nmx_instr_id << 16) | ( 1 << 8) | ( chan_code);

	pTime = nmx_seconds_double;

	pDataPtr = outdata;

	pNSamp = nout;

	/* Lookup the station and channel code */
	sta = strdup(lookupChannelName(pKey, channelList));
	if ( (chan = strchr(sta, '.')) == NULL ) {
	    nmxp_gen_log(1,0, "Channel name not in STN.CHAN format: %s\n", sta);
	    free(sta);
	    return;
	}
	*chan++ = '\0';

	nmxp_gen_log(0, 1, "Channel key %d for %s.%s\n", pKey, sta, chan);

	/* Send it off to the controlling SeedLink server */
	/*
	  f ( send_raw_depoch(sta, chan, pTime, 0, 100, pDataPtr, pNSamp) < 0 ) {
	    nmxp_gen_log(1,0, "cannot send data to seedlink: %s", strerror(errno));
	    exit(1);
	}
	*/

	nmxp_gen_log(0, 0, "%10d %5s_%3s %10.4f, %04d, %04d, %03d\n", pKey, sta, chan, pTime, length_data, pNSamp, pSampRate);

	/* print out header and/or data for different packet types */
	nmxp_gen_log(0,2, "Received compressed data for stream %ld (%s_%s)\n",
		pKey, sta, chan);
	nmxp_gen_log(0,2, "  length: %d, nsamp: %d, samprate: %d, time: %f\n",
		length_data, pNSamp, pSampRate, pTime);

	free(sta);
}

int nmxp_receiveCompressedData(int isock, nmxp_ChannelList *channelList) {
    int ret;

    enum NMXP_MSG_SERVER type;
    void *buffer;
    uint32_t length;

    ret = nmxp_receiveMessage(isock, &type, &buffer, &length);

    if(type != NMXPMSG_COMPRESSED) {
	nmxp_gen_log(1, 0, "Type %d is not NMXPMSG_COMPRESSED!\n", type);
    } else {

	nmxp_processCompressedData(buffer, length, channelList);
    }

    return ret;
}

int nmxp_receiveDecompressedData(int isock, nmxp_ChannelList *channelList) {
    int ret;

    enum NMXP_MSG_SERVER type;
    void *buffer;
    uint32_t length;

    ret = nmxp_receiveMessage(isock, &type, &buffer, &length);

    if(type != NMXPMSG_DECOMPRESSED) {
	nmxp_gen_log(1, 0, "Type %d is not NMXPMSG_DECOMPRESSED!\n", type);
    } else {

	nmxp_processDecompressedData(buffer, length, channelList);
    }

    return ret;
}

