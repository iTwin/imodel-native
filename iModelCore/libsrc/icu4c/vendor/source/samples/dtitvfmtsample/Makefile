# Copyright (C) 2016 and later: Unicode, Inc. and others.
# License & terms of use: http://www.unicode.org/copyright.html
#
# Copyright (c) 2000-2006 IBM, Inc. and others
# sample code makefile

# Usage:
#  - configure, build, install ICU (make install)
#  - make sure "icu-config" (in the ICU installed bin directory) is on
#     the path
#  - do 'make' in this directory

#### definitions
# Name of your target
TARGET=dtitvfmtsample

# All object files (C or C++)
OBJECTS=dtitvfmtsample.o

#### rules
# Load in standard makefile definitions
include ../defs.mk

LDFLAGS += $(LDFLAGS_USTDIO)

# the actual rules (this is a simple sample)
include ../rules.mk
