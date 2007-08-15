/*! \mainpage 
 *
 * \section intro_sec Introduction
 *
 * This is the documentation for the <i>APIs</i> that implement the <tt>Nanometrics Protocols</tt>.
 * They have been developed for interacting with \c NaqsServer and \c DataServer.
 *
 * The Nanometrics \c NaqsServer provides online access to time-series, serial data, triggers, and state-of-health data via TCP subscription.
 * 
 * The Nanometrics \c DataServer provides local and remote access to nanometrics, serial, and state-of-health data via TCP/IP.
 *
 * The library offers APIs to:
 * \li interact with \c NaqsServer that uses version 1.4 of the <i>Private Data Stream Protocol</i>
 * \li interact with \c DataServer that uses version 1.0 of the <i>Nanometrics Data Access Protocol</i>
 * \li manage Nanometrics data formats
 * \li request, receive and interpret online and offline data
 *
 * moreover, you can use them to develop software to:
 * \li analyze data in realtime (waveforms, triggers, ...)
 * \li retrieve and convert on the fly data into the mini-SEED records (optional)
 * \li feed SeedLink server (optional)
 *
 *
 * \section dependencies_sec Dependencies
 *
 * An optional library is needed to allow \c libnmxp to save mini-SEED records:
 *
 * \li \c libmseed: http://www.iris.edu/manuals/\n
 * The Mini-SEED library. A C library framework for manipulating and managing SEED data records.\n
 * Author: Chad Trabant, <i>IRIS DMC</i>\n
 *
 *
 * \section install_sec Installation
 *
 * \c nmxp library has been developed using <i>GNU Build Tools</i> (\c automake, \c autoconf and \c configure script)
 * taking in account the POSIX Cross-Platform aspects.
 * So you should be able to compile and install it everywhere you can launch the following commands:
 *
 * <tt>./configure</tt>
 * 
 * <tt>make</tt>
 *
 * <tt>make install</tt>
 *
 * Please, refer to the file \b README and \b INSTALL for more details.
 *
 * \section tools_sec Tools
 *
 * Inside the distribution is available a tool which is a client that interact with \c NaqsServer and \c DataServer.
 *
 * \c nmxptool:
 * 	\li implements the <i>Nanometrics Private Data Stream Protocol 1.4</i> and permits to retrieve data in near-realtime.\n
 * 	\li implements the <i>Nanometrics Data Access Protocol 1.0</i> and permits to retrieve backward data.\n
 * Please, refer to the \b README file or help <tt>nmxptool --help</tt>.
 *
 *
 * \subsection examples_sec Examples
 *
 * etc...
 *
 *
 * \section license_sec License
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU Library General Public License (GNU-LGPL) for more details.
 * File \b COPYING inside this distribution.
 * The GNU-LGPL and further information can be found here: http://www.gnu.org/
 *
 *
 * \section about_sec About
 *
 * Matteo Quintiliani - <i>Istituto Nazionale di Geofisica e Vulcanologia</i> - Italy
 *
 * Mail bug reports and suggestions to <quintiliani@ingv.it>
 *
 */



/*! \file
 *
 * \brief Nanometrics Protocol Library
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
int nmxp_sendAddTimeSeriesChannel(int isock, NMXP_CHAN_LIST *channelList, int32_t shortTermCompletion, uint32_t out_format, NMXP_BUFFER_FLAG buffer_flag);


/*! \brief Receive Compressed or Decompressed Data message from a socket and launch func_processData() on the extracted data
 *
 * \param isock A descriptor referencing the socket.
 * \param channelList Channel list.
 * \param network_code Network code. It can be NULL.
 *
 * \retval Pointer to the structure NMXP_DATA_PROCESS on success
 * \retval NULL on error
 * 
 */
NMXP_DATA_PROCESS *nmxp_receiveData(int isock, NMXP_CHAN_LIST *channelList, const char *network_code);


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


/*! \brief Get the list of the start and end time for the available data for each channel.
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
NMXP_META_CHAN_LIST *nmxp_getMetaChannelList(char * hostname, int portnum, NMXP_DATATYPE datatype, int flag_request_channelinfo);

#endif

