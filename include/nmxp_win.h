/*! \file
 *
 * \brief Function for Windows OS
 *
 * Author:
 * 	Matteo Quintiliani
 * 	Istituto Nazionale di Geofisica e Vulcanologia - Italy
 *	quintiliani@ingv.it
 *
 * $Id: nmxp_win.h,v 1.3 2008-03-11 07:28:55 mtheo Exp $
 *
 */

/* Code is based on guide and example from:
 * http://tangentsoft.net/wskfaq/articles/bsd-compatibility.html
 * http://tangentsoft.net/wskfaq/examples/basics/index.html
 * http://tangentsoft.net/wskfaq/examples/basics/ws-util.cpp
 */

#ifndef NMXP_WIN_H
#define NMXP_WIN_H 1


/*! \brief Winsock initialization
 */
void nmxp_initWinsock();

/*! \brief A function similar in spirit to Unix's perror().
 *
 * This function returns a pointer to an internal static buffer, so you must
 * copy the data from this function before you call it again.  It follows that
 * this function is also not thread-safe.
 *
 * \param nErrorID
 *
 * \return Return a pointer to a static string which contains error description.
 *
 */
char* WSAGetLastErrorMessage(int nErrorID);


#endif

