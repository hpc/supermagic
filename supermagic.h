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

/**
 * @author Samuel K. Gutierrez - samuelREMOVEME@lanl.gov
 * found a bug? have an idea? please let me know.
 */

#ifndef SUPERMAGIC_H
#define SUPERMAGIC_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include "mpi.h"

/* only include the following if building with cell support */
#if WITH_CELL_TESTS == 1
#include "cell_check/cell_check.h"
#endif

/* binary bloat size (B) - default is 1 MB */
#ifndef SMGC_BIN_SIZE
    #define SMGC_BIN_SIZE (1 << 20)
#elif defined SMGC_BIN_SIZE && (SMGC_BIN_SIZE) <= 0
    #undef SMGC_BIN_SIZE
    #define SMGC_BIN_SIZE 1
#endif

/* message size default */
#define SMGC_MSG_SIZE (512 * 1024)

/* messaging timeout macros */
#define TIMER_ENABLE(itimer)                                                   \
do {                                                                           \
    if (msg_timeout > 0) {                                                     \
        (itimer).it_value.tv_sec = msg_timeout;                                \
        (itimer).it_value.tv_usec = 0;                                         \
        (itimer).it_interval = (itimer).it_value;                              \
        signal(SIGALRM, &kill_mpi_messaging);                                  \
        setitimer(ITIMER_REAL, &(itimer), NULL);                               \
    }                                                                          \
} while (0)

#define TIMER_DISABLE(itimer)                                                  \
do {                                                                           \
    if (msg_timeout > 0) {                                                     \
        (itimer).it_value.tv_sec = 0;                                          \
        (itimer).it_value.tv_usec = 0;                                         \
        (itimer).it_interval = (itimer).it_value;                              \
        setitimer(ITIMER_REAL, &(itimer), NULL);                               \
    }                                                                          \
} while (0)

/* invalid color - all valid colors are expected to be positive values */
#define SMGC_COLOR_INVALID -1

/* return codes used for internal purposes */
enum {
    SMGC_SUCCESS = 0,
    SMGC_ERROR
};

enum {
    IO_STATS_MBS = 0,
    IO_STATS_TIME_S
};

#define SMGC_USAGE                                                             \
"usage:\n"                                                                     \
"    mpirun -np N ./"PACKAGE_NAME" [OPTION] ... \n\n"                          \
"options:\n"                                                                   \
"    [-a|--all]                   run all tests in suite\n"                    \
"    [-h|--help]                  display this message\n"                      \
"    [-m|--msg-size x[B,k,M,G]]   change message size\n"                       \
"    [-M|--file-size B[B,k,M,G]]  change file size (per rank)\n"               \
"    [-n|--n-iters X]             run X iterations of a test suite\n"          \
"    [-q|--quiet]                 run in quiet mode\n"                         \
"    [-s|--stat /a/path]          add /a/path to stat list\n"                  \
"    [-t|--with-tests t1[,t2,tn]] run tests in requested order\n"              \
"    [-w|--write /a/path]         add /a/path to IO tests\n"                   \
"    [-V|--verbose]               display verbose output\n"                    \

#define SMGC_EXAMPLE                                                           \
"example:\n"                                                                   \
"    mpirun -np 4 ./"PACKAGE_NAME" -s /glob/usr/file -s /usr/proj -n 2\n"

/* "master" rank */
#define SMGC_MASTER_RANK       0
#define SMGC_MAX_TIME_LEN      32
#define SMGC_HOST_NAME_MAX     MPI_MAX_PROCESSOR_NAME
#define SMGC_PATH_MAX          256
#define SMGC_MAX_TESTS         64

#define SMGC_MBS_UNIT_STR     "MB/s"
#define SMGC_TIME_S_UNIT_STR  "s"
#define SMGC_DATE_FORMAT      "%Y%m%d-%H%M%S"
#define SMGC_MPI_FILE_NAME    "FS_TEST_FILE-YOU_CAN_DELETE_ME"

/* dictates what a "large" job is (in rank processes) */
#define SMGC_LRG_JB            256
/* maximum amount of stat paths */
#define SMGC_MAX_STAT_PATHS    256
/* maximum amount of fs test paths */
#define SMGC_MAX_FS_TEST_PATHS 64
/* default number of test iterations */
#define SMGC_DEF_NUM_ITRS      1
/* large job message size */
#define SMGC_LRG_JB_MSG_SIZE   1024
/* for converting bandwidth to MB/s */
#define SMGC_MB_SIZE           (1 << 20)
/* i/o file size: 4 KB per rank process is the default */
#define SMGC_MPI_IO_BUFF_SIZE (1 << 12)

/* stringification stuff */
#define SMGC_STRINGIFY(x)     #x
#define SMGC_TOSTRING(x)      SMGC_STRINGIFY(x)

