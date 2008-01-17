/*! \file
 *
 * \brief Data for Nanometrics Protocol Library
 *
 * Author:
 * 	Matteo Quintiliani
 * 	Istituto Nazionale di Geofisica e Vulcanologia - Italy
 *	quintiliani@ingv.it
 *
 * $Id: nmxp_data.c,v 1.48 2008-01-17 08:13:51 mtheo Exp $
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


/*
For a portable version of timegm(), set the TZ environment variable  to
UTC, call mktime() and restore the value of TZ.  Something like
*/
#ifndef HAVE_TIMEGM

time_t my_timegm (struct tm *tm) {
    time_t ret;
#ifndef HAVE_SETENV
#warning Computation of packet latencies could be wrong if local time is not equal to UTC.
    static int first_time = 1;
    if(first_time) {
	    first_time = 0;
	    nmxp_log(NMXP_LOG_WARN, NMXP_LOG_D_ANY, "Computation of packet latencies could be wrong if local time is not equal to UTC.\n");
    }
#else
    char *tz;

    tz = getenv("TZ");
    setenv("TZ", "", 1);
    tzset();
#endif
    ret = mktime(tm);
#ifdef HAVE_SETENV
    if (tz)
	setenv("TZ", tz, 1);
    else
	unsetenv("TZ");
    tzset();
#endif
    return ret;
}

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


int nmxp_data_unpack_bundle (int32_t *outdata, unsigned char *indata, int32_t *prev)
{         
	int32_t nsamples = 0;
	int32_t d4[4];
	int16_t d2[2];
	int32_t cb[4];  
	//mtheo int i, j, k;
	int32_t i, j, k=0;
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
		nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "cb[%d]=%d\n", j, cb[j]);
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
			/* nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "val = %d, diff[%d] = %d, *prev = %d\n",
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
    time_t time_t_start_time;
    struct tm *tm_start_time;

    if(time_d > 0.0) {
	    time_t_start_time = (time_t) time_d;
    } else {
	    time_t_start_time = 0;
    }
    tm_start_time = gmtime(&time_t_start_time);
    
    // sprintf(out_str, "%04d/%02d/%02d %02d:%02d:%02d.%04d",
    sprintf(out_str, "%04d.%03d,%02d:%02d:%02d.%04d",
	    tm_start_time->tm_year + 1900,
	    /*
	    tm_start_time->tm_mon + 1,
	    tm_start_time->tm_mday,
	    */
	    tm_start_time->tm_yday + 1,
	    tm_start_time->tm_hour,
	    tm_start_time->tm_min,
	    tm_start_time->tm_sec,
	    (time_t_start_time == 0)? 0 : (int) (  ((time_d - (double) time_t_start_time)) * 10000.0 )
	   );
    
    return 0;
}


