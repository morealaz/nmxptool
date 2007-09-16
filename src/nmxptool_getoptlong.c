/*! \file
 *
 * \brief Nanometrics Protocol Tool
 *
 * Author:
 * 	Matteo Quintiliani
 * 	Istituto Nazionale di Geofisica e Vulcanologia - Italy
 *	quintiliani@ingv.it
 *
 * $Id: nmxptool_getoptlong.c,v 1.16 2007-09-16 21:38:51 mtheo Exp $
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
    0,
    0,
    NULL,
    NULL,
    DEFAULT_STC,
    DEFAULT_RATE,
    NULL,
    DEFAULT_DELAY,
    DEFAULT_MAX_PDLIST_ITEMS,
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
    nmxp_log(NMXP_LOG_NORM_NO, 0, "\
Matteo Quintiliani - Istituto Nazionale di Geofisica e Vulcanologia - Italy\n\
Mail bug reports and suggestions to <%s>.\n",
	    PACKAGE_BUGREPORT
	    );
}


void nmxptool_version() {
    nmxp_log(NMXP_LOG_NORM_NO, 0, "\
%s %s, Nanometrics tool based on %s\n\
        (Data Access Protocol 1.0, Private Data Stream 1.4)\n",
	PACKAGE_NAME, PACKAGE_VERSION,
	nmxp_log_version()
	);
    nmxptool_author_support();

}


void nmxptool_usage(struct option long_options[])
{
    nmxptool_version();
    nmxp_log(NMXP_LOG_NORM_NO, 0, "\
\n\
Usage: %s -H hostname --listchannels [...]\n\
             Receive list of available channels on the host\n\
\n\
       %s -H hostname -C channellist [...]\n\
             Receive data from hostname by PDS\n\
\n\
       %s -H hostname -C channellist -s DATE -e DATE [...]\n\
             Receive data from hostname by DAP\n\
\n\
Arguments:\n\
  -H, --hostname=HOST     Nanometrics hostname.\n\
  -C, --channels=LIST     Channel list STA1.HH?,STA2.??Z,...\n\
\n\
",
	    PACKAGE_NAME,
	    PACKAGE_NAME,
	    PACKAGE_NAME
	  );

    nmxp_log(NMXP_LOG_NORM_NO, 0, "\
Other arguments:\n\
  -P, --portpds=PORT      NaqsServer port number (default %d).\n\
  -D, --portdap=PORT      DataServer port number (default %d).\n\
  -N, --network=NET       Declare Network code for all stations (default %s).\n\
  -L, --location=LOC      Location code for writing file.\n\
  -v, --verbose           Be verbose.\n\
  -g, --logdata           Print info about data.\n\
  -l, --listchannels      Output list of channel available on NaqsServer.\n\
  -i, --channelinfo       Output list of channel available on NaqsServer and channelinfo.\n\
",
	    DEFAULT_PORT_PDS,
	    DEFAULT_PORT_DAP,
	    DEFAULT_NETWORK
		);

#ifdef HAVE_LIBMSEED
    nmxp_log(NMXP_LOG_NORM_NO, 0, "\
  -m, --writeseed         Pack received data in Mini-SEED records and write to a file.\n");
#endif

    nmxp_log(NMXP_LOG_NORM_NO, 0, "\
  -w, --writefile         Write received data to a file.\n");

#ifdef HAVE___SRC_SEEDLINK_PLUGIN_C
    nmxp_log(NMXP_LOG_NORM_NO, 0, "\
  -k, --slink=plug_name   Send received data to SeedLink like as plug-in.\n\
                          plug_name is set by SeisComP daemon.\n\
                          THIS OPTION MUST BE THE LAST WITHOUT plug_name IN seedlink.ini!\n");
#endif
    nmxp_log(NMXP_LOG_NORM_NO, 0, "\
  -h, --help              Print this help.\n\
\n");

    nmxp_log(NMXP_LOG_NORM_NO, 0, "\
DAP Arguments:\n\
  -s, --start_time=DATE   Start time in date format.\n\
  -e, --end_time=DATE     End time in date format.\n\
                          DATE can be in formats:\n\
                              <date>,<time> | <date>\n\
                          where:\n\
                              <date> = yyyy/mm/dd | yyy.jjj\n\
                              <time> = hh:mm:ss | hh:mm\n\
  -d, --delay=secs        Receive continuosly data with delay.\n\
  -u, --username=USER     DataServer username.\n\
  -p, --password=PASS     DataServer password.\n\
\n\
");

    nmxp_log(NMXP_LOG_NORM_NO, 0, "\
PDS arguments:\n\
  -S, --stc=SECs          Short-term-completion (default %d) -1 is for Raw Stream.\n\
  -R, --rate=HZ           Receive decompressed data with specified sample rate.\n\
                          -1 is for original sample rate and compressed data.\n\
                           0 is for original sample rate and decompressed data.\n\
                          >0 is for specified sample rate and decompressed data.\n\
  -b, --buffered          Request also recent packets into the past.\n\
  -n, --npackets=NUM      Number of packets contained into the buffer (default %d).\n\
                          Usable only with --stc=-1\n\
\n\
",
	    DEFAULT_STC,
	    DEFAULT_MAX_PDLIST_ITEMS
	  );

    nmxptool_author_support();

    /*
    if(long_options) {
	int i=0;
	while(long_options[i].name) {
	    nmxp_log(NMXP_LOG_NORM_NO, 0, "%s %d %d %d %c\n", long_options[i].name, long_options[i].has_arg, (long_options[i].flag)? *(long_options[i].flag) : 0, long_options[i].val, long_options[i].val);
	    i++;
	}
    }
    */
}


