/*! \file
 *
 * \brief Nanometrics Protocol Tool
 *
 * Author:
 * 	Matteo Quintiliani
 * 	Istituto Nazionale di Geofisica e Vulcanologia - Italy
 *	quintiliani@ingv.it
 *
 * $Id: nmxptool_getoptlong.c,v 1.50 2007-12-28 10:39:02 mtheo Exp $
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "config.h"
#include "nmxp.h"

#include "nmxptool_getoptlong.h"


const NMXPTOOL_PARAMS NMXPTOOL_PARAMS_DEFAULT =
{
    NULL,
    DEFAULT_PORT_DAP,
    DEFAULT_PORT_PDS,
    NULL,
    NULL,
    NULL,
    0.0,
    0.0,
    DEFAULT_INTERVAL,
    NULL,
    NULL,
    DEFAULT_STC,
    DEFAULT_RATE,
    NULL,
    DEFAULT_DELAY,
    DEFAULT_MAX_TOLERABLE_LATENCY,
    DEFAULT_TIMEOUTRECV,
    DEFAULT_VERBOSE_LEVEL,
    NULL,
    NULL,
    DEFAULT_BUFFERED_TIME,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};


void nmxptool_author_support() {
    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "\
Matteo Quintiliani - Istituto Nazionale di Geofisica e Vulcanologia - Italy\n\
Mail bug reports and suggestions to <%s>.\n",
	    PACKAGE_BUGREPORT
	    );
}


void nmxptool_version() {
    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "\
%s %s%s, Nanometrics tool based on %s\n\
        (Data Access Protocol 1.0, Private Data Stream 1.4)\n",
	PACKAGE_NAME, PACKAGE_VERSION, PACKAGE_BUILD,
	nmxp_log_version()	
	);

    nmxptool_supports();
}

void nmxptool_supports() {
    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "\
         Support for: libmseed ");
#ifdef HAVE_LIBMSEED
    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "YES");
#else
    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "NO");
#endif

    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, ", SeedLink ");
#ifdef HAVE___SRC_SEEDLINK_PLUGIN_C
    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "YES");
#else
    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "NO");
#endif

    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, ", Earthworm ");
#ifdef HAVE_EARTHWORMOBJS
    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "YES");
#else
    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "NO");
#endif
    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, ".\n");
}


void nmxptool_usage(struct option long_options[])
{
    nmxptool_version();

    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "\
\n\
Usage: %s -H hostname   --listchannels | --listchannelsnaqs\n\
             Print list of available channels on DataServer or NaqsServer.\n\
\n\
       %s -H hostname -C channellist [...]\n\
       %s -H hostname -F statefile [...]\n\
             Receive data from NaqsServer by PDS.\n\
\n\
       %s -H hostname -C channellist -s DATE -e DATE [...]\n\
       %s -H hostname -C channellist -s DATE -t SECs [...]\n\
             Receive data from DataServer by DAP.\n\
\n", PACKAGE_NAME, PACKAGE_NAME, PACKAGE_NAME, PACKAGE_NAME, PACKAGE_NAME);

#ifdef HAVE_EARTHWORMOBJS
    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "\
       %s nmxptool.d\n\
             Run as earthworm module receiving data from NaqsServer by PDS.\n\
\n", PACKAGE_NAME);
#endif

    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "\
Arguments:\n\
  -H, --hostname=HOST     Nanometrics hostname.\n\
  -C, --channels=LIST     Sequence of NET.STA.CHAN separated by comma.\n\
                          NET  is optional and used only for output.\n\
                          STA  can be '*', stands for all stations.\n\
                          CHAN can contain '?', stands for any character.\n\
                          Example:  *.HH?,N1.STA2.??Z,STA3.?H?\n");

    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "\
PDS arguments:\n\
  -F, --statefile=FILE    Load/Save time of last sample of each channel.\n\
                          Allow data continuity between program restarts\n\
                          and within data buffered by the NaqsServer.\n\
                          Enable option -b. Do not use with -C.\n\
  -S, --stc=SECs          Short-term-completion (default %d).\n\
                          -1 is for Raw Stream, no short-term completion.\n\
                           0 chronological order without waiting\n\
                             for missing data.\n\
                          [1..300] chronological order waiting a period\n\
                             for missing data.\n\
                          Raw Stream is usable only with --rate=-1.\n\
  -R, --rate=Hz           Receive data with specified sample rate (default %d).\n\
                          -1 for original sample rate and compressed data.\n\
                           0 for original sample rate and decompressed data.\n\
                          >0 for specified sample rate and decompressed data.\n\
  -b, --buffered          Request also recent packets into the past.\n\
  -B, --buff_date=DATE    Request also recent packets into the past\n\
                          but consider only samples after DATE.\n\
  -L, --listchannelsnaqs  Print list of available channels on NaqsServer.\n\
  -M, --maxlatency=SECs   Max tolerable latency (default %d) [%d..%d].\n\
  -T, --timeoutrecv=SECs  Time-out for flushing buffered packets.\n\
                          (default %d. No time-out.) [%d..%d].\n\
                          -T is useful for retrieving Data On Demand.\n\
                          -M, -T are usable only with Raw Stream --stc=-1.\n\
                          In general, -M and -T are not used together.\n\
\n\
",
	    DEFAULT_STC,
	    DEFAULT_RATE,
	    DEFAULT_MAX_TOLERABLE_LATENCY,
	    DEFAULT_MAX_TOLERABLE_LATENCY_MINIMUM,
	    DEFAULT_MAX_TOLERABLE_LATENCY_MAXIMUM,
	    DEFAULT_TIMEOUTRECV,
	    DEFAULT_TIMEOUTRECV_MINIMUM,
	    DEFAULT_TIMEOUTRECV_MAXIMUM
	  );

    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "\
DAP Arguments:\n\
  -s, --start_time=DATE   Start time in date format.\n\
  -e, --end_time=DATE     End time in date format.\n\
                          DATE can be in formats:\n\
                              <date>,<time> | <date>\n\
                          where:\n\
                              <date> = yyyy/mm/dd | yyy.jjj\n\
                              <time> = hh:mm:ss | hh:mm:ss.dddd | hh:mm\n\
  -t, --interval=SECs     Time interval from start_time.\n\
  -d, --delay=SECs        Receive continuosly data with delay [%d..%d].\n\
  -u, --username=USER     DataServer username.\n\
  -p, --password=PASS     DataServer password.\n\
  -l, --listchannels      Print list of available channels on DataServer.\n\
  -i, --channelinfo       Print list of available channels and channelinfo.\n\
\n\
",
DEFAULT_DELAY_MINIMUM,
DEFAULT_DELAY_MAXIMUM);

    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "\
Other arguments:\n\
  -P, --portpds=PORT      NaqsServer port number (default %d).\n\
  -D, --portdap=PORT      DataServer port number (default %d).\n\
  -N, --network=NET       Default output Network code. (default '%s').\n\
  -L, --location=LOC      Default output Location code. DISABLED!\n\
  -v, --verbose=level     Be verbose. level is a bitmap:\n\
                          %d Packet, %d Channel, %d Raw Stream,\n\
                          %d CRC32, %d Connection flow,\n\
                          %d Packet Management, %d Extra, %d Date,\n\
                          %d Gap, %d DOD, %d All messages.\n\
  -g, --logdata           Print info about data.\n\
",
	    DEFAULT_PORT_PDS,
	    DEFAULT_PORT_DAP,
	    DEFAULT_NETWORK,
	    NMXP_LOG_D_PACKET,
	    NMXP_LOG_D_CHANNEL,
	    NMXP_LOG_D_RAWSTREAM,
	    NMXP_LOG_D_CRC,
	    NMXP_LOG_D_CONNFLOW,
	    NMXP_LOG_D_PACKETMAN,
	    NMXP_LOG_D_EXTRA,
	    NMXP_LOG_D_DATE,
	    NMXP_LOG_D_GAP,
	    NMXP_LOG_D_DOD,
	    NMXP_LOG_D_ANY
		);

#ifdef HAVE_LIBMSEED
    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "\
  -m, --writeseed         Pack received data in Mini-SEED records\n\
                          and write to a file.\n");
#endif

    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "\
  -w, --writefile         Dump received data to a file.\n");

#ifdef HAVE___SRC_SEEDLINK_PLUGIN_C
    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "\
  -k, --slink=plug_name   Send received data to SeedLink as a plug-in.\n\
                          plug_name is set by SeisComP daemon.\n\
                          INTO THE FILE seedlink.in, THIS OPTION MUST BE\n\
                          THE LAST WITHOUT ADDING VALUE FOR plug_name!\n");
#endif

    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "\
  -V, --version           Print tool version.\n\
  -h, --help              Print this help.\n\
\n");

    nmxptool_author_support();

    /*
    if(long_options) {
	int i=0;
	while(long_options[i].name) {
	    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_EXTRA, "%s %d %d %d %c\n", long_options[i].name, long_options[i].has_arg, (long_options[i].flag)? *(long_options[i].flag) : 0, long_options[i].val, long_options[i].val);
	    i++;
	}
    }
    */
}


