###############################################################################
# Copyright (c) 2010-2011 Los Alamos National Security, LLC.
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

################################################################################
# Makefile for cell_check and cell_check_unit_test
#
# Change History:
#
# Date
# 2010-09-16  cgw  Initial Version
# 2010-09-20  cgw  Use DaCS debug libraries
################################################################################

include ../Makefile.include

LDFLAGS = -L /usr/lib64/dacs/debug -l dacs_hybrid
LDFLAGSPPU = -L /opt/cell/sysroot/usr/lib64/dacs/debug -l dacs_hybrid -lstdc++

all: cell_check_ppu cell_check_unit_test libcell_check.a

cell_check_ppu: cell_check_ppu.o cw_util_ppu.o
	${PRETTY_PPULD} $(CFLAGS) -o $@ $(LDFLAGSPPU) $^

cell_check_ppu.o: cell_check_ppu.c cell_check.h cell_check_internal.h cw_util.h
	${PRETTY_PPUCC} -c $(CFLAGS) -o $@ $<

cw_util_ppu.o: cw_util.c cw_util.h
	${PRETTY_PPUCC} -c $(CFLAGS) -o $@ $<

cell_check_unit_test: cell_check_unit_test.o cell_check.o cw_util.o
	${PRETTY_LD} $(CFLAGS) -o $@ $(LDFLAGS) $^

cell_check_unit_test.o: cell_check_unit_test.c cell_check.h
	${PRETTY_CC} -c $(CFLAGS) -o $@ $<

cell_check.o: cell_check.c cell_check.h cell_check_internal.h cw_util.h
	${PRETTY_CC} -c $(CFLAGS) -o $@ $<

cw_util.o: cw_util.c cw_util.h
	${PRETTY_CC} -c $(CFLAGS) -o $@ $<

libcell_check.a: cell_check.o cw_util.o
	${PRETTY_AR} -rcu $@ $^ 
	${PRETTY_RANLIB} $@

clean:
	/bin/rm -f cell_check_ppu cell_check_unit_test *.o *.a
