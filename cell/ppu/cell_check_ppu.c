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
* cell_check_ppu.c - Roadrunner Cell connectivity check out routine
* 
* Perform a series of basic tests to ensure that Cells can be reserved,
* PPU programs launched and basic message function.
*
* Change History:
*
* 2010-09-16  cgw  Initial version  
* 2010-09-20  cgw  Clean up failure path
*
*****************************************************************************/
#include <stdlib.h>
#include "cell_check.h"
#include "cell_check_internal.h"

int main(int argc, char * * argv) {
    dacs_wid_t wid;
    uint32_t msgR, msgS;
    int init_ok = 0, wid_reserve_ok = 0;

    cwMsgSetTag(NULL, "cell_check@$H[$A] ");
    cwSighandInit(NULL);
    g.version = 11;
    g.fail = 0;
    g.msg_level = CELL_CHECK_MSG_DEBUG;
    g.test_level = CELL_CHECK_TEST_INIT;

    if (argc >= 2) g.msg_level = strtol(argv[1], NULL, 10);
    if (argc >= 3) g.test_level = strtol(argv[2], NULL, 10);

    // Initialize the runtime
    DACS( 51, 1, dacs_init, (DACS_INIT_FLAGS_NONE) );
    init_ok = !g.fail;

    if (g.test_level >= CELL_CHECK_TEST_SENDREC) {
        // Receive a message containing a single uint32_t 
        if (! g.fail) {
            DACS( 52, 1, dacs_wid_reserve, (&wid) );
            wid_reserve_ok = !g.fail;
        }
        if (! g.fail) {
            DACS( 52, 2, dacs_recv, (&msgR, sizeof(msgR), DACS_DE_PARENT, DACS_PID_PARENT,
                                     DACS_STREAM_ALL, wid, DACS_BYTE_SWAP_WORD) );
        } 
        if (! g.fail) {
            DACS( 52, 3, dacs_wait, (wid) );
        }
        // Send the uint32_t back, incremented by 1
        if (! g.fail) {
            msgS = msgR + 1;
            DACS( 52, 4, dacs_send, (&msgS, sizeof(msgS), DACS_DE_PARENT, DACS_PID_PARENT,
                                     0, wid, DACS_BYTE_SWAP_WORD) );
        }
        if (! g.fail) {        
            DACS( 52, 5, dacs_wait, (wid) );
        }
        if (wid_reserve_ok) {
            DACS( 52, 6, dacs_wid_release, (&wid) );
        }
    } // if (g.test_level >= CELL_CHECK_TEST_SENDREC) 

    if (init_ok) {
        DACS( 53, 1, dacs_mailbox_write, (&g.fail, DACS_DE_PARENT, DACS_PID_PARENT) );
        DACS( 53, 2, dacs_exit, () );
    } 

    cwSighandTerm();
    return g.fail;
};

/* --- end of cell_check_ppu.c --- */