#define MAXSIZE_LINE_CHAN_STATE 2048
#define MAXSIZECHANNELSTRINGARGUMENT 8000
#define MAXSIZE_CHANNEL_STRING 64

char *get_channel_list_argument_from_state_file(const char *filename) {
    char *ret_channel_string = NULL;
    char line[MAXSIZE_LINE_CHAN_STATE];
    char str_chan[MAXSIZE_CHANNEL_STRING];
    int k;
    FILE *fstatefile = NULL;

    fstatefile = fopen(filename, "r");

    /* Read only channel names from state file */
    if(fstatefile) {
	ret_channel_string = (char *) malloc (MAXSIZECHANNELSTRINGARGUMENT);
	ret_channel_string[0] = 0;
	while(fgets(line, MAXSIZE_LINE_CHAN_STATE, fstatefile) != NULL) {
	    k = 0;
	    while(line[k] != 0
		    &&  line[k] != ' '
		    &&  line[k] != 10
		    &&  line[k] != 13
		    &&  k < MAXSIZE_CHANNEL_STRING) {
		str_chan[k] = line[k];
		k++;
	    }
	    str_chan[k] = 0;
	    if(ret_channel_string[0] == 0) {
		strncpy(ret_channel_string, str_chan, MAXSIZECHANNELSTRINGARGUMENT);
	    } else {
		strncat(ret_channel_string, ",", MAXSIZECHANNELSTRINGARGUMENT);
		strncat(ret_channel_string, str_chan, MAXSIZECHANNELSTRINGARGUMENT);
	    }
	}
	fclose(fstatefile);
    }
    return ret_channel_string;
}

