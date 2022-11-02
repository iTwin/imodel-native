@REM Copyright (c) Bentley Systems, Incorporated. All rights reserved.
@REM See LICENSE.md in the repository root for full copyright notice.

call bb -r geomlibs -f geomlibs -p StructsTest b -f %1
call bb -r geomlibs -f geomlibs -p CurvePrimitiveTest b -f %1
call bb -r geomlibs -f geomlibs -p PolyfaceTest b -f %1
call bb -r geomlibs -f geomlibs -p QuadratureTest b -f %1
call bb -r geomlibs -f geomlibs -p RootTest b -f %1
call bb -r geomlibs -f geomlibs -p BsplineTest b -f %1
call bb -r geomlibs -f geomlibs -p BezierTest b -f %1
call bb -r geomlibs -f geomlibs -p PolylineOpsTest b -f %1
rem call bb -r geomlibs -f geomlibs -p VuTest b -f %1
call bb -r geomlibs -f geomlibs -p GUnitTests_Published_GeomLibs b -f %1


