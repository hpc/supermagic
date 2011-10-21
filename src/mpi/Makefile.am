###############################################################################
# Copyright (c) 2011      Los Alamos National Security, LLC.
#                         All rights reserved.
#
# This program was prepared by Los Alamos National Security, LLC at Los Alamos
# National Laboratory (LANL) under contract No. DE-AC52-06NA25396 with the U.S.
# Department of Energy (DOE). All rights in the program are reserved by the DOE
# and Los Alamos National Security, LLC. Permission is granted to the public to
# copy and use this software without charge, provided that this Notice and any
# statement of authorship are reproduced on all copies. Neither the U.S.
# Government nor LANS makes any warranty, express or implied, or assumes any
# liability or responsibility for the use of this software.
################################################################################

lib_LTLIBRARIES = \
libsmgc_mpi.la

libsmgc_mpi_la_SOURCES = \
smgc_mpi.h smgc_mpi.c

libsmgc_mpi_la_CFLAGS = \
-I$(top_srcdir)/src/base

libsmgc_mpi_la_LDFLAGS = \
-L$(top_srcdir)/src/base

libsmgc_mpi_la_LIBADD = \
-lsmgc_base
