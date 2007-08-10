/*! \file
 *
 * \brief Data for Nanometrics Protocol Library
 *
 * Author:
 * 	Matteo Quintiliani
 * 	Istituto Nazionale di Geofisica e Vulcanologia - Italy
 *	quintiliani@ingv.it
 *
 */

#include "nmxp_data.h"
#include "nmxp_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "config.h"

#ifdef HAVE_LIBMSEED
#include <libmseed.h>
#endif


int nmxp_data_init(NMXP_DATA_PROCESS *pd) {
    pd->key = -1;
    pd->network[0] = 0;
    pd->station[0] = 0;
    pd->channel[0] = 0;
    pd->packet_type = -1;
    pd->x0 = -1;
    pd->xn = -1;
    pd->x0n_significant = 0;
    pd->oldest_seq_no = -1;
    pd->seq_no = -1;
    pd->time = -1.0;
    pd->buffer = NULL;
    pd->length = 0;
    pd->pDataPtr = NULL;
    pd->nSamp = 0;
    pd->sampRate = -1;
    return 0;
}


int nmxp_data_unpack_bundle (int *outdata, unsigned char *indata, int *prev)
{         
	int nsamples = 0;
	int d4[4];
	short int d2[2];
	int cb[4];  
	//mtheo int i, j, k;
	int i, j, k=0;
	unsigned char cbits;
	/* TOREMOVE int my_order = get_my_wordorder(); */
	int my_host_is_bigendian = nmxp_data_bigendianhost();

	cbits = (unsigned char)indata[0];
	if (cbits == 9) return (-1);
	++indata;

	/* Get the compression bits for the bundle. */
	for (i=0,j=6; j>=0; i++,j-=2) {
		cb[i] = (cbits>>j) & 3;
	}       

	for (j=0; j<4; j++) {
		/*
		nmxp_log (0,1, "cb[%d]=%d\n", j, cb[j]);
		*/
		switch (cb[j]) 
		{   
			case 0:       /* not used     */
				k=0;
				break;       
			case 1:       /* 4 byte diffs */
				d4[0] = (signed char)indata[0];
				d4[1] = (signed char)indata[1];
				d4[2] = (signed char)indata[2];
				d4[3] = (signed char)indata[3];
				k=4;
				break;
			case 2:       /* 2 16-bit diffs */
				memcpy (&d2[0],indata,2);
				memcpy (&d2[1],indata+2,2);
				/* TOREMOVE if (my_order != SEED_LITTLE_ENDIAN) { */
				if (my_host_is_bigendian) {
					nmxp_data_swap_2b (&d2[0]);
					nmxp_data_swap_2b (&d2[1]);
				}
				d4[0] = d2[0];
				d4[1] = d2[1];
				k=2;
				break;
			case 3:       /* 1 32-bit diff */
				memcpy (&d4[0],indata,4);
				/* TOREMOVE if (my_order != SEED_LITTLE_ENDIAN) { */
				if (my_host_is_bigendian) {
					nmxp_data_swap_4b (&d4[0]);
				}
				k=1;
				break;
		}
		indata += 4;

		for (i=0; i<k; i++) {
			*outdata = *prev + d4[i];
			/*
				nmxp_log(0, 1, "val = %d, diff[%d] = %d, *prev = %d\n",
						*outdata, i, d4[i], *prev);
						*/
			*prev = *outdata;

			outdata++;
			++nsamples;
		}
	}
	return (nsamples);
}


int nmxp_data_to_str(char *out_str, double time_d) {
    time_t time_t_start_time = (time_t) time_d;
    struct tm *tm_start_time = gmtime(&time_t_start_time);
    
    // sprintf(out_str, "%04d/%02d/%02d %02d:%02d:%02d.%04d",
    sprintf(out_str, "%04d.%d %02d:%02d:%02d.%04d",
	    tm_start_time->tm_year + 1900,
	    /*
	    tm_start_time->tm_mon + 1,
	    tm_start_time->tm_mday,
	    */
	    tm_start_time->tm_yday + 1,
	    tm_start_time->tm_hour,
	    tm_start_time->tm_min,
	    tm_start_time->tm_sec,
	    (int) (  ((time_d - (double) time_t_start_time)) * 10000.0 )
	   );
    
    return 0;
}

