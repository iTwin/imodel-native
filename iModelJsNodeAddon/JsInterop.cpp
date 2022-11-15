/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#if defined (_WIN32) && !defined(BENTLEY_WINRT)
#include <windows.h>
#endif
#include "IModelJsNative.h"
#include <Bentley/Base64Utilities.h>
#include <Bentley/Desktop/FileSystem.h>
#include <GeomSerialization/GeomSerializationApi.h>
#include <ECDb/ChangedIdsIterator.h>
#include <DgnPlatform/FunctionalDomain.h>
#if !defined (BENTLEYCONFIG_NO_VISUALIZATION)
    #include <Visualization/Visualization.h>
#endif
#include <DgnPlatform/EntityIdsChangeGroup.h>
#include <chrono>
#include <tuple>

#if defined (BENTLEYCONFIG_PARASOLID)
#include <PSBRepGeometry/PSBRepGeometry.h>
#endif

static Utf8String s_lastECDbIssue;
static BeFileName s_addonDllDir;
static BeFileName s_tempDir;

using namespace ElementDependency;
namespace Render = Dgn::Render;

namespace IModelJsNative {


/*=================================================================================**//**
* An implementation of IKnownLocationsAdmin that is useful for desktop applications.
* This implementation works for Windows, Linux, and MacOS.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct KnownLocationsAdmin : PlatformLib::Host::IKnownLocationsAdmin
{
    BeFileName m_tempDirectory;
    BeFileName m_assetsDirectory;

    BeFileNameCR _GetLocalTempDirectoryBaseName() override {return m_tempDirectory;}
    BeFileNameCR _GetDgnPlatformAssetsDirectory() override {return m_assetsDirectory;}

    //! Construct an instance of the KnownDesktopLocationsAdmin
    KnownLocationsAdmin()
        {
        m_tempDirectory = s_tempDir;
        m_assetsDirectory = s_addonDllDir;
        m_assetsDirectory.AppendToPath(L"Assets");
        }
};

//=======================================================================================
// @bsistruct
//=======================================================================================
struct JsTexture : Texture
{
private:
    // We keep a copy of the image data in memory so that it can be embedded into tiles which reference the texture.
    // This way the front-end can obtain the image data without making a request to the backend.
    ImageSource m_imageSource;
    Dimensions m_dimensions;
    TextureTransparency m_transparency;

    JsTexture(CreateParams const& params, ImageSource::Format format, ByteStream&& data, Dimensions dimensions, TextureTransparency transparency)
        : Texture(params), m_imageSource(format, std::move(data)), m_dimensions(dimensions), m_transparency(transparency) { }

    static RefCountedPtr<JsTexture> CreateForImage(CreateParams const& params, ImageCR image);

    static TextureTransparency DetermineTransparency(ImageCR);
public:
    static RefCountedPtr<JsTexture> Create(CreateParams const& params, ImageSourceCR image);
    static RefCountedPtr<JsTexture> Create(CreateParams const& params, ImageCR image) { return params.GetKey().IsValid() ? CreateForImage(params, image) : nullptr; }
    static RefCountedPtr<JsTexture> Create(GradientSymbCR);

    ImageSourceCP GetImageSource() const override { return &m_imageSource; }
    Dimensions GetDimensions() const override { return m_dimensions; }
    TextureTransparency GetTransparency() const override { return m_transparency; }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<JsTexture> JsTexture::Create(CreateParams const& params, ImageSourceCR src)
    {
    if (!params.GetKey().IsValid() || !src.IsValid())
        return nullptr;

    auto size = src.GetSize();
    if (size.x <= 0 || size.y <= 0)
        return nullptr;

    auto transparency = TextureTransparency::Opaque;
    if (src.SupportsTransparency())
        {
        Image image(src, Image::Format::Rgba);
        transparency = DetermineTransparency(image);
        }

    return new JsTexture(params, src.GetFormat(), ByteStream(src.GetByteStream()), Dimensions(size.x, size.y), transparency);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<JsTexture> JsTexture::Create(GradientSymbCR grad)
    {
    constexpr size_t size = 0x100;
    Image image = grad.GetImage(size, size);
    return CreateForImage(Texture::CreateParams(), image);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<JsTexture> JsTexture::CreateForImage(CreateParams const& params, ImageCR image)
    {
    if (!image.IsValid())
        return nullptr;

    auto format = Image::Format::Rgba == image.GetFormat() ? ImageSource::Format::Png : ImageSource::Format::Jpeg;
    ImageSource src(image, format);
    if (!src.IsValid())
        return nullptr;

    return new JsTexture(params, format, std::move(src.GetByteStreamR()), Dimensions(image.GetWidth(), image.GetHeight()), DetermineTransparency(image));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TextureTransparency JsTexture::DetermineTransparency(ImageCR image)
    {
    if (!image.IsValid() || Image::Format::Rgba != image.GetFormat())
        return TextureTransparency::Opaque;

    bool haveOpaque = false;
    bool haveTranslucent = false;
    uint8_t maxAlpha = 240; // See DisplayParams::GetMinTransparency()
    ByteStreamCR bytes = image.GetByteStream();
    uint8_t const* data = bytes.GetData();
    for (size_t i = 0; i < bytes.size(); i += 4)
        {
        uint8_t a = data[i + 3];
        if (a > maxAlpha)
            {
            haveOpaque = true;
            if (haveTranslucent)
                return TextureTransparency::Mixed;
            }
        else
            {
            haveTranslucent = true;
            if (haveOpaque)
                return TextureTransparency::Mixed;
            }
        }

    BeAssert(!haveOpaque || !haveTranslucent);
    return haveTranslucent ? TextureTransparency::Translucent : TextureTransparency::Opaque;
    }

//=======================================================================================
// @bsistruct
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
DEFINE_REF_COUNTED_PTR(JsMaterial);

//=======================================================================================
// This caches textures per-DgnDb so that we do not have to recreate them constantly or
// hold duplicates of the same texture in memory. We preserve the image data so that it
// can be embedded into tile bytes to be deserialized on front-end.
// @bsistruct
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
    JsMaterialPtr GetMaterial(Material::CreateParams const& params);

    static ResourceCache& Get(DgnDbR db) { return *db.ObtainAppData(GetKey(), []() { return new ResourceCache(); }); }

    ~ResourceCache() { m_textures.Clear(); m_gradients.Clear(); m_materials.Clear(); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ResourceCache::Add(TextureKey key, JsTexture* texture)
    {
    // We only cache persistent textures. We cache even if null, so we don't waste time repeatedly trying and failing to create the same texture.
    BeAssert(key.IsValid());
    if (key.IsPersistent())
        m_textures.Insert(key, texture);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
JsMaterialPtr ResourceCache::GetMaterial(Material::CreateParams const& params)
    {
    BeMutexHolder lock(m_mutex);
    RefCountedPtr<JsMaterial> mat = FindMaterial(params.m_key);
    if (mat.IsNull())
        {
        mat = JsMaterial::Create(params);
        if (params.m_key.IsPersistent())
            Add(params.m_key, mat.get());
        }

    return mat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
JsTexturePtr ResourceCache::CreateTexture(ImageCR img, Texture::CreateParams const& params)
    {
    BeAssert(params.GetKey().IsValid());
    BeAssert(FindTexture(params.GetKey()).IsNull());

    auto tex = JsTexture::Create(params, img);
    Add(params.GetKey(), tex.get());
    return tex;
    }

#define NO_IMPL(ret) { BeAssert(false); return ret; }
#define NULL_IMPL NO_IMPL(nullptr)
#define RETURN_GRAPHIC { return new Render::Graphic(db); }

//=======================================================================================
// @bsistruct
//=======================================================================================
struct JsRenderSystem : Render::System
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

//=======================================================================================
// @bsistruct
//=======================================================================================
struct JsDgnHost : PlatformLib::Host {
private:
    BeMutex m_mutex;
    JsRenderSystem m_renderSystem;
    IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new KnownLocationsAdmin(); }
#if !defined (BENTLEYCONFIG_NO_VISUALIZATION)
    VisualizationAdmin& _SupplyVisualizationAdmin() override {
      auto viz = VisualizationUPtr(iTwinVisualization_create(&m_renderSystem));
      return *new VisualizationAdmin(std::move(viz));
    }
#endif
#if defined (BENTLEYCONFIG_PARASOLID)
    BRepGeometryAdmin& _SupplyBRepGeometryAdmin() override {return *new BentleyApi::Psolid::PSolidKernelAdmin();}
#endif

public:
    JsDgnHost() { BeAssertFunctions::SetBeAssertHandler(&JsInterop::HandleAssertion);}
};


} // namespace IModelJsNative

using namespace IModelJsNative;

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void JsInterop::Initialize(BeFileNameCR addonDllDir, Napi::Env env, BeFileNameCR tempDir) {
    Env() = env;
    MainThreadId() = BeThreadUtilities::GetCurrentThreadId();
    s_addonDllDir = addonDllDir;
    s_tempDir = tempDir;

#if defined(BENTLEYCONFIG_OS_WINDOWS_DESKTOP) // excludes WinRT
    // Include this location for delay load of pskernel...
    WString newPath;
    newPath = L"PATH=" + addonDllDir + L";";
PUSH_DISABLE_DEPRECATION_WARNINGS
    newPath.append(::_wgetenv(L"PATH"));
POP_DISABLE_DEPRECATION_WARNINGS
    _wputenv(newPath.c_str());

    // Defeat node's attempt to turn off WER
    auto errMode = GetErrorMode();
    errMode &= ~SEM_NOGPFAULTERRORBOX;
    SetErrorMode(errMode);
#endif

    static std::once_flag s_initFlag;
    std::call_once(s_initFlag, []() {
        auto jsHost = new JsDgnHost();
        PlatformLib::Initialize(*jsHost);
        RegisterOptionalDomains();
        InitLogging();
        InitializeSolidKernel();

        BeFileName path = PlatformLib::GetHost().GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
        path.AppendToPath(L"RscFonts.itwin-workspace");
        FontManager::AddWorkspaceDb(path.GetNameUtf8().c_str(), nullptr);
    });
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void JsInterop::RegisterOptionalDomains()
    {
    DgnDomains::RegisterDomain(FunctionalDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::No);
    }



static RefCountedPtr<IRefCounted> s_solidKernelMainThreadMark;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::InitializeSolidKernel()
    {
    T_HOST.GetBRepGeometryAdmin()._StartSession();

    if (s_solidKernelMainThreadMark.IsNull())
        s_solidKernelMainThreadMark = T_HOST.GetBRepGeometryAdmin()._CreateMainThreadMark();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
NativeLogging::CategoryLogger JsInterop::GetNativeLogger() {
    return NativeLogging::CategoryLogger("imodeljs");
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void JsInterop::ConcurrentQueryExecute(ECDbCR ecdb, Napi::Object requestObj, Napi::Function callback) {
    auto& mgr = ConcurrentQueryMgr::GetInstance(ecdb);
    BeJsValue beJsReq(requestObj);
    auto request = QueryRequest::Deserialize(beJsReq);
    if (request->UsePrimaryConnection()) {
        mgr.Enqueue(std::move(request), [&](QueryResponse::Ptr value) {
            auto jsResp = Napi::Object::New(Env());
            auto beJsResp = BeJsValue(jsResp);
            if (value->GetKind() == QueryResponse::Kind::NoResult) {
                value->ToJs(beJsResp, false);
            }
            else if (value->GetKind() == QueryResponse::Kind::ECSql) {
                auto& resp = value->GetAsConst<ECSqlResponse>();
                resp.ToJs(beJsResp, false);
                if (!resp.asJsonString().empty()) {
                    auto parse = Env().Global().Get("JSON").As<Napi::Object>().Get("parse").As<Napi::Function>();
                    auto rows = Napi::String::New(Env(), resp.asJsonString());
                    jsResp[ECSqlResponse::JData] = parse({ rows });
                }
            }
            else if (value->GetKind() == QueryResponse::Kind::BlobIO) {
                auto& resp = value->GetAsConst<BlobIOResponse>();
                if (resp.GetLength() > 0) {
                    resp.ToJs(beJsResp, false);
                    auto blob = Napi::Uint8Array::New(Env(), resp.GetLength());
                    memcpy(blob.Data(), resp.GetData(), resp.GetLength());
                    jsResp[BlobIOResponse::JData] = blob;
                }
            }
            else {
                BeNapi::ThrowJsException(Env(), "concurrent query: unsupported response type");
            }
            callback.Call({ jsResp });
        });
        return;
    }
    auto threadSafeFunc = Napi::ThreadSafeFunction::New(requestObj.Env(), callback, "concurrent_query", 0, 1);
    mgr.Enqueue(std::move(request), [=](QueryResponse::Ptr value) {
        if(threadSafeFunc.BlockingCall (
            [=]( Napi::Env env, Napi::Function jsCallback) {
                auto jsResp = Napi::Object::New(env);
                auto beJsResp = BeJsValue(jsResp);
                if (value->GetKind() ==  QueryResponse::Kind::NoResult) {
                    value->ToJs(beJsResp, false);
                } else if (value->GetKind() ==  QueryResponse::Kind::ECSql) {
                    auto& resp = value->GetAsConst<ECSqlResponse>();
                    resp.ToJs(beJsResp, false);
                    if (!resp.asJsonString().empty()) {
                        auto parse = env.Global().Get("JSON").As<Napi::Object>().Get("parse").As<Napi::Function>();
                        auto rows = Napi::String::New(env, resp.asJsonString());
                        jsResp[ECSqlResponse::JData] = parse({rows});
                    }
                } else if (value->GetKind() ==  QueryResponse::Kind::BlobIO) {
                    auto& resp = value->GetAsConst<BlobIOResponse>();
                    if (resp.GetLength() > 0) {
                        resp.ToJs(beJsResp, false);
                        auto blob = Napi::Uint8Array::New(env, resp.GetLength());
                        memcpy(blob.Data(), resp.GetData(), resp.GetLength());
                        jsResp[BlobIOResponse::JData] = blob;
                    }
                } else {
                    BeNapi::ThrowJsException(env, "concurrent query: unsupported response type");
                }
                jsCallback.Call({jsResp});
        }) != napi_ok) {
            // do nothing
        }
        const_cast<Napi::ThreadSafeFunction&>(threadSafeFunc).Release();
    });
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnDbPtr JsInterop::CreateDgnDb(Utf8StringCR filenameIn, BeJsConst props) {
    auto rootSubject = props[json_rootSubject()];
    if (!rootSubject.isStringMember(json_name()))
        BeNapi::ThrowJsException(Env(), "Root subject name is missing");

    BeFileName filename(filenameIn);
    BeFileName path = filename.GetDirectoryName();
    if (!path.DoesPathExist()) {
        Utf8String err = Utf8String("Path [") + path.GetNameUtf8() + "] does not exist";
        BeNapi::ThrowJsException(Env(), err.c_str());
    }

    CreateDgnDbParams params(rootSubject[json_name()].asCString());
    if (rootSubject.isStringMember(json_description()))
        params.SetRootSubjectDescription(rootSubject[json_description()].asCString());
    if (props.isMember(json_globalOrigin()))
        params.m_globalOrigin = BeJsGeomUtils::ToDPoint3d(props[json_globalOrigin()]);
    if (props.isStringMember(json_guid()))
        params.m_guid.FromString(props[json_guid()].asCString());
    if (props.isMember(json_projectExtents()))
        params.m_projectExtents.FromJson(props[json_projectExtents()]);
    if (props.isStringMember(json_client()))
        params.m_client = props[json_client()].asCString();

    RefCountedPtr<BusyRetry> retryHandler;
    if (!params.IsReadonly())
        params.SetBusyRetry(new BeSQLite::BusyRetry(40, 500)); // retry 40 times, 1/2 second intervals (20 seconds total)
    DbResult result;
    DgnDbPtr db = DgnDb::CreateDgnDb(&result, filename, params);
    if (!db.IsValid())
        throwSqlResult("cannot create iModel", filenameIn.c_str(), result);

    return db;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
RevisionStatus JsInterop::DumpChangeSet(DgnDbR dgndb, BeJsConst changeSet)
    {
    DgnRevisionPtr revision = GetRevision(dgndb.GetDbGuid().ToString(), changeSet);
    revision->Dump(dgndb);
    return RevisionStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnDbStatus JsInterop::ExtractChangedInstanceIdsFromChangeSets(BeJsValue jsonOut, DgnDbR db, const bvector<BeFileName>& changeSetFiles)
    {
    Json::Value elementJson(Json::ValueType::objectValue);
    Json::Value elementInsertIds(Json::ValueType::arrayValue);
    Json::Value elementUpdateIds(Json::ValueType::arrayValue);
    Json::Value elementDeleteIds(Json::ValueType::arrayValue);

    Json::Value aspectJson(Json::ValueType::objectValue);
    Json::Value aspectInsertIds(Json::ValueType::arrayValue);
    Json::Value aspectUpdateIds(Json::ValueType::arrayValue);
    Json::Value aspectDeleteIds(Json::ValueType::arrayValue);

    Json::Value modelJson(Json::ValueType::objectValue);
    Json::Value modelInsertIds(Json::ValueType::arrayValue);
    Json::Value modelUpdateIds(Json::ValueType::arrayValue);
    Json::Value modelDeleteIds(Json::ValueType::arrayValue);

    Json::Value relationshipJson(Json::ValueType::objectValue);
    Json::Value relationshipInsertIds(Json::ValueType::arrayValue);
    Json::Value relationshipUpdateIds(Json::ValueType::arrayValue);
    Json::Value relationshipDeleteIds(Json::ValueType::arrayValue);

    Json::Value codeSpecJson(Json::ValueType::objectValue);
    Json::Value codeSpecInsertIds(Json::ValueType::arrayValue);
    Json::Value codeSpecUpdateIds(Json::ValueType::arrayValue);
    Json::Value codeSpecDeleteIds(Json::ValueType::arrayValue);

    Json::Value fontJson(Json::ValueType::objectValue);
    Json::Value fontInsertIds(Json::ValueType::arrayValue);
    Json::Value fontUpdateIds(Json::ValueType::arrayValue);
    Json::Value fontDeleteIds(Json::ValueType::arrayValue);

    EntityIdsChangeGroup entityIdsChangeGroup;
    entityIdsChangeGroup.ExtractChangedInstanceIdsFromChangeSets(db, changeSetFiles);
    for (auto& opsAndJsonIds : {
        std::tie(entityIdsChangeGroup.elementOps, elementInsertIds, elementUpdateIds, elementDeleteIds),
        std::tie(entityIdsChangeGroup.aspectOps, aspectInsertIds, aspectUpdateIds, aspectDeleteIds),
        std::tie(entityIdsChangeGroup.modelOps, modelInsertIds, modelUpdateIds, modelDeleteIds),
        std::tie(entityIdsChangeGroup.relationshipOps, relationshipInsertIds, relationshipUpdateIds, relationshipDeleteIds),
        std::tie(entityIdsChangeGroup.codeSpecOps, codeSpecInsertIds, codeSpecUpdateIds, codeSpecDeleteIds),
        std::tie(entityIdsChangeGroup.fontOps, fontInsertIds, fontUpdateIds, fontDeleteIds)
    })
        {
        // can replace this all in C++17 with a destructuring assignment in the loop decl
        const auto& opMap = std::get<0>(opsAndJsonIds);
        auto& insertIds = std::get<1>(opsAndJsonIds);
        auto& updateIds = std::get<2>(opsAndJsonIds);
        auto& deleteIds = std::get<3>(opsAndJsonIds);
        for (const auto& entry : opMap)
            {
            const auto& id = entry.first;
            const auto& op = entry.second;
            if (op == DbOpcode::Insert) insertIds.append(id.ToHexStr());
            if (op == DbOpcode::Update) updateIds.append(id.ToHexStr());
            if (op == DbOpcode::Delete) deleteIds.append(id.ToHexStr());
            }
        }

    if (elementInsertIds.size() > 0) elementJson["insert"] = elementInsertIds;
    if (elementUpdateIds.size() > 0) elementJson["update"] = elementUpdateIds;
    if (elementDeleteIds.size() > 0) elementJson["delete"] = elementDeleteIds;
    if (!elementJson.empty()) jsonOut["element"].From(elementJson);

    if (aspectInsertIds.size() > 0) aspectJson["insert"] = aspectInsertIds;
    if (aspectUpdateIds.size() > 0) aspectJson["update"] = aspectUpdateIds;
    if (aspectDeleteIds.size() > 0) aspectJson["delete"] = aspectDeleteIds;
    if (!aspectJson.empty()) jsonOut["aspect"].From(aspectJson);

    if (modelInsertIds.size() > 0) modelJson["insert"] = modelInsertIds;
    if (modelUpdateIds.size() > 0) modelJson["update"] = modelUpdateIds;
    if (modelDeleteIds.size() > 0) modelJson["delete"] = modelDeleteIds;
    if (!modelJson.empty()) jsonOut["model"].From(modelJson);

    if (relationshipInsertIds.size() > 0) relationshipJson["insert"] = relationshipInsertIds;
    if (relationshipUpdateIds.size() > 0) relationshipJson["update"] = relationshipUpdateIds;
    if (relationshipDeleteIds.size() > 0) relationshipJson["delete"] = relationshipDeleteIds;
    if (!relationshipJson.empty()) jsonOut["relationship"].From(relationshipJson);

    if (codeSpecInsertIds.size() > 0) codeSpecJson["insert"] = codeSpecInsertIds;
    if (codeSpecUpdateIds.size() > 0) codeSpecJson["update"] = codeSpecUpdateIds;
    if (codeSpecDeleteIds.size() > 0) codeSpecJson["delete"] = codeSpecDeleteIds;
    if (!codeSpecJson.empty()) jsonOut["codeSpec"].From(codeSpecJson);

    if (fontInsertIds.size() > 0) fontJson["insert"] = fontInsertIds;
    if (fontUpdateIds.size() > 0) fontJson["update"] = fontUpdateIds;
    if (fontDeleteIds.size() > 0) fontJson["delete"] = fontDeleteIds;
    if (!fontJson.empty()) jsonOut["font"].From(fontJson);

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnRevisionPtr JsInterop::GetRevision(Utf8StringCR dbGuid, BeJsConst arg) {
    if (!arg.isStringMember("id") || !arg.isNumericMember("index") || !arg.isStringMember("pathname") || !arg.isStringMember("parentId"))
        ThrowJsException("id, index, pathname, and parentId must all be string members of ChangesetProps");

    BeFileName changeSetPathname(arg["pathname"].asString().c_str(), true);
    if (!changeSetPathname.DoesPathExist())
        ThrowJsException("changeset file not found");

    auto revision = DgnRevision::Create(arg["id"].asString(), arg["index"].asInt(), arg["parentId"].asString(), dbGuid, &changeSetPathname);
    if (!revision.IsValid())
        ThrowJsException("invalid revision id");

    if (arg.isStringMember("pushDate"))
        revision->SetDateTime(DateTime::FromString(arg["pushDate"].asString().c_str()));

    return revision;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bvector<DgnRevisionPtr> JsInterop::GetRevisions(bool& containsSchemaChanges, Utf8StringCR dbGuid, BeJsConst changeSets) {
    containsSchemaChanges = false;
    if (!changeSets.isArray())
        ThrowJsException("changesets must be an array");

    bvector<DgnRevisionPtr> revisionPtrs;
    for (uint32_t i = 0; i < changeSets.size(); ++i) {
        BeJsConst changeSet = changeSets[i];
        revisionPtrs.push_back(GetRevision(dbGuid, changeSet));
        if (!containsSchemaChanges)
            containsSchemaChanges = changeSet["isSchemaChange"].GetBoolean();
    }

    return revisionPtrs;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
RevisionStatus JsInterop::ApplySchemaChangeSet(BeFileNameCR dbFileName, bvector<DgnRevisionCP> const& revisions, RevisionProcessOption applyOption)
    {
    SchemaUpgradeOptions schemaUpgradeOptions(revisions, applyOption);
    schemaUpgradeOptions.SetUpgradeFromDomains(SchemaUpgradeOptions::DomainUpgradeOptions::SkipCheck);

    DgnDb::OpenParams openParams(Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, schemaUpgradeOptions);
    DbResult result;
    DgnDbPtr dgndb = DgnDb::OpenDgnDb(&result, dbFileName, openParams);
    POSTCONDITION(result == BE_SQLITE_OK, RevisionStatus::ApplyError);
    result = dgndb->SaveChanges();
    POSTCONDITION(result == BE_SQLITE_OK, RevisionStatus::ApplyError);
    dgndb->CloseDb();
    return RevisionStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void JsInterop::GetRowAsJson(BeJsValue rowJson, ECSqlStatement& stmt)
    {
    JsonECSqlSelectAdapter adapter(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsHexadecimalString));
    adapter.GetRow(rowJson, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void JsInterop::GetECValuesCollectionAsJson(BeJsValue json, ECN::ECValuesCollectionCR props)
    {
    for (ECN::ECPropertyValue const& prop : props)
        {
        if (prop.HasChildValues())
            GetECValuesCollectionAsJson(json[prop.GetValueAccessor().GetAccessString(prop.GetValueAccessor().GetDepth()-1)], *prop.GetChildValues());
        else
          {
          ECN::PrimitiveECPropertyCP propertyPtr = prop.GetValueAccessor().GetECProperty()->GetAsPrimitiveProperty();
          ECN::IECInstanceCR instance = prop.GetInstance();
          if(propertyPtr != nullptr)
            {
            Utf8CP propName = propertyPtr->GetName().c_str();
            JsonEcInstanceWriter::WritePrimitiveValue(json, *propertyPtr, instance, propName);
            }
          }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult JsInterop::OpenECDb(ECDbR ecdb, BeFileNameCR pathname, BeSQLite::Db::OpenParams const& params)
    {
    if (!pathname.DoesPathExist())
        return BE_SQLITE_NOTFOUND;

    DbResult res = ecdb.OpenBeSQLiteDb(pathname, params);
    if (res != BE_SQLITE_OK)
        return res;

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult JsInterop::CreateECDb(ECDbR ecdb, BeFileNameCR pathname)
    {
    BeFileName path = pathname.GetDirectoryName();
    if (!path.DoesPathExist())
        return BE_SQLITE_NOTFOUND;

    DbResult res = ecdb.CreateNewDb(pathname);
    if (res != BE_SQLITE_OK)
        return res;

    return BE_SQLITE_OK;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult JsInterop::ImportSchema(ECDbR ecdb, BeFileNameCR pathname)
    {
    if (!pathname.DoesPathExist())
        return BE_SQLITE_NOTFOUND;

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext(false /*=acceptLegacyImperfectLatestCompatibleMatch*/, true /*=includeFilesWithNoVerExt*/);
    JsInterop::AddSchemaSearchPaths(schemaContext);
    schemaContext->SetFinalSchemaLocater(ecdb.GetSchemaLocater());

    ECSchemaPtr schema;
    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile(schema, pathname.GetName(), *schemaContext);
    if (SchemaReadStatus::Success != schemaStatus)
        return BE_SQLITE_ERROR;

    bvector<ECSchemaCP> schemas;
    schemas.push_back(schema.get());
    BentleyStatus status = ecdb.Schemas().ImportSchemas(schemas);
    if (status != SUCCESS)
        return BE_SQLITE_ERROR;

    return ecdb.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void JsInterop::AddSchemaSearchPaths(ECSchemaReadContextPtr schemaContext) 
    {
    BeFileName rootDir = PlatformLib::GetHost().GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    rootDir.AppendToPath(L"ECSchemas");
    BeFileName dgnPath = rootDir;
    dgnPath.AppendToPath(L"Dgn");
    dgnPath.AppendSeparator();
    BeFileName domainPath = rootDir;
    domainPath.AppendToPath(L"Domain");
    domainPath.AppendSeparator();
    BeFileName ecdbPath = rootDir;
    ecdbPath.AppendToPath(L"ECDb");
    ecdbPath.AppendSeparator();
    schemaContext->AddSchemaPath(dgnPath);
    schemaContext->AddSchemaPath(domainPath);
    schemaContext->AddSchemaPath(ecdbPath);
    return;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult JsInterop::ImportSchemasDgnDb(DgnDbR dgndb, bvector<Utf8String> const& schemaFileNames)
    {
    if (0 == schemaFileNames.size())
        return BE_SQLITE_NOTFOUND;

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext(false /*=acceptLegacyImperfectLatestCompatibleMatch*/, true /*=includeFilesWithNoVerExt*/);
    JsInterop::AddSchemaSearchPaths(schemaContext);
    schemaContext->SetFinalSchemaLocater(dgndb.GetSchemaLocater());
    bvector<ECSchemaCP> schemas;

    for (Utf8String schemaFileName : schemaFileNames)
        {
        BeFileName schemaFile(schemaFileName.c_str(), BentleyCharEncoding::Utf8);
        if (!schemaFile.DoesPathExist())
            return BE_SQLITE_NOTFOUND;


        SchemaReadStatus schemaStatus;
        ECSchemaPtr schema = ECSchema::LocateSchema(schemaFileName.c_str(), *schemaContext, SchemaMatchType::Exact, &schemaStatus);

        if (SchemaReadStatus::DuplicateSchema == schemaStatus)
            continue;

        if (SchemaReadStatus::Success != schemaStatus)
            return BE_SQLITE_ERROR;

        schemas.push_back(schema.get());
        }

    if (0 == schemas.size())
        return BE_SQLITE_NOTFOUND;

    SchemaStatus status = dgndb.ImportSchemas(schemas); // NOTE: this calls DgnDb::ImportSchemas which has additional processing over SchemaManager::ImportSchemas
    if (status != SchemaStatus::Success)
        return DgnDb::SchemaStatusToDbResult(status, true);

    return dgndb.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult JsInterop::ImportXmlSchemas(DgnDbR dgndb, bvector<Utf8String> const& serializedXmlSchemas)
    {
    if (0 == serializedXmlSchemas.size())
        return BE_SQLITE_NOTFOUND;

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext(false /*=acceptLegacyImperfectLatestCompatibleMatch*/, true /*=includeFilesWithNoVerExt*/);
    schemaContext->SetFinalSchemaLocater(dgndb.GetSchemaLocater());
    bvector<ECSchemaCP> schemas;

    for (Utf8String schemaXml : serializedXmlSchemas)
        {
        ECSchemaPtr schema;
        SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *schemaContext);
        if (SchemaReadStatus::DuplicateSchema == schemaStatus)
            continue;

        if (SchemaReadStatus::Success != schemaStatus)
            return BE_SQLITE_ERROR;

        schemas.push_back(schema.get());
        }

    if (0 == schemas.size())
        return BE_SQLITE_NOTFOUND;

    SchemaStatus status = dgndb.ImportSchemas(schemas); // NOTE: this calls DgnDb::ImportSchemas which has additional processing over SchemaManager::ImportSchemas
    if (status != SchemaStatus::Success)
        return DgnDb::SchemaStatusToDbResult(status, true);

    return dgndb.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult JsInterop::ImportFunctionalSchema(DgnDbR db)
    {
    return SchemaStatus::Success == FunctionalDomain::GetDomain().ImportSchema(db) ? BE_SQLITE_OK : BE_SQLITE_ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECClassCP JsInterop::GetClassFromInstance(ECDbCR ecdb, BeJsConst jsonInstance)
    {
    return ECJsonUtilities::GetClassFromClassNameJson(jsonInstance[ECJsonUtilities::json_className()], ecdb.GetClassLocater());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECInstanceId JsInterop::GetInstanceIdFromInstance(ECDbCR ecdb, BeJsConst jsonInstance)
    {
    if (!jsonInstance.isMember(ECJsonUtilities::json_id()))
        return ECInstanceId();

    ECInstanceId instanceId;
    if (SUCCESS != ECInstanceId::FromString(instanceId, jsonInstance[ECJsonUtilities::json_id()].asCString()))
        return ECInstanceId();

    return instanceId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
[[noreturn]] void JsInterop::ThrowJsException(Utf8CP msg) { throw Napi::Error::New(Env(), msg); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::SetMaxTileCacheSize(uint64_t maxBytes) {
  T_HOST.Visualization().SetMaxTileCacheSize(maxBytes);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
HexStrSqlFunction& HexStrSqlFunction::GetSingleton()
    {
    static HexStrSqlFunction* s_singleton = nullptr;
    if (s_singleton == nullptr)
        s_singleton = new HexStrSqlFunction();

    return *s_singleton;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void HexStrSqlFunction::_ComputeScalar(Context& ctx, int nArgs, DbValue* args)
    {
    DbValue const& numValue = args[0];
    if (numValue.IsNull())
        {
        ctx.SetResultNull();
        return;
        }

    if (numValue.GetValueType() != DbValueType::IntegerVal)
        {
        ctx.SetResultError("Argument of function HEXSTR is expected to be an integral number.");
        return;
        }

    static const size_t stringBufferLength = 19;
    Utf8Char stringBuffer[stringBufferLength];
    BeStringUtilities::FormatUInt64(stringBuffer, stringBufferLength, numValue.GetValueUInt64(), HexFormatOptions::IncludePrefix);
    ctx.SetResultText(stringBuffer, (int) strlen(stringBuffer), Context::CopyData::Yes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
StrSqlFunction& StrSqlFunction::GetSingleton()
    {
    static StrSqlFunction* s_singleton = nullptr;
    if (s_singleton == nullptr)
        s_singleton = new StrSqlFunction();

    return *s_singleton;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void StrSqlFunction::_ComputeScalar(Context& ctx, int nArgs, DbValue* args)
    {
    DbValue const& numValue = args[0];
    if (numValue.IsNull())
        {
        ctx.SetResultNull();
        return;
        }

    if (numValue.GetValueType() != DbValueType::IntegerVal)
        {
        ctx.SetResultError("Argument of function STR is expected to be an integral number.");
        return;
        }

    static const size_t stringBufferLength = std::numeric_limits<uint64_t>::digits + 1; //+1 for the trailing 0 character

    Utf8Char stringBuffer[stringBufferLength]; //+1 for the trailing 0 character;
    BeStringUtilities::FormatUInt64(stringBuffer, numValue.GetValueUInt64());
    ctx.SetResultText(stringBuffer, (int) strlen(stringBuffer), Context::CopyData::Yes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::SerializeValue(Napi::Env env, DbValue&value) {
    if (!value.IsValid()) {
        return env.Undefined();
    }

    if (value.IsNull()) {
        return env.Null();
    }

    switch(value.GetValueType()) {
        case DbValueType::IntegerVal:
        case DbValueType::FloatVal:
            return Napi::Number::New(env, value.GetValueDouble());
        case DbValueType::TextVal:
            return Napi::String::New(env, value.GetValueText());
        case DbValueType::BlobVal: {
            const auto length = value.GetValueBytes();
            auto dataArray = Napi::Uint8Array::New(env, length);
            memcpy(dataArray.Data(), value.GetValueBlob(), length);
            return dataArray;
        }
    }
    return env.Undefined();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::Open(Napi::Env env, Utf8CP changesetFile, bool invert) {
    if (m_reader != nullptr) {
        return Napi::Number::New(env, (int) BE_SQLITE_ERROR);
    }

    BeFileName input;
    input.AppendUtf8(changesetFile);
    // changesetId = Utf8String(input.GetFileNameWithoutExtension());
    if (!input.DoesPathExist()) {
        return Napi::Number::New(env, (int) BE_SQLITE_CANTOPEN);
    }
    m_fileName = input;
    m_invert = invert;
    m_reader = std::unique_ptr<RevisionChangesFileReaderBase>( new RevisionChangesFileReaderBase({input}, m_unusedDb));
    return Napi::Number::New(env, (int) BE_SQLITE_OK);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::Close(Napi::Env env) {
    m_currentChange = Changes::Change(nullptr, false);
    m_changes = nullptr;
    m_reader = nullptr;
    return Napi::Number::New(env, (int) BE_SQLITE_OK);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::Reset(Napi::Env env) {
    m_currentChange = Changes::Change(nullptr, false);
    m_changes = nullptr;
    return Napi::Number::New(env, (int) BE_SQLITE_OK);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::Step(Napi::Env env) {
    if (!IsOpen()) {
        return Napi::Number::New(env, (int) BE_SQLITE_ERROR);
    }

    if (m_changes == nullptr) {
        m_changes = std::make_unique<Changes>(*m_reader, m_invert);
        m_currentChange = m_changes->begin();
    } else {
        ++m_currentChange;
    }

    if (!m_currentChange.IsValid()) {
        return Napi::Number::New(env, (int) BE_SQLITE_DONE);
    }

    auto rc = m_currentChange.GetOperation(&m_tableName, &m_columnCount, &m_opcode, &m_indirect);
    if (rc != BE_SQLITE_OK) {
        return Napi::Number::New(env, (int) rc);
    }

    rc = m_currentChange.GetPrimaryKeyColumns(&m_primaryKeyColumns, &m_primaryKeyColumnCount);
    if (rc != BE_SQLITE_OK) {
        return Napi::Number::New(env, (int) rc);
    }

    return Napi::Number::New(env, (int) BE_SQLITE_ROW);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::GetFileName(Napi::Env env) {
    if (IsOpen())
        return Napi::String::New(env, m_fileName.GetNameUtf8());

    return env.Undefined();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::GetTableName(Napi::Env env) {
    if (HasRow()) {
        return Napi::String::New(env, m_tableName);
    }

    return env.Undefined();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::GetOpCode(Napi::Env env) {
    if (HasRow()) {
        return Napi::Number::New(env, (int)m_opcode);
    }

    return env.Undefined();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::IsIndirectChange(Napi::Env env) {
    if (HasRow()) {
        return Napi::Boolean::New(env, (int)m_indirect);
    }

    return env.Undefined();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::IsPrimaryKeyColumn(Napi::Env env, int col) {
    if (IsValidPrimaryKeyColumnIndex(col)) {
        return env.Undefined();
    }

    return Napi::Boolean::New(env, m_primaryKeyColumns[col] == 1);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::GetColumnCount(Napi::Env env) {
    if (HasRow()) {
        return Napi::Number::New(env, m_columnCount);
    }

    return env.Undefined();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::GetColumnValueType(Napi::Env env, int col, int target) {
    if (HasRow()) {
        auto val = target == 0 ? m_currentChange.GetOldValue(col) : m_currentChange.GetNewValue(col);
        if (!val.IsValid()) {
            return env.Undefined();
        }
        return Napi::Number::New(env, (int)val.GetValueType());
    }
    return env.Undefined();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::GetDdlChanges(Napi::Env env) {
    if (IsOpen()) {
        DdlChanges ddlChanges;
        bool hasSchemaChanges;
        m_reader->MakeReader()->GetSchemaChanges(hasSchemaChanges, ddlChanges);
        if (!ddlChanges._IsEmpty()) {
            auto ddl = ddlChanges.ToString();
            return Napi::String::New(env, ddl.c_str());
        };
    }
    return env.Undefined();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::GetColumnValue(Napi::Env env, int col, int target) {
    if (HasRow()) {
        auto val = target == 0 ? m_currentChange.GetOldValue(col) : m_currentChange.GetNewValue(col);
        if (!val.IsValid()) {
            return env.Undefined();
        }
        return SerializeValue(env, val);
    }
    return env.Undefined();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::GetRow(Napi::Env env) {
    if (!HasRow()) {
        return env.Undefined();
    }

    auto row = Napi::Array::New(env, m_columnCount);
    for (int i =0; i< m_columnCount; ++i)  {
        auto colVal = Napi::Object::New(env);
        if (m_opcode == DbOpcode::Insert) {
            auto newVal = m_currentChange.GetNewValue(i);
            if (newVal.IsValid()) {
                colVal.Set("new", SerializeValue(env, newVal));
            }
        } else if (m_opcode == DbOpcode::Delete) {
            auto oldVal = m_currentChange.GetOldValue(i);
            if (oldVal.IsValid()) {
                colVal.Set("old", SerializeValue(env, oldVal));
            }
        } else {
            auto newVal = m_currentChange.GetNewValue(i);
            if (newVal.IsValid()) {
                colVal.Set("new", SerializeValue(env, newVal));
            }
            auto oldVal = m_currentChange.GetOldValue(i);
            if (oldVal.IsValid()) {
                colVal.Set("old", SerializeValue(env, oldVal));
            }
        }
        row[i] = colVal;
    }
    return row;
}
