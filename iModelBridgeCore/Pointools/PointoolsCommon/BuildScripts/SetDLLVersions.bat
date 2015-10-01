@echo off

@REM -----------------------------------------------------------------------------------------------------------
@REM (c)2014 Bentley Systems Ltd.
@REM 
@REM Description: Version stamps PointoolsLibs distribution.
@REM              This script must be run on a distribution by hand before placing libs in repository.
@REM              All versions must be kept up to date.
@REM
@REM Author     : Lee Bull
@REM
@REM -----------------------------------------------------------------------------------------------------------

@REM -----------------------------------------------------------------------------------------------------------
@REM Initialize
@REM -----------------------------------------------------------------------------------------------------------

set InjectVersion=%srcroot%\bsitools\winX86\InjectVersion.exe

cd %srcroot%


@REM -----------------------------------------------------------------------------------------------------------
@REM Riegl
@REM -----------------------------------------------------------------------------------------------------------

%InjectVersion% "%srcroot%\Bin\Riegl\rivlib-2_0_2-x86-windows-vc110\lib\scanifc-mt.dll" -n2.0.2.0 -p2.0.2.0.0
if %ERRORLEVEL% == 1 goto ERROR
@REM %InjectVersion% "%srcroot%\Bin\Riegl\rivlib-2_0_2-x86_64-windows-vc110\lib\scanifc-mt.dll" -n2.0.2.0 -p2.0.2.0.0  NOTE: THIS WILL BREAK THE DLL IF IT IS DONE !!

@REM -----------------------------------------------------------------------------------------------------------
@REM TopCon
@REM -----------------------------------------------------------------------------------------------------------

@REM %InjectVersion% "%SrcRoot%\Bin\topcon\TopconCodec\x86\Release\TopconCodec.dll" -n1.0.0.0 -p1.0.0.0
if %ERRORLEVEL% == 1 goto ERROR
@REM %InjectVersion% "%SrcRoot%\Bin\topcon\TopconCodec\x64\Release\TopconCodec.dll" -n1.0.0.0 -p1.0.0.0
if %ERRORLEVEL% == 1 goto ERROR

@REM -----------------------------------------------------------------------------------------------------------
@REM LIBXML
@REM -----------------------------------------------------------------------------------------------------------

@REM %InjectVersion% "gnome\xml\win32\bin.msvc\libxml2.dll" -n2.6.7.0 -p2.6.7.0
if %ERRORLEVEL% == 1 goto ERROR
@REM %InjectVersion% "gnome\libxml2\2.9.1\vc11\x64\bin\libxml2.dll" -n2.9.1.0 -p2.9.1.0
if %ERRORLEVEL% == 1 goto ERROR

@REM -----------------------------------------------------------------------------------------------------------
@REM FARO
@REM -----------------------------------------------------------------------------------------------------------

%InjectVersion% "%SrcRoot%\Bin\Faro\5.4.3.10\VS2012\x86\jsoncpp.dll"  -n1.0.0.0 -p1.0.0.0
if %ERRORLEVEL% == 1 goto ERROR
%InjectVersion% "%SrcRoot%\Bin\Faro\5.4.3.10\VS2012\x86\MSPCoordinateConversionService.dll"  -n1.0.0.0 -p1.0.0.0
if %ERRORLEVEL% == 1 goto ERROR
%InjectVersion% "%SrcRoot%\Bin\Faro\5.4.3.10\VS2012\x86\geotifflib.dll"  -n1.0.0.0 -p1.0.0.0
if %ERRORLEVEL% == 1 goto ERROR
%InjectVersion% "%SrcRoot%\Bin\Faro\5.4.3.10\VS2012\x86\opencv_calib3d242.dll" -n1.0.0.0 -p1.0.0.0
if %ERRORLEVEL% == 1 goto ERROR
%InjectVersion% "%SrcRoot%\Bin\Faro\5.4.3.10\VS2012\x86\opencv_core242.dll" -n1.0.0.0 -p1.0.0.0
if %ERRORLEVEL% == 1 goto ERROR
%InjectVersion% "%SrcRoot%\Bin\Faro\5.4.3.10\VS2012\x86\opencv_features2d242.dll" -n1.0.0.0 -p1.0.0.0
if %ERRORLEVEL% == 1 goto ERROR
%InjectVersion% "%SrcRoot%\Bin\Faro\5.4.3.10\VS2012\x86\opencv_flann242.dll" -n1.0.0.0 -p1.0.0.0
if %ERRORLEVEL% == 1 goto ERROR
%InjectVersion% "%SrcRoot%\Bin\Faro\5.4.3.10\VS2012\x86\opencv_highgui242.dll" -n1.0.0.0 -p1.0.0.0
if %ERRORLEVEL% == 1 goto ERROR
%InjectVersion% "%SrcRoot%\Bin\Faro\5.4.3.10\VS2012\x86\opencv_imgproc242.dll" -n1.0.0.0 -p1.0.0.0
if %ERRORLEVEL% == 1 goto ERROR


%InjectVersion% "%SrcRoot%\Bin\Faro\5.4.3.10\VS2012\x64\jsoncpp.dll"  -n1.0.0.0 -p1.0.0.0
if %ERRORLEVEL% == 1 goto ERROR
%InjectVersion% "%SrcRoot%\Bin\Faro\5.4.3.10\VS2012\x64\MSPCoordinateConversionService.dll"  -n1.0.0.0 -p1.0.0.0
if %ERRORLEVEL% == 1 goto ERROR
%InjectVersion% "%SrcRoot%\Bin\Faro\5.4.3.10\VS2012\x64\geotifflib.dll"  -n1.0.0.0 -p1.0.0.0
if %ERRORLEVEL% == 1 goto ERROR
%InjectVersion% "%SrcRoot%\Bin\Faro\5.4.3.10\VS2012\x64\opencv_calib3d242.dll" -n1.0.0.0 -p1.0.0.0
if %ERRORLEVEL% == 1 goto ERROR
%InjectVersion% "%SrcRoot%\Bin\Faro\5.4.3.10\VS2012\x64\opencv_core242.dll" -n1.0.0.0 -p1.0.0.0
if %ERRORLEVEL% == 1 goto ERROR
%InjectVersion% "%SrcRoot%\Bin\Faro\5.4.3.10\VS2012\x64\opencv_features2d242.dll" -n1.0.0.0 -p1.0.0.0
if %ERRORLEVEL% == 1 goto ERROR
%InjectVersion% "%SrcRoot%\Bin\Faro\5.4.3.10\VS2012\x64\opencv_flann242.dll" -n1.0.0.0 -p1.0.0.0
if %ERRORLEVEL% == 1 goto ERROR
%InjectVersion% "%SrcRoot%\Bin\Faro\5.4.3.10\VS2012\x64\opencv_highgui242.dll" -n1.0.0.0 -p1.0.0.0
if %ERRORLEVEL% == 1 goto ERROR
%InjectVersion% "%SrcRoot%\Bin\Faro\5.4.3.10\VS2012\x64\opencv_imgproc242.dll" -n1.0.0.0 -p1.0.0.0
if %ERRORLEVEL% == 1 goto ERROR
%InjectVersion% "%SrcRoot%\Bin\Faro\5.4.3.10\VS2012\x64\opencv_video242.dll" -n1.0.0.0 -p1.0.0.0
if %ERRORLEVEL% == 1 goto ERROR

exit /b 0


ERROR:

exit /b %ERRORLEVEL%