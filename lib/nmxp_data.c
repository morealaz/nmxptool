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
#include <time.h>

#include <libmseed.h>


int nmxp_data_init(NMXP_DATA_PROCESS *pd) {
    pd->key = -1;
    pd->network[0] = 0;
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
	/* TOREMOVE int my_order = get_my_wordorder(); */
	int my_host_is_bigendian = ms_bigendianhost();

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

    str_start[0] = 0;
    str_end[0] = 0;
    
    nmxp_data_to_str(str_start, pd->time);
    nmxp_data_to_str(str_end, pd->time + ((double) pd->nSamp / (double) pd->sampRate));

    nmxp_log(0, 0, "%12d %5s.%3s rate=%03d (%s - %s) [%d, %d] pts=%04d (%d, %d, %d, %d) len=%d\n",
	    pd->key,
	    (strlen(pd->station) == 0)? "XXXX" : pd->station,
	    (strlen(pd->channel) == 0)? "XXX" : pd->channel,
	    pd->sampRate,
	    str_start,
	    str_end,
	    pd->packet_type,
	    pd->seq_no,
	    pd->nSamp,
	    pd->x0,
	    (pd->pDataPtr == NULL)? 0 : pd->pDataPtr[0],
	    (pd->pDataPtr == NULL || pd->nSamp < 1)? 0 : pd->pDataPtr[pd->nSamp-1],
	    (pd->pDataPtr == NULL || pd->nSamp < 1)? 0 : pd->pDataPtr[pd->nSamp],
	    pd->length
	    );
	return 0;
}

int nmxp_data_seed_init(NMXP_DATA_SEED *data_seed) {
    data_seed->srcname[0] = 0;
    data_seed->outfile_mseed = NULL;
    data_seed->filename_mseed[0] = 0;
    return 0;
}

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

int nmxp_data_msr_pack(NMXP_DATA_PROCESS *pd, NMXP_DATA_SEED *data_seed) {
    int ret =0;

    int psamples;
    int precords;
    MSRecord *msr;
    flag verbose = 0;

    msr = msr_init (NULL);

    /* Populate MSRecord values */
    strcpy (msr->network, pd->network);
    strcpy (msr->station, pd->station);
    strcpy (msr->channel, pd->channel);
    // TODO
    // msr->starttime = ms_seedtimestr2hptime ("2004,350,00:00:00.00");
    msr->starttime = MS_EPOCH2HPTIME(pd->time);
    msr->samprate = pd->sampRate;
    // TODO
    // msr->reclen = 4096;         /* 4096 byte record length */
    msr->reclen = 512;         /* byte record length */
    // TODO
    // msr->encoding = DE_STEIM2;  /* Steim 2 compression */
    msr->encoding = DE_STEIM1;  /* Steim 1 compression */
    // TODO
    // msr->byteorder = 0;         /* big endian byte order */
    msr->byteorder = ms_bigendianhost ();

    int sizetoallocate = sizeof(int) * (pd->nSamp + 1);
    msr->datasamples = malloc (sizetoallocate); 
    memcpy(msr->datasamples, pd->pDataPtr, sizetoallocate); /* pointer to 32-bit integer data samples */
    msr->numsamples = pd->nSamp + 1;
    msr->sampletype = 'i';      /* declare type to be 32-bit integers */

    msr_srcname (msr, data_seed->srcname, 0);

    /* Pack the record(s) */
    precords = msr_pack (msr, &nmxp_data_msr_write_handler, data_seed->srcname, &psamples, 1, verbose);

    // ms_log (0, "Packed %d samples into %d records\n", psamples, precords);
    nmxp_log (0, 1, "Packed %d samples into %d records\n", psamples, precords);

    msr_free (&msr);

    return ret;
}



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

