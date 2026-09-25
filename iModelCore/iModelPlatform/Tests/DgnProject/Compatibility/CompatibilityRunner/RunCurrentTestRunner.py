#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
from __future__ import print_function
import sys
import os
import shutil
import subprocess
import time
from concurrent.futures import ThreadPoolExecutor, as_completed

# Runs the current compatibility test runner.
#
# Optimized mode (default):
#   1) wipes run/Output (incl. the upgrade cache) shared by all shards
#   2) runs the *CreateTestFiles tests once, which populates run/NewFiles (shared by all shards)
#   3) runs the remaining tests as N concurrent processes (gtest shards, GTEST_TOTAL_SHARDS / GTEST_SHARD_INDEX)
# Non-optimized mode (RUN_EVOLUTION_TESTS_OPTIMIZED=0): runs the test runner once, as a single process, like before.
#
# Every process writes its own gtest log into the log folder.

TESTRUNNER_EXE = "iModelEvolutionTests.exe"
SHARDS_ENV_VAR = "IMODELEVOLUTION_SHARDS"
OPTIMIZED_ENV_VAR = "RUN_EVOLUTION_TESTS_OPTIMIZED"
SETUP_TEST_FILTER = "*CreateTestFiles"

#------------------------------------------------------------------------
# bsimethod
#------------------------------------------------------------------------
def applyOptimizedOption(args):
    """--optimized=0|1 overrides (and is propagated to the test runner processes via) the environment variable."""
    for arg in args:
        if arg.startswith("--optimized="):
            os.environ[OPTIMIZED_ENV_VAR] = arg[len("--optimized="):].strip()

#------------------------------------------------------------------------
# bsimethod
#------------------------------------------------------------------------
def isOptimized():
    value = os.environ.get(OPTIMIZED_ENV_VAR, "1").strip().lower()
    return value not in ("0", "false", "")