int nmxp_data_log(NMXP_DATA_PROCESS *pd) {

    char str_start[200], str_end[200];
    double latency = 0.0;

    time_t time_now;
    struct tm *tm_now;
    time(&time_now);
    tm_now = gmtime(&time_now);
    time_now = nmxp_data_tm_to_time(tm_now);

    str_start[0] = 0;
    str_end[0] = 0;
    
    if(pd) {
	nmxp_data_to_str(str_start, pd->time);
	nmxp_data_to_str(str_end, pd->time + ((double) pd->nSamp / (double) pd->sampRate));

	latency = ((double) time_now) - (pd->time + ((double) pd->nSamp / (double) pd->sampRate));

	// nmxp_log(0, 0, "%12d %5s.%3s rate=%03d (%s - %s) [%d, %d] pts=%04d (%d, %d, %d, %d) lat=%.1f len=%d\n",
	// printf("%10d %5s.%3s 03dHz (%s - %s) lat=%.1fs [%d, %d] pts=%04d (%d, %d, %d, %d) len=%d\n",
	printf("%s.%s.%3s %3dHz (%s - %s) lat %.1fs [%d, %d] (%d) %4dpts (%d, %d, %d, %d, %d) %d\n",
		/* pd->key, */
		pd->network,
		(strlen(pd->station) == 0)? "XXXX" : pd->station,
		(strlen(pd->channel) == 0)? "XXX" : pd->channel,
		pd->sampRate,
		str_start,
		str_end,
		latency,
		pd->packet_type,
		pd->seq_no,
		pd->oldest_seq_no,
		pd->nSamp,
		pd->x0,
		(pd->pDataPtr == NULL)? 0 : pd->pDataPtr[0],
		(pd->pDataPtr == NULL || pd->nSamp < 1)? 0 : pd->pDataPtr[pd->nSamp-1],
		pd->xn,
		pd->x0n_significant,
		pd->length
	      );
    } else {
	printf("Pointer to NMXP_DATA_PROCESS is NULL!\n");
    }

    return 0;
}


