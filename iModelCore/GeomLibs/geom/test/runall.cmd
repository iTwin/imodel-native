@REM Copyright (c) Bentley Systems, Incorporated. All rights reserved.
@REM See LICENSE.md in the repository root for full copyright notice.

for %%f in (StructsTest CurvePrimitiveTest PolyfaceTest QuadratureTest RootTest BsplineTest) do %outRoot%winx86\Build\GeomLibs\build\basegeom\test\%%f\%%f.exe
