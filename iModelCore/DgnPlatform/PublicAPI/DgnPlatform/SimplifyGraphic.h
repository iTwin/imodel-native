/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/SimplifyGraphic.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatform.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Re-interpret complex geometry types as simple types
//! @ingroup GROUP_Geometry
// @bsiclass                                                      Brien.Bastings  06/05
//=======================================================================================
struct SimplifyGraphic : Render::Graphic, Render::IGraphicBuilder
{
    DEFINE_T_SUPER(Render::Graphic);

protected:
    IGeometryProcessorR     m_processor;
    ViewContextR            m_context;
    IFacetOptionsPtr        m_facetOptions;

    DVec3d                  m_textAxes[2];
    bool                    m_inPatternDraw;
    bool                    m_inSymbolDraw;
    bool                    m_inTextDraw;
    bool                    m_isOpen = true;

    Render::GraphicParams   m_currGraphicParams;
    Render::GeometryParams  m_currGeometryParams;

    DGNPLATFORM_EXPORT void _ActivateGraphicParams(Render::GraphicParamsCR graphicParams, Render::GeometryParamsCP geomParams) override;
    DGNPLATFORM_EXPORT void _AddLineString(int numPoints, DPoint3dCP points) override;
    DGNPLATFORM_EXPORT void _AddLineString2d(int numPoints, DPoint2dCP points, double zDepthe) override;
    DGNPLATFORM_EXPORT void _AddPointString(int numPoints, DPoint3dCP points) override;
    DGNPLATFORM_EXPORT void _AddPointString2d(int numPoints, DPoint2dCP points, double zDepthe) override;
    DGNPLATFORM_EXPORT void _AddShape(int numPoints, DPoint3dCP points, bool filled) override;
    DGNPLATFORM_EXPORT void _AddShape2d(int numPoints, DPoint2dCP points, bool filled, double zDepth) override;
    DGNPLATFORM_EXPORT void _AddTriStrip(int numPoints, DPoint3dCP points, int32_t usageFlags) override;
    DGNPLATFORM_EXPORT void _AddTriStrip2d(int numPoints, DPoint2dCP points, int32_t usageFlags, double zDepth) override;
    DGNPLATFORM_EXPORT void _AddArc(DEllipse3dCR ellipse, bool isEllipse, bool filled) override;
    DGNPLATFORM_EXPORT void _AddArc2d(DEllipse3dCR ellipse, bool isEllipse, bool filled, double zDepth) override;
    DGNPLATFORM_EXPORT void _AddBSplineCurve(MSBsplineCurveCR curve, bool filled) override;
    DGNPLATFORM_EXPORT void _AddBSplineCurve2d(MSBsplineCurveCR curve, bool filled, double zDepth) override;
    DGNPLATFORM_EXPORT void _AddCurveVector(CurveVectorCR curves, bool isFilled) override;
    DGNPLATFORM_EXPORT void _AddCurveVector2d(CurveVectorCR curves, bool isFilled, double zDepth) override;
    DGNPLATFORM_EXPORT void _AddSolidPrimitive(ISolidPrimitiveCR primitive) override;
    DGNPLATFORM_EXPORT void _AddBSplineSurface(MSBsplineSurfaceCR) override;
    DGNPLATFORM_EXPORT void _AddPolyface(PolyfaceQueryCR meshData, bool filled = false) override;
    DGNPLATFORM_EXPORT void _AddTriMesh(TriMeshArgs const&) override;
    DGNPLATFORM_EXPORT void _AddBody(ISolidKernelEntityCR entity) override;
    DGNPLATFORM_EXPORT void _AddTextString(TextStringCR text) override;
    DGNPLATFORM_EXPORT void _AddTextString2d(TextStringCR text, double zDepth) override;
    DGNPLATFORM_EXPORT void _AddTile(Render::TextureCR tile, DPoint3dCP corners) override;
    DGNPLATFORM_EXPORT void _AddDgnOle(Render::DgnOleDraw*) override;
    DGNPLATFORM_EXPORT void _AddPointCloud(Render::PointCloudDraw* drawParams) override;
    DGNPLATFORM_EXPORT void _AddSubGraphic(Render::GraphicR, TransformCR, Render::GraphicParamsCR) override;
    DGNPLATFORM_EXPORT Render::GraphicBuilderPtr _CreateSubGraphic(TransformCR) const override;

