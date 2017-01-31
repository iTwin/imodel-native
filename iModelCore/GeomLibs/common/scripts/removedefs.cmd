set fullName=%1
if EXIST %fullName% goto doit
set fullName=%1.mjava
if EXIST %fullName% goto doit
set fullName=%1.java
if EXIST %fullName% goto doit
echo removedefs.cmd: NOT A RECOGNIZED FILE: %1, %1.java, or %1.mjava
goto done

:doit
copy %fullName% %TEMP%
gema -arglen 8192 -f %GEMADIR%g\removedefs.g %TEMP%\%fullName% %fullName%
:done
