#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import os, glob, sys, string, stat, re, subprocess

#class bcolors:
#    HEADER = '\033[95m'
#    OKBLUE = '\033[94m'
#    OKGREEN = '\033[92m'
#    WARNING = '\033[93m'
#    FAIL = '\033[91m'
#    ENDC = '\033[0m'
#    BOLD = '\033[1m'
#    UNDERLINE = '\033[4m'

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def main():
    if (len (sys.argv) < 2):
        print ("Syntax: " + sys.argv[0] + " [-q] outputfilename")
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

    if sys.version_info[0] >= 3 and sys.version_info[1] >= 7:
        sys.stdin.reconfigure(encoding='utf-8')

    with open (logfilename, 'a+') as logfile:
        try:
            while True:
                try:
                    if sys.version_info[0] > 2:
                        line = input ('')
                    else:
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
                        #print bcolors.BOLD + progress.ljust(10) + bcolors.ENDC + line
                        print (progress.ljust(10) + line)
                except UnicodeEncodeError as exc:
                    print(exc)
                    continue

        except EOFError:
            exit (0)

if __name__ == '__main__':
    main()
