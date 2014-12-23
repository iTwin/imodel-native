set licr=%pointoolsvortex%\diagnosticsMon
set deb=%licr%\bin\Debug
set rel=%licr%\bin\Release
set dll=%PointoolsLib%
set pt_r=%Pointools%\BentleyPointools\bin\vc9\release
set pt_d=%Pointools%\BentleyPointools\bin\vc9\debug

mkdir %deb%\scripts
mkdir %rel%\scripts
mkdir %deb%\config
mkdir %rel%\config
mkdir %rel%\images
mkdir %deb%\images

copy "%PointoolsLib%\devIL-1.7.8\x86\ILU.dll"					%deb%
copy "%PointoolsLib%\devIL-1.7.8\x86\devIL.dll"					%deb%
copy "%PointoolsLib%\XML\win32\bin.msvc\libxml2.dll"				%deb%
copy "%PointoolsLib%\fltk-2.0.x-r5940\lib\fltk2dlld.dll"			%deb%


copy "%PointoolsLib%\devIL-1.7.8\x86\ILU.dll"					%rel%
copy "%PointoolsLib%\devIL-1.7.8\x86\devIL.dll"					%rel%
copy "%PointoolsLib%\XML\win32\bin.msvc\libxml2.dll"				%rel%
copy "%PointoolsLib%\fltk-2.0.x-r5940\lib\fltk2dlld.dll"			%rel%

copy "%Pointools%\BentleyPointools\bin\vc9\win32\debug\ptlangd.dll"		%deb%
copy "%Pointools%\BentleyPointools\bin\vc9\win32\debug\ptwin32d.dll"		%deb%
copy "%Pointools%\BentleyPointools\bin\vc9\win32\debug\ptclassesd.dll"		%deb%
copy "%Pointools%\BentleyPointools\bin\vc9\win32\debug\ptuilwdlld.dll"		%deb%


copy "%Pointools%\BentleyPointools\bin\vc9\win32\release\ptlang.dll"		%rel%
copy "%Pointools%\BentleyPointools\bin\vc9\win32\release\ptwin32.dll"		%rel%
copy "%Pointools%\BentleyPointools\bin\vc9\win32\release\ptclasses.dll"		%rel%
copy "%Pointools%\BentleyPointools\bin\vc9\win32\release\ptuilwdll.dll"		%rel%

copy "%PointoolsLib%\boost_1_37_0\bin\vc9\Win32\boost_thread-vc90-mt-gd-1_37.dll"  %rel%
copy "%PointoolsLib%\boost_1_37_0\bin\vc9\Win32\boost_thread-vc90-mt-gd-1_37.dll"  %deb%

copy %licr%\scripts\*.* 	%rel%\scripts
copy %licr%\scripts\*.* 	%deb%\scripts
copy %licr%\config\*.* 	%rel%\config
copy %licr%\config\*.* 	%deb%\config

//copy 	%img%\*.*	%deb%\images
//copy 	%img%\*.*	%rel%\images