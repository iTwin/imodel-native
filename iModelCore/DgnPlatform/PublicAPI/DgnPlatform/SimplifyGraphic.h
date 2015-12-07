/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/SimplifyGraphic.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include    "ViewContext.h"
#include    <Bentley/bvector.h>

BEGIN_BENTLEY_DGN_NAMESPACE

struct  SimplifyDrawUnClippedProcessor;

//=======================================================================================
//! Re-interpret complex geometry types as simple types
// @bsiclass                                                      Brien.Bastings  06/05
//=======================================================================================
struct SimplifyGraphic : Render::Graphic
{
protected:
    ViewContextP        m_context;
    IFacetOptionsPtr    m_defaultFacetOptions;
    DVec3d              m_textAxes[2];
    bool                m_inPatternDraw;
    bool                m_inSymbolDraw;
    bool                m_inTextDraw;
    bool                m_inThicknessDraw;
    bool                m_processingMaterialGeometryMap;
    Transform           m_localToWorldTransform;

private:
    void ClipAndProcessGlyph(DgnFontCR, DgnGlyphCR, DPoint3dCR glyphOffset);
    
protected:
    // Functions implemented by sub-classes to allow them to control output.
    virtual bool _DoClipping() const {return false;}
    virtual bool _DoTextGeometry() const {return false;} // When false text is treated as a bounding shape.
    virtual bool _DoSymbolGeometry() const {return false;}
    virtual bool _ProcessAsWireframe() const {return true;} // Output wireframe representation of surfaces/solids/meshes not handled as higher level geometry through _ProcessCurveVector.
    virtual bool _ProcessAsFacets(bool isPolyface) const {return isPolyface;} // Output surfaces/solids not handled directly or are clipped through _ProcessFacetSet.
    virtual bool _ProcessAsBody(bool isCurved) const {return false;} // Output surfaces/solids not handled directly or are clipped through _ProcessBody.
    virtual bool _ProcessAsStrokes(bool isCurved) const {return _ProcessAsFacets(false);} // Output CurveVector not handled directly through _ProcessLinearSegments (or _ProcessFacetSet if region and _ProcessAsFacets).

#if defined (NEEDS_WORK_MATERIAL)
    virtual bool _ProduceMaterialGeometryMaps(MaterialCR material, MaterialMapCR materialMap) const {return false;}
#endif
    virtual bool _ProduceTextureOutlines() const {return false;}

    virtual bool _ClipPreservesRegions() const {return !m_inSymbolDraw;} // Want "fast" clip for patterns/lstyle symbols...
    virtual IFacetOptionsP _GetFacetOptions() {return m_defaultFacetOptions.get();}

    // Process functions implemented by sub-classes.
    virtual StatusInt _ProcessCurvePrimitive(ICurvePrimitiveCR, bool closed, bool filled) {return ERROR;}
    virtual StatusInt _ProcessCurveVector(CurveVectorCR, bool filled) {return ERROR;}
    virtual StatusInt _ProcessSolidPrimitive(ISolidPrimitiveCR) {return ERROR;}
    virtual StatusInt _ProcessSurface(MSBsplineSurfaceCR) {return ERROR;}
    virtual StatusInt _ProcessBody(ISolidKernelEntityCR) {return ERROR;}
    virtual StatusInt _ProcessFacetSet(PolyfaceQueryCR, bool filled) {return ERROR;}
    virtual StatusInt _ProcessLinearSegments(DPoint3dCP points, size_t numPoints, bool closed, bool filled) {return ERROR;}

