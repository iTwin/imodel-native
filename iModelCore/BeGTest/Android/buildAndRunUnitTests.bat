if not exist %ANDROID_TEST_TARGET_PROJECT_ROOT%\build.xml goto :prepProject
if not exist %ANDROID_TEST_TEST_PROJECT_ROOT%\build.xml goto :prepProject

if .%1 == .clean goto :clean

REM --------------------------------------------------------
REM Building SOs
call bb -sBeTestAndroid reb BentleyDll DgnCore DgnHandlers BUnitTest* Generate* CopyTo*
if errorlevel 1 goto :xit

if not exist %ANDROID_TEST_TARGET_PROJECT_ROOT%\libs\armeabi-v7a\lib*.so goto :mustRunBB
if not exist %ANDROID_TEST_TEST_PROJECT_ROOT%\src\com\bentley\unittest\*.java goto :mustRunBB

REM --------------------------------------------------------
REM Building the .class files and the .APK files

del %ANDROID_TEST_TARGET_PROJECT_ROOT%bin\bin\Test-debug*.apk 
del %ANDROID_TestTest_TARGET_PROJECT_ROOT%bin\bin\TestTest-debug*.apk

cd/d  %ANDROID_TEST_TARGET_PROJECT_ROOT%
REM call ant -q clean
call ant -q debug

cd/d %ANDROID_TEST_TEST_PROJECT_ROOT%
REM call ant -q clean
call ant -q debug

@echo on

copy %ANDROID_TEST_TARGET_PROJECT_ROOT%bin\Test-debug.apk           %ANDROID_TEST_TARGET_PROJECT_ROOT%bin\Test.apk
copy %ANDROID_TestTest_TARGET_PROJECT_ROOT%bin\TestTest-debug.apk   %ANDROID_TEST_TEST_PROJECT_ROOT%bin\TestTest.apk

REM Un-install the APK, if necessary - not sure when you need to do this
REM adb uninstall com.bentley.test
REM adb uninstall com.bentley.unittest

REM --------------------------------------------------------
REM Installing the APKs on the device...
adb -d install -r %ANDROID_TEST_TARGET_PROJECT_ROOT%bin\Test-debug.apk
adb -d install -r %ANDROID_TEST_TEST_PROJECT_ROOT%bin\TestTest-debug.apk

REM --------------------------------------------------------
REM Run unittests from the command line:
REM Run the following to monitor logging messages:
REM    adb logcat

REM --------------------------------------------------------
REM 
REM adb shell am instrument -e class com.bentley.unittest.BentleyUnitTests#testbmap_testSimple -w com.bentley.unittest/android.test.InstrumentationTestRunner  
adb shell am instrument -w com.bentley.unittest/android.test.InstrumentationTestRunner 

goto :xit


REM --------------------- clean ---------------------------
:clean
    call bb -sBeTestAndroid reb CopyTo* -c
    rd/s %ANDROID_TEST_TARGET_PROJECT_ROOT%bin
    rd/s %ANDROID_TEST_TEST_PROJECT_ROOT%bin
    rd/s %ANDROID_TEST_TARGET_PROJECT_ROOT%\libs
    rd/s %ANDROID_TEST_TARGET_PROJECT_ROOT%\obj
    rd/s %ANDROID_TEST_TEST_PROJECT_ROOT%\src
    goto :xit

REM --------------------- errors ---------------------------
:prepProject
    REM ERROR - %ANDROID_TEST_TARGET_PROJECT_ROOT%\build.xml and/or %ANDROID_TEST_TEST_PROJECT_ROOT%\build.xml not found.
    REM Run the following commands to prep your projects:
    REM android create project --package com.bentley.test --activity TestActivity     --target 1 --path %ANDROID_TEST_TARGET_PROJECT_ROOT%/Test
    REM android create project --package com.bentley.test --activity UnitTestActivity --target 2 --path %ANDROID_TEST_TEST_PROJECT_ROOT%/TestTest
    REM android update project --path %ANDROID_TEST_TARGET_PROJECT_ROOT% 
    REM android update project --path %ANDROID_TEST_TEST_PROJECT_ROOT%

:mustRunBB
    REM ERROR - No shared objects found in %ANDROID_TEST_TARGET_PROJECT_ROOT%\libs\armeabi-v7a\lib. Run bb to build the native libraries before calling this.
    goto :xit

:xit