int nmxptool_getopt_long(int argc, char **argv, NMXPTOOL_PARAMS *params)
{
    int ret_errors = 0;

    NMXP_TM_T tmp_tmt;
    int i;
    char one_time_option[255];
    int c;

    struct option long_options[] =
    {
	/* These options set a flag. */
	/* It is not safe use reference to params in this way */
	/* {"verbose",        no_argument,       &(params->flag_verbose), 1}, */
	/* {"quiet",          no_argument,       &(params->flag_verbose), 0}, */
	/* These options don't set a flag.
	 *                   We distinguish them by their indices. */
	{"hostname",     required_argument, 0, 'H'},
	{"portpds",      required_argument, 0, 'P'},
	{"portdap",      required_argument, 0, 'D'},
	{"channels",     required_argument, 0, 'C'},
	{"network",      required_argument, 0, 'N'},
	{"location",     required_argument, 0, 'n'},
	{"stc",          required_argument, 0, 'S'},
	{"rate",         required_argument, 0, 'R'},
	{"start_time",   required_argument, 0, 's'},
	{"end_time",     required_argument, 0, 'e'},
	{"interval",     required_argument, 0, 't'},
	{"delay",        required_argument, 0, 'd'},
	{"username",     required_argument, 0, 'u'},
	{"password",     required_argument, 0, 'p'},
	{"maxlatency",   required_argument, 0, 'M'},
	{"timeoutrecv",  required_argument, 0, 'T'},
	{"verbose",      required_argument, 0, 'v'},
	{"bufferedt",    required_argument, 0, 'B'},
	/* Following are flags */
	{"logdata",      no_argument,       0, 'g'},
	{"buffered",     no_argument,       0, 'b'},
	{"listchannels", no_argument,       0, 'l'},
	{"listchannelsnaqs", no_argument,   0, 'L'},
	{"channelinfo",  no_argument,       0, 'i'},
#ifdef HAVE_LIBMSEED
	{"writeseed",    no_argument,       0, 'm'},
#endif
	{"writefile",    no_argument,       0, 'w'},
#ifdef HAVE___SRC_SEEDLINK_PLUGIN_C
	{"slink",        required_argument, 0, 'k'},
#endif
	{"statefile",    required_argument, 0, 'F'},
	{"help",         no_argument,       0, 'h'},
	{"version",      no_argument,       0, 'V'},
	{0, 0, 0, 0}
    };

    char optstr[100] = "H:P:D:C:N:n:S:R:s:e:t:d:u:p:M:T:v:B:F:gblLiwhV";

#ifdef HAVE_LIBMSEED
    strcat(optstr, "m");
#endif

#ifdef HAVE___SRC_SEEDLINK_PLUGIN_C
    strcat(optstr, "k:");
#endif


    /* getopt_long stores the option index here. */
    int option_index = 0;

    /* init array for checking one time option */
    for(i=0; i<255; i++) {
	one_time_option[i] = 0;
    }


    /* init params */
    memcpy(params, &NMXPTOOL_PARAMS_DEFAULT, sizeof(NMXPTOOL_PARAMS_DEFAULT));

    /* Check number of command line arguments for earthworm */
    if (argc == 2)
    {
	int l = strlen(argv[1]);
	if(l >= 3) {
	    if(argv[1][0] != '-') {
		if(argv[1][l-2] == '.'  &&  argv[1][l-1] == 'd') {
		    params->ew_configuration_file = argv[1];
		    return 0;
		}
	    }
	}
    }

    while ( (c = getopt_long (argc, argv, optstr, long_options, &option_index)) != -1) {

	/* BE CAREFUL if use synonym options !!! */
	one_time_option[c]++;

	if(one_time_option[c] > 1) {
	    ret_errors++;
	    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "Replicated option -%c (value %s)\n", c, optarg);
	} else {
	    switch (c)
	    {
		case 0:
		    /* If this option set a flag, do nothing else now. */
		    if (long_options[option_index].flag != 0)
			break;
		    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "option %s", long_options[option_index].name);
		    if (optarg)
			nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, " with arg %s", optarg);
		    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "\n");
		    break;

		case 'H':
		    params->hostname = optarg;
		    break;

		case 'P':
		    params->portnumberpds = atoi(optarg);
		    break;

		case 'D':
		    params->portnumberdap = atoi(optarg);
		    break;

		case 'C':
		    if(params->channels) {
			nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_ANY, "Channels have been already defined by State File (option -F)!\n");
			ret_errors++;
		    } else {
			params->channels = optarg;
		    }
		    break;

		case 'N':
		    params->network = optarg;
		    break;

		case 'n':
		    if(1) {
			nmxp_log(NMXP_LOG_WARN, NMXP_LOG_D_ANY, "Location is currently disabled!\n");
		    } else {
			params->location = optarg;
		    }
		    break;

		case 'S':
		    params->stc = atoi(optarg);
		    nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_ANY, "Short-Term-Completion %d.\n", params->stc);
		    break;

		case 'R':
		    params->rate = atoi(optarg);
		    break;

		case 's':
		    if(nmxp_data_parse_date(optarg, &tmp_tmt) == -1) {
			// MESSAGE ERROR
			ret_errors++;
		    } else {
			params->start_time = nmxp_data_tm_to_time(&tmp_tmt);
		    }
		    break;

		case 'e':
		    if(nmxp_data_parse_date(optarg, &tmp_tmt) == -1) {
			// MESSAGE ERROR
			ret_errors++;
		    } else {
			params->end_time = nmxp_data_tm_to_time(&tmp_tmt);
		    }
		    break;

		case 't':
		    params->interval = atoi(optarg);
		    break;

		case 'd':
		    params->delay = atoi(optarg);
		    break;

		case 'u':
		    params->datas_username = optarg;
		    break;

		case 'p':
		    params->datas_password = optarg;
		    break;

		case 'M':
		    params->max_tolerable_latency = atoi(optarg);
		    nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_ANY, "Max_tolerable_latency %d\n", params->max_tolerable_latency);
		    break;

		case 'T':
		    params->timeoutrecv = atoi(optarg);
		    nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_ANY, "Time-out receiving %d\n", params->timeoutrecv);
		    break;

		case 'v':
		    params->verbose_level = atoi(optarg);
		    break;

		case 'B':
		    params->flag_buffered = 1;
		    if(nmxp_data_parse_date(optarg, &tmp_tmt) == -1) {
			// MESSAGE ERROR
			ret_errors++;
		    } else {
			params->buffered_time = nmxp_data_tm_to_time(&tmp_tmt);
		    }
		    break;


