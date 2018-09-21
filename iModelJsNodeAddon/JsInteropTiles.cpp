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

    static Dimensions ComputeImageSourceDimensions(ImageSourceCR);
public:
    static RefCountedPtr<JsTexture> Create(CreateParams const& params, ImageSourceCR image);
    static RefCountedPtr<JsTexture> Create(CreateParams const& params, ImageCR image) { return params.GetKey().IsValid() ? CreateForImage(params, image) : nullptr; }
    static RefCountedPtr<JsTexture> Create(GradientSymbCR);

    ImageSource GetImageSource() const override { return m_imageSource; } // ###TODO why does this return a copy??
    Dimensions GetDimensions() const override { return m_dimensions; }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<JsTexture> JsTexture::Create(CreateParams const& params, ImageSourceCR image)
    {
    if (params.GetKey().IsValid() && image.IsValid())
        return new JsTexture(params, image.GetFormat(), ByteStream(image.GetByteStream()), ComputeImageSourceDimensions(image));
    else
        return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
Texture::Dimensions JsTexture::ComputeImageSourceDimensions(ImageSourceCR src)
    {
    Image img(src, Image::Format::Rgb);
    return Dimensions(img.GetWidth(), img.GetHeight());
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

    auto format = Image::Format::Rgba == image.GetFormat() ? ImageSource::Format::Png : ImageSource::Format::Jpeg;
    ImageSource src(image, format);
    if (src.IsValid())
        return new JsTexture(params, format, std::move(src.GetByteStreamR()), Dimensions(image.GetWidth(), image.GetHeight()));
    else
        return nullptr;
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

    void Add(TextureKey key, JsTexture* texture) { BeAssert(key.IsValid()); m_textures.Insert(key, texture); }
    void Add(GradientSymbCR grad, JsTexture* texture) { m_gradients.Insert(grad, texture); }
    void Add(MaterialKey key, JsMaterial* material) { BeAssert(key.IsValid()); m_materials.Insert(key, material); }

    JsTexture* CreateTexture(ImageCR, Texture::CreateParams const&);
    JsTexture* CreateTexture(ImageSourceCR, Texture::CreateParams const&);

    template<typename F> auto UnderMutex(F func) -> decltype(func())
        {
        BeMutexHolder lock(m_mutex);
        return func();
        }
public:
    JsTexture* FindTexture(TextureKey key) { return key.IsValid() ? UnderMutex([&]() { return m_textures.Find(key); }) : nullptr; }
    JsMaterial* FindMaterial(MaterialKey key) { return key.IsValid() ? UnderMutex([&]() { return m_materials.Find(key); }) : nullptr; }

    JsTexture* GetGradient(GradientSymbCR);
    JsTexture* GetTexture(ImageSourceCR, Texture::CreateParams const&, Image::BottomUp = Image::BottomUp::No);
    JsTexture* GetTexture(ImageCR, Texture::CreateParams const&);
    JsMaterial* GetMaterial(Material::CreateParams const& params);

    static ResourceCache& Get(DgnDbR db) { return *db.ObtainAppdata(GetKey(), []() { return new ResourceCache(); }); }

    ~ResourceCache() { m_textures.Clear(); m_gradients.Clear(); m_materials.Clear(); }
};

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
JsTexture* ResourceCache::GetGradient(GradientSymbCR grad)
    {
    BeMutexHolder lock(m_mutex);
    RefCountedPtr<JsTexture> tex = m_gradients.Find(grad);
    if (tex.IsNull())
        {
        tex = JsTexture::Create(grad);
        Add(grad, tex.get());
        }

    return tex.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
JsTexture* ResourceCache::GetTexture(ImageSourceCR src, Texture::CreateParams const& params, Image::BottomUp)
    {
    if (!params.GetKey().IsValid())
        return nullptr;

    BeMutexHolder lock(m_mutex);
    auto tex = FindTexture(params.GetKey());
    if (nullptr == tex)
        tex = CreateTexture(src, params);

    return tex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
JsTexture* ResourceCache::GetTexture(ImageCR img, Texture::CreateParams const& params)
    {
    if (!params.GetKey().IsValid())
        return nullptr;

    BeMutexHolder lock(m_mutex);
    auto tex = FindTexture(params.GetKey());
    if (nullptr == tex)
        tex = CreateTexture(img, params);

    return tex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
JsTexture* ResourceCache::CreateTexture(ImageSourceCR src, Texture::CreateParams const& params)
    {
    BeAssert(params.GetKey().IsValid());
    BeAssert(nullptr == FindTexture(params.GetKey()));

    auto tex = JsTexture::Create(params, src);
    Add(params.GetKey(), tex.get());
    return tex.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
JsTexture* ResourceCache::CreateTexture(ImageCR img, Texture::CreateParams const& params)
    {
    BeAssert(params.GetKey().IsValid());
    BeAssert(nullptr == FindTexture(params.GetKey()));

    auto tex = JsTexture::Create(params, img);
    Add(params.GetKey(), tex.get());
    return tex.get();
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
// @bsistruct                                                   Paul.Connelly   08/18
//=======================================================================================
struct TileTreeAppData : DgnModel::AppData
{
    bmap<Utf8String, Tile::TreePtr>     m_trees;
    mutable BeMutex                     m_mutex;

    void _OnUnload(DgnModelR model) override
        {
        BeMutexHolder lock(m_mutex);
        for (auto& tree : m_trees)
            {
            if (tree.second.IsValid())
                {
                // JsInterop::GetLogger().debugv("Canceling all tile loads for %s", tree.second->GetId().ToString().c_str());
                tree.second->CancelAllTileLoads();
                }
            }
        }

    void _OnUnloaded(DgnModelR model) override
        {
        BeMutexHolder lock(m_mutex);
        for (auto& tree : m_trees)
            {
            if (tree.second.IsValid())
                {
                // JsInterop::GetLogger().debugv("Waiting for all tile loads for %s", tree.second->GetId().ToString().c_str());
                // just in case somebody request more tiles after _OnUnload()...
                tree.second->CancelAllTileLoads();
                // Wait for them all to finish canceling...
                tree.second->WaitForAllLoads();
                // No one else should be holding a pointer to tile tree any more.
                BeAssert(1 == tree.second->GetRefCount());
                tree.second = nullptr;
                }
            }
        }

    static Tile::TreeP FindTileTree(GeometricModelR model, Tile::Tree::Id const& id)
        {  
        static Key  s_key;
        auto appData = model.ObtainAppData(s_key, [&]() { return new TileTreeAppData(); });

        Utf8String  idString = id.GetPrefixString();
    
        BeMutexHolder lock(appData->m_mutex);

        auto found = appData->m_trees.find(idString);
        if (found != appData->m_trees.end())
            return found->second.get();

        auto    tree = Tile::Tree::Create(model, s_system, id);
        appData->m_trees.Insert(idString, tree); 
        return tree.get();
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/18
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::TreePtr JsInterop::FindTileTree(GeometricModelR model, Tile::Tree::Id const& id)
    {
    return TileTreeAppData::FindTileTree(model, id);
    }

