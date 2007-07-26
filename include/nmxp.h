/*! \mainpage 
 *
 * \section intro_sec Introduction
 *
 * This is the introduction.
 *
 * \section dependencies_sec Dependencies
 *
 * qlib2
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

#include "nmxp_base.h"
#include "nmxp_chan.h"
#include "nmxp_crc32.h"

/*! \brief Flag for buffered packets */
typedef enum {
    NMXP_BUFFER_NO			= 0,
    NMXP_BUFFER_YES			= 1
} NMXP_BUFFER_FLAG;


/*! \brief Sends the message "Connect" on a socket
 *
 * \param isock A descriptor referencing the socket.
 *
 * \retval SOCKET_OK on success
 * \retval SOCKET_ERROR on error
 * 
 */
int nmxp_sendConnect(int isock);


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
int nmxp_sendTerminateSubscription(int isock, NMXP_REASON_SHUTDOWN reason, char *message);


/*! \brief Receive message "NMXP_CHAN_LIST" from a socket
 *
 * \param isock A descriptor referencing the socket.
 * \param pchannelList[out] List of channels. It will need to be freed!
 *
 * \retval SOCKET_OK on success
 * \retval SOCKET_ERROR on error
 * 
 */
int nmxp_receiveChannelList(int isock, NMXP_CHAN_LIST **pchannelList);


/*! \brief Sends the message "AddTimeSeriesChannels" on a socket
 *
 * \param isock A descriptor referencing the socket.
 * \param channelList List of channel.
 * \param shortTermCompletion Short-term-completion time = s, 1<= s <= 300 seconds.
 * \param out_format Output format.
 * 	-1 Compressed packets.
 * 	0 Uncompressed packets.
 * 	0 < out_format, requested output sample rate.
 * \param buffer_flag Server will send or not buffered packets.
 *
 * \retval SOCKET_OK on success
 * \retval SOCKET_ERROR on error
 * 
 */
int nmxp_sendAddTimeSeriesChannel(int isock, NMXP_CHAN_LIST *channelList, uint32_t shortTermCompletion, uint32_t out_format, NMXP_BUFFER_FLAG buffer_flag);


/*! \brief Receive Compressed Data message from a socket
 *
 * \param isock A descriptor referencing the socket.
 * \param channelList Channel list.
 *
 * \retval SOCKET_OK on success
 * \retval SOCKET_ERROR on error
 * 
 */
int nmxp_receiveCompressedData(int isock, NMXP_CHAN_LIST *channelList);


/*! \brief Receive Decompressed Data message from a socket
 *
 * \param isock A descriptor referencing the socket.
 * \param channelList Channel list.
 *
 * \retval SOCKET_OK on success
 * \retval SOCKET_ERROR on error
 * 
 */
int nmxp_receiveDecompressedData(int isock, NMXP_CHAN_LIST *channelList);

#endif

