.PHONY: all help dirs debug release

#SUBDIRS := hello_world_sample_app

SDK_API_VERSION=ddi_acontis_sdk_1.1

SDK_LIBRARY_FILENAME=libddi_acontis_sdk.so
SDK_RAW_SOCKET_FILENAME=libemllSockRaw.so
SDK_ECMASTER_LIB_FILENAME=libEcMaster.a
SDK_I8254_FILENAME=libemllI8254x.so

OS ?= $(shell uname)

SRC_ROOT := $(shell pwd)
WORKSPACE_ROOT := $(abspath $(SRC_ROOT)/)
SRC_ROOT := $(shell pwd)
ACONTIS_ROOT ?= $(abspath $(SRC_ROOT)/acontis_lib)
SDK_ROOT ?= $(ACONTIS_ROOT)/SDK
LIB_ROOT := $(SDK_ROOT)/LIB/Linux/x64
DST_ROOT ?= $(SRC_ROOT)/build
BIN_ROOT := $(ACONTIS_ROOT)/Bin/Linux/x64
DDI_COMMON ?= $(abspath $(SRC_ROOT)/ddi_common_lib)


#export header section
ACONTIS_EXPORT_INCLUDES := $(SDK_ROOT)/INC

ACONTIS_EXPORT_INCLUDES_OS := $(SDK_ROOT)/INC/Linux/EcOsPlatform.h

#these files are exported to the lib/$(SDK_API_VERSION) directory
SDK_EXPORT_HEADERS := \
  include/ddi_sdk_common.h \
  include/ddi_sdk_ecat_link_layer.h \
  include/ddi_sdk_ecat_sdo.h \
  include/ddi_sdk_processing.h \
  include/ddi_sdk_display.h \
  include/ddi_sdk_ecat_master.h \
  include/ddi_sdk_fusion_interface.h \
  include/ddi_odbc.h \
  include/ddi_sdk_rim_config.h

ifdef INSTALL_ROOT
override INSTALL_ROOT := $(subst //,/,$(patsubst %,%/,$(INSTALL_ROOT)))
else
INSTALL_ROOT := $(WORKSPACE_ROOT)/
endif

AR  := ar
CC := gcc
CXX := g++ -fPIC -g
MD  := mkdir -p

SHARED_LIBS := \
 $(DST_ROOT)/$(SDK_LIBRARY_FILENAME)

C_OPTIONS := -fPIC -DDEBUG

CPP_OPTIONS := -fPIC -std=c++11 -DDEBUG
# -fPIC
# -std=c99

C_SOURCES := 

COMMON_SOURCES := \
 $(DDI_COMMON)/ddi_os_posix.c \
 $(DDI_COMMON)/ddi_ntime.c \
 $(DDI_COMMON)/ddi_queue.c

CPP_SOURCES := \
 src/ddi_sdk_ecat_master.cpp \
 src/ddi_sdk_fusion_interface.cpp \
 src/ddi_sdk_ecat_sdo.cpp \
 src/ddi_sdk_ecat_link_layer.cpp \
 src/ddi_sdk_common.cpp \
 src/ddi_sdk_display.cpp \
 src/ddi_odbc.cpp \
 src/ddi_sdk_processing.cpp 

LIBS := \
 $(LIB_ROOT)/libEcMaster.a \
 -lodbc \
 -lpthread \
 -ldl

INCLUDE_PATHS := \
 $(SRC_ROOT)/include/ \
 $(SDK_ROOT)/INC \
 $(SDK_ROOT)/INC/Linux \
 $(SDK_ROOT)/../Sources/Common \
 $(DDI_COMMON)

DEPS = -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)"

define OBJS
	$(patsubst %,$(DST_ROOT)/obj/%.o,$(basename $(1)))
endef


