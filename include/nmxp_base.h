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

#ifndef NMXP_BASE_H
#define NMXP_BASE_H 1

#include <sys/types.h>

/*! Maximum time between connection attempts (seconds). */
#define NMXP_SLEEPMAX 10

/*! Return message for succes on socket. */
#define NMXP_SOCKET_OK          0

/*! Return message for error on socket.*/
#define NMXP_SOCKET_ERROR      -1


/*! First 4 bytes of all messages. */
#define NMX_SIGNATURE 0x7abcde0f

/*! \brief Defines the type for reason of shutdown */
enum NMXP_REASON_SHUTDOWN {
    NMXP_NORMAL_SHUTDOWN		= 1,
    NMXP_ERROR_SHUTDOWN			= 2,
    NMXP_TIMEOUT_SHUTDOWN		= 3
};

/*! \brief Defines the client message types */
enum NMXP_MSG_CLIENT {
   NMXPMSG_CONNECT			= 100,
   NMXPMSG_REQUESTPENDIG		= 110,
   NMXPMSG_TERMINATESUBSCRIPTION	= 200,

   NMXPMSG_ADDTIMESERIESCHANNELS	= 120,
   NMXPMSG_ADDSOHCHANNELS		= 121,
   NMXPMSG_ADDSERIALCHANNELS		= 124,
   NMXPMSG_ADDTRIGGERCHANNELS		= 122,
   NMXPMSG_ADDEVENTS			= 123,

   NMXPMSG_REMOVETIMESERIESCHANNELS	= 130,
   NMXPMSG_REMOVESOHCHANNELS		= 131,
   NMXPMSG_REMOVESERIALCHANNELS		= 134,
   NMXPMSG_REMOVETRIGGERCHANNELS	= 132,
   NMXPMSG_REMOVEEVENTS			= 133,

   NMXPMSG_CONNECTREQUEST		= 206,
   NMXPMSG_CHANNELLISTREQUEST		= 209,
   NMXPMSG_PRECISLISTREQUEST		= 203,
   NMXPMSG_CHANNELINFOREQUEST		= 226,
   NMXPMSG_DATASIZEREQUEST		= 229,
   NMXPMSG_DATAREQUEST			= 227,
   NMXPMSG_TRIGGERREQUEST		= 231,
   NMXPMSG_EVENTREQUEST			= 232
   
};

/*! \brief Defines the server message types. */
enum NMXP_MSG_SERVER {
    NMXPMSG_CHANNELLIST			= 150,
    NMXPMSG_ERROR			= 190,
    NMXPMSG_COMPRESSED			= 1,
    NMXPMSG_DECOMPRESSED		= 4,
    NMXPMSG_TRIGGER			= 5,
    NMXPMSG_EVENT			= 6,

    NMXPMSG_READY			= 208,
    NMXPMSG_PRECISLIST			= 253,
    NMXPMSG_CHANNELHEADER		= 256,
    NMXPMSG_DATASIZE			= 257,
    NMXPMSG_NAQSEVENT			= 260,
    NAQSTRIGGER				= 259

};


/*! \brief Header for all messages. */
typedef struct nmxp_MessageHeader
{
  uint32_t signature;
  uint32_t type;
  uint32_t length;
} nmxp_MessageHeader;


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
int nmxp_sendHeader(int isock, enum NMXP_MSG_CLIENT type, uint32_t length);


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
int nmxp_receiveHeader(int isock, enum NMXP_MSG_SERVER *type, uint32_t *length);


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
int nmxp_sendMessage(int isock, enum NMXP_MSG_CLIENT type, void *buffer, uint32_t length);


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
int nmxp_receiveMessage(int isock, enum NMXP_MSG_SERVER *type, void **buffer, uint32_t *length);


/*! \brief A generic logging/printing routine
 * 
 *   This function works in two modes:
 *
 *   -# Initialization, expecting 2 arguments with the first (level)
 *       being -1 and the second being verbosity.  This will set the
 *       verbosity for all future calls, the default is 0.  Can be used
 *       to change the verbosity at any time. I.e. 'sl_log(-1,2);'
 *   -# Expecting 3+ arguments, log level, verbosity level, printf
 *       format, and printf arguments.  If the verbosity level is less
 *       than or equal to the set verbosity (see mode 1), the printf
 *       format and arguments will be printed at the appropriate log
 *       level. I.e. 'sl_log(0, 0, "error: %s", result);'
 *
 *   \retval new_verbosity if using mode 1.
 *   \retval n the number of characters formatted on success, and a
 *     a negative value on error if using mode 2.
 *
 *   \param level
 *   \param verb
 *   \param ...
 */
int nmxp_gen_log(int level, int verb, ... );

#endif

