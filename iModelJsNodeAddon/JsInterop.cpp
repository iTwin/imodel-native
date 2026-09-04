/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#if defined (_WIN32) && !defined(BENTLEY_WINRT)
#include <windows.h>
#endif
#include "IModelJsNative.h"
#include "CsvRowsReader.h"
#include "V8SerializedRowsReader.h"
#include <Bentley/Base64Utilities.h>
#include <Bentley/Desktop/FileSystem.h>
#include <GeomSerialization/GeomSerializationApi.h>
#include <ECDb/ChangedIdsIterator.h>
#include <DgnPlatform/FunctionalDomain.h>
#if !defined (BENTLEYCONFIG_NO_VISUALIZATION)
    #include <Visualization/Visualization.h>
#endif
#include <DgnPlatform/EntityIdsChangeGroup.h>
#include <algorithm>
#include <cerrno>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <string>
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
    // The whole native operation is guarded: WithInstance throws for a closed db and Deserialize throws
    // for malformed/unsupported requests. Letting either escape into the N-API layer would call
    // std::terminate and take down the process.
    try {
        ConcurrentQueryMgr::WithInstance(ecdb, [&](ConcurrentQueryMgr& mgr) -> void {
            BeJsValue beJsReq(requestObj);
            QueryRequest::Ptr request = QueryRequest::Deserialize(beJsReq);
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
                        // this runs from the thread safe function, which N-API invokes through a plain
                        // C callback, so nothing may be thrown out of here. Turn any failure into a
                        // pending JS exception instead of letting it reach std::terminate.
                        try {
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
                        } catch (Napi::Error const& err) {
                            err.ThrowAsJavaScriptException();
                        } catch (std::exception const& ex) {
                            Napi::Error::New(env, ex.what()).ThrowAsJavaScriptException();
                        }
                }) != napi_ok) {
                    // do nothing
                }
                const_cast<Napi::ThreadSafeFunction&>(threadSafeFunc).Release();
            });
        });
    } catch (Napi::Error const&) {
        throw; // a JS exception must keep propagating so N-API can turn it back into a JS throw
    } catch (std::exception const& ex) {
        THROW_JS_IMODEL_NATIVE_EXCEPTION(Env(), ex.what(), IModelJsNativeErrorKey::BadArg);
    }
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
    EntityIdsChangeGroup entityIdsChangeGroup;
    entityIdsChangeGroup.ExtractChangedInstanceIdsFromChangeSets(db, changeSetFiles);

    auto addCategory = [&](Utf8CP categoryName, auto const& opMap)
        {
        bvector<Utf8String> insertIds, updateIds, deleteIds;
        for (auto const& entry : opMap)
            {
            if (entry.second == DbOpcode::Insert) insertIds.push_back(entry.first.ToHexStr());
            if (entry.second == DbOpcode::Update) updateIds.push_back(entry.first.ToHexStr());
            if (entry.second == DbOpcode::Delete) deleteIds.push_back(entry.first.ToHexStr());
            }

        if (insertIds.empty() && updateIds.empty() && deleteIds.empty())
            return;

        auto category = jsonOut[categoryName];
        auto addIds = [&](Utf8CP key, bvector<Utf8String> const& ids)
            {
            if (ids.empty())
                return;
            auto arr = category[key];
            arr.toArray();
            for (auto const& id : ids)
                arr.appendValue() = id;
            };

        addIds("insert", insertIds);
        addIds("update", updateIds);
        addIds("delete", deleteIds);
        };

    addCategory("element", entityIdsChangeGroup.elementOps);
    addCategory("aspect", entityIdsChangeGroup.aspectOps);
    addCategory("model", entityIdsChangeGroup.modelOps);
    addCategory("relationship", entityIdsChangeGroup.relationshipOps);
    addCategory("codeSpec", entityIdsChangeGroup.codeSpecOps);
    addCategory("font", entityIdsChangeGroup.fontOps);

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

