/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ElementGraphics.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
    DGNPLATFORM_EXPORT static CurveVectorPtr CollectCurves(ISolidPrimitiveCR, DgnDbR, bool includeEdges = true, bool includeFaceIso = false, ViewContextP context = nullptr);
    DGNPLATFORM_EXPORT static CurveVectorPtr CollectCurves(MSBsplineSurfaceCR, DgnDbR, bool includeEdges = true, bool includeFaceIso = false, ViewContextP context = nullptr);
    DGNPLATFORM_EXPORT static CurveVectorPtr CollectCurves(ISolidKernelEntityCR, DgnDbR, bool includeEdges = true, bool includeFaceIso = false, ViewContextP context = nullptr);

    DGNPLATFORM_EXPORT static PolyfaceHeaderPtr CollectPolyface(ISolidKernelEntityCR, DgnDbR, IFacetOptionsR);

    DGNPLATFORM_EXPORT static void CollectCurves(ISolidKernelEntityCR, DgnDbR, bvector<CurveVectorPtr>& curves, bvector<Render::GeometryParams>& params, bool includeEdges = true, bool includeFaceIso = false);
    DGNPLATFORM_EXPORT static void CollectPolyfaces(ISolidKernelEntityCR, DgnDbR, bvector<PolyfaceHeaderPtr>& polyfaces, bvector<Render::GeometryParams>& params, IFacetOptionsR);

    DGNPLATFORM_EXPORT static void Draw(Render::GraphicBuilderR, ISolidPrimitiveCR, ViewContextR, bool includeEdges = true, bool includeFaceIso = true);
    DGNPLATFORM_EXPORT static void Draw(Render::GraphicBuilderR, MSBsplineSurfaceCR, ViewContextR, bool includeEdges = true, bool includeFaceIso = true);
    DGNPLATFORM_EXPORT static void Draw(Render::GraphicBuilderR, ISolidKernelEntityCR, ViewContextR, bool includeEdges = true, bool includeFaceIso = true);

    DGNPLATFORM_EXPORT static void DrawOutline(CurveVectorCR, Render::GraphicBuilderR);
    DGNPLATFORM_EXPORT static void DrawOutline2d(CurveVectorCR, Render::GraphicBuilderR, double zDepth);
};

END_BENTLEY_DGN_NAMESPACE
