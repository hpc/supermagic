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
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>

#include "smgc_base_constants.h"
#include "smgc_base_util.h"
#include "smgc_base_err.h"
#include "smgc_mpi.h"

#include "mpi.h"

/* default message sizez (in B) */
#define SMGC_DEFAULT_MSG_SIZE 1024

/* define integer constants for getop_long_only - NOTE: NOT valid characters */
#define LONG_OPT_HELP         0x100
#define LONG_OPT_USAGE        0x101
#define LONG_OPT_MSG_SIZE     0x102

/* options for mpi */
static struct option long_options[] = {
    {"mpi-message-size", required_argument, NULL, LONG_OPT_MSG_SIZE},
    {NULL, 0, NULL, 0}
};

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

/* ////////////////////////////////////////////////////////////////////////// */
static int
process_user_args(smgc_mpi_t *mpip,
                  int argc,
                  char **argv)
{
    int opt_index = 0, c = 0;
    /* suppress getopt_long_only error messages */
    opterr = 0;

    while (true) {
        opt_index = 0;

        c = getopt_long_only(argc, argv, "", long_options, &opt_index);

        if (-1 == c) {
            break;
        }

        switch (c) {
            /* check for unknown option or missing argument */
            case '?':
                printf("? %d\n", optopt);
                break;
            /* change message size for use in mpi communication */
            case (int)LONG_OPT_MSG_SIZE:
                mpip->msg_size = (int)atoi(optarg);
                break;
            default:
                return SMGC_FAILURE;
                break;
        }
    }
    return SMGC_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
smgc_mpi_get_default_msg_size(void)
{
    return SMGC_DEFAULT_MSG_SIZE;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
smgc_mpi_construct(smgc_mpi_t **mpip,
                   int argc,
                   char **argv)
{
    smgc_mpi_t *smpip = NULL;
    int rc = SMGC_FAILURE;

    if (NULL == mpip) return SMGC_FAILURE_INVALID_ARG;

    if (NULL == (smpip = (smgc_mpi_t *)calloc(1, sizeof(smgc_mpi_t)))) {
        smgc_err(__FILE__, __LINE__, "out of resources");
        smpip = NULL;
        return SMGC_FAILURE_OOR;
    }
    /* set host immediately for error reporting */
    if (0 != gethostname(smpip->host, MPI_MAX_PROCESSOR_NAME - 1)) {
        int err = errno;
        smgc_err(__FILE__, __LINE__, "gethostname failure: %d (%s)", err,
                 strerror(err));
        rc = SMGC_FAILURE;
        goto fail;
    }
    if (NULL == (smpip->start_time = smgc_get_time_str())) {
        smgc_err(__FILE__, __LINE__, "out of resources");
        rc = SMGC_FAILURE_OOR;
        goto fail;
    }
    /* set everything else to default values */
    smpip->rank = SMGC_RANK_INVALID;
    smpip->num_ranks = 0;
    smpip->smp_id = SMGC_SMP_ID_INVALID;
    smpip->msg_size = 0;
    smpip->smp_comm = MPI_COMM_NULL;

    char **argvdup = smgc_base_util_dupargv(argc, argv);

    /* now process user args */
    process_user_args(smpip, argc, argvdup);

    *mpip = smpip;
    return SMGC_SUCCESS;

fail:
    smgc_mpi_destruct(mpip);
    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
smgc_mpi_destruct(smgc_mpi_t **mpip)
{
    smgc_mpi_t *smpip = NULL;

    if (NULL == mpip) return SMGC_FAILURE_INVALID_ARG;

    smpip = *mpip;

    if (NULL != smpip->start_time) {
        free(smpip->start_time);
        smpip->start_time = NULL;
    }
    if (NULL != smpip) {
        free(smpip);
        smpip = NULL;
    }
    return SMGC_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
char *
smgc_mpi_rc2estr(int rc)
{
    static char errstr[MPI_MAX_ERROR_STRING];
    int elen;
    if (smgc_mpi_initialized()) {
        MPI_Error_string(rc, errstr, &elen);
    }
    else {
        snprintf(errstr, MPI_MAX_ERROR_STRING - 1, "%s",
                 "mpi not initialized: error unknown");
    }
    return errstr;
}

/* ////////////////////////////////////////////////////////////////////////// */
bool
smgc_mpi_initialized(void)
{
    int flag, rc;

    if (MPI_SUCCESS != (rc = MPI_Initialized(&flag))) {
        smgc_err(__FILE__, __LINE__, "%s failure detected [mpi rc: %d (%s)]",
                 "MPI_Initialized", rc, smgc_mpi_rc2estr(rc));
        return false;
    }
    return (bool)flag;
}

/* ////////////////////////////////////////////////////////////////////////// */
int
smgc_mpi_init(smgc_mpi_t *mpip,
              int argc,
              char **argv)
{
    int rc;
    char *bad_func = NULL;

    if (NULL == mpip) return SMGC_FAILURE_INVALID_ARG;


    if (MPI_SUCCESS != (rc = MPI_Init(&argc, &argv))) {
        bad_func = "MPI_Init";
        goto cleanup;
    }
    if (MPI_SUCCESS != (rc = MPI_Comm_rank(MPI_COMM_WORLD, &mpip->rank))) {
        bad_func = "MPI_Comm_rank";
        goto cleanup;
    }
    if (MPI_SUCCESS != (rc = MPI_Comm_size(MPI_COMM_WORLD, &mpip->num_ranks))) {
        bad_func = "MPI_Comm_size";
        goto cleanup;
    }

cleanup:
    if (NULL != bad_func) {
        smgc_err(__FILE__, __LINE__, "%s failure detected [mpi rc: %d (%s)]",
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

    if (NULL == mpip) return SMGC_FAILURE_INVALID_ARG;

    if (MPI_COMM_NULL != mpip->smp_comm) {
        if (MPI_SUCCESS != (rc = MPI_Comm_free(&mpip->smp_comm))) {
            bad_func = "MPI_Comm_free";
            goto out;
        }
    }
    if (MPI_SUCCESS != (rc = MPI_Finalize())) {
        bad_func = "MPI_Finalize";
        goto out;
    }

out:
    if (NULL != bad_func) {
        smgc_err(__FILE__, __LINE__, "%s failure detected [mpi rc: %d (%s)]",
                 bad_func, rc, smgc_mpi_rc2estr(rc));
        return SMGC_FAILURE_MPI;
    }
    return SMGC_SUCCESS;
}
