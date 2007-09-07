/*! \file
 *
 * \brief Base for Nanometrics Protocol Library
 *
 * Author:
 * 	Matteo Quintiliani
 * 	Istituto Nazionale di Geofisica e Vulcanologia - Italy
 *	quintiliani@ingv.it
 *
 * $Id: nmxp_base.h,v 1.20 2007-09-07 07:08:30 mtheo Exp $
 *
 */

#ifndef NMXP_BASE_H
#define NMXP_BASE_H 1

#include "nmxp_data.h"
#include "nmxp_chan.h"
#include "nmxp_log.h"

/*! Maximum time between connection attempts (seconds). */
#define NMXP_SLEEPMAX 10

/*! Return message for succes on socket. */
#define NMXP_SOCKET_OK          0

/*! Return message for error on socket.*/
#define NMXP_SOCKET_ERROR      -1


/*! \brief Looks up target host, opens a socket and connects
 *
 *  \param hostname	hostname
 *  \param portNum	port number
 *
 *  \retval sd A descriptor referencing the socket.
 *  \retval -1 "Empty host name", "Cannot lookup host", ...
 *
 */
int nmxp_openSocket(char *hostname, int portNum);


/*! \brief Close a socket.
 *
 *  \param isock  A descriptor referencing the socket.
 *
 *  \retval  0 Success
 *  \retval -1 Error
 *
 */
int nmxp_closeSocket(int isock);


/*! \brief Sends a buffer on a socket.
 *
 * \param isock A descriptor referencing the socket.
 * \param buffer Data buffer.
 * \param length Length in bytes.
 *
 * \retval NMXP_SOCKET_OK on success
 * \retval NMXP_SOCKET_ERROR on error
 *
 */
int nmxp_send_ctrl(int isock, void *buffer, int length);


/*! \brief Receives length bytes in a buffer from a socket.
 *
 * \param isock A descriptor referencing the socket.
 * \param[out] buffer Data buffer.
 * \param length Length in bytes.
 *
 * \warning Data buffer it has to be allocated before and big enough to contain length bytes!
 *
 * \retval NMXP_SOCKET_OK on success
 * \retval NMXP_SOCKET_ERROR on error
 *
 */
int nmxp_recv_ctrl(int isock, void *buffer, int length);


/*! \brief Sends header of a message.
 *
 * \param isock A descriptor referencing the socket.
 * \param type Type of message within \ref NMXP_MSG_CLIENT.
 * \param length Length in bytes.
 *
 * \retval NMXP_SOCKET_OK on success
 * \retval NMXP_SOCKET_ERROR on error
 *
 */
int nmxp_sendHeader(int isock, NMXP_MSG_CLIENT type, int32_t length);


/*! \brief Receives header of a message.
 * 
 * \param isock A descriptor referencing the socket.
 * \param[out] type Type of message within \ref NMXP_MSG_CLIENT.
 * \param[out] length Length in bytes.
 *
 * \retval NMXP_SOCKET_OK on success
 * \retval NMXP_SOCKET_ERROR on error
 *
 */
int nmxp_receiveHeader(int isock, NMXP_MSG_SERVER *type, int32_t *length);


/*! \brief Sends header and body of a message.
 *
 * \param isock A descriptor referencing the socket.
 * \param type Type of message within \ref NMXP_MSG_CLIENT.
 * \param buffer Data buffer. It could be NULL.
 * \param length Length in bytes. It must be greater or equal to zero.
 *
 * \retval NMXP_SOCKET_OK on success
 * \retval NMXP_SOCKET_ERROR on error
 *
 */
int nmxp_sendMessage(int isock, NMXP_MSG_CLIENT type, void *buffer, int32_t length);


/*! \brief Receives header and body of a message.
 *
 * \param isock A descriptor referencing the socket.
 * \param[out] type Type of message within \ref NMXP_MSG_SERVER.
 * \param[out] buffer Data buffer. It will need to be freed!
 * \param[out] length Length in bytes.
 *
 * \warning buffer will need to be freed!
 *
 * \retval NMXP_SOCKET_OK on success
 * \retval NMXP_SOCKET_ERROR on error
 *
 */
int nmxp_receiveMessage(int isock, NMXP_MSG_SERVER *type, void **buffer, int32_t *length);


/*! \brief Process Compressed Data message by function func_processData().
 *
 * \param buffer_data Pointer to the data buffer containing Compressed Nanometrics packets.
 * \param length_data Buffer length in bytes.
 * \param channelList Pointer to the Channel List.
 * \param network_code Network code. It can be NULL.
 *
 * \return Return a pointer to static struct NMXP_DATA_PROCESS.
 *
 */
NMXP_DATA_PROCESS *nmxp_processCompressedData(char* buffer_data, int length_data, NMXP_CHAN_LIST *channelList, const char *network_code);


/*! \brief Process decompressed Data message by function func_processData().
 *
 * \param buffer_data Pointer to the data buffer containing Decompressed Nanometrics packets.
 * \param length_data Buffer length in bytes.
 * \param channelList Pointer to the Channel List.
 * \param network_code Network code. It can be NULL.
 *
 * \return Return a pointer to static struct NMXP_DATA_PROCESS.
 *
 */
NMXP_DATA_PROCESS *nmxp_processDecompressedData(char* buffer_data, int length_data, NMXP_CHAN_LIST *channelList, const char *network_code);

#endif