/* error reporting macros */
#define SMGC_ERR_AT           __FILE__ " ("SMGC_TOSTRING(__LINE__)")"
#define SMGC_ERR_PREFIX       "-[SMGC ERROR: "SMGC_ERR_AT" <results> FAILED ]- "
#define SMGC_MSG_PREFIX       "-[supermagic]- "

/* error message */
#define SMGC_ERR_MSG(pfargs...)                                                \
do {                                                                           \
    fprintf(stderr, SMGC_ERR_PREFIX);                                          \
    fprintf(stderr, pfargs);                                                   \
} while (0)

/* memory alloc check */
#define SMGC_MEMCHK(_ptr_,_gt_)                                                \
do {                                                                           \
    if (NULL == (_ptr_)) {                                                     \
        SMGC_ERR_MSG("memory allocation error on %s\n", host_name_buff);       \
        goto _gt_;                                                             \
    }                                                                          \
} while (0)

/* mpi check */
#define SMGC_MPICHK(_ret_,_gt_)                                                \
do {                                                                           \
    if (MPI_SUCCESS != (_ret_)) {                                              \
        MPI_Error_string((_ret_), err_str, &err_str_len);                      \
        SMGC_ERR_MSG("mpi success not returned on %s... %s (errno: %d)\n",     \
                     host_name_buff, err_str, (_ret_));                        \
        goto _gt_;                                                             \
    }                                                                          \
} while (0)

/* test check */
#define SMGC_TSTCHK(_ret_,_gt_)                                                \
do {                                                                           \
    if (SMGC_SUCCESS != (_ret_)) {                                             \
        SMGC_ERR_MSG("test failure detected on %s ...\n", host_name_buff);     \
        goto _gt_;                                                             \
    }                                                                          \
} while (0)

/* master rank printf */
#define SMGC_MPF(pfargs...)                                                    \
do {                                                                           \
    if ((SMGC_MASTER_RANK) == my_rank) {                                       \
        fprintf(stdout, pfargs);                                               \
        fflush(stdout);                                                        \
    }                                                                          \
} while (0)

/* fprintf with flush */
#define SMGC_FPF(stream,pfargs...)                                             \
do {                                                                           \
    fprintf((stream), pfargs);                                                 \
    fflush((stream));                                                          \
} while (0)

/* ////////////////////////////////////////////////////////////////////////// */
/* globals                                                                    */
/* ////////////////////////////////////////////////////////////////////////// */
static int glob_loop_iter = 0;
static int glob_l_neighbor = 0;
static int glob_r_neighbor = 0;
/* no timeout by default */
static int msg_timeout = -1;

/* ////////////////////////////////////////////////////////////////////////// */
/* static forward declarations - typedefs - etc.                              */
/* ////////////////////////////////////////////////////////////////////////// */
/* test function pointer */
typedef int (*func_ptr)(void);

typedef struct smgc_test_t {
    /* test name */
    char *tname;
    /* test function pointer */
    func_ptr tfp;
} smgc_test_t;

typedef struct double_int_t {
    double val;
    int rank;
} double_int_t;

#if 0
static int
get_net_num(const char *target_hostname,
            unsigned long int *out_net_num);
#endif

static int
get_mult(char symbol, int *resp);

static int
create_test_list(const char *, smgc_test_t **);

static int
get_msg_size(const char *, const char *, int *);

static int
small_allreduce_max(void);

static int
hostname_exchange(void);

#if 0
static int
host_info_exchange(void);
#endif

static int
large_all_to_root_ptp(void);

static int
alt_sendrecv_ring(void);

static int
large_sendrecv_ring(void);

static int
small_all_to_all_ptp(void);

static int
large_all_to_all_ptp(void);

static int
stat_paths(void);

static int
root_bcast(void);

static int
rand_root_bcast(void);

static void
usage(void);

static char *
get_time_str(time_t *);

static char *
get_rhn(int rank);

static void
set_jb_params(void);

static int
get_num_tests(smgc_test_t *);

static void
upd_test_suite(smgc_test_t *);

static int
mpi_io(void);

static int
n_to_n_io(void);

static int
io_stats(double_int_t, char *, int);

static void
kill_mpi_messaging(int sig);

#if WITH_CELL_TESTS == 1
static int
cell_sanity(void);
#endif

static int
hello_world(void);

