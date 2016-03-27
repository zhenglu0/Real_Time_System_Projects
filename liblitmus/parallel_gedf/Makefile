# figure out what kind of host we are running on
host-arch := $(shell uname -m | \
	sed -e s/i.86/i386/ -e s/sun4u/sparc64/ -e s/arm.*/arm/)

# ##############################################################################
# User variables

# user variables can be specified in the environment or in a .config file
-include .config

# ARCH -- what architecture are we compiling for?
ARCH ?= ${host-arch}

# LITMUS_KERNEL -- where to find the litmus kernel?
LITMUS_KERNEL ?= ../../litmus-rt


# ##############################################################################
# Internal configuration.

# compiler flags
flags-debug    = -O2 -Wall -g -Wno-write-strings
flags-api      = -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -fopenmp

# architecture-specific flags
flags-i386     = -m32
flags-x86_64   = -m64
flags-sparc64  = -mcpu=v9 -m64
# default: none

# name of the directory that has the arch headers in the Linux source
include-i386     = x86
include-x86_64   = x86
include-sparc64  = sparc
# default: the arch name
include-${ARCH} ?= ${ARCH}

# name of the file(s) that holds the actual system call numbers
unistd-i386      = uapi/asm/unistd.h generated/uapi/asm/unistd_32.h
unistd-x86_64    = uapi/asm/unistd.h generated/uapi/asm/unistd_64.h
# default: unistd.h
unistd-${ARCH}  ?= uapi/asm/unistd.h

# by default we use the local version
LIBLITMUS ?= .

# where to find header files
headers =  -I${LIBLITMUS}/include -I${LIBLITMUS}/arch/${include-${ARCH}}/include
headers += -I${LIBLITMUS}/arch/${include-${ARCH}}/include/uapi
headers += -I${LIBLITMUS}/arch/${include-${ARCH}}/include/generated/uapi


CPPFLAGS = ${flags-debug} ${flags-api} ${flags-${ARCH}} -DARCH=${ARCH} ${headers}
CFLAGS   = ${flags-debug}
LDFLAGS  = ${flags-${ARCH}}

# how to link against liblitmus
liblitmus-flags = -L${LIBLITMUS} -llitmus 

# Force gcc instead of cc, but let the user specify a more specific version if
# desired.
ifeq (${CC},cc)
CC = gcc
CXX = g++
endif

# incorporate cross-compiler (if any)
CC  := ${CROSS_COMPILE}${CC}
LD  := ${CROSS_COMPILE}${LD}
AR  := ${CROSS_COMPILE}${AR}

# ##############################################################################
# Targets

all     = lib ${EXECUTABLE} 

EXECUTABLE = task_manager clustering_launcher synthetic_task synthetic_task_utilization\
             simple_task  simple_task_utilization

.PHONY: all lib clean dump-config TAGS tags cscope help

all: ${all} inc/config.makefile

# Write a distilled version of the flags for clients of the library. Ideally,
# this should depend on liblitmus.a, but that requires LIBLITMUS to be a
# private override. Private overrides are only supported starting with make
# 3.82, which is not yet in common use.
inc/config.makefile: LIBLITMUS = $${LIBLITMUS}
inc/config.makefile: Makefile
	@printf "%-15s= %-20s\n" \
		ARCH ${ARCH} \
		CFLAGS '${CFLAGS}' \
		LDFLAGS '${LDFLAGS}' \
		LDLIBS '${liblitmus-flags}' \
		CPPFLAGS '${CPPFLAGS}' \
		CC '${shell which ${CC}}' \
		LD '${shell which ${LD}}' \
		AR '${shell which ${AR}}' \
	> $@

dump-config:
	@echo Build configuration:
	@printf "%-15s= %-20s\n" \
		ARCH ${ARCH} \
		LITMUS_KERNEL "${LITMUS_KERNEL}" \
		CROSS_COMPILE "${CROSS_COMPILE}" \
		headers "${headers}" \
		"kernel headers" "${imported-headers}" \
		CFLAGS "${CFLAGS}" \
		LDFLAGS "${LDFLAGS}" \
		CPPFLAGS "${CPPFLAGS}" \
		CC "${CC}" \
		CPP "${CPP}" \
		LD "${LD}" \
		AR "${AR}" \
		obj-all "${obj-all}"

help:
	@cat INSTALL

clean:
	rm -f ${EXECUTABLE}
	rm -f *.o *.d *.a *.rtps *.pyc *~ test_catalog.inc
	rm -f ${imported-headers}
	rm -f inc/config.makefile
	rm -f tags TAGS cscope.files cscope.out

# Emacs Tags
TAGS:
	@echo TAGS
	@find . -type f -and  -iname '*.[ch]' | xargs etags

# Vim Tags
tags:
	@echo tags
	@find . -type f -and  -iname '*.[ch]' | xargs ctags

# cscope DB
cscope:
	@echo cscope
	@find . -type f -and  -iname '*.[ch]' | xargs printf "%s\n" > cscope.files
	@cscope -b

# ##############################################################################
# Kernel headers.
# The kernel does not like being #included directly, so let's
# copy out the parts that we need.

# Litmus headers
include/litmus/%.h: ${LITMUS_KERNEL}/include/litmus/%.h
	@mkdir -p ${dir $@}
	cp $< $@

