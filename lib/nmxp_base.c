/*! \file
 *
 * \brief Base for Nanometrics Protocol Library
 *
 * Author:
 * 	Matteo Quintiliani
 * 	Istituto Nazionale di Geofisica e Vulcanologia - Italy
 *	quintiliani@ingv.it
 *
 * $Id: nmxp_base.c,v 1.48 2007-12-16 21:08:32 mtheo Exp $
 *
 */

#include "config.h"
#include "nmxp_base.h"
#ifdef HAVE_WINDOWS_H
#include "nmxp_win.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef HAVE_WINDOWS_H
#include "winsock2.h"
#warning You are compiling on Windows MinGW...
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#endif

#define MAX_OUTDATA 4096


int nmxp_openSocket(char *hostname, int portNum)
{
  static int sleepTime = 1;
  int isock = 0;
  struct hostent *hostinfo = NULL;
  struct sockaddr_in psServAddr;
  struct in_addr hostaddr;

#ifdef HAVE_WINDOWS_H
  nmxp_initWinsock();
#endif

  if (!hostname)
  {
    nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_CONNFLOW, "Empty host name?\n");
    return -1;
  }

  if ( (hostinfo = gethostbyname(hostname)) == NULL) {
    nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_CONNFLOW, "Cannot lookup host: %s\n", hostname);
    return -1;
  }

  while(1)
  {
    isock = socket (AF_INET, SOCK_STREAM, 0);

#ifdef HAVE_WINDOWS_H
    if (isock == INVALID_SOCKET) {
	    nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_CONNFLOW,"Error at socket()\n");
	    WSACleanup();
	    exit(1);
    }
#else
    if (isock < 0)
    {
      nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_CONNFLOW, "Can't open stream socket\n");
      exit(1);
    }
#endif

    /* Fill in the structure "psServAddr" with the address of server
       that we want to connect with */
    memset (&psServAddr, 0, sizeof(psServAddr));
    psServAddr.sin_family = AF_INET;
    psServAddr.sin_port = htons((unsigned short) portNum);
#ifdef HAVE_WINDOWS_H
    unsigned long address;
    memcpy(&address, hostinfo->h_addr, (size_t) hostinfo->h_length);
    psServAddr.sin_addr.s_addr = address;
#else
    psServAddr.sin_addr = *(struct in_addr *)hostinfo->h_addr_list[0];
#endif

    /* Report action and resolved address */
    memcpy(&hostaddr.s_addr, *hostinfo->h_addr_list,
	   sizeof (hostaddr.s_addr));
    nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_CONNFLOW, "Attempting to connect to %s port %d\n",
	    inet_ntoa(hostaddr), portNum);

    if (connect(isock, (struct sockaddr *)&psServAddr, sizeof(psServAddr)) >= 0)
    {
      sleepTime = 1;
      nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_CONNFLOW, "Connection established: socket=%i,IP=%s,port=%d\n",
	      isock, inet_ntoa(hostaddr), portNum);
      return isock;
    }
    else
    {
      nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_CONNFLOW, "Connecting to %s port %d. Trying again after %d seconds...\n",
	      inet_ntoa(hostaddr), portNum, sleepTime);
      nmxp_closeSocket(isock);
      nmxp_sleep (sleepTime);
      sleepTime *= 2;
      if (sleepTime > NMXP_SLEEPMAX)
        sleepTime = NMXP_SLEEPMAX;
    }
  }
}


int nmxp_closeSocket(int isock) {
	int ret;
	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_CONNFLOW, "Closed connection.\n");
#ifdef HAVE_WINDOWS_H
	ret = closesocket(isock);
	WSACleanup();
#else
	ret = close(isock);
#endif
	return ret;
}


int nmxp_send_ctrl(int isock, void* buffer, int length)
{
  int sendCount = send(isock, (char*) buffer, length, 0);
  
  if (sendCount != length)
    return NMXP_SOCKET_ERROR;

  return NMXP_SOCKET_OK;
}


