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

#include <stdlib.h>
#include <stdio.h>

#include "smgc_constants.h"
#include "smgc_mpi.h"
#include "smgc_err.h"

#include "mpi.h"

/* ////////////////////////////////////////////////////////////////////////// */
char *
smgc_mpi_rc2estr(int rc)
{
    static char errstr[MPI_MAX_ERROR_STRING];
    int elen;
    MPI_Error_string(rc, errstr, &elen);
    return errstr;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
smgc_mpi_init(smgc_mpi_t *mpip,
              int argc,
              char **argv)
{
    int rc;
    char *bad_func = NULL;

    if (MPI_SUCCESS != (rc = MPI_Init(&argc, &argv))) {
        bad_func = "MPI_Init";
        goto cleanup;
    }
    if (MPI_SUCCESS != (rc = MPI_Comm_rank(MPI_COMM_WORLD, &mpip->rank))) {
        bad_func = "MPI_Comm_rank";
        goto cleanup;
    }
    if (MPI_SUCCESS != (rc = MPI_Comm_rank(MPI_COMM_WORLD, &mpip->num_ranks))) {
        bad_func = "MPI_Comm_rank";
        goto cleanup;
    }
    mpip->smp_id = SMGC_SMP_ID_INVALID;
    mpip->smp_comm = MPI_COMM_NULL;

cleanup:
    if (NULL != bad_func) {
        smgc_err(__FILE__, __LINE__, "%s failure detected [mpi rc: %d (%s)]\n",
                 bad_func, rc, smgc_mpi_rc2estr(rc));
        return SMGC_FAILURE_MPI;
    }
    return SMGC_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
smgc_mpi_finalize(smgc_mpi_t *mpip)
{
    int rc;
    char *bad_func = NULL;

    if (MPI_SUCCESS != (rc = MPI_Finalize())) {
        bad_func = "MPI_Finalize";
        goto bail;
    }
    mpip->rank = SMGC_RANK_INVALID;
    mpip->num_ranks = 0;
    mpip->smp_id = SMGC_SMP_ID_INVALID;
    mpip->smp_comm = MPI_COMM_NULL;

bail:
    if (NULL != bad_func) {
        /* TODO add err msg */
        return SMGC_FAILURE_MPI;
    }
    return SMGC_SUCCESS;
}
