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
#include <stdlib.h>

int nmxp_chan_lookupKey(char* name, NMXP_CHAN_LIST *channelList)
{
    int chan_number = channelList->number;
    int i_chan = 0;

    for (i_chan = 0; i_chan < chan_number; i_chan++)
    {
	if (strcasecmp(name, channelList->channel[i_chan].name) == 0)
	    return channelList->channel[i_chan].key;
    }

    return -1;
}

char *nmxp_chan_lookupName(uint32_t key, NMXP_CHAN_LIST *channelList)
{
    int chan_number = channelList->number;
    int i_chan = 0;

    for (i_chan = 0; i_chan < chan_number; i_chan++)
    {
	if ( key == channelList->channel[i_chan].key )
	    return &channelList->channel[i_chan].name[0];
    }

    return NULL;
}

NMXP_CHAN_LIST *nmxp_chan_getType(NMXP_CHAN_LIST *channelList, uint8_t dataType) {
    NMXP_CHAN_LIST *ret_channelList = NULL;

    int chan_number = channelList->number;
    int i_chan = 0;

    ret_channelList = (NMXP_CHAN_LIST *) malloc(sizeof(NMXP_CHAN_LIST));
    ret_channelList->number = 0;

    for (i_chan = 0; i_chan < chan_number; i_chan++)
    {
	if ( ((channelList->channel[i_chan].key >> 8) & 0xff) == dataType) {
	    ret_channelList->channel[ret_channelList->number].key = channelList->channel[i_chan].key;
	    strcpy(ret_channelList->channel[ret_channelList->number].name, channelList->channel[i_chan].name);
	    ret_channelList->number++;
	}
    }

    return ret_channelList;
}