int nmxp_data_trim(NMXP_DATA_PROCESS *pd, double trim_start_time, double trim_end_time, unsigned char exclude_bitmap) {
    int ret = 0;
    double first_time, last_time;
    int first_nsamples_to_remove = 0;
    int last_nsamples_to_remove = 0;
    int32_t new_nSamp = 0;
    int32_t i;


    if(pd) {
	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "nmxp_data_trim(pd, %.4f, %.4f, %d)\n", trim_start_time, trim_end_time, exclude_bitmap);
	first_time = pd->time;
	last_time = pd->time + ((double) pd->nSamp / (double) pd->sampRate);
	if(first_time <= trim_start_time &&  trim_start_time <= last_time) {
	    first_nsamples_to_remove = (int) ( ((trim_start_time - first_time) * (double) pd->sampRate) + 0.5 );
	    if((exclude_bitmap & NMXP_DATA_TRIM_EXCLUDE_FIRST)) {
		first_nsamples_to_remove++;
		nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "Excluded the first sample!\n");
	    }
	}
	if(first_time <= trim_end_time  &&  trim_end_time <= last_time) {
	    last_nsamples_to_remove = (int) ( ((last_time - trim_end_time) * (double) pd->sampRate) + 0.5 );
	    if((exclude_bitmap & NMXP_DATA_TRIM_EXCLUDE_LAST)) {
		last_nsamples_to_remove++;
		nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "Excluded the last sample!\n");
	    }
	}

	if( (first_time < trim_start_time  &&  last_time < trim_start_time) ||
		(first_time > trim_end_time  &&  last_time > trim_end_time) ) {
	    first_nsamples_to_remove = pd->nSamp;
	    nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "Excluded all samples!\n");
	}

	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "first_time=%.2f last_time=%.2f trim_start_time=%.2f trim_end_time=%.2f\n",
		first_time, last_time, trim_start_time, trim_end_time);
	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "first_nsamples_to_remove=%d last_nsamples_to_remove=%d pd->nSamp=%d\n",
		first_nsamples_to_remove,
		last_nsamples_to_remove,
		pd->nSamp);

	if(first_nsamples_to_remove > 0 || last_nsamples_to_remove > 0) {

	    new_nSamp = pd->nSamp - (first_nsamples_to_remove + last_nsamples_to_remove);

	    if(new_nSamp > 0) {

		if(first_nsamples_to_remove > 0) {
		    pd->x0 = pd->pDataPtr[first_nsamples_to_remove];
		}

		if(last_nsamples_to_remove > 0) {
		    pd->xn = pd->pDataPtr[pd->nSamp - last_nsamples_to_remove];
		}

		for(i=0; i < pd->nSamp - first_nsamples_to_remove; i++) {
		    pd->pDataPtr[i] = pd->pDataPtr[first_nsamples_to_remove + i];
		}
		pd->nSamp = new_nSamp;
		pd->time += ((double) first_nsamples_to_remove / (double) pd->sampRate);

		ret = 1;


	    } else if(new_nSamp == 0) {
		if(pd->pDataPtr) {
		    nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "nmxp_data_trim() nSamp = %d\n", new_nSamp);
		}
		pd->nSamp = 0;
		pd->x0 = -1;
		pd->xn = -1;
		ret = 1;
	    } else {
		    nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_PACKETMAN, "Error in nmxp_data_trim() nSamp = %d\n", new_nSamp);
	    }

	} else {
	    ret = 2;
	}
    } else {
	nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_PACKETMAN, "nmxp_data_trim() is called with pd = NULL\n");
    }

    if(ret == 1) {
	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "nmxp_data_trim() trimmed data! (Output %d samples)\n", pd->nSamp);
    }

    nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "nmxp_data_trim() exit ret=%d\n", ret);

    return ret;
}

time_t nmxp_data_gmtime_now() {
    time_t time_now;
    struct tm *tm_now;
    NMXP_TM_T tmt_now;
    time(&time_now);
    tm_now = gmtime(&time_now);
    memcpy(&(tmt_now.t), tm_now, sizeof(struct tm));
    tmt_now.d = 0;
    time_now = nmxp_data_tm_to_time(&tmt_now);

    return time_now;
}

double nmxp_data_latency(NMXP_DATA_PROCESS *pd) {
    double latency = 0.0;
    time_t time_now = nmxp_data_gmtime_now();
    
    if(pd) {
	latency = ((double) time_now) - (pd->time + ((double) pd->nSamp / (double) pd->sampRate));
    }

    return latency;
}


int nmxp_data_log(NMXP_DATA_PROCESS *pd) {

    char str_start[200], str_end[200];

    str_start[0] = 0;
    str_end[0] = 0;
    
    if(pd) {
	nmxp_data_to_str(str_start, pd->time);
	nmxp_data_to_str(str_end, pd->time + ((double) pd->nSamp / (double) pd->sampRate));

	// nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "%12d %5s.%3s rate=%03d (%s - %s) [%d, %d] pts=%04d (%d, %d, %d, %d) lat=%.1f len=%d\n",
	// printf("%10d %5s.%3s 03dHz (%s - %s) lat=%.1fs [%d, %d] pts=%04d (%d, %d, %d, %d) len=%d\n",
	nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "%s.%s.%3s %3dHz (%s - %s) lat %.1fs [%d, %d] (%d) %4dpts (%d, %d, %d, %d, %d) %d\n",
		/* pd->key, */
		pd->network,
		(strlen(pd->station) == 0)? "XXXX" : pd->station,
		(strlen(pd->channel) == 0)? "XXX" : pd->channel,
		pd->sampRate,
		str_start,
		str_end,
		nmxp_data_latency(pd),
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
	/*
	if(pd->nSamp > 0) {
	    int i;
	    for(i=0; i < pd->nSamp; i++) {
		printf("%6d ", pd->pDataPtr[i]);
		if((i + 1) % 20 == 0) {
		    printf("\n");
		}
	    }
	    printf("\n");
	}
	*/
    } else {
	nmxp_log(NMXP_LOG_WARN, NMXP_LOG_D_PACKETMAN, "Pointer to NMXP_DATA_PROCESS is NULL!\n");
    }

    return 0;
}