#------------------------------------------------------------------------
# bsimethod
#------------------------------------------------------------------------
def getShardCount(args):
    """Number of concurrent test runner processes. Anything non-numeric (empty, 'auto') means cpu_count/2."""
    if not isOptimized():
        return 1
    value = os.environ.get(SHARDS_ENV_VAR, "")
    for arg in args:
        if arg.startswith("--shards="):
            value = arg[len("--shards="):]
    if value.strip().isdigit():
        return max(1, int(value.strip()))
    return max(1, (os.cpu_count() or 2) // 2)

#------------------------------------------------------------------------
# bsimethod
#------------------------------------------------------------------------
def runTestRunner(exePath, logPath, extraArgs, gtestFilter, env):
    """Runs the test runner once. Returns (exit code, elapsed seconds)."""
    start = time.time()
    command = [exePath] + extraArgs
    if gtestFilter:
        command.append("--gtest_filter=" + gtestFilter)

    with open(logPath, "w") as log:
        # CheckLogfilesForFailures.py expects the name of the test runner in the first line
        log.write(exePath + "\n")
        log.flush()
        returnCode = subprocess.call(command, stdout=log, stderr=subprocess.STDOUT, cwd=os.path.dirname(exePath), env=env)

    return (returnCode, time.time() - start)

#------------------------------------------------------------------------
# bsimethod
#------------------------------------------------------------------------
def runShard(exePath, logDir, extraArgs, shardIndex, shardCount):
    env = dict(os.environ)
    env["GTEST_TOTAL_SHARDS"] = str(shardCount)
    env["GTEST_SHARD_INDEX"] = str(shardIndex)
    logPath = os.path.join(logDir, "shard{0}.log".format(shardIndex))
    returnCode, elapsed = runTestRunner(exePath, logPath, extraArgs, "-" + SETUP_TEST_FILTER, env)
    return (shardIndex, returnCode, logPath, elapsed)

#------------------------------------------------------------------------
# bsimethod
#------------------------------------------------------------------------
def writeCombinedLog(combinedLogPath, logPaths):
    """Concatenates the per-process logs into one file."""
    with open(combinedLogPath, "w") as combined:
        for logPath in logPaths:
            combined.write("\n\n==================== {0} ====================\n".format(os.path.basename(logPath)))
            with open(logPath, "r", errors="replace") as log:
                shutil.copyfileobj(log, combined)

#------------------------------------------------------------------------
# bsimethod
#------------------------------------------------------------------------
def cleanupOutput(outputDir):
    """Deletes the shards' output folders and the upgrade cache but keeps everything else in run/Output. Only called after a fully successful run."""
    if not os.path.isdir(outputDir):
        return

    for name in os.listdir(outputDir):
        path = os.path.join(outputDir, name)
        if not os.path.isdir(path):
            continue
        if name.lower().startswith("shard") or name.lower() == "upgradecache":
            shutil.rmtree(path, ignore_errors=True)
    print ("All tests passed. Deleted shard output and upgrade cache in {0}. Use --keep-output to keep them.".format(outputDir))

#------------------------------------------------------------------------
# bsimethod
#------------------------------------------------------------------------
def main():
    OWN_OPTIONS = ("--shards=", "--optimized=", "--setup-only", "--keep-output")
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    extraArgs = [a for a in sys.argv[1:] if a.startswith("--") and not a.startswith(OWN_OPTIONS)]
    setupOnly = "--setup-only" in sys.argv[1:]
    keepOutput = "--keep-output" in sys.argv[1:]
    if len(args) < 3:
        print ("Arg 1: Folder of the current test runner (contains {0} and the run folder)".format(TESTRUNNER_EXE))
        print ("Arg 2: Log folder (one gtest log per process is written there)")
        print ("Arg 3: Path of the combined log (all per-process logs concatenated)")
        print ("Optional: --optimized=0|1 (or env {0}) master switch for the optimizations, defaults to 1".format(OPTIMIZED_ENV_VAR))
        print ("Optional: --shards=N (or env {0}=N) number of concurrent test runner processes".format(SHARDS_ENV_VAR))
        print ("Optional: --setup-only  only wipe the upgrade cache and create the test files (for debugging a shard afterwards)")
        print ("Optional: --keep-output  do not delete the shards' output (cloned/upgraded test files) after a successful run")
        print ("Any other --option is passed through to the test runner (e.g. --timeout=-1)")
        return sys.exit(1)

    applyOptimizedOption(sys.argv[1:])
    testRunnerDir = args[0]
    logDir = args[1]
    combinedLogPath = args[2]
    exePath = os.path.join(testRunnerDir, TESTRUNNER_EXE)
    if not os.path.exists(exePath):
        print ("Current test runner '{0}' does not exist.".format(exePath), file=sys.stderr)
        return sys.exit(1)

    if not os.path.exists(logDir):
        os.makedirs(logDir)

    # remove logs of a previous run so that CheckLogfilesForFailures.py only sees this run's logs
    for name in os.listdir(logDir):
        if name.endswith(".log"):
            os.remove(os.path.join(logDir, name))

    shardCount = getShardCount(sys.argv[1:])
    if shardCount == 1 and not setupOnly:
        print ("Running {0} as a single process ({1}={2})...".format(exePath, OPTIMIZED_ENV_VAR, "1" if isOptimized() else "0"))
        sys.stdout.flush()
        logPath = os.path.join(logDir, "test.log")
        returnCode, elapsed = runTestRunner(exePath, logPath, extraArgs, None, dict(os.environ))
        print ("Finished after {0:.0f}s with exit code {1}.".format(elapsed, returnCode))
        writeCombinedLog(combinedLogPath, [logPath])
        return sys.exit(0) # failures are detected by CheckLogfilesForFailures.py from the logs

    # 1) run/Output (incl. the upgrade cache) is shared by all shards and must not contain files from a previous run.
    #    Shards (gtest processes with GTEST_TOTAL_SHARDS set) do not wipe it themselves, as that would destroy what other shards
    #    already produced. So it is wiped here once before anything starts. (The non-sharded setup process below wipes it too.)
    outputDir = os.path.join(testRunnerDir, "run", "Output")
    if os.path.exists(outputDir):
        shutil.rmtree(outputDir)

    # 2) Create the test files once, before the shards start executing.
    print ("Creating test files ({0})...".format(SETUP_TEST_FILTER))
    sys.stdout.flush()
    setupLogPath = os.path.join(logDir, "setup.log")
    returnCode, elapsed = runTestRunner(exePath, setupLogPath, extraArgs, SETUP_TEST_FILTER, dict(os.environ))
    print ("Test file creation finished after {0:.0f}s with exit code {1}.".format(elapsed, returnCode))
    if returnCode != 0:
        print ("Test file creation failed. Not running any shards.", file=sys.stderr)
        writeCombinedLog(combinedLogPath, [setupLogPath])
        return sys.exit(0 if not setupOnly else 1) # failures are detected by CheckLogfilesForFailures.py from the logs

    if setupOnly:
        print ("--setup-only: test files created, not running any shards.")
        return sys.exit(0)

    # 3) Run everything else as concurrent shards.
    print ("Running the remaining tests in {0} shards...".format(shardCount))
    sys.stdout.flush()
    start = time.time()
    logPaths = [setupLogPath]
    allSucceeded = True
    with ThreadPoolExecutor(max_workers=shardCount) as executor:
        futures = [executor.submit(runShard, exePath, logDir, extraArgs, i, shardCount) for i in range(shardCount)]
        for future in as_completed(futures):
            shardIndex, returnCode, logPath, elapsed = future.result()
            logPaths.append(logPath)
            allSucceeded = allSucceeded and returnCode == 0
            print ("  Shard {0}: exit code {1} after {2:.0f}s (log: {3})".format(shardIndex, returnCode, elapsed, logPath))
            sys.stdout.flush()

    print ("All shards finished after {0:.0f}s.".format(time.time() - start))
    writeCombinedLog(combinedLogPath, sorted(logPaths))

    # 4) The shards' output (cloned/upgraded test files, several GB) is only needed for triaging failures.
    if allSucceeded and not keepOutput:
        cleanupOutput(outputDir)

    return sys.exit(0) # failures are detected by CheckLogfilesForFailures.py from the logs

if __name__ == '__main__':
    main()
