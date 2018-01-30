@echo off

echo;
echo Build executable with:
echo    bb -ax64 -r WSClient -f WSClient -p WSClientCompatibilityTestTool b
echo Running without parameters - returns all tests:
echo    RunWSClientCompatibilityTestToolForKnownServices.bat
echo Run all tests:
echo    RunWSClientCompatibilityTestToolForKnownServices.bat *
echo Run specific test by number:
echo    RunWSClientCompatibilityTestToolForKnownServices.bat */10

set gtestAction=--gtest_filter=*RepositoryCompatibilityTests*%1
if "%1" == "" set gtestAction=--gtest_list_tests

set workdir=%OutRoot%WSClientCompatibilityTestTool\
set outfile=%workdir%results.xml

if exist %outfile% del %outfile%

@echo on

call %OutRoot%Winx64\Product\WSClientCompatibilityTestTool\WSClientCompatibilityTests.exe ^
    %2 %3 %4 %5 %6 %7 %8 %9 ^
    %gtestAction% ^
    --gtest_output=xml:%outfile% ^
    --workdir %workdir% ^
    --config %~dp0\RunWSClientCompatibilityTestToolForKnownServices.config

@echo off

if exist %outfile% (
call python %~dp0\TestReport\makereport.py "%outfile%" > %workdir%results.htm
start %workdir%results.htm
)
