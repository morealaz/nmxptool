/*! \file
 *
 * \brief Log for Nanometrics Protocol Libray
 *
 * Author:
 * 	Matteo Quintiliani
 * 	Istituto Nazionale di Geofisica e Vulcanologia - Italy
 *	quintiliani@ingv.it
 *
 */

#include "nmxp_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <time.h>

#include "config.h"

#define MAX_LOG_MESSAGE_LENGTH 200


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

    if ( level == 1 ) {
      printf("%s - %s: error: %s", timestr, PACKAGE_NAME, message);
    }
    else {
      printf("%s - %s: %s", timestr, PACKAGE_NAME, message);
    }

    fflush(stdout);
    va_end(listptr);
  }

  return retvalue;
} /* End of nmxp_log() */

