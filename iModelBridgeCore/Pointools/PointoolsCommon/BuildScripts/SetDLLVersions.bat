@REM -----------------------------------------------------------------------------------------------------------
@REM (c)2014 Bentley Systems Ltd.
@REM 
@REM Product    : PointoolsIO 1.6.0.8
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

cd %srcroot%\Pointools\PointoolsLibs

@REM -----------------------------------------------------------------------------------------------------------
@REM LasZIP
@REM -----------------------------------------------------------------------------------------------------------

%InjectVersion% "laszip\laszip-2.1.0\bin\vc11\x64\laszip.dll" -n2.1.0.0 -p2.1.0.0

%InjectVersion% "laszip\laszip-2.1.0\bin\vc8\x86\release\laszip.dll" -n2.1.0.0 -p2.1.0.0
%InjectVersion% "laszip\laszip-2.1.0\bin\vc8\x86\debug\laszip.dll" -n2.1.0.0 -p2.1.0.0

@REM -----------------------------------------------------------------------------------------------------------
@REM Riegl
@REM -----------------------------------------------------------------------------------------------------------

%InjectVersion% "riegl\rivlib-1.32\x86-windows-vc90\lib\scanifc-mt.dll" -n2.1.1.7 -p2.1.1.7
@REM %InjectVersion% "riegl\rivlib-1.32\x86_64-windows-vc110\lib\scanifc-mt.dll" -n2.1.1.7 -p2.1.1.7 NOTE: THIS WILL BREAK THE DLL IF IT IS DONE !!

@REM -----------------------------------------------------------------------------------------------------------
@REM TopCon
@REM -----------------------------------------------------------------------------------------------------------

%InjectVersion% "topcon\TopconCodec\x86\Release\TopconCodec.dll" -n1.0.0.0 -p1.0.0.0
%InjectVersion% "topcon\TopconCodec\x64\Release\TopconCodec.dll" -n1.0.0.0 -p1.0.0.0

@REM -----------------------------------------------------------------------------------------------------------
@REM LIBXML
@REM -----------------------------------------------------------------------------------------------------------

%InjectVersion% "gnome\xml\win32\bin.msvc\libxml2.dll" -n2.6.7.0 -p2.6.7.0
%InjectVersion% "gnome\libxml2\2.9.1\vc11\x64\bin\libxml2.dll" -n2.9.1.0 -p2.9.1.0

@REM -----------------------------------------------------------------------------------------------------------
@REM FARO
@REM -----------------------------------------------------------------------------------------------------------

%InjectVersion% "faro\5.2.0\x86\geotifflib.dll" -n1.0.0.0 -p1.0.0.0
%InjectVersion% "faro\5.2.0\x64\geotifflib.dll" -n1.0.0.0 -p1.0.0.0
%InjectVersion% "faro\5.2.0\x86\jsoncpp.dll" -n1.0.0.0 -p1.0.0.0  
%InjectVersion% "faro\5.2.0\x64\jsoncpp.dll" -n1.0.0.0 -p1.0.0.0  
%InjectVersion% "faro\5.2.0\x86\MSPCoordinateConversionService.dll" -n1.0.0.0 -p1.0.0.0  
%InjectVersion% "faro\5.2.0\x64\MSPCoordinateConversionService.dll" -n1.0.0.0 -p1.0.0.0  
%InjectVersion% "faro\5.2.0\x86\opencv_calib3d242.dll" -n2.4.2.0 -p2.4.2.0	 
%InjectVersion% "faro\5.2.0\x64\opencv_calib3d242.dll" -n2.4.2.0 -p2.4.2.0	 
%InjectVersion% "faro\5.2.0\x86\opencv_core242.dll" -n2.4.2.0 -p2.4.2.0 	 
%InjectVersion% "faro\5.2.0\x64\opencv_core242.dll" -n2.4.2.0 -p2.4.2.0 	 
%InjectVersion% "faro\5.2.0\x86\opencv_features2d242.dll" -n2.4.2.0 -p2.4.2.0 	 
%InjectVersion% "faro\5.2.0\x64\opencv_features2d242.dll" -n2.4.2.0 -p2.4.2.0 	 
%InjectVersion% "faro\5.2.0\x86\opencv_flann242.dll" -n2.4.2.0 -p2.4.2.0 	 
%InjectVersion% "faro\5.2.0\x64\opencv_flann242.dll" -n2.4.2.0 -p2.4.2.0 	 
%InjectVersion% "faro\5.2.0\x86\opencv_imgproc242.dll" -n2.4.2.0 -p2.4.2.0 	 
%InjectVersion% "faro\5.2.0\x64\opencv_imgproc242.dll" -n2.4.2.0 -p2.4.2.0 	 