    virtual void _ActivateMatSymb(Render::GraphicParamsCP matSymb) override {}
    DGNPLATFORM_EXPORT virtual void _AddLineString(int numPoints, DPoint3dCP points, DPoint3dCP range) override;
    DGNPLATFORM_EXPORT virtual void _AddLineString2d(int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range) override;
    DGNPLATFORM_EXPORT virtual void _AddPointString(int numPoints, DPoint3dCP points, DPoint3dCP range) override;
    DGNPLATFORM_EXPORT virtual void _AddPointString2d(int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range) override;
    DGNPLATFORM_EXPORT virtual void _AddShape(int numPoints, DPoint3dCP points, bool filled, DPoint3dCP range) override;
    DGNPLATFORM_EXPORT virtual void _AddShape2d(int numPoints, DPoint2dCP points, bool filled, double zDepth, DPoint2dCP range) override;
    DGNPLATFORM_EXPORT virtual void _AddTriStrip(int numPoints, DPoint3dCP points, int32_t usageFlags, DPoint3dCP range) override;
    DGNPLATFORM_EXPORT virtual void _AddTriStrip2d(int numPoints, DPoint2dCP points, int32_t usageFlags, double zDepth, DPoint2dCP range) override;
    DGNPLATFORM_EXPORT virtual void _AddArc(DEllipse3dCR ellipse, bool isEllipse, bool filled, DPoint3dCP range) override;
    DGNPLATFORM_EXPORT virtual void _AddArc2d(DEllipse3dCR ellipse, bool isEllipse, bool filled, double zDepth, DPoint2dCP range) override;
    DGNPLATFORM_EXPORT virtual void _AddBSplineCurve(MSBsplineCurveCR curve, bool filled) override;
    DGNPLATFORM_EXPORT virtual void _AddBSplineCurve2d(MSBsplineCurveCR curve, bool filled, double zDepth) override;
    DGNPLATFORM_EXPORT virtual void _AddCurveVector(CurveVectorCR curves, bool isFilled) override;
    DGNPLATFORM_EXPORT virtual void _AddCurveVector2d(CurveVectorCR curves, bool isFilled, double zDepth) override;
    DGNPLATFORM_EXPORT virtual void _AddSolidPrimitive(ISolidPrimitiveCR primitive) override;
    DGNPLATFORM_EXPORT virtual void _AddBSplineSurface(MSBsplineSurfaceCR) override;
    DGNPLATFORM_EXPORT virtual void _AddPolyface(PolyfaceQueryCR meshData, bool filled = false) override;
    DGNPLATFORM_EXPORT virtual StatusInt _AddBody(ISolidKernelEntityCR entity, double pixelSize = 0.0) override;
    DGNPLATFORM_EXPORT virtual void _AddTextString(TextStringCR text, double* zDepth) override;
    DGNPLATFORM_EXPORT virtual void _AddRaster3d(DPoint3d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, Byte const* texels, DPoint3dCP range) override;
    DGNPLATFORM_EXPORT virtual void _AddRaster2d(DPoint2d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, Byte const* texels, double zDepth, DPoint2d const *range) override;
    DGNPLATFORM_EXPORT virtual void _AddDgnOle(Render::DgnOleDraw*) override;
    DGNPLATFORM_EXPORT virtual void _AddPointCloud(Render::PointCloudDraw* drawParams) override;
    DGNPLATFORM_EXPORT virtual void _AddMosaic(int numX, int numY, uintptr_t const* tileIds, DPoint3d const* verts) override;
    DGNPLATFORM_EXPORT virtual void _AddSubGraphic(Graphic&, TransformCR, Render::GraphicParams&) override;

public:
    DGNPLATFORM_EXPORT SimplifyGraphic(bool addNormals = false, bool addParameters = false);
    virtual ~SimplifyGraphic() {}

    ViewContextP GetViewContext() const {return m_context;};
    void SetViewContext(ViewContextP context) {m_context = context;} // NOTE: Required!!!

    TransformCR GetLocalToWorldTransform() const {return m_localToWorldTransform;}
    void SetLocalToWorldTransform(TransformCR transform) {m_localToWorldTransform = transform;}

    DMatrix4d GetLocalToView() const
        {
        DMatrix4d   localToWorld = DMatrix4d::From(m_localToWorldTransform);
        DMatrix4d   worldToView = m_context->GetWorldToView().M0;
        DMatrix4d   localToView;

        localToView.InitProduct(worldToView, localToWorld);

        return localToView;
        }

