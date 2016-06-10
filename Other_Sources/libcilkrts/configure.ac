#  Copyright (C) 2011
#  Intel Corporation
#  
#  This file is part of the Intel Cilk Plus Library.  This library is free
#  software; you can redistribute it and/or modify it under the
#  terms of the GNU General Public License as published by the
#  Free Software Foundation; either version 3, or (at your option)
#  any later version.
#  
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  Under Section 7 of GPL version 3, you are granted additional
#  permissions described in the GCC Runtime Library Exception, version
#  3.1, as published by the Free Software Foundation.
#  
#  You should have received a copy of the GNU General Public License and
#  a copy of the GCC Runtime Library Exception along with this program;
#  see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
#  <http://www.gnu.org/licenses/>.

AC_INIT([Cilk Runtime Library], [2.0], [cilk@intel.com])
AC_PREREQ([2.64])
# AM_INIT_AUTOMAKE([1.11 no-define])
AM_INIT_AUTOMAKE(foreign no-dist)

# Build a DLL on Windows
# AC_LIBTOOL_WIN32_DLL
AC_PROG_CXX
AC_PROG_CC
# AC_PROG_LIBTOOL
# AC_CONFIG_MACRO_DIR([..])
AC_CONFIG_FILES([Makefile])
AM_ENABLE_MULTILIB(, ..)

if test "${multilib}" = "yes"; then
  multilib_arg="--enable-multilib"
else
  multilib_arg=
fi

AC_MSG_CHECKING([for --enable-version-specific-runtime-libs])
AC_ARG_ENABLE([version-specific-runtime-libs],
  AC_HELP_STRING([--enable-version-specific-runtime-libs],
                 [Specify that runtime libraries should be installed in a compiler-specific directory]),
  [case "$enableval" in
    yes) enable_version_specific_runtime_libs=yes ;;
    no)  enable_version_specific_runtime_libs=no ;;
    *)   AC_MSG_ERROR([Unknown argument to enable/disable version-specific libs]);;
   esac],
  [enable_version_specific_runtime_libs=no])
AC_MSG_RESULT($enable_version_specific_runtime_libs)


# Calculate toolexeclibdir
# Also toolexecdir, though it's only used in toolexeclibdir
case ${enable_version_specific_runtime_libs} in
  yes)
    # Need the gcc compiler version to know where to install libraries
    # and header files if --enable-version-specific-runtime-libs option
    # is selected.
    toolexecdir='$(libdir)/gcc/$(target_alias)'
    toolexeclibdir='$(toolexecdir)/$(gcc_version)$(MULTISUBDIR)'
    ;;
  no)
    if test -n "$with_cross_host" &&
       test x"$with_cross_host" != x"no"; then
      # Install a library built with a cross compiler in tooldir, not libdir.
      toolexecdir='$(exec_prefix)/$(target_alias)'
      toolexeclibdir='$(toolexecdir)/lib'
    else
      toolexecdir='$(libdir)/gcc-lib/$(target_alias)'
      toolexeclibdir='$(libdir)'
    fi
    multi_os_directory=`$CC -print-multi-os-directory`
    case $multi_os_directory in
      .) ;; # Avoid trailing /.
      *) toolexeclibdir=$toolexeclibdir/$multi_os_directory ;;
    esac
    ;;
esac
AC_SUBST(toolexecdir)
AC_SUBST(toolexeclibdir)

AM_PROG_LIBTOOL
AC_SUBST(enable_shared)
AC_SUBST(enable_static)
AC_OUTPUT
