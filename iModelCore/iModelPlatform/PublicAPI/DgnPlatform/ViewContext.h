/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "DgnPlatform.h"
#include "Render.h"
#include "ClipVector.h"
#include "TransformClipStack.h"
#include "DgnModel.h"

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ILineStyleComponent
{
    virtual bool _IsContinuous() const = 0;
    virtual bool _HasWidth() const = 0;
    virtual double _GetMaxWidth() const = 0;
    virtual double _GetLength() const = 0;
    virtual StatusInt _StrokeLineString(LineStyleContextR, Render::LineStyleSymbR, DPoint3dCP, int nPts, bool isClosed) const = 0;
    virtual StatusInt _StrokeLineString2d(LineStyleContextR, Render::LineStyleSymbR, DPoint2dCP, int nPts, double zDepth, bool isClosed) const = 0;
    virtual StatusInt _StrokeArc(LineStyleContextR, Render::LineStyleSymbR, DEllipse3dCR, bool is3d, double zDepth, bool isClosed) const = 0;
    virtual StatusInt _StrokeBSplineCurve(LineStyleContextR, Render::LineStyleSymbR, MSBsplineCurveCR, bool is3d, double zDepth) const = 0;
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ILineStyle
{
    virtual Utf8CP _GetName() const = 0;
    virtual ILineStyleComponent const* _GetComponent() const = 0;
    virtual bool _IsSnappable() const = 0;
    virtual Render::Texture* _GetTexture(double& textureWidth, ViewContextR, Render::GeometryParamsCR, bool createGeometryTexture) = 0;
    double GetMaxWidth () const { return nullptr == _GetComponent() ? 0.0 : _GetComponent()->_GetMaxWidth(); }
    double GetLength () const { return nullptr == _GetComponent() ? 0.0 : _GetComponent()->_GetLength(); }
};

//=======================================================================================
//! @ingroup GROUP_ViewContext
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ViewContext : NonCopyableClass, CheckStop
{
    friend struct SimplifyGraphic;

    struct AreaPatternTolerance
    {
    private:
        double  m_angleTolerance;
        double  m_chordTolerance;
    public:
        explicit AreaPatternTolerance(double chord=0.0, Angle angle=Angle::FromDegrees(20.0)) : m_angleTolerance(angle.Radians()), m_chordTolerance(chord) { }

        double GetAngleTolerance() const { return m_angleTolerance; }
        double GetChordTolerance() const { return m_chordTolerance; }
    };

    //=======================================================================================
    //! Describes how the ViewContext prefers text to be output.
    // @bsistruct
    //=======================================================================================
    enum class GlyphOutputType
    {
        Box, //!< Output an opaque range box.
        Raster, //!< Output a range box mapped to a raster image of the glyph.
        Stroked, //!< Stroke the glyph curves.
    };
protected:
    DgnDbP m_dgndb = nullptr;
    bool m_is3dView = true;
    bool m_wantMaterials = false;
    bool m_ignoreViewRange = false;
    Render::ViewFlags m_viewflags;
    DrawPurpose m_purpose;
    DMap4d m_worldToNpc;
    DMap4d m_worldToView;
    Render::FrustumPlanes m_frustumPlanes;
    ClipVectorCPtr m_volume;
    Utf8String m_auxChannel;

    DGNPLATFORM_EXPORT virtual StatusInt _OutputGeometry(GeometrySourceCR);
    DGNPLATFORM_EXPORT virtual void _AddSubGraphic(Render::GraphicBuilderR, DgnGeometryPartId, TransformCR, Render::GeometryParamsR);
    virtual void _OutputGraphic(Render::GraphicR, GeometrySourceCP) {}
    DGNPLATFORM_EXPORT virtual Render::GraphicPtr _StrokeGeometry(GeometrySourceCR source, double pixelSize);

    DGNPLATFORM_EXPORT virtual bool _WantAreaPatterns();
    DGNPLATFORM_EXPORT virtual void _DrawAreaPattern(Render::GraphicBuilderR, CurveVectorCR, Render::GeometryParamsR, bool doCook);
    virtual void _BeginAreaPattern(Render::GraphicBuilderR, CurveVectorCR, Render::GeometryParamsR) { }
    virtual void _EndAreaPattern(Render::GraphicBuilderR, CurveVectorCR, Render::GeometryParamsR) { }
    virtual AreaPatternTolerance _GetAreaPatternTolerance(CurveVectorCR) {return AreaPatternTolerance();}

    DGNPLATFORM_EXPORT virtual bool _WantLineStyles();
    DGNPLATFORM_EXPORT virtual void _DrawStyledCurveVector(Render::GraphicBuilderR, CurveVectorCR, Render::GeometryParamsR, bool doCook);

    DGNPLATFORM_EXPORT virtual StatusInt _VisitGeometry(GeometrySourceCR);
    DGNPLATFORM_EXPORT virtual bool _AnyPointVisible(DPoint3dCP worldPoints, int nPts, double tolerance);
    virtual bool _IsSubCategoryVisible(DgnSubCategoryId subCategoryId) { return true; }
    virtual Render::GraphicBuilderPtr _CreateGraphic(Render::GraphicBuilder::CreateParams const& params) = 0;
    virtual Render::GraphicPtr _CreateBranch(Render::GraphicBranch&, DgnDbR db, TransformCR tf, ClipVectorCP clips) = 0;
    DGNPLATFORM_EXPORT virtual void _CookGeometryParams(Render::GeometryParamsR, Render::GraphicParamsR);
    DGNPLATFORM_EXPORT virtual void _SetDgnDb(DgnDbR);
    DGNPLATFORM_EXPORT virtual StatusInt _VisitElement(DgnElementId elementId, bool allowLoad);
    DGNPLATFORM_EXPORT virtual Render::MaterialPtr _GetMaterial(RenderMaterialId id) const;
    DGNPLATFORM_EXPORT virtual Render::TexturePtr _CreateTexture(Render::ImageCR image) const;
    DGNPLATFORM_EXPORT virtual Render::TexturePtr _CreateTexture(Render::ImageSourceCR source, Render::Image::BottomUp bottomUp) const;
    virtual Render::SystemP _GetRenderSystem() const { return nullptr; }
    virtual bool _WantGlyphBoxes(double sizeInPixels) const { return false; }
    DGNPLATFORM_EXPORT virtual double _DepthFromDisplayPriority(int32_t priority) const;
    DGNPLATFORM_EXPORT ViewContext();

public:
    DMap4dCR GetWorldToView() const {return m_worldToView;}
    DMap4dCR GetWorldToNpc() const {return m_worldToNpc;}
    bool GetWantMaterials() {return m_wantMaterials;};
    void SetWantMaterials(bool wantMaterials) {m_wantMaterials = wantMaterials;}
    DGNPLATFORM_EXPORT StatusInt Attach(DrawPurpose purpose);
    enum class WantBoresite : bool {Yes=true, No=false};
    DGNPLATFORM_EXPORT bool IsRangeVisible(DRange3dCR range, double tolerance=1.0e-8);
    DGNPLATFORM_EXPORT bool IsGeometryVisible(Render::GeometryParamsCR geomParams, DRange3dCP range);
    DGNPLATFORM_EXPORT bool IsPointVisible(DPoint3dCR worldPoint, WantBoresite boresite, double tolerance=1.0e-8);
    DGNPLATFORM_EXPORT Frustum GetFrustum();
    Render::FrustumPlanes const& GetFrustumPlanes() const {return m_frustumPlanes;}
    void OutputGraphic(Render::GraphicR graphic, GeometrySourceCP source) {_OutputGraphic(graphic, source);}
    void SetActiveVolume(ClipVectorCR volume) {m_volume=&volume;}
    ClipVectorCPtr GetActiveVolume() const {return m_volume;}
    Utf8StringCR GetActiveAuxChannel() const { return m_auxChannel; }
    void SetActiveAuxChannel(Utf8CP auxChannel) { m_auxChannel = nullptr == auxChannel ? Utf8String() : Utf8String(auxChannel); }

    Render::GraphicBuilderPtr CreateSceneGraphic(TransformCR tf=Transform::FromIdentity())
        { return _CreateGraphic(Render::GraphicBuilder::CreateParams::Scene(GetDgnDb(), tf)); }

    Render::TexturePtr CreateTexture(Render::ImageCR image) const { return _CreateTexture(image); }
    Render::TexturePtr CreateTexture(Render::ImageSourceCR source, Render::Image::BottomUp bottomUp=Render::Image::BottomUp::No) const
        { return _CreateTexture(source, bottomUp); }

    Render::GraphicPtr CreateBranch(Render::GraphicBranch& branch, DgnDbR db, TransformCR tf, ClipVectorCP clips=nullptr) {return _CreateBranch(branch, db, tf, clips);}
    void AddSubGraphic(Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, Render::GeometryParamsR geomParams) {return _AddSubGraphic(graphic, partId, subToGraphic, geomParams);}
    StatusInt VisitGeometry(GeometrySourceCR elem) {return _VisitGeometry(elem);}
    Render::MaterialPtr GetMaterial(RenderMaterialId id) const { return _GetMaterial(id); }

    DGNPLATFORM_EXPORT void NpcToView(DPoint3dP viewPts, DPoint3dCP npcPts, int nPts) const;
    DGNPLATFORM_EXPORT void ViewToNpc(DPoint3dP npcPts, DPoint3dCP viewPts, int nPts) const;
    DGNPLATFORM_EXPORT void NpcToWorld(DPoint3dP worldPts, DPoint3dCP npcPts, int nPts) const;
    DGNPLATFORM_EXPORT void WorldToNpc(DPoint3dP npcPts, DPoint3dCP worldPts, int nPts) const;
    DGNPLATFORM_EXPORT void WorldToView(DPoint4dP viewPts, DPoint3dCP worldPts, int nPts) const;
    DGNPLATFORM_EXPORT void WorldToView(DPoint3dP viewPts, DPoint3dCP worldPts, int nPts) const;
    DGNPLATFORM_EXPORT void WorldToView(Point2dP viewPts, DPoint3dCP worldPts, int nPts) const;
    DGNPLATFORM_EXPORT void WorldToView(DPoint2dP viewPts, DPoint3dCP worldPts, int nPts) const;
    DGNPLATFORM_EXPORT void ViewToWorld(DPoint3dP worldPts, DPoint4dCP viewPts, int nPts) const;
    DGNPLATFORM_EXPORT void ViewToWorld(DPoint3dP worldPts, DPoint3dCP viewPts, int nPts) const;

    Render::ViewFlags GetViewFlags() const {return m_viewflags;}
    void SetViewFlags(Render::ViewFlags flags) {m_viewflags = flags;}
    DgnDbR GetDgnDb() const {BeAssert(nullptr != m_dgndb); return *m_dgndb;}
    void SetDgnDb(DgnDbR dgnDb) {return _SetDgnDb(dgnDb);}
    DrawPurpose GetDrawPurpose() const {return m_purpose;}

    Render::SystemP GetRenderSystem() const {return _GetRenderSystem();}

    bool Is3dView() const {return m_is3dView;}
    void CookGeometryParams(Render::GeometryParamsR geomParams, Render::GraphicParamsR graphicParams) {_CookGeometryParams(geomParams, graphicParams);}
    DGNPLATFORM_EXPORT void CookGeometryParams(Render::GeometryParamsR geomParams, Render::GraphicBuilderR graphic);
    bool WantAreaPatterns() {return _WantAreaPatterns();}
    void DrawAreaPattern(Render::GraphicBuilderR graphic, CurveVectorCR boundary, Render::GeometryParamsR params, bool doCook = true) {_DrawAreaPattern(graphic, boundary, params, doCook);}
    AreaPatternTolerance GetAreaPatternTolerance(CurveVectorCR boundary) {return _GetAreaPatternTolerance(boundary);}

    bool WantLineStyles() {return _WantLineStyles();}
    void DrawStyledCurveVector(Render::GraphicBuilderR graphic, CurveVectorCR curve, Render::GeometryParamsR params, bool doCook = true) {_DrawStyledCurveVector(graphic, curve, params, doCook);}

    StatusInt VisitElement(DgnElementId elementId, bool allowLoad=true) {return _VisitElement(elementId, allowLoad);}

    bool CheckStop() {return _CheckStop();}

    // Given the maximum dimension of a text string in pixels, return how its glyphs should be output.
    // Note: Raster is only supported for TrueType fonts, and only if a raster image can successfully be created from the glyph - if
    // raster is requested and cannot be produced, the glyph will be stroked instead.
    virtual GlyphOutputType GetGlyphOutputType(double glyphSizeInPixels) const { return GlyphOutputType::Stroked; }

    double DepthFromDisplayPriority(int32_t priority) const {return _DepthFromDisplayPriority(priority);}
}; // ViewContext

END_BENTLEY_DGN_NAMESPACE
