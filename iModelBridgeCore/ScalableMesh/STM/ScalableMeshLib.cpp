/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshLib.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"

#if defined(VANCOUVER_API) || defined(DGNDB06_API)
USING_NAMESPACE_BENTLEY_DGNPLATFORM
#endif

#include <ScalableMesh/ScalableMeshLib.h>
//#include <TerrainModel/ElementHandler/DTMElementHandlerManager.h>
#include "Plugins/ScalableMeshTypeConversionFilterPlugins.h"
#include "ScalableMeshFileMoniker.h"
#ifndef LINUX_SCALABLEMESH_BUILD
#include "ScalableMeshRDSProvider.h"
#endif
#include <ScalableMesh/IScalableMeshProgressiveQuery.h>
#include "SMMemoryPool.h"
#include <CloudDataSource/DataSourceManager.h>
#include <ImagePP/all/h/HFCCallbacks.h>
#include <ImagePP/all/h/HFCCallbackRegistry.h>
#include <ImagePP/all/h/ImageppLib.h>
#include <Logging/bentleylogging.h>
#ifndef LINUX_SCALABLEMESH_BUILD
#include <DgnPlatform/DgnPlatformLib.h>
#endif

#ifndef LINUX_SCALABLEMESH_BUILD
#include <CCApi/CCPublic.h>
#endif
#include <curl/curl.h>


#ifndef VANCOUVER_API
USING_NAMESPACE_IMAGEPP
#endif


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

#define BING_AUTHENTICATION_KEY "AnLjDxNA_guaYuWWJifrpWnqvlxWPl8lLHzT1ixQH3vXLwb3CTEolWX34nbn4HfS"

struct SMImagePPHost : public ImageppLib::Host
    {
    virtual void             _RegisterFileFormat() override
        {

        }
    };

