
set PointoolsVortexAPI=%PointoolsVortexBase%\src\Pointools\PointoolsVortexAPI

set PointoolsLibs=%PointoolsVortexBase%\src\Pointools\PointoolsLibs

SET folder=%PointoolsVortexAPI%\examples

mkdir %PointoolsVortexAPI%\examples\include\ptapi

copy %PointoolsVortexAPI%\src\ptapi\PointoolsVortexAPI_import.cpp %folder%\src\
copy %PointoolsVortexAPI%\include\ptapi\PointoolsVortexAPI_import.h %PointoolsVortexAPI%\examples\include
copy %PointoolsVortexAPI%\include\ptapi\PointoolsVortexAPI_resultCodes.h %PointoolsVortexAPI%\examples\include\ptapi
copy %PointoolsVortexAPI%\include\vortexobjects\VortexObjects_import.h %PointoolsVortexAPI%\examples\include
copy %PointoolsVortexAPI%\include\vortexobjects\IClashObjectManager.h %PointoolsVortexAPI%\examples\include
copy %PointoolsVortexAPI%\include\vortexobjects\IClashObject.h %PointoolsVortexAPI%\examples\include
copy %PointoolsVortexAPI%\include\vortexobjects\IClashTree.h %PointoolsVortexAPI%\examples\include
copy %PointoolsVortexAPI%\include\vortexobjects\IClashNode.h %PointoolsVortexAPI%\examples\include

mkdir "%folder%\lib\vc80\x86"
mkdir "%folder%\lib\vc80\x64"
mkdir "%folder%\lib\vc90\x86"
mkdir "%folder%\lib\vc90\x64"
mkdir "%folder%\lib\vc110\x86"
mkdir "%folder%\lib\vc110\x64"

mkdir "%folder%\bin\vc80\x86"
mkdir "%folder%\bin\vc80\x64"
mkdir "%folder%\bin\vc90\x86"
mkdir "%folder%\bin\vc90\x64"
mkdir "%folder%\bin\vc110\x86"
mkdir "%folder%\bin\vc110\x64"
mkdir "%folder%\lic"

copy "%PointoolsVortexAPI%\lic\*.c" 					"%folder%\lic"

copy "%PointoolsVortexAPI%\bin\vc8\PointoolsVortexAPI.dll"   		"%folder%\bin\vc80\x86\"
copy "%PointoolsVortexAPI%\bin\vc8\PointoolsVortexAPI.lib"   		"%folder%\lib\vc80\x86\" 
copy "%PointoolsVortexAPI%\bin\vc8\PointoolsVortexAPI64.dll"   		"%folder%\bin\vc80\x64\" 
copy "%PointoolsVortexAPI%\bin\vc8\PointoolsVortexAPI64.lib"   		"%folder%\lib\vc80\x64\"
copy "%PointoolsVortexAPI%\bin\vc8\PointoolsVortexAPId.dll"   		"%folder%\bin\vc80\x86\" 
copy "%PointoolsVortexAPI%\bin\vc8\PointoolsVortexAPId.lib"   		"%folder%\lib\vc80\x86\" 
copy "%PointoolsVortexAPI%\bin\vc8\PointoolsVortexAPI64d.dll"   	"%folder%\bin\vc80\x64\" 
copy "%PointoolsVortexAPI%\bin\vc8\PointoolsVortexAPI64d.lib"   	"%folder%\lib\vc80\x64\" 

copy "%PointoolsVortexAPI%\bin\vc9\PointoolsVortexAPI.dll"   		"%folder%\bin\vc90\x86\"
copy "%PointoolsVortexAPI%\bin\vc9\PointoolsVortexAPI.lib"   		"%folder%\lib\vc90\x86\" 
copy "%PointoolsVortexAPI%\bin\vc9\PointoolsVortexAPI64.dll"   		"%folder%\bin\vc90\x64\" 
copy "%PointoolsVortexAPI%\bin\vc9\PointoolsVortexAPI64.lib"   		"%folder%\lib\vc90\x64\"
copy "%PointoolsVortexAPI%\bin\vc9\PointoolsVortexAPId.dll"   		"%folder%\bin\vc90\x86\" 
copy "%PointoolsVortexAPI%\bin\vc9\PointoolsVortexAPId.lib"   		"%folder%\lib\vc90\x86\" 
copy "%PointoolsVortexAPI%\bin\vc9\PointoolsVortexAPI64d.dll"   	"%folder%\bin\vc90\x64\" 
copy "%PointoolsVortexAPI%\bin\vc9\PointoolsVortexAPI64d.lib"   	"%folder%\lib\vc90\x64\" 

