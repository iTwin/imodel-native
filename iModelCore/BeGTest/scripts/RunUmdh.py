import os, sys, time, subprocess, linecache
import linecache

scriptsFolderPath = sys.argv[1]
TestExePath = sys.argv[2]
WindowsSdkDir = sys.argv[3]
LogfilesPath = sys.argv[4]
TestExeName = sys.argv[5]
def ExecuteCoverageCommand(cmd):        
        result = os.system('"' + cmd + '"')
        print '\n'+cmd+' Commandline Status: ',result
        if not result == 0:
            print 'Command Failed to execute'
            sys.exit(1)
        

gFlagPath = WindowsSdkDir+ '"\Debuggers/x64\gflags.exe"'
umdhPath = WindowsSdkDir+ '"\Debuggers/x64\umdh.exe"'
tlistPath = WindowsSdkDir+ '"\Debuggers/x64/tlist.exe"'
cmdSetGFlags = gFlagPath +' -i '+ TestExePath + ' +ust'
cmdRemoveGFlags = gFlagPath +' -i '+ TestExePath + ' -ust'
print cmdSetGFlags
processIdCmd = tlistPath + ' -p '+TestExeName
print processIdCmd

#setting user stack trace on for test exe
ExecuteCoverageCommand(cmdSetGFlags)

#Get Process Id
ExecuteCoverageCommand(processIdCmd)
processId = linecache.getline(LogfilesPath+'output.txt',3)
processId = processId[:-1]
print processId

#Run Generate first Log with
if processId !='-1':
        firstSnapShot = umdhPath + ' -p:'+processId+' -f:'+LogfilesPath+'FirstSnapShot.log'
        print firstSnapShot
        ExecuteCoverageCommand(firstSnapShot)
else:
        print "Failed to get first snapshot because process was not running\n"
        sys.exit()

#sleep for 0.1 secs
time.sleep(0.1)
#Run Generate Second Log with
logsGenerated = 0
if processId !='-1':
        logsGenerated = 1
        secondSnapShot = umdhPath + ' -p:'+processId+' -f:'+LogfilesPath+'SecondSnapShot.log'
        print secondSnapShot
        ExecuteCoverageCommand(secondSnapShot)
else:
        print "Failed to get second snapshot because process was not running\n"
        sys.exit()


#Get the Difference of two Snapshots
if logsGenerated == 1:
        differenceLog = umdhPath+ ' -d '+LogfilesPath+'FirstSnapShot.log '+LogfilesPath+ 'SecondSnapShot.log > '+LogfilesPath+'Comparison.log'
        ExecuteCoverageCommand(differenceLog)

#setting removing stack trace on for test exe
ExecuteCoverageCommand(cmdRemoveGFlags)