ScalableMeshLib::Host*        t_scalableTerrainModelHost;
SMImagePPHost*        t_ippLibHost;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                     11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ScalableMeshAdmin& ScalableMeshLib::Host::_SupplyScalableMeshAdmin()
    {
    return *new ScalableMeshAdmin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Elenie.Godzaridis                     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef VANCOUVER_API
STMAdmin& ScalableMeshLib::Host::_SupplySTMAdmin()
    {
    return *new STMAdmin();
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.St-Pierre  05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
#ifndef LINUX_SCALABLEMESH_BUILD
//=======================================================================================
// @bsiclass
//=======================================================================================
struct WebServiceKey
    {
    private:
        Utf8String m_key;
        DateTime m_expiration;

    public:
        WebServiceKey() :m_key(""), m_expiration(DateTime()) {}
        WebServiceKey(Utf8StringCR key) : m_key(key), m_expiration(DateTime()) {}
        WebServiceKey(Utf8StringCR key, DateTimeCR expiration) : m_key(key), m_expiration(expiration) {}
        bool IsValid() const { return !m_key.empty(); }
        Utf8String GetKey() const { return m_key; }
        void SetKey(Utf8StringCR key) { m_key = key; }
        DateTime GetExpiration() const { return m_expiration; }
        void SetExpiration(DateTimeCR expiration) { m_expiration = expiration; }
        bool IsExpired() const { return m_expiration.IsValid() && DateTime::Compare(m_expiration, DateTime::GetCurrentTimeUtc()) != DateTime::CompareResult::LaterThan; }
    };

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Mathieu.St-Pierre               10/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
    ((Utf8String*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Mathieu.St-Pierre               10/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
void GetCertificateAutoritiesFileUrl(Utf8String& pemFileName)
    {
    HMODULE hm = NULL;

    if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            (LPCSTR) &WriteCallback,
                            &hm))
        {
        assert(!"Cannot get ScalableMesh DLL path");
        }

    WCHAR wccwd[FILENAME_MAX];
    GetModuleFileNameW(hm, &wccwd[0], (DWORD) FILENAME_MAX);
    BeFileName cwdfn(wccwd);

#ifdef VANCOUVER_API    
    pemFileName.append(Utf8String(BeFileName::GetDirectoryName(cwdfn.c_str()).c_str()).c_str());
#else    
    pemFileName.append(Utf8String(cwdfn.GetDirectoryName().c_str()).c_str());
#endif

    pemFileName.append("\\ScalableMeshCacert.pem");
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Mathieu.St-Pierre                 10/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
CURLcode RequestHttp(Utf8StringCR url, Utf8StringCP writeString, FILE* fp, Utf8StringCR postFields)
    {
    BeAssert(nullptr != writeString || nullptr != fp);
    auto curl = curl_easy_init();
    if (nullptr == curl)
        {
        //m_error = CONCEPTSTATIONL10N_GETSTRING(STATUS_ERR_CurlNotAvailable);
        return CURLcode::CURLE_FAILED_INIT;
        }

    //Adjusting headers for the POST method
    struct curl_slist *headers = NULL;
    if (!postFields.empty())
        {
        headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "charsets: utf-8");
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postFields.length());
        }
  
#ifdef DGNDB06_API
    CurlConstructor curlConstructor;
#else
    RequestConstructor curlConstructor;
#endif
    //headers = curl_slist_append(headers, ConnectTokenManager::GetInstance().GetToken().c_str());

    headers = curl_slist_append(headers, curlConstructor.GetToken().c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    Utf8String pemFileName;

#ifdef VANCOUVER_API //On Bim02 CURL seems not to use OpenSSL, so using certificate file will result in an error.
    GetCertificateAutoritiesFileUrl(pemFileName);
    curl_easy_setopt(curl, CURLOPT_CAINFO, pemFileName.c_str());
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
#endif

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_HEADEROPT, CURLHEADER_SEPARATE);    

    ScalableMeshAdmin::ProxyInfo proxyInfo(ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetProxyInfo());
    if (!proxyInfo.m_serverUrl.empty())
        {
        curl_easy_setopt(curl, CURLOPT_PROXY, proxyInfo.m_serverUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
        if (!proxyInfo.m_user.empty() && !proxyInfo.m_password.empty())
            {
            Utf8String proxyCreds = proxyInfo.m_user;
            proxyCreds.append(":");
            proxyCreds.append(proxyInfo.m_password);
            curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, proxyCreds.c_str());
            }
        }

    if (nullptr != fp)
        {
        /*
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        */
        }
    else
        {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, writeString);
        }

    CURLcode result = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return result;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Mathieu.St-Pierre                 10/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
CURLcode PerformCurl(Utf8StringCR url, Utf8StringCP writeString, FILE* fp, Utf8StringCR postFields)
    {
    CURLcode code = RequestHttp(url, writeString, fp, postFields);    
    return code;
    }
//#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.St-Pierre  10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
WebServiceKey GetBingKey()
    {    

    Utf8String readBuffer;    
    BENTLEY_NAMESPACE_NAME::NativeLogging::ILogger*   logger = BENTLEY_NAMESPACE_NAME::NativeLogging::LoggingManager::GetLogger("Bing");

    logger->debug("Retrieving Bing Key from CC");

    WString serverUrl;

    WString buddiUrl;
    UINT32 bufLen;
    CallStatus status = APIERR_SUCCESS;
    try
        {
        char tempBuffer[100000];

    CCAPIHANDLE api = CCApi_InitializeApi(COM_THREADING_Multi);

        bool sessionActive = false;
        status = CCApi_IsUserSessionActive(api, &sessionActive);
        sprintf(tempBuffer, "user Session Active status: %ld Active: %ld", status, sessionActive ? 1 : 0);
        logger->debug(tempBuffer);

        wchar_t* buffer;
        status = CCApi_GetBuddiUrl(api, L"ContextServices", NULL, &bufLen);
        if (APIERR_SUCCESS != status)
            {
            sprintf(tempBuffer, "1st GetBuddiURL status : %ld", status);
            logger->error(tempBuffer);
        }

        bufLen++;
        buffer = (wchar_t*) calloc(1, bufLen * sizeof(wchar_t));
        status = CCApi_GetBuddiUrl(api, L"ContextServices", buffer, &bufLen);
        if (APIERR_SUCCESS != status)
            {
            char tempBuffer2[100000];
            sprintf(tempBuffer2, "2nd GetBuddiURL status: %ld", status);
            logger->error(tempBuffer);
            }
        serverUrl.assign(buffer);
        CCApi_FreeApi(api);
        }
    catch (...)
        {
        logger->error("CC exception caught");
        return WebServiceKey();
    }

    logger->debug(serverUrl.c_str());

    Utf8String contextServiceURL;
    contextServiceURL.assign(Utf8String(serverUrl.c_str()).c_str());

    uint64_t productId(ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetProductId());

    Utf8String productIdStr;

#ifdef VANCOUVER_API    
    wchar_t prodIdStr[200];    
    BeStringUtilities::Snwprintf(prodIdStr, 200, L"%u", productId);
    BeStringUtilities::WCharToUtf8(productIdStr, prodIdStr);
#else
    Utf8Char prodIdStr[200];
    BeStringUtilities::FormatUInt64(prodIdStr, productId);
    productIdStr.append(prodIdStr);
#endif

    Utf8String bingKeyUrl(contextServiceURL);
    bingKeyUrl.append("v2.4/repositories/ContextKeyService--Server/ContextKeyServiceSchema/BingApiKey?$filter=productId+eq+");
    bingKeyUrl.append(productIdStr.c_str());

    logger->debug("Perform curl using");
    logger->debug(bingKeyUrl.c_str());


#if 0 //Would be more generic to use (and would alleviate the need for security code maintance) but seems to have singleton crash problem.
    Http::Request httpRequest(bingKeyUrl);
    httpRequest.SetValidateCertificate(true);

    Http::HttpRequestHeadersR httpHeader(httpRequest.GetHeaders());


    RequestConstructor curlConstructor;
    Utf8String token = curlConstructor.GetToken();
    
    Utf8String field;
    Utf8String value;

    int tokenInd = 0;
    size_t offset = 0;
    Utf8String m;
    while ((offset = token.GetNextToken (m, ":", offset)) != Utf8String::npos)
        {
        if (tokenInd == 0)
            {
            field = m;
            }
        else
            {
            value += m /*+ Utf8String(" ")*/;
            }

        tokenInd++;
        }

    httpHeader.AddValue(field, value);

    folly::Future<Http::Response> httpResponseAsync(httpRequest.Perform());

    httpResponseAsync.wait();

    if (!httpResponseAsync.get().IsSuccess())
        {
        logger->error("curl failed, returning empty key");
        return WebServiceKey();
        }

    /*
    Http::HttpBodyCR httpBody(httpResponseAsync.get().GetBody());
    readBuffer = httpBody.AsString();
    */

    Http::HttpBodyPtr httpBody(httpRequest.GetResponseBody());
    readBuffer = httpBody->AsString();        
#endif

    Utf8String postFields;
    CURLcode result = PerformCurl(bingKeyUrl, &readBuffer, nullptr, postFields);
    
    if (CURLE_OK != result)
        {
        logger->error("curl failed, returning empty key");
        return WebServiceKey();
        }

    Json::Value packageInfos(Json::objectValue);
    Json::Reader::Parse(readBuffer, packageInfos);

    if (!packageInfos.isMember("instances"))
        {
        logger->error("instances is not a member of packageInfos, returning empty key");
        return WebServiceKey();
        }

    if (packageInfos["instances"].isArray() &&
        packageInfos["instances"].isValidIndex(0) &&
        packageInfos["instances"][0].isMember("properties") &&
        packageInfos["instances"][0]["properties"].isMember("key") &&
        packageInfos["instances"][0]["properties"].isMember("expirationDate"))
        {
        DateTime expiration;
        DateTime::FromString(expiration, packageInfos["instances"][0]["properties"]["expirationDate"].asCString());
        logger->debug("Key retrieved");
        logger->debug(packageInfos["instances"][0]["properties"]["key"].asCString());

        return WebServiceKey(packageInfos["instances"][0]["properties"]["key"].asString(), expiration);
        }

    logger->error("invalid instances in packageInfos, returning empty key");
    return WebServiceKey();
    }

//=======================================================================================
// @bsiclass                                                    Raphael.Lemieux 09/2017
//=======================================================================================
class BingAuthenticationCallback : public HFCAuthenticationCallback, public RefCountedBase
    {
    private:
        mutable WebServiceKey m_bingKey;
    public:
        BingAuthenticationCallback() : m_bingKey(WebServiceKey()) {};
        virtual             ~BingAuthenticationCallback() {};


        bool       GetAuthentication(HFCAuthentication* pio_Authentication) const override;
        unsigned short RetryCount(HCLASS_ID pi_AuthenticationType) const override { return 1; }
        bool       IsCancelled() const override { return !m_bingKey.IsValid(); }
        bool       CanAuthenticate(HCLASS_ID pi_AuthenticationType) const override { return true; }
    };

typedef RefCountedPtr<BingAuthenticationCallback> BingAuthenticationCallbackPtr;

//=======================================================================================
// @bsiclass                                                    Raphael.Lemieux 09/2017
//=======================================================================================
bool BingAuthenticationCallback::GetAuthentication(HFCAuthentication* pio_Authentication) const
    {
    HFCInternetAuthentication* pAuth = dynamic_cast<HFCInternetAuthentication*>(pio_Authentication);

    if (pAuth != nullptr)
        {
#if defined(VANCOUVER_API) || defined(DGNDB06_API)
		if (nullptr == pAuth || !pAuth->GetServer().ContainsI(L"bing"))
			return false;
#else
        if (nullptr == pAuth || !pAuth->GetServer().ContainsI("bing"))
            return false;
#endif

        WString key = L"";
        if (!m_bingKey.IsValid() || m_bingKey.IsExpired())
            {
            m_bingKey = GetBingKey();
            }

        if (m_bingKey.IsValid() && !m_bingKey.IsExpired())
            key.AssignUtf8(m_bingKey.GetKey().c_str());
#if defined(VANCOUVER_API) || defined(DGNDB06_API)
        pAuth->SetPassword(key);
#else
		pAuth->SetPassword(Utf8String(key));
#endif
        return !key.empty();
        }

    HFCProxyAuthentication* pProxyAuth = dynamic_cast<HFCProxyAuthentication*>(pio_Authentication);

    if (pProxyAuth != nullptr)
        {
        //std::shared_ptr<ProxyHttpHandler> pProxyConfig(AppSettings::GetSharedPointer()->GetProxyConfig());

        ScalableMeshAdmin::ProxyInfo proxyInfo(ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetProxyInfo());
        if (!proxyInfo.m_serverUrl.empty())
            {
#if defined(VANCOUVER_API) || defined(DGNDB06_API)
            pProxyAuth->SetUser(WString(proxyInfo.m_user.c_str(), true));
            pProxyAuth->SetPassword(WString(proxyInfo.m_password.c_str(), true));
            pProxyAuth->SetServer(WString(proxyInfo.m_serverUrl.c_str(), true));
#else
			pProxyAuth->SetUser(proxyInfo.m_user);
            pProxyAuth->SetPassword(proxyInfo.m_password);
            pProxyAuth->SetServer(proxyInfo.m_serverUrl);
#endif
            return true;
            }

        return false;
        }

	HFCCertificateAutoritiesAuthentication* pCertAutorityAuth = dynamic_cast<HFCCertificateAutoritiesAuthentication*>(pio_Authentication);

    if (pCertAutorityAuth != nullptr)
        {
        //std::shared_ptr<ProxyHttpHandler> pProxyConfig(AppSettings::GetSharedPointer()->GetProxyConfig());

        Utf8String pemFileName;

        GetCertificateAutoritiesFileUrl(pemFileName);        

        if (!pemFileName.empty())
            {
#ifdef VANCOUVER_API
            pCertAutorityAuth->SetCertificateAuthFileUrl(WString(pemFileName.c_str(), true));
#else	
            //On Bim02 CURL seems not to use OpenSSL, so using certificate file will result in an error. 
            //pCertAutorityAuth->SetCertificateAuthFileUrl(pemFileName);
#endif
            return true;
            }

        return false;
        }
    assert(!"Unknown/unsupported HFCAuthentication type");

    return false;
    }


#ifdef VANCOUVER_API

    #if defined(__BENTLEYSTM_BUILD__) && defined(__BENTLEYSTMIMPORT_BUILD__) 
        void RegisterPODImportPlugin();
    #else
        RegisterPODImportPluginFP ScalableMeshLib::s_PODImportRegisterFP = nullptr;        
    #endif

#endif

static BingAuthenticationCallbackPtr s_bingAuthCallback;
#endif
void ScalableMeshLib::Host::Initialize()
    {
    BeAssert(NULL == m_scalableTerrainModelAdmin);
    SMMemoryPool::GetInstance();
    m_scalableTerrainModelAdmin = &_SupplyScalableMeshAdmin();
#ifdef VANCOUVER_API
    m_stmAdmin = &_SupplySTMAdmin();
#endif
    m_smPaths = new bmap<WString, IScalableMesh*>();
    InitializeProgressiveQueries();

#ifdef VANCOUVER_API

#if defined(__BENTLEYSTM_BUILD__) && defined(__BENTLEYSTMIMPORT_BUILD__) 

    RegisterPODImportPlugin();

#else
       
    if (ScalableMeshLib::GetPodRegister() != nullptr)
        {
        (*ScalableMeshLib::GetPodRegister())();
        }
#endif
    
#else
    //NEEDS_WORK_SM_POD_B0200
    //RegisterPODImportPlugin();
#endif
    
    BeFileName geocoordinateDataPath(L".\\GeoCoordinateData\\");
    GeoCoordinates::BaseGCS::Initialize(geocoordinateDataPath.c_str());
    //BENTLEY_NAMESPACE_NAME::TerrainModel::Element::DTMElementHandlerManager::InitializeDgnPlatform();

#ifndef LINUX_SCALABLEMESH_BUILD
    //Ensure to avoid overwriting any specialized Imagepp authentication provided by an application.
    if (m_scalableTerrainModelAdmin->_ProvideImageppAuthentication())
        {
        s_bingAuthCallback = new BingAuthenticationCallback();

        HFCCallbackRegistry::GetInstance()->AddCallback(s_bingAuthCallback.get());
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.St-Pierre  05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ScalableMeshLib::Host::Terminate(bool onProgramExit)
    {
    if (NULL == t_scalableTerrainModelHost)
        return;

    if (t_ippLibHost != NULL)
        {
        t_ippLibHost->Terminate(onProgramExit);
        delete t_ippLibHost;
        }

    //Terminate host objects
    for (bvector<ObjEntry>::iterator itr = m_hostObj.begin(); itr != m_hostObj.end(); ++itr)
        {
        IHostObject* pValue(itr->GetValue());
#if defined(VANCOUVER_API) || defined(DGNDB06_API)
        TERMINATE_HOST_OBJECT(pValue, onProgramExit);
#else
		ON_HOST_TERMINATE(pValue, onProgramExit);
#endif
        }

    m_hostObj.clear();
    m_hostVar.clear();


#if defined(VANCOUVER_API) || defined(DGNDB06_API)
    TERMINATE_HOST_OBJECT(m_scalableTerrainModelAdmin, onProgramExit);
#else
    ON_HOST_TERMINATE(m_scalableTerrainModelAdmin, onProgramExit);
#endif

    delete m_smPaths;

    t_scalableTerrainModelHost = NULL;
    TerminateProgressiveQueries();

    DataSourceManager::Shutdown();

    }



IScalableMeshPtr ScalableMeshLib::Host::GetRegisteredScalableMesh(const WString& path)
    {
    if (m_smPaths->count(path) > 0) return (*m_smPaths)[path];
    return nullptr;
    }

void             ScalableMeshLib::Host::RemoveRegisteredScalableMesh(const WString& path)
    {
    if (m_smPaths && m_smPaths->count(path) > 0) m_smPaths->erase(path);
    }

void ScalableMeshLib::Host::RegisterScalableMesh(const WString& path, IScalableMeshPtr& ref)
    {
    if (!m_smPaths->insert(make_bpair(path, ref.get())).second)
        BeAssert(!"Path already exists");
    }

/*======================================================================+
|                                                                       |
|   Major Public Code Section                                           |
|                                                                       |
+======================================================================*/


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.St-Pierre  05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ScalableMeshLib::Initialize(ScalableMeshLib::Host& host)
    {
    if (!ImageppLib::IsInitialized())
        {
        t_ippLibHost = new SMImagePPHost();
        ImageppLib::Initialize(*t_ippLibHost);
        }
    //BeAssert(NULL == t_scalableTerrainModelHost);  // It is ok to be called twice on the same thread
    if (NULL != t_scalableTerrainModelHost)
        return;


    //register types

    const WChar TIN_AS_LINEAR_HEADER_TYPE_NAME[] = L"TINAsLinearHeader";
    static BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::DimensionType::Register s_RegisterTINAsLinearHeaderType(TIN_AS_LINEAR_HEADER_TYPE_NAME, sizeof(ISMStore::FeatureHeader));

    const WChar TIN_AS_LINEAR_POINT_TYPE_NAME[] = L"TINAsLinearPoint";
    static BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::DimensionType::Register s_RegisterTINAsLinearPointType(TIN_AS_LINEAR_POINT_TYPE_NAME, sizeof(DPoint3d));

    const WChar MESH_AS_LINEAR_HEADER_TYPE_NAME[] = L"MeshAsLinearHeader";
    BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::DimensionType::Register s_RegisterMesAsLinearPointType(MESH_AS_LINEAR_HEADER_TYPE_NAME, sizeof(ISMStore::FeatureHeader));

    const WChar MESH_AS_LINEAR_POINT_TYPE_NAME[] = L"MeshAsLinearPoint";
    BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::DimensionType::Register s_RegisterMeshAsLinearPointIdxType(MESH_AS_LINEAR_POINT_TYPE_NAME, sizeof(DPoint3d));
    const WChar MESH_PTS_NAME[] = L"MeshPoints";
    BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::DimensionType::Register s_RegisterMeshHeaderType(MESH_PTS_NAME, sizeof(DPoint3d));

    const WChar MESH_INDEX_NAME[] = L"MeshIndex";
    BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::DimensionType::Register s_RegisterMeshPointType(MESH_INDEX_NAME, sizeof(int32_t));

    const WChar MESH_METADATA_NAME[] = L"MeshMetadata";
    BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::DimensionType::Register s_RegisterMeshMetadataType(MESH_METADATA_NAME, sizeof(uint8_t));

    const WChar MESH_TEX_NAME[] = L"MeshTex";
    BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::DimensionType::Register s_RegisterMeshTexType(MESH_TEX_NAME, sizeof(uint8_t));

    const WChar MESH_UV_NAME[] = L"MeshUv";
    BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::DimensionType::Register s_RegisterMeshUvType(MESH_UV_NAME, sizeof(DPoint2d));

    // Register point converters:
    static RegisterIDTMPointConverter<DPoint3d, DPoint3d>                        s_ptTypeConv0;

    static RegisterConverter<IDTMPointConverter< IDTMPointDimConverter<DPoint3d, DPoint3d> >,
        PointType3d64f_R16G16B16_I16Creator,
        PointType3d64fCreator>                                  s_ptTypeConvSp0;


    // Register linear converters
    static RegisterIDTMLinearConverter<DPoint3d, DPoint3d>                       s_linTypeConv0;

    // Register linear to point converters
    static RegisterIDTMLinearToPointConverter<DPoint3d, DPoint3d>                s_linToPtTypeConv0;



    // Register mesh converters
    static RegisterMeshAsIDTMLinearConverter<DPoint3d, DPoint3d>                 s_meshTypeConv0;
    // Register mesh to points converters
    static RegisterMeshAsIDTMLinearToPointConverter                                  s_meshToPtTypeConv0;
    // Register mesh to linear converters
    static RegisterMeshAsIDTMLinearToIDTMLinearConverter                             s_meshToLinTypeConv0;


    // Register TIN converters
    static RegisterTINAsIDTMLinearConverter<DPoint3d, DPoint3d>                  s_tinTypeConv0;
    // Register TIN to points converters
    static RegisterTINAsIDTMLinearToPointConverter                                   s_tinToPtTypeConv0;
    // Register TIN to linear converters
    static RegisterTINAsIDTMLinearToIDTMLinearConverter                              s_tinToLinTypeConv0;

    static RegisterMeshConverter<DPoint3d, DPoint3d>                        s_ptMeshConv0;



    // Register Moniker



    t_scalableTerrainModelHost = &host;
    t_scalableTerrainModelHost->Initialize();
    BeFileName tempDir;
    
#ifdef VANCOUVER_API       
    BeFileNameStatus beStatus = BeFileName::BeGetTempPath(tempDir);
    assert(BeFileNameStatus::Success == beStatus);
#else
#ifndef LINUX_SCALABLEMESH_BUILD
    DgnPlatformLib::Host::IKnownLocationsAdmin& locationAdmin(DgnPlatformLib::QueryHost()->GetIKnownLocationsAdmin());
    tempDir = locationAdmin.GetLocalTempDirectoryBaseName();
    assert(!tempDir.IsEmpty());
#endif
#endif

    BeSQLiteLib::Initialize(tempDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois  06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ScalableMeshLib::Terminate(ScalableMeshLib::Host& host)
    {
    assert(t_scalableTerrainModelHost == &host);
    t_scalableTerrainModelHost->Terminate(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                     11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScalableMeshLib::IsInitialized()
    {
    return NULL != t_scalableTerrainModelHost;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                     11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ScalableMeshLib::Host& ScalableMeshLib::GetHost()
    {
    return *t_scalableTerrainModelHost;
    }   

#ifdef VANCOUVER_API

#if defined(__BENTLEYSTM_BUILD__) && !defined(__BENTLEYSTMIMPORT_BUILD__)     

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                 05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RegisterPODImportPluginFP ScalableMeshLib::GetPodRegister()
    {
    return s_PODImportRegisterFP;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                 05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ScalableMeshLib::SetPodRegister(RegisterPODImportPluginFP podRegisterFP)
    {
    s_PODImportRegisterFP = podRegisterFP;
    }

#endif

#endif


END_BENTLEY_SCALABLEMESH_NAMESPACE