int nmxp_data_parse_date(const char *pstr_date, struct tm *ret_tm) {
/* Input formats: 
 *     <date>,<time> | <date>
 *
 * where:
 *     <date> = yyyy/mm/dd | yyy.jjj
 *     <time> = hh:mm:ss | hh:mm
 *
 *     yyyy = year
 *     mm   = month       (1-12)
 *     dd   = day         (1-31)
 *     jjj  = day-of-year (1-365)
 *     hh   = hour        (0-23)
 *     mm   = minute      (0-59)
 *     ss   = second      (0-59)
 */

    int ret = 0;

#define MAX_LENGTH_STR_MESSAGE 30
    char str_date[MAX_LENGTH_STR_MESSAGE] = "NO DATE";

#define MAX_LENGTH_ERR_MESSAGE 500
    char err_message[MAX_LENGTH_ERR_MESSAGE] = "NO MESSAGE";

    char *pEnd = NULL;
    long int app;
    int state;
    int flag_finished = 0;

    nmxp_log(0, 1, "Date to validate '%s'\n", pstr_date);
	
    strncpy(str_date, pstr_date, MAX_LENGTH_STR_MESSAGE);
    pEnd = str_date;
    app = strtol(str_date, &pEnd, 10);
    state = 0;
    if(  errno == EINVAL ||  errno == ERANGE ) {
	nmxp_log(1, 0, "%s\n", strerror(errno));
	ret = -1;
    }

    if(pEnd[0] != 0  &&  ret != -1) {
	ret = 0;
    } else {
	strncpy(err_message, "Error parsing year!", MAX_LENGTH_ERR_MESSAGE);
	ret = -1;
    }

    /* initialize ret_tm */
    time_t time_now;
    struct tm *tm_now;
    time(&time_now);
    tm_now = gmtime(&time_now);

    ret_tm->tm_sec = 0 ;
    ret_tm->tm_min = 0;
    ret_tm->tm_hour = 0;
    ret_tm->tm_mday = tm_now->tm_mday;
    ret_tm->tm_mon = tm_now->tm_mon;
    ret_tm->tm_year = tm_now->tm_year;
    ret_tm->tm_wday = tm_now->tm_wday;
    ret_tm->tm_yday = tm_now->tm_yday;
    ret_tm->tm_isdst = tm_now->tm_isdst;
    ret_tm->tm_gmtoff = tm_now->tm_gmtoff;

    
    /* loop for parsing by a finite state machine */
    while( 
	    !flag_finished
	    && ret == 0
	    &&  errno != EINVAL
	    &&  errno != ERANGE
	    ) {

    nmxp_log(0, 1, "state=%d value=%d flag_finished=%d ret=%d pEnd[0]=%c [%d]  (%s)\n", state, app, flag_finished, ret, (pEnd[0]==0)? '_' : pEnd[0], pEnd[0], pEnd);

	/* switch on state */
	switch(state) {
	    case 0: /* Parse year */
		ret_tm->tm_year = app - 1900;
		if(pEnd[0] == '/') {
		    state = 1; /* Month */
		} else if(pEnd[0] == '.') {
		    state = 3; /* Julian Day */
		} else {
		    strncpy(err_message, "Wrong separator after year!", MAX_LENGTH_ERR_MESSAGE);
		    ret = -1;
		}
		break;

	    case 1: /* Parse month */
		ret_tm->tm_mon = app - 1;
		if(pEnd[0] == '/')
		    state = 2; /* Day of month */
		else {
		    strncpy(err_message, "Wrong separator after month!", MAX_LENGTH_ERR_MESSAGE);
		    ret = -1;
		}
		break;

	    case 2: /* Parse day of month */
		ret_tm->tm_mday = app;
		if(pEnd[0] == 0) {
		    flag_finished = 1;
		} else if(pEnd[0] == ',') {
		    state = 4; /* Hour */
		} else {
			strncpy(err_message, "Wrong separator after day of month!", MAX_LENGTH_ERR_MESSAGE);
			ret = -1;
		    }
		break;

	    case 3: /* Parse Julian Day */
		ret_tm->tm_yday = app - 1;

		int month_days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
		int m, d, day_sum, jday=app;

		if(NMXP_DATA_IS_LEAP(ret_tm->tm_year)) {
		    month_days[1]++;
		}

		m=0;
		day_sum = 0;
		while(month_days[m] < (jday - day_sum)) {
		    day_sum += month_days[m++];
		}
		d = jday-day_sum;

		ret_tm->tm_mon = m;
		ret_tm->tm_mday = d;

		nmxp_log(0, 1, "Month %d Day %d\n", m, d);

		if(pEnd[0] == 0) {
		    flag_finished = 1;
		} else if(pEnd[0] == ',') {
		    state = 4; /* Hour */
		} else {
		    strncpy(err_message, "Wrong separator after julian day!", MAX_LENGTH_ERR_MESSAGE);
		    ret = -1;
		}
		break;

	    case 4: /* Parse hour */
		ret_tm->tm_hour = app;
		if(pEnd[0] == ':') {
		    state = 5; /* Minute */
		} else {
		    strncpy(err_message, "Wrong separator after hour!", MAX_LENGTH_ERR_MESSAGE);
		    ret = -1;
		}
		break;

	    case 5: /* Parse minute */
		ret_tm->tm_min = app;
		if(pEnd[0] == 0) {
		    flag_finished = 1;
		} else if(pEnd[0] == ':') {
		    state = 6; /* Second */
		} else {
		    strncpy(err_message, "Wrong separator after minute!", MAX_LENGTH_ERR_MESSAGE);
		    ret = -1;
		}
		break;

	    case 6: /* Parse second */
		ret_tm->tm_sec = app;
		if(pEnd[0] == 0) {
		    flag_finished = 1;
		} else {
		    strncpy(err_message, "Error parsing after second!", MAX_LENGTH_ERR_MESSAGE);
		    ret = -1;
		}
		break;

	    default : /* NOT DEFINED */
		snprintf(err_message, MAX_LENGTH_ERR_MESSAGE, "State %d not defined!", state);
		ret = -1;
		break;
	}
	if(pEnd[0] != 0  && !flag_finished  &&  ret == 0) {
	    pEnd[0] = ' '; /* overwrite separator with space */
	    app = strtol(pEnd, &pEnd, 10);
	    if(  errno == EINVAL ||  errno == ERANGE ) {
		nmxp_log(1, 0, "%s\n", strerror(errno));
		ret = -1;
	    }
	}
    }

    nmxp_log(0, 1, "FINAL: state=%d value=%d flag_finished=%d ret=%d pEnd[0]=%c [%d]  (%s)\n", state, app, flag_finished, ret, (pEnd[0]==0)? '_' : pEnd[0], pEnd[0], pEnd);

    if(!flag_finished && (ret == 0)) {
	strncpy(err_message, "Date incomplete!", MAX_LENGTH_ERR_MESSAGE);
	ret = -1;
    }

    if(ret == -1) {
	nmxp_log(1, 0, "in date '%s' %s\n", pstr_date, err_message);
    } else {
	nmxp_log(0, 1, "Date '%s' has been validate! %04d/%02d/%02d %02d:%02d:%02d\n",
		pstr_date,
		ret_tm->tm_year,
		ret_tm->tm_mon,
		ret_tm->tm_mday,
		ret_tm->tm_hour,
		ret_tm->tm_min,
		ret_tm->tm_sec
		);
    }

    return ret;
}

