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

#include <time.h>
#include <string.h>

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
