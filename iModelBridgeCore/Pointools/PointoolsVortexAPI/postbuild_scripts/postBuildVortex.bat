set PTIO=%POINTOOLS%\PointoolsIO\workingbuild\bin

if exist %PointoolsVortex%\bin\vc8\PointoolsVortexAPI.dll (xcopy %PointoolsVortex%\bin\vc8\PointoolsVortexAPI.dll %PointoolsVortex%\examples\bin\x86\ /Y /Q)
if exist %PointoolsVortex%\bin\vc8\PointoolsVortexAPId.dll (xcopy %PointoolsVortex%\bin\vc8\PointoolsVortexAPId.dll %PointoolsVortex%\examples\bin\x86\ /Y /Q)
if exist %PointoolsVortex%\bin\vc8\PointoolsVortexAPI64.dll (xcopy %PointoolsVortex%\bin\vc8\PointoolsVortexAPI64.dll %PointoolsVortex%\examples\bin\x64\ /Y /Q)
if exist %PointoolsVortex%\bin\vc8\PointoolsVortexAPI64d.dll (xcopy %PointoolsVortex%\bin\vc8\PointoolsVortexAPI64d.dll %PointoolsVortex%\examples\bin\x64\ /Y /Q)

if exist %PointoolsVortex%\bin\vc8\PointoolsVortexAPI.dll (xcopy %PointoolsVortex%\bin\vc8\PointoolsVortexAPI.dll %PTIO%\PointoolsVortexAPI.dll /Y /Q)
if exist %PointoolsVortex%\bin\vc8\PointoolsVortexAPId.dll (xcopy %PointoolsVortex%\bin\vc8\PointoolsVortexAPId.dll %PTIO%\PointoolsVortexAPId.dll /Y /Q)
if exist %PointoolsVortex%\bin\vc8\PointoolsVortexAPI64.dll (xcopy %PointoolsVortex%\bin\vc8\PointoolsVortexAPI64.dll %PTIO%\PointoolsVortexAPI64.dll /Y /Q)
if exist %PointoolsVortex%\bin\vc8\PointoolsVortexAPI64d.dll (xcopy %PointoolsVortex%\bin\vc8\PointoolsVortexAPI64d.dll %PTIO%\PointoolsVortexAPI64d.dll /Y /Q)

xcopy %PointoolsVortex%\include\ptapi\PointoolsVortexAPI_import.h %PointoolsVortex%\examples\include /Y /Q
xcopy %PointoolsVortex%\src\ptapi\PointoolsVortexAPI_import.cpp %PointoolsVortex%\examples\src /Y /Q

if exist %PointoolsVortex%\bin\vc8\PointoolsVortexAPI.dll (xcopy %PointoolsVortex%\bin\vc8\PointoolsVortexAPI.dll %PointoolsVortex%\support\QVortex\Debug\ /Y /Q)
if exist %PointoolsVortex%\bin\vc8\PointoolsVortexAPI.dll (xcopy %PointoolsVortex%\bin\vc8\PointoolsVortexAPI.dll %PointoolsVortex%\support\QVortex\Release\ /Y /Q)
if exist %PointoolsVortex%\bin\vc8\PointoolsVortexAPId.dll (xcopy %PointoolsVortex%\bin\vc8\PointoolsVortexAPId.dll %PointoolsVortex%\support\QVortex\Debug\ /Y /Q)
if exist %PointoolsVortex%\bin\vc8\PointoolsVortexAPId.dll (xcopy %PointoolsVortex%\bin\vc8\PointoolsVortexAPId.dll %PointoolsVortex%\support\QVortex\Release\ /Y /Q)

if exist %PointoolsVortex%\bin\vc8\PointoolsVortexAPId.dll (xcopy %PointoolsVortex%\bin\vc8\PointoolsVortexAPId.dll "C:\Program Files\Tecnomatix_12.1.1\eMPower\PointoolsVortexAPI.dll" /Y /Q)

xcopy %PointoolsVortex%\include\ptapi\PointoolsVortexAPI_import.h %PointoolsVortex%\support\QVortex\QVortex\PointTools\inc\ /Y /Q
xcopy %PointoolsVortex%\src\ptapi\PointoolsVortexAPI_import.cpp %PointoolsVortex%\support\QVortex\QVortex\PointTools\src\ /Y /Q

if exist %PointoolsVortex%\bin\vc8\PointoolsVortexAPI.dll (xcopy %PointoolsVortex%\bin\vc8\PointoolsVortexAPI.dll %Pointools%\Pointools4Rhino\bin\Plugin\ /Y /Q)



