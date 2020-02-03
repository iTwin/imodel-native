@rem *************************************************************************************************************

@rem Description : Set up Vortex DLL binaries ready for examples to execute.

@rem Author      : Lee Bull

@rem Date        : Jan 2014

@rem *************************************************************************************************************

@echo off

SET folder=%PointoolsVortexBase%\src\Pointools\PointoolsVortexAPI\examples

if exist "%Pointools_Vortex_Bin_x86_VS2005%\PointoolsVortexAPI.dll" xcopy /C /Y "%Pointools_Vortex_Bin_x86_VS2005%\PointoolsVortexAPI.dll"   				"%folder%\bin\vc80\x86\"
if exist "%Pointools_Vortex_Bin_x64_VS2005%\PointoolsVortexAPI.dll" xcopy /C /Y "%Pointools_Vortex_Bin_x64_VS2005%\PointoolsVortexAPI.dll"   				"%folder%\bin\vc80\x64\"
if exist "%Pointools_FeatureExtract_Bin_x86_VS2005%\VortexFeatureExtract.dll" xcopy /C /Y "%Pointools_FeatureExtract_Bin_x86_VS2005%\VortexFeatureExtract.dll"   	"%folder%\bin\vc80\x86\"
if exist "%Pointools_FeatureExtract_Bin_x64_VS2005%\VortexFeatureExtract.dll" xcopy /C /Y "%Pointools_FeatureExtract_Bin_x64_VS2005%\VortexFeatureExtract.dll"   	"%folder%\bin\vc80\x64\"

if exist "%Pointools_Vortex_Bin_x86_VS2012%\PointoolsVortexAPI.dll" xcopy /C /Y "%Pointools_Vortex_Bin_x86_VS2012%\PointoolsVortexAPI.dll"   				"%folder%\bin\vc110\x86\"
if exist "%Pointools_Vortex_Bin_x64_VS2012%\PointoolsVortexAPI.dll" xcopy /C /Y "%Pointools_Vortex_Bin_x64_VS2012%\PointoolsVortexAPI.dll"   				"%folder%\bin\vc110\x64\"
if exist "%Pointools_FeatureExtract_Bin_x86_VS2012%\VortexFeatureExtract.dll" xcopy /C /Y "%Pointools_FeatureExtract_Bin_x86_VS2012%\VortexFeatureExtract.dll"   	"%folder%\bin\vc110\x86\"
if exist "%Pointools_FeatureExtract_Bin_x64_VS2012%\VortexFeatureExtract.dll" xcopy /C /Y "%Pointools_FeatureExtract_Bin_x64_VS2012%\VortexFeatureExtract.dll"   	"%folder%\bin\vc110\x64\"

xcopy /C /Y %Pointools_Libs_Bin_x86_VS2005%\Glut32.dll		"%folder%\bin\vc80\x86\"
xcopy /C /Y %Pointools_Libs_Bin_x64_VS2005%\Glut64.dll		"%folder%\bin\vc80\x64\"

xcopy /C /Y %Pointools_Libs_Bin_x86_VS2005%\freeglut.dll	"%folder%\bin\vc80\x86\"
xcopy /C /Y %Pointools_Libs_Bin_x86_VS2005%\freeglutd.dll	"%folder%\bin\vc80\x86\"
xcopy /C /Y %Pointools_Libs_Bin_x64_VS2005%\freeglut.dll	"%folder%\bin\vc80\x64\"
xcopy /C /Y %Pointools_Libs_Bin_x64_VS2005%\freeglutd.dll	"%folder%\bin\vc80\x64\"

xcopy /C /Y %Pointools_Libs_Bin_x86_VS2012%\freeglut.dll	"%folder%\bin\vc110\x86\"
xcopy /C /Y %Pointools_Libs_Bin_x64_VS2012%\freeglut.dll	"%folder%\bin\vc110\x64\"