DbResult JsInterop::DropSchemas(ECDbR ecdb, bvector<Utf8String>& schemaNames)
{
    NativeLogging::CategoryLogger logger("JsInterop");

    DropSchemaResult res = ecdb.Schemas().DropSchemas(schemaNames);
    if (!res.IsSuccess()) {
        Utf8String joined = BeStringUtilities::Join(schemaNames, ", ");
        logger.errorv("Failed to drop schema(s): %s", joined.c_str());
        return BE_SQLITE_ERROR;    
    }
    return ecdb.SaveChanges();
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
    
    // We want to manually add all schema folders here so when we later try and lookup schemas, the right paths are always consistently available
    if (sourceType == SchemaSourceType::File)
        {
        for(auto it = schemaSources.rbegin(); it != schemaSources.rend(); ++it)
            {
            BeFileName schemaFile(it->c_str(), BentleyCharEncoding::Utf8);
            BeFileName schemaDirectory (BeFileName::DevAndDir, schemaFile.GetWCharCP());
            schemaContext->AddSchemaPath(schemaDirectory, true); // We always add the last path we used to the top in the priority list, if it does not exist yet
            }
        }

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

        auto describeSchema = [](ECSchemaCP schema) -> Utf8PrintfString {
            return Utf8PrintfString("Schema: %s (version %d.%d.%d, origin: %s)",
                                   schema->GetName().c_str(),
                                   schema->GetVersionRead(),
                                   schema->GetVersionWrite(),
                                   schema->GetVersionMinor(),
                                   schema->GetOrigin().c_str());
        };

        Utf8PrintfString errorDetails("Schema paths provided to the method call (%d):\n", schemaSources.size());
        for(const auto& schemaFile : schemaSources)
        {
            errorDetails.append("    ").append(schemaFile).append("\n");
        }
        Utf8PrintfString providedSchemasMsg("Schemas provided to import schemas (%d):\n", schemas.size());
        errorDetails.append(providedSchemasMsg.c_str());
        for (const auto& schema : schemas)
        {
            errorDetails.append("    ").append(describeSchema(schema)).append(")\n");
        }
        const auto& cachedSchemas = schemaContext->GetCache().GetSchemas();
        Utf8PrintfString cachedSchemasMsg("Cached schemas in the context (%d):\n", cachedSchemas.size());
        errorDetails.append(cachedSchemasMsg.c_str());
        for(const auto& schema: cachedSchemas)
        {
            errorDetails.append("    ").append(describeSchema(schema)).append(")\n");
        }
        logger.errorv("Failed to import schemas. Details:\n%s", errorDetails.c_str());
        return DgnDb::SchemaStatusToDbResult(status, true);
        }

    if (!opts.m_skipSaveChanges)
        return dgndb.SaveChanges();

    return BE_SQLITE_OK;
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

