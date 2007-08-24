/*! \file
 *
 * \brief Channels for Nanometrics Protocol Library
 *
 * Author:
 * 	Matteo Quintiliani
 * 	Istituto Nazionale di Geofisica e Vulcanologia - Italy
 *	quintiliani@ingv.it
 *
 */

#ifndef NMXP_CHAN_H
#define NMXP_CHAN_H 1

#include <stdint.h>

/*! \brief Channel list */
typedef struct NMXP_META_CHAN_LIST {
    int32_t key;
    char name[12];
    int32_t start_time;
    int32_t end_time;
    char network[12];
    struct NMXP_META_CHAN_LIST *next;
} NMXP_META_CHAN_LIST;

typedef enum {
    NMXP_META_SORT_KEY = 1,
    NMXP_META_SORT_NAME,
    NMXP_META_SORT_START_TIME,
    NMXP_META_SORT_END_TIME
} NMXP_META_CHAN_LIST_SORT_TYPE;


/*! \brief The key/name info for one channel */
typedef struct {
    int32_t key;
    char name[12];
} NMXP_CHAN_KEY;

/*! \brief Max number of channels */
#define MAX_N_CHAN 1000

/*! \brief Channel list */
typedef struct {
    int32_t number;
    NMXP_CHAN_KEY channel[MAX_N_CHAN];
} NMXP_CHAN_LIST;

/*! \brief Precis Channel item */
typedef struct {
    int32_t key;
    char name[12];
    int32_t start_time;
    int32_t end_time;
} NMXP_CHAN_PRECISITEM;

/*! \brief Precis Channel list */
typedef struct {
    int32_t number;
    NMXP_CHAN_PRECISITEM channel[MAX_N_CHAN];
} NMXP_CHAN_PRECISLIST;

/*! \brief Type of Data */
typedef enum {
    NMXP_DATA_TIMESERIES	= 1,
    NMXP_DATA_SOH		= 2,
    NMXP_DATA_TRANSERIAL	= 6
} NMXP_DATATYPE;

/*! \brief Precis list request body */
typedef struct {
    int32_t instr_id;
    NMXP_DATATYPE datatype;
    int32_t type_of_channel;
} NMXP_MSGBODY_PRECISLISTREQUEST;

/*! \brief Channel info request body */
typedef struct {
    int32_t key;
    char name[12];
    char network[12];
} NMXP_MSGBODY_CHANNELINFORESPONSE;

/*! \brief Channel info request body */
typedef struct {
    int32_t key;
    int32_t ignored;
} NMXP_MSGBODY_CHANNELINFOREQUEST;


/*! \brief Character separator for channel list */
#define sep_chan_list  ','


/*! \brief Return type of data from a channel key */
#define getDataTypeFromKey(key) ((key >> 8) & 0xff)

/*! \brief Return channel number from a channel key */
#define getChannelNumberFromKey(key) ((key) & 0x000f)



/*! \brief Copy station code and channel code from name
 *
 * \param station_dot_channel string containing STA.CHAN
 * \param[out] station_code Pointer to string for station code
 * \param[out] channel_code Pointer to string for char code
 *
 * \warning Parametes can not be NULL!
 *
 * \retval 1 on success
 * \retval 0 on error
 *
 */
int nmxp_chan_cpy_sta_chan(const char *station_dot_channel, char *station_code, char *channel_code);


/*! \brief Match station_dot_channel against pattern, treating errors as no match.
 *
 * \param station_dot_channel STA.CHAN format
 * \param pattern STA.?HZ or STA.H?Z or STA.HH? or STA.?H? or ....
 *
 * \retval 1 for match
 * \retval 0 for no match
 * \retval -1 on error for invalid pattern
 * \retval -2 on error for invalid station_dot_channel
 *
 */
int nmxp_chan_match(const char *station_dot_channel, char *pattern);



/*! \brief Looks up a channel key in the list using the name
 *
 * \param name Channel name.
 * \param channelList Channel list.
 *
 * \return Key of the channel with name. -1 On error.
 *
 */
int nmxp_chan_lookupKey(char* name, NMXP_CHAN_LIST *channelList);


/*! \brief Looks up a channel name in the list using a key
 *
 * \param key Channel key.
 * \param channelList Channel list.
 *
 * \return Index of channel with key. -1 on error.
 *
 */
int nmxp_chan_lookupKeyIndex(int32_t key, NMXP_CHAN_LIST *channelList);


/*! \brief Looks up a channel name in the list using a key
 *
 * \param key Channel key.
 * \param channelList Channel list.
 *
 * \return Name of channel with key. NULL on error.
 *
 */
char *nmxp_chan_lookupName(int32_t key, NMXP_CHAN_LIST *channelList);


/*! \brief Looks up a channel with specified data type.
 *
 * \param channelList Channel list.
 * \param dataType Type of channel.
 *
 * \return Channel list with specified dataType. It will need to be freed!
 *
 * \warning Returned value will need to be freed!
 *
 */
NMXP_CHAN_LIST *nmxp_chan_getType(NMXP_CHAN_LIST *channelList, NMXP_DATATYPE dataType);


/*! \brief Looks up a channel with specified data type.
 *
 * \param channelList Channel list.
 * \param dataType Type of channel.
 * \param sta_chan_list String list of item STA.CHAN, separeted by comma.
 *
 * \return Channel list with specified dataType. It will need to be freed!
 *
 * \warning Returned value will need to be freed!
 *
 */
NMXP_CHAN_LIST *nmxp_chan_subset(NMXP_CHAN_LIST *channelList, NMXP_DATATYPE dataType, char *sta_chan_list);


/*! Sort list by channel key
 *
 * \param channelList Channel List
 *
 */
void nmxp_chan_sortByKey(NMXP_CHAN_LIST *channelList);


/*! Sort list by channel name
 *
 * \param channelList Channel List
 *
 */
void nmxp_chan_sortByName(NMXP_CHAN_LIST *channelList);


/*! Print channel information
 *
 * \param channelList Channel List
 *
 */
void nmxp_chan_print_channelList(NMXP_CHAN_LIST *channelList);


void nmxp_meta_chan_free(NMXP_META_CHAN_LIST **chan_list);

NMXP_META_CHAN_LIST *nmxp_meta_chan_add(NMXP_META_CHAN_LIST **chan_list, int32_t key, char *name, int32_t start_time, int32_t end_time, char *network, NMXP_META_CHAN_LIST_SORT_TYPE sorttype);

NMXP_META_CHAN_LIST *nmxp_meta_chan_search_key(NMXP_META_CHAN_LIST *chan_list, int32_t key);

NMXP_META_CHAN_LIST *nmxp_meta_chan_set_name(NMXP_META_CHAN_LIST *chan_list, int32_t key, char *name);

NMXP_META_CHAN_LIST *nmxp_meta_chan_set_times(NMXP_META_CHAN_LIST *chan_list, int32_t key, int32_t start_time, int32_t end_time);

NMXP_META_CHAN_LIST *nmxp_meta_chan_set_network(NMXP_META_CHAN_LIST *chan_list, int32_t key, char *network);

void nmxp_meta_chan_print(NMXP_META_CHAN_LIST *chan_list);

#endif

