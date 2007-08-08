#ifndef NMXP_DAP_GETLONG_H
#define NMXP_DAP_GETLONG_H 1

#include <getopt.h>
#include <stdint.h>

#define DEFAULT_NETWORK "XX"

/*! \brief Struct that stores information about parameter of the program */
typedef struct {
    char *hostname;
    int portnumberdap;
    int portnumberpds;
    char *channels;
    char *network;
    char *location;
    uint32_t start_time;
    uint32_t end_time;
    char *datas_username;
    char *datas_password;
    int flag_writeseed;
    int flag_verbose;
    int flag_listchannels;
    int flag_writefile;
    int flag_writeseedlink;
} NMXP_DAP_PARAMS;

/*! \brief Print the usage of paramters */
void nmxptool_usage(struct option long_options[]);


/*! \brief Calls getopt_long and set struct NMXP_DAP_PARAMS
 *
 * \param argc
 * \param argv
 * \param params
 *
 * \retval 0 on success.
 * \retval >0 Number of errors.
 *
 */
int nmxptool_getopt_long(int argc, char **argv, NMXP_DAP_PARAMS *params);


/*! \brief Check semantyc of values in struct NMXP_DAP_PARAMS
 *
 * \param params Struct to validate.
 *
 * \retval 0 on success.
 * \retval -1 on error.
 */
int nmxptool_check_params(NMXP_DAP_PARAMS *params);

#endif

