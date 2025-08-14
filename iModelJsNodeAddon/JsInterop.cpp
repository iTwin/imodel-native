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
    static std::unique_ptr<PlatformLib::Host> s_jsHost;
    std::call_once(s_initFlag, []() {
        s_jsHost = std::make_unique<JsDgnHost>();
        PlatformLib::Initialize(*s_jsHost);
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
Napi::Object JsInterop::ConcurrentQueryResetConfig(Napi::Env env) {
    auto outConf = Napi::Object::New(env);
    ConcurrentQueryMgr::Config::Reset(std::nullopt).To(outConf);
    return outConf;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Napi::Object JsInterop::ConcurrentQueryResetConfig(Napi::Env env, Napi::Object configObj) {
    if (configObj.IsObject()) {
        auto outConf = Napi::Object::New(env);
        BeJsValue inJsConf(configObj);
        auto inConf = ConcurrentQueryMgr::Config::From(inJsConf);
        ConcurrentQueryMgr::Config::Reset(inConf).To(outConf);
        return outConf;
    }
    return ConcurrentQueryResetConfig(env);
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void JsInterop::ConcurrentQueryExecute(ECDbCR ecdb, Napi::Object requestObj, Napi::Function callback) {
    ConcurrentQueryMgr::WithInstance(ecdb, [&](ConcurrentQueryMgr& mgr) -> void {
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
                    THROW_JS_IMODEL_NATIVE_EXCEPTION(Env(), "concurrent query: unsupported response type", IModelJsNativeErrorKey::BadArg);
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
                        THROW_JS_IMODEL_NATIVE_EXCEPTION(env, "concurrent query: unsupported response type", IModelJsNativeErrorKey::BadArg);
                    }
                    jsCallback.Call({jsResp});
            }) != napi_ok) {
                // do nothing
            }
            const_cast<Napi::ThreadSafeFunction&>(threadSafeFunc).Release();
        });
    });
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnDbPtr JsInterop::CreateIModel(Utf8StringCR filenameIn, BeJsConst props) {
    auto rootSubject = props[json_rootSubject()];
    if (!rootSubject.isStringMember(json_name()))
        THROW_JS_IMODEL_NATIVE_EXCEPTION(Env(), "Root subject name is missing", IModelJsNativeErrorKey::BadArg);

    BeFileName filename(filenameIn);
    BeFileName path = filename.GetDirectoryName();
    if (!path.DoesPathExist()) {
        Utf8String err = Utf8String("Path [") + path.GetNameUtf8() + "] does not exist";
        THROW_JS_IMODEL_NATIVE_EXCEPTION(Env(), err.c_str(), IModelJsNativeErrorKey::NotFound);
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
    DgnDbPtr db = DgnDb::CreateIModel(&result, filename, params);
    if (!db.IsValid())
        throwSqlResult("cannot create iModel", filenameIn.c_str(), result);

    return db;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ChangesetStatus JsInterop::DumpChangeSet(DgnDbR dgndb, BeJsConst changeSet)
    {
    ChangesetPropsPtr revision = GetChangesetProps(dgndb.GetDbGuid().ToString(), changeSet);
    revision->Dump(dgndb);
    return ChangesetStatus::Success;
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
ChangesetPropsPtr JsInterop::GetChangesetProps(Utf8StringCR dbGuid, BeJsConst arg) {
    if (!arg.isStringMember("id") || !arg.isNumericMember("index") || !arg.isStringMember("pathname") || !arg.isStringMember("parentId"))
        ThrowJsException("id, index, pathname, and parentId must all be members of ChangesetProps");

    BeFileName changeSetPathname(arg["pathname"].asString().c_str(), true);
    if (!changeSetPathname.DoesPathExist())
        ThrowJsException("changeset file not found");

    ChangesetPropsPtr changeset = new ChangesetProps(arg["id"].asString(), arg["index"].asInt(), arg["parentId"].asString(), dbGuid, changeSetPathname, (ChangesetProps::ChangesetType)arg["changesType"].asInt());

    if (arg.isStringMember("pushDate"))
        changeset->SetDateTime(DateTime::FromString(arg["pushDate"].asString().c_str()));

    if (arg.hasMember("uncompressedSize"))
        changeset->SetUncompressedSize(arg["uncompressedSize"].asInt64());

    return changeset;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bvector<ChangesetPropsPtr> JsInterop::GetChangesetPropsVec(bool& containsSchemaChanges, Utf8StringCR dbGuid, BeJsConst changeSets) {
    containsSchemaChanges = false;
    if (!changeSets.isArray())
        ThrowJsException("changesets must be an array");

    bvector<ChangesetPropsPtr> changesetVec;
    for (uint32_t i = 0; i < changeSets.size(); ++i) {
        BeJsConst changeSet = changeSets[i];
        changesetVec.push_back(GetChangesetProps(dbGuid, changeSet));
        if (!containsSchemaChanges)
            containsSchemaChanges = changeSet["isSchemaChange"].GetBoolean();
    }

    return changesetVec;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ChangesetStatus JsInterop::ApplySchemaChangeSet(BeFileNameCR dbFileName, bvector<ChangesetPropsCP> const& revisions, RevisionProcessOption applyOption)
    {
    SchemaUpgradeOptions schemaUpgradeOptions(revisions, applyOption);
    schemaUpgradeOptions.SetUpgradeFromDomains(SchemaUpgradeOptions::DomainUpgradeOptions::SkipCheck);

    DgnDb::OpenParams openParams(Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, schemaUpgradeOptions);
    DbResult result;
    DgnDbPtr dgndb = DgnDb::OpenIModelDb(&result, dbFileName, openParams);
    POSTCONDITION(result == BE_SQLITE_OK, ChangesetStatus::ApplyError);
    result = dgndb->SaveChanges();
    POSTCONDITION(result == BE_SQLITE_OK, ChangesetStatus::ApplyError);
    dgndb->CloseDb();
    return ChangesetStatus::Success;
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
            JsonEcInstanceWriter::WritePrimitiveValue(json, *propertyPtr, instance, nullptr);
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
    JsInterop::AddFallbackSchemaLocaters(ecdb.GetSchemaLocater(), schemaContext);

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
void JsInterop::AddFallbackSchemaLocaters(IECSchemaLocaterR ecdbLocater, ECSchemaReadContextPtr schemaContext)
    {
    // Add the db then the standard schema paths as fallback locations to load referenced schemas.
    schemaContext->AddFirstSchemaLocater(ecdbLocater);
    AddFallbackSchemaLocaters(schemaContext);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void JsInterop::AddFallbackSchemaLocaters(ECSchemaReadContextPtr schemaContext)
    {
    // Add the standard schema paths as fallback locations to load referenced schemas.
    BeFileName rootDir = PlatformLib::GetHost().GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    rootDir.AppendToPath(L"ECSchemas");
    BeFileName dgnPath = rootDir;
    dgnPath.AppendToPath(L"Dgn").AppendSeparator();

    BeFileName domainPath = rootDir;
    domainPath.AppendToPath(L"Domain").AppendSeparator();
    BeFileName ecdbPath = rootDir;
    ecdbPath.AppendToPath(L"ECDb").AppendSeparator();
    bvector<WString> paths {dgnPath, domainPath, ecdbPath};
    schemaContext->AddFinalSchemaPaths(paths);
    }

DropSchemaResult JsInterop::RemoveUnusedSchemaReferences(ECDb ecdb, bvector<Utf8String>& schemaNames, const SchemaImportOptions& opts)
    {
        if (!opts.m_schemaLockHeld) {
            NativeLogging::CategoryLogger("JsInterop").error("Schema lock must be held when dropping schemas");
            return DropSchemaResult::Error;
        }
        
        if (schemaNames.size() == 0) {
            auto stmt = ecdb.GetCachedStatement(R"sql(
            SELECT [ss].[Name]
            FROM   [ec_Schema] [ss]
            JOIN [ec_CustomAttribute] [ca] ON [ca].[ContainerId] = [ss].[Id]
            JOIN [ec_Class] [cc] ON [cc].[Id] = [ca].[ClassId]
            JOIN [ec_Schema] [sa] ON [sa].[Id] = [cc].[SchemaId]
            WHERE  [cc].[Name] = 'DynamicSchema'
                    AND [sa].[Name] = 'CoreCustomAttributes'
                    AND [ca].[ContainerType] = 1
            ORDER  BY [ss].Id DESC;)sql");

            while (stmt->Step() == BE_SQLITE_ROW)
                {
                schemaNames.push_back(Utf8String(stmt->GetValueText(0)));
                }
        }
        
        NativeLogging::CategoryLogger logger("JsInterop");
        for (Utf8String const& schemaName : schemaNames) {
            ECSchemaPtr schema;
            SchemaReadStatus schemaStatus = ECSchema::LocateSchema(schema, schemaName.c_str(), ecdb.GetSchemaContext(), SchemaMatchType::LatestCompatible, nullptr);
            if (schemaStatus != SchemaReadStatus::Success)
                {
                logger.errorv("Failed to locate schema %s. Status: %d", schemaName.c_str(), static_cast<int>(schemaStatus));
                return DropSchemaResult::Error;
                }
            DropSchemaResult res = ecdb.Schemas().DropSchema(schemaName.c_str());
            if (res != DropSchemaResult::Success) {
                logger.errorv("Failed to drop schema %s. Status: %d", schemaName.c_str(), static_cast<int>(res));
                return DropSchemaResult::Error;
            };
        }
        ecdb.SaveChanges();
        return DropSchemaResult::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult JsInterop::ImportSchemas(DgnDbR dgndb, bvector<Utf8String> const& schemaSources, SchemaSourceType sourceType, const SchemaImportOptions& opts)
    {
    if (0 == schemaSources.size())
        return BE_SQLITE_ERROR;

    NativeLogging::CategoryLogger logger("JsInterop");

    ECSchemaReadContextPtr schemaContext = opts.m_customSchemaContext;
    if (schemaContext.IsNull())
        schemaContext = ECSchemaReadContext::CreateContext(false /*=acceptLegacyImperfectLatestCompatibleMatch*/, true /*=includeFilesWithNoVerExt*/);

    SanitizingSchemaLocater finalLocater(dgndb.GetSchemaLocater());
    JsInterop::AddFallbackSchemaLocaters(finalLocater, schemaContext);
    bvector<ECSchemaCP> schemas;

    for (Utf8String schemaSource : schemaSources)
        {
        ECSchemaPtr schema;
        SchemaReadStatus schemaStatus;
        if (sourceType == SchemaSourceType::File)
            {
            BeFileName schemaFile(schemaSource.c_str(), BentleyCharEncoding::Utf8);
            if (!schemaFile.DoesPathExist())
                return BE_SQLITE_ERROR_FileNotFound;
            // This method, first attempts to pull the schema from the context, if it loads the schema, it adds its directory to search paths
            schema = ECSchema::LocateSchema(schemaSource.c_str(), *schemaContext, SchemaMatchType::Exact, &schemaStatus);
            }
        else
            schemaStatus = ECSchema::ReadFromXmlString(schema, schemaSource.c_str(), *schemaContext);

        if (SchemaReadStatus::DuplicateSchema == schemaStatus)
            continue;

        if (SchemaReadStatus::Success != schemaStatus)
            {
            Utf8String contextDesc = schemaContext->GetDescription();
            logger.errorv("Failed to read schema from %s. Context setup: %s", schemaSource.c_str(), contextDesc.c_str());
            return BE_SQLITE_ERROR;
            }

        schemas.push_back(schema.get());
        }

    if (0 == schemas.size())
        return BE_SQLITE_ERROR;

    SchemaStatus status = dgndb.ImportSchemas(schemas, opts.m_schemaLockHeld, DgnDb::SyncDbUri(opts.m_schemaSyncDbUri.c_str())); // NOTE: this calls DgnDb::ImportSchemas which has additional processing over SchemaManager::ImportSchemas
    if (status != SchemaStatus::Success)
        {
        Utf8String contextDesc = schemaContext->GetDescription();
        logger.errorv("ImportSchemas returned non-success code. Context setup: %s", contextDesc.c_str());
        return DgnDb::SchemaStatusToDbResult(status, true);
        }

    return dgndb.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Napi::Value JsInterop::InsertInstance(ECDbR db, NapiInfoCR info) {
    REQUIRE_ARGUMENT_ANY_OBJ(0, instanceObj);
    REQUIRE_ARGUMENT_ANY_OBJ(1, argsObj);
    // it hold write token
    auto& repo = db.GetInstanceRepository();
    auto inst = BeJsValue(instanceObj);
    auto args = BeJsValue(argsObj);

    auto fmt = JsFormat::Standard;
    if (args.isBoolMember("useJsNames") && args.asBool(false)){
        fmt = JsFormat::JsName;
    }

    ECInstanceKey newKey;
    auto rc = repo.Insert(inst, args, fmt, newKey);
    if (rc != BE_SQLITE_DONE) {
        if (repo.GetLastError().empty()) {
            THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to insert instance", rc);
        }
        THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), repo.GetLastError().c_str(), rc);
    }

    return Napi::Value::From(info.Env(), newKey.GetInstanceId().ToHexStr());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Napi::Value JsInterop::UpdateInstance(ECDbR db, NapiInfoCR info) {
    REQUIRE_ARGUMENT_ANY_OBJ(0, instanceObj);
    REQUIRE_ARGUMENT_ANY_OBJ(1, argsObj);

    auto& repo = db.GetInstanceRepository();
    auto inst = BeJsValue(instanceObj);
    auto args = BeJsValue(argsObj);

    auto fmt = JsFormat::Standard;
    if (args.isBoolMember("useJsNames") && args.asBool(false)){
        fmt = JsFormat::JsName;
    }

    auto rc = repo.Update(inst, args, fmt);
    if (rc != BE_SQLITE_DONE) {
        if (repo.GetLastError().empty()) {
            THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to insert instance", rc);
        }
        THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), repo.GetLastError().c_str(), rc);
    }
    return Napi::Value::From(info.Env(), db.GetModifiedRowCount() > 0);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Napi::Value JsInterop::DeleteInstance(ECDbR db, NapiInfoCR info) {
    REQUIRE_ARGUMENT_ANY_OBJ(0, keyObj);
    REQUIRE_ARGUMENT_ANY_OBJ(1, argsObj);

    auto& repo = db.GetInstanceRepository();
    auto key = BeJsValue(keyObj);
    auto args = BeJsValue(argsObj);

    auto fmt = JsFormat::Standard;
    if (args.isBoolMember("useJsNames") && args.asBool(false)){
        fmt = JsFormat::JsName;
    }

    auto rc = repo.Delete(key, args, fmt);
    if (rc != BE_SQLITE_DONE) {
        if (repo.GetLastError().empty()) {
            THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to insert instance", rc);
        }
        THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), repo.GetLastError().c_str(), rc);
    }
    return Napi::Value::From(info.Env(), db.GetModifiedRowCount() > 0);;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Napi::Value JsInterop::ReadInstance(ECDbR db, NapiInfoCR info) {
    REQUIRE_ARGUMENT_ANY_OBJ(0, keyObj);
    REQUIRE_ARGUMENT_ANY_OBJ(1, argsObj);

    auto& repo = db.GetInstanceRepository();
    auto key = BeJsValue(keyObj);
    auto args = BeJsValue(argsObj);

    auto fmt = JsFormat::Standard;
    if (args.isBoolMember("useJsNames") && args.asBool(false)){
        fmt = JsFormat::JsName;
    }

    auto outInstance = BeJsNapiObject(info.Env());
    auto rc = repo.Read(key, outInstance, args, fmt);
    if (rc != BE_SQLITE_ROW) {
        if (repo.GetLastError().empty()) {
            THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to read instance", rc);
        }
        THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), repo.GetLastError().c_str(), rc);
    }
    return outInstance;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Napi::Value JsInterop::PatchJsonProperties(NapiInfoCR info) {
    REQUIRE_ARGUMENT_STRING(0, jsonProps);

    // Remove Null values from jsonProps
    BeJsDocument doc;
    doc.Parse(jsonProps.c_str());
    if (doc.hasParseError())
        return Napi::Value::From(info.Env(), jsonProps);
    doc.PurgeNulls();

    // Handle relClassNames
    auto relClassNames = BeJsPath::Extract(BeJsValue(doc), "$");
    if (relClassNames.has_value()) {
        relClassNames.value().ForEachProperty([&](auto memberName, auto memberJson) {
            if (memberJson.isStringMember("relClassName")) {
                // Fix Class Names that were not converted to the TS format
                auto relClassName = memberJson["relClassName"];
                auto relClassNameJson = relClassNames->Get(memberName)["relClassName"];
                Utf8String correctedRelClassName = relClassName.Stringify();
                correctedRelClassName.DropQuotes();
                correctedRelClassName.ReplaceAll(".", ":");
                (BeJsValue&)relClassNameJson = correctedRelClassName;
            }
            return false;
        });
    }
    // Handle renderMaterial TextureIds
    auto map = BeJsPath::Extract(BeJsValue(doc), "$.materialAssets.renderMaterial.Map");
    if (map.has_value()) {
        map.value().ForEachProperty([&](auto memberName, auto memberJson) {
            if (memberJson.isNumericMember("TextureId")) {
                // Fix IDs that were previously stored as 64-bit integers rather than as ID strings.
                auto textureIdAsStringForLogging = memberJson["TextureId"].Stringify();
                auto textureId = memberJson["TextureId"].template GetId64<DgnTextureId>();
                auto textureIdJson = map->Get(memberName)["TextureId"];
                (BeJsValue&)textureIdJson = textureId.ToHexStr();
                if (!textureId.IsValid()) {
                    Utf8PrintfString msg("RenderMaterial had a textureId %s that was invalid.", textureIdAsStringForLogging.c_str());
                }
            }
            return false;
        });
    }
    // Handle DisplayStyle subcategory overrides
    auto subCategoryOvr = BeJsPath::Extract(BeJsValue(doc), "$.styles.subCategoryOvr");
    if (subCategoryOvr.has_value()) {
        subCategoryOvr.value().ForEachArrayMember([&](auto index, auto memberJson) {
            if (memberJson.isNumericMember("subCategory")) {
                // Fix IDs that were previously stored as 64-bit integers rather than as ID strings.
                auto subcategoryAsStringForLogging = memberJson["subCategory"].Stringify();
                auto subcategoryId = memberJson["subCategory"].template GetId64<DgnTextureId>();
                auto subcategoryJson = subCategoryOvr->Get(index)["subCategory"];
                (BeJsValue&)subcategoryJson = subcategoryId.ToHexStr();
                if (!subcategoryId.IsValid()) {
                    Utf8PrintfString msg("Style had a subCategory Override %s that was invalid.", subcategoryAsStringForLogging.c_str());
                }
            }
            return false;
        });
    }
    return Napi::Value::From(info.Env(), doc.Stringify());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void JsInterop::ClearECDbCache(ECDbR db, NapiInfoCR info) {
    db.ClearECDbCache();
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
BentleyStatus JsInterop::ConvertSchemas(bvector<Utf8String> const& inputStrings, bvector<Utf8String>& outputStrings, ECSchemaReadContextPtr schemaContext, bool convertCA)
    {
    if (0 == inputStrings.size())
        return BentleyStatus::ERROR;

    if (schemaContext.IsNull())
        schemaContext = ECSchemaReadContext::CreateContext(false /*=acceptLegacyImperfectLatestCompatibleMatch*/, true /*=includeFilesWithNoVerExt*/);

    bvector<bpair<SchemaKey, ECSchemaPtr>> schemaKeyPairs;
    StringSchemaLocater locater;
    for (Utf8String inputString : inputStrings)
        {
        SchemaKey key;
        SchemaReadStatus status = ECSchema::ReadSchemaKey(inputString, key);
        if (SchemaReadStatus::Success != status)
            return BentleyStatus::ERROR;
        locater.AddSchemaString(key, inputString);
        schemaKeyPairs.push_back(std::make_pair(key, nullptr));
        }
    schemaContext->AddSchemaLocater(locater);
    JsInterop::AddFallbackSchemaLocaters(schemaContext);

    if (0 == schemaKeyPairs.size())
        return BentleyStatus::ERROR;
    BeAssert(inputStrings.size() == schemaKeyPairs.size());

    for (int i = 0; i < schemaKeyPairs.size(); i++)
        {
        bpair<SchemaKey, ECSchemaPtr>& schemaKeyPair = schemaKeyPairs[i];
        ECSchemaPtr schema = ECSchema::LocateSchema(schemaKeyPair.first, *schemaContext);
        if (!schema.IsValid())
            return BentleyStatus::ERROR;

        schemaKeyPair.second = schema;
        }

    outputStrings.resize(schemaKeyPairs.size());
    if (convertCA)
        {
        // Make a copy of the schemaKeyPairs bvector
        bvector<ECSchemaCP> schemas;
        // Use std::transform to extract the ECSchemaPtr
        std::transform(schemaKeyPairs.begin(), schemaKeyPairs.end(), std::back_inserter(schemas),
            [](const bpair<SchemaKey, ECSchemaPtr>& schemaPair) { return schemaPair.second.get(); });

        ECSchema::SortSchemasInDependencyOrder(schemas);

        for (ECSchemaCP schema : schemas)
            {
            bool conversionStatus = ECSchemaConverter::Convert(*const_cast<ECSchemaP> (schema), *schemaContext);
            if (!conversionStatus)
                return BentleyStatus::ERROR;
            for (int i = 0; i < schemaKeyPairs.size(); i++)
                {
                if (schemaKeyPairs[i].first.Matches(schema->GetSchemaKey(), SchemaMatchType::Exact))
                    {
                    SchemaWriteStatus writeStatus = schema->WriteToXmlString(outputStrings[i]);
                    if (SchemaWriteStatus::Success != writeStatus)
                        {
                        outputStrings.clear();
                        return BentleyStatus::ERROR;
                        }
                    break;
                    }
                }
            }
        }
    else
        {
        outputStrings.resize(schemaKeyPairs.size());
        for (int i = 0; i < schemaKeyPairs.size(); i++)
            {
            ECSchemaPtr schema = schemaKeyPairs[i].second;
            SchemaWriteStatus writeStatus = schema->WriteToXmlString(outputStrings[i]);
            if (SchemaWriteStatus::Success != writeStatus)
                {
                outputStrings.clear();
                return BentleyStatus::ERROR;
                }
            }
        }
    return BentleyStatus::SUCCESS;
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
            return Napi::String::New(env, BeInt64Id(value.GetValueUInt64()).ToHexStr());
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
void NativeChangeset::OpenFile(Napi::Env env, Utf8StringCR changesetFile, bool invert) {
    BeFileName input;
    input.AppendUtf8(changesetFile.c_str());

    if (!input.DoesPathExist()) {
        THROW_JS_BE_SQLITE_EXCEPTION(env, "open(): changeset file specified does not exists", BE_SQLITE_CANTOPEN);
    }

    auto reader = std::make_unique<ChangesetFileReaderBase>(bvector<BeFileName>{input});
    DdlChanges ddlChanges;
    bool hasSchemaChanges;
    reader->MakeReader()->GetSchemaChanges(hasSchemaChanges, ddlChanges);
    if (!ddlChanges._IsEmpty())
        m_ddl = ddlChanges.ToString();

    OpenChangeStream(env, std::move(reader), invert);
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void NativeChangeset::OpenChangeStream(Napi::Env env, std::unique_ptr<ChangeStream> changeStream, bool invert) {
    if (m_changeStream != nullptr) {
        THROW_JS_BE_SQLITE_EXCEPTION(env, "openChangeStream(): reader is already in open state.", BE_SQLITE_ERROR);
    }

    if (changeStream == nullptr) {
        THROW_JS_BE_SQLITE_EXCEPTION(env, "openChangeStream(): could not open a empty changeStream", BE_SQLITE_ERROR);
    }

    m_invert = invert;
    m_changeStream = std::move(changeStream);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void NativeChangeset::OpenGroup(Napi::Env env, T_Utf8StringVector const& changesetFiles, Db const& db, bool invert) {
    m_changeGroup = std::make_unique<ChangeGroup>(db);
    DdlChanges ddlGroup;
    for(auto& changesetFile : changesetFiles) {
        BeFileName inputFile(changesetFile);
        if (!inputFile.DoesPathExist()) {
            THROW_JS_BE_SQLITE_EXCEPTION(env, SqlPrintfString("openGroup(): changeset file specified does not exists (%s)", inputFile.GetNameUtf8().c_str()), BE_SQLITE_CANTOPEN);
        }

        ChangesetFileReader reader(inputFile);
        bool containsSchemaChanges;
        DdlChanges ddlChanges;
        if (BE_SQLITE_OK != reader.MakeReader()->GetSchemaChanges(containsSchemaChanges, ddlChanges)){
            THROW_JS_BE_SQLITE_EXCEPTION(env, "openGroup(): unable to read schema changes", BE_SQLITE_ERROR);
        }
        for(auto& ddl : ddlChanges.GetDDLs()) {
            ddlGroup.AddDDL(ddl.c_str());
        }
        if (BE_SQLITE_OK != reader.AddToChangeGroup(*m_changeGroup)){
            THROW_JS_BE_SQLITE_EXCEPTION(env, "openGroup(): unable to add changeset to group", BE_SQLITE_ERROR);
        }
    }

    m_changeStream = std::make_unique<ChangeSet>();
    if (BE_SQLITE_OK != m_changeStream->FromChangeGroup(*m_changeGroup)){
        THROW_JS_BE_SQLITE_EXCEPTION(env, "openGroup(): unable to create change stream", BE_SQLITE_ERROR);
    }
    m_ddl = ddlGroup.ToString();
    m_invert = invert;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void NativeChangeset::WriteToFile(Napi::Env env, Utf8String const& fileName, bool containChanges, bool override) {
    const auto kStmtDelimiter = ";";
    BeFileName outputFile(fileName);
    DdlChanges ddlChanges;
    bvector<Utf8String> individualDDLs;
    BeStringUtilities::Split(m_ddl.c_str(), kStmtDelimiter, individualDDLs);

    for(auto const& ddl : individualDDLs) {
        ddlChanges.AddDDL(ddl.c_str());
    }

    if (outputFile.DoesPathExist() && !override) {
        THROW_JS_BE_SQLITE_EXCEPTION(env, "writeToFile(): changeset file already exists", BE_SQLITE_ERROR);
    }

    if(outputFile.DoesPathExist() && override) {
        if (outputFile.BeDeleteFile() != BeFileNameStatus::Success) {
            THROW_JS_BE_SQLITE_EXCEPTION(env, "writeToFile(): unable to delete existing changeset file", BE_SQLITE_ERROR);
        }
    }

    ChangesetFileWriter writer(outputFile, containChanges, ddlChanges, nullptr);
    if (BE_SQLITE_OK !=  writer.Initialize()){
        THROW_JS_BE_SQLITE_EXCEPTION(env, "writeToFile(): unable to initialize changeset writer", BE_SQLITE_ERROR);
    }

    if(m_changeGroup){
        writer.FromChangeGroup(*m_changeGroup);
    } else if (m_changeStream) {
        ChangeGroup changeGroup;
        m_changeStream->AddToChangeGroup(changeGroup);
        writer.FromChangeGroup(changeGroup);
    } else {
        THROW_JS_BE_SQLITE_EXCEPTION(env, "writeToFile(): no changeset to write", BE_SQLITE_ERROR);
    }
    if (!outputFile.DoesPathExist()) {
        THROW_JS_BE_SQLITE_EXCEPTION(env, "writeToFile(): unable to write changeset file", BE_SQLITE_ERROR);
    }
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void NativeChangeset::Close(Napi::Env env) {
    m_currentChange = Changes::Change(nullptr, false);
    m_changes = nullptr;
    m_changeStream = nullptr;
    m_changeGroup = nullptr;
    m_invert = false;
    m_ddl.clear();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void NativeChangeset::Reset(Napi::Env env) {
    m_currentChange = Changes::Change(nullptr, false);
    m_changes = nullptr;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::Step(Napi::Env env) {
    if (!IsOpen()) {
        THROW_JS_BE_SQLITE_EXCEPTION(env, "step(): no changeset opened.", BE_SQLITE_ERROR);
    }

    if (m_changes == nullptr) {
        m_changes = std::make_unique<Changes>(*m_changeStream, m_invert);
        m_currentChange = m_changes->begin();
    } else {
        ++m_currentChange;
    }

    if (!m_currentChange.IsValid()) {
        return Napi::Boolean::New(env, false);
    }

    auto rc = m_currentChange.GetOperation(&m_tableName, &m_columnCount, &m_opcode, &m_indirect);
    if (rc != BE_SQLITE_OK) {
        THROW_JS_BE_SQLITE_EXCEPTION(env, "step(): unable to read changeset", rc);
    }

    rc = m_currentChange.GetPrimaryKeyColumns(&m_primaryKeyColumns, &m_primaryKeyColumnCount);
    if (rc != BE_SQLITE_OK) {
        THROW_JS_BE_SQLITE_EXCEPTION(env, "step(): unable to read changeset", rc);
    }

    m_primaryKeyCount = 0;
    for (int i = 0; i < m_primaryKeyColumnCount; ++i) {
        if (m_primaryKeyColumns[i])
            ++m_primaryKeyCount;
    }

    return Napi::Boolean::New(env, true);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::GetTableName(Napi::Env env) {
    if (!HasRow()) {
        THROW_JS_BE_SQLITE_EXCEPTION(env, "getTableName(): there is no current row.", BE_SQLITE_ERROR);
    }

    return Napi::String::New(env, m_tableName);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::GetOpCode(Napi::Env env) {
    if (!HasRow()) {
        THROW_JS_BE_SQLITE_EXCEPTION(env, "getOpCode(): there is no current row.", BE_SQLITE_ERROR);
    }

    return Napi::Number::New(env, (int)m_opcode);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::IsIndirectChange(Napi::Env env) {
    if (!HasRow()) {
        THROW_JS_BE_SQLITE_EXCEPTION(env, "isIndirectChange(): there is no current row.", BE_SQLITE_ERROR);
    }

    return Napi::Boolean::New(env, (int)m_indirect);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::GetColumnCount(Napi::Env env) {
    if (!HasRow()) {
        THROW_JS_BE_SQLITE_EXCEPTION(env, "getColumnCount(): there is no current row.", BE_SQLITE_ERROR);
    }

    return Napi::Number::New(env, m_columnCount);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::GetHasRow(Napi::Env env) {
    return Napi::Boolean::New(env, HasRow());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::GetColumnValueInteger(Napi::Env env, int col, int target){
    if (!HasRow() || !(col >= 0 && col < m_columnCount) || (target != 0 && target != 1)) {
        return env.Undefined();
    }

    // old value can be called by updated and deleted row.
    if (target == 0 && m_opcode == DbOpcode::Insert)
        return env.Undefined();

    // new value can be called by updated and inserted row.
    if (target != 0 && m_opcode == DbOpcode::Delete)
        return env.Undefined();

    auto val = target == 0 ? m_currentChange.GetOldValue(col) : m_currentChange.GetNewValue(col);
    if (!val.IsValid()) {
        return env.Undefined();
    }

    if (val.IsNull()) {
        return env.Null();
    }

    return Napi::Number::New(env, static_cast<double>(val.GetValueInt64()));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::GetColumnValueId(Napi::Env env, int col, int target){
    if (!HasRow() || !(col >= 0 && col < m_columnCount) || (target != 0 && target != 1)) {
        return env.Undefined();
    }

    // old value can be called by updated and deleted row.
    if (target == 0 && m_opcode == DbOpcode::Insert)
        return env.Undefined();

    // new value can be called by updated and inserted row.
    if (target != 0 && m_opcode == DbOpcode::Delete)
        return env.Undefined();

    auto val = target == 0 ? m_currentChange.GetOldValue(col) : m_currentChange.GetNewValue(col);
    if (!val.IsValid()) {
        return env.Undefined();
    }

    if (val.IsNull()) {
        return env.Null();
    }

    return Napi::String::New(env, BeInt64Id(val.GetValueUInt64()).ToHexStr());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::GetColumnValueDouble(Napi::Env env, int col, int target){
    if (!HasRow() || !(col >= 0 && col < m_columnCount) || (target != 0 && target != 1)) {
        return env.Undefined();
    }

    // old value can be called by updated and deleted row.
    if (target == 0 && m_opcode == DbOpcode::Insert)
        return env.Undefined();

    // new value can be called by updated and inserted row.
    if (target != 0 && m_opcode == DbOpcode::Delete)
        return env.Undefined();

    auto val = target == 0 ? m_currentChange.GetOldValue(col) : m_currentChange.GetNewValue(col);
    if (!val.IsValid()) {
        return env.Undefined();
    }

    if (val.IsNull()) {
        return env.Null();
    }

    return Napi::Number::New(env, val.GetValueDouble());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::GetColumnValueText(Napi::Env env, int col, int target) {
    if (!HasRow() || !(col >= 0 && col < m_columnCount) || (target != 0 && target != 1)) {
        return env.Undefined();
    }

    // old value can be called by updated and deleted row.
    if (target == 0 && m_opcode == DbOpcode::Insert)
        return env.Undefined();

    // new value can be called by updated and inserted row.
    if (target != 0 && m_opcode == DbOpcode::Delete)
        return env.Undefined();

    auto val = target == 0 ? m_currentChange.GetOldValue(col) : m_currentChange.GetNewValue(col);
    if (!val.IsValid()) {
        return env.Undefined();
    }

    if (val.IsNull()) {
        return env.Null();
    }
    return Napi::String::New(env, val.GetValueText());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::GetColumnValueBinary(Napi::Env env, int col, int target) {
    if (!HasRow() || !(col >= 0 && col < m_columnCount) || (target != 0 && target != 1)) {
        return env.Undefined();
    }

    // old value can be called by updated and deleted row.
    if (target == 0 && m_opcode == DbOpcode::Insert)
        return env.Undefined();

    // new value can be called by updated and inserted row.
    if (target != 0 && m_opcode == DbOpcode::Delete)
        return env.Undefined();

    auto val = target == 0 ? m_currentChange.GetOldValue(col) : m_currentChange.GetNewValue(col);
    if (!val.IsValid()) {
        return env.Undefined();
    }
    auto nBytes = val.GetValueBytes();
    auto blob = Napi::Uint8Array::New(env, nBytes);
    memcpy(blob.Data(), val.GetValueBlob(), nBytes);
    return blob;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::IsColumnValueNull(Napi::Env env, int col, int target) {
    if (!HasRow() || !(col >= 0 && col < m_columnCount) || (target != 0 && target != 1)) {
        return env.Undefined();
    }

    // old value can be called by updated and deleted row.
    if (target == 0 && m_opcode == DbOpcode::Insert)
        return env.Undefined();

    // new value can be called by updated and inserted row.
    if (target != 0 && m_opcode == DbOpcode::Delete)
        return env.Undefined();

    auto val = target == 0 ? m_currentChange.GetOldValue(col) : m_currentChange.GetNewValue(col);
    if (!val.IsValid()) {
        return env.Undefined();
    }
    return Napi::Boolean::New(env, val.IsNull());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::GetColumnValueType(Napi::Env env, int col, int target) {
    if (!HasRow() || !(col >= 0 && col < m_columnCount) || (target != 0 && target != 1)) {
        return env.Undefined();
    }
    // old value can be called by updated and deleted row.
    if (target == 0 && m_opcode == DbOpcode::Insert)
        return env.Undefined();

    // new value can be called by updated and inserted row.
    if (target != 0 && m_opcode == DbOpcode::Delete)
        return env.Undefined();

    auto val = target == 0 ? m_currentChange.GetOldValue(col) : m_currentChange.GetNewValue(col);
    if (!val.IsValid()) {
        return env.Undefined();
    }
    return Napi::Number::New(env, (int)val.GetValueType());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::GetDdlChanges(Napi::Env env) {
    if (!IsOpen()) {
        THROW_JS_BE_SQLITE_EXCEPTION(env, "getDdlChanges(): no changeset opened.", BE_SQLITE_ERROR);
    }

    if (!m_ddl.empty())
        return Napi::String::New(env, m_ddl.c_str());

    return env.Undefined();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::GetColumnValue(Napi::Env env, int col, int target) {
    if (!HasRow() || !(col >= 0 && col < m_columnCount) || (target != 0 && target != 1)) {
        return env.Undefined();
    }

    // old value can be called by updated and deleted row.
    if (target == 0 && m_opcode == DbOpcode::Insert)
        return env.Undefined();

    // new value can be called by updated and inserted row.
    if (target != 0 && m_opcode == DbOpcode::Delete)
        return env.Undefined();

    auto val = target == 0 ? m_currentChange.GetOldValue(col) : m_currentChange.GetNewValue(col);
    if (!val.IsValid()) {
        return env.Undefined();
    }
    return SerializeValue(env, val);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::GetPrimaryKeyColumnIndexes(Napi::Env env) {
    if (!HasRow()) {
        THROW_JS_BE_SQLITE_EXCEPTION(env, "getPrimaryKeyColumnIndexes(): there is no current row.",  BE_SQLITE_ERROR);
    }

    auto row = Napi::Array::New(env, m_primaryKeyCount);
    uint32_t k = 0;
    for (int i = 0; i < m_columnCount; ++i) {
        if (m_primaryKeyColumns[i]) {
            row[k++] = Napi::Number::New(env, i);
        }
    }
    return row;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::GetRow(Napi::Env env, int target) {
    if (!HasRow()) {
        THROW_JS_BE_SQLITE_EXCEPTION(env, "getRow(): there is no current row.",  BE_SQLITE_ERROR);
    }
    // old value can be called by updated and deleted row.
    if (target == 0 && m_opcode == DbOpcode::Insert)
        return env.Undefined();

    // new value can be called by updated and inserted row.
    if (target != 0 && m_opcode == DbOpcode::Delete)
        return env.Undefined();

    auto row = Napi::Array::New(env, m_columnCount);
    for (int i = 0; i< m_columnCount; ++i)  {
        row[i] = GetColumnValue(env, i, target);
    }
    return row;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeChangeset::GetPrimaryKeys(Napi::Env env) {
    if (!HasRow()) {
        THROW_JS_BE_SQLITE_EXCEPTION(env, "getPrimaryKeys(): there is no current row.",  BE_SQLITE_ERROR);
    }

    auto row = Napi::Array::New(env, m_primaryKeyCount);
    auto k = 0;
    for (int i = 0; i< m_columnCount; ++i)  {
        if (m_primaryKeyColumns[i]) {
            row[k++] = GetColumnValue(env, i, m_opcode == DbOpcode::Insert ? 1 : 0 );
        }
    }
    return row;
}
