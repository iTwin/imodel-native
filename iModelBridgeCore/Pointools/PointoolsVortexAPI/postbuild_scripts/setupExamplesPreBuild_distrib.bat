@rem *************************************************************************************************************
@rem Description : Set up examples environment variables before building against a Vortex distribution
@rem *************************************************************************************************************

@echo off

echo.
echo ***********************************************************************
echo *
echo * Setting up environment variables for building Vortex example projects

setx Pointools_Examples_Out ".."

@rem Libs paths for external external examples build
setx Pointools_Libs_Include_x86_VS2005 "..\include" > Nul
setx Pointools_Libs_Libs_x86_VS2005 "..\lib" > Nul
setx Pointools_Libs_Bin_x86_VS2005 "..\bin\x86\vc8" > Nul

setx Pointools_Libs_Include_x64_VS2012 "..\include" > Nul
setx Pointools_Libs_Libs_x64_VS2012 "..\lib" > Nul
setx Pointools_Libs_Bin_x64_VS2012 "..\bin\x64\vc11" > Nul

@rem Vortex paths for VS2005 for external external examples build
@setx Pointools_Vortex_Include_x86_VS2005 "..\..\VortexAPI\include" > Nul
@setx Pointools_Vortex_Libs_x86_VS2005 "..\..\..\VortexAPI\lib\vc8" > Nul
@setx Pointools_Vortex_Bin_x86_VS2005 "..\..\..\VortexAPI\distrib\vc8" > Nul

@setx Pointools_FeatureExtract_Include_x86_VS2005 "..\..\..\VortexExtensions\include" > Nul
@setx Pointools_FeatureExtract_Libs_x86_VS2005 "..\..\..\VortexExtensions\lib\vc8" > Nul
@setx Pointools_FeatureExtract_Bin_x86_VS2005 "..\..\..\VortexExtensions\bin\vc8" > Nul

@setx Pointools_IO_Include_x86_VS2005 "..\..\..\PointoolsIO\include" > Nul
@setx Pointools_IO_Libs_x86_VS2005 "..\..\..\PointoolsIO\lib\x64" > Nul
@setx Pointools_IO_Bin_x86_VS2005 "..\..\..\PointoolsIO\bin\x64" > Nul

@rem Vortex paths for VS2012 for external external examples build
@setx Pointools_Vortex_Include_x64_VS2012 "..\..\..\VortexAPI\include" > Nul
@setx Pointools_Vortex_Libs_x64_VS2012 "..\..\..\VortexAPI\lib\vc11" > Nul
@setx Pointools_Vortex_Bin_x64_VS2012 "..\..\..\VortexAPI\distrib\vc11" > Nul

@setx Pointools_FeatureExtract_Include_x64_VS2012 "..\..\..\VortexExtensions\include" > Nul
@setx Pointools_FeatureExtract_Libs_x64_VS2012 "..\..\..\VortexExtensions\lib\vc11" > Nul
@setx Pointools_FeatureExtract_Bin_x64_VS2012 "..\..\..\VortexExtensions\bin\vc11" > Nul

@setx Pointools_IO_Include_x64_VS2012 "..\..\..\PointoolsIO\include" > Nul
@setx Pointools_IO_Libs_x64_VS2012 "..\..\..\PointoolsIO\lib\x64" > Nul
@setx Pointools_IO_Bin_x64_VS2012 "..\..\..\PointoolsIO\bin\x64" > Nul


echo *
echo ***********************************************************************
echo.
echo.