#ifdef HAVE___SRC_SEEDLINK_PLUGIN_C
		case 'k':
		    params->flag_slink = 1;
		    params->plugin_slink = optarg;
		    break;
#endif

		case 'F':
		    params->flag_buffered = 1;
		    params->statefile = optarg;
		    if(params->channels == NULL) {
			params->channels = get_channel_list_argument_from_state_file(params->statefile);
			if(params->channels) {
			    /* Do nothing */
			} else {
			    ret_errors++;
			    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "State file %s not found or unable to read!\n", params->statefile);
			}
		    } else {
			ret_errors++;
			nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_ANY, "Channels have been already defined by option -C!\n");
		    }
		    break;

		case 'g':
		    params->flag_logdata = 1;
		    break;

		case 'b':
		    params->flag_buffered = 1;
		    break;

		case 'l':
		    params->flag_listchannels = 1;
		    break;

		case 'L':
		    params->flag_listchannelsnaqs = 1;
		    break;

		case 'i':
		    params->flag_request_channelinfo = 1;
		    break;

#ifdef HAVE_LIBMSEED
		case 'm':
		    params->flag_writeseed = 1;
		    break;
#endif

		case 'w':
		    params->flag_writefile = 1;
		    break;

		case 'h':
		    nmxptool_usage(long_options);
		    exit (1);
		    break;

		case 'V':
		    nmxptool_version();
		    nmxptool_author_support();
		    exit (1);
		    break;

		case '?':
		    /* getopt_long already printed an error message. */
		    ret_errors++;
		    break;

		default:
		    nmxptool_usage(long_options);
		    exit (1);
	    }
	}
    }

    /* Print any remaining command line arguments (not options). */
    if (optind < argc)
    {
	ret_errors += optind;

	nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "non-option ARGV-elements: ");
	while (optind < argc)
	    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "%s ", argv[optind++]);
	putchar ('\n');
    }

    return ret_errors;
}