    virtual bool _IsOpen() const override { return m_isOpen; }
    virtual StatusInt _Close() override { m_isOpen = false; return SUCCESS; }
    virtual StatusInt _EnsureClosed() override { return m_isOpen ? _Close() : SUCCESS; }
public:
    DGNPLATFORM_EXPORT explicit SimplifyGraphic(Render::Graphic::CreateParams const& params, IGeometryProcessorR, ViewContextR);

    virtual ~SimplifyGraphic() {}

    ViewContextR GetViewContext() const {return m_context;};

    //! Get current local to view DMatrix4d.
    DGNPLATFORM_EXPORT DMatrix4d GetLocalToView() const;

    //! Get current view to local DMatrix4d.
    DGNPLATFORM_EXPORT DMatrix4d GetViewToLocal() const;

    //! Transform an array of points in the current local coordinate system into DgnCoordSystem::View coordinates.
    //! @param[out]     viewPts     An array to receive the points in DgnCoordSystem::View. Must be dimensioned to hold \c nPts points.
    //! @param[in]      localPts    Input array in current local coordinates,
    //! @param[in]      nPts        Number of points in both arrays.
    DGNPLATFORM_EXPORT void LocalToView(DPoint4dP viewPts, DPoint3dCP localPts, int nPts) const;

    //! Transform an array of points in the current local coordinate system into DgnCoordSystem::View coordinates.
    //! @param[out]     viewPts     An array to receive the points in DgnCoordSystem::View. Must be dimensioned to hold \c nPts points.
    //! @param[in]      localPts    Input array in current local coordinates,
    //! @param[in]      nPts        Number of points in both arrays.
    DGNPLATFORM_EXPORT void LocalToView(DPoint3dP viewPts, DPoint3dCP localPts, int nPts) const;

    //! Transform an array of points in DgnCoordSystem::View into the current local coordinate system.
    //! @param[out]     localPts    An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
    //! @param[in]      viewPts     Input array in DgnCoordSystem::View.
    //! @param[in]      nPts        Number of points in both arrays.
    DGNPLATFORM_EXPORT void ViewToLocal(DPoint3dP localPts, DPoint4dCP viewPts, int nPts) const;

    //! Transform an array of points in DgnCoordSystem::View into the current local coordinate system.
    //! @param[out]     localPts    An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
    //! @param[in]      viewPts     Input array in DgnCoordSystem::View.
    //! @param[in]      nPts        Number of points in both arrays.
    DGNPLATFORM_EXPORT void ViewToLocal(DPoint3dP localPts, DPoint3dCP viewPts, int nPts) const;

    //! Call to stroke a CurveVector and process using IGeometryProcessor::_ProcessAsLinearSegment.
    DGNPLATFORM_EXPORT void ProcessAsLinearSegments(CurveVectorCR, bool filled);

    //! Call to output a CurveVector as individual ICurvePrimitives using IGeometryProcessor::_ProcessCurvePrimitive.
    DGNPLATFORM_EXPORT void ProcessAsCurvePrimitives(CurveVectorCR, bool filled);

    ClipVectorCP GetCurrentClip() const {return nullptr;}

    DGNPLATFORM_EXPORT void ClipAndProcessCurveVector(CurveVectorCR, bool filled);
    DGNPLATFORM_EXPORT void ClipAndProcessSolidPrimitive(ISolidPrimitiveCR);
    DGNPLATFORM_EXPORT void ClipAndProcessSurface(MSBsplineSurfaceCR);
    DGNPLATFORM_EXPORT void ClipAndProcessPolyface(PolyfaceQueryCR, bool filled);
    DGNPLATFORM_EXPORT void ClipAndProcessPolyfaceAsCurves(PolyfaceQueryCR);
    DGNPLATFORM_EXPORT void ClipAndProcessBody(ISolidKernelEntityCR);
    DGNPLATFORM_EXPORT void ClipAndProcessBodyAsPolyface(ISolidKernelEntityCR);
    DGNPLATFORM_EXPORT void ClipAndProcessText(TextStringCR);
    DGNPLATFORM_EXPORT void ClipAndProcessGlyph(DgnFontCR, DgnGlyphCR, DPoint3dCR glyphOffset);

