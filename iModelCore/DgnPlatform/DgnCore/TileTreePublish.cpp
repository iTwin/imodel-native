/*-------------------------------------------------------------------------------------+                                                                                           
|
|     $Source: DgnCore/TileTreePublish.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/TileIO.h>
#include <folly/BeFolly.h>

USING_NAMESPACE_BENTLEY_RENDER
                         

namespace TileTreePublish
{

DEFINE_POINTER_SUFFIX_TYPEDEFS(Tile);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Texture);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Context);
DEFINE_POINTER_SUFFIX_TYPEDEFS(RenderSystem);
DEFINE_REF_COUNTED_PTR(Tile)
DEFINE_REF_COUNTED_PTR(RenderSystem)

//=======================================================================================
// @bsiclass                                                     Ray.Bentley     04/2017
//=======================================================================================
struct Texture : Render::Texture
{
    CreateParams        m_createParams;
    ImageSource         m_imageSource;
    Image::BottomUp     m_bottomUp;

    Texture(ImageCR image, Texture::CreateParams const& createParams) : m_createParams(createParams), m_imageSource(image, Image::Format::Rgba == image.GetFormat() ? ImageSource::Format::Png : ImageSource::Format::Jpeg), m_bottomUp(Image::BottomUp::No)  { }
    Texture(ImageSourceCR source, Image::Format targetFormat, Image::BottomUp bottomUp, Texture::CreateParams const& createParams) : m_createParams(createParams), m_imageSource(source), m_bottomUp(bottomUp) { }

    Render::TileTextureImagePtr CreateTileTexture() const { return TileTextureImage::Create(ImageSource(m_imageSource), !m_createParams.m_isTileSection); }
};  // Texture
    
//=======================================================================================
// @bsiclass                                                     Ray.Bentley     04/2017
//=======================================================================================
struct Tile : Render::TileNode
{
private:
    PublishableTileGeometry                         m_geometry;
    bmap<Render::TextureP, TileDisplayParamsCPtr>   m_paramsMap;

protected:
    Tile(DgnModelCR model, DRange3dCR range, TransformCR transformFromDgn, size_t depth, size_t siblingIndex, TileNodeP parent, double tolerance)
        : Render::TileNode(model, range, transformFromDgn, depth, siblingIndex, parent, tolerance) { m_isEmpty = false; }

    virtual WString _GetFileExtension() const override { return L"b3dm"; }
    void _ClearGeometry() override { m_geometry = PublishableTileGeometry(); }
    virtual PublishableTileGeometry _GeneratePublishableGeometry(DgnDbR dgndb, TileGeometry::NormalMode normalMode, bool doSurfacesOnly, ITileGenerationFilterCP filter) const override {return std::move(m_geometry);}

public:
    bool    GeometryExists() const { return !m_geometry.IsEmpty(); }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Ray.Bentley     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static  TilePtr Create(DgnModelCR model, TileTree::TileCR inputTile, TransformCR transformFromDgn, size_t depth, size_t siblingIndex, TileNodeP parent)
    { 
    double          tileTolerance = (0.0 == inputTile._GetMaximumSize() || inputTile.GetRange().IsNull()) ? 1.0E6 : inputTile.GetRange().DiagonalDistance() / inputTile._GetMaximumSize();     // off by factor two??

    return new Tile(model, inputTile.GetRange(), transformFromDgn, depth, siblingIndex, parent, tileTolerance);
    }

                                  
/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Ray.Bentley     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void AddTriMesh(TriMeshArgsCR triMesh, SimplifyGraphic& simplifyGraphic)
    {
    TextureP                texture = dynamic_cast <TextureP> (triMesh.m_texture.get());
    TileDisplayParamsCPtr   displayParams;

    auto const& foundParams = m_paramsMap.find(texture);

    if (foundParams == m_paramsMap.end())
        {
        Render::TileTextureImagePtr     tileTexture = texture->CreateTileTexture();

        displayParams = TileDisplayParams::Create(0xffffff, tileTexture.get(), true);
        m_paramsMap.Insert(texture, displayParams);
        }
    else
        {
        displayParams = foundParams->second;
        }
    
    auto                            mesh = TileMesh::Create(*displayParams);

//  mesh->AddTriMesh(triMesh, Transform::FromProduct(m_transformFromDgn, simplifyGraphic.GetLocalToWorldTransform()), nullptr != texture && Image::BottomUp::Yes == texture->m_bottomUp);

    m_geometry.Meshes().push_back(mesh);
    }

};  // Tile


/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     04/2017
+===============+===============+===============+===============+===============+======*/
struct GeometryProcessor : Dgn::IGeometryProcessor
{
    TileP                   m_tile;
    IFacetOptionsPtr        m_facetOptions;

