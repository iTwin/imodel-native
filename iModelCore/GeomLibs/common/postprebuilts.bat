@ECHO OFF
@IF NOT "%ECHO%."=="." ECHO %ECHO%
rem -------------------------------------------------------------------------
rem
rem $Source: common/postprebuilts.bat $
rem
rem -------------------------------------------------------------------------

setlocal

rem -------------------------------------------------------------------------
rem Check environment
rem -------------------------------------------------------------------------
IF NOT DEFINED OutGeomLibs set OutGeomLibs=$(OutRoot)Geom/
IF NOT DEFINED PREBUILTS_USER GOTO NOUSER_Error
IF NOT DEFINED PREBUILTS_PASSFILE GOTO NOPASS_Error

SET prebuiltsServer=prebuilt-share.bentley.com
SET prebuiltsRoot=prebuilt-firebug
SET prebuiltsDir=/geomlibs/beijing/

REM -------------------------------------------------------------------------
REM Do WinX86
REM -------------------------------------------------------------------------
SET prebuiltsPlatform=Winx86
SET localPrebuiltDir=%OutRoot%%prebuiltsPlatform%\Geomlibs\PrebuiltsForServer

python %SrcRoot%bsicommon\build\bentleybuild.py -s geomlibs;firebug savelkg -x86 %localPrebuiltDir%
IF ERRORLEVEL 1 GOTO EXEC_Error

pushd %localPrebuiltDir%

python %SrcRoot%bsitools\anycpu\RetryCommand.py "%RSYNC_HOME%rsync.exe -vrzt --delete --password-file=%PREBUILTS_PASSFILE% * %PREBUILTS_USER%@%prebuiltsServer%::%prebuiltsRoot%/%prebuiltsPlatform%%prebuiltsDir%"

IF ERRORLEVEL 1 GOTO :EXEC_Error

popd

REM -------------------------------------------------------------------------
REM Do WinX64
REM -------------------------------------------------------------------------
SET prebuiltsPlatform=Winx64
IF NOT EXIST %OutRoot%%prebuiltsPlatform%\Geomlibs goto NO_X64

SET localPrebuiltDir=%OutRoot%%prebuiltsPlatform%\Geomlibs\PrebuiltsForServer

python %SrcRoot%bsicommon\build\bentleybuild.py -s geomlibs;firebug savelkg -x64 %localPrebuiltDir%
IF ERRORLEVEL 1 GOTO EXEC_Error

pushd %localPrebuiltDir%

python %SrcRoot%bsitools\anycpu\RetryCommand.py "%RSYNC_HOME%rsync.exe -vrzt --delete --password-file=%PREBUILTS_PASSFILE% * %PREBUILTS_USER%@%prebuiltsServer%::%prebuiltsRoot%/%prebuiltsPlatform%%prebuiltsDir%"

IF ERRORLEVEL 1 GOTO :EXEC_Error

popd

:NO_X64

goto END
:NOOUT_Error
ECHO #-----------------------------------------------------------------------
ECHO # ERROR: OutGeomLibs not defined.
ECHO # Solution: Run in shared shell.
ECHO #-----------------------------------------------------------------------

goto EXIT_ERROR

:NOUSER_Error
ECHO #-----------------------------------------------------------------------
ECHO # ERROR: PREBUILTS_USER not defined
ECHO # You must define PREBUILTS_USER to be the username used to
ECHO # connect to the prebuilt server
ECHO #-----------------------------------------------------------------------

goto EXIT_ERROR

:NOPASS_Error
ECHO #-----------------------------------------------------------------------
ECHO # ERROR: PREBUILTS_PASSFILE not defined
ECHO # You must define PREBUILTS_PASSFILE to point to the rsync password
ECHO # file used to connect to the prebuilt server
ECHO #-----------------------------------------------------------------------

goto EXIT_ERROR

REM -------------------------------------------------------------------------
REM Something bad happened
REM -------------------------------------------------------------------------
:EXEC_Error

ECHO Exiting %0 with ERROR!

:EXIT_ERROR
REM -------------------------------------------------------------------------
REM See if we are running CMD.EXE or 4NT.EXE
REM -------------------------------------------------------------------------
IF "%@eval[2+2]" == "4" GOTO 4NTEXIT

REM -------------------------------------------------------------------------
REM See if we are running in batch or interactive mode
REM -------------------------------------------------------------------------
IF DEFINED BATCH_BUILD GOTO BATCHEXIT

:DOSEXIT
EXIT /B 1
GOTO END

:BATCHEXIT
EXIT 1
GOTO END

:4NTEXIT
QUIT 1
GOTO END

:END
endlocal