int nmxp_recv_ctrl(int isock, void *buffer, int length, int timeoutsec, int *recv_errno )
{
  int recvCount;
  int cc;
  char *buffer_char = buffer;

#ifdef HAVE_WINDOWS_H
  char *recv_errno_str;
#else
#ifdef HAVE_STRERROR_R
#define MAXLEN_RECV_ERRNO_STR 200
  char recv_errno_str[MAXLEN_RECV_ERRNO_STR];
#else
  char *recv_errno_str;
#endif
#endif

#ifdef HAVE_WINDOWS_H
  int timeos;
#else
  struct timeval timeo;
#endif


  /*
  struct timeval timeout;
  socklen_t size_timeout = sizeof(timeout);
  getsockopt(isock, SOL_SOCKET, SO_RCVTIMEO, &timeout, &size_timeout);
  */

  if(timeoutsec > 0) {
#ifdef HAVE_WINDOWS_H
      timeos  = timeoutsec * 1000;
      if (setsockopt(isock, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeos, sizeof(timeos)) < 0)
      {
	  perror("setsockopt SO_RCVTIMEO");
      }
#else
      timeo.tv_sec  = timeoutsec;
      timeo.tv_usec = 0;
      if (setsockopt(isock, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(timeo)) < 0) {
	  perror("setsockopt SO_RCVTIMEO");
      }
#endif
  }
  
  cc = 1;
  *recv_errno  = 0;
  recvCount = 0;
  while(cc > 0 && *recv_errno == 0  && recvCount < length) {
      cc = recv(isock, buffer_char + recvCount, length - recvCount, 0);
#ifdef HAVE_WINDOWS_H
      *recv_errno  = WSAGetLastError();
#else
      *recv_errno  = errno;
#endif
      if(cc <= 0) {
	  /*
	  nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_CONNFLOW, "nmxp_recv_ctrl(): (cc=%d <= 0) errno=%d  recvCount=%d  length=%d\n", cc, *recv_errno, recvCount, length);
	  */
      } else {
	  recvCount += cc;
      }
  }

  if(timeoutsec > 0) {
#ifdef HAVE_WINDOWS_H
      timeos  = 0;
      if (setsockopt(isock, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeos, sizeof(timeos))
	      < 0) {
	  perror("setsockopt SO_RCVTIMEO");
      }
#else
      timeo.tv_sec  = 0;
      timeo.tv_usec = 0;
      if (setsockopt(isock, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(timeo)) < 0) {
	  perror("setsockopt SO_RCVTIMEO");
      }
#endif
  }

  if (recvCount != length  ||  *recv_errno != 0  ||  cc <= 0) {
#ifdef HAVE_WINDOWS_H
      recv_errno_str = WSAGetLastErrorMessage(*recv_errno);
#else
#ifdef HAVE_STRERROR_R
      strerror_r(*recv_errno, recv_errno_str, MAXLEN_RECV_ERRNO_STR);
#else
      recv_errno_str = strerror(*recv_errno);
#endif
#endif
    nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_CONNFLOW, "nmxp_recv_ctrl(): recvCount=%d  length=%d  (cc=%d) errno=%d (%s)\n", recvCount, length, cc, *recv_errno, recv_errno_str);
	    
#ifdef HAVE_WINDOWS_H
    if(recvCount != length || *recv_errno != WSAEWOULDBLOCK) {
#else
    if(recvCount != length || *recv_errno != EWOULDBLOCK) {
#endif
	return NMXP_SOCKET_ERROR;
    }
  }
  
  return NMXP_SOCKET_OK;
}


int nmxp_sendHeader(int isock, NMXP_MSG_CLIENT type, int32_t length)
{  
    NMXP_MESSAGE_HEADER msg;

    msg.signature = htonl(NMX_SIGNATURE);
    msg.type      = htonl(type);
    msg.length    = htonl(length);

    return nmxp_send_ctrl(isock, &msg, sizeof(NMXP_MESSAGE_HEADER));
}