copy "%PointoolsVortexAPI%\bin\vc11\PointoolsVortexAPI.dll"   		"%folder%\bin\vc110\x86\"
copy "%PointoolsVortexAPI%\bin\vc11\PointoolsVortexAPI.lib"   		"%folder%\lib\vc110\x86\" 
copy "%PointoolsVortexAPI%\bin\vc11\PointoolsVortexAPI64.dll"   	"%folder%\bin\vc110\x64\" 
copy "%PointoolsVortexAPI%\bin\vc11\PointoolsVortexAPI64.lib"   	"%folder%\lib\vc110\x64\"
copy "%PointoolsVortexAPI%\bin\vc11\PointoolsVortexAPId.dll"   		"%folder%\bin\vc110\x86\" 
copy "%PointoolsVortexAPI%\bin\vc11\PointoolsVortexAPId.lib"   		"%folder%\lib\vc110\x86\" 
copy "%PointoolsVortexAPI%\bin\vc11\PointoolsVortexAPI64d.dll"   	"%folder%\bin\vc110\x64\" 
copy "%PointoolsVortexAPI%\bin\vc11\PointoolsVortexAPI64d.lib"   	"%folder%\lib\vc110\x64\" 


copy "%Pointools%\PointoolsIO\workingbuild\bin\PODwriter.dll"   	"%folder%\bin\vc80\x86\"
copy "%Pointools%\PointoolsIO\workingbuild\bin\PODwriterd.dll"  	"%folder%\bin\vc80\x86\"
copy "%Pointools%\PointoolsIO\workingbuild\bin\PODwriter64.dll" 	"%folder%\bin\vc80\x64\"
copy "%Pointools%\PointoolsIO\workingbuild\bin\PODwriter64d.dll" 	"%folder%\bin\vc80\x64\"

copy "%PointoolsLibs%\gl\bin\vc80\glut32.dll"  				"%folder%\bin\vc80\x86\"
copy "%PointoolsLibs%\gl\bin\vc80\glut64.dll"  				"%folder%\bin\vc80\x64\"
copy "%PointoolsLibs%\gl\bin\vc90\glut32.dll"  				"%folder%\bin\vc90\x86\"
copy "%PointoolsLibs%\gl\bin\vc90\glut64.dll"  				"%folder%\bin\vc90\x64\"
copy "%PointoolsLibs%\gl\bin\vc110\glut32.dll"  			"%folder%\bin\vc110\x86\"
copy "%PointoolsLibs%\gl\bin\vc110\glut64.dll"  			"%folder%\bin\vc110\x64\"

copy "%PointoolsLibs%\gl\lib\vc80\glut32.lib" 				"%folder%\lib\vc80\x86\"
copy "%PointoolsLibs%\gl\lib\vc80\glut64.lib" 				"%folder%\lib\vc80\x64\"
copy "%PointoolsLibs%\gl\lib\vc90\glut32.lib" 				"%folder%\lib\vc90\x86\"
copy "%PointoolsLibs%\gl\lib\vc90\glut64.lib" 				"%folder%\lib\vc90\x64\"
copy "%PointoolsLibs%\gl\lib\vc110\glut32.lib" 				"%folder%\lib\vc110\x86\"
copy "%PointoolsLibs%\gl\lib\vc110\glut64.lib" 				"%folder%\lib\vc110\x64\"

copy "%PointoolsLibs%\gl\lib\vc80\glui32.lib" 				"%folder%\lib\vc80\x86\"
copy "%PointoolsLibs%\gl\lib\vc80\glui64.lib" 				"%folder%\lib\vc80\x64\"
copy "%PointoolsLibs%\gl\lib\vc90\glui32.lib" 				"%folder%\lib\vc90\x86\"
copy "%PointoolsLibs%\gl\lib\vc90\glui64.lib" 				"%folder%\lib\vc90\x64\"
copy "%PointoolsLibs%\gl\lib\vc110\glui32.lib" 				"%folder%\lib\vc110\x86\"
copy "%PointoolsLibs%\gl\lib\vc110\glui64.lib" 				"%folder%\lib\vc110\x64\"

copy "%PointoolsLibs%\gl\lib\vc80\glui32d.lib" 				"%folder%\lib\vc80\x86\"
copy "%PointoolsLibs%\gl\lib\vc80\glui64d.lib" 				"%folder%\lib\vc80\x64\"
copy "%PointoolsLibs%\gl\lib\vc90\glui32d.lib" 				"%folder%\lib\vc90\x86\"
copy "%PointoolsLibs%\gl\lib\vc90\glui64d.lib" 				"%folder%\lib\vc90\x64\"
copy "%PointoolsLibs%\gl\lib\vc110\glui32d.lib" 			"%folder%\lib\vc110\x86\"
copy "%PointoolsLibs%\gl\lib\vc110\glui64d.lib" 			"%folder%\lib\vc110\x64\"