int nmxp_data_parse_date(const char *pstr_date, NMXP_TM_T *ret_tmt) {
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

    char str_tt[20];
    int k;

#define MAX_LENGTH_STR_MESSAGE 30
    char str_date[MAX_LENGTH_STR_MESSAGE] = "NO DATE";

#define MAX_LENGTH_ERR_MESSAGE 500
    char err_message[MAX_LENGTH_ERR_MESSAGE] = "NO MESSAGE";

    char *pEnd = NULL;
    int32_t app;
    int state;
    int flag_finished = 0;

    nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_DATE, "Date to validate '%s'\n", pstr_date);
	
    strncpy(str_date, pstr_date, MAX_LENGTH_STR_MESSAGE);
    pEnd = str_date;
    app = strtol(str_date, &pEnd, 10);
    state = 0;
    if(  errno == EINVAL ||  errno == ERANGE ) {
	nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_DATE, "%s\n", strerror(errno));
	ret = -1;
    }

    if(pEnd[0] != 0  &&  ret != -1) {
	ret = 0;
    } else {
	strncpy(err_message, "Error parsing year!", MAX_LENGTH_ERR_MESSAGE);
	ret = -1;
    }

    /* initialize ret_tmt */
    time_t time_now;
    struct tm *tm_now;
    time(&time_now);
    tm_now = gmtime(&time_now);

    ret_tmt->t.tm_sec = 0 ;
    ret_tmt->t.tm_min = 0;
    ret_tmt->t.tm_hour = 0;
    ret_tmt->t.tm_mday = tm_now->tm_mday;
    ret_tmt->t.tm_mon = tm_now->tm_mon;
    ret_tmt->t.tm_year = tm_now->tm_year;
    ret_tmt->t.tm_wday = tm_now->tm_wday;
    ret_tmt->t.tm_yday = tm_now->tm_yday;
    ret_tmt->t.tm_isdst = tm_now->tm_isdst;
#ifdef HAVE_STRUCT_TM_TM_GMTOFF
    ret_tmt->t.tm_gmtoff = tm_now->tm_gmtoff;
