/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/CesiumTileTree.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/Render.h>
#include <DgnPlatform/RenderPrimitives.h>
#include <DgnPlatform/TileIO.h>
#include <Bentley/BeAtomic.h>
#include <Bentley/CancellationToken.h>
#include <folly/futures/Future.h>

#define BEGIN_DGN_CESIUM_NAMESPACE BEGIN_BENTLEY_DGN_NAMESPACE namespace Cesium {
#define END_DGN_CESIUM_NAMESPACE } END_BENTLEY_DGN_NAMESPACE
#define USING_NAMESPACE_DGN_CESIUM using namespace BentleyApi::Dgn::Cesium;

BEGIN_DGN_CESIUM_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(ICesiumPublisher);
DEFINE_POINTER_SUFFIX_TYPEDEFS(PublishedTile);
DEFINE_POINTER_SUFFIX_TYPEDEFS(MeshMaterial);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Root);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Tile);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Output);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Texture);
DEFINE_POINTER_SUFFIX_TYPEDEFS(LoadState);

DEFINE_REF_COUNTED_PTR(PublishedTile);
DEFINE_REF_COUNTED_PTR(Root);
DEFINE_REF_COUNTED_PTR(Tile);
DEFINE_REF_COUNTED_PTR(Output);
DEFINE_REF_COUNTED_PTR(Texture);
DEFINE_REF_COUNTED_PTR(LoadState);

using ChildTiles = bvector<TilePtr>;
DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(ChildTiles);

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/18
//=======================================================================================
struct Texture : Render::Texture
{
    Render::ImageSource m_imageSource;
    Dimensions m_dimensions;

    Texture(Render::ImageSource&& source, uint32_t width, uint32_t height) : Render::Texture(CreateParams()),
        m_imageSource(std::move(source)), m_dimensions(width, height) { }

