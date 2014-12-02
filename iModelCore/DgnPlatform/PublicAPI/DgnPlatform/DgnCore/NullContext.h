/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/NullContext.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include "ViewContext.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
  Output that doesn't do anything.
  @bsiclass                                                     Brien.Bastings  09/12
+===============+===============+===============+===============+===============+======*/
struct NullOutput : IViewDraw
{
// IDrawGeom methods
virtual ViewFlagsCP _GetDrawViewFlags () override {return NULL;}
virtual void        _SetDrawViewFlags (ViewFlagsCP)  override {}
virtual void        _ActivateMatSymb (ElemMatSymbCP matSymb) override {}
virtual void        _ActivateOverrideMatSymb (OvrMatSymbCP ovrMatSymb) override {}

virtual void        _DrawLineString3d (int numPoints, DPoint3dCP points, DPoint3dCP range) override {}
virtual void        _DrawLineString2d (int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range) override {}
virtual void        _DrawPointString3d (int numPoints, DPoint3dCP points, DPoint3dCP range) override {}
virtual void        _DrawPointString2d (int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range) override {}
virtual void        _DrawShape3d (int numPoints, DPoint3dCP points, bool filled, DPoint3dCP range) override {}
virtual void        _DrawShape2d (int numPoints, DPoint2dCP points, bool filled, double zDepth, DPoint2dCP range) override {}
virtual void        _DrawTriStrip3d (int numPoints, DPoint3dCP points, Int32 usageFlags, DPoint3dCP range) override {}
virtual void        _DrawTriStrip2d (int numPoints, DPoint2dCP points, Int32 usageFlags, double zDepth, DPoint2dCP range) override {}
virtual void        _DrawArc3d (DEllipse3dCR ellipse, bool isEllipse, bool filled, DPoint3dCP range) override {}
virtual void        _DrawArc2d (DEllipse3dCR ellipse, bool isEllipse, bool filled, double zDepth, DPoint2dCP range) override {}
virtual void        _DrawBSplineCurve (MSBsplineCurveCR curve, bool filled) override {}
virtual void        _DrawBSplineCurve2d (MSBsplineCurveCR curve, bool filled, double zDepth) override {}
virtual void        _DrawCurveVector (CurveVectorCR curves, bool isFilled) override {}
virtual void        _DrawCurveVector2d (CurveVectorCR curves, bool isFilled, double zDepth) override {}
virtual void        _DrawSolidPrimitive (ISolidPrimitiveCR primitive) override {}
virtual void        _DrawBSplineSurface (MSBsplineSurfaceCR surface) override {}
virtual void        _DrawPolyface (PolyfaceQueryCR meshData, bool filled = false) override {}
virtual StatusInt   _DrawBody (ISolidKernelEntityCR, IFaceMaterialAttachmentsCP attachments = NULL, double pixelSize = 0.0) override {return ERROR;}
virtual void        _DrawTextString (TextStringCR text, double* zDepth = NULL) override {}
virtual void        _DrawRaster2d (DPoint2d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, byte const* texels, double zDepth, DPoint2d const *range) override {}
virtual void        _DrawRaster (DPoint3d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, byte const* texels, DPoint3dCP range) override {}
virtual void        _DrawDgnOle (IDgnOleDraw*) override {}
virtual void        _DrawPointCloud (IPointCloudDrawParams* drawParams) override {}
virtual void        _DrawMosaic (int numX, int numY, uintptr_t const* tileIds, DPoint3d const* verts) override {}

virtual void        _PushTransClip (TransformCP trans, ClipPlaneSetCP clip = NULL) override {}
virtual void        _PopTransClip () override {}

virtual void        _PushClipStencil (QvElem* qvElem) override {}
virtual void        _PopClipStencil () override {}

virtual RangeResult _PushBoundingRange3d (DPoint3dCP range) override {return RangeResult::Outside;}
virtual RangeResult _PushBoundingRange2d (DPoint2dCP range, double zDepth) override {return RangeResult::Outside;}
virtual void        _PopBoundingRange () override {}

// IViewDraw methods
virtual void        _SetToViewCoords (bool yesNo) override {}
virtual void        _SetSymbology (UInt32 lineColorTBGR, UInt32 fillColorTBGR, int lineWidth, UInt32 linePattern) override {}
virtual void        _DrawGrid (bool doIsoGrid, bool drawDots, DPoint3dCR gridOrigin, DVec3dCR xVector, DVec3dCR yVector, UInt32 gridsPerRef, Point2dCR repetitions) override {}
virtual bool        _DrawSprite (ISprite* sprite, DPoint3dCP location, DPoint3dCP xVec, int transparency) override {return false;}
virtual void        _DrawTiledRaster (ITiledRaster* tiledRaster) override {}
virtual void        _DrawQvElem3d (QvElem* qvElem, int subElemIndex) override {}
virtual void        _DrawQvElem2d (QvElem* qvElem, double zDepth, int subElemIndex) override {}
virtual void        _PushRenderOverrides (ViewFlags, CookedDisplayStyleCP displayOverrides = NULL) override {}
virtual void        _PopRenderOverrides () override {}
//virtual StatusInt   _BeginViewlet (DPoint3dCR frustum, double fraction, DPoint3dCR center, double width, double height, ClipVectorCP clips, RgbColorDef const* bgColor, DgnAttachmentCP refP) override {return ERROR;} removed in graphite
//virtual void        _EndViewlet () override {}
virtual void        _ClearZ () override {}
virtual bool        _IsOutputQuickVision () const override {return false;}
virtual bool        _DeferShadowsToHeal () const override {return false;}
virtual bool        _ApplyMonochromeOverrides (ViewFlagsCR) const override {return false;}
virtual StatusInt   _TestOcclusion (int numVolumes, DPoint3dP verts, int* results) override {return ERROR;}

virtual CookedDisplayStyleCP _GetDrawDisplayStyle () const override {return NULL;}


}; // NullOutput

