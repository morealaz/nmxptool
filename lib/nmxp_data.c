/*! \file
 *
 * \brief Data for Nanometrics Protocol Libray
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


int nmxp_init_process_data(NMXP_PROCESS_DATA *pd) {
    pd->key = -1;
    pd->sta = NULL;
    pd->chan = NULL;
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


int nmxp_unpack_bundle (int *outdata, unsigned char *indata, int *prev)
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
		nmxp_log (0,0, "cb[%d]=%d\n", j, cb[j]);
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
				nmxp_log(0,0, "val = %d, diff[%d] = %d, *prev = %d\n",
						*outdata, i, d4[i], *prev);
						*/
			*prev = *outdata;

			outdata++;
			++nsamples;
		}
	}
	return (nsamples);
}


int nmxp_log_process_data(NMXP_PROCESS_DATA *pd) {
    nmxp_log(0, 0, "%12d %5s.%3s (%10.4f - %10.4f) nsamp: %04d, srate: %03d, len: %04d [%d, %d] (%d, %d, %d)\n",
	    pd->key,
	    pd->sta,
	    pd->chan,
	    pd->time,
	    pd->time + ((double) pd->nSamp / (double) pd->sampRate),
	    pd->nSamp,
	    pd->sampRate,
	    pd->length,
	    pd->packet_type,
	    pd->seq_no,
	    pd->x0,
	    pd->pDataPtr[0],
	    pd->pDataPtr[pd->nSamp-1]
	    );
	return 0;
}


void nmxp_processDecompressedDataFunc(char* buffer_data, int length_data, NMXP_CHAN_LIST *channelList,
	int (*func_processData)(NMXP_PROCESS_DATA *pd)
	)
{
  int32_t   netInt    = 0;
  int32_t   pKey      = 0;
  double    pTime     = 0.0;
  int32_t   pNSamp    = 0;
  int32_t   pSampRate = 0;
  int32_t  *pDataPtr  = 0;
  int       swap      = 0;
  int       idx;

  char *sta = 0;      /* The station code */
  char *chan = 0;     /* The channel code */

  /* copy the header contents into local fields and swap */
  memcpy(&netInt, &buffer_data[0], 4);
  pKey = ntohl(netInt);
  if ( pKey != netInt ) { swap = 1; }

  memcpy(&pTime, &buffer_data[4], 8);
  if ( swap ) { swab8(&pTime); }

  memcpy(&netInt, &buffer_data[12], 4);
  pNSamp = ntohl(netInt);
  memcpy(&netInt, &buffer_data[16], 4);
  pSampRate = ntohl(netInt);

  /* There should be (length_data - 20) bytes of data as 32-bit ints here */
  pDataPtr = (int32_t *) &buffer_data[20];

  /* Swap the data samples to host order */
  for ( idx=0; idx < pNSamp; idx++ ) {
      netInt = ntohl(pDataPtr[idx]);
      pDataPtr[idx] = netInt;
  }

  /* Lookup the station and channel code */
  sta = strdup(nmxp_chan_lookupName(pKey, channelList));
  if ( (chan = strchr(sta, '.')) == NULL ) {
    nmxp_log(1,0, "Channel name not in STN.CHAN format: %s\n", sta);
    free(sta);
    return;
  }
  *chan++ = '\0';
  
  /* Send it off to the controlling SeedLink server */
  /*
  if ( send_raw_depoch(sta, chan, pTime, 0, 100, pDataPtr, pNSamp) < 0 ) {
    nmxp_log(1,0, "cannot send data to seedlink: %s", strerror(errno));
    exit(1);
  }
  */
  NMXP_PROCESS_DATA pd;

  nmxp_init_process_data(&pd);

  pd.key = pKey;
  pd.sta = sta;
  pd.chan = chan;
  pd.packet_type = NMXP_MSG_DECOMPRESSED;
  // pd.x0 = ;
  // pd.seq_no = ;
  pd.time = pTime;
  pd.buffer = buffer_data;
  pd.length = length_data;
  pd.nSamp = pNSamp;
  pd.pDataPtr = pDataPtr;
  pd.sampRate = pSampRate;

  if(func_processData) {
      func_processData(&pd);
  }

  free(sta);
}


