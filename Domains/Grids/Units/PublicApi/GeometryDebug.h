/*--------------------------------------------------------------------------------------+
|
|     $Source: Units/PublicApi/GeometryDebug.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <DgnView/DgnTool.h>
#include <DgnView/Locate.h>
#include <DgnView\SelectionSetManager.h>
#include <DgnView\AccuSnap.h>
#include <BuildingMacros.h>
#include <DgnView/DgnViewLib.h>

BEGIN_BUILDING_NAMESPACE

// Static methods to present geometry to debugger . .
struct GeometryDebug
    {
    // 0==> no output
    // 1==> minimal
    // 5==> quick summaries of primitives
    // 10==>full json
    // 
    static int s_debug;

    static bvector<bpair<CurveVectorPtr, Dgn::ColorDef>> m_debugCurves;

    BUILDINGUNITS_EXPORT static void SetDebugLevel(int dbg){ s_debug = dbg;};
    BUILDINGUNITS_EXPORT static void Announce(CurveVectorCR cv, char const *name);
    BUILDINGUNITS_EXPORT static void Announce(PolyfaceHeaderCR mesh, char const *name);
    BUILDINGUNITS_EXPORT static void Announce(CurveVectorWithDistanceIndexCR path, char const *name);
    BUILDINGUNITS_EXPORT static void Announce(bvector<PathLocationDetailPair> const &pathAIntervals, bvector<PathLocationDetailPair> const &pathBIntervals, char const * name);

    BUILDINGUNITS_EXPORT static void AddDebugCurve(CurveVectorP curve, Dgn::ColorDef color)         { m_debugCurves.push_back(bpair<CurveVectorPtr, Dgn::ColorDef>(curve, color)); }
    BUILDINGUNITS_EXPORT static void AddDebugPrimitive(ICurvePrimitiveP prim, Dgn::ColorDef color)  { CurveVectorPtr c = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Open); c->Add(prim); AddDebugCurve(c.get(), color); }
    BUILDINGUNITS_EXPORT static void AddDebugPoints(bvector<DPoint3d>& points, Dgn::ColorDef color) { CurveVectorPtr c = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_None); c->Add(ICurvePrimitive::CreatePointString(points)); m_debugCurves.push_back(bpair<CurveVectorPtr, Dgn::ColorDef>(c, color)); }
    BUILDINGUNITS_EXPORT static void AddDebugPoint(DPoint3d point, Dgn::ColorDef color)             { bvector<DPoint3d> points; points.push_back(point); AddDebugPoints(points, color); }
    BUILDINGUNITS_EXPORT static const bvector<bpair<CurveVectorPtr, Dgn::ColorDef>>& GetDebugCurves() { return m_debugCurves; }
    BUILDINGUNITS_EXPORT static void ClearDebugCurves();
    };

END_BUILDING_NAMESPACE