
@echo;
@echo Running specific test cases. Get test list:
@echo    RunWSClientCompatibilityTestToolForKnownServices.bat "--gtest_list_tests"
@echo Filter on test number:
@echo    RunWSClientCompatibilityTestToolForKnownServices.bat "--gtest_filter=*/12"
@echo;

set workdir=%OutRoot%WSClientCompatibilityTestTool\
call %OutRoot%Winx64\Product\WSClientCompatibilityTestTool\WSClientCompatibilityTests.exe %1 %2 %3 %4 %5 %6 %7 %8 %9 ^
    --gtest_output=xml:%workdir%results.xml ^
    --workdir %workdir% ^
    --config %~dp0\RunWSClientCompatibilityTestToolForKnownServices.config

call python %~dp0\TestReport\makereport.py "%workdir%results.xml" > %workdir%results.htm
start %workdir%results.htm
