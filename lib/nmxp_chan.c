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

#include "nmxp_chan.h"
#include "nmxp_base.h"

#include <string.h>
#include <stdlib.h>


/*
 * Match string against the extended regular expression in
 * pattern, treating errors as no match.
 *
 * return 1 for match, 0 for no match, -1 on error for invalid pattern, -2 on error for invalid station_dot_channel
 */
int nmxp_chan_match(const char *station_dot_channel, char *pattern)
{
    int ret = 0;
    int i, l;
    char sta_pattern[20];
    char *cha_pattern;
    char sta_sdc[20];
    char *cha_sdc;

    /* validate pattern channel */
    strcpy(sta_pattern, pattern);
    if( (cha_pattern = strchr(sta_pattern, '.')) == NULL ) {
	nmxp_log(1, 0, "Channel pattern %s is not in STA.CHAN format!\n", pattern);
	return -1;
    }
    if(cha_pattern) {
	*cha_pattern++ = '\0';
    }

    l = strlen(sta_pattern);
    i = 0;
    while(i < l  &&  ret != -1) {
	if(  !(
		(sta_pattern[i] >= 'A'  &&  sta_pattern[i] <= 'Z')
		|| (sta_pattern[i] >= 'a'  &&  sta_pattern[i] <= 'z')
		|| (sta_pattern[i] >= '0'  &&  sta_pattern[i] <= '9')
		|| (sta_pattern[i] == '_' )
	     )
	  ) {
	    nmxp_log(1, 0, "Channel pattern %s has not valid STA format!\n", pattern);
	    return -1;
	}
	i++;
    }
    
    l = strlen(cha_pattern);
    if(l != 3) {
	nmxp_log(1, 0, "Channel pattern %s has not valid CHAN format!\n", pattern);
	return -1;
    }
    i = 0;
    while(i < l  &&  ret != -1) {
	if(  !(
		    (cha_pattern[i] >= 'A'  &&  cha_pattern[i] <= 'Z')
		    || (cha_pattern[i] >= 'a'  &&  cha_pattern[i] <= 'z')
		    || (cha_pattern[i] == '_' )
		    || (cha_pattern[i] == '?' )
	      )
	  ) {
	    nmxp_log(1, 0, "Channel pattern %s has not valid CHAN format!\n", pattern);
	    return -1;
	}
	i++;
    }

    strcpy(sta_sdc, station_dot_channel);
    if( (cha_sdc = strchr(sta_sdc, '.')) == NULL ) {
	nmxp_log(1, 0, "Channel %s is not in STA.CHAN format!\n", station_dot_channel);
	return -2;
    }
    if(cha_sdc) {
	*cha_sdc++ = '\0';
    }
    l = strlen(cha_sdc);
    if(l != 3) {
	nmxp_log(1, 0, "Channel %s has not valid CHAN format!\n", pattern);
	return -1;
    }

    if (strcasecmp(sta_sdc, sta_pattern) == 0) {
	/* matching CHAN */
	ret = 1;
	i = 0;
	while(i < 3  &&  ret != 0) {
	    ret = ((cha_pattern[i] == '?')? 1 : (cha_pattern[i] == cha_sdc[i]));
	    i++;
	}
    }


    return ret;
}


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
    int i_chan = 0;
    static char ret[12];

    ret[0] = 0;

    for (i_chan = 0; i_chan < channelList->number; i_chan++)
    {
	if ( key == channelList->channel[i_chan].key ) {
	    strcpy(ret, channelList->channel[i_chan].name);
	}
    }

    if(ret[0] == 0) {
	nmxp_log(1, 0, "Key %d not found!", key);
	return NULL;
    } else {
	return ret;
    }
}

NMXP_CHAN_LIST *nmxp_chan_getType(NMXP_CHAN_LIST *channelList, NMXP_DATATYPE dataType) {
    NMXP_CHAN_LIST *ret_channelList = NULL;

    int chan_number = channelList->number;
    int i_chan = 0;

    ret_channelList = (NMXP_CHAN_LIST *) malloc(sizeof(NMXP_CHAN_LIST));
    ret_channelList->number = 0;

    for (i_chan = 0; i_chan < chan_number; i_chan++)
    {
	if ( getDataTypeFromKey(channelList->channel[i_chan].key) == dataType) {
	    ret_channelList->channel[ret_channelList->number].key = channelList->channel[i_chan].key;
	    strcpy(ret_channelList->channel[ret_channelList->number].name, channelList->channel[i_chan].name);
	    ret_channelList->number++;
	}
    }

    return ret_channelList;
}


NMXP_CHAN_LIST *nmxp_chan_subset(NMXP_CHAN_LIST *channelList, NMXP_DATATYPE dataType, char *sta_chan_list) {
    NMXP_CHAN_LIST *ret_channelList = NULL;
    int istalist, ista;
    char sta_chan_code_pattern[100];
    int i_chan, ret_match;

    ret_channelList = (NMXP_CHAN_LIST *) malloc(sizeof(NMXP_CHAN_LIST));
    ret_channelList->number = 0;

    istalist = 0;
    while(sta_chan_list[istalist] != sep_chan_list  &&  sta_chan_list[istalist] != 0) {
	ista = 0;
	while(sta_chan_list[istalist] != sep_chan_list  &&  sta_chan_list[istalist] != 0) {
	    sta_chan_code_pattern[ista++] = sta_chan_list[istalist++];
	}
	sta_chan_code_pattern[ista] = 0;
	if(sta_chan_list[istalist] == sep_chan_list) {
	    istalist++;
	}
	ret_match = 1;
	i_chan = 0;
	while(i_chan < channelList->number  &&  ret_match != -1) {
	    ret_match = nmxp_chan_match(channelList->channel[i_chan].name, sta_chan_code_pattern);
	    if(ret_match) {
		    if(i_chan != -1  && getDataTypeFromKey(channelList->channel[i_chan].key) == dataType) {
			ret_channelList->channel[ret_channelList->number].key =        channelList->channel[i_chan].key;
			strcpy(ret_channelList->channel[ret_channelList->number].name, channelList->channel[i_chan].name);
			ret_channelList->number++;
		    }
	    }
	    i_chan++;
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

void nmxp_chan_print_channelList(NMXP_CHAN_LIST *channelList) {
    int chan_number = channelList->number;
    int i_chan = 0;

    nmxp_log(0, 0, "%04d channels:\n", chan_number);

    for (i_chan = 0; i_chan < chan_number; i_chan++)
    {
	nmxp_log(0, 0, "%04d %12d %s\n", i_chan+1, channelList->channel[i_chan].key, channelList->channel[i_chan].name);
    }

}

