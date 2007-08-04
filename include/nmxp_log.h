/*! \file
 *
 * \brief LOG for Nanometrics Protocol Libray
 *
 * Author:
 * 	Matteo Quintiliani
 * 	Istituto Nazionale di Geofisica e Vulcanologia - Italy
 *	quintiliani@ingv.it
 *
 */

#ifndef NMXP_LOG_H
#define NMXP_LOG_H 1


/*! \brief A generic logging/printing routine
 * 
 *   This function works in two modes:
 *
 *   -# Initialization, expecting 2 arguments with the first (level)
 *       being -1 and the second being verbosity.  This will set the
 *       verbosity for all future calls, the default is 0.  Can be used
 *       to change the verbosity at any time. I.e. 'sl_log(-1,2);'
 *   -# Expecting 3+ arguments, log level, verbosity level, printf
 *       format, and printf arguments.  If the verbosity level is less
 *       than or equal to the set verbosity (see mode 1), the printf
 *       format and arguments will be printed at the appropriate log
 *       level. I.e. 'sl_log(0, 0, "error: %s", result);'
 *
 *   \retval new_verbosity if using mode 1.
 *   \retval n the number of characters formatted on success, and a
 *     a negative value on error if using mode 2.
 *
 *   \param level
 *   \param verb
 *   \param ...
 */
int nmxp_log(int level, int verb, ... );

#endif
