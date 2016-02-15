/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/NullContext.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "ViewContext.h"

BEGIN_BENTLEY_DGN_NAMESPACE
/*=================================================================================**//**
  Context that doesn't draw anything. NOTE: Every context must set up an output!
  @bsiclass                                                     KeithBentley    01/02
+===============+===============+===============+===============+===============+======*/
struct NullContext : ViewContext
{
    DEFINE_T_SUPER(ViewContext)

    /*=================================================================================**//**
      @bsiclass                                                     Brien.Bastings  09/12
    +===============+===============+===============+===============+===============+======*/
    struct NullGraphic : Render::Graphic
    {
        virtual void _ActivateGraphicParams(Render::GraphicParamsCR, Render::GeometryParamsCP) override {}
        virtual void _AddLineString(int numPoints, DPoint3dCP points, DPoint3dCP range) override {}
        virtual void _AddLineString2d(int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range) override {}
        virtual void _AddPointString(int numPoints, DPoint3dCP points, DPoint3dCP range) override {}
        virtual void _AddPointString2d(int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range) override {}
        virtual void _AddShape(int numPoints, DPoint3dCP points, bool filled, DPoint3dCP range) override {}
        virtual void _AddShape2d(int numPoints, DPoint2dCP points, bool filled, double zDepth, DPoint2dCP range) override {}
        virtual void _AddTriStrip(int numPoints, DPoint3dCP points, int32_t usageFlags, DPoint3dCP range) override {}
        virtual void _AddTriStrip2d(int numPoints, DPoint2dCP points, int32_t usageFlags, double zDepth, DPoint2dCP range) override {}
        virtual void _AddArc(DEllipse3dCR ellipse, bool isEllipse, bool filled, DPoint3dCP range) override {}
        virtual void _AddArc2d(DEllipse3dCR ellipse, bool isEllipse, bool filled, double zDepth, DPoint2dCP range) override {}
        virtual void _AddBSplineCurve(MSBsplineCurveCR curve, bool filled) override {}
        virtual void _AddBSplineCurve2d(MSBsplineCurveCR curve, bool filled, double zDepth) override {}
        virtual void _AddCurveVector(CurveVectorCR curves, bool isFilled) override {}
        virtual void _AddCurveVector2d(CurveVectorCR curves, bool isFilled, double zDepth) override {}
        virtual void _AddSolidPrimitive(ISolidPrimitiveCR primitive) override {}
        virtual void _AddBSplineSurface(MSBsplineSurfaceCR surface) override {}
        virtual void _AddPolyface(PolyfaceQueryCR meshData, bool filled = false) override {}
        virtual void _AddBody(ISolidKernelEntityCR, double pixelSize = 0.0) override {}
        virtual void _AddTextString(TextStringCR text) override {}
        virtual void _AddTextString2d(TextStringCR text, double zDepth) override {}
        virtual void _AddMosaic(int numX, int numY, uintptr_t const* tileIds, DPoint3d const* verts) override {}
        virtual void _AddRaster(DPoint3d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, Byte const* texels, DPoint3dCP range) override {}
        virtual void _AddRaster2d(DPoint2d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, Byte const* texels, double zDepth, DPoint2d const *range) override {}
        virtual void _AddDgnOle(Render::DgnOleDraw*) override {}
        virtual void _AddPointCloud(Render::PointCloudDraw* drawParams) override {}
        virtual void _AddSubGraphic(Render::GraphicR, TransformCR, Render::GraphicParamsCR) override {}
        virtual Render::GraphicPtr _CreateSubGraphic(TransformCR) const override {return new NullGraphic();}
    };

protected:
    virtual Render::GraphicPtr _CreateGraphic(Render::Graphic::CreateParams const& params) override {return new NullGraphic();}

public:
    NullContext() {m_ignoreViewRange = true;}
};

END_BENTLEY_DGN_NAMESPACE
