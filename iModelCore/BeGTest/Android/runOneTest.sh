#!/bin/zsh

adbCmd=${ANDROID_SDK_ROOT}'/platform-tools/adb'
instrumentation='com.bentley.unittest/com.bentley.unittest.BeInstrumentationTestRunner'

# Repository
if [[ $# == 1 ]]; then
	echo 'Running all tests in repository "'$1'"'
	eval ${adbCmd}' shell am instrument -e package com.bentley.unittest.'$1' -w '$instrumentation
	
# Test Fixture
elif [[ $# == 2 ]]; then
	echo 'Running all tests in repository "'$1'" test fixture "'$2'"'
	eval ${adbCmd}' shell am instrument -e class com.bentley.unittest.'$1'.'$2' -w '$instrumentation
	
# Single test
elif [[ $# == 3 ]]; then
	echo 'Running a single test in repository "'$1'" test fixture "'$2'": "'$3'"'
	eval ${adbCmd}' shell am instrument -e class com.bentley.unittest.'$1'.'$2'#test_'$3' -w '$instrumentation
	
# Too few / many paramaters
else
	echo 'Syntax: runOneTestMac.sh <Repository> <Test Fixture> <Test Name>'
	echo 'To run all of the published DgnDb unit tests:'
	echo '    runOneTestMac.sh PublishedDgnDbUnitTests'
	echo 'To run all tests in the specific test fixture:'
	echo '    runOneTestMac.sh PublishedDgnDbUnitTests MyTestFixture'
	echo 'To run TEST_F (MyTestFixture, MyTests) in published DgnDb unit tests'
	echo '    runOneTestMac.sh PublishedDgnDbUnitTests MyTestFixture MyTests'
fi