int nmxptool_check_params(NMXPTOOL_PARAMS *params) {
    int ret = 0;

    if(params->start_time != 0.0 && params->interval != 0   &&   params->end_time == 0.0) {
	params->end_time = params->start_time + params->interval;
	params->interval = 0;
    }

    if(params->ew_configuration_file != NULL) {
	/* Do nothing */
    } else if(params->hostname == NULL) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "<hostname> is required!\n");
    } else if(params->flag_listchannels) {
	if(params->flag_listchannelsnaqs) {
	    ret = -1;
	    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "-l and -L can not be used together!\n");
	}
    } else if(params->flag_listchannelsnaqs) {
	/* Do nothing */
    } else if(params->hostname == NULL) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "<hostname> is required!\n");
    } else if(params->channels == NULL) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "Channel list is required!\n");
    } else if(params->start_time == 0.0 &&  params->end_time != 0.0) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "<end_time> is required when declaring <start_time>!\n");
    } else if(params->start_time != 0.0 &&  params->end_time == 0.0) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "<start_time> has to be used with <end_time> or <interval>!\n");
    } else if(params->start_time != 0.0 && params->interval != 0   &&   params->end_time != 0.0) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "<end_time> and <interval> can not be used together!\n");
    } else if(params->start_time != 0.0   &&   params->end_time != 0.0
	    && params->start_time >= params->end_time) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "<start_time> is less than <end_time>!\n");
    } else if(params->stc < DEFAULT_STC_MINIMUM   ||   params->stc > DEFAULT_STC_MAXIMUM) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "<stc> has to be in the interval [%d..%d] secs.\n",
		DEFAULT_STC_MINIMUM, DEFAULT_STC_MAXIMUM);
    } else if(params->stc == -1   &&   params->rate != DEFAULT_RATE) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "<rate> has to be equal to -1 when <stc> is equal to -1 (Raw Stream).\n");
    } else if(params->delay > 0 && params->start_time != 0.0   &&   params->end_time != 0.0) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "<delay> can not be used with options <start_time> and <end_time>.\n");
    } else if( params->delay != DEFAULT_DELAY &&
	    (params->delay < DEFAULT_DELAY_MINIMUM  || params->delay > DEFAULT_DELAY_MAXIMUM) ) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "<delay> has to be in the interval [%d..%d] secs.\n",
		DEFAULT_DELAY_MINIMUM, DEFAULT_DELAY_MAXIMUM);
    } else if(params->verbose_level < DEFAULT_VERBOSE_LEVEL_MINIMUM  ||  params->verbose_level > DEFAULT_VERBOSE_LEVEL_MAXIMUM) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "<verbose_level> has to be in the interval [%d..%d].\n",
		DEFAULT_VERBOSE_LEVEL_MINIMUM, DEFAULT_VERBOSE_LEVEL_MAXIMUM);
    } else if(params->rate < DEFAULT_RATE_MINIMUM  ||  params->rate > DEFAULT_RATE_MAXIMUM) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "<rate> has to be in the interval [%d..%d].\n",
		DEFAULT_RATE_MINIMUM, DEFAULT_RATE_MAXIMUM);
    } else if(params->rate != -1 && params->start_time != 0.0   &&   params->end_time != 0.0) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "<rate> can not be used with options <start_time> and <end_time>.\n");
    } else if(params->flag_buffered != 0 && params->start_time != 0.0   &&   params->end_time != 0.0) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "<buffered> can not be used with options <start_time> and <end_time>.\n");
    } else if( params->stc == -1
	    && (params->max_tolerable_latency < DEFAULT_MAX_TOLERABLE_LATENCY_MINIMUM  ||
		params->max_tolerable_latency > DEFAULT_MAX_TOLERABLE_LATENCY_MAXIMUM)) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "<maxlatency> has to be within [%d..%d].\n",
		DEFAULT_MAX_TOLERABLE_LATENCY_MINIMUM,
		DEFAULT_MAX_TOLERABLE_LATENCY_MAXIMUM);
    } else if( params->stc == -1
	    && (params->timeoutrecv < DEFAULT_TIMEOUTRECV_MINIMUM  ||
		params->timeoutrecv > DEFAULT_TIMEOUTRECV_MAXIMUM)) {
	if(params->timeoutrecv != 0) {
	    ret = -1;
	    nmxp_log(NMXP_LOG_NORM_NO, NMXP_LOG_D_ANY, "<timeoutrecv> has to be within [%d..%d] or equal to zero for not time-out.\n",
		    DEFAULT_TIMEOUTRECV_MINIMUM,
		    DEFAULT_TIMEOUTRECV_MAXIMUM);
	}
    } else if( params->stc != -1 && params->max_tolerable_latency > 0 ){
	nmxp_log(NMXP_LOG_WARN, NMXP_LOG_D_ANY, "<maxlatency> ignored since not defined --stc=-1.\n");
    } else if(params->stc != -1 && params->timeoutrecv > 0) {
	params->timeoutrecv = 0;
	nmxp_log(NMXP_LOG_WARN, NMXP_LOG_D_ANY, "<timeoutrecv> ignored since not defined --stc=-1.\n");
    }

    /*
    if( params->stc == -1 ) {
	nmxp_log(NMXP_LOG_WARN, NMXP_LOG_D_ANY, "<maxlatency> is equal to %d sec.\n", params->max_tolerable_latency);
    }
    */

    return ret;
}
