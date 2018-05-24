/*--------------------------------------------------------------------------------------+
|
|     $Source: JsInteropTiles.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "IModelJsNative.h"

using namespace IModelJsNative;
namespace Render = Dgn::Render;

#define NO_IMPL(ret) { BeAssert(false); return ret; }
#define NULL_IMPL NO_IMPL(nullptr)
#define RETURN_GRAPHIC { return new Render::Graphic(db); }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   05/18
//=======================================================================================
struct System : Render::System
{
    int _Initialize(void*, bool) override NO_IMPL(0)
    Render::TargetPtr _CreateTarget(Render::Device&, double) override NULL_IMPL
    Render::TargetPtr _CreateOffscreenTarget(Render::Device&, double) override NULL_IMPL
    Render::GraphicBuilderPtr _CreateGraphic(Render::GraphicBuilder::CreateParams const&) const override NULL_IMPL
    Render::GraphicPtr _CreateSprite(Render::ISprite&, DPoint3dCR, DPoint3dCR, int, Dgn::DgnDbR) const override NULL_IMPL
    Render::GraphicPtr _CreateViewlet(Render::GraphicBranch&, Render::PlanCR, Render::ViewletPosition const&) const override NULL_IMPL
    Render::TexturePtr _CreateGeometryTexture(Render::GraphicCR, DRange2dCR, bool, bool) const override NULL_IMPL
    Render::LightPtr _CreateLight(Dgn::Lighting::Parameters const&, DVec3dCP, DPoint3dCP) const override NULL_IMPL

    Render::MaterialPtr _FindMaterial(Render::MaterialKeyCR, Dgn::DgnDbR) const override NULL_IMPL
    Render::MaterialPtr _CreateMaterial(Render::Material::CreateParams const&, Dgn::DgnDbR) const override NULL_IMPL
    Render::TexturePtr _FindTexture(Render::TextureKeyCR, Dgn::DgnDbR) const override NULL_IMPL
    Render::TexturePtr _CreateTexture(Render::ImageCR, Dgn::DgnDbR, Render::Texture::CreateParams const&) const override NULL_IMPL
    Render::TexturePtr _CreateTexture(Render::ImageSourceCR, Render::Image::BottomUp, Dgn::DgnDbR, Render::Texture::CreateParams const&) const override NULL_IMPL
    Render::TexturePtr _GetTexture(Render::GradientSymbCR, Dgn::DgnDbR) const override NULL_IMPL

    uint32_t _GetMaxFeaturesPerBatch() const override { return 2048; }
    Render::GraphicPtr _CreateTriMesh(Render::TriMeshArgsCR args, Dgn::DgnDbR db) const override RETURN_GRAPHIC
    Render::GraphicPtr _CreateIndexedPolylines(Render::IndexedPolylineArgsCR args, Dgn::DgnDbR db) const override RETURN_GRAPHIC
    Render::GraphicPtr _CreatePointCloud(Render::PointCloudArgsCR args, Dgn::DgnDbR db) const override RETURN_GRAPHIC
    Render::GraphicPtr _CreateGraphicList(bvector<Render::GraphicPtr>&& graphics, Dgn::DgnDbR db) const override RETURN_GRAPHIC
    Render::GraphicPtr _CreateBranch(Render::GraphicBranch&& branch, Dgn::DgnDbR db, TransformCR, ClipVectorCP) const override RETURN_GRAPHIC
    Render::GraphicPtr _CreateBatch(Render::GraphicR graphic, Render::FeatureTable&& features, DRange3dCR range) const override { return new Render::Graphic(graphic.GetDgnDb()); }
};

