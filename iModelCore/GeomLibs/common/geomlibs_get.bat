@ECHO OFF
@IF NOT "%ECHO%."=="." ECHO %ECHO%
setlocal
REM -------------------------------------------------------------------------
REM
REM $Source: common/geomlibs_get.bat $
REM
REM -------------------------------------------------------------------------

rem -------------------------------------------------------------------------
rem Set up the CVS stuff
rem -------------------------------------------------------------------------
IF NOT DEFINED SharedShell goto MustBeInSharedShellError

rem -------------------------------------------------------------------------
rem Set up for tagged get
rem -------------------------------------------------------------------------
IF NOT ".%1."==".." SET GEOMLIBS_STAMP=%1
IF DEFINED GEOMLIBS_STAMP SET CVS_STAMP=-r%GEOMLIBS_STAMP%


ECHO #-----------------------------------------------------------------------
ECHO # Checking out directories
ECHO #-----------------------------------------------------------------------

chdir /d %SrcGeomLibs%..\
bmake +dSOURCE=%SrcGeomLibs%../ +dPolicyFile=%SrcRoot%bsicommon\sharedmki\AssertInternalSystemPolicy.mki %SrcGeomLibs%common/geomlibs_get.mke

IF ERRORLEVEL 1 GOTO :EXEC_Error

goto end

:MustBeInSharedShellError
ECHO #---------------------------------------------------------------------------
ECHO # ERROR: SharedShell is not defined.  You must run Geomlibs_get.bat in the shared shell.
ECHO #---------------------------------------------------------------------------

goto EXEC_Error

:EXEC_Error

REM -------------------------------------------------------------------------
REM See if we are running CMD.EXE or 4NT.EXE
REM -------------------------------------------------------------------------
if "%@eval[2+2]" == "4" GOTO 4NTEXIT

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

:end
endlocal
