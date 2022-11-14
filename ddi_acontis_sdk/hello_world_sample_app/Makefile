.PHONY: all clean install

#version to link against
VERSION=ddi_acontis_sdk_1.0

DDI_SDK_LICENSE_FILENAME=acontis_licenses.csv

SRC_ROOT := $(shell pwd)
ACONTIS_ROOT ?= $(abspath $(SRC_ROOT)/../../acontis_v3/)
SDK_ROOT ?= $(ACONTIS_ROOT)/SDK
WORKSPACE_ROOT := $(abspath $(SRC_ROOT)/../../../)
DST_ROOT ?= $(SRC_ROOT)/build/
DDI_COMMON ?= $(abspath $(SRC_ROOT)/../../ddi_common)
PROJ_ROOT := ../../../
DDI_ACONTIS_SDK_LIB := $(PROJ_ROOT)/lib/ddi_acontis_sdk_1.0/
DDI_ACONTIS_SDK_INC := $(PROJ_ROOT)/include/ddi_acontis_sdk_1.0/
DDI_ACONTIS_INC := $(PROJ_ROOT)/include/ddi_acontis_sdk_1.0/acontis/

include $(DDI_COMMON)/help.mk

ifdef INSTALL_ROOT
override INSTALL_ROOT := $(subst //,/,$(patsubst %,%/,$(INSTALL_ROOT)))
else
INSTALL_ROOT := $(WORKSPACE_ROOT)/
endif

CC  := gcc -g -DDEBUG -Wall
CXX := g++ -g -DDEBUG -Wall
MD  := mkdir -p

TARGET := $(DST_ROOT)/$(notdir $(basename $(SRC_ROOT)))

RUN_SH := $(DST_ROOT)/run.sh

CPP_SOURCES := \
 hello_world.cpp

LIBS := \
 -lpthread \
 -lddi_acontis_sdk \
 -lemllSockRaw \
 -lemllI8254x \
 -lpthread \
 -lncurses \
 -ldl

LIB_PATH := \
 $(DDI_ACONTIS_SDK_LIB)

INCLUDE_PATHS := \
 $(DDI_ACONTIS_SDK_INC) \
 $(DDI_ACONTIS_INC) \
 $(SDK_ROOT)/../Sources/Common \
 $(DDI_COMMON)

INCLUDES := $(patsubst %,-I%,$(INCLUDE_PATHS))
SOURCES := $(C_SOURCES) $(CPP_SOURCES) $(COMMON_SOURCES)
OBJECTS := $(patsubst %,$(DST_ROOT)/obj/%.o,$(notdir $(basename $(SOURCES))))

#$(info SOURCES = $(SOURCES))
#$(info OBJECTS = $(OBJECTS))

all: $(TARGET) $(RUN_SH)

install:
	@-$(MD) $(INSTALL_ROOT)/bin/
	cp $(TARGET) $(INSTALL_ROOT)/bin/
	cp $(DST_ROOT)/run.sh $(INSTALL_ROOT)/bin/
	cp $(DDI_SDK_LICENSE_FILENAME) $(INSTALL_ROOT)/bin/

$(TARGET): $(DST_ROOT) $(OBJECTS)
	$(CXX) $(OBJECTS) -L$(LIB_PATH) -o $@ $(LIBS)

$(DST_ROOT):
	@$(MD) $@/obj

$(DST_ROOT)/obj/%.o: %.c
	@echo 'Building file: $<'
	$(CC) $(INCLUDES) $(C_OPTIONS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o $@ $<
	@echo

$(DST_ROOT)/obj/%.o: %.cpp
	@echo 'Building file: $<'
	$(CXX) $(INCLUDES) $(CPP_OPTIONS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o $@ $<
	@echo

$(RUN_SH):
	@echo "#/usr/bin/bash" > $@
	@echo "" >> $@
	@echo if [ -z $1 ]\; then >> $@
	@echo "export ECAT_NIC=i8254:1" >> $@
	@echo else >> $@
	@echo "export ECAT_NIC=\$$1" >> $@
	@echo "fi" >> $@
	@echo "" >> $@
	#hello world arugments:
	#-i network interface to attach to
	#note: use -i i8254:<instance_number> to talk to the optimized link layer
	#common values for the instance_number are 0,1,2
	#the sqa computer in the lab uses instance_number of 1 and 2 for the optimized link layer
	#-e eni.xml to use for parsing
	#-d process data output display rate in cyclic data cylces
	#-s cyclic data rate in microseconds
	@echo "#-i network interface to attach to" >> $@
	@echo "#-e eni.xml to use for parsing"     >> $@
	@echo "#-d process data output display rate in cyclic data cylces" >> $@
	@echo "#-s cyclic data rate in microseconds" >> $@
	@echo export LD_LIBRARY_PATH=../../../lib/$(VERSION) >> $@
	@echo  ./$(notdir $(TARGET)) -e eni.xml -s 2000 -i \$$ECAT_NIC -d 1000 >> $@
	@echo "" >> $@
	@chmod +x $@

clean:
	@rm -rf $(DST_ROOT)

