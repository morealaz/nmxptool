/*! \mainpage 
 *
 * \section intro_sec Introduction
 *
 * This is the introduction.
 *
 * \section install_sec Installation
 *
 * ./configure
 * 
 * make
 *
 * make install
 *
 * \section usage_sec Usage
 *
 * \subsection step1 Step 1: Opening the box
 *  
 * etc...
 *
 * \section about_sec About
 *
 * Author:
 * 	Matteo Quintiliani
 * 	Istituto Nazionale di Geofisica e Vulcanologia - Italy
 *	quintiliani@ingv.it
 */


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

#ifndef NMXP_H
#define NMXP_H 1

#include "nmxpbase.h"

/*! \brief Sends the message "Connect" on a socket
 *
 * \param isock A descriptor referencing the socket.
 *
 * \retval SOCKET_OK on success
 * \retval SOCKET_ERROR on error
 * 
 */
int sendConnect(int isock);


/*! \brief Sends the message "TerminateSubscription" on a socket
 *
 * \param isock A descriptor referencing the socket.
 * \param reason Reason for the shutdown.
 * \param message String message. It could be NULL.
 *
 * \retval SOCKET_OK on success
 * \retval SOCKET_ERROR on error
 * 
 */
int sendTerminateSubscription(int isock, enum NMXP_REASON_SHUTDOWN reason, char *message);

/*! \brief The key/name info for one channel */
typedef struct ChannelKey
{
    uint32_t key;
    char name[12];
} ChannelKey;

#define MAX_N_CHAN 1000
/*! \brief Channel List */
typedef struct ChannelList
{
    uint32_t number;
    ChannelKey channel[MAX_N_CHAN];
} ChannelList;


/*! \brief Receive message "ChannelList" from a socket
 *
 * \param isock A descriptor referencing the socket.
 * \param pchannelList[out] List of channels. It will need to be freed!
 *
 * \retval SOCKET_OK on success
 * \retval SOCKET_ERROR on error
 * 
 */
int receiveChannelList(int isock, ChannelList **pchannelList);

#endif
