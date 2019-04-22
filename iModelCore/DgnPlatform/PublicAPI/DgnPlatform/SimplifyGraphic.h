/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
struct SimplifyGraphic : Render::GraphicBuilder
{
    DEFINE_T_SUPER(Render::Graphic);

protected:
    IGeometryProcessorR m_processor;
    ViewContextR m_context;
    IFacetOptionsPtr m_facetOptions;
    Render::GraphicParams m_currGraphicParams;
    Render::GeometryParams m_currGeometryParams;
    GeometryStreamEntryId m_currGeomEntryId;
    bool m_isOpen = true;

    DGNPLATFORM_EXPORT void _ActivateGraphicParams(Render::GraphicParamsCR graphicParams, Render::GeometryParamsCP geomParams) override;
    DGNPLATFORM_EXPORT void _AddLineString(int numPoints, DPoint3dCP points) override;
    DGNPLATFORM_EXPORT void _AddLineString2d(int numPoints, DPoint2dCP points, double zDepthe) override;
    DGNPLATFORM_EXPORT void _AddPointString(int numPoints, DPoint3dCP points) override;
    DGNPLATFORM_EXPORT void _AddPointString2d(int numPoints, DPoint2dCP points, double zDepthe) override;
    DGNPLATFORM_EXPORT void _AddShape(int numPoints, DPoint3dCP points, bool filled) override;
    DGNPLATFORM_EXPORT void _AddShape2d(int numPoints, DPoint2dCP points, bool filled, double zDepth) override;
    DGNPLATFORM_EXPORT void _AddTriStrip(int numPoints, DPoint3dCP points, AsThickenedLine usageFlags) override;
    DGNPLATFORM_EXPORT void _AddTriStrip2d(int numPoints, DPoint2dCP points, AsThickenedLine usageFlags, double zDepth) override;
    DGNPLATFORM_EXPORT void _AddArc(DEllipse3dCR ellipse, bool isEllipse, bool filled) override;
    DGNPLATFORM_EXPORT void _AddArc2d(DEllipse3dCR ellipse, bool isEllipse, bool filled, double zDepth) override;
    DGNPLATFORM_EXPORT void _AddBSplineCurve(MSBsplineCurveCR curve, bool filled) override;
    DGNPLATFORM_EXPORT void _AddBSplineCurve2d(MSBsplineCurveCR curve, bool filled, double zDepth) override;
    DGNPLATFORM_EXPORT void _AddCurveVector(CurveVectorCR curves, bool isFilled) override;
    DGNPLATFORM_EXPORT void _AddCurveVector2d(CurveVectorCR curves, bool isFilled, double zDepth) override;
    DGNPLATFORM_EXPORT void _AddSolidPrimitive(ISolidPrimitiveCR primitive) override;
    DGNPLATFORM_EXPORT void _AddBSplineSurface(MSBsplineSurfaceCR) override;
    DGNPLATFORM_EXPORT void _AddPolyface(PolyfaceQueryCR meshData, bool filled = false) override;
    DGNPLATFORM_EXPORT void _AddBody(IBRepEntityCR entity) override;
    DGNPLATFORM_EXPORT void _AddTextString(TextStringCR text) override;
    DGNPLATFORM_EXPORT void _AddTextString2d(TextStringCR text, double zDepth) override;
    DGNPLATFORM_EXPORT void _AddDgnOle(Render::DgnOleDraw*) override;
    DGNPLATFORM_EXPORT void _AddSubGraphic(Render::GraphicR, TransformCR, Render::GraphicParamsCR, ClipVectorCP clip) override;
    DGNPLATFORM_EXPORT Render::GraphicBuilderPtr _CreateSubGraphic(TransformCR, ClipVectorCP clip) const override;
    DGNPLATFORM_EXPORT bool _WantStrokeLineStyle(Render::LineStyleSymbCR symb, IFacetOptionsPtr& facetOptions) override;
    DGNPLATFORM_EXPORT bool _WantStrokePattern(PatternParamsCR pattern) override;

    bool _IsOpen() const override {return m_isOpen;}
    Render::GraphicPtr _Finish() override;

    GeometryStreamEntryIdCP _GetGeometryStreamEntryId() const override {return &m_currGeomEntryId;}
    void _SetGeometryStreamEntryId(GeometryStreamEntryIdCP entry) override {if (nullptr != entry) m_currGeomEntryId = *entry; else m_currGeomEntryId.Init();}

public:
    DGNPLATFORM_EXPORT explicit SimplifyGraphic(Render::GraphicBuilder::CreateParams const& params, IGeometryProcessorR, ViewContextR);

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