    GeometryProcessor(TileP tile) : m_tile(tile) { if (nullptr != tile) m_facetOptions = TileGenerator::CreateTileFacetOptions(tile->GetTolerance()); }

    // We don't expect the higher order primitives (or anything but Tiles and TriMeshes) to ever exist within a tile..
    virtual UnhandledPreference _GetUnhandledPreference(CurveVectorCR, SimplifyGraphic&) const override         { return UnhandledPreference::Facet;}
    virtual UnhandledPreference _GetUnhandledPreference(ISolidPrimitiveCR, SimplifyGraphic&) const override     { BeAssert(false); return UnhandledPreference::Facet;}
    virtual UnhandledPreference _GetUnhandledPreference(MSBsplineSurfaceCR, SimplifyGraphic&) const override    { BeAssert(false); return UnhandledPreference::Facet;}
    virtual UnhandledPreference _GetUnhandledPreference(PolyfaceQueryCR, SimplifyGraphic&) const override       { BeAssert(false); return UnhandledPreference::Facet;}
    virtual UnhandledPreference _GetUnhandledPreference(IBRepEntityCR, SimplifyGraphic&) const override         { BeAssert(false); return UnhandledPreference::Facet;}
    virtual UnhandledPreference _GetUnhandledPreference(TextStringCR, SimplifyGraphic&) const override          { BeAssert(false); return UnhandledPreference::Facet;}
    virtual UnhandledPreference _GetUnhandledPreference(TriMeshArgsCR, SimplifyGraphic&) const override         { return UnhandledPreference::Auto;}
    virtual IFacetOptionsP _GetFacetOptionsP() override { return m_facetOptions.get(); }
    virtual bool _DoClipping() const override {return true;}

    virtual bool _ProcessTriMesh(Render::TriMeshArgsCR args, SimplifyGraphic& simplifyGraphic) override         { m_tile->AddTriMesh (args, simplifyGraphic); return true;     }

};  // GeometryProcessor

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     04/2017
+===============+===============+===============+===============+===============+======*/
struct Context
{
    TilePtr                         m_outputTile;
    TileTree::TilePtr               m_inputTile;
    Transform                       m_transformFromDgn;
    TileGenerator::ITileCollector*  m_collector;
    double                          m_leafTolerance;
    DgnModelP                       m_model;
    ClipVectorPtr                   m_clip;

    Context(TileP outputTile, TileTree::TileP inputTile, TransformCR transformFromDgn, ClipVectorCP clip, TileGenerator::ITileCollector* collector, double leafTolerance, DgnModelP model) : 
            m_outputTile(outputTile), m_inputTile(inputTile), m_transformFromDgn(transformFromDgn), m_collector(collector), m_leafTolerance(leafTolerance), m_model(model) { if (nullptr != clip) m_clip = clip->Clone(nullptr); }

    Context(TileP outputTile, TileTree::TileP inputTile, Context const& inContext) :
            m_outputTile(outputTile), m_inputTile(inputTile), m_transformFromDgn(inContext.m_transformFromDgn), m_clip(inContext.m_clip), m_collector(inContext.m_collector), m_leafTolerance(inContext.m_leafTolerance), m_model(inContext.m_model) { }

};

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     04/2017
+===============+===============+===============+===============+===============+======*/
struct RenderSystem : Render::System
{
    mutable TileTreePublish::GeometryProcessor  m_processor;
    ContextR                                    m_context;

    struct Graphic : SimplifyGraphic
        {
        Graphic(ClipVectorPtr& clip, Render::GraphicBuilder::CreateParams const& params, IGeometryProcessorR processor, ViewContextR viewContext) : SimplifyGraphic(params, processor, viewContext) {  }
        }; 

    RenderSystem(ContextR context) :m_context(context), m_processor(context.m_outputTile.get()) {  }
    ~RenderSystem() { }

