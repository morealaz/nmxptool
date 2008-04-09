/*! \file
 *
 * \brief Nanometrics Protocol Tool
 *
 * Author:
 * 	Matteo Quintiliani
 * 	Istituto Nazionale di Geofisica e Vulcanologia - Italy
 *	quintiliani@ingv.it
 *
 * $Id: nmxptool_sigcondition.h,v 1.1 2008-04-09 07:58:01 mtheo Exp $
 *
 */

#ifndef NMXPTOOL_SIGCONDITION_H
#define NMXPTOOL_SIGCONDITION_H 1

void nmxptool_sigcondition_init();
void nmxptool_sigocondition_destroy();
int  nmxptool_sigcondition_read();
void nmxptool_sigcondition_write(int new_sig);

#endif
