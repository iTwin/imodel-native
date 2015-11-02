/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/NullContext.h $
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
        virtual void _DrawLineString3d(int numPoints, DPoint3dCP points, DPoint3dCP range) override {}
        virtual void _DrawLineString2d(int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range) override {}
        virtual void _DrawPointString3d(int numPoints, DPoint3dCP points, DPoint3dCP range) override {}
        virtual void _DrawPointString2d(int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range) override {}
        virtual void _DrawShape3d(int numPoints, DPoint3dCP points, bool filled, DPoint3dCP range) override {}
        virtual void _DrawShape2d(int numPoints, DPoint2dCP points, bool filled, double zDepth, DPoint2dCP range) override {}
        virtual void _DrawTriStrip3d(int numPoints, DPoint3dCP points, int32_t usageFlags, DPoint3dCP range) override {}
        virtual void _DrawTriStrip2d(int numPoints, DPoint2dCP points, int32_t usageFlags, double zDepth, DPoint2dCP range) override {}
        virtual void _DrawArc3d(DEllipse3dCR ellipse, bool isEllipse, bool filled, DPoint3dCP range) override {}
        virtual void _DrawArc2d(DEllipse3dCR ellipse, bool isEllipse, bool filled, double zDepth, DPoint2dCP range) override {}
        virtual void _DrawBSplineCurve(MSBsplineCurveCR curve, bool filled) override {}
        virtual void _DrawBSplineCurve2d(MSBsplineCurveCR curve, bool filled, double zDepth) override {}
        virtual void _DrawCurveVector(CurveVectorCR curves, bool isFilled) override {}
        virtual void _DrawCurveVector2d(CurveVectorCR curves, bool isFilled, double zDepth) override {}
        virtual void _DrawSolidPrimitive(ISolidPrimitiveCR primitive) override {}
        virtual void _DrawBSplineSurface(MSBsplineSurfaceCR surface) override {}
        virtual void _DrawPolyface(PolyfaceQueryCR meshData, bool filled = false) override {}
        virtual StatusInt _DrawBody(ISolidKernelEntityCR, double pixelSize = 0.0) override {return ERROR;}
        virtual void _DrawTextString(TextStringCR text, double* zDepth = NULL) override {}
        virtual void _DrawRaster2d(DPoint2d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, Byte const* texels, double zDepth, DPoint2d const *range) override {}
        virtual void _DrawRaster(DPoint3d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, Byte const* texels, DPoint3dCP range) override {}
        virtual void _DrawDgnOle(Render::DgnOleDraw*) override {}
        virtual void _DrawPointCloud(Render::PointCloudDraw* drawParams) override {}
        virtual void _DrawMosaic(int numX, int numY, uintptr_t const* tileIds, DPoint3d const* verts) override {}
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
    virtual void _DrawSymbol(Render::IDisplaySymbol* symbolDef, TransformCP trans, ClipPlaneSetP clip, bool ignoreColor, bool ignoreWeight) override {}
    virtual bool _FilterRangeIntersection(GeometricElementCR element) override {if (m_setupScan) return T_Super::_FilterRangeIntersection(element); return false;}
    virtual void _CookDisplayParams(Render::ElemDisplayParamsR, Render::ElemMatSymbR) override {}
    virtual Render::GraphicPtr _BeginGraphic() override {return m_nullGraphic;}

public:
    NullContext(bool setupScan = false) {m_setupScan = setupScan; m_ignoreViewRange = true; m_nullGraphic=new NullGraphic();}
};

END_BENTLEY_DGN_NAMESPACE
