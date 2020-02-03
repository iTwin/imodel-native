@rem *************************************************************************************************************

@rem Description : Set up examples so they will build against the Bentley Build Vortex and Libraries

@rem Author      : Lee Bull

@rem Date        : Jan 2014

@rem *************************************************************************************************************

@echo off

if "%~1"=="" goto :usage

goto :setupLibraries


:usage
echo.
echo SetupExamples Source
echo.
echo Where:
echo.
echo Source = bb (Bentley Build)
echo Source = vs (Visual Studio)
echo.
echo Description : Setups up env variables for Pointools Vortex Examples build
echo Note        : Set PointoolsVortexBase env variable first, e.g. C:\Topaz
echo.

goto :end


:setupLibraries

if %1==bb set SetupExamplesMode=BB
if %1==BB set SetupExamplesMode=BB
if %1==vs set SetupExamplesMode=VS
if %1==VS set SetupExamplesMode=VS
if %1==c goto :Clear
if %1==C goto :Clear

echo.
echo *********************************************************************
echo *
echo * Setting up Pointools Vortex Examples Build Environment:
echo *
if %SetupExamplesMode%==BB echo * Vortex Source    : Bentley Build
if %SetupExamplesMode%==VS echo * Vortex Source    : Visual Studio Build
echo *


@rem *************************************************************************************************************
@rem Set up env variables for building against Bentley Build Library parts
@rem *************************************************************************************************************

setx Pointools_Libs_Include_x86_VS2005 %PointoolsVortexBase%\out\Winx86\BuildContexts\PointoolsLibs_VS2005\VendorAPI > Nul
setx Pointools_Libs_Libs_x86_VS2005 %PointoolsVortexBase%\out\Winx86\BuildContexts\PointoolsLibs_VS2005\Delivery > Nul
setx Pointools_Libs_Bin_x86_VS2005 %PointoolsVortexBase%\out\Winx86\BuildContexts\PointoolsLibs_VS2005\Delivery > Nul

setx Pointools_Libs_Include_x64_VS2005 %PointoolsVortexBase%\out\Winx64\BuildContexts\PointoolsLibs_VS2005\VendorAPI > Nul
setx Pointools_Libs_Libs_x64_VS2005 %PointoolsVortexBase%\out\Winx64\BuildContexts\PointoolsLibs_VS2005\Delivery > Nul
setx Pointools_Libs_Bin_x64_VS2005 %PointoolsVortexBase%\out\Winx64\BuildContexts\PointoolsLibs_VS2005\Delivery > Nul

setx Pointools_Libs_Include_x86_VS2012 %PointoolsVortexBase%\out\Winx86\BuildContexts\PointoolsLibs_VS2012\VendorAPI > Nul
setx Pointools_Libs_Libs_x86_VS2012 %PointoolsVortexBase%\out\Winx86\BuildContexts\PointoolsLibs_VS2012\Delivery > Nul
setx Pointools_Libs_Bin_x86_VS2012 %PointoolsVortexBase%\out\Winx86\BuildContexts\PointoolsLibs_VS2012\Delivery > Nul

setx Pointools_Libs_Include_x64_VS2012 %PointoolsVortexBase%\out\Winx64\BuildContexts\PointoolsLibs_VS2012\VendorAPI > Nul
setx Pointools_Libs_Libs_x64_VS2012 %PointoolsVortexBase%\out\Winx64\BuildContexts\PointoolsLibs_VS2012\Delivery > Nul
setx Pointools_Libs_Bin_x64_VS2012 %PointoolsVortexBase%\out\Winx64\BuildContexts\PointoolsLibs_VS2012\Delivery > Nul

echo *    Libraries     : Bentley Build


if %SetupExamplesMode%==BB goto :setupBB
if %SetupExamplesMode%==VS goto :setupVS

goto :finish


@rem *************************************************************************************************************
@rem Set up env variables for building VS2005 Examples based on Bentley Build
@rem *************************************************************************************************************

:setupBB

@setx Pointools_Vortex_Include_x86_VS2005 %PointoolsVortexBase%\out\Winx86\BuildContexts\PointoolsVortexAPI_VS2005\VendorAPI\PointoolsVortexAPI_DLL > Nul
@setx Pointools_Vortex_Libs_x86_VS2005 %PointoolsVortexBase%\out\Winx86\BuildContexts\PointoolsVortexAPI_VS2005\Delivery > Nul
@setx Pointools_Vortex_Bin_x86_VS2005 %PointoolsVortexBase%\out\Winx86\BuildContexts\PointoolsVortexAPI_VS2005\Delivery > Nul