/*=================================================================================**//**
  Context that doesn't draw anything. NOTE: Every context must setup an output!
  
  A sub-class of NullContext can set a specific output:
  \code
  struct MyNullContext : NullContext
    {
    NullOutput  m_output;

    virtual void _SetupOutputs () override {SetIViewDraw (m_output);}
    };
  \endCode

  Non-subclass users of NullContext must explicitly supply an IViewDrawP: (Failing to do so will trigger an assert)
  \code
  NullOutput    output;
  NullContext   context (&output);

  if (SUCCESS == context.Attach (viewport, DrawPurpose::NotSpecified))
    {
    // Do something.
    context.Detach ();
    }
  \endCode
  @bsiclass                                                     KeithBentley    01/02
+===============+===============+===============+===============+===============+======*/
struct NullContext : ViewContext
{
DEFINE_T_SUPER(ViewContext)

protected:

bool    m_setupScan;

DGNPLATFORM_EXPORT virtual void _AllocateScanCriteria () override;
DGNPLATFORM_EXPORT virtual QvElem* _DrawCached (CachedDrawHandleCR, IStrokeForCache&, Int32 qvIndex) override;

virtual void _DrawSymbol (IDisplaySymbol* symbolDef, TransformCP trans, ClipPlaneSetP clip, bool ignoreColor, bool ignoreWeight) override {}
virtual void _DeleteSymbol (IDisplaySymbol*) override {}
virtual bool _FilterRangeIntersection (ElementHandleCR eh) override {if (m_setupScan) return T_Super::_FilterRangeIntersection (eh); return false;}
//virtual bool _CallElementSubstitutionAsynchs (ElementHandleP* newIter, ElementHandleCR elIter) override {return false;} removed in graphite
virtual bool _WantShowDefaultFieldBackground () override {return false;}
virtual void _DrawQvElem (QvElem* qvElem, bool is3d) override {}
#ifdef WIP_VANCOUVER_MERGE // SymbolCache
virtual void _EmptySymbolCache () override {}
#endif
virtual void _CookDisplayParams (ElemDisplayParamsR, ElemMatSymbR) override {}
virtual void _CookDisplayParamsOverrides () override {}
virtual void _SetupOutputs () override {BeAssert (NULL != m_IViewDraw); SetIViewDraw (*m_IViewDraw);} // Output CAN NOT be NULL!

public:

NullContext (IViewDrawP viewDraw = NULL, bool setupScan = false) {m_IViewDraw = viewDraw; m_setupScan = setupScan; m_IDrawGeom = viewDraw; m_ignoreViewRange = true; }

}; // NullContext

END_BENTLEY_DGNPLATFORM_NAMESPACE
