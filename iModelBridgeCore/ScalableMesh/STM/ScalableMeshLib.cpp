/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshLib.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM

#include <ScalableMesh\ScalableMeshLib.h>
//#include <TerrainModel/ElementHandler/DTMElementHandlerManager.h>
#include "Plugins\ScalableMeshTypeConversionFilterPlugins.h"
#include "ScalableMeshFileMoniker.h"
#include "ScalableMeshRDSProvider.h"
#include <ScalableMesh\IScalableMeshProgressiveQuery.h>
#include "SMMemoryPool.h"
#include <CloudDataSource/DataSourceManager.h>
#include <ImagePP/all/h/HFCCallbacks.h>
#include <ImagePP/all/h/HFCCallbackRegistry.h>
#include <ImagePP/all/h/ImageppLib.h>





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
void RegisterPODImportPlugin();


//=======================================================================================
// @bsiclass
//=======================================================================================
struct WebServiceKey
    {
private:
    Utf8String m_key;
    DateTime m_expiration;

public:
    WebServiceKey() :m_key(""), m_expiration(DateTime()) { }
    WebServiceKey(Utf8StringCR key) : m_key(key), m_expiration(DateTime()) { }
    WebServiceKey(Utf8StringCR key, DateTimeCR expiration) : m_key(key), m_expiration(expiration) { }
    bool IsValid() const { return !m_key.empty(); }
    Utf8String GetKey() const { return m_key; }
    void SetKey(Utf8StringCR key) { m_key = key; }
    DateTime GetExpiration() const { return m_expiration; }
    void SetExpiration(DateTimeCR expiration) { m_expiration = expiration; }
    bool IsExpired() const { return m_expiration.IsValid() && DateTime::Compare(m_expiration, DateTime::GetCurrentTimeUtc()) != DateTime::CompareResult::LaterThan; }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.St-Pierre  10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
WebServiceKey GetBingKey()
    {
    //CONCEPTSTATION_LOGD("KeyServer: Retreiving bing key from server");
    Utf8String readBuffer;
    
#ifndef REMOVE_WHEN_KEYSERVICE_IN_PRODUCTION
/*
    Utf8String serviceUrl = _getUrl();
    Utf8String bingKeyUrl = serviceUrl.append("/ContextKeyServiceSchema/BingApiKey");
*/
    Utf8String rdsURL(ScalableMeshRDSProvider::GetBuddiUrl());

    if (true/*Service::IsProductionServer(serviceUrl)*/) //Production server do not know this API yet, return the local key if Connected for the moment.
        {
        bool isConnected = true;//DgnClientApp::AbstractUiState().GetValue("BentleyConnect_SignedIn", false);
        return isConnected ? WebServiceKey(BING_AUTHENTICATION_KEY) : WebServiceKey();
        }
#endif // !REMOVE_WHEN_KEYSERVICE_IN_PRODUCTION

#if 0 //TODO
    CURLcode result = performCurl(bingKeyUrl, &readBuffer);

    if (CURLE_OK != result)
        {
        if (result == CURLE_RECV_ERROR)
            {
            error() = CONCEPTSTATIONL10N_GETSTRING(STATUS_ERR_InvalidProxyCredentials);
            }
        else
            {
            error() = Utf8PrintfString(CONCEPTSTATIONL10N_GETSTRING(STATUS_ERR_ContextServerError_d).c_str(), 7);
            }
        CONCEPTSTATION_LOGW(error().c_str());
        return WebServiceKey();
        }

    Json::Value packageInfos(Json::objectValue);
    Json::Reader::Parse(readBuffer, packageInfos);

    if (!packageInfos.isMember("instances"))
        {
        if (packageInfos.isMember("errorMessage"))
            {
            error() = packageInfos["errorMessage"].asCString();
            }
        else
            error() = Utf8PrintfString(CONCEPTSTATIONL10N_GETSTRING(STATUS_ERR_ContextServerError_d).c_str(), 8);
        CONCEPTSTATION_LOGW(error().c_str());
        return WebServiceKey();
        }
    error() = "";
    if (packageInfos["instances"].isArray() &&
        packageInfos["instances"].isValidIndex(0) &&
        packageInfos["instances"][0].isMember("properties") &&
        packageInfos["instances"][0]["properties"].isMember("key") &&
        packageInfos["instances"][0]["properties"].isMember("expirationDate"))
        {
        DateTime expiration;
        DateTime::FromString(expiration, packageInfos["instances"][0]["properties"]["expirationDate"].asCString());
        return WebServiceKey(packageInfos["instances"][0]["properties"]["key"].asString(), expiration);

        }
    error() = Utf8PrintfString(CONCEPTSTATIONL10N_GETSTRING(STATUS_ERR_ContextServerError_d).c_str(), 9);
    CONCEPTSTATION_LOGW(error().c_str());
#endif

    return WebServiceKey();
    }

//=======================================================================================
// @bsiclass                                                    Raphael.Lemieux 09/2017
//=======================================================================================
class BingAuthenticationCallback : public ImagePP::HFCAuthenticationCallback, public RefCountedBase
    {
private:
    mutable WebServiceKey m_bingKey;
public:
    BingAuthenticationCallback() : m_bingKey(WebServiceKey())     {    };
    virtual             ~BingAuthenticationCallback()     {    };


    bool       GetAuthentication(ImagePP::HFCAuthentication* pio_Authentication) const override;
    unsigned short RetryCount(ImagePP::HCLASS_ID pi_AuthenticationType) const override     { return 1;     }
    bool       IsCancelled() const override     { return !m_bingKey.IsValid();     }
    bool       CanAuthenticate(ImagePP::HCLASS_ID pi_AuthenticationType) const override     { return true;     }
    };

typedef RefCountedPtr<BingAuthenticationCallback> BingAuthenticationCallbackPtr;


//=======================================================================================
// @bsiclass                                                    Raphael.Lemieux 09/2017
//=======================================================================================
bool BingAuthenticationCallback::GetAuthentication(ImagePP::HFCAuthentication* pio_Authentication) const
    {
    ImagePP::HFCInternetAuthentication* pAuth = dynamic_cast<ImagePP::HFCInternetAuthentication*>(pio_Authentication);

    if (pAuth != nullptr)
        {

        if (nullptr == pAuth || !pAuth->GetServer().ContainsI(L"bing"))
            return false;

        WString key = L"";
        if (!m_bingKey.IsValid() || m_bingKey.IsExpired())
            {
/*
            Json::Value error;
            KeyServicePtr service = KeyService::Create(error);
   
            if (service.IsValid())
                {
                m_bingKey = service->GetKey(Bing);
                }
*/
            m_bingKey = GetBingKey();
            }

        if (m_bingKey.IsValid() && !m_bingKey.IsExpired())
            key.AssignUtf8(m_bingKey.GetKey().c_str());

        pAuth->SetPassword(key);
        return !key.empty();
        }

    ImagePP::HFCProxyAuthentication* pProxyAuth = dynamic_cast<ImagePP::HFCProxyAuthentication*>(pio_Authentication);

    if (pProxyAuth != nullptr)
        {
        //std::shared_ptr<ProxyHttpHandler> pProxyConfig(AppSettings::GetSharedPointer()->GetProxyConfig());

        ScalableMeshAdmin::ProxyInfo proxyInfo(ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetProxyInfo());
        
        if (!proxyInfo.m_serverUrl.empty())
            {
            pProxyAuth->SetUser(WString(proxyInfo.m_user.c_str(), true));
            pProxyAuth->SetPassword(WString(proxyInfo.m_password.c_str(), true));
            pProxyAuth->SetServer(WString(proxyInfo.m_serverUrl.c_str(), true));
            return true;
            }

        return false;
        }

    assert(!"Unknown/unsupported HFCAuthentication type");

    return false;
    }


static BingAuthenticationCallbackPtr s_bingAuthCallback;


void ScalableMeshLib::Host::Initialize()
    {
    BeAssert (NULL == m_scalableTerrainModelAdmin);   
    SMMemoryPool::GetInstance();
    m_scalableTerrainModelAdmin = &_SupplyScalableMeshAdmin();  
#ifdef VANCOUVER_API
	m_stmAdmin = &_SupplySTMAdmin();
#endif
    m_smPaths = new bmap<WString, IScalableMesh*>();
    InitializeProgressiveQueries();
    RegisterPODImportPlugin();
    BeFileName geocoordinateDataPath(L".\\GeoCoordinateData\\");
    GeoCoordinates::BaseGCS::Initialize(geocoordinateDataPath.c_str());
    //BENTLEY_NAMESPACE_NAME::TerrainModel::Element::DTMElementHandlerManager::InitializeDgnPlatform();

    s_bingAuthCallback = new BingAuthenticationCallback();

    HFCCallbackRegistry::GetInstance()->AddCallback(s_bingAuthCallback.get());    
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
    for(bvector<ObjEntry>::iterator itr=m_hostObj.begin(); itr!=m_hostObj.end(); ++itr)
        {
        IHostObject* pValue(itr->GetValue());
        TERMINATE_HOST_OBJECT(pValue, onProgramExit);
        }

    m_hostObj.clear();
    m_hostVar.clear();
                                
    TERMINATE_HOST_OBJECT(m_scalableTerrainModelAdmin, onProgramExit);    
    delete m_smPaths;
    t_scalableTerrainModelHost = NULL;
    TerminateProgressiveQueries();

    //DataSourceManager::Shutdown();

    }



IScalableMeshPtr ScalableMeshLib::Host::GetRegisteredScalableMesh(const WString& path)
    {
    if (m_smPaths->count(path) > 0) return (*m_smPaths)[path];
    return nullptr;
    }

void             ScalableMeshLib::Host::RemoveRegisteredScalableMesh(const WString& path)
    {
    m_smPaths->erase(path);
    }

void ScalableMeshLib::Host::RegisterScalableMesh(const WString& path, IScalableMeshPtr& ref)
    {
    m_smPaths->insert(make_bpair(path, ref.get()));
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
    BeAssert (NULL == t_scalableTerrainModelHost);  // cannot be called twice on the same thread
    if (NULL != t_scalableTerrainModelHost)
        return;    

    // Register point converters:
    static const RegisterIDTMPointConverter<DPoint3d, DPoint3d>                        s_ptTypeConv0;

    static const RegisterConverter<IDTMPointConverter< IDTMPointDimConverter<DPoint3d, DPoint3d> >,
        PointType3d64f_R16G16B16_I16Creator,
        PointType3d64fCreator>                                  s_ptTypeConvSp0;


    // Register linear converters
    static const RegisterIDTMLinearConverter<DPoint3d, DPoint3d>                       s_linTypeConv0;

    // Register linear to point converters
    static const RegisterIDTMLinearToPointConverter<DPoint3d, DPoint3d>                s_linToPtTypeConv0;



    // Register mesh converters
    static const RegisterMeshAsIDTMLinearConverter<DPoint3d, DPoint3d>                 s_meshTypeConv0;
    // Register mesh to points converters
    static const RegisterMeshAsIDTMLinearToPointConverter                                  s_meshToPtTypeConv0;
    // Register mesh to linear converters
    static const RegisterMeshAsIDTMLinearToIDTMLinearConverter                             s_meshToLinTypeConv0;


    // Register TIN converters
    static const RegisterTINAsIDTMLinearConverter<DPoint3d, DPoint3d>                  s_tinTypeConv0;
    // Register TIN to points converters
    static const RegisterTINAsIDTMLinearToPointConverter                                   s_tinToPtTypeConv0;
    // Register TIN to linear converters
    static const RegisterTINAsIDTMLinearToIDTMLinearConverter                              s_tinToLinTypeConv0;

    static const RegisterMeshConverter<DPoint3d, DPoint3d>                        s_ptMeshConv0;

    
    // Register Moniker

    

    t_scalableTerrainModelHost = &host;
    t_scalableTerrainModelHost->Initialize();
    BeFileName tempDir;
    BeFileNameStatus beStatus = BeFileName::BeGetTempPath(tempDir);
    assert(BeFileNameStatus::Success == beStatus);
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
bool ScalableMeshLib::IsInitialized ()
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



END_BENTLEY_SCALABLEMESH_NAMESPACE