    virtual MaterialPtr _GetMaterial(RenderMaterialId, DgnDbR) const override { return nullptr; }
    virtual MaterialPtr _CreateMaterial(Material::CreateParams const&) const override { return nullptr; } 
    virtual GraphicPtr _CreateSprite(ISprite& sprite, DPoint3dCR location, DPoint3dCR xVec, int transparency, DgnDbR db) const override { BeAssert(false); return nullptr; }
    virtual GraphicPtr _CreateBranch(GraphicBranch&& branch, DgnDbR dgndb, TransformCR transform, ClipVectorCP clips) const override { BeAssert(false); return nullptr; }
    virtual GraphicPtr _CreateViewlet(GraphicBranch& branch, PlanCR, ViewletPosition const&) const override { BeAssert(false); return nullptr; };
    virtual TexturePtr _GetTexture(DgnTextureId textureId, DgnDbR db) const override { BeAssert(false); return nullptr; }
//  virtual TexturePtr _CreateTexture(ImageCR image, Render::Texture::CreateParams const& params) const override {return new Texture(image, params);}
//  virtual TexturePtr _CreateTexture(ImageSourceCR source, Image::Format targetFormat, Image::BottomUp bottomUp, Texture::CreateParams const& params) const override {return new Texture(source, targetFormat, bottomUp, params); }

    virtual TexturePtr _CreateGeometryTexture(GraphicCR graphic, DRange2dCR range, bool useGeometryColors, bool forAreaPattern) const override { BeAssert(false); return nullptr; }
    virtual LightPtr   _CreateLight(Lighting::Parameters const&, DVec3dCP direction, DPoint3dCP location) const override { BeAssert(false); return nullptr; }
    virtual int _Initialize(void* systemWindow, bool swRendering) override { return  0; }
    virtual Render::TargetPtr _CreateTarget(Render::Device& device, double tileSizeModifier) override { return nullptr; }
    virtual Render::TargetPtr _CreateOffscreenTarget(Render::Device& device, double tileSizeModifier) override { return nullptr; }
    virtual GraphicPtr _CreateVisibleEdges(MeshEdgeArgsCR args, DgnDbR dgndb)  const override { return nullptr; }
    virtual GraphicPtr _CreateSilhouetteEdges(SilhouetteEdgeArgsCR args, DgnDbR dgndb)  const override { return nullptr; }
    virtual GraphicPtr _CreateGraphicList(bvector<GraphicPtr>&& primitives, DgnDbR dgndb) const override { return nullptr; }
    virtual GraphicPtr _CreateBatch(GraphicR graphic, FeatureTable&& features) const override {return nullptr; }
    virtual uint32_t   _GetMaxFeaturesPerBatch() const override { return 0xffffffff; }

