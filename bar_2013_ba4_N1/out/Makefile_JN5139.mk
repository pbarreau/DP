#########################################################################
#
# MODULE:   bar_2013_ba4
#
# DESCRIPTION: bar_2013_ba4_N1 MakeFile
#
############################################################################
# 
#
#############################################################################
# Subversion variables
# $HeadURL:  $
# $Revision: $
# $LastChangedBy:  $
# $LastChangedDate:  $
# $Id:  $ 
#
#############################################################################

# Application target name

TARGET = bar_2013_ba4_N1

#  Note: Target name must be the same as the subfolder name
##############################################################################
#User definable make parameters that may be overwritten from the command line

# Default target device is the JN5139
JENNIC_CHIP ?= JN5139

##############################################################################
# Default DK2 development kit target hardware

JENNIC_PCB ?= DEVKIT2

##############################################################################
# Select the network stack (e.g. MAC, ZBPRO)

JENNIC_STACK ?= JN
# For compatibility with V1 makefile
JENIE_IMPL    ?= JN
# Specify device type (e.g. CR (Coordinator/router), ED (End Device))
DEVICE_TYPE ?= CR
##############################################################################
# Debug options - define DEBUG for debug type
#DEBUG ?=SW
#
#
# Define which UART to use for debug
DEBUG_PORT ?= UART1

##############################################################################
# Define TRACE to use with DBG module
#TRACE ?=1

##############################################################################
# Path definitions
# Select definitions for either single or multiple targets

# Use if application directory contains multiple targets
SDK_BASE_DIR   	 	= $(abspath /cygdrive/c/Jennic/cygwin/jennic/SDK/)
APP_BASE            = $(abspath ../..)
APP_BLD_DIR			= $(APP_BASE)/$(TARGET)/out
APP_SRC_DIR 	    = $(APP_BASE)/$(TARGET)/src
APP_INC_DIR 	    = $(APP_BASE)/$(TARGET)/inc
APP_OBJ_DIR 	    = $(APP_BASE)/$(TARGET)/o39
APP_COMMON_SRC_DIR 	= $(APP_BASE)/module/src
APP_COMMON_INC_DIR 	= $(APP_BASE)/module/inc

##############################################################################
# Application Source files