    DMatrix4d GetViewToLocal() const
        {
        Transform   worldToLocalTrans;

        worldToLocalTrans.InverseOf(m_localToWorldTransform);

        DMatrix4d   worldToLocal = DMatrix4d::From(worldToLocalTrans);
        DMatrix4d   viewToWorld = m_context->GetWorldToView().M1;
        DMatrix4d   viewToLocal;

        viewToLocal.InitProduct(worldToLocal, viewToWorld);

        return viewToLocal;
        }

    //! Transform an array of points in the current local coordinate system into DgnCoordSystem::View coordinates.
    //! @param[out]     viewPts     An array to receive the points in DgnCoordSystem::View. Must be dimensioned to hold \c nPts points.
    //! @param[in]      localPts    Input array in current local coordinates,
    //! @param[in]      nPts        Number of points in both arrays.
    void LocalToView(DPoint4dP viewPts, DPoint3dCP localPts, int nPts) const
        {
        GetLocalToView().Multiply(viewPts, localPts, nullptr, nPts);
        }

    //! Transform an array of points in the current local coordinate system into DgnCoordSystem::View coordinates.
    //! @param[out]     viewPts     An array to receive the points in DgnCoordSystem::View. Must be dimensioned to hold \c nPts points.
    //! @param[in]      localPts    Input array in current local coordinates,
    //! @param[in]      nPts        Number of points in both arrays.
    void LocalToView(DPoint3dP viewPts, DPoint3dCP localPts, int nPts) const
        {
        DMatrix4dCR  localToView = GetLocalToView();

        if (nullptr != m_context->GetViewport() && m_context->GetViewport()->IsCameraOn())
            localToView.MultiplyAndRenormalize(viewPts, localPts, nPts);
        else
            localToView.MultiplyAffine(viewPts, localPts, nPts);
        }

    //! Transform an array of points in DgnCoordSystem::View into the current local coordinate system.
    //! @param[out]     localPts    An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
    //! @param[in]      viewPts     Input array in DgnCoordSystem::View.
    //! @param[in]      nPts        Number of points in both arrays.
    void ViewToLocal(DPoint3dP localPts, DPoint4dCP viewPts, int nPts) const
        {
        Transform   worldToLocal;

        worldToLocal.InverseOf(m_localToWorldTransform);
        m_context->ViewToWorld(localPts, viewPts, nPts);
        worldToLocal.Multiply(localPts, localPts, nPts);
        }

    //! Transform an array of points in DgnCoordSystem::View into the current local coordinate system.
    //! @param[out]     localPts    An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
    //! @param[in]      viewPts     Input array in DgnCoordSystem::View.
    //! @param[in]      nPts        Number of points in both arrays.
    void ViewToLocal(DPoint3dP localPts, DPoint3dCP viewPts, int nPts) const
        {
        Transform   worldToLocal;

        worldToLocal.InverseOf(m_localToWorldTransform);
        m_context->ViewToWorld(localPts, viewPts, nPts);
        worldToLocal.Multiply(localPts, localPts, nPts);
        }

    bool PerformClip();
    DGNPLATFORM_EXPORT ClipVectorCP GetCurrClip();

    DGNPLATFORM_EXPORT void ClipAndProcessCurveVector(CurveVectorCR, bool filled);
    DGNPLATFORM_EXPORT void ClipAndProcessSolidPrimitive(ISolidPrimitiveCR);
    DGNPLATFORM_EXPORT void ClipAndProcessSurface(MSBsplineSurfaceCR);
    DGNPLATFORM_EXPORT void ClipAndProcessBody(ISolidKernelEntityCR entity, SimplifyDrawUnClippedProcessor*);
    DGNPLATFORM_EXPORT void ClipAndProcessBodyAsFacets(ISolidKernelEntityCR entity);
    DGNPLATFORM_EXPORT void ClipAndProcessFacetSet(PolyfaceQueryCR, bool filled);
    DGNPLATFORM_EXPORT void ClipAndProcessFacetSetAsCurves(PolyfaceQueryCR);
    DGNPLATFORM_EXPORT void ClipAndProcessText(TextStringCR, double* zDepth);
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    DGNPLATFORM_EXPORT void ClipAndProcessSymbol(IDisplaySymbolP, TransformCP, ClipPlaneSetP);
#endif
    DGNPLATFORM_EXPORT BentleyStatus CurveVectorOutputProcessor(CurveVectorCR curves, bool filled);