int nmxp_receiveHeader(int isock, NMXP_MSG_SERVER *type, int32_t *length, int timeoutsec, int *recv_errno )
{  
    int ret ;
    NMXP_MESSAGE_HEADER msg;

    ret = nmxp_recv_ctrl(isock, &msg, sizeof(NMXP_MESSAGE_HEADER), timeoutsec, recv_errno);

    *type = 0;
    *length = 0;

    if(ret == NMXP_SOCKET_OK) {
	msg.signature = ntohl(msg.signature);
	msg.type      = ntohl(msg.type);
	msg.length    = ntohl(msg.length);

	if (msg.signature != NMX_SIGNATURE)
	{
	    ret = NMXP_SOCKET_ERROR;
	    nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_CONNFLOW, "nmxp_receiveHeader(): signature mismatches. signature = %d, type = %d, length = %d\n",
		    msg.signature, msg.type, msg.length);
	} else {
	    *type = msg.type;
	    *length = msg.length;
	}
    }

    return ret;
}


int nmxp_sendMessage(int isock, NMXP_MSG_CLIENT type, void *buffer, int32_t length) {
    int ret;
    ret = nmxp_sendHeader(isock, type, length);
    if( ret == NMXP_SOCKET_OK) {
	if(buffer && length > 0) {
	    ret = nmxp_send_ctrl(isock, buffer, length);
	}
    }
    return ret;
}


int nmxp_receiveMessage(int isock, NMXP_MSG_SERVER *type, void **buffer, int32_t *length, int timeoutsec, int *recv_errno ) {
    int ret;
    *buffer = NULL;
    *length = 0;

    ret = nmxp_receiveHeader(isock, type, length, timeoutsec, recv_errno);

    if( ret == NMXP_SOCKET_OK  ) {
	if (*length > 0) {
	    *buffer = malloc(*length);
	    ret = nmxp_recv_ctrl(isock, *buffer, *length, 0, recv_errno);

	    if(*type == NMXP_MSG_ERROR) {
		nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_PACKETMAN, "Received ErrorMessage: %s\n", *buffer);
	    } else {
		nmxp_log(NMXP_LOG_WARN, NMXP_LOG_D_PACKETMAN, "Received message type: %d  length=%d\n", *type, *length);
	    }

	}
    }

    if(*recv_errno != 0) {
#ifdef HAVE_WINDOWS_H
	if(*recv_errno == WSAEWOULDBLOCK) {
#else
	if(*recv_errno == EWOULDBLOCK) {
#endif
	    nmxp_log(NMXP_LOG_WARN, NMXP_LOG_D_CONNFLOW, "Timeout receiving in nmxp_receiveMessage()\n");
	} else {
	    nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_CONNFLOW, "Error in nmxp_receiveMessage()\n");
	}
    }

    return ret;
}


NMXP_DATA_PROCESS *nmxp_processDecompressedData(char* buffer_data, int length_data, NMXP_CHAN_LIST_NET *channelList, const char *network_code_default)
{
  int32_t   netInt    = 0;
  int32_t   pKey      = 0;
  double    pTime     = 0.0;
  int32_t   pNSamp    = 0;
  int32_t   pSampRate = 0;
  int32_t  *pDataPtr  = 0;
  int       swap      = 0;
  int       idx;
  int32_t outdata[MAX_OUTDATA];

  char station_code[20];
  char channel_code[20];
  char network_code[20];

  static NMXP_DATA_PROCESS pd;

  /* copy the header contents into local fields and swap */
  memcpy(&netInt, &buffer_data[0], 4);
  pKey = ntohl(netInt);
  if ( pKey != netInt ) { swap = 1; }

  memcpy(&pTime, &buffer_data[4], 8);
  if ( swap ) { nmxp_data_swap_8b((int64_t *) &pTime); }

  memcpy(&netInt, &buffer_data[12], 4);
  pNSamp = ntohl(netInt);
  memcpy(&netInt, &buffer_data[16], 4);
  pSampRate = ntohl(netInt);

  /* There should be (length_data - 20) bytes of data as 32-bit ints here */
  memcpy(outdata , (int32_t *) &buffer_data[20], length_data - 20);
  pDataPtr = outdata;

  /* Swap the data samples to host order */
  for ( idx=0; idx < pNSamp; idx++ ) {
      netInt = ntohl(pDataPtr[idx]);
      pDataPtr[idx] = netInt;
  }

  if(!nmxp_chan_cpy_sta_chan(nmxp_chan_lookupName(pKey, channelList), station_code, channel_code, network_code)) {
    nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_PACKETMAN, "Channel name not in STA.CHAN format: %s\n", nmxp_chan_lookupName(pKey, channelList));
  }
  
  nmxp_data_init(&pd);

  pd.key = pKey;
  if(network_code[0] != 0) {
      strcpy(pd.network, network_code);
  } else {
      strcpy(pd.network, network_code_default);
  }
  if(station_code[0] != 0) {
      strncpy(pd.station, station_code, STATION_LENGTH);
  }
  if(channel_code[0] != 0) {
      strncpy(pd.channel, channel_code, CHANNEL_LENGTH);
  }
  pd.packet_type = NMXP_MSG_DECOMPRESSED;
  pd.x0 = -1;
  pd.xn = -1;
  pd.x0n_significant = 0;
  // TODO
  // pd.oldest_seq_no = ;
  // pd.seq_no = ;
  pd.time = pTime;
  pd.buffer = buffer_data;
  pd.length = length_data;
  pd.nSamp = pNSamp;
  pd.pDataPtr = pDataPtr;
  pd.sampRate = pSampRate;

  return &pd;
}


