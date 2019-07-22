/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include "CivilBaseGeometry.h"

BEGIN_BENTLEY_CIVILGEOMETRY_NAMESPACE

//=======================================================================================
// @bsiclass
// Static methods to present geometry to debugger . .
//=======================================================================================
struct GeometryDebug
    {
    typedef uint32_t ColorInt;
    //=======================================================================================
    // @bsiclass
    // An entry for a debug curve.
    // We use 'ColorTag' instead of Dgn::ColorDef so we don't introduce a dependency on DgnPlatform
    //=======================================================================================
    struct CurveEntry
    {
    CurveVectorPtr curve;
    ColorInt colorInt;
    };

private:
    // 0==> no output
    // 1==> minimal
    // 5==> quick summaries of primitives
    // 10==>full json
    static int s_debug;
    static bvector<CurveEntry> s_debugCurves;

public:
    CIVILBASEGEOMETRY_EXPORT static void Announce(CurveVectorCR cv, char const *name);
    CIVILBASEGEOMETRY_EXPORT static void Announce(CurveVectorWithDistanceIndexCR path, char const *name);
    CIVILBASEGEOMETRY_EXPORT static void Announce(bvector<PathLocationDetailPair> const &pathAIntervals, bvector<PathLocationDetailPair> const &pathBIntervals, char const * name);

    CIVILBASEGEOMETRY_EXPORT static void AddDebugCurve(CurveVectorP curve, ColorInt colorInt);
    CIVILBASEGEOMETRY_EXPORT static void AddDebugPrimitive(ICurvePrimitiveP prim, ColorInt colorInt);
    CIVILBASEGEOMETRY_EXPORT static void AddDebugPoints(bvector<DPoint3d> const& points, ColorInt colorInt);
    CIVILBASEGEOMETRY_EXPORT static void AddDebugPoint(DPoint3dCR point, ColorInt colorInt);
    CIVILBASEGEOMETRY_EXPORT static void ClearDebugCurves();
    CIVILBASEGEOMETRY_EXPORT static bvector<CurveEntry> const& GetDebugCurves() { return s_debugCurves; }
    };

END_BENTLEY_CIVILGEOMETRY_NAMESPACE