/* ////////////////////////////////////////////////////////////////////////// */
/* global variables                                                           */
/* ////////////////////////////////////////////////////////////////////////// */
static char *rhn_unknown = "UNKNOWN";
/* error string length                                                        */
static int err_str_len;
/* file size for both n-n and mpi_io                                          */
static size_t file_size = SMGC_MPI_IO_BUFF_SIZE;
/* error string buffer                                                        */
static char err_str[MPI_MAX_ERROR_STRING];
/* stat list                                                                  */
static char stat_list[SMGC_MAX_STAT_PATHS][SMGC_PATH_MAX];
/* filesystem test list                                                       */
static char fs_test_list[SMGC_MAX_FS_TEST_PATHS][SMGC_PATH_MAX];
/* bloat array                                                                */
static char bin_bloat[SMGC_BIN_SIZE] = {'x'};
/* host name buffer                                                           */
static char host_name_buff[SMGC_HOST_NAME_MAX];
/* remote hostname lookup table pointer                                       */
static char *rhname_lut_ptr = NULL;
/* start time string                                                          */
static char *start_time_str = NULL;
/* start time                                                                 */
static double start_time = 0.0;
/* start time struct                                                          */
static time_t start_clock;
/* end time                                                                   */
static double end_time;
/* my rank                                                                    */
static int my_rank = 0;
/* size of mpi_comm_world                                                     */
static int num_ranks = 0;
#if 0
/* my network number                                                          */
static unsigned long int my_net_num;
#endif
/* my "color"                                                                 */
int my_color = SMGC_COLOR_INVALID;
/* holds mpi return codes                                                     */
static int mpi_ret_code = MPI_ERR_OTHER;
/* number of paths to stat                                                    */
static int num_stat_paths = 0;
/* number of paths to run IO tests on                                         */
static int num_fs_test_paths = 0;
/* message size                                                               */
static int msg_size = SMGC_MSG_SIZE;
/* flag that dictates whether or not verbose output will be displayed         */
static bool be_verbose = false;
/* flag that dictates whether or not we are in quiet mode                     */
static bool be_quiet = false;
/* execution time                                                             */
static double exec_time = 0.0;
/* points to the selected test suite                                          */
static smgc_test_t *smgc_test_ptr = NULL;
/* number of tests that will be executed                                      */
static int num_tests = 0;
/* were the tests allocated on the heap?                                      */
static bool tests_on_heap = false;

/* ////////////////////////////////// */
/* o add new tests below              */
/* ////////////////////////////////// */

/* all tests */
static smgc_test_t smgc_all_tests[] =
{
    {"hostname_exchange"    , &hostname_exchange    },
    {"stat_paths"           , &stat_paths           },
    {"mpi_io"               , &mpi_io               },
    {"n_to_n_io"            , &n_to_n_io            },
#if WITH_CELL_TESTS == 1
    {"cell_sanity"          , &cell_sanity          },
#endif
    {"small_all_to_all_ptp" , &small_all_to_all_ptp },
    {"small_allreduce_max"  , &small_allreduce_max  },
    {"alt_sendrecv_ring"    , &alt_sendrecv_ring    },
    {"root_bcast"           , &root_bcast           },
    {"large_sendrecv_ring"  , &large_sendrecv_ring  },
    {"rand_root_bcast"      , &rand_root_bcast      },
    {"large_all_to_root_ptp", &large_all_to_root_ptp},
    {"large_all_to_all_ptp" , &large_all_to_all_ptp },
    {"hello_world"          , &hello_world          },
    {NULL                   , NULL                  } /* MUST BE LAST ELEMENT */
};

/* subset of tests - used for "smaller" jobs */
static smgc_test_t smgc_small_jb_tests[] =
{
    {"hostname_exchange"    , &hostname_exchange    },
    {"stat_paths"           , &stat_paths           },
    {"mpi_io"               , &mpi_io               },
#if WITH_CELL_TESTS == 1
    {"cell_sanity"          , &cell_sanity          },
#endif
    {"small_all_to_all_ptp" , &small_all_to_all_ptp },
    {"small_allreduce_max"  , &small_allreduce_max  },
    {"alt_sendrecv_ring"    , &alt_sendrecv_ring    },
    {"root_bcast"           , &root_bcast           },
    {"large_sendrecv_ring"  , &large_sendrecv_ring  },
    {"rand_root_bcast"      , &rand_root_bcast      },
    {"large_all_to_root_ptp", &large_all_to_root_ptp},
    {"large_all_to_all_ptp" , &large_all_to_all_ptp },
    {NULL                   , NULL                  } /* MUST BE LAST ELEMENT */
};

/* subset of tests - used for "large" jobs */
static smgc_test_t smgc_lrg_jb_tests[] =
{
    {"hostname_exchange"    , &hostname_exchange  },
    {"stat_paths"           , &stat_paths         },
    {"mpi_io"               , &mpi_io             },
#if WITH_CELL_TESTS == 1
    {"cell_sanity"          , &cell_sanity        },
#endif
    {"small_allreduce_max"  , &small_allreduce_max},
    {"alt_sendrecv_ring"    , &alt_sendrecv_ring  },
    {"root_bcast"           , &root_bcast         },
    {"rand_root_bcast"      , &rand_root_bcast    },
    {"large_sendrecv_ring"  , &large_sendrecv_ring},
    {NULL                   , NULL                } /* MUST BE LAST ELEMENT */
};

#endif /* SUPERMAGIC_H */