# asm headers
arch/${include-${ARCH}}/include/uapi/asm/%.h: \
	${LITMUS_KERNEL}/arch/${include-${ARCH}}/include/uapi/asm/%.h
	@mkdir -p ${dir $@}
	cp $< $@

arch/${include-${ARCH}}/include/generated/uapi/asm/%.h: \
	${LITMUS_KERNEL}/arch/${include-${ARCH}}/include/generated/uapi/asm/%.h
	@mkdir -p ${dir $@}
	cp $< $@

litmus-headers = \
	include/litmus/rt_param.h \
	include/litmus/fpmath.h \
	include/litmus/unistd_32.h \
	include/litmus/unistd_64.h

unistd-headers = \
  $(foreach file,${unistd-${ARCH}},arch/${include-${ARCH}}/include/$(file))


imported-headers = ${litmus-headers} ${unistd-headers}

# Let's not copy these twice.
.SECONDARY: ${imported-headers}

# ##############################################################################
# liblitmus

lib: liblitmus.a

# all .c file in src/ are linked into liblitmus
vpath %.c src/
obj-lib = $(patsubst src/%.c,%.o,$(wildcard src/*.c))

liblitmus.a: ${obj-lib}
	${AR} rcs $@ $+

# ##############################################################################
# clusering
CLUSTERING_OBJECTS = single_use_barrier.o timespec_functions.o
libclustering.a: $(CLUSTERING_OBJECTS)
	${AR} rcsf libclustering.a $(CLUSTERING_OBJECTS)

# ##############################################################################
# these .cpp source files
vpath %.cpp 
obj-task_manager = task_manager.o synthetic_task.o timespec_functions.o
lib-task_manager = -fopenmp -lrt -lm

obj-clustering_launcher = clustering_launcher.o single_use_barrier.o
lib-clustering_launcher = -lclustering -lrt -lm

obj-simple_task =  simple_task.o task_manager.o
lib-simple_task = -lclustering -lrt -lm -fopenmp

obj-simple_task_utilization =  simple_task.o utilization_calculator.o
lib-simple_task_utilization = -lclustering -lrt -lm -fopenmp

obj-synthetic_task =  synthetic_task.o task_manager.o
lib-synthetic_task = -lclustering -lrt -lm -fopenmp

obj-synthetic_task_utilization =  synthetic_task.o utilization_calculator.o
lib-synthetic_task_utilization = -lclustering -lrt -lm -fopenmp

# ##############################################################################
# Build everything that depends on liblitmus.

.SECONDEXPANSION:
${EXECUTABLE}: $${obj-$$@} liblitmus.a libclustering.a
	$(CXX) -o $@ $(LDFLAGS) ${ldf-$@} $(filter-out liblitmus.a,$+) $(LDLIBS) ${liblitmus-flags} ${lib-$@}

# ##############################################################################
# Dependency resolution.

obj-all = ${sort ${foreach target,${all},${obj-${target}}}}

# rule to generate dependency files
%.d: %.c ${imported-headers}
	@set -e; rm -f $@; \
		$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$

# rule to generate dependency files
%.d: %.cpp ${imported-headers}
	@set -e; rm -f $@; \
		$(CXX) -MM $(CPPFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$

ifeq ($(MAKECMDGOALS),)
MAKECMDGOALS += all
endif

ifneq ($(filter-out dump-config clean help,$(MAKECMDGOALS)),)

# Pull in dependencies.
-include ${obj-all:.o=.d}

# Let's make sure the kernel header path is ok.
config-ok  := $(shell test -d "${LITMUS_KERNEL}" || echo invalid path. )
config-ok  += $(shell test -f "${LITMUS_KERNEL}/${word 1,${litmus-headers}}" \
	|| echo cannot find header. )
ifneq ($(strip $(config-ok)),)
$(info (!!) Could not find a LITMUS^RT kernel at ${LITMUS_KERNEL}: ${config-ok})
$(info (!!) Are you sure the path is correct?)
$(info (!!) Run 'make dump-config' to see the build configuration.)
$(info (!!) Edit the file .config to override the default configuration.)
$(error Cannot build without access to the LITMUS^RT kernel source)
endif

kernel-unistd-hdrs := $(foreach file,${unistd-headers},${LITMUS_KERNEL}/$(file))
hdr-ok     := $(shell egrep '\#include ["<]litmus/unistd|__NR_litmus_lock' ${kernel-unistd-hdrs} )
ifeq ($(strip $(hdr-ok)),)
$(info (!!) Could not find LITMUS^RT system calls in ${kernel-unistd-hdrs}.)
$(error Your kernel headers do not seem to be LITMUS^RT headers)
endif

config-ok  := $(shell test -f "${LITMUS_KERNEL}/${word 1,${unistd-headers}}" \
	|| echo fail )
ifneq ($(config-ok),)
$(info (!!) Could not find the architecture-specifc Linux headers.)
$(info (!!) Are you sure ARCH=${ARCH} is correct?)
$(info (!!) Run 'make dump-config' to see the build configuration.)
$(info (!!) Edit the file '.config' to override the default configuration.)
$(error Cannot build without access to the architecture-specific files)
endif

endif
