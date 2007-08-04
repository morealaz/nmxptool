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

#include <qlib2.h>


int nmxp_data_init(NMXP_DATA_PROCESS *pd) {
    pd->key = -1;
    pd->station[0] = 0;
    pd->channel[0] = 0;
    pd->packet_type = -1;
    pd->x0 = -1;
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
	int my_order = get_my_wordorder();

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
				if (my_order != SEED_LITTLE_ENDIAN) {
					swab2 (&d2[0]);
					swab2 (&d2[1]);
				}
				d4[0] = d2[0];
				d4[1] = d2[1];
				k=2;
				break;
			case 3:       /* 1 32-bit diff */
				memcpy (&d4[0],indata,4);
				if (my_order != SEED_LITTLE_ENDIAN) {
					swab4 (&d4[0]);
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


void nmxp_data_ext_time_to_str(char *s, EXT_TIME et) {
    // typedef struct _ext_time {
	// int         year;           /* Year.                        */
	// int         doy;            /* Day of year (1-366)          */
	// int         month;          /* Month (1-12)                 */
	// int         day;            /* Day of month (1-31)          */
	// int         hour;           /* Hour (0-23)                  */
	// int         minute;         /* Minute (0-59)                */
	// int         second;         /* Second (0-60 (leap))         */
	// int         usec;           /* Microseconds (0-999999)      */
    // } EXT_TIME;
    //
    //
    sprintf(s, "%04d/%02d/%02d %02d:%02d:%02d.%04d",
	    et.year,
	    et.month,
	    et.day,
	    et.hour,
	    et.minute,
	    et.second,
	    et.usec / 100
	   );

}

int nmxp_data_log(NMXP_DATA_PROCESS *pd) {

    INT_TIME int_time_start, int_time_end;
    EXT_TIME ext_time_start, ext_time_end;
    char str_start[200], str_end[200];
    
    int_time_start = nepoch_to_int(pd->time);
    int_time_end = nepoch_to_int(pd->time + ((double) pd->nSamp / (double) pd->sampRate));

    ext_time_start = int_to_ext(int_time_start);
    ext_time_end = int_to_ext(int_time_end);

    nmxp_data_ext_time_to_str(str_start, ext_time_start);
    nmxp_data_ext_time_to_str(str_end, ext_time_end);

    // nmxp_log(0, 0, "%12d %5s.%3s (%10.4f - %10.4f) nsamp: %04d, srate: %03d, len: %04d [%d, %d] (%d, %d, %d, %d)\n",
    // nmxp_log(0, 0, "%12d %5s.%3s (%10.4f - %10.4f) (%s - %s) nsamp: %04d, srate: %03d, len: %04d [%d, %d] (%d, %d, %d, %d)\n",
    nmxp_log(0, 0, "%12d %5s.%3s (%s - %s) nsamp: %04d, srate: %03d, len: %04d [%d, %d] (%d, %d, %d, %d)\n",
	    pd->key,
	    (strlen(pd->station) == 0)? "XXXX" : pd->station,
	    (strlen(pd->channel) == 0)? "XXX" : pd->channel,
	    /*
	    pd->time,
	    pd->time + ((double) pd->nSamp / (double) pd->sampRate),
	    */
	    str_start,
	    str_end,
	    pd->nSamp,
	    pd->sampRate,
	    pd->length,
	    pd->packet_type,
	    pd->seq_no,
	    pd->x0,
	    (pd->pDataPtr == NULL)? 0 : pd->pDataPtr[0],
	    (pd->pDataPtr == NULL || pd->nSamp < 1)? 0 : pd->pDataPtr[pd->nSamp-1],
	    (pd->pDataPtr == NULL || pd->nSamp < 1)? 0 : pd->pDataPtr[pd->nSamp]
	    );
	return 0;
}