    Render::ImageSource GetImageSource() const override { return m_imageSource; } // ###TODO don't make a copy...
    Dimensions GetDimensions() const override { return m_dimensions; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/18
//=======================================================================================
struct Output : RefCountedBase, NonCopyableClass
{
    virtual void _AddBatch(DRange3dCR range) = 0;
    virtual void _AddTriMesh(Render::TriMeshArgsCR) = 0;
    virtual void _AddPointCloud(Render::PointCloudArgsCR) = 0;

    Render::TexturePtr CreateTexture(Render::ImageCR image) const
        {
        Render::ImageSource source(image, Render::Image::Format::Rgba == image.GetFormat() ? Render::ImageSource::Format::Png : Render::ImageSource::Format::Jpeg);
        return new Texture(std::move(source), image.GetWidth(), image.GetHeight());
        }

    Render::TexturePtr CreateTexture(Render::ImageSource&& source) const
        {
        bool hasAlpha = source.GetFormat() == ImageSource::Format::Png;
        Render::Image image(source, hasAlpha ? Render::Image::Format::Rgba : Render::Image::Format::Rgb);
        return new Texture(std::move(source), image.GetWidth(), image.GetHeight());
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/18
//=======================================================================================
struct Tile : RefCountedBase, NonCopyableClass
{
    enum class LoadStatus : int {NotLoaded=0, Queued=1, Loading=2, Ready=3, NotFound=4, Abandoned=5};
protected:
    RootR m_root;
    TileCP m_parent;
    ElementAlignedBox3d m_range;
    mutable BeAtomic<LoadStatus> m_loadStatus;
public:
    Tile(RootR root, TileCP parent) : m_root(root), m_parent(parent), m_loadStatus(LoadStatus::NotLoaded) { }

    ElementAlignedBox3dCR GetRange() const { return m_range; }
    RootR GetRoot() { return m_root; }
    RootCR GetRoot() const { return m_root; }

    LoadStatus GetLoadStatus() const { return m_loadStatus.load(); }
    bool IsNotLoaded() const { return LoadStatus::NotLoaded == GetLoadStatus(); }
    bool IsQueued() const { return LoadStatus::Queued == GetLoadStatus(); }
    bool IsLoading() const { return LoadStatus::Loading == GetLoadStatus(); }
    bool IsReady() const { return LoadStatus::Ready == GetLoadStatus(); }
    bool IsNotFound() const { return LoadStatus::NotFound == GetLoadStatus(); }
    bool IsAbandoned() const { return LoadStatus::Abandoned == GetLoadStatus(); }

    void SetLoadStatus(LoadStatus status) { m_loadStatus.store(status); }
    void SetIsNotLoaded() { SetLoadStatus(LoadStatus::NotLoaded); }
    void SetIsQueued() { SetLoadStatus(LoadStatus::Queued); }
    void SetIsLoading() { SetLoadStatus(LoadStatus::Loading); }
    void SetIsReady() { SetLoadStatus(LoadStatus::Ready); }
    void SetIsNotFound() { SetLoadStatus(LoadStatus::NotFound); }
    void SetIsAbandoned() { SetLoadStatus(LoadStatus::Abandoned); }

    virtual Utf8String _GetName() const = 0;
    virtual double _GetMaximumSize() const = 0;
    virtual ChildTilesCP _GetChildren(bool create) const = 0;
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/18
//=======================================================================================
struct LoadState : ICancellationToken, RefCountedBase, NonCopyableClass
{
private:
    TileCPtr m_tile;
    BeAtomic<bool> m_canceled;
public:
    explicit LoadState(TileCR tile) : m_tile(&tile) { }
    DGNPLATFORM_EXPORT ~LoadState();

    bool IsCanceled() override { return m_canceled.load(); }
    void SetCanceled() { m_canceled.store(true); }
    void Register(std::weak_ptr<ICancellationListener> listener) override { }

    TileCR GetTile() const { return *m_tile; }

    struct PtrComparator
    {
        using is_transparent = std::true_type;

        bool operator()(LoadStatePtr const& lhs, LoadStatePtr const& rhs) const { return operator()(lhs->m_tile.get(), rhs->m_tile.get()); }
        bool operator()(LoadStatePtr const& lhs, TileCP rhs) const { return operator()(lhs->m_tile.get(), rhs); }
        bool operator()(TileCP lhs, LoadStatePtr const& rhs) const { return operator()(lhs, rhs->m_tile.get()); }
        bool operator()(TileCP lhs, TileCP rhs) const { return lhs < rhs; }
    };
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/18
//=======================================================================================
struct Root : RefCountedBase, NonCopyableClass
{
protected:
    TilePtr m_rootTile;
    DgnDbR m_db;
    Transform m_location;
    Utf8String m_rootResource;
    OutputPtr m_output;
    bool m_isHttp;

    DGNPLATFORM_EXPORT Root(DgnDbR db, TransformCR location, Utf8CP rootResource, OutputR output);

    //! Clear the current tiles and wait for all pending download requests to complete/abort.
    //! All subclasses of Root must call this method in their destructor. This is necessary, since it must be called while the subclass vtable is 
    //! still valid and that cannot be accomplished in the destructor of Root.
    DGNPLATFORM_EXPORT void ClearAllTiles(); 
public:
    ~Root() { BeAssert(m_rootTile.IsNull()); } // Subclasses MUST call CLearAlLTiles in their destructor!

    DgnDbR GetDgnDb() const { return m_db; }
    TilePtr GetRootTile() const { return m_rootTile; }
    ElementAlignedBox3d GetRange() const { BeAssert(m_rootTile.IsValid()); return m_rootTile->GetRange(); }
    OutputR GetOutput() const { return *m_output; }
    bool IsHttp() const { return m_isHttp; }

    TransformCR GetLocation() const { return m_location; }
    void SetLocation(TransformCR location) { m_location = location; }

    virtual Utf8String _ConstructTileResource(TileCR tile) const { return m_rootResource + tile._GetName(); }
    DGNPLATFORM_EXPORT virtual folly::Future<BentleyStatus> _RequestTile(TileR tile, LoadStateR loads);
};

//=======================================================================================
// @bsistruct                                                       Ray.Bentley     08/2017
//! A node in the published tileset tree.  
//! This tree is used to construct the tileset metadata.
//=======================================================================================
struct PublishedTile : RefCountedBase
{
private:
    BeFileName                  m_outputDirectory;
    Utf8String                  m_name;
    Utf8String                  m_extension;
    bvector<PublishedTilePtr>   m_children;
    DRange3d                    m_publishedRange;
    DRange3d                    m_tileRange;
    double                      m_tolerance;

public:
    PublishedTile(TileCR inputTile, BeFileNameCR outputDirectory);

    bvector<PublishedTilePtr>& GetChildren() { return m_children; }
    bvector<PublishedTilePtr>const& GetChildren() const { return m_children; }
    void SetPublishedRange(DRange3dCR range) { m_publishedRange = range; }
    bool GetIsEmpty() const { return m_publishedRange.IsNull(); }
    DRange3dCR GetPublishedRange() const { return m_publishedRange; }
    DRange3dCR GetTileRange() const { return m_tileRange; }
    double GetTolerance() const { return m_tolerance; }
    void SetExtension(CharCP extension) { m_extension = extension; }
    Utf8String GetURL() const;
    BeFileName GetFileName() const;
};

//=======================================================================================
// @bsistruct                                                       Ray.Bentley     08/2017
//! Interface adopted by an object that publishes a cesium tileset to represent the
//! a GeometricModel.
//=======================================================================================
struct ICesiumPublisher
{
    //! Returns directory for tileset.
    virtual BeFileName  _GetOutputDirectory(GeometricModelCR model) const = 0;

    //! Invoked before a model is processed.
    virtual Dgn::Tile::IO::WriteStatus _BeginProcessModel(GeometricModelCR model) { return Dgn::Tile::IO::WriteStatus::Success; }

    //! Invoked after a model is processed, with the result of processing.
    virtual Dgn::Tile::IO::WriteStatus _EndProcessModel(GeometricModelCR model, TransformCR location, PublishedTileCR rootTile, Dgn::Tile::IO::WriteStatus status) = 0;

    //! Write cesium tileset for a GeometricModel
    DGNPLATFORM_EXPORT static Dgn::Tile::IO::WriteStatus PublishCesiumTileset(ICesiumPublisher& publisher, GeometricModelCR model, TransformCR transformFromDgn, double leafTolerance);
    DGNPLATFORM_EXPORT static Dgn::Tile::IO::WriteStatus WriteCesiumTileset(BeFileName outputFileName, BeFileNameCR tileOutputDirectory, GeometricModelCR model, TransformCR transformFromDgn, double leafTolerance);
};

END_DGN_CESIUM_NAMESPACE