INCLUDES := $(patsubst %,-I%,$(INCLUDE_PATHS))
SOURCES  := $(C_SOURCES) $(CPP_SOURCES) $(COMMON_SOURCES)
OBJECTS  := $(patsubst %,$(DST_ROOT)/obj/%.o,$(basename $(SOURCES)))
DST_TREE := $(patsubst %,$(DST_ROOT)/obj/%,$(dir $(SOURCES)))

ODSUTIL_OBJS := $(call OBJS,$(ODSUTIL_SOURCES))

DST_TREE := $(patsubst %,$(DST_ROOT)/obj/%,$(dir $(SOURCES) $(ODSUTIL_SOURCES)))

#$(SUBDIRS):
#	$(MAKE) -C $@ $(MAKECMDGOALS)

#ifdef VERBOSE
$(info TARGETS = $(TARGETS))
$(info SOURCES = $(SOURCES))
$(info OBJECTS = $(OBJECTS))
#endif

all: ##            Builds all targets (default)
#all: $(DST_ROOT)/libddi_acontis_sdk.so
all: $(SHARED_LIBS)

apps: ##           Builds all apps
apps: $(APPS)

clean: ##          Cleans all built components
	-@rm -rf $(DST_ROOT)
	-@rm -f $(TARGETS)

help: ##           Show this help
	@echo $(SECT_COLOR)-----------------------------------------------------------------------$(CLEAR)
	@echo $(SECT_COLOR)Makefile help:$(CLEAR)
	@echo $(SECT_COLOR)-----------------------------------------------------------------------$(CLEAR)
	@$(call HELP,Makefile)
	@echo

$(DST_ROOT)/libddi_acontis_sdk.so: $(DST_ROOT) $(OBJECTS)
	@echo $(RED)'Building target: $@'$(CLEAR)
	$(CXX) $(OBJECTS) $(INCLUDES) --shared -o $@ $(LIBS)
	@echo

$(DST_ROOT):
	@$(MD) $(DST_TREE)

$(DST_ROOT)/obj/%.o: %.c
	@echo $(YELLOW)'Building file: $<'$(CLEAR)
	$(CC) -Wall $(INCLUDES) $(C_OPTIONS) $(DEPS) -c $< -o $@
	@echo

$(DST_ROOT)/obj/%.o: %.cpp
	@echo $(YELLOW)'Building file: $<'$(CLEAR)
	$(CXX) $(INCLUDES) $(CPP_OPTIONS) $(DEPS) -c $< -o $@

install: ##        Installs libddi_acontis_sdk.so
         ##        Installs libddi_acontis_sdk header files
         ##        Installs Acontis header and library
         ##        Install to $(INSTALL_ROOT)
         ##        Optional: [cyan]INSTALL_ROOT[clear]=/path to install
install:
	@-$(MD) $(INSTALL_ROOT)/lib/$(SDK_API_VERSION)
	@-$(MD) $(INSTALL_ROOT)/include/$(SDK_API_VERSION)/acontis/
	@-$(MD) $(INSTALL_ROOT)/bin
	@cp -r $(ACONTIS_EXPORT_INCLUDES)/* $(ACONTIS_EXPORT_INCLUDES_OS) $(INSTALL_ROOT)/include/$(SDK_API_VERSION)/acontis/
	@cp $(SDK_EXPORT_HEADERS) $(INSTALL_ROOT)/include/$(SDK_API_VERSION)/
	@cp $(BIN_ROOT)/$(SDK_RAW_SOCKET_FILENAME) $(INSTALL_ROOT)/lib/$(SDK_API_VERSION)/
	@cp $(BIN_ROOT)/$(SDK_I8254_FILENAME) $(INSTALL_ROOT)/lib/$(SDK_API_VERSION)/
	@cp $(DST_ROOT)/$(SDK_LIBRARY_FILENAME) $(INSTALL_ROOT)/lib/$(SDK_API_VERSION)/
	@cp $(LIB_ROOT)/$(SDK_ECMASTER_LIB_FILENAME) $(INSTALL_ROOT)/include/$(SDK_API_VERSION)

