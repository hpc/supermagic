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
* cell_check.h - Roadrunner Cell connectivity check out routine
* 
* Perform a series of basic tests to ensure that Cells can be reserved,
* PPU programs launched and basic message function.
*
* Typical invocation from a higher level test program:
*
*   rc = cell_check(CELL_CHECK_MSG_FAIL, CELL_CHECK_TEST_ALL);
*   if (rc != 0) printf("cell_check failure rc=%d\n", rc);
*
* Change History:
*
* 2010-09-15  cgw  Initial version  
*
*****************************************************************************/

#ifndef _CELL_CHECK_H
#define _CELL_CHECK_H

typedef enum {
  CELL_CHECK_MSG_NONE,		// No messages ever, results via RC
  CELL_CHECK_MSG_FAIL,		// Failures only, to stderr
  CELL_CHECK_MSG_RESULT,	// Results to stdout
  CELL_CHECK_MSG_PROGRESS,      // Test start to stdout
  CELL_CHECK_MSG_DEBUG,		// Debug messages        
} CELL_CHECK_MSG_T;

typedef enum {
  CELL_CHECK_TEST_INIT,	        // dacs_init/dacs_exit only
  CELL_CHECK_TEST_RESERVE,	// Reserve Cells
  CELL_CHECK_TEST_START,	// Launch PPU program
  CELL_CHECK_TEST_SENDREC,	// Send messages to/from PPU
  CELL_CHECK_TEST_ALL		// Perform all tests
} CELL_CHECK_TEST_T;

unsigned long cell_check(CELL_CHECK_MSG_T msg_level, CELL_CHECK_TEST_T test_level);

#endif
/* --- end of cell_check.h --- */
