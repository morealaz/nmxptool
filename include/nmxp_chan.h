/*! \file
 *
 * \brief Channels for Nanometrics Protocol Libray
 *
 * Author:
 * 	Matteo Quintiliani
 * 	Istituto Nazionale di Geofisica e Vulcanologia - Italy
 *	quintiliani@ingv.it
 *
 */

#ifndef NMXP_CHAN_H
#define NMXP_CHAN_H 1

#include <sys/types.h>

/*! \brief The key/name info for one channel */
typedef struct {
    uint32_t key;
    char name[12];
} NMXP_CHAN_KEY;

/*! \brief Max number of channels */
#define MAX_N_CHAN 1000

/*! \brief Channel list */
typedef struct {
    uint32_t number;
    NMXP_CHAN_KEY channel[MAX_N_CHAN];
} NMXP_CHAN_LIST;


/*! \brief Type of Data */
typedef enum {
    NMXP_DATA_TIMESERIES	= 1,
    NMXP_DATA_SOH		= 2,
    NMXP_DATA_TRANSERIAL	= 6
} NMXP_DATATYPE;


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
 * \return Name of channel with key. NULL on error.
 *
 */
char *nmxp_chan_lookupName(uint32_t key, NMXP_CHAN_LIST *channelList);

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

#endif

