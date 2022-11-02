/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <stdlib.h>

#include <Bentley/BentleyPortable.h>
#include <Bentley/BeNumerical.h>
#include <Geom/GeomApi.h>
#include <Vu/VuApi.h>
#include <Geom/bspApi.h>
#include <Mtg/MtgApi.h>
#include <Mtg/capi/mtgprint_capi.h>
#include <Regions/regionsAPI.h>
#include <Regions/rimsbsAPI.h>
#include <Vu/VuSet.h>

// Externally deprecated ... no longer pulled into GeomApi.h
#include <Geom/internal/capi/trigfuncs_capi.h>
#include <Geom/internal/BezierTriangle.h>
#include <Geom/internal/capi/rotmatrix_dep_capi.h>

#include <Bentley/Logging.h>
#define LOG (NativeLogging::CategoryLogger("GeomLibs"))
