/* Copyright (c) 2011 Los Alamos National Security, LLC.
 *                    All rights reserved.
 *
 * This program was prepared by Los Alamos National Security, LLC at Los Alamos
 * National Laboratory (LANL) under contract No. DE-AC52-06NA25396 with the U.S.
 * Department of Energy (DOE). All rights in the program are reserved by the DOE
 * and Los Alamos National Security, LLC. Permission is granted to the public to
 * copy and use this software without charge, provided that this Notice and any
 * statement of authorship are reproduced on all copies. Neither the U.S.
 * Government nor LANS makes any warranty, express or implied, or assumes any
 * liability or responsibility for the use of this software.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "smgc_base_err.h"

#include <stdlib.h>
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_STRINGS_H
#include <string.h>
#endif

#define SMGC_DATE_FORMAT_STR "%Y%m%d-%H%M%S"
#define SMGC_TIME_MAX_STR_LEN 32

/* ////////////////////////////////////////////////////////////////////////// */
char *
smgc_get_time_str(void)
{
    time_t raw_time;
    char tbuf[SMGC_TIME_MAX_STR_LEN];
    struct tm *tmp = NULL;

    time(&raw_time);
    tmp = localtime(&raw_time);

    strftime(tbuf, SMGC_TIME_MAX_STR_LEN - 1, SMGC_DATE_FORMAT_STR,
             tmp);
    return strdup(tbuf);
}

/* ////////////////////////////////////////////////////////////////////////// */
void
smgc_base_util_freeargv(char **argv)
{
    char **tmp;
    if (NULL == argv) return;
    for (tmp = argv; NULL != *tmp; ++tmp) {
        free(*tmp);
        *tmp = NULL;
    }
    free(argv);
}

/* ////////////////////////////////////////////////////////////////////////// */
char **
smgc_base_util_dupargv(int argc,
                       char **argv)
{
    char **dup = NULL;
    int i;

    if (NULL == argv) return NULL;

    /* allocate one extra to cap with NULL */
    if (NULL == (dup = (char **)calloc(argc + 1, sizeof(char *)))) {
        smgc_err(__FILE__, __LINE__, "out of resources");
        return NULL;
    }
    for (i = 0; NULL != argv[i]; ++i) {
            int len = strlen(argv[i]) + 1;
            if (NULL == (dup[i] = calloc(len, sizeof(char *)))) {
                smgc_base_util_freeargv(dup);
                return NULL;
            }
            (void)memmove(dup[i], argv[i], len);
    }
    dup[i] = NULL;
    return dup;
}
