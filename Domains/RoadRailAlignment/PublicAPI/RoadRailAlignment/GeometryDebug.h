/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailAlignment/GeometryDebug.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include "RoadRailAlignment.h"

BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

//=======================================================================================
// @bsiclass
// Static methods to present geometry to debugger . .
//=======================================================================================
struct GeometryDebug
    {
    // 0==> no output
    // 1==> minimal
    // 5==> quick summaries of primitives
    // 10==>full json
    // 
    static int s_debug;

    static bvector<bpair<CurveVectorPtr, Dgn::ColorDef>> m_debugCurves;

    ROADRAILALIGNMENT_EXPORT static void Announce(CurveVectorCR cv, char const *name);
    ROADRAILALIGNMENT_EXPORT static void Announce(CurveVectorWithDistanceIndexCR path, char const *name);
    ROADRAILALIGNMENT_EXPORT static void Announce(bvector<PathLocationDetailPair> const &pathAIntervals, bvector<PathLocationDetailPair> const &pathBIntervals, char const * name);

    ROADRAILALIGNMENT_EXPORT static void AddDebugCurve(CurveVectorP curve, Dgn::ColorDef color)         { m_debugCurves.push_back(bpair<CurveVectorPtr, Dgn::ColorDef>(curve, color)); }
    ROADRAILALIGNMENT_EXPORT static void AddDebugPrimitive(ICurvePrimitiveP prim, Dgn::ColorDef color)  { CurveVectorPtr c = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Open); c->Add(prim); AddDebugCurve(c.get(), color); }
    ROADRAILALIGNMENT_EXPORT static void AddDebugPoints(bvector<DPoint3d>& points, Dgn::ColorDef color) { CurveVectorPtr c = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_None); c->Add(ICurvePrimitive::CreatePointString(points)); m_debugCurves.push_back(bpair<CurveVectorPtr, Dgn::ColorDef>(c, color)); }
    ROADRAILALIGNMENT_EXPORT static void AddDebugPoint(DPoint3d point, Dgn::ColorDef color)             { bvector<DPoint3d> points; points.push_back(point); AddDebugPoints(points, color); }
    ROADRAILALIGNMENT_EXPORT static const bvector<bpair<CurveVectorPtr, Dgn::ColorDef>>& GetDebugCurves() { return m_debugCurves; }
    ROADRAILALIGNMENT_EXPORT static void ClearDebugCurves();
    };

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE
