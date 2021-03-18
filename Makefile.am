###############################################################################
# Copyright (c) 2011-2021 Los Alamos National Security, LLC.
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

bin_PROGRAMS = supermagic

SUBDIRS = cell

supermagic_SOURCES = \
supermagic.h supermagic.c

supermagic_CFLAGS =
supermagic_LDFLAGS =
supermagic_LDADD =

if SMGC_BUILD_CELL
supermagic_CFLAGS += -I$(top_srcdir)/cell
supermagic_LDFLAGS += -L$(top_srcdir)/cell
supermagic_LDADD += -lcell_check
endif

EXTRA_DIST = \
Makefile.top AUTHORS README.md \
THANKS COPYRIGHT autogen \
ChangeLog \
util/crunch-hang util/rr-job-prep util/rr-run-job

ACLOCAL_AMFLAGS = -I m4

dist-hook:
	ln -s Makefile.top $(distdir)/Makefile
