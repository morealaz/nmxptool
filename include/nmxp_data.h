/*! \file
 *
 * \brief Data for Nanometrics Protocol Libray
 *
 * Author:
 * 	Matteo Quintiliani
 * 	Istituto Nazionale di Geofisica e Vulcanologia - Italy
 *	quintiliani@ingv.it
 *
 */

#ifndef NMXP_DATA_H
#define NMXP_DATA_H 1

#include "nmxp_chan.h"

#include <sys/types.h>

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
typedef struct {
    int key;		/*!< \brief Channel Key */
    char *sta;		/*!< \brief Station code */
    char *chan;		/*!< \brief Channel code */
    int packet_type;	/*!< \brief Packet type */
    int x0;		/*!< \brief First sample */
    int seq_no;		/*!< \brief Sequence number */
    double time;	/*!< \brief Time first sample. Epochs. */
    void *buffer;	/*!< \brief Nanometrics packet data  */
    int length;		/*!< \brief Packet length */
    int *pDataPtr;	/*!< \brief Array of samples */
    int nSamp;		/*!< \brief Number or samples */
    int sampRate;	/*!< \brief Sample rate */
} NMXP_PROCESS_DATA;


/*! \brief Initialize a structure NMXP_PROCESS_DATA
 *
 *  \param pd
 *
 */
int nmxp_init_process_data(NMXP_PROCESS_DATA *pd);


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


/*! \brief Process Compressed Data message by function func_processData().
 *
 * \param buffer_data Pointer to the data buffer containing Compressed Nanometrics packets.
 * \param length_data Buffer length in bytes.
 * \param channelList Pointer to the Channel List.
 * \param func_processData Pointer to the function manages data extracted. It could be NULL.
 *
 */
void nmxp_processCompressedDataFunc(char* buffer_data, int length_data, NMXP_CHAN_LIST *channelList,
	int (*func_processData)(NMXP_PROCESS_DATA *pd)
	);


/*! \brief Process decompressed Data message by function func_processData().
 *
 * \param buffer_data Pointer to the data buffer containing Decompressed Nanometrics packets.
 * \param length_data Buffer length in bytes.
 * \param channelList Pointer to the Channel List.
 * \param func_processData Pointer to the function manages data extracted. It could be NULL.
 *
 */
void nmxp_processDecompressedDataFunc(char* buffer_data, int length_data, NMXP_CHAN_LIST *channelList,
	int (*func_processData)(NMXP_PROCESS_DATA *pd)
  );


#endif

