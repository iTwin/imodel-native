# This make file was been written with the EXPRESS requirement that it
# exists in the "CsMapDev/TestCpp" directory and that directory is
# the current working directory of the make executable which is processing
# it; with the original intent being that this makefile would be invoked
# by a higher level make file which executes something like the following:
#
#	$(MAKE) -e -C./TestCpp -fTestCpp.mak
#
PRJ_NAME = TestCpp
TRG_BASE = CS_TestCpp
LIB_NAME = TestCpp
#
# Set the following default values so that this makefile can be used at
# this level.
#
VERSION ?= 47
CONFIGURATION ?= Linux
PROCESSOR ?= x64
CSMAP_LIB_NAME ?= CsMap
#
# The following definitions are instituted to facilitate building and
# testing multiple versions of the Library. The variables referenced by
# these definitions are normally expected to be passed from a parent
# makefile.  The default values set here represent rather generic values
# which can be useful when in maintenance/development mode with respect
# to the test module itself.
#
OUT_DIR ?= ../bin$(VERSION)/$(CONFIGURATION)
LIB_DIR ?= ../lib$(VERSION)/$(CONFIGURATION)
INT_DIR ?= ../obj$(VERSION)/$(PRJ_NAME)/$(CONFIGURATION)
DICTIONARY_DIR ?= ../Dictionaries
#
# The following options are chosen to be rather generic; something that
# should work in any UNIX/Linux type environment.  More sophisticated
# specifications could/should be coded in the parent make file.  It
# appears that we cannot modify variable which are in the environment,
# so we use our own local variables (LCL_C_FLG, LCL_CXX_FLG) so we
# modify as required by the PROCESSOR specification.
#
C_FLG ?= -c -w -O2 -I../Include
CXX_FLG ?= -c -w -O2 -I../Include
LCL_C_FLG = $(C_FLG) -I../../Include
LCL_CXX_FLG = $(CXX_FLG) -I../../Include
#
# Adjust the above defines for the various processors, currently only
# two: x86 (32 bits) and x64 (64 bit x86)
#
ifeq ($(PROCESSOR),x64)
	OUT_DIR := $(OUT_DIR)64
	INT_DIR := $(INT_DIR)64
	LIB_DIR := $(LIB_DIR)64
	LCL_C_FLG += -m64 -fPIC
	LCL_CXX_FLG += -m64 -fPIC
endif

ifeq ($(PROCESSOR),x86)
	OUT_DIR := $(OUT_DIR)32
	INT_DIR := $(INT_DIR)32
	LIB_DIR := $(LIB_DIR)32
#	C_FLG += -m32
#	CXX_FLG += -m32
endif
#
# Define the targets of this make file.
#
ALL : $(OUT_DIR)/$(TRG_BASE)

$(INT_DIR)/$(TRG_NAME).o : Source/$(TRG_NAME).cpp
	$(CXX) $(LCL_CXX_FLG) -o $(INT_DIR)/$(TRG_NAME).o Source/$(TRG_NAME).cpp

$(OUT_DIR)/$(TRG_BASE) : $(INT_DIR)/$(TRG_BASE).o $(LIB_DIR)/$(LIB_NAME).a $(LIB_DIR)/$(CSMAP_LIB_NAME).a
	$(CXX) -o $(OUT_DIR)/$(TRG_BASE) $(INT_DIR)/$(TRG_BASE).o $(LIB_DIR)/$(LIB_NAME).a $(LIB_DIR)/$(CSMAP_LIB_NAME).a -lm -lc -lgcc -lstdc++

$(LIB_DIR)/$(LIB_NAME).a :
	$(MAKE) -e -C ./Source -f TestCppLib.mak

$(LIB_DIR)/$(CS_LIB_NAME).a :
	$(MAKE) -e -C ../Source -f Library.mak

.PHONY : clean
clean :
	rm -f $(INT_DIR)/.o
	rm -f $(LIB_DIR)/$(LIB_NAME).a
	rm -f $(OUT_DIR)/$(TRG_BASE)

rebuild: clean $(OUT_DIR)/$(TRG_BASE)

$(INT_DIR)/$(TRG_BASE).o : | $(INT_DIR)

$(OUT_DIR)/$(TRG_BASE) : | $(OUT_DIR)

$(INT_DIR) :
	mkdir -p $(INT_DIR)

$(OUT_DIR) :
	mkdir -p $(OUT_DIR)