    StatusInt ProcessCurvePrimitive(ICurvePrimitiveCR curve, bool closed, bool filled) {return _ProcessCurvePrimitive(curve, closed, filled);}
    StatusInt ProcessCurveVector(CurveVectorCR profile, bool filled) {return _ProcessCurveVector(profile, filled);}
    StatusInt ProcessSolidPrimitive(ISolidPrimitiveCR primitive) {return _ProcessSolidPrimitive(primitive);}
    StatusInt ProcessSurface(MSBsplineSurfaceCR surface) {return _ProcessSurface(surface);}
    StatusInt ProcessBody(ISolidKernelEntityCR entity) {return _ProcessBody(entity);}
    StatusInt ProcessFacetSet(PolyfaceQueryCR facets, bool filled) {return _ProcessFacetSet(facets, filled);}
    StatusInt ProcessGeometryMapOrFacetSet(PolyfaceQueryCR facets, bool filled);

    DGNPLATFORM_EXPORT Render::GraphicParamsR GetEffectiveGraphicParams(Render::GraphicParamsR matSymb); // Get GraphicParams adjusted for overrides...

    IFacetOptionsP GetFacetOptions() {return _GetFacetOptions();}

    DGNPLATFORM_EXPORT bool IsRangeTotallyInside(DRange3dCR range);
    DGNPLATFORM_EXPORT bool IsRangeTotallyInsideClip(DRange3dCR range);
    DGNPLATFORM_EXPORT bool ArePointsTotallyInsideClip(DPoint3dCP points, int nPoints);
    DGNPLATFORM_EXPORT bool ArePointsTotallyOutsideClip(DPoint3dCP points, int nPoints);

    void SetInPatternDraw(bool isPattern) {m_inPatternDraw = isPattern;}
    bool GetInPatternDraw() {return m_inPatternDraw;}

    void SetInSymbolDraw(bool isSymbol) {m_inSymbolDraw = isSymbol;}
    bool GetInSymbolDraw() {return m_inSymbolDraw;}

    void SetInTextDraw(bool isText) {m_inTextDraw = isText;}
    bool GetInTextDraw() {return m_inTextDraw;}

    void SetInThicknessDraw(bool isThickness) {m_inThicknessDraw = isThickness;}
    bool GetInThicknessDraw() {return m_inThicknessDraw;}

    DGNPLATFORM_EXPORT IPolyfaceConstructionPtr GetPolyfaceBuilder();

    DGNPLATFORM_EXPORT BentleyStatus    GetElementToWorldTransform(TransformR transform);
    DGNPLATFORM_EXPORT BentleyStatus    GetLocalToElementTransform(TransformR transform);

    bool                                GetInMaterialGeometryMap() const {return m_processingMaterialGeometryMap;}
#ifdef NEEDS_WORK_GEOMETRY_MAPS
    DGNPLATFORM_EXPORT MaterialCP       GetCurrentMaterial() const;
    DGNPLATFORM_EXPORT MaterialMapCP    GetCurrentGeometryMap() const;

    StatusInt                       ProcessGeometryMap(PolyfaceQueryCR facets);
    StatusInt                       ProcessTextureOutlines(PolyfaceQueryCR facets);
    StatusInt                       ProcessFacetTextureOutlines(IPolyfaceConstructionR, DPoint3dCP points, DPoint2dCP params, bool const* edgeHidden, size_t nPoints, bvector<DPoint3d>&, bvector<int32_t>&);
    DGNPLATFORM_EXPORT void         StrokeGeometryMap(CurveVectorCR curves);
#endif
}; // SimplifyGraphic

END_BENTLEY_DGN_NAMESPACE
