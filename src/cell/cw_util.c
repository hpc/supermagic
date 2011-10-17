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
// cw_util.c - convenience functions
//
// Change History:
// ---------------
// Date        Who  Description
// 2009-10-14  cgw  Initial version
// 2010-09-16  cgw  Extract from dacsx.c
// 2010-09-20  cgw  Add cwMsgE
//****************************************************************************
#define _GNU_SOURCE
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <sched.h>
#include <errno.h>
#include <signal.h>
#include <execinfo.h>
#define __USE_GNU 1
#include <string.h>
#include <ucontext.h>
#undef __USE_GNU
#ifdef CW_MPI
  #include <mpi.h>
#endif // ifdef CW_MPI
#include "cw_util.h"

//-----------------------------------------------------------------------------
// Return CPU affinity in a form suitable for messages.  Single CPU affinity
// returns a non-negative integer CPU number.  Multi CPU affinity returns the
// negative of the bit mask of affine CPUs.  Affinity to no CPUs returns -1.
//----------------------------------------------------------------------------
int32_t cwGetCPUaffinity(void) {
  int numCPU = sysconf( _SC_NPROCESSORS_CONF );
  cpu_set_t af;
  int32_t i, afmask = 0, afcount = 0, afCPU=-1;

  sched_getaffinity(0, sizeof(af), &af);

  for (i=0; i<numCPU; ++i) {
    if (CPU_ISSET(i, &af)) {
      afCPU = i;
      afmask |= (1 << i);
      afcount++;
    }
  }
  if (afcount <= 1) return afCPU;
  else return -afmask;
}

//-----------------------------------------------------------------------------
// Replace $H, $A & $R tags in string.  See cwMsgTagSet for more information.
//-----------------------------------------------------------------------------
void cwDoSub(char * src, char * dest, int destLen) {
  char * ptr = dest;
#ifdef CW_MPI
  int myRank;
#endif  // ifdef CW_MPI
  while (*src && ptr < (dest + destLen)) {
    if (*src != '$') {
      *ptr++ = *src++;
    } else {  
      switch (*(++src)) {
      case 'H':
        gethostname(ptr, dest + destLen - ptr);
        ptr = strchrnul(ptr, '.');
        src++;
        break;
      case 'A': 
        ptr += snprintf(ptr, dest + destLen - ptr, "%d", cwGetCPUaffinity());
        src++;
        break;
#ifdef CW_MPI
      case 'R':
        if (MPI_SUCCESS == MPI_Comm_rank(MPI_COMM_WORLD, &myRank)) {
          ptr += snprintf(ptr, dest + destLen - ptr, "%d", myRank);
        } else {
          *ptr++ = '*';
        }
        src++;
        break;
#endif // ifdef CW_MPI
      case '$':
        *ptr++ = '$';
        src++;
        break;
      default:   // Also handles '\0' case
        *ptr++ = '$';
        break;     
      }
    }
  }
  *(MIN(ptr, dest+destLen-1)) = '\0';
}  


//-----------------------------------------------------------------------------
// Static structure to store log tag strings
//-----------------------------------------------------------------------------
  static struct cwMsgParam {
    char * tagStr;                // Formatted tag string or null
    char * tsFormat;              // Format string for timestamp or null
    int initFlag;
  } cwMsgParam = {NULL, NULL, 0};

//-----------------------------------------------------------------------------
// cwMsgSetTag - set leading message tags.  The parameters provided are
// an strftime format string and a printf format string and
// parameters.  In both format strings, the following additional
// substitutions will be performed when cwMsgSetTag() is called:
//   $H --> short hostname
//   $A --> CPU affinity
//   $R --> MPI rank (-DCW_MPI only)
//
// If tsFormat and/or tagFormat are null, the current value will not
// be changed.  If either is an empty string, it will not be prepended
// to the message. 
//
// If cwMsgSetTag has not been called, the default message tags are:
//   "%Y%m%d_%H:%M:%S" "$H.$A"
//-----------------------------------------------------------------------------
void cwMsgSetTag(char * tsFormat, char * tagFormat, ...) {
  va_list ap;
  char tmp1[200], tmp2[200];

  if (!cwMsgParam.initFlag) {
    cwMsgParam.initFlag = 1;
    cwMsgSetTag("%Y%m%d_%H:%M:%S ", "$H.$A ");
  }

  if (tsFormat) {
    if (strlen(tsFormat) == 0) {
      cwMsgParam.tsFormat = NULL;
    } else { 
      cwDoSub(tsFormat, tmp2, sizeof(tmp2));
      cwMsgParam.tsFormat = strdup(tmp2);
    }
  }

  if (tagFormat) {
    if (strlen(tagFormat) == 0) {
      cwMsgParam.tagStr = NULL;
    } else { 
      va_start(ap, tagFormat);
      vsnprintf(tmp1, sizeof(tmp1), tagFormat, ap);
      va_end(ap); 
      cwDoSub(tmp1, tmp2, sizeof(tmp2));
      cwMsgParam.tagStr = strdup(tmp2);
    }
  }

}

