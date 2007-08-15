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

int nmxp_chan_cpy_sta_chan(const char *station_dot_channel, char *station_code, char *channel_code) {
    int ret = 0;
    char *sta_code_tmp, *cha_code_tmp;

    if(station_dot_channel || station_code || channel_code) {
	station_code[0] = 0;
	channel_code[0] = 0;

	/* validate pattern channel */
	sta_code_tmp = strdup(station_dot_channel);
	if( (cha_code_tmp = strchr(sta_code_tmp, '.')) == NULL ) {
	    nmxp_log(1, 0, "Name %s is not in STA.CHAN format!\n", station_dot_channel);
	} else {
	    if(cha_code_tmp) {
		*cha_code_tmp++ = '\0';
		strcpy(station_code, sta_code_tmp);
		strcpy(channel_code, cha_code_tmp);
		free(sta_code_tmp);
		ret = 1;
	    } else {
		nmxp_log(1, 0, "Name %s is not in STA.CHAN format! Channel is missing?\n", station_dot_channel);
	    }
	}
    } else {
	nmxp_log(1, 0, "Some parameter is NULL in nmxp_chan_cpy_sta_chan().\n",  station_dot_channel);
    }

    return ret;
}



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
    char cha_pattern[20];
    char sta_sdc[20];
    char *cha_sdc;

    /* validate pattern channel */
    if(!nmxp_chan_cpy_sta_chan(pattern, sta_pattern, cha_pattern)) {
	nmxp_log(1, 0, "Channel pattern %s is not in STA.CHAN format!\n", pattern);
	return -1;
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


int nmxp_chan_lookupKeyIndex(int32_t key, NMXP_CHAN_LIST *channelList)
{
    int i_chan = 0;
    int ret = -1;

    i_chan = 0;
    while(i_chan < channelList->number  &&  ret == -1)
    {
	if ( key == channelList->channel[i_chan].key ) {
	    ret = i_chan;
	}
	i_chan++;
    }

    return ret;
}


char *nmxp_chan_lookupName(int32_t key, NMXP_CHAN_LIST *channelList)
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


void nmxp_meta_chan_free(NMXP_META_CHAN_LIST **chan_list) {
    NMXP_META_CHAN_LIST *iter = *chan_list;
    NMXP_META_CHAN_LIST *iter_next = NULL;

    nmxp_log(0, 1, "nmxp_meta_chan_free()\n");

    if(iter) {
	iter_next = iter->next;
	while(iter) {
	    free(iter);
	    iter = iter_next;
	    iter_next = iter->next;
	}
	*chan_list = NULL;
    }

}

int nmxp_meta_chan_compare(NMXP_META_CHAN_LIST *item1, NMXP_META_CHAN_LIST *item2) {
    int ret = 0;
    if(item1->key > item2->key) {
	ret = 1;
    } else if(item1->key < item2->key) {
	ret = -1;
    }
    return ret;
}

NMXP_META_CHAN_LIST *nmxp_meta_chan_add(NMXP_META_CHAN_LIST **chan_list, int32_t key, char *name, int32_t start_time, int32_t end_time, char *network) {
    NMXP_META_CHAN_LIST *iter = NULL;
    NMXP_META_CHAN_LIST *new_item = NULL;

    nmxp_log(0, 1, "nmxp_meta_chan_add(%d, %d, %s, %d, %d, %s)\n", *chan_list, key, name, start_time, end_time, network);

    new_item = (NMXP_META_CHAN_LIST *) malloc(sizeof(NMXP_META_CHAN_LIST));
    new_item->key = 0;
    new_item->name[0] = 0;
    new_item->start_time = 0;
    new_item->end_time = 0;
    new_item->network[0] = 0;
    new_item->next = NULL;
    new_item->key = key;
    if(name) {
	strncpy(new_item->name, name, 12);
    }
    new_item->start_time = start_time;
    new_item->end_time = end_time;
    if(network) {
	strncpy(new_item->network, network, 12);
    }


    if(*chan_list == NULL) {
	*chan_list = new_item;
    } else {
	if(nmxp_meta_chan_compare(new_item, *chan_list) < 0) {
	    new_item->next = *chan_list;
	    *chan_list = new_item;
	} else {
	    for(iter = *chan_list; iter->next != NULL  && nmxp_meta_chan_compare(new_item, iter->next) > 0; iter = iter->next) {
	    }
	    new_item->next = iter->next;
	    iter->next = new_item;
	}
    }

    return new_item;
}

NMXP_META_CHAN_LIST *nmxp_meta_chan_search_key(NMXP_META_CHAN_LIST *chan_list, int32_t key) {
    NMXP_META_CHAN_LIST *iter = chan_list;
    int found = 0;

    nmxp_log(0, 1, "nmxp_meta_chan_search_key()\n");

    while(iter != NULL  &&  !found) {
	if(iter->key == key) {
	    found = 1;
	} else {
	    iter = iter->next;
	}
    }

    return iter;
}

NMXP_META_CHAN_LIST *nmxp_meta_chan_set_name(NMXP_META_CHAN_LIST *chan_list, int32_t key, char *name) {
    NMXP_META_CHAN_LIST *ret = NULL;

    nmxp_log(0, 1, "nmxp_meta_chan_set_name()\n");

    if( (ret = nmxp_meta_chan_search_key(chan_list, key)) ) {
	strncpy(ret->name, name, 12);
    }

    return ret;
}

NMXP_META_CHAN_LIST *nmxp_meta_chan_set_times(NMXP_META_CHAN_LIST *chan_list, int32_t key, int32_t start_time, int32_t end_time) {
    NMXP_META_CHAN_LIST *ret = NULL;

    nmxp_log(0, 1, "nmxp_meta_chan_set_times()\n");

    if( (ret = nmxp_meta_chan_search_key(chan_list, key)) ) {
	ret->start_time = start_time;
	ret->end_time = end_time;
    }

    return ret;
}

NMXP_META_CHAN_LIST *nmxp_meta_chan_set_network(NMXP_META_CHAN_LIST *chan_list, int32_t key, char *network) {
    NMXP_META_CHAN_LIST *ret = NULL;

    nmxp_log(0, 1, "nmxp_meta_chan_set_network()\n");

    if( (ret = nmxp_meta_chan_search_key(chan_list, key)) ) {
	strncpy(ret->network, network, 12);
    }

    return ret;
}

void nmxp_meta_chan_print(NMXP_META_CHAN_LIST *chan_list) {
    NMXP_META_CHAN_LIST *iter = chan_list;
    char str_start_time[200], str_end_time[200];
    str_start_time[0] = 0;
    str_end_time[0] = 0;

    nmxp_log(0, 1, "nmxp_meta_chan_print()\n");

    while(iter != NULL) {
	nmxp_data_to_str(str_start_time, iter->start_time);
	nmxp_data_to_str(str_end_time,   iter->end_time);

	nmxp_log(0, 0, "%10d %11s.%-5s (%s  -  %s)\n",
		iter->key,
		iter->name,
		iter->network,
		str_start_time,
		str_end_time
		);
	iter = iter->next;
    }
}