namespace {
// Support code shared by ImportCSVData (V8-serialized rows from JS) and ImportCSVFile (rows streamed
// from disk). Both build one prepared INSERT statement from a column-index-to-property-name mapping
// and bind each mapped column's decoded text into it, row by row, inside a single savepoint.
struct CsvImportBinding {
    ECPropertyCP m_property;
    IECSqlBinder* m_binder;
    PrimitiveType m_primitiveType = PRIMITIVETYPE_Binary;
    bool m_isPrimitive = false;
};

struct CsvImportPlan {
    bvector<ECPropertyCP> m_properties;
    bvector<Utf8String> m_accessStrings;
};

ECPropertyCP resolveCsvImportProperty(ECClassCR ecClass, Utf8StringCR accessString, Utf8StringR canonicalAccessString) {
    ECClassCP currentClass = &ecClass;
    ECPropertyCP property = nullptr;
    size_t start = 0;
    while (start < accessString.size()) {
        const size_t end = accessString.find('.', start);
        Utf8String segment = accessString.substr(start, end == Utf8String::npos ? Utf8String::npos : end - start);
        if (segment.empty())
            return nullptr;

        property = currentClass->GetPropertyP(segment.c_str(), true);
        if (nullptr == property)
            return nullptr;

        if (!canonicalAccessString.empty())
            canonicalAccessString.append(".");
        canonicalAccessString.append("[").append(property->GetName()).append("]");

        if (end == Utf8String::npos)
            return property;

        const auto structProperty = property->GetAsStructProperty();
        if (nullptr == structProperty)
            return nullptr;

        currentClass = &structProperty->GetType();
        start = end + 1;
    }

    return nullptr;
}

bool createCsvImportPlan(CsvImportPlan& plan, Utf8StringR error, ECClassCR ecClass, BeJsConst propertyNames) {
    if (0 == propertyNames.size()) {
        error = "propertyNames must not be empty";
        return false;
    }

    bset<Utf8String, CompareIUtf8Ascii> canonicalNames;
    plan.m_properties.reserve(propertyNames.size());
    plan.m_accessStrings.reserve(propertyNames.size());
    for (BeJsConst::ArrayIndex i = 0; i < propertyNames.size(); ++i) {
        const auto propertyName = propertyNames[i];
        if (!propertyName.isString()) {
            error = "propertyNames must contain only strings";
            return false;
        }

        Utf8String canonicalName;
        const auto property = resolveCsvImportProperty(ecClass, propertyName.asString(), canonicalName);
        if (nullptr == property) {
            error = "propertyNames contains an invalid or unsupported property path";
            return false;
        }
        if (!canonicalNames.insert(canonicalName).second) {
            error = "propertyNames must not contain duplicates";
            return false;
        }

        plan.m_properties.push_back(property);
        plan.m_accessStrings.push_back(std::move(canonicalName));
    }
    return true;
}

bvector<CsvImportBinding> createCsvImportBindings(CsvImportPlan const& plan, ECSqlStatement& statement) {
    bvector<CsvImportBinding> bindings;
    bindings.reserve(plan.m_properties.size());
    for (size_t i = 0; i < plan.m_properties.size(); ++i) {
        const auto primitiveProperty = plan.m_properties[i]->GetAsPrimitiveProperty();
        bindings.push_back({
            plan.m_properties[i],
            &statement.GetBinder(static_cast<int>(i + 1)),
            nullptr == primitiveProperty ? PRIMITIVETYPE_Binary : primitiveProperty->GetType(),
            nullptr != primitiveProperty,
        });
    }
    return bindings;
}

bool decodeSerializedCSVValue(Utf8StringR decoded, V8SerializedRowsReader::Value const& value) {
    using ValueType = V8SerializedRowsReader::ValueType;
    if (ValueType::Utf8String == value.m_type) {
        decoded.assign(reinterpret_cast<Utf8CP>(value.m_bytes), value.m_byteCount);
        return true;
    }
    if (ValueType::Latin1String == value.m_type) {
        const auto end = value.m_bytes + value.m_byteCount;
        const auto nonAscii = std::find_if(value.m_bytes, end, [](uint8_t byte) { return 0 != (byte & 0x80U); });
        if (end == nonAscii) {
            decoded.assign(reinterpret_cast<Utf8CP>(value.m_bytes), value.m_byteCount);
            return true;
        }

        decoded.clear();
        decoded.reserve(value.m_byteCount * 2);
        for (auto current = value.m_bytes; current != end; ++current) {
            if (*current < 0x80U) {
                decoded.push_back(static_cast<char>(*current));
            } else {
                decoded.push_back(static_cast<char>(0xc0U | (*current >> 6U)));
                decoded.push_back(static_cast<char>(0x80U | (*current & 0x3fU)));
            }
        }
        return true;
    }
    if (ValueType::Utf16String == value.m_type) {
        std::u16string utf16(value.m_byteCount / sizeof(char16_t), u'\0');
        std::memcpy(utf16.data(), value.m_bytes, value.m_byteCount);
        return SUCCESS == BeStringUtilities::Utf16ToUtf8(decoded, reinterpret_cast<Utf16CP>(utf16.data()), utf16.size());
    }
    return false;
}

ECSqlStatus bindCsvImportValue(CsvImportBinding const& binding, Utf8StringCR value, Utf8CP nullValue) {
    if (nullptr != nullValue && value.Equals(nullValue))
        return binding.m_binder->BindNull();
    if (!binding.m_isPrimitive)
        return ECSqlStatus::Error;

    const auto begin = value.data();
    const auto end = begin + value.size();
    switch (binding.m_primitiveType) {
        case PRIMITIVETYPE_Boolean:
            if (value.Equals("true") || value.Equals("1"))
                return binding.m_binder->BindBoolean(true);
            if (value.Equals("false") || value.Equals("0"))
                return binding.m_binder->BindBoolean(false);
            return ECSqlStatus::Error;
        case PRIMITIVETYPE_Double: {
            if (value.empty())
                return ECSqlStatus::Error;
            char* parsedEnd = nullptr;
            errno = 0;
            const double parsed = std::strtod(begin, &parsedEnd);
            return 0 == errno && end == parsedEnd && std::isfinite(parsed) ? binding.m_binder->BindDouble(parsed) : ECSqlStatus::Error;
        }
        case PRIMITIVETYPE_Integer: {
            int32_t parsed = 0;
            const auto result = std::from_chars(begin, end, parsed);
            return std::errc() == result.ec && end == result.ptr ? binding.m_binder->BindInt(parsed) : ECSqlStatus::Error;
        }
        case PRIMITIVETYPE_String:
            return binding.m_binder->BindText(value.c_str(), IECSqlBinder::MakeCopy::No, static_cast<int>(value.size()));
        default:
            return ECSqlStatus::Error;
    }
}

ECSqlStatus bindSerializedCSVValue(CsvImportBinding const& binding, V8SerializedRowsReader::Value const& value, Utf8StringR decoded, Utf8CP nullValue) {
    if (!decodeSerializedCSVValue(decoded, value))
        return ECSqlStatus::Error;
    return bindCsvImportValue(binding, decoded, nullValue);
}

bool supportsCSVImportBinding(CsvImportBinding const& binding) {
    if (!binding.m_isPrimitive)
        return false;
    switch (binding.m_primitiveType) {
        case PRIMITIVETYPE_Boolean:
        case PRIMITIVETYPE_Double:
        case PRIMITIVETYPE_Integer:
        case PRIMITIVETYPE_String:
            return true;
        default:
            return false;
    }
}

bool parseCSVImportMapping(Napi::Array const& mapping, Napi::Array& propertyNames, bvector<uint32_t>& columnIndexes, uint32_t& minimumColumnCount, Utf8StringR error) {
    if (0 == mapping.Length()) {
        error = "mapping must not be empty";
        return false;
    }

    bset<uint32_t> seenColumnIndexes;
    columnIndexes.reserve(mapping.Length());
    minimumColumnCount = 0;
    for (uint32_t mappingIndex = 0; mappingIndex < mapping.Length(); ++mappingIndex) {
        const auto value = mapping.Get(mappingIndex);
        if (!value.IsObject()) {
            error = "mapping must contain only objects";
            return false;
        }

        const auto entry = value.As<Napi::Object>();
        const auto columnIndexValue = entry.Get("columnIndex");
        const auto propertyName = entry.Get("propertyName");
        if (!columnIndexValue.IsNumber() || !propertyName.IsString()) {
            error = "each mapping entry must contain a numeric columnIndex and string propertyName";
            return false;
        }

        const double columnIndexNumber = columnIndexValue.As<Napi::Number>().DoubleValue();
        if (columnIndexNumber < 0 || columnIndexNumber >= std::numeric_limits<uint32_t>::max() || std::floor(columnIndexNumber) != columnIndexNumber) {
            error = "mapping columnIndex values must be non-negative integers";
            return false;
        }

        const uint32_t columnIndex = static_cast<uint32_t>(columnIndexNumber);
        if (!seenColumnIndexes.insert(columnIndex).second) {
            error = "mapping must not contain duplicate columnIndex values";
            return false;
        }

        propertyNames.Set(mappingIndex, propertyName);
        columnIndexes.push_back(columnIndex);
        minimumColumnCount = std::max(minimumColumnCount, columnIndex + 1);
    }
    return true;
}
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Napi::Value JsInterop::ImportCSVData(ECDbR db, NapiInfoCR info) {
    REQUIRE_ARGUMENT_STRING(0, className);
    REQUIRE_ARGUMENT_ANY_OBJ(1, serializedRows);
    REQUIRE_ARGUMENT_ARRAY(2, mapping);
    OPTIONAL_ARGUMENT_ANY_OBJ(3, options, Napi::Object::New(info.Env()));

    if (!serializedRows.IsTypedArray() || serializedRows.As<Napi::TypedArray>().TypedArrayType() != napi_uint8_array)
        THROW_JS_TYPE_EXCEPTION("serializedRows must be a Uint8Array")
    const auto bytes = serializedRows.As<Napi::Uint8Array>();

    Utf8String nullValue;
    Utf8CP nullValuePtr = nullptr;
    const auto nullValueOption = options.Get("nullValue");
    if (!nullValueOption.IsUndefined()) {
        if (!nullValueOption.IsString())
            THROW_JS_TYPE_EXCEPTION("options.nullValue must be a string")
        nullValue = nullValueOption.As<Napi::String>().Utf8Value();
        nullValuePtr = nullValue.c_str();
    }

    auto propertyNames = Napi::Array::New(info.Env(), mapping.Length());
    bvector<uint32_t> csvColumnIndexes;
    uint32_t minimumColumnCount = 0;
    Utf8String mappingError;
    if (!parseCSVImportMapping(mapping, propertyNames, csvColumnIndexes, minimumColumnCount, mappingError))
        THROW_JS_TYPE_EXCEPTION(mappingError.c_str())

    const auto ecClass = db.Schemas().FindClass(className.c_str());
    if (nullptr == ecClass)
        THROW_JS_TYPE_EXCEPTION("className does not identify an ECClass")

    CsvImportPlan plan;
    Utf8String planError;
    if (!createCsvImportPlan(plan, planError, *ecClass, BeJsConst(propertyNames)))
        THROW_JS_TYPE_EXCEPTION(planError.c_str())

    Utf8String ecsql("INSERT INTO ");
    ecsql.append(ecClass->GetECSqlName()).append(" (");
    Utf8String valuesClause(") VALUES (");
    for (size_t i = 0; i < plan.m_accessStrings.size(); ++i) {
        if (i > 0) {
            ecsql.append(",");
            valuesClause.append(",");
        }
        ecsql.append(plan.m_accessStrings[i]);
        valuesClause.append("?");
    }
    ecsql.append(valuesClause).append(")");

    ECSqlStatement statement;
    const auto prepareStatus = statement.Prepare(db, ecsql.c_str());
    if (!prepareStatus.IsSuccess()) {
        const auto rc = prepareStatus.IsSQLiteError() ? prepareStatus.GetSQLiteError() : BE_SQLITE_ERROR;
        THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to prepare CSV data import ECSQL", rc)
    }

    auto bindings = createCsvImportBindings(plan, statement);
    if (std::any_of(bindings.begin(), bindings.end(), [](CsvImportBinding const& binding) { return !supportsCSVImportBinding(binding); }))
        THROW_JS_TYPE_EXCEPTION("CSV import supports only boolean, double, integer, and string properties")

    Savepoint savepoint(db, "importCSVData");
    if (!savepoint.IsActive())
        THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to start CSV data import savepoint", BE_SQLITE_ERROR)

    bvector<Utf8String> stringBuffers(plan.m_properties.size());
    bmap<uint32_t, uint32_t> propertyIndexesByColumn;
    for (uint32_t propertyIndex = 0; propertyIndex < csvColumnIndexes.size(); ++propertyIndex)
        propertyIndexesByColumn[csvColumnIndexes[propertyIndex]] = propertyIndex;

    uint32_t failedRow = 0;
    uint32_t failedColumn = 0;
    ECSqlStatus bindStatus = ECSqlStatus::Success;
    DbResult stepStatus = BE_SQLITE_DONE;
    uint32_t rowCount = 0;
    try {
        V8SerializedRowsReader reader(bytes.Data(), bytes.ByteLength());
        rowCount = reader.Read(minimumColumnCount, [&](uint32_t rowIndex, uint32_t columnIndex, uint32_t columnCount, V8SerializedRowsReader::Value const& value) {
            if (0 == columnIndex)
                statement.Reset();

            const auto propertyEntry = propertyIndexesByColumn.find(columnIndex);
            if (propertyEntry != propertyIndexesByColumn.end()) {
                const uint32_t propertyIndex = propertyEntry->second;
                bindStatus = bindSerializedCSVValue(bindings[propertyIndex], value, stringBuffers[propertyIndex], nullValuePtr);
                if (!bindStatus.IsSuccess()) {
                    failedRow = rowIndex;
                    failedColumn = columnIndex;
                    return false;
                }
            }

            if (columnIndex + 1 == columnCount) {
                stepStatus = statement.Step();
                if (BE_SQLITE_DONE != stepStatus) {
                    failedRow = rowIndex;
                    return false;
                }
            }
            return true;
        });
    } catch (V8SerializedRowsError const& error) {
        const auto rollbackStatus = savepoint.Cancel();
        if (BE_SQLITE_OK != rollbackStatus)
            THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to roll back CSV data import", rollbackStatus)
        THROW_JS_TYPE_EXCEPTION(error.what())
    } catch (...) {
        const auto rollbackStatus = savepoint.Cancel();
        if (BE_SQLITE_OK != rollbackStatus)
            THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to roll back CSV data import", rollbackStatus)
        throw;
    }

    if (!bindStatus.IsSuccess()) {
        const auto rollbackStatus = savepoint.Cancel();
        if (BE_SQLITE_OK != rollbackStatus)
            THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to roll back CSV data import", rollbackStatus)
        THROW_JS_TYPE_EXCEPTION(Utf8PrintfString("Failed to bind CSV data row %" PRIu32 " column %" PRIu32, failedRow + 1, failedColumn).c_str())
    }
    if (BE_SQLITE_DONE != stepStatus) {
        const auto rollbackStatus = savepoint.Cancel();
        if (BE_SQLITE_OK != rollbackStatus)
            THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to roll back CSV data import", rollbackStatus)
        THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), Utf8PrintfString("Failed to insert CSV data row %" PRIu32, failedRow + 1).c_str(), stepStatus)
    }

    const auto commitStatus = savepoint.Commit();
    if (BE_SQLITE_OK != commitStatus) {
        const auto rollbackStatus = savepoint.Cancel();
        if (BE_SQLITE_OK != rollbackStatus)
            THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to commit or roll back CSV data import", rollbackStatus)
        THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to commit CSV data import", commitStatus)
    }

    return Napi::Number::New(info.Env(), rowCount);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Napi::Value JsInterop::ImportCSVFile(ECDbR db, NapiInfoCR info) {
    REQUIRE_ARGUMENT_STRING(0, className);
    REQUIRE_ARGUMENT_STRING(1, csvFilePath);
    REQUIRE_ARGUMENT_ARRAY(2, mapping);
    OPTIONAL_ARGUMENT_ANY_OBJ(3, options, Napi::Object::New(info.Env()));

    const auto hasHeaderValue = options.Get("hasHeader");
    if (!hasHeaderValue.IsUndefined() && !hasHeaderValue.IsBoolean())
        THROW_JS_TYPE_EXCEPTION("options.hasHeader must be a boolean")
    const bool hasHeader = !hasHeaderValue.IsUndefined() && hasHeaderValue.As<Napi::Boolean>().Value();

    Utf8String nullValue;
    Utf8CP nullValuePtr = nullptr;
    const auto nullValueOption = options.Get("nullValue");
    if (!nullValueOption.IsUndefined()) {
        if (!nullValueOption.IsString())
            THROW_JS_TYPE_EXCEPTION("options.nullValue must be a string")
        nullValue = nullValueOption.As<Napi::String>().Utf8Value();
        nullValuePtr = nullValue.c_str();
    }

    auto propertyNames = Napi::Array::New(info.Env(), mapping.Length());
    bvector<uint32_t> csvColumnIndexes;
    uint32_t minimumColumnCount = 0;
    Utf8String mappingError;
    if (!parseCSVImportMapping(mapping, propertyNames, csvColumnIndexes, minimumColumnCount, mappingError))
        THROW_JS_TYPE_EXCEPTION(mappingError.c_str())

    const auto ecClass = db.Schemas().FindClass(className.c_str());
    if (nullptr == ecClass)
        THROW_JS_TYPE_EXCEPTION("className does not identify an ECClass")

    CsvImportPlan plan;
    Utf8String planError;
    if (!createCsvImportPlan(plan, planError, *ecClass, BeJsConst(propertyNames)))
        THROW_JS_TYPE_EXCEPTION(planError.c_str())

    Utf8String ecsql("INSERT INTO ");
    ecsql.append(ecClass->GetECSqlName()).append(" (");
    Utf8String valuesClause(") VALUES (");
    for (size_t i = 0; i < plan.m_accessStrings.size(); ++i) {
        if (i > 0) {
            ecsql.append(",");
            valuesClause.append(",");
        }
        ecsql.append(plan.m_accessStrings[i]);
        valuesClause.append("?");
    }
    ecsql.append(valuesClause).append(")");

    ECSqlStatement statement;
    const auto prepareStatus = statement.Prepare(db, ecsql.c_str());
    if (!prepareStatus.IsSuccess()) {
        const auto rc = prepareStatus.IsSQLiteError() ? prepareStatus.GetSQLiteError() : BE_SQLITE_ERROR;
        THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to prepare CSV file import ECSQL", rc)
    }

    auto bindings = createCsvImportBindings(plan, statement);
    if (std::any_of(bindings.begin(), bindings.end(), [](CsvImportBinding const& binding) { return !supportsCSVImportBinding(binding); }))
        THROW_JS_TYPE_EXCEPTION("CSV import supports only boolean, double, integer, and string properties")

    Savepoint savepoint(db, "importCSVFile");
    if (!savepoint.IsActive())
        THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to start CSV file import savepoint", BE_SQLITE_ERROR)

    uint64_t failedRow = 0;
    uint32_t failedColumn = 0;
    ECSqlStatus bindStatus = ECSqlStatus::Success;
    DbResult stepStatus = BE_SQLITE_DONE;
    uint64_t rowCount = 0;
    try {
        CsvRowsReader reader(csvFilePath);
        rowCount = reader.Read(minimumColumnCount, hasHeader, [&](uint64_t recordIndex, CsvRowsReader::Row const& fields) {
            statement.Reset();
            for (uint32_t propertyIndex = 0; propertyIndex < bindings.size(); ++propertyIndex) {
                const uint32_t csvColumnIndex = csvColumnIndexes[propertyIndex];
                bindStatus = bindCsvImportValue(bindings[propertyIndex], fields[csvColumnIndex], nullValuePtr);
                if (!bindStatus.IsSuccess()) {
                    failedRow = recordIndex;
                    failedColumn = csvColumnIndex;
                    return false;
                }
            }

            stepStatus = statement.Step();
            if (BE_SQLITE_DONE != stepStatus) {
                failedRow = recordIndex;
                return false;
            }
            return true;
        });
    } catch (CsvRowsError const& error) {
        const auto rollbackStatus = savepoint.Cancel();
        if (BE_SQLITE_OK != rollbackStatus)
            THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to roll back CSV file import", rollbackStatus)
        THROW_JS_TYPE_EXCEPTION(error.what())
    } catch (...) {
        const auto rollbackStatus = savepoint.Cancel();
        if (BE_SQLITE_OK != rollbackStatus)
            THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to roll back CSV file import", rollbackStatus)
        throw;
    }

    if (!bindStatus.IsSuccess()) {
        const auto rollbackStatus = savepoint.Cancel();
        if (BE_SQLITE_OK != rollbackStatus)
            THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to roll back CSV file import", rollbackStatus)
        THROW_JS_TYPE_EXCEPTION(Utf8PrintfString("Failed to bind CSV record %" PRIu64 " column %" PRIu32, failedRow + 1, failedColumn).c_str())
    }
    if (BE_SQLITE_DONE != stepStatus) {
        const auto rollbackStatus = savepoint.Cancel();
        if (BE_SQLITE_OK != rollbackStatus)
            THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to roll back CSV file import", rollbackStatus)
        THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), Utf8PrintfString("Failed to insert CSV record %" PRIu64, failedRow + 1).c_str(), stepStatus)
    }

    const auto commitStatus = savepoint.Commit();
    if (BE_SQLITE_OK != commitStatus) {
        const auto rollbackStatus = savepoint.Cancel();
        if (BE_SQLITE_OK != rollbackStatus)
            THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to commit or roll back CSV file import", rollbackStatus)
        THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "Failed to commit CSV file import", commitStatus)
    }

    return Napi::Number::New(info.Env(), static_cast<double>(rowCount));
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
Napi::Value SqliteChangesetReader::SerializeValue(Napi::Env env, DbValue&value) {
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
void SqliteChangesetReader::OpenFile(Napi::Env env, Utf8StringCR changesetFile, bool invert) {
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
void SqliteChangesetReader::OpenChangeStream(Napi::Env env, std::unique_ptr<ChangeStream> changeStream, bool invert) {
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
void SqliteChangesetReader::OpenGroup(Napi::Env env, T_Utf8StringVector const& changesetFiles, Db const& db, bool invert) {
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
void SqliteChangesetReader::WriteToFile(Napi::Env env, Utf8String const& fileName, bool containChanges, bool override) {
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
void SqliteChangesetReader::Close(Napi::Env env) {
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
void SqliteChangesetReader::Reset(Napi::Env env) {
    m_currentChange = Changes::Change(nullptr, false);
    m_changes = nullptr;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value SqliteChangesetReader::Step(Napi::Env env) {
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
Napi::Value SqliteChangesetReader::GetTableName(Napi::Env env) {
    if (!HasRow()) {
        THROW_JS_BE_SQLITE_EXCEPTION(env, "getTableName(): there is no current row.", BE_SQLITE_ERROR);
    }

    return Napi::String::New(env, m_tableName);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value SqliteChangesetReader::GetOpCode(Napi::Env env) {
    if (!HasRow()) {
        THROW_JS_BE_SQLITE_EXCEPTION(env, "getOpCode(): there is no current row.", BE_SQLITE_ERROR);
    }

    return Napi::Number::New(env, (int)m_opcode);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value SqliteChangesetReader::IsIndirectChange(Napi::Env env) {
    if (!HasRow()) {
        THROW_JS_BE_SQLITE_EXCEPTION(env, "isIndirectChange(): there is no current row.", BE_SQLITE_ERROR);
    }

    return Napi::Boolean::New(env, (int)m_indirect);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value SqliteChangesetReader::GetColumnCount(Napi::Env env) {
    if (!HasRow()) {
        THROW_JS_BE_SQLITE_EXCEPTION(env, "getColumnCount(): there is no current row.", BE_SQLITE_ERROR);
    }

    return Napi::Number::New(env, m_columnCount);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value SqliteChangesetReader::GetHasRow(Napi::Env env) {
    return Napi::Boolean::New(env, HasRow());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value SqliteChangesetReader::GetColumnValueInteger(Napi::Env env, int col, int target){
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
Napi::Value SqliteChangesetReader::GetColumnValueId(Napi::Env env, int col, int target){
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
Napi::Value SqliteChangesetReader::GetColumnValueDouble(Napi::Env env, int col, int target){
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
Napi::Value SqliteChangesetReader::GetColumnValueText(Napi::Env env, int col, int target) {
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
Napi::Value SqliteChangesetReader::GetColumnValueBinary(Napi::Env env, int col, int target) {
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
Napi::Value SqliteChangesetReader::IsColumnValueNull(Napi::Env env, int col, int target) {
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
Napi::Value SqliteChangesetReader::GetColumnValueType(Napi::Env env, int col, int target) {
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
Napi::Value SqliteChangesetReader::GetDdlChanges(Napi::Env env) {
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
Napi::Value SqliteChangesetReader::GetColumnValue(Napi::Env env, int col, int target) {
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
Napi::Value SqliteChangesetReader::GetPrimaryKeyColumnIndexes(Napi::Env env) {
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
Napi::Value SqliteChangesetReader::GetRow(Napi::Env env, int target) {
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
Napi::Value SqliteChangesetReader::GetPrimaryKeys(Napi::Env env) {
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
