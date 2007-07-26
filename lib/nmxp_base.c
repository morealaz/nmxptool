/*! \file
 *
 * \brief Base for Nanometrics Protocol Libray
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


int nmxp_openSocket(char *hostname, int portNum)
{
  static int sleepTime = 1;
  int isock = 0;
  struct hostent *hostinfo = NULL;
  struct sockaddr_in psServAddr;
  struct in_addr hostaddr;
  
  if (!hostname)
  {
    nmxp_gen_log(1,0, "Empty host name?\n");
    return -1;
  }

  if ( (hostinfo = gethostbyname(hostname)) == NULL) {
    nmxp_gen_log(1,0, "Cannot lookup host: %s\n", hostname);
    return -1;
  }

  while(1)
  {
    isock = socket (AF_INET, SOCK_STREAM, 0);
    if (isock < 0)
    {
      nmxp_gen_log (1,0, "Can't open stream socket\n");
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
    nmxp_gen_log(0,1, "Attempting to connect to %s port %d\n",
	    inet_ntoa(hostaddr), portNum);

    if (connect(isock, (struct sockaddr *)&psServAddr, sizeof(psServAddr)) >= 0)
    {
      sleepTime = 1;
      nmxp_gen_log(0, 1, "Connection established: socket=%i,IP=%s,port=%d\n",
	      isock, inet_ntoa(hostaddr), portNum);
      return isock;
    }
    else
    {
      nmxp_gen_log(0, 1, "Trying again later...Sleeping\n");
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
    nmxp_gen_log(0,0, "nmxp_recv_ctrl(): (recvCount != length) %d != %d - errno = %d (%s)\n", recvCount, length, recv_errno, recv_errno_str);
	    
    return NMXP_SOCKET_ERROR;
  }
  
  return NMXP_SOCKET_OK;
}


int nmxp_sendHeader(int isock, enum NMXP_MSG_CLIENT type, uint32_t length)
{  
    nmxp_MessageHeader msg;

    msg.signature = htonl(NMX_SIGNATURE);
    msg.type      = htonl(type);
    msg.length    = htonl(length);

    return nmxp_send_ctrl(isock, &msg, sizeof(nmxp_MessageHeader));
}


int nmxp_receiveHeader(int isock, enum NMXP_MSG_SERVER *type, uint32_t *length)
{  
    int ret ;
    nmxp_MessageHeader msg;

    ret = nmxp_recv_ctrl(isock, &msg, sizeof(nmxp_MessageHeader));

    *type = 0;
    *length = 0;

    if(ret == NMXP_SOCKET_OK) {
	msg.signature = ntohl(msg.signature);
	msg.type      = ntohl(msg.type);
	msg.length    = ntohl(msg.length);

	nmxp_gen_log(0,1, "nmxp_receiveHeader(): signature = %d, type = %d, length = %d\n",
		    msg.signature, msg.type, msg.length);

	if (msg.signature != NMX_SIGNATURE)
	{
	    ret = NMXP_SOCKET_ERROR;
	    nmxp_gen_log(1,0, "nmxp_receiveHeader(): signature mismatches. signature = %d, type = %d, length = %d\n",
		    msg.signature, msg.type, msg.length);
	} else {
	    *type = msg.type;
	    *length = msg.length;
	}
    }

    return ret;
}


int nmxp_sendMessage(int isock, enum NMXP_MSG_CLIENT type, void *buffer, uint32_t length) {
    int ret;
    ret = nmxp_sendHeader(isock, type, length);
    if( ret == NMXP_SOCKET_OK) {
	if(buffer && length > 0) {
	    ret = nmxp_send_ctrl(isock, buffer, length);
	}
    }
    return ret;
}


int nmxp_receiveMessage(int isock, enum NMXP_MSG_SERVER *type, void **buffer, uint32_t *length) {
    int ret;
    *buffer = NULL;
    *length = 0;

    ret = nmxp_receiveHeader(isock, type, length);
    if( ret == NMXP_SOCKET_OK) {
	 if (*length > 0) {
	     *buffer = malloc(*length);
	     ret = nmxp_recv_ctrl(isock, *buffer, *length);
	 } else {
	     nmxp_gen_log(1,0, "Error in nmxp_receiveMessage()\n");
	 }
    } else {
	nmxp_gen_log(1,0, "Error in nmxp_receiveMessage()\n");
    }
    return ret;
}


int nmxp_gen_log(int level, int verb, ... )
{
  static int staticverb = 0;
  int retvalue = 0;

  if ( level == -1 ) {
    staticverb = verb;
    retvalue = staticverb;
  }
  else if (verb <= staticverb) {
    char message[100];
    char timestr[100];
    char *format;
    va_list listptr;
    time_t loc_time;

    va_start(listptr, verb);
    format = va_arg(listptr, char *);

    /* Build local time string and cut off the newline */
    time(&loc_time);
    // TODO
    strcpy(timestr, asctime(localtime(&loc_time)));
    timestr[strlen(timestr) - 1] = '\0';

    retvalue = vsnprintf(message, 100, format, listptr);

    if ( level == 1 ) {
      printf("%s - libnmxp: error: %s",timestr, message);
    }
    else {
      printf("%s - libnmxp: %s", timestr, message);
    }

    fflush(stdout);
    va_end(listptr);
  }

  return retvalue;
} /* End of nmxp_gen_log() */
