
set JSMETASRCPATH=%srcroot%geomlibs\js\jsmetasrc\
set JSOUT=%JSMETASRCPATH%..\generated\

set GFILE0=-f %JSMETASRCPATH%\expandIncludes.g
set GFILE1=-f %JSMETASRCPATH%\inlineArrays.g -f %JSMETASRCPATH%\resolveIncludes.g
set GFILE3D=-f %JSMETASRCPATH%\3d.g

gema %GFILE0% jsMatrix3.master | gema %GFILE3D% | gema %GFILE1% | gema  -f cpp2js.g | gema -f cleanupDefs.g > %JSOUT%\Matrix3.js

gema %GFILE0% jsVector3.master | gema %GFILE3D% | gema %GFILE1% | gema  -f cpp2js.g | gema -f cleanupDefs.g > %JSOUT%\Vector3.js

gema %GFILE0% jsPoint3.master  | gema %GFILE3D% | gema %GFILE1% | gema  -f cpp2js.g | gema -f cleanupDefs.g > %JSOUT%\Point3.js

