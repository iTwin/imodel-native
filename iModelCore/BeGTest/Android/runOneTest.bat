@echo off

if .%1 == . goto :syntax
REM To see logging messages, execute the following command in another window:
REM .
REM     adb logcat TestRunner:I *:E
REM .

REM Running unit tests...
if .%1 == . goto :xit
if .%2 == . goto :runAllTestsInRepository
if .%3 == . goto :runAllTestsInClass

REM Will run a single test from specified test fixture    
    echo Running a single test from repository %1: %2 :: %3
    adb shell am instrument -e class com.bentley.unittest.%1.%2#test_%3 -w com.bentley.unittest/com.bentley.unittest.BeInstrumentationTestRunner  
    goto :xit
:runAllTestsInRepository
    echo Running all tests from repository %1
    adb shell am instrument -e package com.bentley.unittest.%1 -w com.bentley.unittest/com.bentley.unittest.BeInstrumentationTestRunner  
    goto :xit
:runAllTestsInClass
    echo Running all tests from repository %1 test fixture %2
    adb shell am instrument -e class com.bentley.unittest.%1.%2 -w com.bentley.unittest/com.bentley.unittest.BeInstrumentationTestRunner  
    goto :xit
    
:syntax
echo Syntax: runTest package test
echo .
echo To run all of the published DgnDb unittests:
echo   Example: runOneTest PublishedDgnDbUnitTests 
echo To run all tests in specific test fixture:
echo   Example: runOneTest PublishedDgnDbUnitTests CopyContextTestSharedCellRemap
echo To run TEST_F (CopyContextTestSharedCellRemap, CopyContextRemapSharedCell) in published DgnDb unittest:
echo   Example: runOneTest PublishedDgnDbUnitTests CopyContextTestSharedCellRemap CopyContextRemapSharedCell
:xit