# Note: Path to source file is found using vpath below, so only .c filename is required
APPSRC += $(wildcard $(APP_SRC_DIR)/*.c)
APPSRC += $(wildcard $(APP_COMMON_SRC_DIR)/*.c)

##############################################################################
# Additional Application Source directories
# Define any additional application directories outside the application directory
# e.g. for AppQueueApi

#ADDITIONAL_SRC_DIR += $(COMPONENTS_BASE_DIR)/AppQueueApi/Source
ADDITIONAL_SRC_DIR += $(SDK_BASE_DIR)/Chip/Common/Source
APPSRC +=Printf.c
##############################################################################
# Standard Application header search paths
# JenieAppConfig must go before other INCFLAG definitions on JN5139 
BASE_DIR ?= $(SDK_BASE_DIR)
include $(SDK_BASE_DIR)/Jenie/Library/JenieAppConfig.mk

# Application specific include files
#Set v1 style include paths
INCFLAGS += -I$(SDK_BASE_DIR)/Chip/Common/Include
INCFLAGS += -I$(SDK_BASE_DIR)/Common/Include
ifeq ($(JENNIC_PCB),DEVKIT1)
   INCFLAGS += -I$(SDK_BASE_DIR)/Platform/DK1/Include 
else
   INCFLAGS += -I$(SDK_BASE_DIR)/Platform/DK2/Include 
endif
INCFLAGS += -I$(SDK_BASE_DIR)/Platform/Common/Include

INCFLAGS += -I$(APP_INC_DIR)
INCFLAGS += -I$(APP_COMMON_INC_DIR)

INCFLAGS += -I$(COMPONENTS_BASE_DIR)/AppQueueApi/Include 
INCFLAGS += -I$(SDK_BASE_DIR)/Jenie/Include 

##############################################################################
# Application libraries
# Specify additional Component libraries
#APPLIBS+=

##############################################################################

# You should not need to edit below this line

##############################################################################
##############################################################################
# Configure for the selected chip or chip family
# Call v1 style config.mk
#BASE_DIR=$(SDK_BASE_DIR)
include $(SDK_BASE_DIR)/Common/Build/config.mk

##############################################################################
INCFLAGS += -I$(SDK_BASE_DIR)/Chip/$(JENNIC_CHIP_FAMILY)/Include

APPOBJS = $(addprefix $(APP_OBJ_DIR)/, $(notdir $(APPSRC:.c=.o)))

##############################################################################
# Application dynamic dependencies
APPDEPS = $(APPOBJS:.o=.d)

#########################################################################
# Linker

# Add application libraries before chip specific libraries to linker so
# symbols are resolved correctly (i.e. ordering is significant for GCC)

#LDLIBS := $(addsuffix _$(JENNIC_CHIP_FAMILY),$(APPLIBS)) $(LDLIBS)

# Set linker variables from V1 Style makefile
BOARDAPI_LIB  = $(BOARDAPI_BASE)/Library
BOARD_LIB     = BoardLib_$(JENNIC_CHIP_FAMILY)

STACK_BASE    = $(BASE_DIR)/Chip/$(JENNIC_CHIP_FAMILY)
STACK_BLD     = $(STACK_BASE)/Build
LINKER_FILE = AppBuild_$(JENNIC_CHIP).ld

# Tell linker to garbage collect unused section
LDFLAGS += --gc-sections
# Define entry points to linker
LDFLAGS += -u_AppColdStart -u_AppWarmStart

# Also add compiler flags to allow garbage collection
CFLAGS += -fdata-sections -ffunction-sections

# Add Jenie / JenNet libraries
LIBFILE += $(JENIE_LIB)/Jenie_Tree$(DEVICE_TYPE)Lib.a
LIBFILE += $(JENIE_LIB_COMMON)

#########################################################################
# Dependency rules

.PHONY: all clean
# Path to directories containing application source 
vpath %.c $(APP_SRC_DIR):$(APP_COMMON_SRC_DIR):$(ADDITIONAL_SRC_DIR)



all: $(TARGET)_$(JENNIC_CHIP)$(BIN_SUFFIX).bin


$(APP_OBJ_DIR)/%.o:%.c
	$(info Compilation fichier >> $(notdir $<) <<)
	@echo "-------------------"
	$(CC) -c -o $@ $(CFLAGS) $(INCFLAGS) $< -MD -MF $(APP_OBJ_DIR)/$*.d -MP
	@echo "-------------------"
	@echo
	@echo

$(TARGET)_$(JENNIC_CHIP)$(BIN_SUFFIX).elf: $(APPOBJS)
	$(info cible $<)
	$(info Liaison avec $@ ...)
# Use LD rather than gcc for JN5139
	$(LD) -L$(STACK_BLD) -o $@ $(APPOBJS) -T$(LINKER_FILE) $(LDFLAGS) --start-group $(LIBS) $(LIBFILE) --end-group
	ba-elf-size $@
	@echo

$(TARGET)_$(JENNIC_CHIP)$(BIN_SUFFIX).bin: $(TARGET)_$(JENNIC_CHIP)$(BIN_SUFFIX).elf 
	$(info -> Generation Fichier Binaire <-)
	$(OBJCOPY) -S -O binary $< $@
	@echo
	@echo "-> Compilation Terminee <-"
	@echo

##	
#########################################################################

clean:
	rm -f $(APPOBJS) $(APPDEPS) $(TARGET)_$(JENNIC_CHIP)*.bin $(TARGET)_$(JENNIC_CHIP)*.elf $(TARGET)_$(JENNIC_CHIP)*.map

#########################################################################
