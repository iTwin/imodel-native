@echo off
set BECONNECT_ROOT_DIR=D:\DevApps\NuGet
rem Should have an environment variable named BECONNECT_ROOT_DIR set to root of Bentley CONNECT for this to work
if "%BECONNECT_ROOT_DIR%" == "" goto MissingRootDir
if "%1" == "" goto MissingPackage
%BECONNECT_ROOT_DIR%\NuGet.exe push %1 -s https://nuget.bentley.com/nuget/Default/ -apiKey b4e5da2b-20d4-4018-9ec3-9320b1bca9a2
goto end
:MissingRootDir
echo You need to have the environment variable BECONNECT_ROOT_DIR defined and set to the root of the Bentley CONNECT directory
goto end
:MissingPackage
echo You need to specify a NuGet package to push to the internal Bentley NuGet server
:end
