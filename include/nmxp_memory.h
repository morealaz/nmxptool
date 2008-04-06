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

#ifndef NMXP_MEM_DEBUG

#define NMXP_MEM_MALLOC(size) malloc(size)
#define NMXP_MEM_STRDUP(str) strdup(str)
#define NMXP_MEM_FREE(ptr) free(ptr)
#define NMXP_MEM_PRINT_PTR(print_items) { }

#else

/* Uncomment following line for logging malloc(), strdup() and free() calls */
/* #define NMXP_MEM_DEBUG_LOG_SINGLE 1 */

#define NMXP_MEM_MALLOC(size) nmxp_mem_malloc(size, __FILE__, __LINE__)
#define NMXP_MEM_STRDUP(str) nmxp_mem_strdup(str, __FILE__, __LINE__)
#define NMXP_MEM_FREE(ptr) nmxp_mem_free(ptr, __FILE__, __LINE__)
#define NMXP_MEM_PRINT_PTR(print_items) nmxp_mem_print_ptr(print_items, __FILE__, __LINE__)


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
void nmxp_mem_print_ptr(int print_items, char *source_file, int line);

#endif

#endif

