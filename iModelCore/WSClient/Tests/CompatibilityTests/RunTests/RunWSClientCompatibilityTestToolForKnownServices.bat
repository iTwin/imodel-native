
set workdir=%OutRoot%WSClientCompatibilityTestTool\
call %OutRoot%Winx64\Product\WSClientCompatibilityTestTool\WSClientCompatibilityTests.exe %1 %2 %3 %4 %5 %6 %7 %8 %9 ^
    --gtest_output=xml:%workdir%results.xml ^
    --workdir %workdir% ^
    --config %~dp0\RunWSClientCompatibilityTestToolForKnownServices.config

call python %~dp0\..\Other\TestReport\makereport.py "%workdir%results.xml" > %workdir%results.htm
start %workdir%results.htm
