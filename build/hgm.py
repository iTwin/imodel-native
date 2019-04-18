#--------------------------------------------------------------------------------------
#
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#
#--------------------------------------------------------------------------------------
import multiprocessing
import os
import Queue
import subprocess
import sys
import tempfile
import threading
import time

# ***************************************************************************************************************************************************
# ***************************************************************************************************************************************************

#====================================================================================================================================================
class CommandJob:
    def __init__(self, command, resultList):
        self.m_command = command
        self.m_resultList = resultList

    def GetCommand(self):
        return self.m_command

    def AppendResult(self, result):
        return self.m_resultList.append(result)

    def HasAnyFailures(self):
        return 0 != len([result for result in self.m_resultList if 255 == result.GetReturnCode()])

#====================================================================================================================================================
class CommandResult:
    def __init__(self, command, returnCode, output, elapsedTime):
        self.m_command = command
        self.m_returnCode = returnCode
        self.m_output = output
        self.m_time = elapsedTime

    def GetCommand(self):
        return self.m_command
    
    def GetReturnCode(self):
        return self.m_returnCode
    
    def GetOutput(self):
        return self.m_output

    def GetElapsedTime(self):
        return self.m_time

#----------------------------------------------------------------------------------------------------------------------------------------------------
def runCommand(args):
    workQueue = args
    
    while True:
        job = workQueue.get()
        command = job.GetCommand()

        if job.HasAnyFailures():
            workQueue.task_done()
            continue

        sys.stdout.flush()
        sys.stdout.write("INFO>     running '{0}'...\n".format(command))
        sys.stdout.flush()
        
        startTime = time.time()
        
        with tempfile.TemporaryFile() as procOutputFile:
            proc = subprocess.Popen(command, stdout=procOutputFile, stderr=subprocess.STDOUT)
            
            retVal = proc.wait()

            procOutputFile.seek(0)
            output = procOutputFile.read()

        endTime = time.time()

        job.AppendResult(CommandResult(command, retVal, output, (endTime - startTime)))
        workQueue.task_done()

#----------------------------------------------------------------------------------------------------------------------------------------------------
def runCommands(commands, numProcs=None):
    if not numProcs or numProcs < 1:
        numProcs = max(1, (multiprocessing.cpu_count() / 2))

    workQueue = Queue.Queue()
    results = []
    for i in range(numProcs):
        t = threading.Thread(target=runCommand, args=[workQueue])
        t.daemon = True
        t.start()

    for command in commands:
        workQueue.put(CommandJob(command, results))

    while not workQueue.empty():
        time.sleep(1)

    workQueue.join()

    anyFailures = False

    for nonFailResult in [result for result in results if 255 != result.GetReturnCode()]:
        print
        print(nonFailResult.GetCommand())

        res = nonFailResult.GetOutput().rstrip()
        if len(res) == 0:
            res = "<no output>"
        
        res = "    " + res.replace("\n", "\n    ")
        print(res)
    
    for failResult in [result for result in results if 255 == result.GetReturnCode()]:
        anyFailures = True
        print
        print("WARN>     command failed with status {0}: '{1}'".format(failResult.GetReturnCode(), failResult.GetCommand()))
        print(failResult.GetOutput().rstrip())

    if anyFailures:
        # raw_input("Paused (press enter)...")
        return 1

    return 0

# ***************************************************************************************************************************************************
# ***************************************************************************************************************************************************

#----------------------------------------------------------------------------------------------------------------------------------------------------
def findHgRepos(srcPath):
    repoPaths = []

    for fileName in os.listdir(srcPath):
        currDir = os.path.join(srcPath, fileName)
        if not os.path.isdir(currDir) or os.path.exists(os.path.join(currDir, "CVS")):
            continue

        if "sdksource" in currDir.lower() or "lastknowngood" in currDir.lower():
            continue

        if os.path.exists(os.path.join(currDir, ".hg")):
            repoPaths.append(os.path.realpath(currDir))
            continue

        subPaths = findHgRepos(currDir)
        repoPaths.extend(subPaths)

    return repoPaths

#----------------------------------------------------------------------------------------------------------------------------------------------------
def hgm():
    USAGE_STR = "Usage: <script> HG_ARGS"

    if len(sys.argv) < 1:
        print(USAGE_STR)
        return 1

    hgArgs = " ".join(sys.argv[1:])
    hgRepoPaths = findHgRepos(".")
    commands = []

    for hgRepoPath in hgRepoPaths:
        cmd = "hg -R " + hgRepoPath + " " + hgArgs
        # print("INFO> Queueing '" + cmd + "'")
        commands.append(cmd)

    runCommands(commands)

    return 0

#----------------------------------------------------------------------------------------------------------------------------------------------------
if __name__ == '__main__':
    sys.exit(hgm())
