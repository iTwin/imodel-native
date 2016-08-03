#--------------------------------------------------------------------------------------
#
#     $Source: gtest/TeeGtest.py $
#
#  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os, glob, sys, string, stat, re, subprocess

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

#-------------------------------------------------------------------------------------------
# bsimethod                                     Sam.Wilson              04/2016
#-------------------------------------------------------------------------------------------
def main():
    if (len (sys.argv) < 2):
        print "Syntax: ", sys.argv[0], " [-q] outputfilename"
        exit(1)

    logfilename = ''
    quiet = False
    i = 0
    while i < len(sys.argv):
        if sys.argv[i] == "-q":
            quiet = True        
        else:
            logfilename = sys.argv[i]
        i = i + 1

    #[==========] Running 467 tests from 91 test cases.
    #[----------] 1 test from BackwardsCompatibilityTests

    totalcount = 0
    currentcount = 0

    with open (logfilename, 'a+') as logfile:
        try:
            while True:
                line = raw_input ('')


                if totalcount == 0:
                    if line.startswith('[==========] Running'):
                        totalcount = int(re.search('[0-9]+', line).group(0));
                else:
                    if line.startswith('[ RUN'):
                        currentcount = currentcount + 1;

                logfile.write (line + "\n" )

                if not quiet:
                    progress = '{0}/{1}'.format(currentcount, totalcount)
                    print bcolors.BOLD + progress.ljust(10) + bcolors.ENDC + line

        except EOFError:
            exit (0)

if __name__ == '__main__':
    main()
