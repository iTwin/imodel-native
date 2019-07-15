@rem *************************************************************************************************************

@rem Description : Set up Vortex DLL binaries ready for examples to execute.

@rem Author      : Lee Bull

@rem Date        : Jan 2014

@rem *************************************************************************************************************

@echo off

SET folder_x86=%outroot%\Winx86\PointoolsExamples
SET folder_x64=%outroot%\Winx64\PointoolsExamples

if exist "%Pointools_Vortex_Bin_x86_VS2005%\PointoolsVortexAPI.dll" xcopy /C /Y "%Pointools_Vortex_Bin_x86_VS2005%\PointoolsVortexAPI.dll"   				"%folder_x86%\vc80\Release\"
if exist "%Pointools_Vortex_Bin_x86_VS2005%\PointoolsVortexAPI.dll" xcopy /C /Y "%Pointools_Vortex_Bin_x86_VS2005%\PointoolsVortexAPI.dll"   				"%folder_x86%\vc80\Debug\"

if exist "%Pointools_Vortex_Bin_x64_VS2005%\PointoolsVortexAPI.dll" xcopy /C /Y "%Pointools_Vortex_Bin_x64_VS2005%\PointoolsVortexAPI.dll"   				"%folder_x64%\vc80\Release\"
if exist "%Pointools_Vortex_Bin_x64_VS2005%\PointoolsVortexAPI.dll" xcopy /C /Y "%Pointools_Vortex_Bin_x64_VS2005%\PointoolsVortexAPI.dll"   				"%folder_x64%\vc80\Debug\"

if exist "%Pointools_FeatureExtract_Bin_x86_VS2005%\VortexFeatureExtract.dll" xcopy /C /Y "%Pointools_FeatureExtract_Bin_x86_VS2005%\VortexFeatureExtract.dll"   	"%folder_x86%\vc80\Release\"
if exist "%Pointools_FeatureExtract_Bin_x86_VS2005%\VortexFeatureExtract.dll" xcopy /C /Y "%Pointools_FeatureExtract_Bin_x86_VS2005%\VortexFeatureExtract.dll"   	"%folder_x86%\vc80\Debug\"

if exist "%Pointools_FeatureExtract_Bin_x64_VS2005%\VortexFeatureExtract.dll" xcopy /C /Y "%Pointools_FeatureExtract_Bin_x64_VS2005%\VortexFeatureExtract.dll"   	"%folder_x64%\vc80\Release\"
if exist "%Pointools_FeatureExtract_Bin_x64_VS2005%\VortexFeatureExtract.dll" xcopy /C /Y "%Pointools_FeatureExtract_Bin_x64_VS2005%\VortexFeatureExtract.dll"   	"%folder_x64%\vc80\Debug\"

if exist "%Pointools_Vortex_Bin_x86_VS2012%\PointoolsVortexAPI.dll" xcopy /C /Y "%Pointools_Vortex_Bin_x86_VS2012%\PointoolsVortexAPI.dll"   				"%folder_x86%\vc110\Release\"
if exist "%Pointools_Vortex_Bin_x86_VS2012%\PointoolsVortexAPI.dll" xcopy /C /Y "%Pointools_Vortex_Bin_x86_VS2012%\PointoolsVortexAPI.dll"   				"%folder_x86%\vc110\Debug\"

if exist "%Pointools_Vortex_Bin_x64_VS2012%\PointoolsVortexAPI.dll" xcopy /C /Y "%Pointools_Vortex_Bin_x64_VS2012%\PointoolsVortexAPI.dll"   				"%folder_x64%\vc110\Release\"
if exist "%Pointools_Vortex_Bin_x64_VS2012%\PointoolsVortexAPI.dll" xcopy /C /Y "%Pointools_Vortex_Bin_x64_VS2012%\PointoolsVortexAPI.dll"   				"%folder_x64%\vc110\Debug\"

if exist "%Pointools_FeatureExtract_Bin_x86_VS2012%\VortexFeatureExtract.dll" xcopy /C /Y "%Pointools_FeatureExtract_Bin_x86_VS2012%\VortexFeatureExtract.dll"   	"%folder_x86%\vc110\Release\"
if exist "%Pointools_FeatureExtract_Bin_x86_VS2012%\VortexFeatureExtract.dll" xcopy /C /Y "%Pointools_FeatureExtract_Bin_x86_VS2012%\VortexFeatureExtract.dll"   	"%folder_x86%\vc110\Debug\"

if exist "%Pointools_FeatureExtract_Bin_x64_VS2012%\VortexFeatureExtract.dll" xcopy /C /Y "%Pointools_FeatureExtract_Bin_x64_VS2012%\VortexFeatureExtract.dll"   	"%folder_x64%\vc110\Release\"
if exist "%Pointools_FeatureExtract_Bin_x64_VS2012%\VortexFeatureExtract.dll" xcopy /C /Y "%Pointools_FeatureExtract_Bin_x64_VS2012%\VortexFeatureExtract.dll"   	"%folder_x64%\vc110\Debug\"

xcopy /C /Y %Pointools_Libs_Bin_x86_VS2005%\Glut32.dll		"%folder_x86%\vc80\Debug\"
xcopy /C /Y %Pointools_Libs_Bin_x86_VS2005%\Glut32.dll		"%folder_x86%\vc80\Release\"

xcopy /C /Y %Pointools_Libs_Bin_x64_VS2005%\Glut64.dll		"%folder_x64%\vc80\Debug\"
xcopy /C /Y %Pointools_Libs_Bin_x64_VS2005%\Glut64.dll		"%folder_x64%\vc80\Release\"

xcopy /C /Y %Pointools_Libs_Bin_x86_VS2005%\freeglut.dll	"%folder_x86%\vc80\Release\"
xcopy /C /Y %Pointools_Libs_Bin_x86_VS2005%\freeglutd.dll	"%folder_x86%\vc80\Debug\"

xcopy /C /Y %Pointools_Libs_Bin_x64_VS2005%\freeglut.dll	"%folder_x64%\vc80\Release\"
xcopy /C /Y %Pointools_Libs_Bin_x64_VS2005%\freeglutd.dll	"%folder_x64%\vc80\Debug\"

xcopy /C /Y %Pointools_Libs_Bin_x86_VS2012%\freeglut.dll	"%folder_x86%\vc110\Release\"
xcopy /C /Y %Pointools_Libs_Bin_x86_VS2012%\freeglut.dll	"%folder_x86%\vc110\Debug\"

xcopy /C /Y %Pointools_Libs_Bin_x64_VS2012%\freeglut.dll	"%folder_x64%\vc110\Release\"
xcopy /C /Y %Pointools_Libs_Bin_x64_VS2012%\freeglut.dll	"%folder_x64%\vc110\Debug\"
