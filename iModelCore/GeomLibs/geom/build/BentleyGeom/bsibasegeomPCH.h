/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BentleyPortable.h>
#include <Bentley/BeNumerical.h>
#include <Geom/GeomApi.h>
#include <Vu/VuApi.h>
#include <Geom/bspApi.h>
#include <Mtg/MtgApi.h>
#include <Mtg/mtgprint.fdf>
#include <Regions/regionsAPI.h>
#include <Regions/rimsbsAPI.h>
#include <Vu/VuSet.h>
// Externally deprecated ... no longer pulled into GeomApi.h
#include <Geom/internal/trigfuncs.fdf>

#include <Geom/internal/BezierTriangle.h>
#include <Geom/internal/rotmatrix_dep.fdf>

#include <stdlib.h>
