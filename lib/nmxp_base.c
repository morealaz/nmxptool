/*! \file
 *
 * \brief Base for Nanometrics Protocol Library
 *
 * Author:
 * 	Matteo Quintiliani
 * 	Istituto Nazionale di Geofisica e Vulcanologia - Italy
 *	quintiliani@ingv.it
 *
 */

#include "nmxp_base.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#include <qlib2.h>


int nmxp_openSocket(char *hostname, int portNum)
{
  static int sleepTime = 1;
  int isock = 0;
  struct hostent *hostinfo = NULL;
  struct sockaddr_in psServAddr;
  struct in_addr hostaddr;
  
  if (!hostname)
  {
    nmxp_log(1,0, "Empty host name?\n");
    return -1;
  }

  if ( (hostinfo = gethostbyname(hostname)) == NULL) {
    nmxp_log(1,0, "Cannot lookup host: %s\n", hostname);
    return -1;
  }

  while(1)
  {
    isock = socket (AF_INET, SOCK_STREAM, 0);
    if (isock < 0)
    {
      nmxp_log (1,0, "Can't open stream socket\n");
      exit(1);
    }

    /* Fill in the structure "psServAddr" with the address of server
       that we want to connect with */
    memset (&psServAddr, 0, sizeof(psServAddr));
    psServAddr.sin_family = AF_INET;
    psServAddr.sin_port = htons((unsigned short) portNum);
    psServAddr.sin_addr = *(struct in_addr *)hostinfo->h_addr_list[0];

    /* Report action and resolved address */
    memcpy(&hostaddr.s_addr, *hostinfo->h_addr_list,
	   sizeof (hostaddr.s_addr));
    nmxp_log(0,1, "Attempting to connect to %s port %d\n",
	    inet_ntoa(hostaddr), portNum);

    if (connect(isock, (struct sockaddr *)&psServAddr, sizeof(psServAddr)) >= 0)
    {
      sleepTime = 1;
      nmxp_log(0, 1, "Connection established: socket=%i,IP=%s,port=%d\n",
	      isock, inet_ntoa(hostaddr), portNum);
      return isock;
    }
    else
    {
      nmxp_log(0, 1, "Trying again later...Sleeping\n");
      close (isock);
      sleep (sleepTime);
      sleepTime *= 2;
      if (sleepTime > NMXP_SLEEPMAX)
        sleepTime = NMXP_SLEEPMAX;
    }
  }
}


int nmxp_closeSocket(int isock)
{
    nmxp_log(0, 1, "Closed connection.\n");
    return close(isock);
}


int nmxp_send_ctrl(int isock, void* buffer, int length)
{
  int sendCount = send(isock, (char*) buffer, length, 0);
  
  if (sendCount != length)
    return NMXP_SOCKET_ERROR;

  return NMXP_SOCKET_OK;
}


int nmxp_recv_ctrl(int isock, void* buffer, int length)
{
  int recvCount;
  int recv_errno;
  char recv_errno_str[200];

  recvCount= recv(isock, (char*) buffer, length, MSG_WAITALL);
  recv_errno  = errno;

  if (recvCount != length) {
	  switch(recv_errno) {
		  case EAGAIN : strcpy(recv_errno_str, "EAGAIN"); break;
		  case EBADF : strcpy(recv_errno_str, "EBADF"); break;
		  case ECONNREFUSED : strcpy(recv_errno_str, "ECONNREFUSED"); break;
		  case EFAULT : strcpy(recv_errno_str, "EFAULT"); break;
		  case EINTR : strcpy(recv_errno_str, "EINTR"); break;
		  case EINVAL : strcpy(recv_errno_str, "EINVAL"); break;
		  case ENOMEM : strcpy(recv_errno_str, "ENOMEM"); break;
		  case ENOTCONN : strcpy(recv_errno_str, "ENOTCONN"); break;
		  case ENOTSOCK : strcpy(recv_errno_str, "ENOTSOCK"); break;
		  default:
			  strcpy(recv_errno_str, "DEFAULT_NO_VALUE");
			  break;
	  }
    nmxp_log(0, 1, "nmxp_recv_ctrl(): (recvCount != length) %d != %d - errno = %d (%s)\n", recvCount, length, recv_errno, recv_errno_str);
	    
    return NMXP_SOCKET_ERROR;
  }
  
  return NMXP_SOCKET_OK;
}


int nmxp_sendHeader(int isock, NMXP_MSG_CLIENT type, uint32_t length)
{  
    NMXP_MESSAGE_HEADER msg;

    msg.signature = htonl(NMX_SIGNATURE);
    msg.type      = htonl(type);
    msg.length    = htonl(length);

    return nmxp_send_ctrl(isock, &msg, sizeof(NMXP_MESSAGE_HEADER));
}


