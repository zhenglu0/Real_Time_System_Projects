# Copyright (C) 2011 
# Intel Corporation
# 
# This file is part of the Intel Cilk Plus Library.  This library is free
# software; you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the
# Free Software Foundation; either version 3, or (at your option)
# any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# Under Section 7 of GPL version 3, you are granted additional
# permissions described in the GCC Runtime Library Exception, version
# 3.1, as published by the Free Software Foundation.
# 
# You should have received a copy of the GNU General Public License and
# a copy of the GCC Runtime Library Exception along with this program;
# see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
# <http://www.gnu.org/licenses/>.

AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I .. -I ../config

# Compiler and linker flags.
GENERAL_FLAGS = -I$(top_srcdir)/include -DIN_CILK_RUNTIME=1
GENERAL_FLAGS += -fcilkplus
GENERAL_FLAGS += -DBUILD_USER=\"$(USER)\"
GENERAL_FLAGS += -DBUILD_HOST=\"`hostname`\"
AM_CFLAGS = $(GENERAL_FLAGS) -std=c99
AM_CPPFLAGS = $(GENERAL_FLAGS)
AM_LDFLAGS = -lpthread -ldl

# Target list.
toolexeclib_LTLIBRARIES = libcilkrts.la

libcilkrts_la_SOURCES =            \
  runtime/bug.cpp                  \
  runtime/cilk-abi.c               \
  runtime/cilk-abi-cilk-for.cpp    \
  runtime/cilk-abi-vla.c           \
  runtime/cilk-abi-vla-internal.c  \
  runtime/cilk_api.c               \
  runtime/cilk_fiber.cpp           \
  runtime/cilk_fiber-unix.cpp      \
  runtime/cilk_malloc.c            \
  runtime/c_reducers.c             \
  runtime/except-gcc.cpp           \
  runtime/frame_malloc.c           \
  runtime/full_frame.c             \
  runtime/global_state.cpp         \
  runtime/jmpbuf.c                 \
  runtime/local_state.c            \
  runtime/metacall_impl.c          \
  runtime/os_mutex-unix.c          \
  runtime/os-unix.c                \
  runtime/pedigrees.c              \
  runtime/record-replay.cpp        \
  runtime/reducer_impl.cpp         \
  runtime/scheduler.c              \
  runtime/signal_node.c            \
  runtime/spin_mutex.c             \
  runtime/stats.c                  \
  runtime/symbol_test.c            \
  runtime/sysdep-unix.c            \
  runtime/worker_mutex.c

# Load the $(REVISION) value.
include include/internal/rev.mk

# libcilkrts_la_LDFLAGS  = -rpath '$(libdir)'
# libcilkrts_la_LDFLAGS = -version-info 5:$(CILK_REVISION):0
libcilkrts_la_LDFLAGS = -lpthread -ldl

# C/C++ header files for Cilk.
cilkincludedir = $(includedir)/cilk
cilkinclude_HEADERS =              \
  include/cilk/cilk_api.h          \
  include/cilk/cilk_api_linux.h    \
  include/cilk/cilk.h              \
  include/cilk/cilk_stub.h         \
  include/cilk/cilk_undocumented.h \
  include/cilk/common.h            \
  include/cilk/holder.h            \
  include/cilk/hyperobject_base.h  \
  include/cilk/metaprogramming.h   \
  include/cilk/reducer_file.h      \
  include/cilk/reducer.h           \
  include/cilk/reducer_list.h      \
  include/cilk/reducer_max.h       \
  include/cilk/reducer_min.h       \
  include/cilk/reducer_min_max.h   \
  include/cilk/reducer_opadd.h     \
  include/cilk/reducer_opand.h     \
  include/cilk/reducer_opmul.h     \
  include/cilk/reducer_opor.h      \
  include/cilk/reducer_opxor.h     \
  include/cilk/reducer_ostream.h   \
  include/cilk/reducer_string.h

# Work around what appears to be a GNU make bug handling MAKEFLAGS
# values defined in terms of make variables, as is the case for CC and
# friends when we are called from the top level Makefile.
AM_MAKEFLAGS = \
	"AR_FLAGS=$(AR_FLAGS)" \
	"CC_FOR_BUILD=$(CC_FOR_BUILD)" \
	"CFLAGS=$(CFLAGS)" \
	"CXXFLAGS=$(CXXFLAGS)" \
	"CFLAGS_FOR_BUILD=$(CFLAGS_FOR_BUILD)" \
	"CFLAGS_FOR_TARGET=$(CFLAGS_FOR_TARGET)" \
	"INSTALL=$(INSTALL)" \
	"INSTALL_DATA=$(INSTALL_DATA)" \
	"INSTALL_PROGRAM=$(INSTALL_PROGRAM)" \
	"INSTALL_SCRIPT=$(INSTALL_SCRIPT)" \
	"JC1FLAGS=$(JC1FLAGS)" \
	"LDFLAGS=$(LDFLAGS)" \
	"LIBCFLAGS=$(LIBCFLAGS)" \
	"LIBCFLAGS_FOR_TARGET=$(LIBCFLAGS_FOR_TARGET)" \
	"MAKE=$(MAKE)" \
	"MAKEINFO=$(MAKEINFO) $(MAKEINFOFLAGS)" \
	"PICFLAG=$(PICFLAG)" \
	"PICFLAG_FOR_TARGET=$(PICFLAG_FOR_TARGET)" \
	"SHELL=$(SHELL)" \
	"RUNTESTFLAGS=$(RUNTESTFLAGS)" \
	"exec_prefix=$(exec_prefix)" \
	"infodir=$(infodir)" \
	"libdir=$(libdir)" \
	"prefix=$(prefix)" \
	"includedir=$(includedir)" \
	"AR=$(AR)" \
	"AS=$(AS)" \
	"LD=$(LD)" \
	"LIBCFLAGS=$(LIBCFLAGS)" \
	"NM=$(NM)" \
	"PICFLAG=$(PICFLAG)" \
	"RANLIB=$(RANLIB)" \
	"DESTDIR=$(DESTDIR)"

MAKEOVERRIDES=
