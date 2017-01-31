@echo off

set CDOCTARGET=%MSJO%Bentley\Documentation\cdoc\

set SCRIPTDIR=%GEMADIR%cdoc\
call %SCRIPTDIR%removeDeprecated.cmd

cd /d %CDOCTARGET%

del index.1
del index.2
del index.3
del index.html

rem This line fails in 4NT --- the redirect doesn't take.
for /R %%i in (*.html) do gema -f %SCRIPTDIR%collectindexentries.g %%i >>%CDOCTARGET%index.1

rem tr [A-Z] [a-z] <index.1 | sort > index.2
%windir%\system32\sort index.1 >index.2

gema -f %SCRIPTDIR%indexfrompairs.g index.2 index.3

copy %SCRIPTDIR%index.head+index.3+%SCRIPTDIR%index.tail index.html

gema -f %SCRIPTDIR%masterindexfrompairs.g index.2 master.body
copy %SCRIPTDIR%master.head+master.body+%SCRIPTDIR%master.tail masterframe.html


copy %SCRIPTDIR%cdocmaster.html .

if "%NOCLEANUP%." NEQ "." goto endOfCleanup

rem del index.1
rem del index.2
rem del index.3
rem del master.body
rem del master.bak

:endOfCleanup:

