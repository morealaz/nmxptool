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

NMXP_CHAN_LIST *nmxp_chan_getType(NMXP_CHAN_LIST *channelList, NMXP_DATATYPE dataType) {
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


// Comparison Key Function
int chan_key_compare(const void *a, const void *b)
{
    int ret = 0;
    NMXP_CHAN_KEY *pa = (NMXP_CHAN_KEY *) a; 
    NMXP_CHAN_KEY *pb = (NMXP_CHAN_KEY *) b;

    if(pa->key > pb->key) {
	ret = 1;
    } else if(pa->key < pb->key) {
	ret = -1;
    }
    return ret;
}

void nmxp_chan_sortByKey(NMXP_CHAN_LIST *channelList) {
    qsort (channelList->channel, channelList->number, sizeof (NMXP_CHAN_KEY), chan_key_compare);
}

// Comparison Name Function
int chan_name_compare(const void *a, const void *b)
{
    NMXP_CHAN_KEY *pa = (NMXP_CHAN_KEY *) a; 
    NMXP_CHAN_KEY *pb = (NMXP_CHAN_KEY *) b;

    return strcmp(pa->name, pb->name);
}

void nmxp_chan_sortByName(NMXP_CHAN_LIST *channelList) {
    qsort (channelList->channel, channelList->number, sizeof (NMXP_CHAN_KEY), chan_name_compare);
}
