/* Copyright (c) 2010-2011 Los Alamos National Security, LLC.
 *                         All rights reserved.
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

#include <stdio.h>
#include <stdarg.h>

#include "smgc_base_constants.h"
#include "smgc_base_err.h"

#define SMGC_ERR_PREFIX_STR ""PACKAGE_NAME" ERROR:"

/* ////////////////////////////////////////////////////////////////////////// */
void
smgc_err(const char *file_name,
         int lineno,
         const char *fmt,
         ...)
{
    va_list valist;
    va_start(valist, fmt);
    fprintf(stderr, "*** [%s (%s: %d)] ", SMGC_ERR_PREFIX_STR, file_name,
            lineno);
    fflush(stderr);
    vfprintf(stderr, fmt, valist);
    fflush(stderr);
    fprintf(stderr, " ***\n");
    fflush(stderr);
    va_end(valist);
}