    virtual TexturePtr _GetTexture(GradientSymbCR gradient, DgnDbR db) const override {return nullptr; }
    virtual TexturePtr _CreateTexture(ImageCR image, Texture::CreateParams const& params=Texture::CreateParams())  const override {return nullptr; }
    virtual TexturePtr _CreateTexture(ImageSourceCR source, Image::BottomUp bottomUp, Texture::CreateParams const& params=Texture::CreateParams())  const override {return nullptr; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017 
+---------------+---------------+---------------+---------------+---------------+------*/
virtual GraphicPtr _CreateIndexedPolylines(IndexedPolylineArgsCR args, DgnDbR dgndb) const override 
    { 
    return nullptr; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017 
+---------------+---------------+---------------+---------------+---------------+------*/
virtual GraphicPtr _CreatePointCloud(PointCloudArgsCR args, DgnDbR dgndb)  const override 
    { 
    return nullptr; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017 
+---------------+---------------+---------------+---------------+---------------+------*/
virtual GraphicPtr _CreateTriMesh(TriMeshArgsCR args, DgnDbR dgndb) const override 
    {
    return  nullptr;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017 
+---------------+---------------+---------------+---------------+---------------+------*/
virtual GraphicBuilderPtr _CreateGraphic(Graphic::CreateParams const& params) const override 
    { 
    BeAssert(false);
    return nullptr;
    }
};


} // end TileTreePublish

using namespace TileTreePublish;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2017 
+---------------+---------------+---------------+---------------+---------------+------*/
static TileGenerator::FutureGenerateTileResult generateParentTile (Context context)
    {
    RenderSystem*   renderSystem = new RenderSystem(context);

    return folly::via(&BeFolly::ThreadPool::GetIoPool(), [=]
        {  
        return context.m_inputTile->IsNotLoaded() ? TileTree::TileIO::WriteCesiumTile(BeFileName(), *context.m_inputTile, *context.m_model) : TileTree::TileIO::WriteStatus::Success;
        })
    .then([=](TileTree::TileIO::WriteStatus status)
        {
        delete renderSystem;

        if (TileTree::TileIO::WriteStatus::Success != status || (!context.m_outputTile->GeometryExists() && !context.m_inputTile->_HasChildren()))
            return folly::makeFuture(TileGenerator::GenerateTileResult(TileGeneratorStatus::NoGeometry, nullptr));

        if (context.m_outputTile->GetTolerance() > context.m_leafTolerance && context.m_inputTile->_HasChildren())
            {
            TileTree::Tile::ChildTiles const* children = context.m_inputTile->_GetChildren(true);

            for (auto& child : *children)
                context.m_outputTile->GetChildren().push_back(Tile::Create(*context.m_model, *child, context.m_transformFromDgn, context.m_outputTile->GetDepth()+1, context.m_outputTile->GetChildren().size(), context.m_outputTile.get()));
            }

        if (context.m_outputTile->GeometryExists())
            context.m_collector->_AcceptTile(*context.m_outputTile);
        else
            context.m_outputTile->SetIsEmpty(true);
            
        context.m_outputTile->ClearGeometry();

        return folly::makeFuture(TileGenerator::GenerateTileResult(TileGeneratorStatus::Success, context.m_outputTile->GetRoot()));
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2017 
+---------------+---------------+---------------+---------------+---------------+------*/
static TileGenerator::FutureGenerateTileResult generateChildTiles (TileGeneratorStatus parentStatus, Context context)
    {
    auto root = static_cast<Render::TileNodeP>(context.m_outputTile->GetRoot());
    auto result = TileGenerator::GenerateTileResult(parentStatus, root);

    if (context.m_outputTile->GetChildren().empty() || TileGeneratorStatus::Success != parentStatus)
        return folly::makeFuture(result);

    std::vector<TileGenerator::FutureGenerateTileResult> childFutures;

    for (size_t i=0; i<context.m_outputTile->GetChildren().size(); i++)
        {
        TilePtr             outputChild  = dynamic_cast<TileP> (context.m_outputTile->GetChildren().at(i).get());
        TileTree::TilePtr   inputChild   = context.m_inputTile->_GetChildren(false)->at(i);
        Context             childContext(outputChild.get(), inputChild.get(), context);

        auto childFuture = generateParentTile(childContext).then([=](TileGenerator::GenerateTileResult result) { return generateChildTiles(result.m_status, childContext); });
        childFutures.push_back(std::move(childFuture));
        }

    return folly::unorderedReduce(childFutures, result, [=](TileGenerator::GenerateTileResult, TileGenerator::GenerateTileResult)
        {
        return TileGenerator::GenerateTileResult(TileGeneratorStatus::Success, root);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2017 
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::FutureStatus TileGenerator::GenerateTilesFromTileTree(ITileCollector* collector, double leafTolerance, bool surfacesOnly, GeometricModelP model)
    {
    Context                     context(nullptr, nullptr, Transform::FromIdentity(), nullptr, collector, leafTolerance, model);
    RenderSystem                renderSys(context);
    TileTree::RootCPtr          tileRoot = model->GetTileTree(&renderSys);
    ClipVectorPtr               clip;

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

        Transform           transformFromDgn = Transform::FromProduct(GetTransformFromDgn(*model), tileRoot->GetLocation());
        TilePtr             outputTile = Tile::Create(*model, *tileRoot->GetRootTile(), transformFromDgn, 0, 0, nullptr);
        TileTree::TilePtr   inputTile = tileRoot->GetRootTile();
        Context             context(outputTile.get(), inputTile.get(), transformFromDgn, clip.get(), collector, leafTolerance, model);

        return generateParentTile(context).then([=](GenerateTileResult result) { return generateChildTiles(result.m_status, context); });
        })
    .then([=](GenerateTileResult result)
        {
        m_progressMeter._IndicateProgress(++m_completedModels, m_totalModels);
        return collector->_EndProcessModel(*model, result.m_tile.get(), result.m_status);   
        });
    }                               







                                                        