NMXP_DATA_PROCESS *nmxp_processCompressedData(char* buffer_data, int length_data, NMXP_CHAN_LIST_NET *channelList, const char *network_code_default)
{
    int32_t   pKey      = 0;
    double    pTime     = 0.0;
    int32_t   pNSamp    = 0;
    int32_t   pSampRate = 0;
    int32_t  *pDataPtr  = 0;

    char station_code[20];
    char channel_code[20];
    char network_code[20];

    static NMXP_DATA_PROCESS pd;

    int32_t nmx_rate_code_to_sample_rate[32] = {
	0,1,2,5,10,20,40,50,
	80,100,125,200,250,500,1000,25,
	120,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0};

	int32_t nmx_oldest_sequence_number;
	char nmx_hdr[25];
	unsigned char nmx_ptype;
	int32_t nmx_seconds;
	double nmx_seconds_double;
	int16_t nmx_ticks, nmx_instr_id;
	int32_t nmx_seqno;
	unsigned char nmx_sample_rate;
	int32_t nmx_x0;
	int32_t rate_code, chan_code, this_sample_rate;

	int32_t comp_bytecount;
	unsigned char *indata;
	int32_t outdata[MAX_OUTDATA];
	int32_t nout, i, k;
	int32_t prev_xn;

	// TOREMOVE int my_order = get_my_wordorder();
	int my_host_is_bigendian = nmxp_data_bigendianhost();
	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "my_host_is_bigendian %d\n", my_host_is_bigendian);

	memcpy(&nmx_oldest_sequence_number, buffer_data, 4);
	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "Oldest sequence number = %d\n", nmx_oldest_sequence_number);

	memcpy(nmx_hdr, buffer_data+4, 17);
	/* Decode the Nanometrics packet header bundle. */
	memcpy (&nmx_ptype, nmx_hdr+0, 1);
	if ( (nmx_ptype & 0xf) == 9) {
	    /* Filler packet.  Discard entire packet.   */
	    nmxp_log (NMXP_LOG_ERR, NMXP_LOG_D_PACKETMAN, "Filler packet - discarding\n");
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

	const uint32_t high_scale = 4096 * 2048;
	const uint32_t high_scale_p = 4096 * 4096;
	/* check if nmx_x0 is negative like as signed 3-byte int */
	if( (nmx_x0 & high_scale) ==  high_scale) {
	    // nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "WARNING: changed nmx_x0, old value = %d\n",  nmx_x0);
	    nmx_x0 -= high_scale_p;
	}
	/* TOREMOVE if (my_order != SEED_LITTLE_ENDIAN) { */
	if (my_host_is_bigendian) {
	    nmxp_data_swap_4b ((int32_t *)&nmx_seconds);
	    nmxp_data_swap_2b (&nmx_ticks);
	    nmxp_data_swap_2b (&nmx_instr_id);
	    nmxp_data_swap_4b (&nmx_seqno);
	    nmx_x0 = nmx_x0 >> 8;
	    nmxp_data_swap_4b (&nmx_x0);
	    nmx_x0 = nmx_x0 >> 8;
	    // TODO
	    nmx_x0++;
	}
	nmx_seconds_double = (double) nmx_seconds + ( (double) nmx_ticks / 10000.0 );
	rate_code = nmx_sample_rate>>3;
	chan_code = nmx_sample_rate&7;
	this_sample_rate = nmx_rate_code_to_sample_rate[rate_code];

	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "nmx_ptype          = %d\n", nmx_ptype);
	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "nmx_seconds        = %d\n", nmx_seconds);
	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "nmx_ticks          = %d\n", nmx_ticks);

	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "nmx_seconds_double = %f\n", nmx_seconds_double);
	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "nmx_x0             = %d\n", nmx_x0);

	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "nmx_instr_id       = %d\n", nmx_instr_id);
	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "nmx_seqno          = %d\n", nmx_seqno);
	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "nmx_sample_rate    = %d\n", nmx_sample_rate);
	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "this_sample_rate    = %d\n", this_sample_rate);

	pKey = (nmx_instr_id << 16) | ( 1 << 8) | ( chan_code);

	pTime = nmx_seconds_double;

	pSampRate = this_sample_rate;

	if(!nmxp_chan_cpy_sta_chan(nmxp_chan_lookupName(pKey, channelList), station_code, channel_code, network_code)) {
	    nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_PACKETMAN, "Channel name not in STA.CHAN format: %s\n", nmxp_chan_lookupName(pKey, channelList));
	}
  
	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "Channel key %d for %s.%s\n", pKey, station_code, channel_code);

	comp_bytecount = length_data-21;
	indata = (unsigned char *) buffer_data + 21;

	/* Unpack the data bundles, each 17 bytes long. */
	prev_xn = nmx_x0;
	outdata[0] = nmx_x0;
	nout = 1;
	for (i=0; i<comp_bytecount; i+=17) {
	    if (i+17>comp_bytecount) {
		nmxp_log (NMXP_LOG_ERR, NMXP_LOG_D_PACKETMAN, "comp_bytecount = %d, i+17 = %d\n",
			comp_bytecount, i+17);
		exit(1);
	    }
	    if (nout+16 > MAX_OUTDATA)  {
		nmxp_log (NMXP_LOG_ERR,  NMXP_LOG_D_PACKETMAN, "Output buffer size too small\n");
		exit(1);
	    }
	    k = nmxp_data_unpack_bundle (outdata+nout,indata+i,&prev_xn);
	    if (k < 0) nmxp_log (NMXP_LOG_WARN, NMXP_LOG_D_PACKETMAN, "Null bundle: %s.%s.%s (k=%d) %s %d\n", network_code, station_code, channel_code, k, __FILE__,  __LINE__);
	    if (k < 0) break;
	    nout += k;
	    /* prev_xn = outdata[nout-1]; */
	}
	nout--;

	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "Unpacked %d samples.\n", nout);

	pDataPtr = outdata;

	pNSamp = nout;

	nmxp_data_init(&pd);

	pd.key = pKey;
	if(network_code[0] != 0) {
	    strcpy(pd.network, network_code);
	} else {
	    strcpy(pd.network, network_code_default);
	}
	if(station_code[0] != 0) {
	    strncpy(pd.station, station_code, STATION_LENGTH);
	}
	if(channel_code[0] != 0) {
	    strncpy(pd.channel, channel_code, CHANNEL_LENGTH);
	}
	pd.packet_type = nmx_ptype;
	pd.x0 = nmx_x0;
	pd.xn = pDataPtr[nout];
	pd.x0n_significant = 1;
	pd.oldest_seq_no = nmx_oldest_sequence_number;
	pd.seq_no = nmx_seqno;
	pd.time = pTime;
	pd.buffer = buffer_data;
	pd.length = length_data;
	pd.nSamp = pNSamp;
	pd.pDataPtr = pDataPtr;
	pd.sampRate = pSampRate;

	return &pd;
}


unsigned int nmxp_sleep(unsigned int sleep_time) {
#ifdef HAVE_WINDOWS_H
    Sleep(sleep_time);
    return 0;
#else
    return sleep(sleep_time);
#endif
}

