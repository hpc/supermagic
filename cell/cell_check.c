/**
 * Copyright (c) 2010-2012 Los Alamos National Security, LLC.
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
* cell_check.c - Roadrunner Cell connectivity check out routine
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
* 2010-09-16  cgw  Initial version 
* 2010-09-20  cgw  Clean up failure path, add some checks 
*
*****************************************************************************/
#include "cell_check.h"
#include "cell_check_internal.h"

unsigned long cell_check(CELL_CHECK_MSG_T msg_level, CELL_CHECK_TEST_T test_level) {
    uint32_t    num_accel = 1;
    de_id_t     accel;
    dacs_process_id_t accel_pid;
    uint32_t    msgS = 123, msgR = 0;
    int init_ok = 0, reserve_child_ok = 0, start_ok = 0, wid_reserve_ok = 0;
    dacs_wid_t  wid;
    uint32_t     accel_fail;
    int32_t exit_status;
    char * accel_proc = "cell/ppu/cell_check_ppu";
    DACS_PROC_CREATION_FLAG_T creation_flags = DACS_PROC_LOCAL_FILE;

    g.version = 11;
    g.msg_level = msg_level;
    g.test_level = test_level;
    g.fail = 0;

    cwMsgSetTag(NULL, "cell_check@$H[$A] ");
    cwSighandInit(NULL);

    // Initialize the runtime and reserve an accelerator
    DACS( 1, 1, dacs_init, (DACS_INIT_FLAGS_NONE) );
    init_ok = !g.fail;

    if (g.test_level >= CELL_CHECK_TEST_RESERVE) {
        if (!g.fail) {
            DACS( 2, 1, dacs_get_num_avail_children, (DACS_DE_CBE, &num_accel) );
        }

        if (!g.fail) CHECK( 2, 2, (num_accel >= 1), num_accel);

        num_accel = 1;

        if (!g.fail) {
          DACS( 2, 3, dacs_reserve_children, (DACS_DE_CBE, &num_accel, &accel) );
          reserve_child_ok = !g.fail;
        }

        if (!g.fail) CHECK( 2, 4, (num_accel == 1), num_accel);

        if (g.test_level >= CELL_CHECK_TEST_START) {
            // Start the accelerator process with msg & test level via argv
            char ml[8], tl[8];
            char * argv[3] = {ml, tl, NULL};
            snprintf(ml, sizeof(ml), "%d", g.msg_level);
            snprintf(tl, sizeof(tl), "%d", g.test_level);

            if (!g.fail) {
                DACS( 3, 1, dacs_de_start, (accel,(void *)accel_proc, (char const **) argv,
                                            NULL, creation_flags, &accel_pid) );
                start_ok = !g.fail;
            }
            if (g.test_level >= CELL_CHECK_TEST_SENDREC) {
                // Send message containing a single uint32_t
                if (!g.fail) {
                    DACS( 4, 1, dacs_wid_reserve, (&wid) );
                    wid_reserve_ok = !g.fail;
  		}
                if (!g.fail) {
                    DACS( 4, 2, dacs_send, (&msgS, sizeof(msgS), accel, accel_pid, 
                                            0, wid, DACS_BYTE_SWAP_WORD) );
		}
                if (!g.fail) {
                    DACS( 4, 3, dacs_wait, (wid) );
                }
                // Receive the value back, incremented
                if (!g.fail) {
                    DACS( 4, 4, dacs_recv, (&msgR, sizeof(msgR), accel, accel_pid,
                                            DACS_STREAM_ALL, wid, DACS_BYTE_SWAP_WORD) );
                }
                if (!g.fail) {
                    DACS( 4, 5, dacs_wait, (wid) );
		}

                if (!g.fail) CHECK( 4, 6, (msgR == msgS + 1), msgR);

                if (wid_reserve_ok) {
                    DACS( 4, 6, dacs_wid_release, (&wid) );
                }
            } // if (g.test_level >= CELL_CHECK_TEST_SENDREC) 

            // Wait for accelerator to finish
            if (start_ok) {
                DACS( 5, 1, dacs_mailbox_read, (&accel_fail, accel, accel_pid) );
                if (accel_fail != 0 && g.fail == 0) g.fail = accel_fail;
                CHECK(5, 2, (accel_fail == 0), accel_fail);

                DACS( 5, 3, dacs_de_wait, (accel, accel_pid, &exit_status) );
                CHECK( 5, 4, (exit_status == 0), exit_status);
            }
        } // if (g.test_level >= CELL_CHECK_TEST_START) 

        // Cleanup and exit
        if (reserve_child_ok) {
            DACS( 6, 1, dacs_release_de_list, (num_accel, &accel) );
        }
    } // if (g.test_level >= CELL_CHECK_TEST_RESERVE)
    if (init_ok) {
        DACS( 6, 2, dacs_exit, () );
    }

    cwSighandTerm();
    return g.fail;
};

/* --- end of cell_check.c --- */
