# (c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
# Unpublished copyright. All rights reserved. Contains proprietary and
# confidential trade secrets belonging to DDI. Disclosure or release without
# prior written authorization of DDI is prohibited.

.PHONY: all help .build_dir clean cleanall install

AR    := ar rcs
CC    := gcc
CXX   := g++
MKDIR := mkdir -p
RM    := rm -f
RMDIR := rm -rf

ifeq ($(OS),Windows_NT)
  EXE = .exe
  LIB_EXT = lib
  ifeq ($(MINGW_CHOST),)
    OPTIONS = -static
  endif
else
  LIB_EXT = a
  OPTIONS =
endif

ifdef INSTALL_ROOT
override INSTALL_ROOT := $(subst //,/,$(INSTALL_ROOT))
endif

BUILD_ROOT := build

DDI_COMMON := $(abspath .)
include $(DDI_COMMON)/help.mk

LIB_VERSION := 1.0.0
LIB_HEADERS := \
  ddi_atomic.h \
  ddi_crypto.h \
  ddi_debug.h \
  ddi_defines.h \
  ddi_hexdump.h \
  ddi_macros.h \
  ddi_mem_utils.h \
  ddi_ntime.h \
  ddi_os.h \
  ddi_queue.h \
  ddi_seq_utils.h \
  ddi_status.h \
  ddi_str_utils.h \
  ddi_sys_utils.h \
  ddi_xml.h \
  freebsd/base64.h

$(info Build for ddi_common_lib v$(LIB_VERSION))

ifeq ($(MAKECMDGOALS),install)
  $(info INSTALL_ROOT $(INSTALL_ROOT))
endif

# $(BUILD_ROOT)/lib/ddi/ddi_common_lib/$(LIB_VERSION)/lib$(1).a
define LIB
  $(BUILD_ROOT)/lib/lib$(1)_$(LIB_VERSION).$(LIB_EXT)
endef

define OBJS
  $(patsubst %,$(BUILD_ROOT)/obj/%.o,$(basename $(1)))
endef

INCLUDES := $(DDI_COMMON) $(DDI_COMMON)/freebsd
INCLUDE_PATHS := $(patsubst %,-I%,$(INCLUDES))

ALL_SOURCES := ddi_crypto.c ddi_hexdump.c ddi_mem_utils.c ddi_ntime.c ddi_os_posix.c ddi_queue.c ddi_seq_utils.c ddi_str_utils.c ddi_sys_utils.c ddi_xml.c freebsd/base64.c
ALL_OBJECTS := $(call OBJS,$(ALL_SOURCES))

LIB_COMMON_SOURCES := ddi_hexdump.c ddi_mem_utils.c ddi_str_utils.c
ifneq ($(OS),Windows_NT)
LIB_COMMON_SOURCES += ddi_sys_utils.c
endif

LIB_COMMON_OBJECTS := $(call OBJS,$(LIB_COMMON_SOURCES))

LIB_OS_POSIX_SOURCES := ddi_os_posix.c ddi_queue.c
LIB_OS_POSIX_OBJECTS :=  $(call OBJS,$(LIB_OS_POSIX_SOURCES))

LIB_COMMON    = $(call LIB,ddi_common)
LIB_CRYPTO    = $(call LIB,ddi_crypto)
LIB_NTIME     = $(call LIB,ddi_ntime)
LIB_SEQ_UTILS = $(call LIB,ddi_seq_utils)
LIB_OS_POSIX  = $(call LIB,ddi_os_posix)
LIB_XML       = $(call LIB,ddi_xml)
LIB_BASE64    = $(call LIB,ddi_base64)

MACROS_TEST         := $(BUILD_ROOT)/bin/macros_test$(EXE)
MACROS_TEST_SOURCES := tests/macros_test.c
MACROS_TEST_OBJECTS := $(call OBJS,$(MACROS_TEST_SOURCES))

SEQ_TEST            := $(BUILD_ROOT)/bin/seq_test$(EXE)
SEQ_TEST_SOURCES    := tests/seq_test.c
SEQ_TEST_OBJECTS    := $(call OBJS,$(SEQ_TEST_SOURCES))
SEQ_TEST_LIBS       := $(LIB_SEQ_UTILS)

NTIME_TEST          := $(BUILD_ROOT)/bin/ntime_test$(EXE)
NTIME_TEST_SOURCES  := tests/ntime_test.c
NTIME_TEST_OBJECTS  := $(call OBJS,$(NTIME_TEST_SOURCES))
NTIME_TEST_LIBS     := $(LIB_NTIME)

#$(info DDI_COMMON = $(DDI_COMMON))
#$(info TOSIM_SOURCES = $(TOSIM_SOURCES))

LIB_TARGETS := $(LIB_COMMON) $(LIB_CRYPTO) $(LIB_NTIME) $(LIB_SEQ_UTILS) $(LIB_XML) $(LIB_BASE64)
ifneq ($(OS),Windows_NT)
LIB_TARGETS += $(LIB_OS_POSIX)
endif

#ifneq ($(OS),Windows_NT)
TEST_TARGETS = $(MACROS_TEST) $(SEQ_TEST) $(NTIME_TEST)
#endif

all: ##                   Builds all targets (default)
all: .build_dir libs tests

help: ##                  Show this help
	@echo $(SECT_COLOR)-----------------------------------------------------------------------$(CLEAR)
	@echo $(SECT_COLOR)Makefile help:$(CLEAR)
	@echo $(SECT_COLOR)-----------------------------------------------------------------------$(CLEAR)
	@$(call HELP,Makefile)
	@echo

clean: ##                 Cleans all built components
	-$(RM) $(LIB_TARGETS) $(TEST_TARGETS)
	-$(RMDIR) $(BUILD_ROOT)/obj
ifeq ($(OS),Windows_NT)
endif

cleanall: ##              Cleans all built components and build directories
cleanall: clean
	-$(RMDIR) ./.vs $(BUILD_ROOT) ./Debug ./Release ./x64

libs: ##                  Builds the common libraries
libs: .build_dir $(LIB_TARGETS)

tests: .build_dir libs $(TEST_TARGETS)

# target to compile all sources to object files:
$(BUILD_ROOT)/obj/%.o:%.c
	@echo $(YELLOW) 'Building Source: $<' $(CLEAR)
	$(CC) -Wall $(INCLUDE_PATHS) $(OPTIONS) -c $< -o $@
	@echo

.build_dir:
	@$(MKDIR) $(BUILD_ROOT)/obj/freebsd
	@$(MKDIR) $(BUILD_ROOT)/obj/tests
	@$(MKDIR) $(BUILD_ROOT)/lib
	@$(MKDIR) $(BUILD_ROOT)/bin

# Libraries:
$(LIB_COMMON): $(LIB_COMMON_OBJECTS)
	$(AR) $@ $^

$(LIB_OS_POSIX): $(LIB_OS_POSIX_OBJECTS)
	$(AR) $@ $^

$(LIB_CRYPTO): $(call OBJS,ddi_crypto.c)
	$(AR) $@ $^

$(LIB_NTIME): $(call OBJS,ddi_ntime.c)
	$(AR) $@ $^

$(LIB_SEQ_UTILS): $(call OBJS,ddi_ntime.c ddi_seq_utils.c)
	$(AR) $@ $^

$(LIB_XML): $(call OBJS,ddi_xml.c)
	$(AR) $@ $^

$(LIB_BASE64): $(call OBJS,freebsd/base64.c)
	$(AR) $@ $^

# test apps:
$(MACROS_TEST): $(MACROS_TEST_OBJECTS)
	@echo $(RED) 'Building Target: $@' $(CLEAR)
	$(CC) $(LDFLAGS) $^ -o $@ #$(LDLIBS)
	@echo

$(SEQ_TEST): $(SEQ_TEST_OBJECTS)
	@echo $(RED) 'Building Target: $@' $(CLEAR)
	$(CC) $(LDFLAGS) $^ -o $@ $(SEQ_TEST_LIBS)
	@echo

$(NTIME_TEST): $(NTIME_TEST_OBJECTS)
	@echo $(RED) 'Building Target: $@' $(CLEAR)
	$(CC) $(LDFLAGS) $^ -o $@ $(NTIME_TEST_LIBS)
	@echo

install: ##               Installs lib, bin, and headers to a directory specified by INSTALL_ROOT
##                        e.g. make install [cyan]INSTALL_ROOT[clear]=../..
install:
	@$(MKDIR) $(INSTALL_ROOT)/lib/ddi/
	@$(MKDIR) $(INSTALL_ROOT)/bin
	@$(MKDIR) $(INSTALL_ROOT)/include/ddi/common_lib_$(LIB_VERSION)
	cp $(LIB_TARGETS) $(INSTALL_ROOT)/lib/ddi/
#	cp $(TEST_TARGETS) $(INSTALL_ROOT)/bin/
	cp $(LIB_HEADERS) help.mk $(INSTALL_ROOT)/include/ddi/common_lib_$(LIB_VERSION)/

