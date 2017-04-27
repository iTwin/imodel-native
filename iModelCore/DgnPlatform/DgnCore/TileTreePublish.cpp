/*-------------------------------------------------------------------------------------+                                                                                           
|
|     $Source: DgnCore/MeshTile.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <folly/BeFolly.h>

USING_NAMESPACE_BENTLEY_RENDER
                         

namespace TileTreePublish
{

DEFINE_POINTER_SUFFIX_TYPEDEFS(Tile);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Texture);
DEFINE_POINTER_SUFFIX_TYPEDEFS(RenderSystem);
DEFINE_REF_COUNTED_PTR(Tile)
DEFINE_REF_COUNTED_PTR(RenderSystem)

//=======================================================================================
// @bsiclass                                                     Ray.Bentley     04/2017
//=======================================================================================
struct Texture : Render::Texture
{
    CreateParams        m_createParams;
    Image               m_image;

    Texture (ImageCR image, Texture::CreateParams const& createParams) : m_createParams(createParams), m_image(image) { }

    Render::TileTextureImagePtr CreateTileTexture() const { return TileTextureImage::Create(ImageSource(m_image, ImageSource::Format::Png)); }
};  // Texture
    
//=======================================================================================
// @bsiclass                                                     Ray.Bentley     04/2017
//=======================================================================================
struct Tile : Render::TileNode
{
private:
    PublishableTileGeometry    m_geometry;

protected:
    Tile(DgnModelCR model, DRange3dCR range, TransformCR transformFromDgn, size_t depth, size_t siblingIndex, TileNodeP parent, double tolerance)
        : Render::TileNode(model, range, transformFromDgn, depth, siblingIndex, parent, tolerance) { m_isEmpty = false; }

    virtual WString _GetFileExtension() const override { return L"b3dm"; }
    virtual PublishableTileGeometry _GeneratePublishableGeometry(DgnDbR dgndb, TileGeometry::NormalMode normalMode, bool doSurfacesOnly, ITileGenerationFilterCP filter) const override
        {return std::move(m_geometry);}

public:
    static  TilePtr Create(DgnModelCR model, DRange3dCR range, TransformCR transformFromDgn, size_t depth, size_t siblingIndex, TileNodeP parent, double tolerance) 
        { return new Tile(model, range, transformFromDgn, depth, siblingIndex, parent, tolerance); }

    bool    IsEmpty() const { return m_geometry.IsEmpty(); }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Ray.Bentley     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void AddPolyface(PolyfaceQueryCR polyface, SimplifyGraphic& graphic)
    {
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Ray.Bentley     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void AddRenderTile(Texture const& texture, Render::IGraphicBuilder::TileCorners const& corners, SimplifyGraphic& simplifyGraphic)
    {
    auto    tileTexture = texture.CreateTileTexture();
    auto    tileDisplayParams = TileDisplayParams::Create(0xffffff, tileTexture.get(), true); 
    auto    mesh = TileMesh::Create(*tileDisplayParams);

    mesh->AddRenderTile(corners, Transform::FromProduct(m_transformFromDgn, simplifyGraphic.GetLocalToWorldTransform()));

    m_geometry.Meshes().push_back(mesh);
    }
};  // Tile


/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     04/2017
+===============+===============+===============+===============+===============+======*/
struct GeometryProcessor : Dgn::IGeometryProcessor
{
    TileR                   m_tile;
    IFacetOptionsPtr        m_facetOptions;

    GeometryProcessor(TileR tile) : m_tile(tile) { m_facetOptions = TileGenerator::CreateTileFacetOptions(tile.GetTolerance()); }

    virtual UnhandledPreference _GetUnhandledPreference(CurveVectorCR, SimplifyGraphic&) const override      { return UnhandledPreference::Facet;}
    virtual UnhandledPreference _GetUnhandledPreference(ISolidPrimitiveCR, SimplifyGraphic&) const override  { BeAssert(false); return UnhandledPreference::Facet;}
    virtual UnhandledPreference _GetUnhandledPreference(MSBsplineSurfaceCR, SimplifyGraphic&) const override { BeAssert(false); return UnhandledPreference::Facet;}
    virtual UnhandledPreference _GetUnhandledPreference(PolyfaceQueryCR, SimplifyGraphic&) const override    { BeAssert(false); return UnhandledPreference::Facet;}
    virtual UnhandledPreference _GetUnhandledPreference(IBRepEntityCR, SimplifyGraphic&) const override      { BeAssert(false); return UnhandledPreference::Facet;}
    virtual UnhandledPreference _GetUnhandledPreference(TextStringCR, SimplifyGraphic&) const override       { BeAssert(false); return UnhandledPreference::Facet;}
    virtual IFacetOptionsP _GetFacetOptionsP() override { return m_facetOptions.get(); }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Ray.Bentley     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _ProcessPolyface(PolyfaceQueryCR polyface, bool filled, SimplifyGraphic& graphic) override
    {
    m_tile.AddPolyface(polyface, graphic);

    return true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Ray.Bentley     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _ProcessTile(Render::TextureCR texture, Render::IGraphicBuilder::TileCorners const& corners, SimplifyGraphic& simplifyGraphic) override
    {
    m_tile.AddRenderTile(dynamic_cast<TextureCR> (texture), corners, simplifyGraphic);
    return true;
    }

};  // GeometryProcessor




/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     04/2017
+===============+===============+===============+===============+===============+======*/
struct RenderSystem : Render::System
{
    mutable NullContext                         m_context;
    mutable TileTreePublish::GeometryProcessor  m_processor;

