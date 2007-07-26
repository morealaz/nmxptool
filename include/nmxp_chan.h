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
typedef struct nmxp_ChannelKey
{
    uint32_t key;
    char name[12];
} nmxp_ChannelKey;

#define MAX_N_CHAN 1000
/*! \brief Channel List */
typedef struct nmxp_ChannelList
{
    uint32_t number;
    nmxp_ChannelKey channel[MAX_N_CHAN];
} nmxp_ChannelList;


/*! \brief Looks up a channel key in the nmxp_ChannelList using the name
 *
 * \param name
 * \param channelList
 *
 * \retval
 *
 */
int lookupChannelKey(char* name, nmxp_ChannelList *channelList);

/*! \brief Looks up a channel name in the nmxp_ChannelList using a key
 *
 * \param key
 * \param channelList
 *
 * \retval
 *
 */
char *lookupChannelName(int key, nmxp_ChannelList *channelList);

#endif

