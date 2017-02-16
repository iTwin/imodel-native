/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ElementGraphics.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include "DgnPlatform.h"
#include "SolidKernel.h"
#include "ViewContext.h"

BEGIN_BENTLEY_DGN_NAMESPACE

struct WireframeGeomUtil
{
    DGNPLATFORM_EXPORT static CurveVectorPtr CollectCurves(ISolidPrimitiveCR, DgnDbR, bool includeEdges = true, bool includeFaceIso = false);
    DGNPLATFORM_EXPORT static CurveVectorPtr CollectCurves(MSBsplineSurfaceCR, DgnDbR, bool includeEdges = true, bool includeFaceIso = false);
    DGNPLATFORM_EXPORT static CurveVectorPtr CollectCurves(IBRepEntityCR, DgnDbR, bool includeEdges = true, bool includeFaceIso = false);

    DGNPLATFORM_EXPORT static void Draw(Render::GraphicBuilderR, ISolidPrimitiveCR, CheckStop* stopTester = nullptr, bool includeEdges = true, bool includeFaceIso = true);
    DGNPLATFORM_EXPORT static void Draw(Render::GraphicBuilderR, MSBsplineSurfaceCR, CheckStop* stopTester = nullptr, bool includeEdges = true, bool includeFaceIso = true);
    DGNPLATFORM_EXPORT static void Draw(Render::GraphicBuilderR, IBRepEntityCR, CheckStop* stopTester = nullptr, bool includeEdges = true, bool includeFaceIso = true);

    DGNPLATFORM_EXPORT static void DrawOutline(CurveVectorCR, Render::GraphicBuilderR);
    DGNPLATFORM_EXPORT static void DrawOutline2d(CurveVectorCR, Render::GraphicBuilderR, double zDepth);

    DGNPLATFORM_EXPORT static void DrawControlPolygon(MSBsplineSurfaceCR, Render::GraphicBuilderR, Render::GraphicParamsCR);
    DGNPLATFORM_EXPORT static void DrawControlPolygon(ICurvePrimitiveCR, Render::GraphicBuilderR, Render::GraphicParamsCR, bool is3d = true, double zDepth = 0.0);
};

END_BENTLEY_DGN_NAMESPACE