#endif
    ret_tmt->d = 0;

    
    /* loop for parsing by a finite state machine */
    while( 
	    !flag_finished
	    && ret == 0
	    &&  errno != EINVAL
	    &&  errno != ERANGE
	    ) {

    nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_DATE, "state=%d value=%d flag_finished=%d ret=%d pEnd[0]=%c [%d]  (%s)\n", state, app, flag_finished, ret, (pEnd[0]==0)? '_' : pEnd[0], pEnd[0], pEnd);

	/* switch on state */
	switch(state) {
	    case 0: /* Parse year */
		ret_tmt->t.tm_year = app - 1900;
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
		ret_tmt->t.tm_mon = app - 1;
		if(pEnd[0] == '/')
		    state = 2; /* Day of month */
		else {
		    strncpy(err_message, "Wrong separator after month!", MAX_LENGTH_ERR_MESSAGE);
		    ret = -1;
		}
		break;

	    case 2: /* Parse day of month */
		ret_tmt->t.tm_mday = app;
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
		ret_tmt->t.tm_yday = app - 1;

		int month_days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
		int m, d, day_sum, jday=app;

		if(NMXP_DATA_IS_LEAP(ret_tmt->t.tm_year)) {
		    month_days[1]++;
		}

		m=0;
		day_sum = 0;
		while(month_days[m] < (jday - day_sum)) {
		    day_sum += month_days[m++];
		}
		d = jday-day_sum;

		ret_tmt->t.tm_mon = m;
		ret_tmt->t.tm_mday = d;

		nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_DATE, "Month %d Day %d\n", m, d);

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
		ret_tmt->t.tm_hour = app;
		if(pEnd[0] == ':') {
		    state = 5; /* Minute */
		} else {
		    strncpy(err_message, "Wrong separator after hour!", MAX_LENGTH_ERR_MESSAGE);
		    ret = -1;
		}
		break;

	    case 5: /* Parse minute */
		ret_tmt->t.tm_min = app;
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
		ret_tmt->t.tm_sec = app;
		if(pEnd[0] == 0) {
		    flag_finished = 1;
		} else if(pEnd[0] == '.') {
		    state = 7; /* ten thousandth of second */
		} else {
		    strncpy(err_message, "Error parsing after second!", MAX_LENGTH_ERR_MESSAGE);
		    ret = -1;
		}
		break;

	    case 7: /* Parse ten thousandth of second */
		ret_tmt->d = app;
		if(pEnd[0] == 0) {
		    flag_finished = 1;
		} else {
		    strncpy(err_message, "Error parsing after ten thousandth of second!", MAX_LENGTH_ERR_MESSAGE);
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
	    if(state == 7) {
		pEnd++;
		str_tt[0] = '1';
		str_tt[1] = 0;
		if(pEnd[0] == 0 || strlen(pEnd) > 4) {
		    strncpy(err_message, "Error parsing ten thousandth of second!", MAX_LENGTH_ERR_MESSAGE);
		    ret = -1;
		} else {
		    strncat(str_tt, pEnd, 20);
		    k=0;
		    while(k<5) {
			if(str_tt[k] == 0) {
			    str_tt[k] = '0';
			}
			k++;
		    }
		    str_tt[k] = 0;
		    pEnd = str_tt;
		}
	    }
	    app = strtol(pEnd, &pEnd, 10);
	    if(state == 7) {
		    app -= 10000;
	    }
	    if(  errno == EINVAL ||  errno == ERANGE ) {
		nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_DATE, "%s\n", strerror(errno));
		ret = -1;
	    }
	}
    }

    nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_DATE, "FINAL: state=%d value=%d flag_finished=%d ret=%d pEnd[0]=%c [%d]  (%s)\n", state, app, flag_finished, ret, (pEnd[0]==0)? '_' : pEnd[0], pEnd[0], pEnd);

    if(!flag_finished && (ret == 0)) {
	strncpy(err_message, "Date incomplete!", MAX_LENGTH_ERR_MESSAGE);
	ret = -1;
    }

    if(ret == -1) {
	nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_DATE, "in date '%s' %s\n", pstr_date, err_message);
    } else {
	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_DATE, "Date '%s' has been validate! %04d/%02d/%02d %02d:%02d:%02d.%04d\n",
		pstr_date,
		ret_tmt->t.tm_year,
		ret_tmt->t.tm_mon,
		ret_tmt->t.tm_mday,
		ret_tmt->t.tm_hour,
		ret_tmt->t.tm_min,
		ret_tmt->t.tm_sec,
		ret_tmt->d
		);
    }

    return ret;
}

double nmxp_data_tm_to_time(NMXP_TM_T *tmt) {
    double ret_d = 0.0;
    
#ifdef HAVE_TIMEGM
    ret_d = timegm(&(tmt->t));
#else
    ret_d = my_timegm(&(tmt->t));
#endif

    ret_d += ((double) tmt->d / 10000.0 );

    return ret_d;
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
    if(pd->nSamp > 0) {

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
	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_PACKETMAN, "x0 %d, xn %d\n", pDataDest[0], pDataDest[msr->numsamples-1]);

	/* msr_print(msr, 2); */

	/* Pack the record(s) */
	precords = msr_pack (msr, &nmxp_data_msr_write_handler, data_seed->srcname, &psamples, 1, verbose);

	if ( precords == -1 )
	    ms_log (2, "Cannot pack records\n");
	else {
	    /* ms_log (1, "Packed %d samples into %d records\n", psamples, precords); */
	}

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