    DGNPLATFORM_EXPORT void GetEffectiveGraphicParams(Render::GraphicParamsR graphicParams) const; // Get GraphicParams adjusted for overrides...
    Render::GraphicParamsCR GetCurrentGraphicParams() const {return m_currGraphicParams;}
    Render::GeometryParamsCR GetCurrentGeometryParams() const {return m_currGeometryParams;}

    DGNPLATFORM_EXPORT bool IsRangeTotallyInside(DRange3dCR range) const;
    DGNPLATFORM_EXPORT bool IsRangeTotallyInsideClip(DRange3dCR range) const;
    DGNPLATFORM_EXPORT bool ArePointsTotallyInsideClip(DPoint3dCP points, int nPoints) const;
    DGNPLATFORM_EXPORT bool ArePointsTotallyOutsideClip(DPoint3dCP points, int nPoints) const;

    bool GetIsPatternGraphics() const {return m_inPatternDraw;}
    bool GetIsSymbolGraphics() const {return m_inSymbolDraw;}
    bool GetIsTextGraphics() const {return m_inTextDraw;}

}; // SimplifyGraphic

//=======================================================================================
//! @ingroup GROUP_Geometry
// @bsiclass                                                    Brien.Bastings  12/15
//=======================================================================================
struct IGeometryProcessor
{
public:
    //! Specify how to process a geometric primitive that is not handled by it's specific _Process/_ProcessClipped call.
    //! When returning multiple values, priority is given to the lowest value. For example, if ISolidKernelEntity is the
    //! preferred geometry, but a Polyface is acceptable when conversion to ISolidKernelEntity isn't possible or available, 
    //! the IGeometryProcessor can return UnhandledPreference::BRep | UnhandledPreference::Facets.
    enum class UnhandledPreference
    {
        Ignore  = 0,      //!< Don't convert an unhandled geometric primitive to any other type.
        Auto    = 1,      //!< Process as "best" available type for clipping. Use facets if no curved surfaces/edges or BRep unavailable, etc.
        BRep    = 1 << 1, //!< Process region CurveVector, open CurveVector, ISolidPrimitive, MSBsplineSurface, and PolyfaceQuery as ISolidKernelEntity.
        Facet   = 1 << 2, //!< Process region CurveVector, ISolidPrimitive, MSBsplineSurface, and ISolidKernelEntity as PolyfaceHeader.
        Curve   = 1 << 3, //!< Process ISolidPrimitive, MSBsplineSurface, PolyfaceQuery, and ISolidKernelEntity as edge/face iso CurveVector. Process clipped CurveVector as open curves, drop TextString.
        Box     = 1 << 4, //!< Process TextString, Raster, and Mosasic as a simple rectangle.
    };

//! Reason geometry is being processed which may allow the ElementHandler to optimize it's output.
virtual DrawPurpose _GetProcessPurpose() const {return DrawPurpose::CaptureGeometry;}

//! Whether to output clipped geometry when viewport has clipping.
virtual bool _DoClipping() const {return false;}

//! Whether to drop pattern and process as geometric primitives.
virtual bool _DoExpandPatterns() const {return false;}

//! Whether to drop linestyle and process as geometric primitives.
virtual bool _DoExpandLineStyles(ILineStyleCP lsStyle) const {return false;}

//! Allow processor to override the default facet options.
//! @return A pointer to facet option structure to use or nullptr to use default options.
virtual IFacetOptionsP _GetFacetOptionsP() {return nullptr;}

//! Whether to include edge curves when UnhandledPreference::Curve is requested for a surface/solid geometric primitive.
virtual bool _IncludeWireframeEdges() {return true;}

//! Whether to include face iso curves when UnhandledPreference::Curve is requested for a surface/solid geometric primitive.
virtual bool _IncludeWireframeFaceIso() {return true;}

//! Determine how geometry not specifically handled the geometry _Process method should be treated.
virtual UnhandledPreference _GetUnhandledPreference(CurveVectorCR, SimplifyGraphic&) const {return UnhandledPreference::Ignore;}
virtual UnhandledPreference _GetUnhandledPreference(ISolidPrimitiveCR, SimplifyGraphic&) const {return UnhandledPreference::Ignore;}
virtual UnhandledPreference _GetUnhandledPreference(MSBsplineSurfaceCR, SimplifyGraphic&) const {return UnhandledPreference::Ignore;}
virtual UnhandledPreference _GetUnhandledPreference(PolyfaceQueryCR, SimplifyGraphic&) const {return UnhandledPreference::Ignore;}
virtual UnhandledPreference _GetUnhandledPreference(ISolidKernelEntityCR, SimplifyGraphic&) const {return UnhandledPreference::Ignore;}
virtual UnhandledPreference _GetUnhandledPreference(TextStringCR, SimplifyGraphic&) const {return UnhandledPreference::Ignore;}

//! Call SimplifyGraphic::ProcessAsLinearSegments to output a CurveVector as strokes calling this method.
virtual bool _ProcessLinearSegments(DPoint3dCP points, size_t numPoints, bool closed, bool filled, SimplifyGraphic&) {return false;}

//! Call SimplifyGraphic::ProcessAsCurvePrimitives to output a CurveVector's ICurvePrimitives calling this method.
virtual bool _ProcessCurvePrimitive(ICurvePrimitiveCR, bool closed, bool filled, SimplifyGraphic&) {return false;}

//! Called by SimplifyGraphic when not clipping (or no clips present).
//! @return true if handled or false to process according to _GetUnhandledPreference.
virtual bool _ProcessCurveVector(CurveVectorCR, bool filled, SimplifyGraphic&) {return false;}
virtual bool _ProcessSolidPrimitive(ISolidPrimitiveCR, SimplifyGraphic&) {return false;}
virtual bool _ProcessSurface(MSBsplineSurfaceCR, SimplifyGraphic&) {return false;}
virtual bool _ProcessPolyface(PolyfaceQueryCR, bool filled, SimplifyGraphic&) {return false;}
virtual bool _ProcessBody(ISolidKernelEntityCR, SimplifyGraphic&) {return false;}
virtual bool _ProcessTextString(TextStringCR, SimplifyGraphic&) {return false;}

//! Called by SimplifyGraphic when clipping (and clips are present).
//! @return true if handled or false to process according to _GetUnhandledPreference.
virtual bool _ProcessCurveVectorClipped(CurveVectorCR, bool filled, SimplifyGraphic&, ClipVectorCR) {return false;}
virtual bool _ProcessSolidPrimitiveClipped(ISolidPrimitiveCR, SimplifyGraphic&, ClipVectorCR) {return false;}
virtual bool _ProcessSurfaceClipped(MSBsplineSurfaceCR, SimplifyGraphic&, ClipVectorCR) {return false;}
virtual bool _ProcessPolyfaceClipped(PolyfaceQueryCR, bool filled, SimplifyGraphic&, ClipVectorCR) {return false;}
virtual bool _ProcessBodyClipped(ISolidKernelEntityCR, SimplifyGraphic&, ClipVectorCR) {return false;}
virtual bool _ProcessTextStringClipped(TextStringCR, SimplifyGraphic&, ClipVectorCR) {return false;}

//! Allow processor to output graphics to it's own process methods.
//! @param[in] context The current view context.
//! @remarks The implementor is responsible for setting up the ViewContext. Might want to attach/detach a viewport, etc.
virtual void _OutputGraphics(ViewContextR context) {}

}; // IGeometryProcessor

ENUM_IS_FLAGS(IGeometryProcessor::UnhandledPreference)

//=======================================================================================
//! Provides an implementation of a ViewContext and Render::Graphic suitable for 
//! collecting a "picture" of an element's graphics. The element handler's Draw method is 
//! called and the output is sent to the supplied IGeometryProcessor.
//! @ingroup GROUP_Geometry
// @bsiclass                                                    Brien.Bastings  06/2009
//=======================================================================================
struct GeometryProcessor
{
//! Visit the supplied element and send it's Draw output to the supplied processor.
//! @param[in] processor The object to send the Draw output to.
//! @param[in] source The GeometrySource to output the graphics of.
DGNPLATFORM_EXPORT static void Process(IGeometryProcessorR processor, GeometrySourceCR source);

//! Call _OutputGraphics on the supplied processor and send whatever it draws to it's process methods.
//! @param[in] processor The object to send the Draw output to.
//! @param[in] dgnDb The DgnDb to set for the ViewContext.
DGNPLATFORM_EXPORT static void Process(IGeometryProcessorR processor, DgnDbR dgnDb);

}; // GeometryProcessor

END_BENTLEY_DGN_NAMESPACE