void nmxp_processCompressedDataFunc(char* buffer_data, int length_data, NMXP_CHAN_LIST *channelList,
	int (*func_processData)(NMXP_PROCESS_DATA *pd)
	)
{
    int32_t   pKey      = 0;
    double    pTime     = 0.0;
    int32_t   pNSamp    = 0;
    int32_t   pSampRate = 0;
    int32_t  *pDataPtr  = 0;

    char *sta = 0;      /* The station code */
    char *chan = 0;     /* The channel code */

    const int nmx_rate_code_to_sample_rate[32] = {
	0,1,2,5,10,20,40,50,
	80,100,125,200,250,500,1000,25,
	120,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0};

	int nmx_oldest_sequence_number;
	char nmx_hdr[25];
	unsigned char nmx_ptype;
	unsigned int nmx_seconds;
	double nmx_seconds_double;
	short int nmx_ticks, nmx_instr_id;
	int nmx_seqno;
	unsigned char nmx_sample_rate;
	int nmx_x0;
	int rate_code, chan_code, this_sample_rate;

	int comp_bytecount;
	unsigned char *indata;
#define MAX_OUTDATA 4096
	int32_t outdata[MAX_OUTDATA];
	int nout, i, k;
	int prev_xn;

	int my_order = get_my_wordorder();
	nmxp_log(0, 1, "my_order is %d\n", my_order);

	memcpy(&nmx_oldest_sequence_number, buffer_data, 4);
	nmxp_log(0, 1, "Oldest sequence number = %d\n", nmx_oldest_sequence_number);

	memcpy(nmx_hdr, buffer_data+4, 17);
	/* Decode the Nanometrics packet header bundle. */
	memcpy (&nmx_ptype, nmx_hdr+0, 1);
	if ( (nmx_ptype & 0xf) == 9) {
	    /* Filler packet.  Discard entire packet.   */
	    nmxp_log (1,0, "Filler packet - discarding\n");
	    //m continue;
	    exit(0);
	}

	nmx_x0 = 0;
	memcpy (&nmx_seconds, nmx_hdr+1, 4);
	memcpy (&nmx_ticks, nmx_hdr+5, 2);
	memcpy (&nmx_instr_id, nmx_hdr+7, 2);
	memcpy (&nmx_seqno, nmx_hdr+9, 4);
	memcpy (&nmx_sample_rate, nmx_hdr+13, 1);
	memcpy (&nmx_x0, nmx_hdr+14, 3);

	const unsigned int high_scale = 4096 * 2048;
	const unsigned int high_scale_p = 4096 * 4096;
	/* check if nmx_x0 is negative like as signed 3-byte int */
	if( (nmx_x0 & high_scale) ==  high_scale) {
	    // nmxp_log(0, 0, "WARNING: changed nmx_x0, old value = %d\n",  nmx_x0);
	    nmx_x0 -= high_scale_p;
	}
	if (my_order != SEED_LITTLE_ENDIAN) {
	    swab4 ((int *)&nmx_seconds);
	    swab2 (&nmx_ticks);
	    swab2 (&nmx_instr_id);
	    swab4 (&nmx_seqno);
	    nmx_x0 = nmx_x0 >> 8;
	    swab4 (&nmx_x0);
	    nmx_x0 = nmx_x0 >> 8;
	}
	nmx_seconds_double = (double) nmx_seconds + ( (double) nmx_ticks / 10000.0 );
	rate_code = nmx_sample_rate>>3;
	chan_code = nmx_sample_rate&7;
	this_sample_rate = nmx_rate_code_to_sample_rate[rate_code];

	nmxp_log(0, 1, "nmx_ptype          = %d\n", nmx_ptype);
	nmxp_log(0, 1, "nmx_seconds        = %d\n", nmx_seconds);
	nmxp_log(0, 1, "nmx_ticks          = %d\n", nmx_ticks);

	nmxp_log(0, 1, "nmx_seconds_double = %f\n", nmx_seconds_double);
	nmxp_log(0, 1, "nmx_x0             = %d\n", nmx_x0);

	nmxp_log(0, 1, "nmx_instr_id       = %d\n", nmx_instr_id);
	nmxp_log(0, 1, "nmx_seqno          = %d\n", nmx_seqno);
	nmxp_log(0, 1, "nmx_sample_rate    = %d\n", nmx_sample_rate);
	nmxp_log(0, 1, "this_sample_rate    = %d\n", this_sample_rate);

	comp_bytecount = length_data-21;
	indata = (unsigned char *) buffer_data + 21;

	/* Unpack the data bundles, each 17 bytes long. */
	prev_xn = nmx_x0;
	outdata[0] = nmx_x0;
	nout = 1;
	for (i=0; i<comp_bytecount; i+=17) {
	    if (i+17>comp_bytecount) {
		nmxp_log (1,0, "comp_bytecount = %d, i+17 = %d\n",
			comp_bytecount, i+17);
		exit(1);
	    }
	    if (nout+16 > MAX_OUTDATA)  {
		nmxp_log (1,0, "Output buffer size too small\n");
		exit(1);
	    }
	    k = nmxp_unpack_bundle (outdata+nout,indata+i,&prev_xn);
	    if (k < 0) nmxp_log (1,0, "Break: (k=%d) %s %d\n", k, __FILE__,  __LINE__);
	    if (k < 0) break;
	    nout += k;
	    /* prev_xn = outdata[nout-1]; */
	}
	nout--;

	nmxp_log(0, 1, "Unpacked %d samples.\n", nout);

	pKey = (nmx_instr_id << 16) | ( 1 << 8) | ( chan_code);

	pTime = nmx_seconds_double;

	pDataPtr = outdata;

	pNSamp = nout;

	pSampRate = this_sample_rate;

	/* Lookup the station and channel code */
	sta = strdup(nmxp_chan_lookupName(pKey, channelList));
	if ( (chan = strchr(sta, '.')) == NULL ) {
	    nmxp_log(1,0, "Channel name not in STN.CHAN format: %s\n", sta);
	    free(sta);
	    return;
	}
	*chan++ = '\0';

	nmxp_log(0, 1, "Channel key %d for %s.%s\n", pKey, sta, chan);

	/* Send it off to the controlling SeedLink server */
	/*
	   f ( send_raw_depoch(sta, chan, pTime, 0, 100, pDataPtr, pNSamp) < 0 ) {
	   nmxp_log(1,0, "cannot send data to seedlink: %s", strerror(errno));
	   exit(1);
	   }
	   */

	NMXP_PROCESS_DATA pd;

	nmxp_init_process_data(&pd);

	pd.key = pKey;
	pd.sta = sta;
	pd.chan = chan;
	pd.packet_type = nmx_ptype;
	pd.x0 = nmx_x0;
	pd.seq_no = nmx_seqno;
	pd.time = pTime;
	pd.buffer = buffer_data;
	pd.length = length_data;
	pd.nSamp = pNSamp;
	pd.pDataPtr = pDataPtr;
	pd.sampRate = pSampRate;

	if(func_processData) {
	    func_processData(&pd);
	}

	free(sta);
}

