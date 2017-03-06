/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/NullContext.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "ViewContext.h"

BEGIN_BENTLEY_DGN_NAMESPACE
/*=================================================================================**//**
* Context that doesn't draw anything. 
* @bsiclass                                                     KeithBentley    01/02
+===============+===============+===============+===============+===============+======*/
struct NullContext : ViewContext
{
    DEFINE_T_SUPER(ViewContext)

    //=======================================================================================
    // @bsistruct                                                   Paul.Connelly   03/17
    //=======================================================================================
    struct NullGraphic : Render::Graphic
    {
        NullGraphic(Render::Graphic::CreateParams const& params) : Render::Graphic(params) { }
    };

    /*=================================================================================**//**
      @bsiclass                                                     Brien.Bastings  09/12
    +===============+===============+===============+===============+===============+======*/
    struct NullGraphicBuilder : Render::GraphicBuilder
    {
        Transform m_localToWorldTransform;
        DgnDbR m_dgndb;
        bool m_isOpen = true;

        NullGraphicBuilder(Render::Graphic::CreateParams const& params) : m_localToWorldTransform(params.m_placement), m_dgndb(params.m_dgndb) {}

        bool _IsOpen() const override {return m_isOpen;}
        void _ActivateGraphicParams(Render::GraphicParamsCR, Render::GeometryParamsCP) override {}
        void _AddLineString(int numPoints, DPoint3dCP points) override {}
        void _AddLineString2d(int numPoints, DPoint2dCP points, double zDepth) override {}
        void _AddPointString(int numPoints, DPoint3dCP points) override {}
        void _AddPointString2d(int numPoints, DPoint2dCP points, double zDepthe) override {}
        void _AddShape(int numPoints, DPoint3dCP points, bool filled) override {}
        void _AddShape2d(int numPoints, DPoint2dCP points, bool filled, double zDepth) override {}
        void _AddTriStrip(int numPoints, DPoint3dCP points, int32_t usageFlags) override {}
        void _AddTriStrip2d(int numPoints, DPoint2dCP points, int32_t usageFlags, double zDepth) override {}
        void _AddArc(DEllipse3dCR ellipse, bool isEllipse, bool filled) override {}
        void _AddArc2d(DEllipse3dCR ellipse, bool isEllipse, bool filled, double zDepth) override {}
        void _AddBSplineCurve(MSBsplineCurveCR curve, bool filled) override {}
        void _AddBSplineCurve2d(MSBsplineCurveCR curve, bool filled, double zDepth) override {}
        void _AddCurveVector(CurveVectorCR curves, bool isFilled) override {}
        void _AddCurveVector2d(CurveVectorCR curves, bool isFilled, double zDepth) override {}
        void _AddSolidPrimitive(ISolidPrimitiveCR primitive) override {}
        void _AddBSplineSurface(MSBsplineSurfaceCR surface) override {}
        void _AddPolyface(PolyfaceQueryCR meshData, bool filled = false) override {}
        void _AddTriMesh(TriMeshArgs const& args)  override {}
        void _AddIndexedPolylines(IndexedPolylineArgs const& args) override {}
        void _AddBody(IBRepEntityCR) override {}
        void _AddTextString(TextStringCR text) override {}
        void _AddTextString2d(TextStringCR text, double zDepth) override {}
        void _AddTile(Render::TextureCR tile, TileCorners const& corners) override {}
        void _AddDgnOle(Render::DgnOleDraw*) override {}
        void _AddPointCloud(int32_t numPoints, DPoint3dCR origin, FPoint3d const* points, ByteCP colors) override {}
        void _AddSubGraphic(Render::GraphicR, TransformCR, Render::GraphicParamsCR, ClipVectorCP) override {}
        Render::GraphicBuilderPtr _CreateSubGraphic(TransformCR transform, ClipVectorCP) const override {return new NullGraphicBuilder(Render::Graphic::CreateParams(GetDgnDb(), transform));}
        DgnDbR _GetDgnDb() const override {return m_dgndb;}
        TransformCR _GetLocalToWorldTransform() const override {return m_localToWorldTransform;}
        virtual Render::GraphicPtr _Finish() { m_isOpen = false; return new NullGraphic(GetCreateParams()); }
    };

protected:
    Render::GraphicBuilderPtr _CreateGraphic(Render::Graphic::CreateParams const& params) override {return new NullGraphicBuilder(params);}
    Render::GraphicPtr _CreateBranch(Render::GraphicBranch&, Render::Graphic::CreateParams const& params, ClipVectorCP clips) override {return new NullGraphic(params);}

public:
    NullContext() {m_ignoreViewRange = true;}
};

//=======================================================================================
// Caclulate the view-aligned range of all elements either within a view or from the view's non-range criteria.
// @bsiclass                                                    Keith.Bentley   02/16
//=======================================================================================
struct FitContext : NullContext
{
    DEFINE_T_SUPER(NullContext)

    FitViewParams m_params;
    Transform m_trans;       // usually view transform 
    DRange3d m_fitRange;     // union of all view-aligned element ranges
    mutable DRange3d m_lastRange;    // last view-aligned range tested

    void AcceptRangeElement(DgnElementId id);
    StatusInt _InitContextForView() override;
    StatusInt _VisitGeometry(GeometrySourceCR source) override;
    bool _ScanRangeFromPolyhedron() override;
    RangeIndex::Traverser::Accept _CheckRangeTreeNode(RangeIndex::FBoxCR, bool) const override;
    bool IsRangeContained(RangeIndex::FBoxCR range) const;
    DGNPLATFORM_EXPORT void ExtendFitRange(ElementAlignedBox3dCR box, TransformCR placement);
    DGNPLATFORM_EXPORT void ExtendFitRange(AxisAlignedBox3dCR elementBox);

    FitContext(FitViewParams const& params) : m_params(params) {m_fitRange.Init();}
    FitViewParams const& GetParams() const {return m_params;}
};

END_BENTLEY_DGN_NAMESPACE
