/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/Render.h>
#include <DgnPlatform/RenderPrimitives.h>
#include <DgnPlatform/TileIO.h>
#include <Bentley/BeThread.h>
#include <Bentley/BeAtomic.h>
#include <Bentley/CancellationToken.h>
#include <BeHttp/HttpRequest.h>
#include <folly/futures/Future.h>
#include <forward_list>

#define USING_NAMESPACE_DGN_CESIUM using namespace BentleyApi::Dgn::Cesium;

BEGIN_DGN_CESIUM_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(ICesiumPublisher);
DEFINE_POINTER_SUFFIX_TYPEDEFS(PublishedTile);
DEFINE_POINTER_SUFFIX_TYPEDEFS(MeshMaterial);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Tile);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Texture);
DEFINE_POINTER_SUFFIX_TYPEDEFS(LoadState);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Loader);

DEFINE_REF_COUNTED_PTR(PublishedTile);
DEFINE_REF_COUNTED_PTR(Tile);
DEFINE_REF_COUNTED_PTR(Texture);
DEFINE_REF_COUNTED_PTR(LoadState);
DEFINE_REF_COUNTED_PTR(Loader);

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

    Render::ImageSourceCP GetImageSource() const override { return &m_imageSource; }
    Dimensions GetDimensions() const override { return m_dimensions; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/18
//=======================================================================================
struct Output : RefCountedBase, NonCopyableClass
{
    virtual void _AddBatch(DRange3dCR range, Render::FeatureTable&& features) = 0;
    virtual void _AddTriMesh(Render::TriMeshArgsCR) = 0;
    virtual void _AddPointCloud(Render::PointCloudArgsCR) = 0;

    Render::TexturePtr CreateTexture(Render::ImageCR image) const
        {
        Render::ImageSource source(image, Render::Image::Format::Rgba == image.GetFormat() ? Render::ImageSource::Format::Png : Render::ImageSource::Format::Jpeg);
        return new Texture(std::move(source), image.GetWidth(), image.GetHeight());
        }

    Render::TexturePtr CreateTexture(Render::ImageSource&& source) const
        {
        bool hasAlpha = source.GetFormat() == Render::ImageSource::Format::Png;
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
    friend struct Root;
protected:
    RootR m_root;
    TileCP m_parent;
    ElementAlignedBox3d m_range;
    mutable BeAtomic<LoadStatus> m_loadStatus;

    LoadStatus GetLoadStatus() const { return m_loadStatus.load(); }
    void SetLoadStatus(LoadStatus status) { m_loadStatus.store(status); }
    void SetAbandoned();
public:
    Tile(RootR root, TileCP parent) : m_root(root), m_parent(parent), m_loadStatus(LoadStatus::NotLoaded) { }

    ElementAlignedBox3dCR GetRange() const { return m_range; }
    RootR GetRoot() { return m_root; }
    RootCR GetRoot() const { return m_root; }
    TileCP GetParent() const { return m_parent; }

    bool IsNotLoaded() const { return LoadStatus::NotLoaded == GetLoadStatus(); }
    bool IsQueued() const { return LoadStatus::Queued == GetLoadStatus(); }
    bool IsLoading() const { return LoadStatus::Loading == GetLoadStatus(); }
    bool IsReady() const { return LoadStatus::Ready == GetLoadStatus(); }
    bool IsNotFound() const { return LoadStatus::NotFound == GetLoadStatus(); }
    bool IsAbandoned() const { return LoadStatus::Abandoned == GetLoadStatus(); }

    void SetNotLoaded() { SetLoadStatus(LoadStatus::NotLoaded); }
    void SetIsQueued() { SetLoadStatus(LoadStatus::Queued); }
    void SetIsLoading() { SetLoadStatus(LoadStatus::Loading); }
    void SetIsReady() { SetLoadStatus(LoadStatus::Ready); }
    void SetNotFound() { SetLoadStatus(LoadStatus::NotFound); }

    DGNPLATFORM_EXPORT void ExtendRange(DRange3dCR childRange) const;

    virtual Utf8String _GetName() const = 0;
    virtual double _GetMaximumSize() const = 0;
    virtual ChildTilesCP _GetChildren(bool create) const = 0;
    virtual LoaderPtr _CreateLoader(LoadStateR, OutputR) = 0;
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/18
//=======================================================================================
struct LoadState : RefCountedBase, NonCopyableClass
{
private:
    // Because ICancellationToken has to be passed around as a shared_ptr...
    struct CancellationToken : ICancellationToken
    {
        BeAtomic<bool> m_canceled;
        
        bool IsCanceled() override { return m_canceled.load(); }
        void SetCanceled() { m_canceled.store(true); }
        void Register(std::weak_ptr<ICancellationListener> listener) override { }
    };

    TileCPtr m_tile;
    std::shared_ptr<CancellationToken> m_cancellationToken;

    explicit LoadState(TileCR tile) : m_tile(&tile), m_cancellationToken(std::make_shared<CancellationToken>()) { }
public:
    static LoadStatePtr Create(TileCR tile) { return new LoadState(tile); }

    bool IsCanceled() const { return m_cancellationToken->IsCanceled(); }
    void SetCanceled() { m_cancellationToken->SetCanceled(); }
    ICancellationTokenPtr GetCancellationToken() const { return m_cancellationToken; }

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
// @bsiclass                                                   Mathieu.Marchand  11/2016
//=======================================================================================
struct HttpDataQuery
{
    Http::HttpByteStreamBodyPtr m_responseBody;
    Http::Request m_request;
    LoadStatePtr m_loads;

    DGNPLATFORM_EXPORT HttpDataQuery(Utf8StringCR url, LoadStateP loads);
    HttpDataQuery(Utf8StringCR url, LoadStateR loads) : HttpDataQuery(url, &loads) { }

    Http::Request& GetRequest() {return m_request;}

    //! Valid only after 'Perform' has completed.
    ByteStream const& GetData() const {return m_responseBody->GetByteStream();}
    
    //! Perform http request and wait for the result.
    DGNPLATFORM_EXPORT folly::Future<Http::Response> Perform();
};

//=======================================================================================
// @bsiclass                                                   Mathieu.Marchand  11/2016
//=======================================================================================
struct FileDataQuery
{
    Utf8String m_fileName;
    LoadStatePtr m_loads;

    FileDataQuery(Utf8StringCR fileName, LoadStateP loads) : m_fileName(fileName), m_loads(loads) {}
    FileDataQuery(Utf8StringCR fileName, LoadStateR loads) : FileDataQuery(fileName, &loads) {}

    //! Read the entire file in a single chunk of memory.
    DGNPLATFORM_EXPORT folly::Future<ByteStream> Perform();
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
    mutable BeConditionVariable m_cv;
    mutable std::set<LoadStatePtr, LoadState::PtrComparator> m_activeLoads;
    bool m_isHttp;

    DGNPLATFORM_EXPORT Root(DgnDbR db, TransformCR location, Utf8CP rootResource);

    //! Clear the current tiles and wait for all pending download requests to complete/abort.
    //! All subclasses of Root must call this method in their destructor. This is necessary, since it must be called while the subclass vtable is 
    //! still valid and that cannot be accomplished in the destructor of Root.
    DGNPLATFORM_EXPORT void ClearAllTiles(); 
public:
    ~Root() { BeAssert(m_rootTile.IsNull()); } // Subclasses MUST call CLearAlLTiles in their destructor!

    DgnDbR GetDgnDb() const { return m_db; }
    TilePtr GetRootTile() const { return m_rootTile; }
    ElementAlignedBox3d GetRange() const { BeAssert(m_rootTile.IsValid()); return m_rootTile->GetRange(); }
    bool IsHttp() const { return m_isHttp; }

    TransformCR GetLocation() const { return m_location; }
    void SetLocation(TransformCR location) { m_location = location; }
    DGNPLATFORM_EXPORT void ExtendRange(DRange3dCR childRange) const;

    virtual Utf8String _ConstructTileResource(TileCR tile) const { return m_rootResource + tile._GetName(); }
    DGNPLATFORM_EXPORT virtual folly::Future<BentleyStatus> _RequestTile(TileR tile, LoadStateR loads, OutputR output);
    virtual ClipVectorCP _GetClipVector() const { return nullptr; }

    void StartTileLoad(LoadStateR) const;
    void DoneTileLoad(LoadStateR) const;
    void WaitForAllLoads() {BeMutexHolder holder(m_cv.GetMutex()); while (m_activeLoads.size()>0) m_cv.InfiniteWait(holder);}
    void CancelAllTileLoads();
    void CancelTileLoad(TileCR);
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/18
//=======================================================================================
struct Loader : RefCountedBase, NonCopyableClass
{
protected:
    Utf8String m_resourceName;
    TilePtr m_tile;
    OutputPtr m_output;
    LoadStatePtr m_loads;
    StreamBuffer m_tileBytes;
    Utf8String m_contentType;

    Loader(Utf8StringCR resourceName, TileR tile, OutputR output, LoadStateR loads) : m_resourceName(resourceName), m_tile(&tile), m_output(&output), m_loads(&loads) { }

    BentleyStatus LoadTile();
public:
    bool IsCanceledOrAbandoned() const { return m_loads->IsCanceled() || m_tile->IsAbandoned(); }
    OutputR GetOutput() const { return *m_output; }

    DGNPLATFORM_EXPORT virtual folly::Future<BentleyStatus> _GetFromSource();
    virtual BentleyStatus _LoadTile() = 0;

    struct LoadFlag
    {
        LoadStatePtr m_state;

        LoadFlag(LoadStateR state) : m_state(&state) { state.GetTile().GetRoot().StartTileLoad(state); }
        ~LoadFlag() { m_state->GetTile().GetRoot().DoneTileLoad(*m_state); }
    };

    folly::Future<BentleyStatus> Perform();
};

//=======================================================================================
//! Tiles for reality models typically consist of one or more large textured meshes.
// @bsistruct                                                   Paul.Connelly   07/17
//=======================================================================================
namespace TriMeshTree
{
//=======================================================================================
//! Creates a Render::GraphicPtr created from the mesh, and (for pickable tiles)
//! holds the data required to describe the mesh for picking.
// @bsistruct                                                   Paul.Connelly   07/17
//=======================================================================================
struct TriMesh : RefCountedBase, NonCopyableClass
{
    struct CreateParams
    {
        int32_t m_numIndices = 0;
        int32_t const* m_vertIndex = nullptr;
        int32_t m_numPoints = 0;
        FPoint3d const* m_points = nullptr;
        FPoint3d const* m_normals = nullptr;
        FPoint2d const* m_textureUV = nullptr;
        Render::TexturePtr m_texture;

        Render::QPoint3dList QuantizePoints() const;
        Render::OctEncodedNormalList QuantizeNormals() const;
        DGNPLATFORM_EXPORT PolyfaceHeaderPtr ToPolyface() const;
        DGNPLATFORM_EXPORT void FromTile(Render::TextureCR tile, Render::GraphicBuilder::TileCorners const& corners, FPoint3d* fpts);
    };
protected:
    Render::QPoint3dList m_points = Render::QPoint3dList(DRange3d::NullRange());
    Render::OctEncodedNormalList m_normals;
    bvector<FPoint2d> m_textureUV;
    bvector<int32_t> m_indices;

    DGNPLATFORM_EXPORT Render::TriMeshArgs CreateTriMeshArgs(Render::TextureP texture, FPoint2d const* textureUV) const;
public:
    DGNPLATFORM_EXPORT TriMesh(CreateParams const&, OutputR output);
    TriMesh() { }

    Dgn::Render::QPoint3dListCR GetPoints() const {return m_points;}
    bool IsEmpty() const {return m_points.empty();}
};

DEFINE_POINTER_SUFFIX_TYPEDEFS(TriMesh);
DEFINE_REF_COUNTED_PTR(TriMesh);

typedef std::forward_list<TriMeshPtr> TriMeshList; // a forward_list is smaller than a vector in the common case of a single element.

//=======================================================================================
//! The root of a TriMeshTree
// @bsistruct                                                   Paul.Connelly   07/17
//=======================================================================================
struct Root : Cesium::Root
{
    DEFINE_T_SUPER(Cesium::Root);
protected:
    Root(DgnDbR db, TransformCR location, Utf8CP sceneFile) : T_Super(db, location, sceneFile) { }

    void ClipTriMesh(TriMeshList& triMeshList, TriMesh::CreateParams const& geomParams, OutputR output);
public:
    DGNPLATFORM_EXPORT void CreateGeometry(TriMeshList& triMeshList, TriMesh::CreateParams const& geomParams, OutputR output);
};

//=======================================================================================
//! A TriMeshTree tile.
// @bsistruct                                                   Paul.Connelly   07/17
//=======================================================================================
struct Tile : Cesium::Tile
{
    DEFINE_T_SUPER(Cesium::Tile);

protected:
    double m_maxDiameter;
    double m_factor=1.0;
    ChildTiles m_children;
    TriMeshList m_meshes;

    Tile(Root& root, Tile const* parent, double maxDiameter=0.0) : T_Super(root, parent), m_maxDiameter(maxDiameter) { }

    ChildTiles const* _GetChildren(bool) const override {return IsReady() ? &m_children : nullptr;}
    double _GetMaximumSize() const override {return m_factor * m_maxDiameter;}
public:
    TriMeshList& GetGeometry() {return m_meshes;}
    void ClearGeometry() {m_meshes.clear();}
    Root& GetTriMeshRoot() {return static_cast<Root&>(GetRoot());}
};
} // namespace TriMeshTree

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
    BeFileName GetTilesetName() const;
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
    virtual Utf8String  _GetInputFileName() const = 0;

    //! Invoked before a model is processed.
    virtual Dgn::Tile::IO::WriteStatus _BeginProcessModel(GeometricModelCR model) { return Dgn::Tile::IO::WriteStatus::Success; }

    //! Invoked after a model is processed, with the result of processing.
    virtual Dgn::Tile::IO::WriteStatus _EndProcessModel(GeometricModelCR model, TransformCR location, PublishedTileCR rootTile, Dgn::Tile::IO::WriteStatus status) = 0;

    //! Write cesium tileset for a GeometricModel
    DGNPLATFORM_EXPORT static Dgn::Tile::IO::WriteStatus PublishCesiumTileset(ICesiumPublisher& publisher, GeometricModelCR model, TransformCR transformFromDgn, double leafTolerance);
    DGNPLATFORM_EXPORT static Dgn::Tile::IO::WriteStatus WriteCesiumTileset(Utf8StringCR inputFileName, BeFileName outputFileName, BeFileNameCR tileOutputDirectory, GeometricModelCR model, TransformCR transformFromDgn, double leafTolerance);
};

END_DGN_CESIUM_NAMESPACE

