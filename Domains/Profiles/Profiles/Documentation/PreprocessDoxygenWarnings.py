#--------------------------------------------------------------------------------------
#
#     $Source: Profiles/Documentation/PreprocessDoxygenWarnings.py $
#
#  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os, sys

# Python's parser (or maybe its .tag file) gets confused by this:
# struct S {
#   struct H : Handle<S> {}
# }

s_ignore = {'Warning: Detected potential recursive class relation between class', ' : <unknown>(1)'}

def ignoreLine(line):
    for ignore in s_ignore:
        if line.find(ignore) != -1:
            return True;
    return False;


if __name__ == '__main__':
    if (len (sys.argv) < 2):
        print "Syntax: ", sys.argv[0], " <warningLogFile>"
        exit(1)

    warningLogFile = open (sys.argv[1], 'r')
    warningLines = warningLogFile.readlines()
    for warningLine in warningLines:
        if not ignoreLine(warningLine):
            print warningLine

    exit(0)
