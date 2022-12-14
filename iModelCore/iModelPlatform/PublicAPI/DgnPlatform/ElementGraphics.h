/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "DgnPlatform.h"
#include "ViewContext.h"

BEGIN_BENTLEY_DGN_NAMESPACE

struct WireframeGeomUtil
{
    DGNPLATFORM_EXPORT static bool ComputeRuleArc(DEllipse3dR ellipse, DPoint3dCR startPt, DPoint3dCR originPt, double sweepAngle, TransformCR transform, RotMatrixCR axes, RotMatrixCR invAxes, double ruleTolerance = 1.0e-10);
    DGNPLATFORM_EXPORT static bool CollectLateralRulePoints(CurveVectorCR curves, bvector<DPoint3d>& pts, uint32_t divisor, uint32_t closedDivisor, CheckStop* stopTester = nullptr);
    DGNPLATFORM_EXPORT static bool CollectVertices(CurveVectorCR curves, bvector<DPoint3d>& pts, bool checkDistance, CheckStop* stopTester = nullptr);

    DGNPLATFORM_EXPORT static bool CollectLateralEdges(DgnExtrusionDetailCR detail, bvector<DSegment3d>& edges, CheckStop* stopTester = nullptr);
    DGNPLATFORM_EXPORT static bool CollectLateralEdges(DgnRotationalSweepDetailCR detail, bvector<DEllipse3d>& edges, CheckStop* stopTester = nullptr);
    DGNPLATFORM_EXPORT static bool CollectLateralEdges(DgnRuledSweepDetailCR detail, bvector<DSegment3d>& edges, CheckStop* stopTester = nullptr);

    DGNPLATFORM_EXPORT static CurveVectorPtr CollectCurves(ISolidPrimitiveCR, DgnDbR, GeometryStreamEntryIdCP entryId = nullptr);
    DGNPLATFORM_EXPORT static CurveVectorPtr CollectCurves(MSBsplineSurfaceCR, DgnDbR, GeometryStreamEntryIdCP entryId = nullptr);

    DGNPLATFORM_EXPORT static void Draw(ISolidPrimitiveCR, Render::GraphicBuilderR, CheckStop* stopTester = nullptr);
    DGNPLATFORM_EXPORT static void Draw(MSBsplineSurfaceCR, Render::GraphicBuilderR, CheckStop* stopTester = nullptr);

    DGNPLATFORM_EXPORT static void DrawOutline(CurveVectorCR, Render::GraphicBuilderR);
    DGNPLATFORM_EXPORT static void DrawOutline2d(CurveVectorCR, Render::GraphicBuilderR, double zDepth);

    DGNPLATFORM_EXPORT static void DrawUVRules(MSBsplineSurfaceCR, Render::GraphicBuilderR, Render::GraphicParamsCR, CheckStop* stopTester = nullptr);
    DGNPLATFORM_EXPORT static void DrawControlPolygon(MSBsplineSurfaceCR, Render::GraphicBuilderR, Render::GraphicParamsCR);
    DGNPLATFORM_EXPORT static void DrawControlPolygon(ICurvePrimitiveCR, Render::GraphicBuilderR, Render::GraphicParamsCR, bool is3d = true, double zDepth = 0.0);
};

END_BENTLEY_DGN_NAMESPACE
