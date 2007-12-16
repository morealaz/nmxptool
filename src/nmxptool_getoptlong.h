/*! \file
 *
 * \brief Nanometrics Protocol Tool
 *
 * Author:
 * 	Matteo Quintiliani
 * 	Istituto Nazionale di Geofisica e Vulcanologia - Italy
 *	quintiliani@ingv.it
 *
 * $Id: nmxptool_getoptlong.h,v 1.35 2007-12-16 17:11:39 mtheo Exp $
 *
 */

#ifndef NMXPTOOL_GETLONG_H
#define NMXPTOOL_GETLONG_H 1

#include <getopt.h>
#include <stdint.h>

/* For stable release set to "" */
#define PACKAGE_BUILD "-beta2"

#define DEFAULT_NETWORK  	"XX"
#define DEFAULT_PORT_DAP 	28002
#define DEFAULT_PORT_PDS 	28000

#define DEFAULT_STC_MINIMUM	-1
#define DEFAULT_STC_MAXIMUM	300
#define DEFAULT_STC      	-1

#define DEFAULT_RATE_MINIMUM   	-1
#define DEFAULT_RATE_MAXIMUM   	1000
#define DEFAULT_RATE     	-1

#define DEFAULT_DELAY_MINIMUM  	60
#define DEFAULT_DELAY_MAXIMUM  	86400
#define DEFAULT_DELAY    	0

#define DEFAULT_MAX_TOLERABLE_LATENCY_MINIMUM	60
#define DEFAULT_MAX_TOLERABLE_LATENCY_MAXIMUM	600
#define DEFAULT_MAX_TOLERABLE_LATENCY 		600

#define DEFAULT_TIMEOUTRECV 			0
#define DEFAULT_TIMEOUTRECV_MINIMUM 		10
#define DEFAULT_TIMEOUTRECV_MAXIMUM 		300

#define DEFAULT_VERBOSE_LEVEL 			NMXP_LOG_D_NULL
#define DEFAULT_VERBOSE_LEVEL_MINIMUM		NMXP_LOG_D_NULL
#define DEFAULT_VERBOSE_LEVEL_MAXIMUM		NMXP_LOG_D_ANY


/*! \brief Struct that stores information about parameter of the program */
typedef struct {
    char *hostname;
    int portnumberdap;
    int portnumberpds;
    char *channels;
    char *network;
    char *location;
    int32_t start_time;
    int32_t end_time;
    int32_t interval;
    char *datas_username;
    char *datas_password;
    int32_t stc;
    int32_t rate;
    char *plugin_slink;
    int32_t delay;
    int32_t max_tolerable_latency;
    int32_t timeoutrecv;
    int32_t verbose_level;
    char *ew_configuration_file;
    char *statefile;
    int flag_writeseed;
    int flag_listchannels;
    int flag_listchannelsnaqs;
    int flag_request_channelinfo;
    int flag_writefile;
    int flag_slink;
    int flag_buffered;
    int flag_logdata;
} NMXPTOOL_PARAMS;

/*! \brief Print author and e-mail for support and bugs */
void nmxptool_author_support();

/*! \brief Print version of tool */
void nmxptool_version();

/*! \brief Print supports of tool i.e. libmseed, earthworm, ...*/
void nmxptool_supports();

/*! \brief Print the usage of paramters */
void nmxptool_usage(struct option long_options[]);


/*! \brief Calls getopt_long and set struct NMXPTOOL_PARAMS
 *
 * \param argc
 * \param argv
 * \param params
 *
 * \retval 0 on success.
 * \retval >0 Number of errors.
 *
 */
int nmxptool_getopt_long(int argc, char **argv, NMXPTOOL_PARAMS *params);


/*! \brief Check semantyc of values in struct NMXPTOOL_PARAMS
 *
 * \param params Struct to validate.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int nmxptool_check_params(NMXPTOOL_PARAMS *params);

#endif

