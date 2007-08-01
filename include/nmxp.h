/*! \mainpage 
 *
 * \section intro_sec Introduction
 *
 * This is the introduction.
 *
 * \section dependencies_sec Dependencies
 *
 * qlib2 ftp://quake.geo.berkeley.edu/pub/quanterra/ucb/
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
 * Author: \n
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


/*! \brief Body of ConnectRequest message*/
typedef struct {
    char username[12];
    uint32_t version;
    uint32_t connection_time;
    uint32_t crc32;
} NMXP_CONNECT_REQUEST; 

/*! \brief Body of DataRequest message*/
typedef struct {
    uint32_t chan_key;
    uint32_t start_time;
    uint32_t end_time;
} NMXP_DATA_REQUEST;


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
int nmxp_sendTerminateSubscription(int isock, NMXP_SHUTDOWN_REASON reason, char *message);


/*! \brief Receive message "NMXP_CHAN_LIST" from a socket
 *
 * \param isock A descriptor referencing the socket.
 * \param[out] pchannelList List of channels. It will need to be freed!
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


/*! \brief Receive Compressed or Decompressed Data message from a socket and launch func_processData() on the extracted data
 *
 * \param isock A descriptor referencing the socket.
 * \param channelList Channel list.
 * \param func_processData Pointer to the function manages data extracted. It could be NULL.
 *
 * \retval SOCKET_OK on success
 * \retval SOCKET_ERROR on error
 * 
 */
int nmxp_receiveData(int isock, NMXP_CHAN_LIST *channelList,
	int (*func_processData)(NMXP_PROCESS_DATA *pd)
	);


/*! \brief Sends the message "ConnectRequest" on a socket
 *
 * \param isock A descriptor referencing the socket.
 * \param naqs_username User name (maximum 11 characters), zero terminated.
 * \param naqs_password Password.
 * \param connection_time Time that the connection was opened.
 *
 * \retval SOCKET_OK on success
 * \retval SOCKET_ERROR on error
 * 
 */
int nmxp_sendConnectRequest(int isock, char *naqs_username, char *naqs_password, uint32_t connection_time);


/*! \brief Read connection time from a socket
 *
 * \param isock A descriptor referencing the socket.
 * \param[out] connection_time Time in epoch.
 *
 * \retval SOCKET_OK on success
 * \retval SOCKET_ERROR on error
 * 
 */
int nmxp_readConnectionTime(int isock, uint32_t *connection_time);


/*! \brief Wait the message "Ready" from a socket
 *
 * \param isock A descriptor referencing the socket.
 *
 * \retval SOCKET_OK on success
 * \retval SOCKET_ERROR on error
 * 
 */
int nmxp_waitReady(int isock);


/*! \brief Sends the message "DataRequest" on a socket
 *
 * \param isock A descriptor referencing the socket.
 * \param key Channel key for which data are requested.
 * \param start_time Start time of the interval for which data are requested. Epoch time.
 * \param end_time End time of the interval for which data are requested. Epoch time.
 *
 * \retval SOCKET_OK on success
 * \retval SOCKET_ERROR on error
 * 
 */
int nmxp_sendDataRequest(int isock, uint32_t key, uint32_t start_time, uint32_t end_time);


/*! \brief Get the list of available channels from a server 
 *
 * \param hostname host name
 * \param portnum port number
 * \param datatype Type of data contained in the channel.
 *
 * \return Channel list. It will need to be freed.
 *
 * \warning Returned value will need to be freed.
 * 
 */
NMXP_CHAN_LIST *nmxp_getAvailableChannelList(char * hostname, int portnum, NMXP_DATATYPE datatype);

#endif

