/*! \file
 *
 * \brief Memory management for Nanometrics Protocol Library
 *
 * Author:
 * 	Matteo Quintiliani
 * 	Istituto Nazionale di Geofisica e Vulcanologia - Italy
 *	quintiliani@ingv.it
 *
 * $Id $
 *
 */

#ifndef NMXP_MEMORY_H
#define NMXP_MEMORY_H 1

#define NMXP_MEM_MALLOC(size) nmxp_mem_malloc(size, __FILE__, __LINE__)
#define NMXP_MEM_STRDUP(str) nmxp_mem_strdup(str, __FILE__, __LINE__)
#define NMXP_MEM_FREE(ptr) nmxp_mem_free(ptr, __FILE__, __LINE__)

#include <stdlib.h>

/*! \brief 
 *
 * \param 
 * \param 
 *
 */
void *nmxp_mem_malloc(size_t size, char *source_file, int line);


/*! \brief 
 *
 * \param 
 * \param 
 *
 */
char *nmxp_mem_strdup(const char *str, char *source_file, int line);

/*! \brief 
 *
 * \param 
 * \param 
 *
 */
void nmxp_mem_free(void *ptr, char *source_file, int line);

/*! \brief 
 *
 * \param 
 * \param 
 *
 */
void nmxp_mem_print_ptr();

#endif

