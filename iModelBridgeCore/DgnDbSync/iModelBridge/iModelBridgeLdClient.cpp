/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <iModelBridge/iModelBridgeLdClient.h>
#include <LaunchDarkly/ldapi.h>
#include <mutex>
#include <rapidjson/document.h>
#include <Logging/bentleylogging.h>

#undef LOG
#define LOG (*NativeLogging::LoggingManager::GetLogger(L"iModelBridge"))

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeLdClient::Init(CharCP authKey)
    {
    if (NULL == authKey)
        return ERROR;
    
    m_key = authKey;
    LDSetLogFunction(LD_LOG_TRACE, [](const char* msg) {
        NativeLogging::LoggingManager::GetLogger(L"iModelBridge")->info(msg);
    });

    m_config = LDConfigNew(m_key.c_str());//authKey will be freed in the restart case.
    if (NULL == m_config)
        return ERROR;

    LDConfigSetStreaming(m_config, false);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeLdClient::InitClient()
    {
    if (NULL != m_client)
        return SUCCESS;

    if (NULL == m_user)
        return ERROR;

    InitUserCustomAttributes();
    m_client = LDClientInit(m_config, m_user, 0);
    if (NULL == m_client)
        {
        LOG.error("Failed to initialize client in iModelBridgeLdClient::InitClient");
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeLdClient::iModelBridgeLdClient()
    :m_config(NULL), m_user(NULL), m_client(NULL)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeLdClient::Close()
    {
    if (NULL != m_client)
        {
        LDClientFlush(m_client);
        LDClientClose(m_client);
        m_client = NULL;
        }

    m_user = NULL;
    m_config = NULL;
    LDSetLogFunction(LD_LOG_INFO, NULL);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeLdClient::~iModelBridgeLdClient()
    {
    //Close();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeLdClient::IsFeatureOn(bool& flag, CharCP featureName)
    {
    if (NULL == m_client)
        {
        if (SUCCESS != InitClient())
            return ERROR;
        }

    //static int timeOut = 60 * 1000;
    bool isInitialized = LDClientIsInitialized(m_client);
    for (int index = 0; index < 60 && !isInitialized; ++index)
        {
        BeDuration::FromMilliseconds(1000).Sleep();
        isInitialized = LDClientIsInitialized(m_client);
        }

    if (!isInitialized)
        {
        LOG.error("iModelBridgeLdClient::IsFeatureOn: LDClient is not initialized");
        return ERROR;
        }

    flag = LDBoolVariation (m_client,featureName, flag);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Majerle                  07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeLdClient::GetFeatureValue(Utf8StringR value, CharCP featureName)
    {
    if (NULL == m_client)
        {
        if (SUCCESS != InitClient())
            return ERROR;
        }

    //static int timeOut = 60 * 1000;
    bool isInitialized = LDClientIsInitialized(m_client);
    for (int index = 0; index < 60 && !isInitialized; ++index)
        {
        BeDuration::FromMilliseconds(1000).Sleep();
        isInitialized = LDClientIsInitialized(m_client);
        }

    if (!isInitialized)
        return ERROR;

    char *const s = LDStringVariationAlloc(m_client, featureName, "");    

    value = Utf8String(s);
    free(s);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeLdClient& iModelBridgeLdClient::GetInstance(WebServices::UrlProvider::Environment environment)
    {
    static iModelBridgeLdClient s_instance;
    CharCP key = GetSDKKey(environment);
    if (NULL == s_instance.m_user)
        s_instance.Init(key);
    
    return s_instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP          iModelBridgeLdClient::GetSDKKey(WebServices::UrlProvider::Environment environment)
    {
    switch (environment)
        {
        case WebServices::UrlProvider::Dev:
            return "mob-5b88b541-4a7a-47a9-89b0-898aca1177f2";
        case WebServices::UrlProvider::Qa:
            return "mob-6829cc02-a35a-4c16-ab59-1483ccfff249";
        case WebServices::UrlProvider::Release:
            return "mob-58a19902-b563-4299-8aee-83289f871e49";
            
        case WebServices::UrlProvider::Perf:
        default:
            return "mob-6829cc02-a35a-4c16-ab59-1483ccfff249";
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeLdClient::SetUserName(CharCP userNameIn)
    {
    Utf8String userName(userNameIn);
    m_userName = userName.ToLower();
    if (NULL == m_user)
        m_user = LDUserNew(m_userName.c_str());//TODO check whether we need the user key.
    else
        {
        LDFree(m_user);
        m_user = LDUserNew(m_userName.c_str());
        }
    
    LDUserSetName(m_user, userName.c_str());
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeLdClient::SetProjectDetails(CharCP iModelNameIn, CharCP guidIn)
    {
    //rapidjson::Document json(rapidjson::kObjectType);
    //auto& allocator = json.GetAllocator();
    //json.AddMember("iModelName", rapidjson::Value(iModelName,allocator), allocator);
    //json.AddMember("ConnectProjectGuid", rapidjson::Value(guid, allocator), allocator);
    if (NULL == m_user)
        return ERROR;

    Utf8String iModelName(iModelNameIn);
    iModelName.ToLower();
    Utf8String guid(guidIn);
    guid.ToLower();
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void            iModelBridgeLdClient::InitUserCustomAttributes()
    {
    LDNode* customAttributes = LDNodeCreateHash();
    if (!m_imodelName.empty())
        LDNodeAddString(&customAttributes, "iModelName", m_imodelName.c_str());
    if (!m_connectGuid.empty())
        LDNodeAddString(&customAttributes, "ConnectProjectGuid", m_connectGuid.c_str());
    if (!m_applicationProductId.empty())
        LDNodeAddString(&customAttributes, "ApplicationProductId", m_applicationProductId.c_str());
    if (!m_applicationName.empty())
        LDNodeAddString(&customAttributes, "ApplicationName", m_applicationName.c_str());
    if (!m_applicationVersion.empty())
        LDNodeAddString(&customAttributes, "ApplicationVersion", m_applicationVersion.c_str());
    LDUserSetCustomAttributes(m_user, customAttributes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeLdClient::SetBridgeDetails(Utf8StringCR applicationProductIdIn, Utf8StringCR applicationNameIn, Utf8StringCR applicationVersionIn)
    {
    if (NULL == m_user)
        return ERROR;

    Utf8String applicationProductId(applicationProductIdIn);
    m_applicationProductId = applicationProductId.ToLower();

    Utf8String applicationName(applicationNameIn);
    m_applicationName = applicationName.ToLower();

    Utf8String applicationVersion(applicationVersionIn);
    m_applicationVersion = applicationVersion.ToLower();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeLdClient::RestartClient()
    {
    if (NULL != m_client)
        {
        LDClientFlush(m_client);
        LDClientClose(m_client);//Closing the client releases config and user.
        m_user = NULL;
        m_config = NULL;
        m_client = NULL;
        }
    
    Init(m_key.c_str());
    //Let's Create a new user
    SetUserName(m_userName.c_str());
    return InitClient();
    }