@setx Pointools_Vortex_Include_x64_VS2005 %PointoolsVortexBase%\out\Winx64\BuildContexts\PointoolsVortexAPI_VS2005\VendorAPI\PointoolsVortexAPI_DLL > Nul
@setx Pointools_Vortex_Libs_x64_VS2005 %PointoolsVortexBase%\out\Winx64\BuildContexts\PointoolsVortexAPI_VS2005\Delivery > Nul
@setx Pointools_Vortex_Bin_x64_VS2005 %PointoolsVortexBase%\out\Winx64\BuildContexts\PointoolsVortexAPI_VS2005\Delivery > Nul

@setx Pointools_FeatureExtract_Include_x86_VS2005 %PointoolsVortexBase%\out\Winx86\BuildContexts\PointoolsVortexAPI_VS2005\VendorAPI\PointoolsVortexAPI_FeatureExtract_DLL > Nul
@setx Pointools_FeatureExtract_Libs_x86_VS2005 %PointoolsVortexBase%\out\Winx86\BuildContexts\PointoolsVortexAPI_VS2005\Delivery > Nul
@setx Pointools_FeatureExtract_Bin_x86_VS2005 %PointoolsVortexBase%\out\Winx86\BuildContexts\PointoolsVortexAPI_VS2005\Delivery > Nul

@setx Pointools_FeatureExtract_Include_x64_VS2005 %PointoolsVortexBase%\out\Winx64\BuildContexts\PointoolsVortexAPI_VS2005\VendorAPI\PointoolsVortexAPI_FeatureExtract_DLL > Nul
@setx Pointools_FeatureExtract_Libs_x64_VS2005 %PointoolsVortexBase%\out\Winx64\BuildContexts\PointoolsVortexAPI_VS2005\Delivery > Nul
@setx Pointools_FeatureExtract_Bin_x64_VS2005 %PointoolsVortexBase%\out\Winx64\BuildContexts\PointoolsVortexAPI_VS2005\Delivery > Nul

@setx Pointools_IO_Include_x64_VS2005 %PointoolsVortexBase%\out\Winx64\BuildContexts\PointoolsIO_VS2005\VendorAPI\PointoolsIO_PODFormats_DLL > Nul
@setx Pointools_IO_Libs_x64_VS2005 %PointoolsVortexBase%\out\Winx64\BuildContexts\PointoolsIO_VS2005\Delivery > Nul
@setx Pointools_IO_Bin_x64_VS2005 %PointoolsVortexBase%\out\Winx64\BuildContexts\PointoolsIO_VS2005\Delivery > Nul


@rem *************************************************************************************************************
@rem Set up env variables for building VS2012 Examples based on Bentley Build
@rem *************************************************************************************************************


@setx Pointools_Vortex_Include_x86_VS2012 %PointoolsVortexBase%\out\Winx86\BuildContexts\PointoolsVortexAPI_VS2012\VendorAPI\PointoolsVortexAPI_DLL > Nul
@setx Pointools_Vortex_Libs_x86_VS2012 %PointoolsVortexBase%\out\Winx86\BuildContexts\PointoolsVortexAPI_VS2012\Delivery > Nul
@setx Pointools_Vortex_Bin_x86_VS2012 %PointoolsVortexBase%\out\Winx86\BuildContexts\PointoolsVortexAPI_VS2012\Delivery > Nul

@setx Pointools_Vortex_Include_x64_VS2012 %PointoolsVortexBase%\out\Winx64\BuildContexts\PointoolsVortexAPI_VS2012\VendorAPI\PointoolsVortexAPI_DLL > Nul
@setx Pointools_Vortex_Libs_x64_VS2012 %PointoolsVortexBase%\out\Winx64\BuildContexts\PointoolsVortexAPI_VS2012\Delivery > Nul
@setx Pointools_Vortex_Bin_x64_VS2012 %PointoolsVortexBase%\out\Winx64\BuildContexts\PointoolsVortexAPI_VS2012\Delivery > Nul

@setx Pointools_FeatureExtract_Include_x86_VS2012 %PointoolsVortexBase%\out\Winx86\BuildContexts\PointoolsVortexAPI_VS2012\VendorAPI\PointoolsVortexAPI_FeatureExtract_DLL > Nul
@setx Pointools_FeatureExtract_Libs_x86_VS2012 %PointoolsVortexBase%\out\Winx86\BuildContexts\PointoolsVortexAPI_VS2012\Delivery > Nul
@setx Pointools_FeatureExtract_Bin_x86_VS2012 %PointoolsVortexBase%\out\Winx86\BuildContexts\PointoolsVortexAPI_VS2012\Delivery > Nul

@setx Pointools_FeatureExtract_Include_x64_VS2012 %PointoolsVortexBase%\out\Winx64\BuildContexts\PointoolsVortexAPI_VS2012\VendorAPI\PointoolsVortexAPI_FeatureExtract_DLL > Nul
@setx Pointools_FeatureExtract_Libs_x64_VS2012 %PointoolsVortexBase%\out\Winx64\BuildContexts\PointoolsVortexAPI_VS2012\Delivery > Nul
@setx Pointools_FeatureExtract_Bin_x64_VS2012 %PointoolsVortexBase%\out\Winx64\BuildContexts\PointoolsVortexAPI_VS2012\Delivery > Nul

