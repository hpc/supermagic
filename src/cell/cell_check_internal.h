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
* cell_check_internal.h - Roadrunner Cell connectivity check out routine
* 
* Common macros and subroutines for x86 and PPU modules of cell_check.
*
* Change History:
*
* 2010-09-15  cgw  Initial version  
* 2010-09-20  cgw  Clean up failure path, add CHECK macro
*
*****************************************************************************/

#ifndef _CELL_CHECK_INTERNAL_H
#define _CELL_CHECK_INTERNAL_H

#include <stdio.h>
#include <dacs.h>
#include "cw_util.h"

// Global data used by macros
struct {
    uint32_t version;
    CELL_CHECK_MSG_T msg_level;
    CELL_CHECK_TEST_T test_level;
    uint32_t fail;
} g;

#define FAIL_CODE(MAJOR,MINOR,RC) ((RC)%1000 + (MINOR)*1000 + (MAJOR)*100000 + g.version*10000000)

#define DACS(MAJOR,MINOR,API,PARAM) {\
    DACS_ERR_T dacs_rc;\
    uint32_t fail_code;\
    if (g.msg_level >= CELL_CHECK_MSG_PROGRESS) cwMsg("Call %s", #API#PARAM);\
    dacs_rc = (API PARAM);\
    if (g.msg_level >= CELL_CHECK_MSG_RESULT)\
        cwMsg("%s rc:%d [%s]", #API, dacs_rc, dacs_strerror(dacs_rc));\
    if (dacs_rc < DACS_SUCCESS) {\
        fail_code = FAIL_CODE(MAJOR, MINOR, -dacs_rc);\
        if (g.fail == 0) g.fail = fail_code;\
        if (g.msg_level >= CELL_CHECK_MSG_FAIL)\
            cwMsgE("FAIL:%lu; %s rc:%d [%s]", fail_code, #API#PARAM, dacs_rc, dacs_strerror(dacs_rc));\
    }\
}

#define CHECK(MAJOR,MINOR,COND,RC) {\
    if COND {\
        if (g.msg_level >= CELL_CHECK_MSG_PROGRESS)\
            cwMsg("Check %s OK", #COND);\
    } else {\
        uint32_t fail_code;\
        fail_code = FAIL_CODE(MAJOR, MINOR, RC);\
        if (g.fail == 0) g.fail = fail_code;\
        if (g.msg_level >= CELL_CHECK_MSG_FAIL)\
            cwMsgE("FAIL:%lu; check %s failed rc:%lu", fail_code, #COND, RC);\
    }\
}

#endif
/* --- end of cell_check_internal.h --- */
