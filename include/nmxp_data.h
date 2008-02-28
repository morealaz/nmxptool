/*! \file
 *
 * \brief Data for Nanometrics Protocol Library
 *
 * Author:
 * 	Matteo Quintiliani
 * 	Istituto Nazionale di Geofisica e Vulcanologia - Italy
 *	quintiliani@ingv.it
 *
 * $Id: nmxp_data.h,v 1.28 2008-02-28 14:01:17 mtheo Exp $
 *
 */

#ifndef NMXP_DATA_H
#define NMXP_DATA_H 1

#include <stdint.h>
#include <stdio.h>
#include <time.h>


/*! \brief struct tm plus ten thousandth second field */
typedef struct {
    struct tm t;
    uint32_t d;
} NMXP_TM_T;



/*! First 4 bytes of all messages. */
#define NMX_SIGNATURE 0x7abcde0f

/*! */
#define NMXP_DATA_IS_LEAP(yr)     ( yr%400==0 || (yr%4==0 && yr%100!=0) )


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
    int32_t signature;
    int32_t type;
    int32_t length;
} NMXP_MESSAGE_HEADER;


/*! \brief Length in bytes of channel strings */
#define NETWORK_LENGTH 10

/*! \brief Length in bytes of station strings */
#define STATION_LENGTH 10

/*! \brief Length in bytes of channel strings */
#define CHANNEL_LENGTH 10

/*! \brief Parameter structure for functions that process data */
typedef struct {
    int32_t key;			/*!< \brief Channel Key */
    char network[NETWORK_LENGTH];	/*!< \brief Network code */
    char station[STATION_LENGTH];	/*!< \brief Station code */
    char channel[CHANNEL_LENGTH];	/*!< \brief Channel code */
    int32_t packet_type;			/*!< \brief Packet type */
    int32_t x0;				/*!< \brief First sample. It is significant only if x0n_significant != 0 */
    int32_t xn;				/*!< \brief Last sample. It is significant only if x0n_significant != 0 */
    int32_t x0n_significant;			/*!< \brief Declare if xn significant */
    int32_t oldest_seq_no;			/*!< \brief Oldest Sequence number */
    int32_t seq_no;				/*!< \brief Sequence number */
    double time;			/*!< \brief Time first sample. Epochs. */
    void *buffer;			/*!< \brief Nanometrics packet data  */
    int32_t length;				/*!< \brief Packet length */
    int *pDataPtr;			/*!< \brief Array of samples */
    int32_t nSamp;				/*!< \brief Number or samples */
    int32_t sampRate;			/*!< \brief Sample rate */
} NMXP_DATA_PROCESS;


/*! \brief Parameter structure for functions that handle mini-seed records */
typedef struct {
    char srcname[50];
    FILE *outfile_mseed;
    char filename_mseed[500];
} NMXP_DATA_SEED;

/*! \brief Initialize a structure NMXP_DATA_PROCESS
 *
 *  \param pd Pointer to a NMXP_DATA_PROCESS structure.
 *
 */
int nmxp_data_init(NMXP_DATA_PROCESS *pd);


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
int nmxp_data_unpack_bundle (int32_t *outdata, unsigned char *indata, int32_t *prev);


/* \brief Value for parameter exclude_bitmap in the function nmxp_data_trim() */
#define NMXP_DATA_TRIM_EXCLUDE_FIRST 2

/* \brief Value for parameter exclude_bitmap in the function nmxp_data_trim() */
#define NMXP_DATA_TRIM_EXCLUDE_LAST  4

/*! \brief Convert epoch in string
 */
int nmxp_data_to_str(char *out_str, double time_d);


/*! \brief Trim data within a time interval
 *
 * \param pd Pointer to struct NMXP_DATA_PROCESS
 * \param trim_start_time Start time.
 * \param trim_end_time End time.
 * \param exclude_bitmap Bitmap for excluding or not the first and/or the last sample.
 *
 * \retval 2 On success, data has not been trimmed.
 * \retval 1 On success, data has been trimmed.
 * \retval 0 On error.
 *
 */
int nmxp_data_trim(NMXP_DATA_PROCESS *pd, double trim_start_time, double trim_end_time, unsigned char exclude_bitmap);


/*! \brief Return number of epochs in GMT 
 *
 */
time_t nmxp_data_gmtime_now();


/*! \brief Compute latency from current time and struct NMXP_DATA_PROCESS
 *
 * \param pd Pointer to struct NMXP_DATA_PROCESS
 *
 */
double nmxp_data_latency(NMXP_DATA_PROCESS *pd);


/*! \brief Print info about struct NMXP_DATA_PROCESS
 *
 * \param pd Pointer to struct NMXP_DATA_PROCESS
 * \param flag_sample If it is not equal to zero sample values will be printed
 *
 */
int nmxp_data_log(NMXP_DATA_PROCESS *pd, int flag_sample);


/*! \brief Parse string and set value in ret_tm
 *
 *
 */
int nmxp_data_parse_date(const char *pstr_date, NMXP_TM_T *ret_tmt);


/*! \brief Wrapper for timegm
 *
 *
 */
double nmxp_data_tm_to_time(NMXP_TM_T *tmt);


/*! \brief Initialize a structure NMXP_DATA_SEED
 *
 *  \param data_seed Pointer to a NMXP_DATA_SEED structure.
 *
 */
int nmxp_data_seed_init(NMXP_DATA_SEED *data_seed);


/*! \brief Write mini-seed records from a NMXP_DATA_PROCESS structure.
 *
 * \param pd Pointer to struct NMXP_DATA_PROCESS.
 * \param data_seed Pointer to struct NMXP_DATA_SEED.
 * \param pmsr Pointer to mini-SEED record.
 *
 * \warning pmsr is used like (void *) but it has to be a pointer to MSRecord !!!
 *
 * \return Returns the number records created on success and -1 on error. Return value of msr_pack().
 *
 */
int nmxp_data_msr_pack(NMXP_DATA_PROCESS *pd, NMXP_DATA_SEED *data_seed, void *pmsr);


/*! \brief Swap 2 bytes. 
 *
 * \param in Variable length 2 bytes.
 *
 */
void nmxp_data_swap_2b (int16_t *in);


/*! \brief Swap 3 bytes. 
 *
 * \param in Variable length 3 bytes.
 *
 */
void nmxp_data_swap_3b (unsigned char *in);


/*! \brief Swap 4 bytes. 
 *
 * \param in Variable length 4 bytes.
 *
 */
void nmxp_data_swap_4b (int32_t *in);


/*! \brief Swap 8 bytes. 
 *
 * \param in Variable length 8 bytes.
 *
 */
void nmxp_data_swap_8b (double *in);


/*! \brief Determine the byte order of the host machine. 
 *  Due to the lack of portable defines to determine host byte order this
 *  run-time test is provided.  The code below actually tests for
 *  little-endianess, the only other alternative is assumed to be big endian.
 *
 *  \retval 0 if the host is little endian.
 *  \retval 1 otherwise.
 */
int nmxp_data_bigendianhost ();


#endif