int nmxp_receiveHeader(int isock, NMXP_MSG_SERVER *type, uint32_t *length)
{  
    int ret ;
    NMXP_MESSAGE_HEADER msg;

    ret = nmxp_recv_ctrl(isock, &msg, sizeof(NMXP_MESSAGE_HEADER));

    *type = 0;
    *length = 0;

    if(ret == NMXP_SOCKET_OK) {
	msg.signature = ntohl(msg.signature);
	msg.type      = ntohl(msg.type);
	msg.length    = ntohl(msg.length);

	nmxp_log(0,1, "nmxp_receiveHeader(): signature = %d, type = %d, length = %d\n",
		    msg.signature, msg.type, msg.length);

	if (msg.signature != NMX_SIGNATURE)
	{
	    ret = NMXP_SOCKET_ERROR;
	    nmxp_log(1,0, "nmxp_receiveHeader(): signature mismatches. signature = %d, type = %d, length = %d\n",
		    msg.signature, msg.type, msg.length);
	} else {
	    *type = msg.type;
	    *length = msg.length;
	}
    }

    return ret;
}


int nmxp_sendMessage(int isock, NMXP_MSG_CLIENT type, void *buffer, uint32_t length) {
    int ret;
    ret = nmxp_sendHeader(isock, type, length);
    if( ret == NMXP_SOCKET_OK) {
	if(buffer && length > 0) {
	    ret = nmxp_send_ctrl(isock, buffer, length);
	}
    }
    return ret;
}


int nmxp_receiveMessage(int isock, NMXP_MSG_SERVER *type, void **buffer, uint32_t *length) {
    int ret;
    *buffer = NULL;
    *length = 0;

    ret = nmxp_receiveHeader(isock, type, length);

    if( ret == NMXP_SOCKET_OK) {
	if (*length > 0) {
	    *buffer = malloc(*length);
	    ret = nmxp_recv_ctrl(isock, *buffer, *length);

	    if(*type == NMXP_MSG_ERROR) {
		nmxp_log(1,0, "Received ErrorMessage: %s\n", *buffer);
	    }

	}
    } else {
	nmxp_log(1,0, "Error in nmxp_receiveMessage()\n");
    }

    return ret;
}


NMXP_DATA_PROCESS *nmxp_processDecompressedDataFunc(char* buffer_data, int length_data, NMXP_CHAN_LIST *channelList)
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

  static NMXP_DATA_PROCESS pd;

  /* copy the header contents into local fields and swap */
  memcpy(&netInt, &buffer_data[0], 4);
  pKey = ntohl(netInt);
  if ( pKey != netInt ) { swap = 1; }

  memcpy(&pTime, &buffer_data[4], 8);
  if ( swap ) { swab8(&pTime); }

  memcpy(&netInt, &buffer_data[12], 4);
  pNSamp = ntohl(netInt);
  memcpy(&netInt, &buffer_data[16], 4);
  pSampRate = ntohl(netInt);

  /* There should be (length_data - 20) bytes of data as 32-bit ints here */
  pDataPtr = (int32_t *) &buffer_data[20];

  /* Swap the data samples to host order */
  for ( idx=0; idx < pNSamp; idx++ ) {
      netInt = ntohl(pDataPtr[idx]);
      pDataPtr[idx] = netInt;
  }

  /* Lookup the station and channel code */
  sta = strdup(nmxp_chan_lookupName(pKey, channelList));
  if ( (chan = strchr(sta, '.')) == NULL ) {
    nmxp_log(1,0, "Channel name not in STN.CHAN format: %s\n", sta);
    /*
    free(sta);
    return;
    */
  }
  if(chan) {
      *chan++ = '\0';
  }
  
  nmxp_data_init(&pd);

  pd.key = pKey;
  if(sta) {
      strlcpy(pd.station, sta, STATION_LENGTH);
  }
  if(chan) {
      strlcpy(pd.channel, chan, CHANNEL_LENGTH);
  }
  pd.packet_type = NMXP_MSG_DECOMPRESSED;
  // pd.x0 = ;
  // pd.seq_no = ;
  pd.time = pTime;
  pd.buffer = buffer_data;
  pd.length = length_data;
  pd.nSamp = pNSamp;
  pd.pDataPtr = pDataPtr;
  pd.sampRate = pSampRate;

  free(sta);

  return &pd;
}