int nmxptool_getopt_long(int argc, char **argv, NMXPTOOL_PARAMS *params)
{
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
	{"location",     required_argument, 0, 'L'},
	{"stc",          required_argument, 0, 'S'},
	{"rate",         required_argument, 0, 'R'},
	{"start_time",   required_argument, 0, 's'},
	{"end_time",     required_argument, 0, 'e'},
	{"delay",        required_argument, 0, 'd'},
	{"username",     required_argument, 0, 'u'},
	{"password",     required_argument, 0, 'p'},
	{"npackets",     required_argument, 0, 'n'},
	/* Following are flags */
	{"verbose",      no_argument,       0, 'v'},
	{"logdata",      no_argument,       0, 'g'},
	{"buffered",     no_argument,       0, 'b'},
	{"listchannels", no_argument,       0, 'l'},
	{"channelinfo",  no_argument,       0, 'i'},
#ifdef HAVE_LIBMSEED
	{"writeseed",    no_argument,       0, 'm'},
#endif
	{"writefile",    no_argument,       0, 'w'},
#ifdef HAVE___SRC_SEEDLINK_PLUGIN_C
	{"slink",        required_argument, 0, 'k'},
#endif
	{"help",         no_argument,       0, 'h'},
	{"version",      no_argument,       0, 'V'},
	{0, 0, 0, 0}
    };

    int ret_errors = 0;

    struct tm tmp_tm;
    int i;
    char one_time_option[255];

    int c;


    /* getopt_long stores the option index here. */
    int option_index = 0;

    /* init array for checking one time option */
    for(i=0; i<255; i++) {
	one_time_option[i] = 0;
    }


    /* init params */
    memcpy(params, &NMXPTOOL_PARAMS_DEFAULT, sizeof(NMXPTOOL_PARAMS_DEFAULT));

    char optstr[100] = "H:P:D:C:N:L:S:R:s:e:d:u:p:n:vgbliwhV";

#ifdef HAVE_LIBMSEED
    strcat(optstr, "m");
#endif

#ifdef HAVE___SRC_SEEDLINK_PLUGIN_C
    strcat(optstr, "k:");
