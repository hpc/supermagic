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

/* @author Samuel K. Gutierrez - samuelREMOVEME@lanl.gov
 * found a bug? have an idea? please let me know.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>

#include "smgc_base_constants.h"
#include "smgc_base_err.h"
#include "smgc_mpi.h"

/* ////////////////////////////////////////////////////////////////////////// */
/* TODO it would probably be cool if each test could ouput header info */
static void
print_banner(smgc_mpi_t *mpip)
{
    printf("\n$$$ %s $$$\n\n", PACKAGE_STRING);
    printf("start yyyymmdd-hhmmss  : %s\n", mpip->start_time); 
    printf("numpe                  : %d\n", mpip->num_ranks);
    printf("bin bloat              : %d B\n", SMGC_DEFAULT_BIN_BLOAT);
    printf("default msg size       : %d B\n", smgc_mpi_get_default_msg_size());
    printf("actual msg size        : %d B\n", 0);
    printf("default file size/rank : %d B\n", 0);
    printf("actual file size/rank  : %lu B\n", 0l);
    printf("num iters              : %d\n", 0);
    printf("num tests              : %d\n", 0);
    printf("\n");
    fflush(stdout);
}

/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
/* main                                                                       */
/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
int
main(int argc,
     char **argv)
{
    int rc;
    smgc_mpi_t *smgc_mpi = NULL;

    if (SMGC_SUCCESS != (rc = smgc_mpi_construct(&smgc_mpi))) {
        /* TODO print smgc errstr */
        exit(EXIT_FAILURE);
    }
    if (SMGC_SUCCESS != (rc = smgc_mpi_init(smgc_mpi, argc, argv))) {
        /* TODO print smgc errstr */
        goto cleanup;
    }
    if (SMGC_RANK_ZERO == smgc_mpi->rank) {
        print_banner(smgc_mpi);
    }

cleanup:
    /* do we need to finalize? */
    if (smgc_mpi_initialized()) {
        if (SMGC_SUCCESS != (rc = smgc_mpi_finalize(smgc_mpi))) {
            /* TODO print smgc errstr */
            exit(EXIT_FAILURE);
        }
    }
    if (SMGC_SUCCESS != (rc = smgc_mpi_destruct(&smgc_mpi))) {
        /* TODO print smgc errstr */
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}
