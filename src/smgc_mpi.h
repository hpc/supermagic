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

#ifndef SMGC_MPI_INCLUDED
#define SMGC_MPI_INCLUDED

#include "mpi.h"

#define SMGC_WORKER_ID_INVALID -1

typedef smgc_mpi_t {
    int rank;
    int num_ranks;
    int smp_id;
    MPI_Comm smp_comm;
} smgc_mpi_t;

char *
smgc_mpi_rc2estr(int rc);

int
smgc_mpi_init(smgc_mpi_t *mpip,
              int argc,
              char **argv);

#endif /* ifndef SMGC_MPI_INCLUDED */
