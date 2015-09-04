rem Usage: InstallAll action destination platform
rem   action:
rem      /I  --> Install
rem      /x  --> Uninstall
rem   destination: SDK install directory
rem   platform: x86 or x64

set PointoolsAction=%1
set PointoolsVortexDest=%2
set PointoolsPlatform=%3
msiexec %PointoolsAction% PointoolsVortexAPI_VS2012\PointoolsVortexAPI_Runtime_VS2012%PointoolsPlatform%.msi INSTALLDIR=%PointoolsVortexDest%\bin\VS2012\%PointoolsPlatform%
msiexec %PointoolsAction% PointoolsVortexAPI_VS2012\PointoolsVortexAPI_API_VS2012%PointoolsPlatform%.msi INSTALLDIR=%PointoolsVortexDest%
msiexec %PointoolsAction% PointoolsIO_VS2012\PointoolsIO_PODFormats_Runtime_VS2012%PointoolsPlatform%.msi INSTALLDIR=%PointoolsVortexDest%\bin\VS2012\%PointoolsPlatform%
msiexec %PointoolsAction% PointoolsIO_VS2012\PointoolsIO_PODFormats_API_VS2012%PointoolsPlatform%.msi INSTALLDIR=%PointoolsVortexDest%
msiexec %PointoolsAction% PointoolsIO_VS2012\PointoolsIO_PODWriter_Runtime_VS2012%PointoolsPlatform%.msi INSTALLDIR=%PointoolsVortexDest%\bin\VS2012\%PointoolsPlatform%
msiexec %PointoolsAction% PointoolsIO_VS2012\PointoolsIO_PODWriter_API_VS2012%PointoolsPlatform%.msi INSTALLDIR=%PointoolsVortexDest%