#endif

    while ( (c = getopt_long (argc, argv, optstr, long_options, &option_index)) != -1) {

	/* BE CAREFUL if use synonym options !!! */
	one_time_option[c]++;

	if(one_time_option[c] > 1) {
	    ret_errors++;
	    nmxp_log(NMXP_LOG_NORM_NO, 0, "Replicated option -%c (value %s)\n", c, optarg);
	} else {
	    switch (c)
	    {
		case 0:
		    /* If this option set a flag, do nothing else now. */
		    if (long_options[option_index].flag != 0)
			break;
		    nmxp_log(NMXP_LOG_NORM_NO, 0, "option %s", long_options[option_index].name);
		    if (optarg)
			nmxp_log(NMXP_LOG_NORM_NO, 0, " with arg %s", optarg);
		    nmxp_log(NMXP_LOG_NORM_NO, 0, "\n");
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
		    params->channels = optarg;
		    break;

		case 'N':
		    params->network = optarg;
		    break;

		case 'L':
		    params->location = optarg;
		    break;

		case 'S':
		    params->stc = atoi(optarg);
		    nmxp_log(0, 0, "Short-Term-Completion %d.\n", params->stc);
		    break;

		case 'R':
		    params->rate = atoi(optarg);
		    break;

		case 's':
		    if(nmxp_data_parse_date(optarg, &tmp_tm) == -1) {
			// MESSAGE ERROR
		    } else {
			params->start_time = nmxp_data_tm_to_time(&tmp_tm);
		    }
		    break;

		case 'e':
		    if(nmxp_data_parse_date(optarg, &tmp_tm) == -1) {
			// MESSAGE ERROR
		    } else {
			params->end_time = nmxp_data_tm_to_time(&tmp_tm);
		    }
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

		case 'n':
		    params->max_pdlist_items = atoi(optarg);
		    nmxp_log(0, 0, "Max_number_of_packets %d.\n", params->max_pdlist_items);
		    break;

#ifdef HAVE___SRC_SEEDLINK_PLUGIN_C
		case 'k':
		    params->flag_slink = 1;
		    params->plugin_slink = optarg;
		    break;
#endif

		case 'v':
		    params->flag_verbose = 1;
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

	nmxp_log(NMXP_LOG_NORM_NO, 0, "non-option ARGV-elements: ");
	while (optind < argc)
	    nmxp_log(NMXP_LOG_NORM_NO, 0, "%s ", argv[optind++]);
	putchar ('\n');
    }

    return ret_errors;
}


int nmxptool_check_params(NMXPTOOL_PARAMS *params) {
    int ret = 0;

    if(params->hostname == NULL) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, 0, "<hostname> is required!\n");
    } else if(params->flag_listchannels) {
	/* Do nothing */
    } else if(params->hostname == NULL) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, 0, "<hostname> is required!\n");
    } else if(params->channels == NULL) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, 0, "<STA.CHAN> is required!\n");
    } else if(params->start_time != 0   &&   params->end_time != 0
	    && params->start_time >= params->end_time) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, 0, "<start_time> is less than <end_time>!\n");
    } else if(params->stc < -1   ||   params->stc > 300) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, 0, "<stc> has to be in the interval -1..300 secs.\n");
    } else if(params->stc == -1   &&   params->rate != DEFAULT_RATE) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, 0, "<rate> can not be declared when <stc> is equal to -1 (Raw Stream).\n");
    } else if(params->delay > 0 && params->start_time != 0   &&   params->end_time != 0) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, 0, "<delay> can not be used with options <start_time> and <end_time>.\n");
    } else if(params->rate != -1 && params->start_time != 0   &&   params->end_time != 0) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, 0, "<rate> can not be used with options <start_time> and <end_time>.\n");
    } else if(params->flag_buffered != -1 && params->start_time != 0   &&   params->end_time != 0) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, 0, "<buffered> can not be used with options <start_time> and <end_time>.\n");
    } else if(params->delay < 0) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, 0, "<delay> has to be greater than zero.\n");
    } else if( params->stc == -1 && (params->max_pdlist_items < 10  ||  params->max_pdlist_items > 100)) {
	ret = -1;
	nmxp_log(NMXP_LOG_NORM_NO, 0, "<npackets> has to be within [10..100].\n");
    } else if( params->stc != -1 && params->max_pdlist_items > 0) {
	nmxp_log(NMXP_LOG_WARN, 0, "<npackets> ignored since not defined --stc=-1.\n");
    }

    return ret;
}