@setx Pointools_IO_Include_x64_VS2012 %PointoolsVortexBase%\out\Winx64\BuildContexts\PointoolsIO_VS2012\VendorAPI\PointoolsIO_PODFormats_DLL > Nul
@setx Pointools_IO_Libs_x64_VS2012 %PointoolsVortexBase%\out\Winx64\BuildContexts\PointoolsIO_VS2012\Delivery > Nul
@setx Pointools_IO_Bin_x64_VS2012 %PointoolsVortexBase%\out\Winx64\BuildContexts\PointoolsIO_VS2012\Delivery > Nul


echo *    Vortex        : Bentley Build

goto :finish

@rem *************************************************************************************************************
@rem Set up env variables for building X64 VS2005 Examples based on Bentley Build
@rem *************************************************************************************************************

:clear

@rem Clear Libs for VS2005 and VS2012

setx Pointools_Libs_Include_x86_VS2005 "" > Nul
setx Pointools_Libs_Libs_x86_VS2005 "" > Nul
setx Pointools_Libs_Bin_x86_VS2005 "" > Nul

setx Pointools_Libs_Include_x64_VS2005 "" > Nul
setx Pointools_Libs_Libs_x64_VS2005 "" > Nul
setx Pointools_Libs_Bin_x64_VS2005 "" > Nul

setx Pointools_Libs_Include_x86_VS2012 "" > Nul
setx Pointools_Libs_Libs_x86_VS2012 "" > Nul
setx Pointools_Libs_Bin_x86_VS2012 "" > Nul

setx Pointools_Libs_Include_x64_VS2012 "" > Nul
setx Pointools_Libs_Libs_x64_VS2012 "" > Nul
setx Pointools_Libs_Bin_x64_VS2012 "" > Nul


@rem Clear VS2005

@setx Pointools_Vortex_Include_x86_VS2005 "" > Nul
@setx Pointools_Vortex_Libs_x86_VS2005 "" > Nul
@setx Pointools_Vortex_Bin_x86_VS2005 "" > Nul

@setx Pointools_Vortex_Include_x64_VS2005 "" > Nul
@setx Pointools_Vortex_Libs_x64_VS2005 "" > Nul
@setx Pointools_Vortex_Bin_x64_VS2005 "" > Nul

@setx Pointools_FeatureExtract_Include_x86_VS2005 "" > Nul
@setx Pointools_FeatureExtract_Libs_x86_VS2005 "" > Nul
@setx Pointools_FeatureExtract_Bin_x86_VS2005 "" > Nul

@setx Pointools_FeatureExtract_Include_x64_VS2005 "" > Nul
@setx Pointools_FeatureExtract_Libs_x64_VS2005 "" > Nul
@setx Pointools_FeatureExtract_Bin_x64_VS2005 "" > Nul

@setx Pointools_IO_Include_x64_VS2005 "" > Nul
@setx Pointools_IO_Libs_x64_VS2005 "" > Nul
@setx Pointools_IO_Bin_x64_VS2005 "" > Nul


@rem Clear VS2012

@setx Pointools_Vortex_Include_x86_VS2012 "" > Nul
@setx Pointools_Vortex_Libs_x86_VS2012 "" > Nul
@setx Pointools_Vortex_Bin_x86_VS2012 "" > Nul

@setx Pointools_Vortex_Include_x64_VS2012 "" > Nul
@setx Pointools_Vortex_Libs_x64_VS2012 "" > Nul
@setx Pointools_Vortex_Bin_x64_VS2012 "" > Nul

@setx Pointools_FeatureExtract_Include_x86_VS2012 "" > Nul
@setx Pointools_FeatureExtract_Libs_x86_VS2012 "" > Nul
@setx Pointools_FeatureExtract_Bin_x86_VS2012 "" > Nul

@setx Pointools_FeatureExtract_Include_x64_VS2012 "" > Nul
@setx Pointools_FeatureExtract_Libs_x64_VS2012 "" > Nul
@setx Pointools_FeatureExtract_Bin_x64_VS2012 "" > Nul

@setx Pointools_IO_Include_x64_VS2012 "" > Nul
@setx Pointools_IO_Libs_x64_VS2012 "" > Nul
@setx Pointools_IO_Bin_x64_VS2012 "" > Nul

goto :finish


@rem *************************************************************************************************************
@rem Set up env variables for building X64 VS2005 Examples based on Bentley Build
@rem *************************************************************************************************************

:setupVS

echo *    Vortex        : Visual Studio Build (Not Implemented)

goto :finish


:finish

echo *
echo *********************************************************************
echo.
echo.

:end
