/*! \file
 *
 * \brief Log for Nanometrics Protocol Library
 *
 * Author:
 * 	Matteo Quintiliani
 * 	Istituto Nazionale di Geofisica e Vulcanologia - Italy
 *	quintiliani@ingv.it
 *
 * $Id: nmxp_log.c,v 1.9 2007-09-30 20:35:08 mtheo Exp $
 *
 */

#include "nmxp_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <time.h>

#include "config.h"

#define MAX_LOG_MESSAGE_LENGTH 8000


const char *nmxp_log_version() {
    static char ret_str[MAX_LOG_MESSAGE_LENGTH] = "";
    sprintf(ret_str, "%s-%s", PACKAGE_NAME, PACKAGE_VERSION);
    return ret_str;
}


int nmxp_log_stdout(char *msg) {
    int ret = fprintf(stdout, msg);
    fflush(stdout);
    return ret;
}

int nmxp_log_stderr(char *msg) {
    int ret = fprintf(stderr, msg);
    fflush(stderr);
    return ret;
}

#define NMXP_MAX_FUNC_LOG 10

int n_func_log = 0;
int n_func_log_err = 0;
int (*p_func_log[NMXP_MAX_FUNC_LOG]) (char *);
int (*p_func_log_err[NMXP_MAX_FUNC_LOG]) (char *);


void nmxp_log_init(int (*func_log)(char *), int (*func_log_err)(char *)) {
    n_func_log = 0;
    n_func_log_err = 0;
    p_func_log[n_func_log++]         = (func_log == NULL)?     func_log     : nmxp_log_stdout;
    p_func_log_err[n_func_log_err++] = (func_log_err == NULL)? func_log_err : nmxp_log_stdout;

}


int nmxp_log(int level, int verb, ... )
{
  static int staticverb = 0;
  int retvalue = 0;

  if ( level == -1 ) {
    staticverb = verb;
    retvalue = staticverb;
  }
  else if (verb <= staticverb) {
    char message[MAX_LOG_MESSAGE_LENGTH];
    char message_final[MAX_LOG_MESSAGE_LENGTH];
    char timestr[100];
    char *format;
    va_list listptr;
    time_t loc_time;

    va_start(listptr, verb);
    format = va_arg(listptr, char *);

    /* Build local time string and cut off the newline */
    time(&loc_time);
    // TODO
    strcpy(timestr, asctime(localtime(&loc_time)));
    timestr[strlen(timestr) - 1] = '\0';

    retvalue = vsnprintf(message, MAX_LOG_MESSAGE_LENGTH, format, listptr);

    switch(level) {
	case NMXP_LOG_ERR:
	    //TOREMOVE printf("%s - %s: error: %s", timestr, PACKAGE_NAME, message);
	    sprintf(message_final, "%s - %s: error: %s", timestr, PACKAGE_NAME, message);
	    if(n_func_log_err > 0) {
		p_func_log_err[0](message_final);
	    } else {
		nmxp_log_stdout(message_final);
	    }
	    break;
	case NMXP_LOG_WARN:
	    //TOREMOVE printf("%s - %s: warning: %s", timestr, PACKAGE_NAME, message);
	    sprintf(message_final, "%s - %s: warning: %s", timestr, PACKAGE_NAME, message);
	    if(n_func_log > 0) {
		p_func_log[0](message_final);
	    } else {
		nmxp_log_stdout(message_final);
	    }
	    break;
	case NMXP_LOG_NORM_NO:
	    //TOREMOVE printf("%s", message);
	    sprintf(message_final, "%s", message);
	    if(n_func_log > 0) {
		p_func_log[0](message_final);
	    } else {
		nmxp_log_stdout(message_final);
	    }
	    break;
	case NMXP_LOG_NORM_PKG:
	    //TOREMOVE printf("%s: %s", PACKAGE_NAME, message);
	    sprintf(message_final, "%s: %s", PACKAGE_NAME, message);
	    if(n_func_log > 0) {
		p_func_log[0](message_final);
	    } else {
		nmxp_log_stdout(message_final);
	    }
	    break;
	default:
	    //TOREMOVE printf("%s - %s: %s", timestr, PACKAGE_NAME, message);
	    sprintf(message_final, "%s - %s: %s", timestr, PACKAGE_NAME, message);
	    if(n_func_log > 0) {
		p_func_log[0](message_final);
	    } else {
		nmxp_log_stdout(message_final);
	    }
	    break;
    }

    //TOREMOVE fflush(stdout);
    va_end(listptr);
  }

  return retvalue;
} /* End of nmxp_log() */

