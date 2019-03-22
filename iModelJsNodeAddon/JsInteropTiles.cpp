/*--------------------------------------------------------------------------------------+
|
|     $Source: JsInteropTiles.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "IModelJsNative.h"
#include <DgnPlatform/Tile.h>
#include <GeomJsonWireFormat/JsonUtils.h> // ###TODO remove...
#include <folly/BeFolly.h>
#include <set>

using namespace IModelJsNative;
namespace Render = Dgn::Render;

#define NO_IMPL(ret) { BeAssert(false); return ret; }
#define NULL_IMPL NO_IMPL(nullptr)
#define RETURN_GRAPHIC { return new Render::Graphic(db); }

struct JsSystem;

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   06/18
//=======================================================================================
struct JsTexture : Texture
{
private:
    // We keep a copy of the image data in memory so that it can be embedded into tiles which reference the texture.
    // This way the front-end can obtain the image data without making a request to the backend.
    ImageSource m_imageSource;
    Dimensions m_dimensions;

    JsTexture(CreateParams const& params, ImageSource::Format format, ByteStream&& data, Dimensions dimensions)
        : Texture(params), m_imageSource(format, std::move(data)), m_dimensions(dimensions) { }

    static RefCountedPtr<JsTexture> CreateForImage(CreateParams const& params, ImageCR image);

    static bool HasAlpha(ImageCR);
    static void StripAlpha(ImageR);
public:
    static RefCountedPtr<JsTexture> Create(CreateParams const& params, ImageSourceCR image);
    static RefCountedPtr<JsTexture> Create(CreateParams const& params, ImageCR image) { return params.GetKey().IsValid() ? CreateForImage(params, image) : nullptr; }
    static RefCountedPtr<JsTexture> Create(GradientSymbCR);

    ImageSourceCP GetImageSource() const override { return &m_imageSource; }
    Dimensions GetDimensions() const override { return m_dimensions; }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<JsTexture> JsTexture::Create(CreateParams const& params, ImageSourceCR src)
    {
    if (!params.GetKey().IsValid() || !src.IsValid())
        return nullptr;

    auto imgFormat = ImageSource::Format::Png == src.GetFormat() ? Image::Format::Rgba : Image::Format::Rgb;
    Image img(src, imgFormat);
    Dimensions dims(img.GetWidth(), img.GetHeight());

    if (Image::Format::Rgba == imgFormat && !HasAlpha(img))
        {
        StripAlpha(img);
        ImageSource jpeg(img, ImageSource::Format::Jpeg);
        return new JsTexture(params, jpeg.GetFormat(), std::move(jpeg.GetByteStreamR()), dims);
        }

    return new JsTexture(params, src.GetFormat(), ByteStream(src.GetByteStream()), dims);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<JsTexture> JsTexture::Create(GradientSymbCR grad)
    {
    constexpr size_t size = 0x100;
    Image image = grad.GetImage(size, size);
    return CreateForImage(Texture::CreateParams(), image);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<JsTexture> JsTexture::CreateForImage(CreateParams const& params, ImageCR image)
    {
    if (!image.IsValid())
        return nullptr;

    Dimensions dims(image.GetWidth(), image.GetHeight());
    if (Image::Format::Rgba == image.GetFormat() && !HasAlpha(image))
        {
        ByteStream opaqueBytes(image.GetWidth() * image.GetHeight() * 3);
        uint8_t* pDst = opaqueBytes.GetDataP();
        uint8_t const* pSrc = image.GetByteStream().GetData();
        for (size_t src = 0, dst = 0; src < image.GetByteStream().size(); src += 4, dst += 3)
            {
            pDst[dst + 0] = pSrc[src + 0];
            pDst[dst + 1] = pSrc[src + 1];
            pDst[dst + 2] = pSrc[src + 2];
            }

        Image opaque(image.GetWidth(), image.GetHeight(), std::move(opaqueBytes), Image::Format::Rgb);
        ImageSource jpeg(opaque, ImageSource::Format::Jpeg);
        return jpeg.IsValid() ? new JsTexture(params, jpeg.GetFormat(), std::move(jpeg.GetByteStreamR()), dims) : nullptr;
        }

    auto format = Image::Format::Rgba == image.GetFormat() ? ImageSource::Format::Png : ImageSource::Format::Jpeg;
    ImageSource src(image, format);
    return src.IsValid() ? new JsTexture(params, format, std::move(src.GetByteStreamR()), dims) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsTexture::HasAlpha(ImageCR image)
    {
    if (Image::Format::Rgba != image.GetFormat())
        return false;

    uint8_t maxAlpha = 255 - Render::Primitives::DisplayParams::GetMinTransparency();
    ByteStreamCR bytes = image.GetByteStream();
    uint8_t const* data = bytes.GetData();
    for (size_t i = 0; i < bytes.size(); i += 4)
        {
        uint8_t a = data[i + 3];
        if (a <= maxAlpha)
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/18
+---------------+---------------+---------------+---------------+---------------+------*/
void JsTexture::StripAlpha(ImageR image)
    {
    BeAssert(Image::Format::Rgba == image.GetFormat());

    ByteStreamR bytes = image.GetByteStreamR();
    uint8_t* data = bytes.GetDataP();
    for (size_t src = 0, dst = 0; src < bytes.size(); src += 4, dst += 3)
        {
        data[dst + 0] = data[src + 0];
        data[dst + 1] = data[src + 1];
        data[dst + 2] = data[src + 2];
        }

    size_t newSize = image.GetWidth() * image.GetHeight() * 3;
    bytes.resize(newSize);
    image.SetFormat(Image::Format::Rgb);
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   06/18
//=======================================================================================
struct JsMaterial : Material
{
private:
    explicit JsMaterial(CreateParams const& params) : Material(params) { }
public:
    static RefCountedPtr<JsMaterial> Create(CreateParams const& params)
        {
        return params.m_key.IsValid() ? new JsMaterial(params) : nullptr;
        }
};

DEFINE_REF_COUNTED_PTR(JsTexture);

//=======================================================================================
// This caches textures per-DgnDb so that we do not have to recreate them constantly or
// hold duplicates of the same texture in memory. We preserve the image data so that it
// can be embedded into tile bytes to be deserialized on front-end.
// @bsistruct                                                   Paul.Connelly   06/18
//=======================================================================================
struct ResourceCache : DgnDb::AppData
{
private:
    template<typename K, typename V> struct ResourceMap
        {
    private:
        typedef RefCountedPtr<V> VPtr;
        bmap<K, VPtr> m_resources;
    public:
        V* Find(K const& key)
            {
            auto it = m_resources.find(key);
            return m_resources.end() == it ? nullptr : it->second.get();
            }

        void Insert(K const& key, V* value) { m_resources.Insert(key, value); }
        void Clear() { m_resources.clear(); }
        size_t size() const { return m_resources.size(); }
        };

    static Key const& GetKey() { static Key s_key; return s_key; }

    BeMutex m_mutex;
    ResourceMap<TextureKey, JsTexture> m_textures;
    ResourceMap<GradientSymb, JsTexture> m_gradients;
    ResourceMap<MaterialKey, JsMaterial> m_materials;

    void Add(TextureKey key, JsTexture* texture);
    void Add(GradientSymbCR grad, JsTexture* texture) { m_gradients.Insert(grad, texture); }
    void Add(MaterialKey key, JsMaterial* material) { BeAssert(key.IsValid()); m_materials.Insert(key, material); }

    JsTexturePtr CreateTexture(ImageCR, Texture::CreateParams const&);
    JsTexturePtr CreateTexture(ImageSourceCR, Texture::CreateParams const&);

    template<typename F> auto UnderMutex(F func) -> decltype(func())
        {
        BeMutexHolder lock(m_mutex);
        return func();
        }
public:
    JsTexturePtr FindTexture(TextureKey key) { return key.IsValid() ? UnderMutex([&]() { return m_textures.Find(key); }) : nullptr; }
    JsMaterial* FindMaterial(MaterialKey key) { return key.IsValid() ? UnderMutex([&]() { return m_materials.Find(key); }) : nullptr; }

    JsTexturePtr GetGradient(GradientSymbCR);
    JsTexturePtr GetTexture(ImageSourceCR, Texture::CreateParams const&, Image::BottomUp = Image::BottomUp::No);
    JsTexturePtr GetTexture(ImageCR, Texture::CreateParams const&);
    JsMaterial* GetMaterial(Material::CreateParams const& params);

    static ResourceCache& Get(DgnDbR db) { return *db.ObtainAppData(GetKey(), []() { return new ResourceCache(); }); }

    ~ResourceCache() { m_textures.Clear(); m_gradients.Clear(); m_materials.Clear(); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser   10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ResourceCache::Add(TextureKey key, JsTexture* texture)
    {
    BeAssert(key.IsValid());
    if (!texture->IsGlyph()) // with glyph atlases, we don't want to cache these
        m_textures.Insert(key, texture);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
JsMaterial* ResourceCache::GetMaterial(Material::CreateParams const& params)
    {
    BeMutexHolder lock(m_mutex);
    RefCountedPtr<JsMaterial> mat = FindMaterial(params.m_key);
    if (mat.IsNull())
        {
        mat = JsMaterial::Create(params);
        if (params.m_key.IsValid())
            Add(params.m_key, mat.get());
        }

    return mat.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
JsTexturePtr ResourceCache::GetGradient(GradientSymbCR grad)
    {
    BeMutexHolder lock(m_mutex);
    RefCountedPtr<JsTexture> tex = m_gradients.Find(grad);
    if (tex.IsNull())
        {
        tex = JsTexture::Create(grad);
        Add(grad, tex.get());
        }

    return tex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
JsTexturePtr ResourceCache::GetTexture(ImageSourceCR src, Texture::CreateParams const& params, Image::BottomUp)
    {
    if (!params.GetKey().IsValid())
        return nullptr;

    BeMutexHolder lock(m_mutex);
    auto tex = FindTexture(params.GetKey());
    if (tex.IsNull())
        tex = CreateTexture(src, params);

    return tex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
JsTexturePtr ResourceCache::GetTexture(ImageCR img, Texture::CreateParams const& params)
    {
    if (!params.GetKey().IsValid())
        return nullptr;

    BeMutexHolder lock(m_mutex);
    auto tex = FindTexture(params.GetKey());
    if (tex.IsNull())
        tex = CreateTexture(img, params);

    return tex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
JsTexturePtr ResourceCache::CreateTexture(ImageSourceCR src, Texture::CreateParams const& params)
    {
    BeAssert(params.GetKey().IsValid());
    BeAssert(FindTexture(params.GetKey()).IsNull());

    auto tex = JsTexture::Create(params, src);
    Add(params.GetKey(), tex.get());
    return tex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
JsTexturePtr ResourceCache::CreateTexture(ImageCR img, Texture::CreateParams const& params)
    {
    BeAssert(params.GetKey().IsValid());
    BeAssert(FindTexture(params.GetKey()).IsNull());

    auto tex = JsTexture::Create(params, img);
    Add(params.GetKey(), tex.get());
    return tex;
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   05/18
//=======================================================================================
struct JsSystem : Render::System
{
    int _Initialize(void*, bool) override NO_IMPL(0)
    Render::GraphicBuilderPtr _CreateGraphic(Render::GraphicBuilder::CreateParams const&) const override NULL_IMPL
    Render::TexturePtr _CreateGeometryTexture(Render::GraphicCR, DRange2dCR, bool, bool) const override NULL_IMPL
    Render::LightPtr _CreateLight(Dgn::Lighting::Parameters const&, DVec3dCP, DPoint3dCP) const override NULL_IMPL

    Render::MaterialPtr _FindMaterial(Render::MaterialKeyCR key, Dgn::DgnDbR db) const override { return ResourceCache::Get(db).FindMaterial(key); }
    Render::MaterialPtr _CreateMaterial(Render::Material::CreateParams const& params, Dgn::DgnDbR db) const override { return ResourceCache::Get(db).GetMaterial(params); }

    Render::TexturePtr _FindTexture(Render::TextureKeyCR key, Dgn::DgnDbR db) const override { return ResourceCache::Get(db).FindTexture(key); }
    Render::TexturePtr _CreateTexture(Render::ImageCR img, Dgn::DgnDbR db, Render::Texture::CreateParams const& params) const override
        {
        return ResourceCache::Get(db).GetTexture(img, params);
        }
    Render::TexturePtr _CreateTexture(Render::ImageSourceCR src, Render::Image::BottomUp, Dgn::DgnDbR db, Render::Texture::CreateParams const& params) const override
        {
        return ResourceCache::Get(db).GetTexture(src, params);
        }
    Render::TexturePtr _GetTexture(Render::GradientSymbCR grad, Dgn::DgnDbR db) const override
        {
        return ResourceCache::Get(db).GetGradient(grad);
        }

    uint32_t _GetMaxFeaturesPerBatch() const override { return 2048*1024; }
    Render::GraphicPtr _CreateTriMesh(Render::TriMeshArgsCR args, Dgn::DgnDbR db) const override RETURN_GRAPHIC
    Render::GraphicPtr _CreateIndexedPolylines(Render::IndexedPolylineArgsCR args, Dgn::DgnDbR db) const override RETURN_GRAPHIC
    Render::GraphicPtr _CreateGraphicList(bvector<Render::GraphicPtr>&& graphics, Dgn::DgnDbR db) const override RETURN_GRAPHIC
    Render::GraphicPtr _CreateBranch(Render::GraphicBranch&& branch, Dgn::DgnDbR db, TransformCR, ClipVectorCP) const override RETURN_GRAPHIC
    Render::GraphicPtr _CreateBatch(Render::GraphicR graphic, Render::FeatureTable&& features, DRange3dCR range) const override { return new Render::Graphic(graphic.GetDgnDb()); }
};

// The TileTree maintains a pointer to the Render::System - it must be kept alive.
// (It is entirely stateless so this is not a problem).
JsSystem s_system;

//=======================================================================================
// This namespace contains types used for tile requests processed on the BeFolly CPU
// thread pool. This prevents potentially long-running tile requests from blocking the
// libuv thread pool used by node to process simpler tasks like ECSql queries. It also
// bypasses the hard-coded (configurable via env var) size of that thread pool.
//
// The add-on exposes a polling-style API for these requests. The caller does not await
// the result; instead it checks back in periodically until the request completes.
// @bsistruct                                                   Paul.Connelly   02/19
//=======================================================================================
BEGIN_TILE_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(Request);
DEFINE_REF_COUNTED_PTR(Request);

//=======================================================================================
// A request for tile content handled by CPU thread pool.
// A Request is always constructed inside the node event loop and added to the set of
// active requests stored on the corresponding model's app data.
// The work of producing the tile content is done on the folly CPU thread pool.
// The result is picked up inside the node event loop by the polling API.
// @bsistruct                                                   Paul.Connelly   02/19
//=======================================================================================
struct Request : RefCountedBase
{
private:
    GeometricModelPtr       m_model;
    Tree::Id                m_treeId;
    ContentId               m_contentId;
    ICancellationTokenPtr   m_cancellationToken;
    BeAtomic<State>         m_state;
    Result                  m_result;

    Request(ICancellationTokenPtr cancel, GeometricModelR model, Tree::Id treeId, ContentId contentId)
        : m_model(&model), m_treeId(treeId), m_contentId(contentId), m_cancellationToken(cancel), m_state(State::New)
        {
        //
        }

    void SetResult(DgnDbStatus status, ContentCP content)
        {
        m_result.m_content = content;
        m_result.m_status = status;
        SetState(State::Completed);
        BeAssert((nullptr == content) == (DgnDbStatus::Success != status));
        }

    void SetState(State state) { m_state.store(state); }

    // Called on folly thread pool to compute result.
    void Process();
public:
    static RequestPtr Create(ICancellationTokenPtr cancel, GeometricModelR model, Tree::Id treeId, ContentId contentId)
        {
        return new Request(cancel, model, treeId, contentId);
        }

    // Enqueue onto folly thread pool.
    void Dispatch();

    ContentId GetContentId() const { return m_contentId; }
    State GetState() const { return m_state.load(); }
    bool IsCompleted() const { return State::Completed == GetState(); }
    bool IsCanceled() const { return nullptr != m_cancellationToken && m_cancellationToken->IsCanceled(); }

    void SetContent(ContentCR content) { SetResult(DgnDbStatus::Success, &content); }
    void SetStatus(DgnDbStatus status) { SetResult(status, nullptr); }

    ResultCR GetResult() const { return m_result; }
    PollResult Poll() const;

    struct PtrComparator
    {
        using is_transparent = std::true_type;

        bool operator()(RequestPtr const& lhs, RequestPtr const& rhs) const { return operator()(lhs->GetContentId(), rhs->GetContentId()); }
        bool operator()(RequestPtr const& lhs, ContentIdCR rhs) const { return operator()(lhs->GetContentId(), rhs); }
        bool operator()(ContentIdCR lhs, RequestPtr const& rhs) const { return operator()(lhs, rhs->GetContentId()); }
        bool operator()(ContentIdCR lhs, ContentIdCR rhs) const { return lhs < rhs; }
    };
};

//=======================================================================================
// Holds all open Requests for tile content for a single tile tree.
// @bsistruct                                                   Paul.Connelly   02/19
//=======================================================================================
using Requests = std::set<RequestPtr, Request::PtrComparator>;
DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(Requests);

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/18
//=======================================================================================
struct AppData : DgnModel::AppData
{
    struct Entry
    {
        Utf8String  m_id;
        TreePtr     m_tree;
        Requests    m_requests;

        Entry() { }
        explicit Entry(Utf8StringCR id) : m_id(id) { }

        struct Comparator
        {
            using is_transparent = std::true_type;

            bool operator()(Entry const& lhs, Entry const& rhs) const { return operator()(lhs.m_id, rhs.m_id); }
            bool operator()(Entry const& lhs, Utf8StringCR rhs) const { return operator()(lhs.m_id, rhs); }
            bool operator()(Utf8StringCR lhs, Entry const& rhs) const { return operator()(lhs, rhs.m_id); }
            bool operator()(Utf8StringCR lhs, Utf8StringCR rhs) const { return lhs < rhs; }
        };
    };

    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(Entry);

    // std::set used for 2 reasons: stable pointers to entries, and transparent comparators (compare entry to id string)
    // std::set is annoying because despite only a subset of the members being used for sorting, all members are treated as const
    // when accessed via iterator.
    std::set<Entry, Entry::Comparator>  m_entries;
    mutable BeMutex                     m_mutex;

    void _OnUnload(DgnModelR model) override
        {
        BeMutexHolder lock(m_mutex);
        for (auto& entry : m_entries)
            {
            if (entry.m_tree.IsValid())
                entry.m_tree->CancelAllTileLoads();
            }
        }

    void _OnUnloaded(DgnModelR model) override
        {
        BeMutexHolder lock(m_mutex);
        for (auto const& cEntry : m_entries)
            {
            auto& entry = const_cast<EntryR>(cEntry); // because std::set...
            if (entry.m_tree.IsValid())
                {
                // just in case somebody request more tiles after _OnUnload()...
                entry.m_tree->CancelAllTileLoads();
                // Wait for them all to finish canceling...
                entry.m_tree->WaitForAllLoads();
                // No one else should be holding a pointer to tile tree any more.
                BeAssert(1 == entry.m_tree->GetRefCount());
                entry.m_tree = nullptr;
                }

            entry.m_requests.clear();
            }

        m_entries.clear();
        }

    static TreePtr GetTileTree(GeometricModelR model, Tree::Id const& id, bool createIfNotFound)
        {
        auto& entry = GetEntry(model, id, createIfNotFound);
        return entry.m_tree;
        }

    static EntryR GetEntry(GeometricModelR model, Tree::Id const& id, bool allocateTree)
        {
        auto data = Get(model);
        Utf8String idString = id.GetPrefixString();

        BeMutexHolder lock(data->m_mutex);
        auto found = data->m_entries.find(idString);
        if (data->m_entries.end() == found)
            found = data->m_entries.insert(Entry(idString)).first;

        auto& entry = const_cast<EntryR>(*found); // because std::set...
        if (allocateTree && entry.m_tree.IsNull())
            entry.m_tree = Tree::Create(model, s_system, id);

        return entry;
        }

    static PollResult PollContent(ICancellationTokenPtr cancel, GeometricModelR model, Tree::Id const& treeId, ContentIdCR contentId);
    static PollResult PollContent(ICancellationTokenPtr cancel, DgnDbR db, Utf8StringCR treeIdStr, Utf8StringCR contentIdStr);

    static RefCountedPtr<AppData> Get(GeometricModelR model)
        {
        static Key s_key;
        auto appData = model.ObtainAppData(s_key, [&]() { return new AppData(); });
        BeAssert(appData.IsValid());
        return appData;
        }
};

END_TILE_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/18
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::TreePtr JsInterop::GetTileTree(GeometricModelR model, Tile::Tree::Id const& id, bool createIfNotFound)
    {
    return Tile::AppData::GetTileTree(model, id, createIfNotFound);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/19
+---------------+---------------+---------------+---------------+---------------+------*/
void Tile::Request::Process()
    {
    if (IsCanceled())
        {
        SetStatus(DgnDbStatus::NotOpen);
        return;
        }

    SetState(State::Loading);

    auto tree = JsInterop::GetTileTree(*m_model, m_treeId, true);
    auto content = tree.IsValid() ? tree->RequestContent(m_contentId, JsInterop::GetUseTileCache()) : nullptr;
    if (content.IsValid())
        SetContent(*content);
    else
        SetStatus(DgnDbStatus::NotFound);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/19
+---------------+---------------+---------------+---------------+---------------+------*/
void Tile::Request::Dispatch()
    {
    SetState(State::Pending);
    Tile::RequestPtr me(this);
    folly::via(&BeFolly::ThreadPool::GetCpuPool(), [me]() { me->Process(); });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/19
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::PollResult Tile::Request::Poll() const
    {
    auto state = GetState();
    return State::Completed == state ? PollResult(GetResult()) : PollResult(state);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/19
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::PollResult Tile::AppData::PollContent(ICancellationTokenPtr cancel, GeometricModelR model, Tree::Id const& treeId, ContentIdCR contentId)
    {
    auto& entry = GetEntry(model, treeId, false);
    auto found = entry.m_requests.find(contentId);
    if (entry.m_requests.end() == found)
        found = entry.m_requests.insert(Request::Create(cancel, model, treeId, contentId)).first;

    RequestPtr request = *found;
    auto result = request->Poll();
    switch (result.m_state)
        {
        case State::New:
            request->Dispatch();
            break;
        case State::Completed:
            entry.m_requests.erase(found);
            break;
        }

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/19
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::PollResult Tile::AppData::PollContent(ICancellationTokenPtr cancel, DgnDbR db, Utf8StringCR treeIdStr, Utf8StringCR contentIdStr)
    {
    if (!db.IsDbOpen())
        return PollResult(DgnDbStatus::NotOpen);

    ContentId contentId;
    auto treeId = Tree::Id::FromString(treeIdStr);
    if (!treeId.IsValid() || !contentId.FromString(contentIdStr.c_str()))
        return PollResult(DgnDbStatus::InvalidId);

    auto model = db.Models().Get<GeometricModel>(treeId.m_modelId);
    if (model.IsNull())
        return PollResult(DgnDbStatus::MissingId);

    return PollContent(cancel, *model, treeId, contentId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/19
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::PollResult JsInterop::PollTileContent(ICancellationTokenPtr cancel, DgnDbR db, Utf8StringCR treeId, Utf8StringCR tileId)
    {
    return Tile::AppData::PollContent(cancel, db, treeId, tileId);
    }