    DGNPLATFORM_EXPORT void ClipAndProcessCurveVector(CurveVectorCR, bool filled);
    DGNPLATFORM_EXPORT void ClipAndProcessSolidPrimitive(ISolidPrimitiveCR);
    DGNPLATFORM_EXPORT void ClipAndProcessSurface(MSBsplineSurfaceCR);
    DGNPLATFORM_EXPORT void ClipAndProcessPolyface(PolyfaceQueryCR, bool filled);
    DGNPLATFORM_EXPORT void ClipAndProcessPolyfaceAsCurves(PolyfaceQueryCR);
    DGNPLATFORM_EXPORT void ClipAndProcessBody(IBRepEntityCR);
    DGNPLATFORM_EXPORT void ClipAndProcessBodyAsPolyface(IBRepEntityCR);
    DGNPLATFORM_EXPORT void ClipAndProcessText(TextStringCR);
    DGNPLATFORM_EXPORT void ClipAndProcessTriMesh(Render::TriMeshArgsCR);

    Render::GraphicParamsCR GetCurrentGraphicParams() const {return m_currGraphicParams;}
    Render::GeometryParamsCR GetCurrentGeometryParams() const {return m_currGeometryParams;}
    GeometryStreamEntryIdCR GetCurrentGeometryStreamEntryId() const {return m_currGeomEntryId;}
    IGeometryProcessorR GetGeometryProcesor() const {return m_processor;}

    DGNPLATFORM_EXPORT bool IsRangeTotallyInside(DRange3dCR range) const;
    DGNPLATFORM_EXPORT bool IsRangeTotallyInsideClip(DRange3dCR range) const;
    DGNPLATFORM_EXPORT bool ArePointsTotallyInsideClip(DPoint3dCP points, int nPoints) const;
    DGNPLATFORM_EXPORT bool ArePointsTotallyOutsideClip(DPoint3dCP points, int nPoints) const;

    struct Base : Render::Graphic
    {
    public:
        explicit Base(DgnDbR db) : Render::Graphic(db) { }
    };
}; // SimplifyGraphic

//=======================================================================================
//! @ingroup GROUP_Geometry
// @bsiclass                                                    Brien.Bastings  12/15
//=======================================================================================
struct IGeometryProcessor
{
public:
    //! Specify how to process a geometric primitive that is not handled by it's specific _Process/_ProcessClipped call.
    //! When returning multiple values, priority is given to the lowest value. For example, if IBRepEntity is the
    //! preferred geometry, but a Polyface is acceptable when conversion to IBRepEntity isn't possible or available, 
    //! the IGeometryProcessor can return UnhandledPreference::BRep | UnhandledPreference::Facets.
    enum class UnhandledPreference
    {
        Ignore  = 0,      //!< Don't convert an unhandled geometric primitive to any other type.
        Auto    = 1,      //!< Process as "best" available type for clipping. Use facets if no curved surfaces/edges or BRep unavailable, etc.
        BRep    = 1 << 1, //!< Process region CurveVector, open CurveVector, ISolidPrimitive, MSBsplineSurface, and PolyfaceQuery as IBRepEntity.
        Facet   = 1 << 2, //!< Process region CurveVector, ISolidPrimitive, MSBsplineSurface, and IBRepEntity as PolyfaceHeader.
        Curve   = 1 << 3, //!< Process ISolidPrimitive, MSBsplineSurface, PolyfaceQuery, and IBRepEntity as edge/face iso CurveVector. Process clipped CurveVector as open curves, drop TextString.
        Box     = 1 << 4, //!< Process TextString, Raster, and Mosasic as a simple rectangle.
    };

