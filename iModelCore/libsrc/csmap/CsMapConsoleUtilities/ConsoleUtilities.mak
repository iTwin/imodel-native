# This make file was been written with the EXPRESS requirement that it
# exists in the "CsMapDev/CsMapConsoleUtilities" directory and that
# directory is the current working directory of the make executable which
# is processing it; with the original intent being that this makefile would
# be invoked by a higher level make file which executes something like the
# following:
#
#	$(MAKE) -e -C./CsMapConsoleUtilities -f ConsoleUtilities.mak
#
PRJ_NAME = ConsoleUtilities
TRG_BASE = ConsoleUtilities
TRG_SRC_BASE = csConsoleUtilites
#
# Set the following default values so that this makefile can be used at
# this level.
#
VERSION ?= 47
CONFIGURATION ?= Linux
PROCESSOR ?= x64
UTILITIES_LIB_BASE ?= ConsoleUtilitiesLib
UTILITIES_LIB_EXT = .a
UTILITIES_LIB_NAME = $(UTILITIES_LIB_BASE)$(UTILITIES_LIB_EXT)
#
# The following definitions are instituted to facilitate building and
# testing multiple versions of the Library. The variables referenced by
# these definitions are normally expected to be passed from a parent
# makefile.  The default values set here represent rather generic values
# which can be useful when in maintenance/development mode with respect
# to the console utility module itself.
#
OUT_DIR ?= ../bin$(VERSION)/$(CONFIGURATION)
LIB_DIR ?= ../lib$(VERSION)/$(CONFIGURATION)
INT_DIR ?= ../obj$(VERSION)/$(PRJ_NAME)/$(CONFIGURATION)
#
# The following options are chosen to be rather generic; something that
# should work in any UNIX/Linux type environment.  More sophisticated
# specifications can/should be coded in the parent make file.
#
C_FLG ?= -c -w -O2 -I../Include
CXX_FLG ?= -c -w -O2 -I../Include
#
# Adjust the above defines for the various processors, currently only
# two: x86 (32 bits) and x64 (64 bit x86)
#
ifeq ($(PROCESSOR),x64)
	OUT_DIR := $(OUT_DIR)64
	INT_DIR := $(INT_DIR)64
	LIB_DIR := $(LIB_DIR)64
	C_FLG += -m64 -fPIC
	CXX_FLG += -m64 -fPIC
endif

ifeq ($(PROCESSOR),x86)
	OUT_DIR := $(OUT_DIR)32
	INT_DIR := $(INT_DIR)32
	LIB_DIR := $(LIB_DIR)32
	C_FLG += -m32
	CXX_FLG += -m32
endif

#
# Define the targets of this make file.
#
ALL : $(OUT_DIR)/$(TRG_BASE) $(LIB_DIR)/$(UTILITIES_LIB_NAME)

$(INT_DIR)/$(TRG_SRC_BASE).o : $(TRG_SRC_BASE).cpp
	$(CC) $(C_FLG) -o $(INT_DIR)/$(TRG_SRC_BASE).o $<

$(OUT_DIR)/$(TRG_BASE) : $(INT_DIR)/$(TRG_SRC_BASE).o $(LIB_DIR)/$(UTILITIES_LIB_NAME) $(LIB_DIR)/$(CSMAP_LIB_NAME).a
	gcc -I../Include -o $(OUT_DIR)/$(TRG_BASE) $(INT_DIR)/$(TRG_SRC_BASE).o \
											   $(LIB_DIR)/$(UTILITIES_LIB_NAME) \
											   $(LIB_DIR)/$(CSMAP_LIB_NAME).a \
											   -lm -lc -lgcc -lstdc++

$(LIB_DIR)/$(UTILITIES_LIB_NAME) :
	$(MAKE) -e -C ./Source -f ConsolUtilitiesLib.mak

$(LIB_DIR)/$(CSMAP_LIB_NAME).a :
	$(MAKE) -e -C ../Source -f Library.mak

.PHONY : clean
clean :
	rm -f $(INT_DIR)/*.o
	rm -f $(OUT_DIR)/$(TRG_BASE)

rebuild: clean $(OUT_DIR)/$(TRG_BASE) $(LIB$(DICTIONARIES)

$(INT_DIR)/$(TRG_BASE).o : | $(INT_DIR)

$(OUT_DIR)/$(TRG_BASE) : | $(OUT_DIR)

$(INT_DIR) :
	mkdir -p $(INT_DIR)

$(OUT_DIR) :
	mkdir -p $(OUT_DIR)