time_t nmxp_data_tm_to_time(struct tm *tm) {
    time_t ret_t = 0;
    
    ret_t = timegm(tm);

    return ret_t;
}
int nmxp_data_seed_init(NMXP_DATA_SEED *data_seed) {
    data_seed->srcname[0] = 0;
    data_seed->outfile_mseed = NULL;
    data_seed->filename_mseed[0] = 0;
    return 0;
}

#ifdef HAVE_LIBMSEED

/* Private function for writing mini-seed records */
    static void nmxp_data_msr_write_handler (char *record, int reclen, void *pdata_seed) {
	NMXP_DATA_SEED *data_seed = pdata_seed;
	if( data_seed->outfile_mseed ) {
	    if ( fwrite(record, reclen, 1, data_seed->outfile_mseed) != 1 ) {
		ms_log (2, "Error writing %s to output file\n", data_seed->filename_mseed);
	    }
	} else {
		ms_log (2, "Error opening file %s\n", data_seed->filename_mseed);
	}
    }


int nmxp_data_msr_pack(NMXP_DATA_PROCESS *pd, NMXP_DATA_SEED *data_seed, void *pmsr) {
    int ret =0;

    MSRecord *msr = pmsr;
    int psamples;
    int precords;
    flag verbose = 0;

    int *pDataDest = NULL;

    if(pd) {

	/* Populate MSRecord values */

	// TODO
	// msr->starttime = ms_seedtimestr2hptime ("2004,350,00:00:00.00");
	msr->starttime = MS_EPOCH2HPTIME(pd->time);
	msr->samprate = pd->sampRate;

	// msr->byteorder = 0;         /* big endian byte order */
	msr->byteorder = nmxp_data_bigendianhost ();

	msr->sequence_number = pd->seq_no % 1000000;

	msr->sampletype = 'i';      /* declare type to be 32-bit integers */

	msr->numsamples = pd->nSamp;
	msr->datasamples = malloc (sizeof(int) * (msr->numsamples)); 
	memcpy(msr->datasamples, pd->pDataPtr, sizeof(int) * pd->nSamp); /* pointer to 32-bit integer data samples */

	msr_srcname (msr, data_seed->srcname, 0);

	pDataDest = msr->datasamples;
	nmxp_log(0, 1, "x0 %d, xn %d\n", pDataDest[0], pDataDest[msr->numsamples-1]);

	/* msr_print(msr, 2); */

	/* Pack the record(s) */
	precords = msr_pack (msr, &nmxp_data_msr_write_handler, data_seed->srcname, &psamples, 1, verbose);

	if ( precords == -1 )
	    ms_log (2, "Cannot pack records\n");
	else {
	    /* ms_log (1, "Packed %d samples into %d records\n", psamples, precords); */
	}

    }

    return ret;
}

#endif



void nmxp_data_swap_2b (int16_t *in) {
    unsigned char *p = (unsigned char *)in;
    unsigned char tmp;
    tmp = *p;
    *p = *(p+1);    
    *(p+1) = tmp;
}   


void nmxp_data_swap_3b (unsigned char *in) {
    unsigned char *p = (unsigned char *)in;
    unsigned char tmp;
    tmp = *p;
    *p = *(p+2);    
    *(p+2) = tmp;
}   


void nmxp_data_swap_4b (int32_t *in) {
    unsigned char *p = (unsigned char *)in;
    unsigned char tmp;
    tmp = *p;
    *p = *(p+3);
    *(p+3) = tmp;
    tmp = *(p+1);
    *(p+1) = *(p+2);
    *(p+2) = tmp;
}


void nmxp_data_swap_8b (int64_t *in) {
    unsigned char *p = (unsigned char *)in;
    unsigned char tmp;
    tmp = *p;
    *p = *(p+7);
    *(p+7) = tmp;
    tmp = *(p+1);
    *(p+1) = *(p+6);
    *(p+6) = tmp;
    tmp = *(p+2);
    *(p+2) = *(p+5);
    *(p+5) = tmp;
    tmp = *(p+3);
    *(p+3) = *(p+4);
    *(p+4) = tmp;
}


int nmxp_data_bigendianhost () {
    int16_t host = 1;
    return !(*((int8_t *)(&host)));
} 
