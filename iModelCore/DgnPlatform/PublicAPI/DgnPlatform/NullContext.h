/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/NullContext.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    struct NullGraphic : RefCounted<Render::Graphic>
    {
        virtual void _ActivateMatSymb(Render::ElemMatSymbCP matSymb) override {}
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
        virtual StatusInt _AddBody(ISolidKernelEntityCR, double pixelSize = 0.0) override {return ERROR;}
        virtual void _AddTextString(TextStringCR text, double* zDepth = NULL) override {}
        virtual void _AddRaster2d(DPoint2d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, Byte const* texels, double zDepth, DPoint2d const *range) override {}
        virtual void _AddRaster3d(DPoint3d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, Byte const* texels, DPoint3dCP range) override {}
        virtual void _AddDgnOle(Render::DgnOleDraw*) override {}
        virtual void _AddPointCloud(Render::PointCloudDraw* drawParams) override {}
        virtual void _AddMosaic(int numX, int numY, uintptr_t const* tileIds, DPoint3d const* verts) override {}
    #if defined (NEEDS_WORK_CONTINUOUS_RENDER)
        virtual void _PushClipStencil(Render::Graphic* qvElem) override {}
        virtual void _PopClipStencil() override {}
        virtual void _SetToViewCoords(bool yesNo) override {}
        virtual void _SetSymbology(ColorDef lineColor, ColorDef fillColor, int lineWidth, uint32_t linePattern) override {}
        virtual void _DrawGrid(bool doIsoGrid, bool drawDots, DPoint3dCR gridOrigin, DVec3dCR xVector, DVec3dCR yVector, uint32_t gridsPerRef, Point2dCR repetitions) override {}
        virtual bool _DrawSprite(ISprite* sprite, DPoint3dCP location, DPoint3dCP xVec, int transparency) override {return false;}
        virtual void _DrawTiledRaster(ITiledRaster* tiledRaster) override {}
        virtual void _ClearZ () override {}
        virtual bool _IsOutputQuickVision() const override {return false;}
        virtual bool _ApplyMonochromeOverrides(ViewFlagsCR) const override {return false;}
    #endif
    };

protected:
    bool  m_setupScan;
    RefCountedPtr<NullGraphic> m_nullGraphic;

    void _AllocateScanCriteria() override {if (m_setupScan) T_Super::_AllocateScanCriteria();}
    virtual bool _FilterRangeIntersection(GeometrySourceCR element) override {if (m_setupScan) return T_Super::_FilterRangeIntersection(element); return false;}
    virtual void _CookDisplayParams(Render::ElemDisplayParamsR, Render::ElemMatSymbR) override {}
    virtual Render::GraphicPtr _BeginGraphic(Render::Graphic::CreateParams const& params) override {return m_nullGraphic;}

public:
    NullContext(bool setupScan = false) {m_setupScan = setupScan; m_ignoreViewRange = true; m_nullGraphic=new NullGraphic();}
};

END_BENTLEY_DGN_NAMESPACE
