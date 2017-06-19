
set JSMETASRCPATH=%srcroot%geomlibs\js\jsmetasrc\
set JSOUT=%JSMETASRCPATH%..\generated\

gema -f %JSMETASRCPATH%\inlineArrays.g -f %JSMETASRCPATH%\resolveIncludes.g jsMatrix3.master | gema -f 3d.g | gema  -f cpp2js.g > %JSOUT%\Matrix3.js

gema -f %JSMETASRCPATH%\inlineArrays.g -f %JSMETASRCPATH%\resolveIncludes.g jsVector3.master | gema -f 3d.g | gema  -f cpp2js.g > %JSOUT%\Vector3.js

gema -f %JSMETASRCPATH%\inlineArrays.g -f %JSMETASRCPATH%\resolveIncludes.g jsPoint3.master | gema -f 3d.g | gema  -f cpp2js.g > %JSOUT%\Point3.js
