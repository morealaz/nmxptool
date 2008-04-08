/*! \file
 *
 * \brief Memory management for Nanometrics Protocol Library
 *
 * Author:
 * 	Matteo Quintiliani
 * 	Istituto Nazionale di Geofisica e Vulcanologia - Italy
 *	quintiliani@ingv.it
 *
 * $Id: nmxp_memory.c,v 1.6 2008-04-08 13:27:10 mtheo Exp $
 *
 */

#ifdef NMXP_MEM_DEBUG

#include "nmxp_memory.h"
#include "nmxp_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "config.h"

#define MAX_LEN_SOURCE_FILE_LINE 100

typedef struct {
    void *p;
    int size;
    char source_file_line[MAX_LEN_SOURCE_FILE_LINE];
    struct timeval tv;
} NMXP_MEM_STRUCT;

#define MAX_MEM_STRUCTS (4096 * 16)

static NMXP_MEM_STRUCT nms[MAX_MEM_STRUCTS];
static int i_nms = 0;

static int nmxp_mem_add_ptr(void *ptr, size_t size, char *source_file_line, struct timeval *tv) {
    int ret = -1;
    if(i_nms < MAX_MEM_STRUCTS) {
	nms[i_nms].p = ptr;
	nms[i_nms].size = size;
	strncpy(nms[i_nms].source_file_line, source_file_line, MAX_LEN_SOURCE_FILE_LINE);
	nms[i_nms].tv.tv_sec = tv->tv_sec;
	nms[i_nms].tv.tv_usec = tv->tv_usec;
	ret = i_nms;
	i_nms++;
    } else {
	nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_ANY, "nmxp_mem_add_ptr i_nms > MAX_MEM_STRUCTS %d > %d\n", i_nms, MAX_MEM_STRUCTS);
    }
    return ret;
}

void nmxp_mem_print_ptr(int print_items, char *source_file, int line) {
    int i;
    static int old_tot_size = 0;
    int tot_size;

    tot_size = 0;
    for(i=0; i < i_nms  &&  i_nms < MAX_MEM_STRUCTS; i++) {
	tot_size += nms[i].size;
    }

    if(tot_size != old_tot_size) {
	if(print_items) {
	    i=0;
	    while(i<i_nms  && i_nms < MAX_MEM_STRUCTS) {
		nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_ANY, "%d %d.%d %010p %d %s\n",
			i,
			nms[i].tv.tv_sec,
			nms[i].tv.tv_usec,
			nms[i].p,
			nms[i].size,
			nms[i].source_file_line
			);
		i++;
	    }
	}
	old_tot_size = tot_size;
    }

    nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_ANY, "nmxp_mem_print_ptr() tot %d  %s:%d\n", tot_size, source_file, line);
}

static int nmxp_mem_rem_ptr(void *ptr, struct timeval *tv, int *size) {
    int i, j;

    tv->tv_sec = 0;
    tv->tv_usec = 0;
    *size = 0;

    i = 0;
    while(i < i_nms && nms[i].p != ptr) {
	i++;
    }

    if(i >= i_nms  ||  i > MAX_MEM_STRUCTS) {
	nmxp_log(NMXP_LOG_ERR, NMXP_LOG_D_ANY, "nmxp_mem_rem_ptr %010p not found i=%d\n", ptr, i);
    } else {
	/* shift */
	tv->tv_sec = nms[i].tv.tv_sec;
	tv->tv_usec = nms[i].tv.tv_usec;
	*size = nms[i].size;
	j = i;
	while(j < i_nms-1) {
	    nms[j].p = nms[j+1].p;
	    nms[j].size = nms[j+1].size;
	    strncpy(nms[j].source_file_line, nms[j+1].source_file_line, MAX_LEN_SOURCE_FILE_LINE);;
	    nms[j].tv.tv_sec = nms[j+1].tv.tv_sec;
	    nms[j].tv.tv_usec = nms[j+1].tv.tv_usec;
	    j++;
	}
	i_nms--;
    }
    return i;
}

static char *nmxp_mem_source_file_line(char *source_file, int line) {
    static char source_file_line[MAX_LEN_SOURCE_FILE_LINE];
    snprintf(source_file_line, MAX_LEN_SOURCE_FILE_LINE, "%s:%d", source_file, line);
    return source_file_line;
}

void *nmxp_mem_malloc(size_t size, char *source_file, int line) {
    void *ret = NULL;
    struct timeval tv;
    int i;
    char source_file_line[MAX_LEN_SOURCE_FILE_LINE];
    
    gettimeofday(&tv, NULL);
    ret = malloc(size);
    strncpy(source_file_line, nmxp_mem_source_file_line(source_file, line), MAX_LEN_SOURCE_FILE_LINE);
    i = nmxp_mem_add_ptr(ret, size, source_file_line, &tv);
#ifdef NMXP_MEM_DEBUG_LOG_SINGLE
    nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_ANY, "nmxp_mem_malloc %d.%d %010p+%d %s_%d\n", tv.tv_sec, tv.tv_usec, ret, size, source_file_line, i);
#endif
    return ret;
}

char *nmxp_mem_strdup(const char *str, char *source_file, int line) {
    char *ret = NULL;
    int size;
    struct timeval tv;
    int i;
    char source_file_line[MAX_LEN_SOURCE_FILE_LINE];

    gettimeofday(&tv, NULL);
    ret = strdup(str);
    if(str) {
	size = strlen(str);
    } else {
	size = 0;
    }
    strncpy(source_file_line, nmxp_mem_source_file_line(source_file, line), MAX_LEN_SOURCE_FILE_LINE);
    i = nmxp_mem_add_ptr(ret, size, source_file_line, &tv);
#ifdef NMXP_MEM_DEBUG_LOG_SINGLE
    nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_ANY, "nmxp_mem_strdup %d.%d %010p+%d %s_%d\n", tv.tv_sec, tv.tv_usec, ret, size, source_file_line, i);
#endif
    return ret;
}


void nmxp_mem_free(void *ptr, char *source_file, int line) {
    int i;
    struct timeval tv;
    int size;
    char source_file_line[MAX_LEN_SOURCE_FILE_LINE];

    if(ptr) {
	i = nmxp_mem_rem_ptr(ptr, &tv, &size);
	strncpy(source_file_line, nmxp_mem_source_file_line(source_file, line), MAX_LEN_SOURCE_FILE_LINE);
#ifdef NMXP_MEM_DEBUG_LOG_SINGLE
	nmxp_log(NMXP_LOG_NORM, NMXP_LOG_D_ANY, "nmxp_mem_free   %d.%d %010p+%d %s_%d\n", tv.tv_sec, tv.tv_usec, ptr, size, source_file_line, i);
#endif
	free(ptr);
    }
}

#endif

