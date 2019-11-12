/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/RenderPrimitives.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES

namespace FakeRender
{
DEFINE_POINTER_SUFFIX_TYPEDEFS(FakeSystem);
DEFINE_POINTER_SUFFIX_TYPEDEFS(FakeGraphicBuilder);
DEFINE_POINTER_SUFFIX_TYPEDEFS(GraphicProcessor);
DEFINE_POINTER_SUFFIX_TYPEDEFS(GraphicProcessorSystem);

DEFINE_REF_COUNTED_PTR(FakeGraphicBuilder);
DEFINE_REF_COUNTED_PTR(GraphicProcessorSystem);

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   10/17
//=======================================================================================
struct FakeGraphic : Graphic
{
protected:
    FakeGraphic(DgnDbR db) : Graphic(db) { }
public:
    static GraphicPtr Create(DgnDbR db) { return new FakeGraphic(db); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   10/17
//=======================================================================================
struct FakeGraphicBuilder : GraphicBuilder
{
protected:
    bool m_isOpen = true;

    FakeGraphicBuilder(CreateParamsCR params) : GraphicBuilder(params) { }

    bool _IsOpen() const override { return m_isOpen; }
    GraphicPtr _Finish() override { m_isOpen = false; return FakeGraphic::Create(GetDgnDb()); }

    void _ActivateGraphicParams(GraphicParamsCR graphicParams, GeometryParamsCP geomParams) { }
    void _AddLineString(int numPoints, DPoint3dCP points) { }
    void _AddLineString2d(int numPoints, DPoint2dCP points, double zDepth) { }
    void _AddPointString(int numPoints, DPoint3dCP points) { }
    void _AddPointString2d(int numPoints, DPoint2dCP points, double zDepth) { }
    void _AddShape(int numPoints, DPoint3dCP points, bool filled) { }
    void _AddShape2d(int numPoints, DPoint2dCP points, bool filled, double zDepth) { }
    void _AddTriStrip(int numPoints, DPoint3dCP points, AsThickenedLine asThickenedLine) { }
    void _AddTriStrip2d(int numPoints, DPoint2dCP points, AsThickenedLine asThickenedLine, double zDepth) { }
    void _AddArc(DEllipse3dCR ellipse, bool isEllipse, bool filled) { }
    void _AddArc2d(DEllipse3dCR ellipse, bool isEllipse, bool filled, double zDepth) { }
    void _AddBSplineCurve(MSBsplineCurveCR curve, bool filled) { }
    void _AddBSplineCurve2d(MSBsplineCurveCR curve, bool filled, double zDepth) { }
    void _AddCurveVector(CurveVectorCR curves, bool isFilled) { }
    void _AddCurveVector2d(CurveVectorCR curves, bool isFilled, double zDepth) { }
    void _AddSolidPrimitive(ISolidPrimitiveCR primitive) { }
    void _AddBSplineSurface(MSBsplineSurfaceCR surface) { }
    void _AddPolyface(PolyfaceQueryCR meshData, bool filled = false) { }
    void _AddBody(IBRepEntityCR) { }
    void _AddTextString(TextStringCR text) { }
    void _AddTextString2d(TextStringCR text, double zDepth) { }
    void _AddDgnOle(DgnOleDraw*) { }
    void _AddSubGraphic(GraphicR, TransformCR, GraphicParamsCR, ClipVectorCP clip) { }
    GraphicBuilderPtr _CreateSubGraphic(TransformCR, ClipVectorCP clip) const { return nullptr; }
public:
    static FakeGraphicBuilderPtr Create(CreateParamsCR params) { return new FakeGraphicBuilder(params); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   01/18
//=======================================================================================
struct FakeMaterial : Material
{
    explicit FakeMaterial(CreateParams const& params) : Material(params) { }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   01/18
//=======================================================================================
struct FakeTexture : Texture
{
    explicit FakeTexture(CreateParams const& params) : Texture(params) { }
    Dimensions GetDimensions() const override { return Dimensions(0, 0); }
};

//=======================================================================================
// A do-nothing Render::System. Override the methods you want to test.
// @bsistruct                                                   Paul.Connelly   10/17
//=======================================================================================
struct FakeSystem : System
{
protected:
    int _Initialize(void*, bool) override { return SUCCESS; }

    MaterialPtr _FindMaterial(MaterialKeyCR id, DgnDbR db) const override { return nullptr; }
    MaterialPtr _CreateMaterial(Material::CreateParams const& params, DgnDbR) const override { return new FakeMaterial(params); }
    TexturePtr _FindTexture(TextureKeyCR, DgnDbR) const override { return nullptr; }
    TexturePtr _GetTexture(GradientSymbCR, DgnDbR) const override { return nullptr; }
    TexturePtr _CreateTexture(ImageCR, DgnDbR, Texture::CreateParams const& params) const override { return new FakeTexture(params); }
    TexturePtr _CreateTexture(ImageSourceCR, Image::BottomUp, DgnDbR, Texture::CreateParams const& params) const override { return new FakeTexture(params); }
    TexturePtr _CreateGeometryTexture(GraphicCR, DRange2dCR, bool, bool) const override { return new FakeTexture(Texture::CreateParams()); }
    LightPtr _CreateLight(Lighting::Parameters const&, DVec3dCP, DPoint3dCP) const override { return new Light(); }

    GraphicBuilderPtr _CreateGraphic(GraphicBuilder::CreateParamsCR params) const override { return FakeGraphicBuilder::Create(params); }

    GraphicPtr _CreateSprite(ISprite& sprite, DPoint3dCR location, DPoint3dCR xVec, int transparency, DgnDbR db) const { return FakeGraphic::Create(db); }
    GraphicPtr _CreateTriMesh(TriMeshArgsCR args, DgnDbR dgndb) const { return FakeGraphic::Create(dgndb); }
    GraphicPtr _CreateIndexedPolylines(IndexedPolylineArgsCR args, DgnDbR dgndb) const { return FakeGraphic::Create(dgndb); }
    GraphicPtr _CreatePointCloud(PointCloudArgsCR args, DgnDbR dgndb) const { return FakeGraphic::Create(dgndb); }
    GraphicPtr _CreateGraphicList(bvector<GraphicPtr>&& primitives, DgnDbR dgndb) const { return FakeGraphic::Create(dgndb); }
    GraphicPtr _CreateBranch(GraphicBranch&& branch, DgnDbR dgndb, TransformCR transform, ClipVectorCP clips) const { return FakeGraphic::Create(dgndb); }
    GraphicPtr _CreateBatch(GraphicR graphic, FeatureTable&& features, DRange3dCR range) const { return FakeGraphic::Create(graphic.GetDgnDb()); }

    uint32_t _GetMaxFeaturesPerBatch() const override { return 0x7fffffff; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   10/17
//=======================================================================================
struct GraphicProcessor
{
    virtual void Process(TriMeshArgsCR args) { }
    virtual void Process(IndexedPolylineArgsCR args) { }
    virtual void Process(PointCloudArgsCR args) { }
    virtual void Process(ISprite&, DPoint3dCR, DPoint3dCR, int) { }

    virtual void ProcessGraphicList(bvector<GraphicPtr>&&) { }
    virtual void ProcessBatch(GraphicR, FeatureTable&&) { }
    virtual void ProcessBranch(GraphicBranch&&, TransformCR, ClipVectorCP) { }

    virtual GraphicBuilderPtr CreateGraphic(SystemR system, GraphicBuilder::CreateParamsCR params) { return new PrimitiveBuilder(system, params); }
};

//=======================================================================================
// Collects arguments to primitive-creation methods like _CreateTriMesh().
// @bsistruct                                                   Paul.Connelly   10/17
//=======================================================================================
struct GraphicProcessorSystem : FakeSystem
{
protected:
    GraphicProcessorP m_proc;
public:
    GraphicBuilderPtr _CreateGraphic(GraphicBuilder::CreateParamsCR params) const override { return m_proc->CreateGraphic(*const_cast<GraphicProcessorSystem*>(this), params); }
    GraphicPtr _CreateTriMesh(TriMeshArgsCR args, DgnDbR db) const override { m_proc->Process(args); return FakeGraphic::Create(db); }
    GraphicPtr _CreateIndexedPolylines(IndexedPolylineArgsCR args, DgnDbR db) const override { m_proc->Process(args); return FakeGraphic::Create(db); }

    GraphicPtr _CreateGraphicList(bvector<GraphicPtr>&& list, DgnDbR db) const override { m_proc->ProcessGraphicList(std::move(list)); return FakeGraphic::Create(db); }
    GraphicPtr _CreateBranch(GraphicBranch&& branch, DgnDbR db, TransformCR tf, ClipVectorCP clip) const override { m_proc->ProcessBranch(std::move(branch), tf, clip); return FakeGraphic::Create(db); }
    GraphicPtr _CreateBatch(GraphicR gf, FeatureTable&& feats, DRange3dCR range) const override { m_proc->ProcessBatch(gf, std::move(feats)); return FakeGraphic::Create(gf.GetDgnDb()); }

    GraphicProcessorSystem(GraphicProcessorR proc) { Reset(proc); }

    void Reset(GraphicProcessorR proc) { m_proc = &proc; }
};

} // namespace FakeRender