    RenderSystem(TileR outputTile) : m_processor(outputTile) { }
    ~RenderSystem() { }

    virtual MaterialPtr _GetMaterial(DgnMaterialId, DgnDbR) const override { return nullptr; }
    virtual MaterialPtr _CreateMaterial(Material::CreateParams const&) const override { return nullptr; } 
    virtual GraphicBuilderPtr _CreateGraphic(Graphic::CreateParams const& params) const override { return new SimplifyGraphic(params, m_processor, m_context); }
    virtual GraphicPtr _CreateSprite(ISprite& sprite, DPoint3dCR location, DPoint3dCR xVec, int transparency) const override { BeAssert(false); return nullptr; }
    virtual GraphicPtr _CreateBranch(GraphicBranch& branch, TransformCP, ClipVectorCP) const override { BeAssert(false); return nullptr; }
    virtual GraphicPtr _CreateViewlet(GraphicBranch& branch, PlanCR, ViewletPosition const&) const override { BeAssert(false); return nullptr; };
    virtual TexturePtr _GetTexture(DgnTextureId textureId, DgnDbR db) const override { BeAssert(false); return nullptr; }
    virtual TexturePtr _CreateTexture(ImageCR image, Render::Texture::CreateParams const& params) const override {return new TileTreePublish::Texture(image, params);}
    virtual TexturePtr _CreateTexture(ImageSourceCR source, Image::Format targetFormat, Image::BottomUp bottomUp, Texture::CreateParams const& params) const override { BeAssert(false); return nullptr; }
    virtual TexturePtr _CreateGeometryTexture(GraphicCR graphic, DRange2dCR range, bool useGeometryColors, bool forAreaPattern) const override { BeAssert(false); return nullptr; }
    virtual LightPtr   _CreateLight(Lighting::Parameters const&, DVec3dCP direction, DPoint3dCP location) const override { BeAssert(false); return nullptr; }

};

} // end TileTreePublish

using namespace TileTreePublish;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2017 
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::FutureStatus TileGenerator::GenerateTilesFromTileTree(IGetTileTreeForPublishingP tileTreePublisher, ITileCollector* collector, double leafTolerance, bool surfacesOnly, DgnModelP model)
    {
    ClipVectorPtr               clip;
    TileTree::RootCPtr          tileRoot = tileTreePublisher->_GetPublishingTileTree(clip, nullptr);

    if (!tileRoot.IsValid())
        return folly::makeFuture(TileGeneratorStatus::NoGeometry);

    return folly::via(&BeFolly::ThreadPool::GetIoPool(), [=]()
        {
        return collector->_BeginProcessModel(*model);
        })
    .then([=](TileGeneratorStatus status)
        {
        if (TileGeneratorStatus::Success != status)
            return folly::makeFuture(GenerateTileResult(TileGeneratorStatus::NoGeometry, nullptr));

        return GenerateTilesFromTileTree(tileRoot->GetRootTile().get(), tileRoot->GetLocation(), leafTolerance, clip.get(), model, collector);
        })
    .then([=](GenerateTileResult result)
        {
        m_progressMeter._IndicateProgress(++m_completedModels, m_totalModels);
        return collector->_EndProcessModel(*model, result.m_tile.get(), result.m_status);   
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2017 
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::FutureGenerateTileResult TileGenerator::GenerateTilesFromTileTree(TileTree::TileP inputTile, TransformCR location, double leafTolerance, ClipVectorCP clip, DgnModelP model, ITileCollector* collector)
    {
    double          rangeDiagonal = inputTile->GetRange().DiagonalDistance();
    double          tileTolerance = rangeDiagonal / inputTile->_GetMaximumSize();     // off by factor two??
    TilePtr         outputTile = Tile::Create(*model, inputTile->GetRange(), Transform::FromProduct(GetTransformFromDgn(*model), location), 0, 0, nullptr, tileTolerance);
    RenderSystem*   renderSystem = new RenderSystem(*outputTile);

    return folly::via(&BeFolly::ThreadPool::GetIoPool(), [=]
        {                                
        TileTree::TileLoadStatePtr          loadState;

        return inputTile->GetRootR()._RequestTile(*inputTile, loadState, renderSystem);     
        })
    .then([=](BentleyStatus status)
        {
        delete renderSystem;

        if (SUCCESS != status || outputTile->IsEmpty())
            return folly::makeFuture(GenerateTileResult(TileGeneratorStatus::NoGeometry, nullptr));

        collector->_AcceptTile(*outputTile);
        return folly::makeFuture(GenerateTileResult(TileGeneratorStatus::Success, outputTile->GetRoot()));
        });
    }
