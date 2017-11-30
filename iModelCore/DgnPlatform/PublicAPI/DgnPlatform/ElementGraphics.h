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
    DGNPLATFORM_EXPORT static CurveVectorPtr CollectCurves(ISolidPrimitiveCR, DgnDbR, GeometryStreamEntryIdCP entryId = nullptr);
    DGNPLATFORM_EXPORT static CurveVectorPtr CollectCurves(MSBsplineSurfaceCR, DgnDbR, GeometryStreamEntryIdCP entryId = nullptr);
    DGNPLATFORM_EXPORT static CurveVectorPtr CollectCurves(IBRepEntityCR, DgnDbR, GeometryStreamEntryIdCP entryId = nullptr);

    DGNPLATFORM_EXPORT static void Draw(ISolidPrimitiveCR, Render::GraphicBuilderR, CheckStop* stopTester = nullptr);
    DGNPLATFORM_EXPORT static void Draw(MSBsplineSurfaceCR, Render::GraphicBuilderR, CheckStop* stopTester = nullptr);
    DGNPLATFORM_EXPORT static void Draw(IBRepEntityCR, Render::GraphicBuilderR, CheckStop* stopTester = nullptr);

    DGNPLATFORM_EXPORT static void DrawOutline(CurveVectorCR, Render::GraphicBuilderR);
    DGNPLATFORM_EXPORT static void DrawOutline2d(CurveVectorCR, Render::GraphicBuilderR, double zDepth);

    DGNPLATFORM_EXPORT static void DrawUVRules(MSBsplineSurfaceCR, Render::GraphicBuilderR, Render::GraphicParamsCR, CheckStop* stopTester = nullptr);
    DGNPLATFORM_EXPORT static void DrawControlPolygon(MSBsplineSurfaceCR, Render::GraphicBuilderR, Render::GraphicParamsCR);
    DGNPLATFORM_EXPORT static void DrawControlPolygon(ICurvePrimitiveCR, Render::GraphicBuilderR, Render::GraphicParamsCR, bool is3d = true, double zDepth = 0.0);
};

END_BENTLEY_DGN_NAMESPACE