    //! Specify how to process geometry that originated as a PolyfaceQuery and is not compatible with the current IFacetOptions.
    enum class IncompatiblePolyfacePreference
    {
        Ignore   = 0, //!< Don't process a PolyfaceQuery that is not compatible with the current IFacetOptions.
        Original = 1, //!< Process PolyfaceQuery as is. Use IFacetOptions only when creating a PolyfaceHeader from other geometry types in response to UnhandledPreference::Facet.
        Modify   = 2, //!< Create a new PolyfaceHeader from PolyfaceQuery that complies with current IFacetOptions, this can be an expensive operation.
    };

//! Reason geometry is being processed which may allow the ElementHandler to optimize it's output.
virtual DrawPurpose _GetProcessPurpose() const {return DrawPurpose::CaptureGeometry;}

//! Whether to output clipped geometry when viewport has clipping.
virtual bool _DoClipping() const {return false;}

//! Whether to drop patterns and process as geometric primitives.
virtual bool _DoPatternStroke(PatternParamsCR, SimplifyGraphic&) const {return false;}

//! Whether to drop linestyles and process as geometric primitives.
virtual bool _DoLineStyleStroke(Render::LineStyleSymbCR, IFacetOptionsPtr&, SimplifyGraphic&) const {return false;}

//! Allow processor to override the default facet options.
//! @return A pointer to facet option structure to use or nullptr to use default options.
virtual IFacetOptionsP _GetFacetOptionsP() {return nullptr;}

//! Adjust z-depth for 2d geometry. By default, z is set to zero.
virtual double _AdjustZDepth(double zDepth) {return 0.0;}

//! Determine how geometry that originated as a PolyfaceQuery and is not compatible with the current IFacetOptions is treated.
virtual IncompatiblePolyfacePreference _GetIncompatiblePolyfacePreference(PolyfaceQueryCR, SimplifyGraphic&) const {return IncompatiblePolyfacePreference::Modify;}

//! Determine how geometry not specifically handled the geometry _Process method should be treated.
virtual UnhandledPreference _GetUnhandledPreference(CurveVectorCR, SimplifyGraphic&) const {return UnhandledPreference::Ignore;}
virtual UnhandledPreference _GetUnhandledPreference(ISolidPrimitiveCR, SimplifyGraphic&) const {return UnhandledPreference::Ignore;}
virtual UnhandledPreference _GetUnhandledPreference(MSBsplineSurfaceCR, SimplifyGraphic&) const {return UnhandledPreference::Ignore;}
virtual UnhandledPreference _GetUnhandledPreference(PolyfaceQueryCR, SimplifyGraphic&) const {return UnhandledPreference::Ignore;}
virtual UnhandledPreference _GetUnhandledPreference(IBRepEntityCR, SimplifyGraphic&) const {return UnhandledPreference::Ignore;}
virtual UnhandledPreference _GetUnhandledPreference(TextStringCR, SimplifyGraphic&) const {return UnhandledPreference::Ignore;}
virtual UnhandledPreference _GetUnhandledPreference(Render::TriMeshArgsCR, SimplifyGraphic&) const {return UnhandledPreference::Ignore;}

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
virtual bool _ProcessBody(IBRepEntityCR, SimplifyGraphic&) {return false;}
virtual bool _ProcessTextString(TextStringCR, SimplifyGraphic&) {return false;}
virtual bool _ProcessTriMesh(Render::TriMeshArgsCR args, SimplifyGraphic&) {return false; }

//! Called by SimplifyGraphic when clipping (and clips are present).
//! @return true if handled or false to process according to _GetUnhandledPreference.
virtual bool _ProcessCurveVectorClipped(CurveVectorCR, bool filled, SimplifyGraphic&, ClipVectorCR) {return false;}
virtual bool _ProcessSolidPrimitiveClipped(ISolidPrimitiveCR, SimplifyGraphic&, ClipVectorCR) {return false;}
virtual bool _ProcessSurfaceClipped(MSBsplineSurfaceCR, SimplifyGraphic&, ClipVectorCR) {return false;}
virtual bool _ProcessPolyfaceClipped(PolyfaceQueryCR, bool filled, SimplifyGraphic&, ClipVectorCR) {return false;}
virtual bool _ProcessBodyClipped(IBRepEntityCR, SimplifyGraphic&, ClipVectorCR) {return false;}
virtual bool _ProcessTextStringClipped(TextStringCR, SimplifyGraphic&, ClipVectorCR) {return false;}
virtual bool _ProcessTriMeshClipped(Render::TriMeshArgsCR args, SimplifyGraphic&, ClipVectorCR) {return false; }

//! Allow processor to initialize per-geometry state information before any _Process method is called.
//! @remarks Can be used to allow a _Process method to distinguish between the natural geometry type and geometry output according to the UnhandledPreference.
virtual void _OnNewGeometry() {}

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
