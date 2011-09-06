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

//****************************************************************************
// cw_util.h - convenience functions
//
// Change History:
// ---------------
// Date        Who  Description
// 2009-10-14  cgw  Initial version
// 2010-09-16  cgw  Extract from dacsx.hi
// 2010-09-20  cgw  Remove unneeded stdlib include
//****************************************************************************
#ifndef _CW_UTIL_H
#define _CW_UTIL_H 1

//-----------------------------------------------------------------------------
// To use this package, certain preprocessr variables may need to be defined
// to ensure correct compilation of this header file and of dacsx.c. 
//
// On x86:  Define CW_MPI to include support for the $R (MPI rank) tag.  This
//          will cause mpi.h to be included and MPI_Comm_rank to be called.
//
// On PPU:  No preprocessr variables need to be defined
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// cwMsg and cwMsgE - Write a message to stdout or to stderr that are
// optionally prefixed with timestamp and node identification tags.
//-----------------------------------------------------------------------------
void cwMsg(char * format, ...);
void cwMsgE(char * format, ...);

//-----------------------------------------------------------------------------
// cwMsgSetTag - set leading message tags. The parameters provided are an
// strftime format string and a printf format string and parameters. In both
// format strings (except on the SPU) the following additional substitutions
// will be performed when cwMsgSetTag() is called:
//   $H --> short hostname  
//   $A --> CPU affinity
//   $R --> MPI rank (-DCW_MPI only)
//
// If tsFormat and/or tagFormat are null, the current value will not be changed.
// If either is an empty string, it will not be prepended to the message. 
//
// If cwMsgSetTag has not been called, the default message tags are:
//   "%Y%m%d_%H:%M:%S" and "$H.$A".  
//
// The $R tag should not be used until after MPI_Init() has been called.
//-----------------------------------------------------------------------------
void cwMsgSetTag(char * tsFormat, char * tagFormat, ...);

//-----------------------------------------------------------------------------
// Enable signal handlers that will print a stack backtrace if an error
// signal occurs.  If lastsig_ptr is not NULL, whenever a signal occurs,
// the indicated int will be set to the signal number.  This can be used to 
// detect termination signals.  Not available on SPU.
//-----------------------------------------------------------------------------
void cwSighandInit(int * lastsig_ptr);
void cwSighandTerm(void);


//----------------------------------------------------------------------------
// Convenience macros
//----------------------------------------------------------------------------
// 1st, 2nd and 3rd dimensions of declared arrays
#define DIM1(array) ( sizeof(array) / sizeof(array[0]) )
#define DIM2(array) ( sizeof(array[0]) / sizeof(array[0][0]) )
#define DIM3(array) ( sizeof(array[0][0]) / sizeof(array[0][0][0]) )
// Round up or down to a power of 2 boundry
#define BDYUP(x, pwr2) ((unsigned)((x)+(pwr2)-1) & ~((unsigned)(pwr2)-1))
#define BDYDN(x, pwr2) ((unsigned)(x) & ~((unsigned)(pwr2)-1))
// Min and Max
#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )
#endif

// --- end of cw_util.h ---
