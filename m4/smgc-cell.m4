dnl
dnl Copyright (c) 2012      Los Alamos National Security, LLC.
dnl                         All rights reserved.
dnl
dnl This program was prepared by Los Alamos National Security, LLC at Los
dnl Alamos National Laboratory (LANL) under contract No. DE-AC52-06NA25396 with
dnl the U.S.  Department of Energy (DOE). All rights in the program are
dnl reserved by the DOE and Los Alamos National Security, LLC. Permission is
dnl granted to the public to copy and use this software without charge,
dnl provided that this Notice and any statement of authorship are reproduced on
dnl all copies. Neither the U.S.  Government nor LANS makes any warranty,
dnl express or implied, or assumes any liability or responsibility for the use
dnl of this software.
dnl

dnl cell support configury

AC_DEFUN([SMGC_CELL], [
    AC_ARG_ENABLE([cell],
                  [AS_HELP_STRING([--enable-cell],
                                  [Enable Cell support (default:disabled)])])

    AC_MSG_CHECKING([if want cell support])

    if test "x$enable_cell" = "xyes"; then
        AC_MSG_RESULT([yes])
        dnl now check if we have adequate support
        AC_PATH_PROG([PPUGCC], [ppu-gcc])
        if test -z "$PPUGCC"; then
            enable_cell_support=0
            AC_MSG_ERROR([Cell support requested, but not available.])
        else
            enable_cell_support=1
        fi
    else
        AC_MSG_RESULT([no])
        enable_cell_support=0
    fi

    AM_CONDITIONAL([SMGC_BUILD_CELL], [test "x$enable_cell_support" = "x1"])

    AC_DEFINE_UNQUOTED([SMGC_HAVE_CELL_SUPPORT],
                       [$enable_cell_support],
                       [Define to 1 if have Cell support.])
])dnl