NMXP_DATA_PROCESS *nmxp_processCompressedDataFunc(char* buffer_data, int length_data, NMXP_CHAN_LIST *channelList)
{
    int32_t   pKey      = 0;
    double    pTime     = 0.0;
    int32_t   pNSamp    = 0;
    int32_t   pSampRate = 0;
    int32_t  *pDataPtr  = 0;

    char *sta = 0;      /* The station code */
    char *chan = 0;     /* The channel code */

    static NMXP_DATA_PROCESS pd;

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

	int my_order = get_my_wordorder();
	nmxp_log(0, 1, "my_order is %d\n", my_order);

	memcpy(&nmx_oldest_sequence_number, buffer_data, 4);
	nmxp_log(0, 1, "Oldest sequence number = %d\n", nmx_oldest_sequence_number);

	memcpy(nmx_hdr, buffer_data+4, 17);
	/* Decode the Nanometrics packet header bundle. */
	memcpy (&nmx_ptype, nmx_hdr+0, 1);
	if ( (nmx_ptype & 0xf) == 9) {
	    /* Filler packet.  Discard entire packet.   */
	    nmxp_log (1,0, "Filler packet - discarding\n");
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
	    // nmxp_log(0, 1, "WARNING: changed nmx_x0, old value = %d\n",  nmx_x0);
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

	nmxp_log(0, 1, "nmx_ptype          = %d\n", nmx_ptype);
	nmxp_log(0, 1, "nmx_seconds        = %d\n", nmx_seconds);
	nmxp_log(0, 1, "nmx_ticks          = %d\n", nmx_ticks);

	nmxp_log(0, 1, "nmx_seconds_double = %f\n", nmx_seconds_double);
	nmxp_log(0, 1, "nmx_x0             = %d\n", nmx_x0);

	nmxp_log(0, 1, "nmx_instr_id       = %d\n", nmx_instr_id);
	nmxp_log(0, 1, "nmx_seqno          = %d\n", nmx_seqno);
	nmxp_log(0, 1, "nmx_sample_rate    = %d\n", nmx_sample_rate);
	nmxp_log(0, 1, "this_sample_rate    = %d\n", this_sample_rate);

	comp_bytecount = length_data-21;
	indata = (unsigned char *) buffer_data + 21;

	/* Unpack the data bundles, each 17 bytes long. */
	prev_xn = nmx_x0;
	outdata[0] = nmx_x0;
	nout = 1;
	for (i=0; i<comp_bytecount; i+=17) {
	    if (i+17>comp_bytecount) {
		nmxp_log (1,0, "comp_bytecount = %d, i+17 = %d\n",
			comp_bytecount, i+17);
		exit(1);
	    }
	    if (nout+16 > MAX_OUTDATA)  {
		nmxp_log (1,0, "Output buffer size too small\n");
		exit(1);
	    }
	    k = nmxp_data_unpack_bundle (outdata+nout,indata+i,&prev_xn);
	    if (k < 0) nmxp_log (1,0, "Break: (k=%d) %s %d\n", k, __FILE__,  __LINE__);
	    if (k < 0) break;
	    nout += k;
	    /* prev_xn = outdata[nout-1]; */
	}
	nout--;

	nmxp_log(0, 1, "Unpacked %d samples.\n", nout);

	pKey = (nmx_instr_id << 16) | ( 1 << 8) | ( chan_code);

	pTime = nmx_seconds_double;

	pDataPtr = outdata;

	pNSamp = nout;

	pSampRate = this_sample_rate;

	/* Lookup the station and channel code */
	sta = strdup(nmxp_chan_lookupName(pKey, channelList));
	if ( (chan = strchr(sta, '.')) == NULL ) {
	    nmxp_log(1, 0, "Channel name not in STN.CHAN format: %s\n", sta);
	    /*
	    free(sta);
	    return;
	    */
	}
	if(chan) {
	    *chan++ = '\0';
	}

	nmxp_log(0, 1, "Channel key %d for %s.%s\n", pKey, sta, chan);

	nmxp_data_init(&pd);

	pd.key = pKey;
	if(sta) {
	    strlcpy(pd.station, sta, STATION_LENGTH);
	}
	if(chan) {
	    strlcpy(pd.channel, chan, CHANNEL_LENGTH);
	}
	pd.packet_type = nmx_ptype;
	pd.x0 = nmx_x0;
	pd.seq_no = nmx_seqno;
	pd.time = pTime;
	pd.buffer = buffer_data;
	pd.length = length_data;
	pd.nSamp = pNSamp;
	pd.pDataPtr = pDataPtr;
	pd.sampRate = pSampRate;

	free(sta);

	return &pd;
}

