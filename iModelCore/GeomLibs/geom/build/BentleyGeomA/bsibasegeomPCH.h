/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/build/BentleyGeomA/bsibasegeomPCH.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//#include <Bentley/BentleyPortable.h>
//#include <Bentley/BeNumerical.h>
#include <Geom/GeomApi.h>
#ifndef SmallGeomLib
#include <Geom/bspApi.h>
#include <Mtg/GpaApi.h>
#include <Mtg/MtgApi.h>
#include <Mtg/mtgprint.fdf>
#include <Regions/regionsAPI.h>
#include <Regions/rimsbsAPI.h>
#endif
#include <Vu/VuApi.h>
#include <Vu/VuSet.h>

// Externally deprecated ... no longer pulled into GeomApi.h
#include <Geom/internal/trigfuncs.fdf>
#ifndef SmallGeomLib
#include <Geom/internal/BezierTriangle.h>
#include <Geom/internal/dmatrix3d.h>
#include <Geom/internal/dtransform3d.h>
#include <Geom/internal/dmatrix3d.fdf>
#include <Geom/internal/dmatrix3d_dep.fdf>
#include <Geom/internal/dtransform3d.fdf>
#include <Geom/internal/dvec3ddmatrix3d.fdf>
#include <Geom/internal/dvec3dtransform.fdf>
#include <Geom/internal/rotmatrix_dep.fdf>
#include <Geom/internal/basegeom_2ms.fdf>
#include <Geom/internal/complex.fdf>

#include <Geom/internal/dsegment3d.fdf>
#include <Geom/internal/dsegment4d.fdf>
#endif


