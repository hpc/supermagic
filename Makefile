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

# Author: Samuel K. Gutierrez

SHELL  = /bin/sh

SMGC_NAME = \
`cat smgc_config.h | grep SMGC_DIST_NAME \
| tr -s ' ' | cut -d ' ' -f 3 | tr -d '"'`

SM_DIST_VER = \
`cat smgc_config.h | grep SMGC_DIST_VERSION | \
tr -s ' ' | cut -d ' ' -f 3 | tr -d '"'`

SM_DIST_NAME_VER = "$(SMGC_NAME)-$(SM_DIST_VER)"

SM_DIST_NAME = "$(SM_DIST_NAME_VER).tar.gz"

SM_DIST_FILES = \
README \
Makefile \
Makefile.include \
$(shell echo *.h) \
$(shell echo *.c)

SM_DIST_CELL_FILES = \
cell_check/Makefile \
$(shell echo cell_check/*.h) \
$(shell ls cell_check/*.c) \

include Makefile.include

TARGET = supermagic

all: $(TARGET)

$(TARGET): $(TARGET).o
	$(PRETTY_MPILD) $(CFLAGS) -o $(TARGET) $^ $(LDFLAGS)

clean:
	/bin/rm -f $(TARGET) *.o cell_check_ppu
	@cd cell_check; make clean

cell:
	@cd cell_check; make all
	@cd ..
	@cp cell_check/cell_check_ppu .
	@rm -f $(TARGET).o
	@make CFLAGS+="-DWITH_CELL_TESTS" LDFLAGS+="$(CELL_LDFLAGS)" $(TARGET)

distclean:
	make clean
	@if test -f $(SM_DIST_NAME); then \
		rm -f $(SM_DIST_NAME); \
		if test "1" == "$$?"; then \
			echo "cannot rm $(SM_DIST_NAME)"; \
			exit 1; \
		fi \
	fi; \
	if test -d $(SM_DIST_NAME_VER); then \
		rm -rf $(SM_DIST_NAME_VER); \
		if test "1" == "$$?"; then \
			echo "cannot rm $(SM_DIST_NAME_VER)"; \
			exit 1; \
		fi \
	fi;

dist:
	make distclean
	@mkdir -p $(SM_DIST_NAME_VER)/cell_check && \
	cp $(SM_DIST_FILES) $(SM_DIST_NAME_VER) && \
	cp $(SM_DIST_CELL_FILES) $(SM_DIST_NAME_VER)/cell_check && \
	tar czvf $(SM_DIST_NAME) $(SM_DIST_NAME_VER) && \
	rm -rf $(SM_DIST_NAME_VER);

.c.o:
	$(PRETTY_MPICC) -o $@ -c $(CFLAGS) $<