#define CW_MSG_IMPL(NAME, FILE) \
void NAME(char * format, ...) {\
  va_list ap;\
  char tmp_str[512] = "";\
\
  if (!cwMsgParam.initFlag) cwMsgSetTag(NULL, NULL);\
\
  if (cwMsgParam.tsFormat) {\
    time_t curtime = time(NULL);\
    struct tm *loctime = localtime(&curtime);\
    strftime(tmp_str, sizeof(tmp_str), cwMsgParam.tsFormat, loctime);\
  }\
\
  if (cwMsgParam.tagStr) {\
    strncat(tmp_str, cwMsgParam.tagStr, sizeof(tmp_str)-strlen(tmp_str)-1);\
  }\
\
  va_start(ap, format);\
  vsnprintf(tmp_str + strlen(tmp_str), sizeof(tmp_str) - strlen(tmp_str), format, ap);\
  va_end(ap);\
  strncat(tmp_str, "\n", sizeof(tmp_str)-strlen(tmp_str)-1);\
\
  fputs(tmp_str, FILE);\
  fflush(FILE);\
}\

CW_MSG_IMPL(cwMsg, stdout)
CW_MSG_IMPL(cwMsgE, stderr)

//-----------------------------------------------------------------------------
// Signal handling
//-----------------------------------------------------------------------------
static int cwSighandSig[] = {SIGFPE, SIGILL, SIGSEGV, SIGBUS, SIGABRT, SIGTERM};
#define SIGCT DIM1(cwSighandSig)
static struct sigaction cwSighandOldact[SIGCT];
static int * cwSighandLastsigPtr = NULL;

void cwSighandHandler(int signo, siginfo_t * siginfo, void * context) {
    void *   array [32];
    int      i;
    size_t   size;
    char * * strings;

    if (cwSighandLastsigPtr) *cwSighandLastsigPtr = signo;
    /* Print summary message */
    if (signo == SIGSEGV || signo == SIGFPE || signo == SIGILL || signo == SIGBUS) {
      cwMsg("%s(%d) errno:%d code:%d address %p",
	     strsignal(signo), signo, siginfo->si_errno, siginfo->si_code,
             siginfo->si_addr);
    } else {   
        cwMsg("%s(%d) errno:%d code:%d",
		strsignal(signo), signo, siginfo->si_errno, siginfo->si_code);
    }

    if (signo == SIGTERM) { 
      // For sigterm, just re-enable this handler and return
      struct sigaction handler;

      memset(&handler, 0, sizeof(handler));
      handler.sa_sigaction = &cwSighandHandler;
      handler.sa_flags = SA_SIGINFO | SA_RESTART;
      sigaction(SIGTERM, &handler, NULL);
    } else {
      // For error signals, print stack trace, restore prior handler and re-raise signal

      /* Print stack trace */
      size = backtrace(array, DIM1(array));
      strings = backtrace_symbols(array, size);

      if (strings) {
          for (i=0; i<(int)size; i++) {
              if (strings[i]) {
		cwMsg("[%2d] %s", i, strings[i]);
              }
            }
        free(strings);
      }
      sleep(5);  
 
      /* Restore original signal handlers and re-raise signal */ 
      cwSighandTerm();  
      raise(signo);
    }
}

void cwSighandInit(int * lastsig_ptr) {
    int rc;
    unsigned i;
    struct sigaction handler;

    memset(&handler, 0, sizeof(handler));
    handler.sa_sigaction = &cwSighandHandler;
    handler.sa_flags = SA_SIGINFO | SA_RESTART;

    for (i=0; i<SIGCT; ++i) {
        rc = sigaction(cwSighandSig[i], &handler, &cwSighandOldact[i]);
        if (rc != 0) {
            cwMsg("sighand_init: sigaction %s(%d) failed %s(%d)",
                   strsignal(cwSighandSig[i]), cwSighandSig[i], strerror(errno), errno);
            raise(SIGTERM);
        }
    }
    cwSighandLastsigPtr = lastsig_ptr;
    if (cwSighandLastsigPtr) *cwSighandLastsigPtr = 0;
    return;
}

void cwSighandTerm(void) {
    int rc;
    unsigned i;

    /* Restore original signal handlers */
    for (i=0; i<SIGCT; ++i) {
        rc = sigaction(cwSighandSig[i], &cwSighandOldact[i], NULL);
        if (rc != 0) {
            cwMsg("cw_sighand_term: sigaction %s(%d) failed %s(%d)",
                   strsignal(cwSighandSig[i]), cwSighandSig[i],
                   strerror(errno), errno);
            raise(SIGTERM);
        }
    }

    return;
}

// --- end of cw_util.c ---

