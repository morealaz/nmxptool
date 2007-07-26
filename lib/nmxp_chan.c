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

int lookupChannelKey(char* name, nmxp_ChannelList *channelList)
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

char *lookupChannelName(int key, nmxp_ChannelList *channelList)
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

