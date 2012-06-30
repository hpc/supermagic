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

/* mpi message timeout in seconds */
#define MSG_TIMEOUT 15

/* messaging timeout macros */
#define TIMER_ENABLE(itimer)                                                   \
do {                                                                           \
    (itimer).it_value.tv_sec = MSG_TIMEOUT;                                    \
    (itimer).it_value.tv_usec = 0;                                             \
    (itimer).it_interval = (itimer).it_value;                                  \
    signal(SIGALRM, &kill_mpi_messaging);                                      \
    setitimer(ITIMER_REAL, &(itimer), NULL);                                   \
} while (0)

#define TIMER_DISABLE(itimer)                                                  \
do {                                                                           \
        (itimer).it_value.tv_sec = 0;                                          \
        (itimer).it_value.tv_usec = 0;                                         \
        (itimer).it_interval = (itimer).it_value;                              \
        setitimer(ITIMER_REAL, &(itimer), NULL);                               \
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

/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
/* private utility functions                                                  */
/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */

#if 0
/* ////////////////////////////////////////////////////////////////////////// */
static int
qsort_cmp_uli(const void *p1,
              const void *p2)
{
    return (*(unsigned long int *)p1 - *(unsigned long int *)p2);
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
get_my_global_color(bool used_cached, int *out_color)
{
    unsigned long int *net_nums = NULL;
    int mpi_rc = MPI_ERR_OTHER, rc = SMGC_ERROR;
    int i = 0, node_i = 0;
    unsigned long int prev_num;

    if (NULL == out_color) {
        return SMGC_ERROR;
    }

    /* do we have to figure this out - or can we use a cached value? */
    if (SMGC_COLOR_INVALID != my_color && used_cached) {
        *out_color = my_color;
        return SMGC_SUCCESS;
    }

    /* if we are here, then we have to do some work :-( */
    net_nums = (unsigned long int *)calloc(num_ranks,
                                           sizeof(unsigned long int));
    if (NULL == net_nums) {
        SMGC_ERR_MSG("out of resources\n");
        return SMGC_ERROR;
    }
    if (SMGC_SUCCESS != get_net_num(host_name_buff, &my_net_num)) {
        SMGC_ERR_MSG("get_net_num failure\n");
        goto out;
    }

    SMGC_MPF("       mpi_comm_world: exchanging network infomation\n"
             "       mpi_allgather buffer size: %lu B\n",
             (num_ranks * sizeof(unsigned long int)));

    mpi_rc = MPI_Allgather(&my_net_num, 1, MPI_UNSIGNED_LONG, net_nums, 1,
                           MPI_UNSIGNED_LONG, MPI_COMM_WORLD);
    SMGC_MPICHK(mpi_rc, out);

    qsort(net_nums, (size_t)num_ranks, sizeof(unsigned long int),
          qsort_cmp_uli);

    prev_num = net_nums[i++];

    while (i < num_ranks && prev_num != my_net_num) {
        while (net_nums[i] == prev_num) {
            ++i;
        }
        ++node_i;
        prev_num = net_nums[i];
    }

    *out_color = node_i;
    rc = SMGC_SUCCESS;

out:
    if (NULL != net_nums) free(net_nums);
    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
get_net_num(const char *target_hostname,
            unsigned long int *out_net_num)
{
    struct hostent *host = NULL;

    if (NULL == target_hostname || NULL == out_net_num) {
        return SMGC_ERROR;
    }

    if (NULL == (host = gethostbyname(target_hostname))) {
        SMGC_ERR_MSG("gethostbyname error\n");
        /* epic fail! */
        return SMGC_ERROR;
    }

    /* htonl used here for good measure - probably not needed */
    *out_net_num = (unsigned long int)
        htonl(inet_network(inet_ntoa(*(struct in_addr *)host->h_addr)));

    return SMGC_SUCCESS;
}
#endif

/* ////////////////////////////////////////////////////////////////////////// */
static int
get_mult(char symbol, int *resp)
{
    *resp = -42;
    switch (symbol) {
        case 'B':
            *resp = 1;
            break;
        case 'k':
            *resp = 1024;
            break;
        case 'M':
            *resp = 1048576;
            break;
        case 'G':
            *resp = 1073741824;
            break;
        default:
            SMGC_ERR_MSG("\'%c\' is not a supported message size option.\n",
                         symbol);
            /* fail */
            return SMGC_ERROR;
            /* never reached */
            break;
    }
    return SMGC_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
get_msg_size(const char *str, const char *label, int *real_msg_size)
{
    long unit_size = -1;
    /* default multiplier is 1 (B) */
    int mult = 1;
    char *end_ptr = NULL;

    errno = 0;
    unit_size = strtol(str, &end_ptr, 10);

    /* was there an error */
    if ((EINVAL == errno && 0 == unit_size) ||
        (ERANGE == errno && (LONG_MIN == unit_size || LONG_MAX == unit_size))) {
            int err = errno;
            SMGC_ERR_MSG("strtol error: %d (%s)\n", err, strerror(err));
            return SMGC_ERROR;
    }
    /* catch negative message sizes */
    else if (unit_size < 0) {
        SMGC_ERR_MSG("negative %s sizes are not supported.\n", label);
        return SMGC_ERROR;
    }
    /* all is well with the value returned by strtol */
    else {
        /* big hammer */
        uint64_t us = 0, m = 0, tmp = 0;
        /* a multiplier was provided by the user */
        if ('\0' != *end_ptr) {
            /* get the multiplier */
            if (SMGC_ERROR == get_mult(end_ptr[0], &mult)) {
                /* fail */
                return SMGC_ERROR;
            }
        }
        us = (uint64_t)unit_size;
        m = (uint64_t)mult;
        /* what is the real message size (B) */
        tmp = (us * m);
        if (tmp > INT_MAX || tmp < us || tmp < m) {
            SMGC_ERR_MSG("requested %s size is too large.\n", label);
            return SMGC_ERROR;
        }
        else {
            *real_msg_size = (int)tmp;
        }
        return SMGC_SUCCESS;
    }
    /* never reached */
    return SMGC_ERROR;
}

/* ////////////////////////////////////////////////////////////////////////// */
/**
 * returns number of tests within test suite pointed to by test_suite_ptr
 */
static int
get_num_tests(smgc_test_t *test_suite_ptr)
{
    int i;
    for (i = 0; i < SMGC_MAX_TESTS; ++i) {
        if (NULL == test_suite_ptr[i].tname || NULL == test_suite_ptr[i].tfp) {
            break;
        }
    }
    return i;
}

/* ////////////////////////////////////////////////////////////////////////// */
/**
 * prints all available tests
 */
static void
list_all_tests(void)
{
    int i, num_tests = get_num_tests(smgc_all_tests);

    SMGC_MPF("available tests:\n");
    for (i = 0; i < num_tests; ++i) {
        SMGC_MPF("    %s\n", smgc_all_tests[i].tname);
    }
    SMGC_MPF("\n");
}

/* ////////////////////////////////////////////////////////////////////////// */
/**
 * prints usage information
 */
static void
usage(void)
{
    SMGC_MPF("\n%s\n", SMGC_USAGE);
    list_all_tests();
    SMGC_MPF("%s\n", SMGC_EXAMPLE);
}

/* ////////////////////////////////////////////////////////////////////////// */
static bool
is_valid_test(char *test_name, int *test_index)
{
    int i;

    for (i = 0; NULL != smgc_all_tests[i].tname; ++i) {
        if (0 == strcmp(test_name, smgc_all_tests[i].tname)) {
            if (NULL != test_index) {
                *test_index = i;
            }
            return true;
        }
    }

    /* if test_name is not valid, *test_index is undefined */
    return false;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
create_test_list(const char *test_list_str,
                 smgc_test_t **smgc_custom_jb_test_ptr)
{
    char *test_string    = NULL;
    char *last           = NULL;
    char *tmp_list       = NULL;
    int num_tests        = 0;
    int test_index       = 0;
    smgc_test_t *tmp_ptr = *smgc_custom_jb_test_ptr;

    if (tests_on_heap) {
        if (NULL != smgc_test_ptr) {
            free(smgc_test_ptr);
            smgc_test_ptr = NULL;
        }
    }

    if (NULL == (tmp_list = strdup(test_list_str))) {
        SMGC_ERR_MSG("out of resources\n");
        return SMGC_ERROR;
    }
    else if (NULL == (tmp_ptr = (smgc_test_t *)malloc(sizeof(smgc_test_t) *
                                                      SMGC_MAX_TESTS))) {
        free(tmp_list);
        SMGC_ERR_MSG("out of resources\n");
        return SMGC_ERROR;
    }

    /* if we are here, then let the games begin */

    for (test_string = strtok_r(tmp_list, ",", &last);
         NULL != test_string && SMGC_MAX_TESTS >= num_tests;
         test_string = strtok_r(NULL, ",", &last)) {
        if (is_valid_test(test_string, &test_index)) {
            tmp_ptr[num_tests++] = smgc_all_tests[test_index];
        }
    }

    /* cap with NULLs */
    tmp_ptr[num_tests].tname = NULL;
    tmp_ptr[num_tests].tfp = NULL;

    /* update the test suite */
    upd_test_suite(tmp_ptr);
    tests_on_heap = true;
    return SMGC_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
static char *
get_time_str(time_t *raw_time)
{
    char tsb[SMGC_MAX_TIME_LEN];
    struct tm *bd_time_ptr = NULL;

    time(raw_time);
    bd_time_ptr = localtime(raw_time);

    strftime(tsb, SMGC_MAX_TIME_LEN - 1, SMGC_DATE_FORMAT, bd_time_ptr);
    /* caller is responsible for freeing returned resources */
    return strdup(tsb);
}

/* ////////////////////////////////////////////////////////////////////////// */
static char *
get_rhn(int rank)
{
    if (NULL != rhname_lut_ptr) {
        return &(rhname_lut_ptr[rank * SMGC_HOST_NAME_MAX]);
    }
    else {
        return rhn_unknown;
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
static void
upd_test_suite(smgc_test_t *new_test_suite_ptr)
{
    smgc_test_ptr = new_test_suite_ptr;
    num_tests = get_num_tests(smgc_test_ptr);
}

/* ////////////////////////////////////////////////////////////////////////// */
static void
set_jb_params()
{
    if (num_ranks >= SMGC_LRG_JB) {
        msg_size = SMGC_LRG_JB_MSG_SIZE;
        be_verbose = false;
        be_quiet = true;
        upd_test_suite(smgc_lrg_jb_tests);
    }
    else {
        upd_test_suite(smgc_small_jb_tests);
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
io_stats(double_int_t in_dint, char *label, int unit_type)
{
    char *unit = NULL;
    double val = in_dint.val;
    double sum = 0.0;
    double_int_t max = {0.0, 0}, min = {0.0, 0};
    int mpi_ret_code = MPI_ERR_OTHER;

    switch (unit_type) {
        case IO_STATS_TIME_S:
            unit = SMGC_TIME_S_UNIT_STR;
            break;
        case IO_STATS_MBS:
            unit = SMGC_MBS_UNIT_STR;
            break;
        default:
            SMGC_ERR_MSG("io_stats::unknow unit_type\n");
            goto err;
            /* never reached */
            break;
    }

    mpi_ret_code = MPI_Reduce(&in_dint, &max, 1, MPI_DOUBLE_INT, MPI_MAXLOC,
                              SMGC_MASTER_RANK, MPI_COMM_WORLD);
    SMGC_MPICHK(mpi_ret_code, err);

    mpi_ret_code = MPI_Reduce(&in_dint, &min, 1, MPI_DOUBLE_INT, MPI_MINLOC,
                              SMGC_MASTER_RANK, MPI_COMM_WORLD);
    SMGC_MPICHK(mpi_ret_code, err);

    mpi_ret_code = MPI_Reduce(&val, &sum, 1, MPI_DOUBLE, MPI_SUM,
                              SMGC_MASTER_RANK, MPI_COMM_WORLD);
    SMGC_MPICHK(mpi_ret_code, err);

    SMGC_MPF("   --- %s:\n", label);
    SMGC_MPF("          max rank: %06d (%s)\n", max.rank, get_rhn(max.rank));
    SMGC_MPF("          max %s: %.3f %s\n", label, max.val, unit);
    SMGC_MPF("          min rank: %06d (%s)\n", min.rank, get_rhn(min.rank));
    SMGC_MPF("          min %s: %.3f %s\n", label, min.val, unit);
    SMGC_MPF("          ave %s: %.3f %s\n", label, sum / num_ranks, unit);
    SMGC_MPF("          aggregate %s: %.3f %s\n", label, sum, unit);

    return SMGC_SUCCESS;
err:
    return SMGC_ERROR;
}

/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
/* test functions                                                             */
/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */

/* ////////////////////////////////////////////////////////////////////////// */
/* test by request only */
static int
n_to_n_io(void)
{
    int i = 0, j = 0, fd = -1, rc = SMGC_ERROR, mpi_rc = MPI_ERR_OTHER;
    /* what we are going to write and what we should read, buff rest char */
    char wr_char = 'j', clobber_char = 'x';
    ssize_t bytes_written = -1, bytes_read = -1;
    /* the size of the file that i'll be writing (in B) */
    size_t buff_size = file_size;
    char *my_file_name = NULL;
    /* points to buffer used for both reading and writing */
    char *buff = NULL;
    /* variables for recording time */
    double open_time = 0.0, close_time = 0.0, write_start = 0.0,
           write_fin = 0.0;
    double read_start = 0.0, read_fin = 0.0, lseek_start = 0.0, lseek_fin = 0.0,
           effe_bw_time_fix = 0.0, memset_start = 0.0, memset_fin = 0.0,
           tmp_dbl = 0.0;
    /* bandwidth variables */
    double effe_bw = 0.0, read_bw = 0.0, write_bw = 0.0;
    /* for reduce operations that find max and min rank */
    double_int_t in_effe = {0.0, 0};
    double_int_t in_wr = {0.0, 0};
    double_int_t in_rd = {0.0, 0};

    /* no work to do, return success and move on */
    if (0 == num_fs_test_paths) {
        SMGC_MPF("       zero paths requested via -w option. skipping test.\n");
        return SMGC_SUCCESS;
    }

    /* if we are here, let the games begin! */

    if (NULL == (buff = (char *)malloc(buff_size * sizeof(char)))) {
        SMGC_ERR_MSG("out of resources\n");
        return SMGC_ERROR;
    }
    memset(buff, wr_char, buff_size);

    SMGC_MPF("       file size (per rank process): %lu B\n", buff_size);

    /* write to all requested paths */
    for (i = 0; i < num_fs_test_paths; ++i) {
        asprintf(&my_file_name, "%s/%s_%d", fs_test_list[i],
                 SMGC_MPI_FILE_NAME, my_rank);
        if (NULL == my_file_name) {
            SMGC_ERR_MSG("out of resources\n");
            goto out;
        }

        /* let the user know we are working on it */
        SMGC_MPF("   === mpi_comm_world: writing to %s\n", fs_test_list[i]);

        /* barrier before we start */
        mpi_rc = MPI_Barrier(MPI_COMM_WORLD);
        SMGC_MPICHK(mpi_rc, out);

        open_time = MPI_Wtime();
        if (-1 == (fd = open(my_file_name, O_CREAT | O_RDWR, 0600))) {
            int error = errno;
            SMGC_ERR_MSG("open failed with errno: %d (%s)\n", error,
                         strerror(error));
            goto out;
        }

        write_start = MPI_Wtime();
        if (-1 == (bytes_written = write(fd, buff, buff_size))) {
            int error = errno;
            SMGC_ERR_MSG("write failed with errno: %d (%s)\n", error,
                         strerror(error));
            goto out;
        }
        write_fin = MPI_Wtime();

        /* seek to beginning of file - time this step so we can subtract the
         * time spent here from our bw calculations
         */
        lseek_start = MPI_Wtime();
        if(-1 == lseek(fd, 0, SEEK_SET)) {
            int error = errno;
            SMGC_ERR_MSG("lseek failed with errno: %d (%s)\n", error,
                         strerror(error));
            goto out;
        }
        lseek_fin = MPI_Wtime();

        memset_start = MPI_Wtime();
        /* overwrite buff's contents before read */
        memset(buff, clobber_char, buff_size);
        memset_fin = MPI_Wtime();

        read_start = MPI_Wtime();
        if (-1 == (bytes_read = read(fd, buff, buff_size))) {
            int error = errno;
            SMGC_ERR_MSG("read failed with errno: %d (%s)\n", error,
                         strerror(error));
            goto out;
        }
        read_fin = MPI_Wtime();

        if (0 != close(fd)) {
            int error = errno;
            SMGC_ERR_MSG("close failed with errno: %d (%s)\n", error,
                         strerror(error));
            goto out;
        }
        close_time = MPI_Wtime();

        fd = -1;

        if (0 != unlink(my_file_name)) {
            int error = errno;
            SMGC_ERR_MSG("unlink failed with errno: %d (%s)\n", error,
                         strerror(error));
            goto out;
        }
        /* check integrity of read */
        if (bytes_written != bytes_read) {
            SMGC_ERR_MSG("write/read mismatch.  wrote %lu read %lu\n",
                         bytes_written, bytes_read);
            goto out;
        }
        /* iterate over char buff - making certain all is well */
        for (j = 0; j < bytes_written; ++j) {
            if (wr_char != buff[j]) {
                SMGC_ERR_MSG(
                    "characters read do not match characters written!\n"
                );
                goto out;
            }
        }

        /* subtract time not spent in benchmarked routines */
        effe_bw_time_fix = (lseek_fin - lseek_start) +
                           (memset_fin - memset_start);

        /* calculate bandwidths */

        /* negative and 0 length file size protection provided by lower level */
        /* zero value fixup - good enough for our purposes */

        /* effective bandwidth */
        if (0.0 >= (tmp_dbl = ((close_time - open_time) - effe_bw_time_fix))) {
            effe_bw = 0.0;
        }
        else {
            effe_bw = ((double)buff_size / tmp_dbl / (double)SMGC_MB_SIZE);
        }
        /* write bandwidth */
        if (0.0 >= (tmp_dbl = (write_fin - write_start))) {
            write_bw = 0.0;
        }
        else {
            write_bw = ((double)buff_size / tmp_dbl / (double)SMGC_MB_SIZE);
        }
        /* read bandwidth */
        if (0.0 >= (tmp_dbl = (read_fin - read_start))) {
            read_bw = 0.0;
        }
        else {
            read_bw = ((double)buff_size / tmp_dbl / (double)SMGC_MB_SIZE);
        }

        /* prepare values for reduce */
        in_effe.val = effe_bw;
        in_effe.rank = my_rank;
        in_wr.val = write_bw;
        in_wr.rank = my_rank;
        in_rd.val = read_bw;
        in_rd.rank = my_rank;

        /* calculate effective bandwidth stats */
        if (SMGC_SUCCESS != (rc = io_stats(in_effe, "effective write bandwidth",
                                           IO_STATS_MBS))) {
            goto out;
        }
        /* calculate write bandwidth stats */
        if (SMGC_SUCCESS != (rc = io_stats(in_wr, "pure write bandwidth",
                                           IO_STATS_MBS))) {
            goto out;
        }
        /* calculate read bandwidth stats */
        if (SMGC_SUCCESS != (rc = io_stats(in_rd, "pure read bandwidth",
                                           IO_STATS_MBS))) {
            goto out;
        }

        /* all is well for this iteration */
        if (NULL != my_file_name) {
            free(my_file_name);
            my_file_name = NULL;
        }
    }

    /* all is well */
    rc = SMGC_SUCCESS;
out:
    if (-1 != fd) {
        close(fd);
        unlink(my_file_name);
    }
    if (NULL != buff) free(buff);
    if (NULL != my_file_name) free(my_file_name);
    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
hello_world(void)
{
    int res_len = 0;
    char name_buff[MPI_MAX_PROCESSOR_NAME + 1];

    mpi_ret_code = MPI_Get_processor_name(name_buff, &res_len);
    SMGC_MPICHK(mpi_ret_code, err);

    SMGC_FPF(stdout, "       hello from rank %06d (%s) of %06d\n", my_rank,
             name_buff, num_ranks);

    return SMGC_SUCCESS;
err:
    return SMGC_ERROR;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
stat_paths(void)
{
    int i = 0;
    struct stat buff;
    /* nothing to do, so that's easy */
    if (0 == num_stat_paths) {
        SMGC_MPF("       zero paths requested via -s option. skipping test.\n");
        return SMGC_SUCCESS;
    }

    for (i = 0; i < num_stat_paths; ++i) {
        SMGC_MPF("       mpi_comm_world: stating %s\n", stat_list[i]);
        /* try to stat the file */
        if (0 != stat(stat_list[i], &buff)) {
            SMGC_FPF(stderr, "   !!! rank %d (%s) unable to stat %s\n", my_rank,
                     host_name_buff, stat_list[i]);
            return SMGC_ERROR;
        }
    }
    return SMGC_SUCCESS;
}

/* ////////////////////////////////////////////////////////////////////////// */
static void
kill_mpi_messaging(int sig)
{
    /* no check in error path  - hope for the best */
    gethostname(host_name_buff, SMGC_HOST_NAME_MAX - 1);
    host_name_buff[SMGC_HOST_NAME_MAX - 1] = '\0';

    fprintf(stderr, "\n########## HANG DETECTED "
            "[on loop iteration: %d] %d (%s) ==> %d (%s) ==> %d (%s) "
            "##########\n",
            glob_loop_iter, glob_l_neighbor, get_rhn(glob_l_neighbor), my_rank,
            host_name_buff, glob_r_neighbor, get_rhn(glob_r_neighbor));

    exit(EXIT_FAILURE);
}

/* ////////////////////////////////////////////////////////////////////////// */
#if WITH_CELL_TESTS == 1

/* in seconds */
#define CELL_TEST_TIMEOUT 480

/* ////////////////////////////////////////////////////////////////////////// */
static void
kill_cell_check(void)
{
    /* no check in error path  - hope for the best */
    gethostname(host_name_buff, SMGC_HOST_NAME_MAX - 1);
    host_name_buff[SMGC_HOST_NAME_MAX - 1] = '\0';

    SMGC_ERR_MSG(
        "rank %d (%s) unable to execute cell_sanity test within %d s.\n",
         my_rank, host_name_buff, CELL_TEST_TIMEOUT);

    exit(EXIT_FAILURE);
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
cell_sanity(void)
{
    unsigned long rc = 0;
    struct itimerval itimer;

    itimer.it_value.tv_sec = CELL_TEST_TIMEOUT;
    itimer.it_value.tv_usec = 0;
    itimer.it_interval = itimer.it_value;

    SMGC_MPF("       mpi_comm_world: running cell diagnostics\n");

    signal(SIGALRM, (void(*)(int))&kill_cell_check);
    setitimer(ITIMER_REAL, &itimer, NULL);

    if (0 != (rc = cell_check(CELL_CHECK_MSG_FAIL, CELL_CHECK_TEST_ALL))) {
        SMGC_ERR_MSG("cell_check failure - return code: %lu\n", rc);
        goto err;
    }
    else {
        /* all is well... disable the alarm */
        itimer.it_value.tv_sec = 0;
        itimer.it_value.tv_usec = 0;
        itimer.it_interval = itimer.it_value;
        setitimer(ITIMER_REAL, &itimer, NULL);
    }

    return SMGC_SUCCESS;
err:
    return SMGC_ERROR;
}
#endif /* WITH_CELL_TESTS */

/* ////////////////////////////////////////////////////////////////////////// */
static int
mpi_io(void)
{
    int mpi_ret_code = MPI_ERR_OTHER, num_elems = 0, i = 0, rc = SMGC_ERROR;
    /* access mode flags */
    int amode = MPI_MODE_RDWR | MPI_MODE_CREATE | MPI_MODE_DELETE_ON_CLOSE;
    char *buff = NULL, path_buff[SMGC_PATH_MAX];
    /* i/o time markers */
    double effe_start = 0.0, effe_fin = 0.0;
    double write_start = 0.0, write_fin = 0.0, read_start = 0.0, read_fin = 0.0;
    /* bandwidth variables */
    double effe_bw = 0.0, write_bw = 0.0, read_bw = 0.0, effe_bw_time_fix = 0.0,
           gete_start = 0.0, gete_fin = 0.0, gete_start2 = 0.0, gete_fin2 = 0.0;
    double tmp_dbl = 0.0;
    /* structs for min/max */
    double_int_t in_wr = {0.0, 0}, in_rd = {0.0, 0}, in_effe = {0.0, 0};
    /* file handle */
    MPI_File mpi_fh;
    MPI_Status status;
    MPI_Offset offset = (MPI_Offset)(my_rank * file_size);

    /* no writing to do, so return SMGC_SUCCESS */
    if (0 == num_fs_test_paths) {
        SMGC_MPF("       zero paths requested via -w option. skipping test.\n");
        return SMGC_SUCCESS;
    }

    buff = (char *)malloc(file_size * sizeof(char));
    SMGC_MEMCHK(buff, out);

    memset(buff, 'j', (size_t)(file_size * sizeof(char)));

    /* if we are here, then let the real work begin */

    SMGC_MPF("       file size (per rank process): %lu B\n", file_size);

    for (i = 0; i < num_fs_test_paths; ++i) {
        snprintf(path_buff, SMGC_PATH_MAX, "%s/%s", fs_test_list[i],
                 SMGC_MPI_FILE_NAME);

        SMGC_MPF("   === mpi_comm_world: writing to %s\n", fs_test_list[i]);

        /* barrier before we start each iteration */
        mpi_ret_code = MPI_Barrier(MPI_COMM_WORLD);
        SMGC_MPICHK(mpi_ret_code, out);

        effe_start = MPI_Wtime();
        mpi_ret_code = MPI_File_open(MPI_COMM_WORLD, path_buff, amode,
                                     MPI_INFO_NULL, &mpi_fh);
        SMGC_MPICHK(mpi_ret_code, out);

        mpi_ret_code = MPI_File_set_view(mpi_fh, (MPI_Offset)0, MPI_CHAR,
                                         MPI_CHAR, "native", MPI_INFO_NULL);
        SMGC_MPICHK(mpi_ret_code, out);

        write_start = MPI_Wtime();
        mpi_ret_code = MPI_File_write_at(mpi_fh, offset, buff,
                                         file_size, MPI_CHAR,
                                         &status);
        SMGC_MPICHK(mpi_ret_code, out);
        write_fin = MPI_Wtime();

        gete_start = MPI_Wtime();
        mpi_ret_code = MPI_Get_elements(&status, MPI_CHAR, &num_elems);
        SMGC_MPICHK(mpi_ret_code, out);
        gete_fin = MPI_Wtime();

        if ((sizeof(char) * file_size) != num_elems) {
            SMGC_ERR_MSG("write size mismatch.  wrote %lu requested %d\n",
                         sizeof(char) * file_size,
                         num_elems);
            goto out;
        }

        read_start = MPI_Wtime();
        mpi_ret_code = MPI_File_read_at(mpi_fh, offset, buff, file_size,
                                        MPI_CHAR, &status);
        SMGC_MPICHK(mpi_ret_code, out);
        read_fin = MPI_Wtime();

        gete_start2 = MPI_Wtime();
        mpi_ret_code = MPI_Get_elements(&status, MPI_CHAR, &num_elems);
        SMGC_MPICHK(mpi_ret_code, out);
        gete_fin2 = MPI_Wtime();

        mpi_ret_code = MPI_File_close(&mpi_fh);
        SMGC_MPICHK(mpi_ret_code, out);
        effe_fin = MPI_Wtime();

        if ((sizeof(char) * file_size) != num_elems) {
            SMGC_ERR_MSG("write/read mismatch.  wrote %lu read %d\n",
                         sizeof(char) * file_size,
                         num_elems);
            goto out;
        }

        /* subtract time not spent in benchmarked routines */
        effe_bw_time_fix = (gete_fin - gete_start) +
                           (gete_fin2 - gete_start2);

        /* calculate bandwidths */

        /* negative and 0 length file size protection provided by lower level */
        /* zero value fixup - good enough for our purposes */

        /* effective bandwidth */
        if (0.0 >= (tmp_dbl = ((effe_fin - effe_start) - effe_bw_time_fix))) {
            effe_bw = 0.0;
        }
        else {
            effe_bw = ((double)file_size / tmp_dbl / (double)SMGC_MB_SIZE);
        }
        /* write bandwidth */
        if (0.0 >= (tmp_dbl = (write_fin - write_start))) {
            write_bw = 0.0;
        }
        else {
            write_bw = ((double)file_size / tmp_dbl / (double)SMGC_MB_SIZE);
        }
        /* read bandwidth */
        if (0.0 >= (tmp_dbl = (read_fin - read_start))) {
            read_bw = 0.0;
        }
        else {
            read_bw = ((double)file_size / tmp_dbl / (double)SMGC_MB_SIZE);
        }

        /* fill structs for calculating min/max */
        in_effe.val = effe_bw;
        in_effe.rank = my_rank;
        in_wr.val = write_bw;
        in_wr.rank = my_rank;
        in_rd.val = read_bw;
        in_rd.rank = my_rank;

        /* calculate effective bandwidth stats */
        if (SMGC_SUCCESS != io_stats(in_effe, "effective write bandwidth",
                                     IO_STATS_MBS)) {
            goto out;
        }
        /* calculate write bandwidth stats */
        if (SMGC_SUCCESS != io_stats(in_wr, "pure write bandwidth",
                                     IO_STATS_MBS)) {
            goto out;
        }
        /* calculate read bandwidth stats */
        if (SMGC_SUCCESS != io_stats(in_rd, "pure read bandwidth",
                                     IO_STATS_MBS)) {
            goto out;
        }
    }

    /* all is well, set rc accordingly */
    rc = SMGC_SUCCESS;
out:
    if (NULL != buff) free(buff);
    return rc;
}

#if 0
/* ////////////////////////////////////////////////////////////////////////// */
static int
host_info_exchange(void)
{
    unsigned long int *net_nums = (unsigned long int *)
                                  calloc(num_ranks, sizeof(unsigned long int));
    int rc = SMGC_ERROR;

    if (NULL == net_nums) {
        SMGC_ERR_MSG("out of resources\n");
        return SMGC_ERROR;
    }
    if (SMGC_SUCCESS != get_my_global_color(true, &my_color)) {
        SMGC_ERR_MSG("get_my_global_color failure\n");
        goto out;
    }

out:
    if (NULL != net_nums) free(net_nums);
    return rc;
}
#endif

/* ////////////////////////////////////////////////////////////////////////// */
static int
hostname_exchange(void)
{
    int buff_size = num_ranks * SMGC_HOST_NAME_MAX * sizeof(char);
    if (NULL == rhname_lut_ptr) {
        rhname_lut_ptr = (char *)calloc(buff_size, sizeof(char));
        SMGC_MEMCHK(rhname_lut_ptr, error);
    }

    memset(rhname_lut_ptr, '\0', buff_size);

    SMGC_MPF("       mpi_comm_world: mpi_allgather buffer size: %d B\n",
             buff_size);
    SMGC_MPF("       mpi_comm_world: exchanging host name information\n");

    /* populate the remote host name lookup table */
    mpi_ret_code = MPI_Allgather(host_name_buff, SMGC_HOST_NAME_MAX, MPI_CHAR,
                                 rhname_lut_ptr, SMGC_HOST_NAME_MAX, MPI_CHAR,
                                 MPI_COMM_WORLD);
    SMGC_MPICHK(mpi_ret_code, error);

    return SMGC_SUCCESS;
error:
    /* only free rhname_lut_ptr in error condition */
    if (NULL != rhname_lut_ptr) {
        free(rhname_lut_ptr);
        rhname_lut_ptr = NULL;
    }
    return SMGC_ERROR;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
small_allreduce_max(void)
{
    double send_buff = (double)my_rank, recv_buff = 0.0;

    SMGC_MPF("       message size: %d B\n", (int)sizeof(double));
    SMGC_MPF("       mpi_comm_world: mpi_allreducing\n");

    mpi_ret_code = MPI_Allreduce(&send_buff, &recv_buff, 1, MPI_DOUBLE,
                                 MPI_MAX, MPI_COMM_WORLD);
    SMGC_MPICHK(mpi_ret_code, error);

    SMGC_MPF("       mpi_comm_world: verifying result\n");

    /* yes, i do want to do it this way :-) */
    if (recv_buff != (double)(num_ranks - 1)) {
        SMGC_ERR_MSG("invalid result detected\n");
        goto error;
    }

    return SMGC_SUCCESS;
error:
    return SMGC_ERROR;
}

/* ////////////////////////////////////////////////////////////////////////// */
/**
 * potentially a very synchronous all to root point-to-point implementation.
 */
static int
large_all_to_root_ptp(void)
{
    int buff_size   = msg_size;
    int src_rank    = 0, tag = 0;
    int rc          = SMGC_ERROR;
    char *char_buff = NULL;
    char *del       = "\b\b\b\b\b\b\b\b\b\b\b\b\b";
    MPI_Status status;

    if (NULL == (char_buff = (char *)calloc(buff_size, sizeof(char)))) {
        SMGC_ERR_MSG("out of resources\n");
        return SMGC_ERROR;
    }

    SMGC_MPF("       message size: %d B\n", buff_size);

    /* if we are here, let the games begin */

    if (SMGC_MASTER_RANK != my_rank) {
        mpi_ret_code = MPI_Send(char_buff, buff_size, MPI_CHAR,
                                SMGC_MASTER_RANK, tag, MPI_COMM_WORLD);
        SMGC_MPICHK(mpi_ret_code, out);
    }
    /* i am the root */
    else {
        SMGC_MPF("       mpi_comm_world: mpi_sending to rank %d - ",
                 SMGC_MASTER_RANK);
        for (src_rank = 0; src_rank < num_ranks; ++src_rank) {
            if (my_rank != src_rank) {
                SMGC_MPF("%s%06d/%06d%s", 1 == src_rank ? "" : del, src_rank,
                         (num_ranks - 1),
                         (num_ranks  -1) == src_rank ? "\n" : "");

                mpi_ret_code = MPI_Recv(char_buff, buff_size, MPI_CHAR,
                                        src_rank, tag, MPI_COMM_WORLD, &status);
                SMGC_MPICHK(mpi_ret_code, out);
            }
        }
    }
    /* we made it - rainbows and butterflies */
    rc = SMGC_SUCCESS;
out:
    if (NULL != char_buff) {
        free(char_buff);
    }
    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
root_bcast(void)
{
    int buff_size = msg_size, rc = SMGC_ERROR;
    char *char_buff = NULL;

    if (NULL == (char_buff = (char *)calloc(buff_size, sizeof(char)))) {
        SMGC_ERR_MSG("out of resources\n");
        return SMGC_ERROR;
    }

    SMGC_MPF("       message size: %d B\n", buff_size);
    SMGC_MPF("       rank %06d (%s): broadcasting to mpi_comm_world\n", my_rank,
             host_name_buff);

    mpi_ret_code = MPI_Bcast(char_buff, buff_size, MPI_CHAR, SMGC_MASTER_RANK,
                             MPI_COMM_WORLD);
    SMGC_MPICHK(mpi_ret_code, out);

    /* if we are here, then all is well - note that fact */
    rc = SMGC_SUCCESS;
out:
    if (NULL != char_buff) {
        free(char_buff);
        char_buff = NULL;
    }
    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
rand_root_bcast(void)
{
    int i = 0, buff_size = msg_size, num_itrs = 8, next_bc_root = 0,
        rc = SMGC_ERROR;
    char *char_buff = NULL;

    if (NULL == (char_buff = (char *)calloc(buff_size, sizeof(char)))) {
        SMGC_ERR_MSG("out of resources\n");
        return SMGC_ERROR;
    }

    srand((int)time(NULL));

    SMGC_MPF("       message size: %d B\n", msg_size);

    for (i = 0; i < num_itrs; ++i) {
        /* let the master rank figure out the next "random" root */
        if (SMGC_MASTER_RANK == my_rank) {
            next_bc_root = rand() % num_ranks;
            SMGC_FPF(stdout,
                     "       %06d (%s): broadcasting to mpi_comm_world\n",
                     next_bc_root, get_rhn(next_bc_root));
        }
        /* let mpi_comm_world know about the next bcast root */
        mpi_ret_code = MPI_Bcast(&next_bc_root, 1, MPI_INT, SMGC_MASTER_RANK,
                                 MPI_COMM_WORLD);
        SMGC_MPICHK(mpi_ret_code, out);

        /* root broadcast! */
        mpi_ret_code = MPI_Bcast(char_buff, buff_size, MPI_CHAR, next_bc_root,
                                 MPI_COMM_WORLD);
        SMGC_MPICHK(mpi_ret_code, out);
    }
    /* success! */
    rc = SMGC_SUCCESS;
out:
    if (NULL != char_buff) {
        free(char_buff);
        char_buff = NULL;
    }
    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
large_sendrecv_ring(void)
{
    int i = 0, num_iters = 4, send_tag  = 42, recv_tag = 42,
        buff_size = msg_size, r_neighbor = 0, l_neighbor = 0;
    char *send_char_buff = NULL, *recv_char_buff = NULL;
    MPI_Status status;

    r_neighbor = (my_rank + 1) % num_ranks;
    l_neighbor = my_rank - 1;

    if (l_neighbor < 0) {
        l_neighbor = num_ranks - 1;
    }

    SMGC_MPF("       message size: %d B\n", buff_size);

    for (i = 0; i < num_iters; ++i) {
        send_char_buff = (char *)calloc(buff_size, sizeof(char));
        SMGC_MEMCHK(send_char_buff, error);
        recv_char_buff = (char *)calloc(buff_size, sizeof(char));
        SMGC_MEMCHK(recv_char_buff, error);

        SMGC_MPF("       =====================================>\n");

        mpi_ret_code = MPI_Sendrecv(send_char_buff, buff_size,
                                    MPI_CHAR, r_neighbor, send_tag,
                                    recv_char_buff, buff_size, MPI_CHAR,
                                    l_neighbor, recv_tag, MPI_COMM_WORLD,
                                    &status);
        SMGC_MPICHK(mpi_ret_code, error);

        SMGC_MPF("       <=====================================\n");

        mpi_ret_code = MPI_Sendrecv(send_char_buff, buff_size,
                                    MPI_CHAR, l_neighbor, send_tag,
                                    recv_char_buff, buff_size, MPI_CHAR,
                                    r_neighbor, recv_tag, MPI_COMM_WORLD,
                                    &status);
        SMGC_MPICHK(mpi_ret_code, error);

        free(send_char_buff);
        send_char_buff = NULL;
        free(recv_char_buff);
        recv_char_buff = NULL;
    }

    return SMGC_SUCCESS;
error:
    if (NULL != send_char_buff) {
        free(send_char_buff);
    }
    if (NULL != recv_char_buff) {
        free(recv_char_buff);
    }
    return SMGC_ERROR;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
alt_sendrecv_ring(void)
{
    int i = 0, num_iters = 4, send_tag  = 42, recv_tag = 42, buff_size = 0,
        large_buff_size = msg_size, small_buff_size = 1, r_neighbor = 0,
        l_neighbor = 0;
    char *send_char_buff = NULL, *recv_char_buff = NULL;
    char *large_msg_size_str = "=====================================";
    char *small_msg_size_str = "-------------------------------------";
    char *cur_size_str_ptr = large_msg_size_str;
    MPI_Status status;

    r_neighbor = (my_rank + 1) % num_ranks;
    l_neighbor = my_rank - 1;

    if (l_neighbor < 0) {
        l_neighbor = num_ranks - 1;
    }

    SMGC_MPF("       message size key: === %d B, --- %d B\n", large_buff_size,
             small_buff_size);

    for (i = 0; i < num_iters; ++i) {
        /* what size buffs are we using this time around? */
        if (0 != i % 2) {
            buff_size = large_buff_size;
            cur_size_str_ptr = large_msg_size_str;
        }
        else {
            buff_size = small_buff_size;
            cur_size_str_ptr = small_msg_size_str;
        }

        send_char_buff = (char *)calloc(buff_size, sizeof(char));
        SMGC_MEMCHK(send_char_buff, error);
        recv_char_buff = (char *)calloc(buff_size, sizeof(char));
        SMGC_MEMCHK(recv_char_buff, error);

        SMGC_MPF("       %s>\n", cur_size_str_ptr);

        mpi_ret_code = MPI_Sendrecv(send_char_buff, buff_size, MPI_CHAR,
                                    r_neighbor, send_tag, recv_char_buff,
                                    buff_size, MPI_CHAR, l_neighbor, recv_tag,
                                    MPI_COMM_WORLD, &status);
        SMGC_MPICHK(mpi_ret_code, error);

        SMGC_MPF("       <%s\n", cur_size_str_ptr);

        mpi_ret_code = MPI_Sendrecv(send_char_buff, buff_size, MPI_CHAR,
                                    l_neighbor, send_tag, recv_char_buff,
                                    buff_size, MPI_CHAR, r_neighbor, recv_tag,
                                    MPI_COMM_WORLD, &status);
        SMGC_MPICHK(mpi_ret_code, error);

        free(send_char_buff);
        send_char_buff = NULL;
        free(recv_char_buff);
        recv_char_buff = NULL;
    }

    return SMGC_SUCCESS;
error:
    if (NULL != send_char_buff) {
        free(send_char_buff);
    }
    if (NULL != recv_char_buff) {
        free(recv_char_buff);
    }
    return SMGC_ERROR;
}

/* ////////////////////////////////////////////////////////////////////////// */
static void
reset_globs(void)
{
    glob_loop_iter = 0;
    glob_l_neighbor = 0;
    glob_r_neighbor = 0;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
small_all_to_all_ptp(void)
{
    int i                = 0;
    int j                = 0;
    int rc               = SMGC_ERROR;
    int l_neighbor       = 0;
    int r_neighbor       = 0;
    int buff_size        = 1;
    char *send_char_buff = NULL;
    char *recv_char_buff = NULL;
    char *del            = "\b\b\b\b\b\b\b\b\b\b\b\b\b";
    struct itimerval itimer;
    MPI_Status status;

    reset_globs();

    send_char_buff = (char *)calloc(buff_size, sizeof(char));
    SMGC_MEMCHK(send_char_buff, out);
    recv_char_buff = (char *)calloc(buff_size, sizeof(char));
    SMGC_MEMCHK(recv_char_buff, out);

    SMGC_MPF("       message size: %d B\n", buff_size);
    SMGC_MPF("       mpi_comm_world: all to all - ");

    for (i = 1; i <= num_ranks; ++i, glob_loop_iter = i) {
        SMGC_MPF("%s%06d/%06d%s", 1 == i ? "" : del, i, num_ranks,
                 num_ranks == i ? "\n" : "");

        r_neighbor = (my_rank + i) % num_ranks;
        l_neighbor = my_rank;

        for (j = 0; j < i; ++j) {
            --l_neighbor;
            if (l_neighbor < 0) {
                l_neighbor = num_ranks - 1;
            }
        }

        glob_l_neighbor = l_neighbor;
        glob_r_neighbor = r_neighbor;

        TIMER_ENABLE(itimer);
        mpi_ret_code = MPI_Sendrecv(send_char_buff, buff_size, MPI_CHAR,
                                    r_neighbor, i, recv_char_buff, buff_size,
                                    MPI_CHAR, l_neighbor, i, MPI_COMM_WORLD,
                                    &status);
        SMGC_MPICHK(mpi_ret_code, out);
        TIMER_DISABLE(itimer);
    }

    /* all is well */
    rc = SMGC_SUCCESS;

out:
    if (NULL != send_char_buff) free(send_char_buff);
    if (NULL != recv_char_buff) free(recv_char_buff);
    return rc;
}

/* ////////////////////////////////////////////////////////////////////////// */
static int
large_all_to_all_ptp(void)
{
    int i                = 0;
    int j                = 0;
    int rc               = SMGC_ERROR;
    int l_neighbor       = 0;
    int r_neighbor       = 0;
    int buff_size        = msg_size;
    char *send_char_buff = NULL;
    char *recv_char_buff = NULL;
    char *del            = "\b\b\b\b\b\b\b\b\b\b\b\b\b";
    MPI_Status status;

    send_char_buff = (char *)calloc(buff_size, sizeof(char));
    SMGC_MEMCHK(send_char_buff, out);
    recv_char_buff = (char *)calloc(buff_size, sizeof(char));
    SMGC_MEMCHK(recv_char_buff, out);

    SMGC_MPF("       message size: %d B\n", buff_size);
    SMGC_MPF("       mpi_comm_world: all to all - ");

    for (i = 1; i <= num_ranks; ++i) {
        SMGC_MPF("%s%06d/%06d%s", 1 == i ? "" : del, i, num_ranks,
                 num_ranks == i ? "\n" : "");

        r_neighbor = (my_rank + i) % num_ranks;
        l_neighbor = my_rank;

        for (j = 0; j < i; ++j) {
            --l_neighbor;
            if (l_neighbor < 0) {
                l_neighbor = num_ranks - 1;
            }
        }

        mpi_ret_code = MPI_Sendrecv(send_char_buff, buff_size, MPI_CHAR,
                                    r_neighbor, i, recv_char_buff, buff_size,
                                    MPI_CHAR, l_neighbor, i, MPI_COMM_WORLD,
                                    &status);
        SMGC_MPICHK(mpi_ret_code, out);
    }

    /* all is well */
    rc = SMGC_SUCCESS;

out:
    if (NULL != send_char_buff) free(send_char_buff);
    if (NULL != recv_char_buff) free(recv_char_buff);
    return rc;
}


/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
/* main                                                                       */
/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
int
main(int argc,
     char **argv)
{
    int i = 0, ii = 0, c = 0, opt_indx = 0, num_iters = SMGC_DEF_NUM_ITRS;
    smgc_test_t *smgc_custom_jb_test_ptr = NULL;

    /* can't hurt */
    memset(bin_bloat, 'x', sizeof(char) * SMGC_BIN_SIZE);
    /* do this here because an error may occur before gethostname.
     * we use the contents of host_name_buff in error messages.
     */
    snprintf(host_name_buff, SMGC_HOST_NAME_MAX - 1, "%s", rhn_unknown);

    /* init MPI */
    mpi_ret_code = MPI_Init(&argc, &argv);
    SMGC_MPICHK(mpi_ret_code, error);
    mpi_ret_code = MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);
    SMGC_MPICHK(mpi_ret_code, error);
    mpi_ret_code = MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    SMGC_MPICHK(mpi_ret_code, error);

    /* set job parameters based on the size of mpi_comm_world
     * do this here, so it can be overwritten by the user
     */
    set_jb_params();

    /* start the timer */
    if (SMGC_MASTER_RANK == my_rank) {
        start_time = MPI_Wtime();
        if (NULL == (start_time_str = get_time_str(&start_clock))) {
            SMGC_ERR_MSG("out of resources\n");
            goto error;
        }
    }

    while (1) {
        opt_indx = 0;

        static struct option long_options[] =
        {
            {"all"       , no_argument,       0, 'a'},
            {"version"   , no_argument,       0, 'v'},
            {"verbose"   , no_argument,       0, 'V'},
            {"help"      , no_argument,       0, 'h'},
            {"stat"      , required_argument, 0, 's'},
            {"write"     , required_argument, 0, 'w'},
            {"n-iters"   , required_argument, 0, 'n'},
            {"msg-size"  , required_argument, 0, 'm'},
            {"file-size" , required_argument, 0, 'M'},
            {"quiet"     , no_argument,       0, 'q'},
            {"with-tests", required_argument, 0, 't'},
            {0           , 0                , 0,  0 }
        };

        c = getopt_long_only(argc, argv, "avVhs:w:n:m:M:qt:", long_options,
                             &opt_indx);

        if (c == -1) {
            break;
        }

        switch (c) {
            case 'a': /* run ALL tests */
                if (tests_on_heap) {
                    free(smgc_test_ptr);
                }
                tests_on_heap = false;
                upd_test_suite(smgc_small_jb_tests);
                break;

            case 'v': /* version */
                SMGC_MPF("%s %s\n", PACKAGE_NAME, PACKAGE_VERSION);
                goto fin;

            case 'V': /* don't shh! */
                ;
                break;

            case 'q': /* shh! */
                ;
                break;

            case 'h': /* socorro! */
                usage();
                goto fin;

            case 's': /* stat something for me please */
                if (num_stat_paths < SMGC_MAX_STAT_PATHS) {
                    strncpy(stat_list[num_stat_paths++], optarg,
                            (SMGC_PATH_MAX - 1));
                }
                break;

            case 'w': /* write some stuff to this path */
                if (num_fs_test_paths < SMGC_MAX_FS_TEST_PATHS) {
                    strncpy(fs_test_list[num_fs_test_paths++], optarg,
                            (SMGC_PATH_MAX - 1));
                }
                break;

            case 'n': /* update number of test iterations */
                i = atoi(optarg);
                if (i >= 0) {
                    num_iters = i;
                }
                break;

            case 'm': /* change the default message size */
                i = atoi(optarg);
                if (i > 0) {
                    if (SMGC_SUCCESS != get_msg_size(optarg, "message", &i)) {
                        goto fin;
                    }
                    else {
                        msg_size = i;
                    }
                }
                /* else we don't change the message size */
                break;

            case 'M': /* change the default file size */
                i = atoi(optarg);
                if (i > 0) {
                    if (SMGC_SUCCESS != get_msg_size(optarg, "file", &i)) {
                        goto fin;
                    }
                    else {
                        file_size = i;
                    }
                }
                /* else we don't change the file size */
                break;

            case 't': /* construct custom list of tests */
                if (SMGC_SUCCESS !=
                    create_test_list(optarg, &smgc_custom_jb_test_ptr)) {
                        SMGC_ERR_MSG("error constructing test list\n");
                        goto fin;
                    }
                break;

            default:
                usage();
                goto fin;
        }
    }

    if (optind < argc) { /* non-option argv elements */
        usage();
        goto fin;
    }

    /* get your host's name */
    if (0 != gethostname(host_name_buff, SMGC_HOST_NAME_MAX - 1)) {
        SMGC_ERR_MSG("unable to get hostname...\n");
        goto error;
    }
    host_name_buff[SMGC_HOST_NAME_MAX - 1] = '\0';

    /* display info header */
    SMGC_MPF("\n   $$$ %s %s $$$\n\n", PACKAGE_NAME, PACKAGE_VERSION);
    SMGC_MPF("   start yyyymmdd-hhmmss  : %s\n", start_time_str);
    SMGC_MPF("   hostname               : %s\n", host_name_buff);
    SMGC_MPF("   numpe                  : %d\n", num_ranks);
    SMGC_MPF("   bin bloat              : %d B\n", SMGC_BIN_SIZE);
    SMGC_MPF("   default msg size       : %d B\n", SMGC_MSG_SIZE);
    SMGC_MPF("   actual msg size        : %d B\n", msg_size);
    SMGC_MPF("   default file size/rank : %d B\n", SMGC_MPI_IO_BUFF_SIZE);
    SMGC_MPF("   actual file size/rank  : %lu B\n", file_size);
    SMGC_MPF("   num iters              : %d\n", num_iters);
    SMGC_MPF("   num tests              : %d\n", num_tests);
    SMGC_MPF("\n");

    for (ii = 0; ii < num_iters; ++ii) {
        SMGC_MPF("   === starting pass %d of %d\n\n", (ii + 1), num_iters);

        /* run each test */
        for (i = 0; i < num_tests; ++i) {
            SMGC_MPF("   === starting : %s test\n", smgc_test_ptr[i].tname);
            /* run and check */
            SMGC_TSTCHK(smgc_test_ptr[i].tfp(), error);
            SMGC_MPF("   === done     : %s test\n", smgc_test_ptr[i].tname);
            SMGC_MPF("\n");
            mpi_ret_code = MPI_Barrier(MPI_COMM_WORLD);
            SMGC_MPICHK(mpi_ret_code, error);
        }
    }

    mpi_ret_code = MPI_Barrier(MPI_COMM_WORLD);
    SMGC_MPICHK(mpi_ret_code, error);

    /* stop the timer */
    if (my_rank == SMGC_MASTER_RANK) {
        end_time = MPI_Wtime();
        exec_time = end_time - start_time;
    }

    SMGC_MPF("   exec time        : %lf (s)\n\n", exec_time);
    SMGC_MPF("   $$$ %s $$$\n\n", "carpe manana");
    SMGC_MPF("   <results> PASSED\n");

    mpi_ret_code = MPI_Finalize();
    SMGC_MPICHK(mpi_ret_code, error);

    if (my_rank == SMGC_MASTER_RANK && NULL != start_time_str) {
        free(start_time_str);
        start_time_str = NULL;
    }

    if (NULL != rhname_lut_ptr) {
        free(rhname_lut_ptr);
    }

    return EXIT_SUCCESS;

fin:
    mpi_ret_code = MPI_Finalize();
    /* jumping to error here is a weird thing, but if MPI_Finalize isn't
     * successful, then at least we'll know.
     */
    SMGC_MPICHK(mpi_ret_code, error);
    return EXIT_SUCCESS;

error:
    MPI_Abort(MPI_COMM_WORLD, mpi_ret_code);
    return EXIT_FAILURE;
}
