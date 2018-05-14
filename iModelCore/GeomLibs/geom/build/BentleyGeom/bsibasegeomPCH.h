/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/build/BentleyGeom/bsibasegeomPCH.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BentleyPortable.h>
#include <Bentley/BeNumerical.h>
#include <Geom/GeomApi.h>
#include <Vu/VuApi.h>
#include <Geom/bspApi.h>
#include <Mtg/GpaApi.h>
#include <Mtg/MtgApi.h>
#include <Mtg/mtgprint.fdf>
#include <Regions/regionsAPI.h>
#include <Regions/rimsbsAPI.h>
#include <Vu/VuSet.h>
// Externally deprecated ... no longer pulled into GeomApi.h
#include <Geom/internal/trigfuncs.fdf>

#include <Geom/internal/BezierTriangle.h>
#include <Geom/internal/dvec3dtransform.fdf>
#include <Geom/internal/rotmatrix_dep.fdf>
#include <Geom/internal/complex.fdf>

#include <Geom/internal/dsegment3d.fdf>
#include <Geom/internal/dsegment4d.fdf>

#include <stdlib.h>
