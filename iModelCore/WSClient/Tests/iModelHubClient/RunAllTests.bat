SET PathToTestExecutable=%OutRoot%Winx64\Product\iModelHubNativeTests\iModelHubNativeTests.exe

REM Execute standard tests
%PathToTestExecutable% --timeout=3500

REM Execute UrlValidator test
%PathToTestExecutable% --gtest_filter=UrlValidator.WhitelistCheck
