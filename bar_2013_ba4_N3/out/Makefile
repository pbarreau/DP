#########################################################################
#
# MODULE:   bar_2013_ba4
#
# DESCRIPTION: bar_2013_ba4_N3 MakeFile
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

TARGET = bar_2013_ba4_N3

#  Note: Target name must be the same as the subfolder name
##############################################################################
#User definable make parameters that may be overwritten from the command line

# Default target device is the JN5148
JENNIC_CHIP ?= JN5148

##############################################################################
# Default DK2 development kit target hardware

JENNIC_PCB ?= DEVKIT2

##############################################################################
# Select the network stack (e.g. MAC, ZBPRO)

JENNIC_STACK ?= JenNet
# Specify device type (e.g. CR (Coordinator/router), ED (End Device))
DEVICE_TYPE ?= ED
##############################################################################
# Debug options define DEBUG for HW debug
#DEBUG ?=HW
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
SDK_BASE_DIR   	 	= $(abspath /cygdrive/c/Jennic)
APP_BASE            = $(abspath ../..)
APP_BLD_DIR			= $(APP_BASE)/$(TARGET)/out
APP_SRC_DIR 	    = $(APP_BASE)/$(TARGET)/src
APP_INC_DIR 	    = $(APP_BASE)/$(TARGET)/inc
APP_OBJ_DIR 	    = $(APP_BASE)/$(TARGET)/o48
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
ADDITIONAL_SRC_DIR += $(COMPONENTS_BASE_DIR)/../cygwin/jennic/SDK/Chip/Common/Source
APPSRC +=Printf.c
##############################################################################
# Standard Application header search paths

INCFLAGS += -I$(APP_INC_DIR)
INCFLAGS += -I$(APP_COMMON_INC_DIR)

# Application specific include files
INCFLAGS += -I$(COMPONENTS_BASE_DIR)/JennicLogo/Include
INCFLAGS += -I$(COMPONENTS_BASE_DIR)/Utilities/Include
INCFLAGS += -I$(COMPONENTS_BASE_DIR)/AppQueueApi/Include 
INCFLAGS += -I$(COMPONENTS_BASE_DIR)/JenNet/Include 
INCFLAGS += -I$(COMPONENTS_BASE_DIR)/Jenie/Include 

##############################################################################
# Application libraries
# Specify additional Component libraries
#APPLIBS+=

##############################################################################

# You should not need to edit below this line

##############################################################################
##############################################################################
# Configure for the selected chip or chip family

include $(SDK_BASE_DIR)/Chip/Common/Build/config.mk
include $(SDK_BASE_DIR)/Platform/Common/Build/Config.mk
include $(SDK_BASE_DIR)/Stack/Common/Build/Config.mk

##############################################################################

APPOBJS = $(addprefix $(APP_OBJ_DIR)/, $(notdir $(APPSRC:.c=.o)))

##############################################################################
# Application dynamic dependencies
APPDEPS = $(APPOBJS:.o=.d)

#########################################################################
# Linker

# Add application libraries before chip specific libraries to linker so
# symbols are resolved correctly (i.e. ordering is significant for GCC)

LDLIBS := $(addsuffix _$(JENNIC_CHIP_FAMILY),$(APPLIBS)) $(LDLIBS)

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

$(TARGET)_$(JENNIC_CHIP)$(BIN_SUFFIX).elf: $(APPOBJS) $(addsuffix _$(JENNIC_CHIP_FAMILY).a,$(addprefix $(COMPONENTS_BASE_DIR)/Library/lib,$(APPLIBS))) 
	$(info Linking $@ ...)
	$(CC) -Wl,--gc-sections -Wl,-u_AppColdStart -Wl,-u_AppWarmStart $(LDFLAGS) -T$(LINKCMD) -o $@ $(APPOBJS) -Wl,--start-group  $(addprefix -l,$(LDLIBS)) -Wl,--end-group -Wl,-Map,$(TARGET)_$(JENNIC_CHIP)$(BIN_SUFFIX).map 

$(TARGET)_$(JENNIC_CHIP)$(BIN_SUFFIX).bin: $(TARGET)_$(JENNIC_CHIP)$(BIN_SUFFIX).elf 
	${SIZE} $<
	@echo
	$(info -> Generation Fichier Binaire <-)
	$(OBJCOPY) -j .flashheader -j .oad -j .mac -j .heap_location -j .rtc_clt -j .rodata -j .data -j .text -j .bss -j .heap -j .stack -S -O binary $< $@
	@echo
	@echo "-> Compilation Terminee <-"
	@echo
	
#########################################################################

clean:
	rm -f $(APPOBJS) $(APPDEPS) $(TARGET)_$(JENNIC_CHIP)*.bin $(TARGET)_$(JENNIC_CHIP)*.elf $(TARGET)_$(JENNIC_CHIP)*.map

#########################################################################
