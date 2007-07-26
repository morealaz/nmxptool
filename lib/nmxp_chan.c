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

#include "nmxp_chan.h"

#include <string.h>

int nmxp_chan_lookupKey(char* name, NMXP_CHAN_LIST *channelList)
{
    int length = channelList->number;
    int ich = 0;

    for (ich = 0; ich < length; ich++)
    {
	if (strcasecmp(name, channelList->channel[ich].name) == 0)
	    return channelList->channel[ich].key;
    }

    return -1;
}

char *nmxp_chan_lookupName(uint32_t key, NMXP_CHAN_LIST *channelList)
{
    int length = channelList->number;
    int ich = 0;

    for (ich = 0; ich < length; ich++)
    {
	if ( key == channelList->channel[ich].key )
	    return &channelList->channel[ich].name[0];
    }

    return NULL;
}

