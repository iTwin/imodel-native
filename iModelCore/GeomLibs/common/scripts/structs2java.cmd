cd %MSJ%utils\geom
cvs update

rem ---- translate new matrix/transform memory format to old format
copy structs\DTransform3d.c structs\transform.c
copy structs\DMatrix3d.c structs\rotmatrix.c
call gemageom transform transform
call gemageom transform rotmatrix

REM ***********************************************
REM *                                                                                     *
REM * NEEDS WORK: distinguish fdf from mdl/bsifdf *
REM *                                                                                     *
REM ***********************************************
rem ---- complete GEOM function definitions: add as necessary
call fdf barycentric
call fdf complex
call fdf dcone3d
call fdf dconic4d
call fdf ddisk3d
call fdf dellipse3d
call fdf dellipse4d
call fdf dellipsoid3d
call fdf dmap4d
call fdf dmatrix2d
call fdf dmatrix3d
call fdf dmatrix4d
call fdf doublefuncs
call fdf dplane3d
call fdf dpoint2d
call fdf dpoint3d
call fdf dsegment4d
call fdf dtoroid3d
call fdf fpoint2d
call fdf fpoint3d
call fdf dpoint3darray
call fdf dpoint2darray
call fdf dpoint4d
call fdf drange2d
call fdf drange3d
call fdf frange2d
call fdf frange3d
call fdf dray3d
call fdf dsegment3d
call fdf dtransform2d
call fdf dtransform3d
call fdf eigensys3d
call fdf graphicspoint
call fdf jmdl_2ms
call fdf migeomstructsinit.c include\geom\migeomstructsinit.fdf
call fdf jmdl_hmtran
call fdf linalg
call fdf lineargeom
call fdf pencil
call fdf polycoff
call fdf polygon2d
call fdf polygon3d
call fdf polygondecomp
call fdf polyline2d
call fdf polyline3d
call fdf polyplane4d
call fdf polysolv
call fdf proximitydata
call fdf qsic
call fdf quadeqn
call fdf quadraticgeom
call fdf quadric
call fdf rctree
call fdf rename
call fdf rotations
call fdf rotconic
call fdf rotmatrix
call fdf simpson
call fdf smallsetrange1d
call fdf svd
call fdf transform
call fdf trigfuncs

bmake +dTRX -dmethods -ddls bsibasegeom

rem --------------------------------------BASETYPES
cd %JMDLSDK%com\bentley\sys\basetypes
cvs update

rem ---- complete GEOM propagation: add as necessary
call fixdefs DConic4d
call fixdefs DEllipse3d
call fixdefs DEllipsoid3d
call fixdefs DMap4d
call fixdefs DMatrix2d
call fixdefs DMatrix3d
call fixdefs DMatrix4d
call fixdefs DPlane3d
call fixdefs FPoint2d
call fixdefs FPoint3d
call fixdefs DPoint2d
call fixdefs DPoint2dPair
call fixdefs DPoint3d
call fixdefs DPoint3dPair
call fixdefs DPoint4d
call fixdefs DRange2d
call fixdefs DRange3d
call fixdefs FRange2d
call fixdefs FRange3d
call fixdefs DRay3d
call fixdefs DSegment3d
call fixdefs DTransform4d
call fixdefs DTransform3d
call fixdefs Geom
call fixdefs GraphicsPoint
call fixdefs Point2d
call fixdefs Point3d
call fixdefs ProximityData
call fixdefs RotMatrix
call fixdefs Transform

bmake basetypes

rem --------------------------------------DGN
rem -- DON'T COMMIT ANY SIGNATURE CHANGES! --
rem -----------------------------------------
cd %COM%bentley\dgn
cvs update

rem ---- complete GEOM propagation: add as necessary
set UPDATE_DGN_PACKAGE=
if "%UPDATE_DGN_PACKAGE%" == "" goto postDGN
  call fixdefs rmatrix.c
  call fixdefs tmatrix.c
  call fixdefs RMatrix.java
  call fixdefs TMatrix.java
  call fixdefs DPoint.java
:postDGN


bmake dgn
