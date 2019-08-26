# This make file is designed primarily to build the entire target,
# CS_TestCpp in this case, from scratch in an "installation" environment.
# The secondary objective of the design is to minimize the maintenance
# requirements of achieving the primary objective.  Thus, coding of a
# comprehensive map of interdependencies between files is deliberately
# avoided.
#
# Thus, one should _NOT_ expect this make file to compile only the
# specific modules you might have changed in a development/debug
# environment.  Such may happen, and you may be thankful; but do _NOT_
# rely on this.  Fortunately, build times on current Linux systems are
# so small I suspect you will probably never consider the build to
# be overly inefficient.
#
# Note that by simply adding files with the appropriate extensions to
# the appropriate directories, automatically includes them in the
# make process.  Removing files from specific directories has the same
# affect.  To achieve this objective, certain files in particular are
# specifically noted and referenced in this make file and removed from the
# construction of the library.  Your maintenance/development activities may
# require you to add a module name to these lists.
#
# This make file was been written with the EXPRESS requirement that it
# exists in the "CsMapDev/Source" and that directory is the current
# working directory of the make executable which is processing it; with the
# original intent being that this makefile would be invoked by a higher
# level make file which executes something like the following:
#
#	$(MAKE) -C./Source -fTestCppLib.mak
#
# Set up the environment in which we will build, before we define any targets.
#
PRJ_NAME = TestCpp
TRG_BASE = TestCpp
TRG_EXT = .a
TRG_NAME = $(TRG_BASE)$(TRG_EXT)
#
# Set the following default values using the ?= assignment operator.  This
# operator causes an assignment only if the variable is not already defined.
# My current gcc compiler is version 4.7.4, so I set the VERSION argument
# to 47.
#
VERSION ?= 47
CONFIGURATION ?= Linux
PROCESSOR ?= x64
#
# CSMAP_MAINS is set to a list of source modules known to contain "main"
# functions, and therefore are inappropriate for inclusion in the library.
# These names are filtered out before the make process starts.
#
CSMAP_MAINS = csTestCpp.cpp
#
# CSMAP_STDHDRS is set to a list of source modules which exist primarily for
# generation of pre-compiled headers when using the Microsoft Visual Studio
# IDE.  We do not need to consider them for this Linux/Unix/Mac library.
#
CSMAP_STDHDRS = CStest0.cpp
#
# The following definitions are instituted to facilitates building multiple
# versions of the Library. The variables referenced by these definitions
# are expected to be passed from a parent makefile.  
#
OUT_DIR ?= ../../lib$(VERSION)/$(CONFIGURATION)
INT_DIR ?= ../../obj$(VERSION)/$(PRJ_NAME)/$(CONFIGURATION)
SRC_DIR ?= $(MAKEDIR)

C_FLG ?= -c -w -O2 -I../Include
CXX_FLG ?= -c -w -O2 -I../Include
LCL_C_FLG = $(C_FLG) -I../../Include
LCL_CXX_FLG = $(CXX_FLG) -I../../Include

ifeq ($(PROCESSOR),x64)
	OUT_DIR := $(OUT_DIR)64
	INT_DIR := $(INT_DIR)64
	LCL_C_FLG += -m64 -fPIC
	LCL_CXX_FLG += -m64 -fPIC
endif

ifeq ($(PROCESSOR),x86)
	OUT_DIR := $(OUT_DIR)32
	INT_DIR := $(INT_DIR)32
#	LCL_C_FLG += -m32
#	LCL_CXX_FLG += -m32
endif
#
# The -o option on the compiler is used to get the objects written to the
# directory specified by the INT_DIR variable. Doing so enables building
# libraries for different configurations without an implied clean of
# configuration(s) already built.  I suspect that it also means that the
# compilations will not be batched and a new instance of the compiler is
# invoked for each module.  Slows us down a bit, but overall helps preserve
# sanity when trying to build multiple configurations of the library from a
# single set of source files, which is a major objective of this make file.
#
$(INT_DIR)/%.o:$(SRC_DIR)%.cpp
	$(CXX) $(LCL_CXX_FLG) -o$(INT_DIR)/$(<:.cpp=.o) $<

# Note that the following causes all .c and .cpp files in the Source
# directory to be included in the TestCpp.lib target. This is often helpful,
# sometimes painful.  In some cases, the features used may not be available
# in your make implementation.  So, in case this is undesirable or does not
# work, a variable which lists all current distribution sources is coded
# below, with a different name. OsGeo contributors must make sure this
# listing remains current even though it may not be used in your
# environment.  Note that modules not intended for the basic library
# are filtered out.
# 
CSMAP_CPP_SRC := $(wildcard *.cpp)
CSMAP_CPP_SRC := $(filter-out $(CSMAP_MAINS),$(CSMAP_CPP_SRC))  
CSMAP_CPP_SRC := $(filter-out $(CSMAP_STDHDRS),$(CSMAP_CPP_SRC))  

CSMAP_SRC_CPP = CS_osGeoTest.cpp \
				CStest1.cpp \
				CStest2.cpp \
				CStest3.cpp \
				CStest4.cpp \
				CStest5.cpp \
				CStest6.cpp \
				CStest7.cpp \
				CStest8.cpp \
				CStest9.cpp \
				CStestA.cpp \
				CStestB.cpp \
				CStestC.cpp \
				CStestD.cpp \
				CStestE.cpp \
				CStestF.cpp \
				CStestG.cpp \
				CStestH.cpp \
				CStestI.cpp \
				CStestJ.cpp \
				CStestK.cpp \
				CStestL.cpp \
				CStestM.cpp \
				CStestN.cpp \
				CStestQ.cpp \
				CStestR.cpp \
				CStestS.cpp \
				CStestSupport.cpp \
				CStestT.cpp

# The following assignment uses the wildcard generated module list.
CSMAP_LIB_SRC += $(CSMAP_CPP_SRC)

# The following two assignment uses the hard coded module list.
#CSMAP_LIB_SRC = $(CSMAP_SRC_CPP)

# From the source list, generate an equivalent list of objects with the
# desired intermediate directory prefix.

CSMAP_LIB_OBJ := $(patsubst %.cpp,%.o,$(CSMAP_LIB_SRC))
CSMAP_LIB_OBJ := $(addprefix $(INT_DIR)/,$(CSMAP_LIB_OBJ))

# OK, we should now be able to do some real work.  This is rather
# simple now.

$(OUT_DIR)/$(TRG_NAME) : $(CSMAP_LIB_OBJ)
	ar rv $(OUT_DIR)/$(TRG_NAME) $?

.PHONY : clean
clean :
	rm -f $(INT_DIR)/*.o
	rm -f $(OUT_DIR)/$(TRG_NAME)

.PHONY : rebuild
rebuild: clean $(OUT_DIR)/$(TRG_NAME)

#
# The following create the directories in which the results are to be
# written if they don't already exist.  The -p option on the 'mkdir'
# command creates all intermediate directories as well; and also inhibits
# an error condition of they already exist.
#
$(CSMAP_LIB_OBJ) : | $(INT_DIR)

$(OUT_DIR)/$(TRG_NAME) : | $(OUT_DIR)

$(INT_DIR) :
	mkdir -p $(INT_DIR)

$(OUT_DIR) :
	mkdir -p $(OUT_DIR)
