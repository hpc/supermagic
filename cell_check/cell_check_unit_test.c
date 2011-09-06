/**
 * Copyright (c) 2010-2011 Los Alamos National Security, LLC.
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

/*****************************************************************************
* cell_check_unit_test.c
* 
* Invoke cell_check for debugging.
*
* Change History:
*
* 2010-09-15  cgw  Initial version  
*
*****************************************************************************/
#include <stdio.h>
#include "cell_check.h"


int main(int argc, char ** argv) {
  (void)argc;
  (void)argv;

  unsigned long rc = cell_check(CELL_CHECK_MSG_FAIL, CELL_CHECK_TEST_ALL);
  printf("cell_check() rc:%lu\n", rc);

  return 0;
}
 
/* --- end of cell_check_unit_test.h --- */
