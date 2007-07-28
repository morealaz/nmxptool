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

#include "nmxp_chan.h"

/*! Maximum time between connection attempts (seconds). */
#define NMXP_SLEEPMAX 10

/*! Return message for succes on socket. */
#define NMXP_SOCKET_OK          0

/*! Return message for error on socket.*/
#define NMXP_SOCKET_ERROR      -1


/*! First 4 bytes of all messages. */
#define NMX_SIGNATURE 0x7abcde0f

/*! \brief Defines the type for reason of shutdown */
typedef enum {
    NMXP_SHUTDOWN_NORMAL		= 1,
    NMXP_SHUTDOWN_ERROR			= 2,
    NMXP_SHUTDOWN_TIMEOUT		= 3
} NMXP_SHUTDOWN_REASON;

/*! \brief Defines the client message types */
typedef enum {
   NMXP_MSG_CONNECT			= 100,
   NMXP_MSG_REQUESTPENDING		= 110,
   NMXP_MSG_CANCELREQUEST		= 205,
   NMXP_MSG_TERMINATESUBSCRIPTION	= 200,

   NMXP_MSG_ADDTIMESERIESCHANNELS	= 120,
   NMXP_MSG_ADDSOHCHANNELS		= 121,
   NMXP_MSG_ADDSERIALCHANNELS		= 124,
   NMXP_MSG_ADDTRIGGERCHANNELS		= 122,
   NMXP_MSG_ADDEVENTS			= 123,

   NMXP_MSG_REMOVETIMESERIESCHANNELS	= 130,
   NMXP_MSG_REMOVESOHCHANNELS		= 131,
   NMXP_MSG_REMOVESERIALCHANNELS	= 134,
   NMXP_MSG_REMOVETRIGGERCHANNELS	= 132,
   NMXP_MSG_REMOVEEVENTS		= 133,

   NMXP_MSG_CONNECTREQUEST		= 206,
   NMXP_MSG_CHANNELLISTREQUEST		= 209,
   NMXP_MSG_PRECISLISTREQUEST		= 203,
   NMXP_MSG_CHANNELINFOREQUEST		= 226,
   NMXP_MSG_DATASIZEREQUEST		= 229,
   NMXP_MSG_DATAREQUEST			= 227,
   NMXP_MSG_TRIGGERREQUEST		= 231,
   NMXP_MSG_EVENTREQUEST		= 232
   
} NMXP_MSG_CLIENT;

/*! \brief Defines the server message types. */
typedef enum {
    NMXP_MSG_CHANNELLIST		= 150,
    NMXP_MSG_ERROR			= 190,
    NMXP_MSG_COMPRESSED			= 1,
    NMXP_MSG_DECOMPRESSED		= 4,
    NMXP_MSG_TRIGGER			= 5,
    NMXP_MSG_EVENT			= 6,

    NMXP_MSG_READY			= 208,
    NMXP_MSG_PRECISLIST			= 253,
    NMXP_MSG_CHANNELHEADER		= 256,
    NMXP_MSG_DATASIZE			= 257,
    NMXP_MSG_NAQSEVENT			= 260,
    NMXP_MSG_NAQSTRIGGER		= 259

} NMXP_MSG_SERVER;


/*! \brief Header for all messages. */
typedef struct {
    uint32_t signature;
    uint32_t type;
    uint32_t length;
} NMXP_MESSAGE_HEADER;


/*! \brief Parameter structure for functions that process data */
Parameter structure struct {
    int key;		/*!< \brief Channel Key */
    char *sta;		/*!< \brief Station code */
    char *chan;		/*!< \brief Channel code */
    double time;	/*!< \brief Time first sample. Epochs. */
    void *buffer;	/*!< \brief Nanometrics packet data  */
    int length;		/*!< \brief Packet length */
    int *pDataPtr;	/*!< \brief Array of samples */
    int nSamp;		/*!< \brief Number or samples */
    int sampRate;	/*!< \brief Sample rate */
} NMXP_PROCESS_DATA;


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
int nmxp_sendHeader(int isock, NMXP_MSG_CLIENT type, uint32_t length);


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
int nmxp_receiveHeader(int isock, NMXP_MSG_SERVER *type, uint32_t *length);


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
int nmxp_sendMessage(int isock, NMXP_MSG_CLIENT type, void *buffer, uint32_t length);


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
int nmxp_receiveMessage(int isock, NMXP_MSG_SERVER *type, void **buffer, uint32_t *length);


/*! \brief Unpack a 17-byte Nanometrics compressed data bundle.           
 *
 * \param[out] outdata
 * \param indata
 * \param prev
 *
 * \return Number of unpacked data samples, -1 if null bundle. 
 *
 * Author:  Doug Neuhauser
 *          UC Berkeley Seismological Laboratory
 *          doug@seismo.berkeley.edu
 *
 */
int nmxp_unpack_bundle (int *outdata, unsigned char *indata, int *prev);

/*! \brief Print info about struct NMXP_PROCESS_DATA
 *
 * \param pd Pointer to struct NMXP_PROCESS_DATA
 *
 */
int nmxp_log_process_data(NMXP_PROCESS_DATA *pd);


/*! \brief Process Compressed Data message
 *
 */
void nmxp_processCompressedDataFunc(char* buffer_data, int length_data, NMXP_CHAN_LIST *channelList,
	int (*func_processData)(NMXP_PROCESS_DATA *pd)
	);


/*! \brief Process decompressed Data message
 *
 * \param buffer
 * \param length
 * \param channelList
 * \param func_processData
 *
 */
void nmxp_processDecompressedDataFunc(char* buffer, int length, NMXP_CHAN_LIST *channelList,
	int (*func_processData)(NMXP_PROCESS_DATA *pd)
  );


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
int nmxp_log(int level, int verb, ... );

#endif

