#--------------------------------------------------------------------------------------
#
#     $Source: Android/RunAndroidJUnitTest.py $
#
#  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os, glob, sys, string, stat, re, subprocess

sys.path.append(os.path.join (os.getenv("SrcRoot"), "bsicommon", "build"))
from bentleybuild.utils import *

#-------------------------------------------------------------------------------------------
# bsimethod                                     Sam.Wilson              04/2016
#-------------------------------------------------------------------------------------------
def main():
    if (len (sys.argv) < 4):
        print "Syntax: ", sys.argv[0], " logfilename installscript logcatscript"
        exit(1)

    exeDir = os.path.dirname(sys.argv[0])

    logfilename = sys.argv[1]
    installscript = sys.argv[2]
    logcatscript = sys.argv[3]

    with open (logfilename, 'w') as logfile:
        with open(installscript, 'r') as installCmds:
            for installCmd in installCmds.readlines():
                print installCmd,
                proc = subprocess.Popen(installCmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell = True)
                procStdOutLine = proc.stdout.readline ()
                while procStdOutLine:
                    print procStdOutLine,
                    logfile.write(procStdOutLine)
                    procStdOutLine = proc.stdout.readline ()
                proc.wait()


    adbCmd = os.path.join(os.path.join(os.environ["ANDROID_SDK_ROOT"], "platform-tools"), "adb");
    cmd = [adbCmd, "shell", "am", "instrument", "-w", "com.bentley.unittest/com.bentley.unittest.BeInstrumentationTestRunner"]
    print " ".join(cmd)
    proc = subprocess.Popen(" ".join(cmd), stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell = True)

    # NEEDS WORK logcat always hangs. I can't seem to kill it
    #logcatCmd = ""
    #with open (logcatscript, 'r') as logcatscriptfile:
    #    logcatCmd = logcatscriptfile.read()
    #print logcatCmd
    #logcatproc = subprocess.Popen(logcatCmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell = True, creationflags = subprocess.CREATE_NEW_PROCESS_GROUP)
    #
    #teeCmd = os.path.join (os.getenv("SrcRoot"), "bsicommon", "build", "Tee.py")
    #tee = subprocess.Popen ("python " + teeCmd + " " + logfilename, shell=True, stdin=logcatproc.stdout, creationflags = subprocess.CREATE_NEW_PROCESS_GROUP)

    failureCount = 0
    procStdOutLine = proc.stdout.readline ()
    while procStdOutLine:
        showRewritableLine(procStdOutLine.replace('\n', ' ').replace('\r', ' '), INFO_LEVEL_Important)
        if procStdOutLine.lower().startswith("failure"):
            failureCount = failureCount + 1
        procStdOutLine = proc.stdout.readline ()
    proc.wait()

    #logcatproc.kill()

    exit(failureCount)

if __name__ == '__main__':
    main()
