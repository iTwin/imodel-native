/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "iModelBridgeLdClient.h"
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

    LDSetLogFunction(LD_LOG_INFO, [](const char* msg) {
        NativeLogging::LoggingManager::GetLogger(L"iModelBridge")->info(msg);
    });

    m_config = LDConfigNew(authKey);
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
    if (NULL == m_user)//If a user is not set fallback to deviceid
        m_user = LDUserNew(NULL);//TODO check whether we need the user key.

    if (NULL != m_client)
        return SUCCESS;

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
BentleyStatus   iModelBridgeLdClient::SetUserName(CharCP userName)
    {
    if (NULL == m_user)
        m_user = LDUserNew(userName);//TODO check whether we need the user key.
    
    LDUserSetName(m_user, userName);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeLdClient::SetProjectDetails(CharCP iModelName, CharCP guid)
    {
    //rapidjson::Document json(rapidjson::kObjectType);
    //auto& allocator = json.GetAllocator();
    //json.AddMember("iModelName", rapidjson::Value(iModelName,allocator), allocator);
    //json.AddMember("ConnectProjectGuid", rapidjson::Value(guid, allocator), allocator);
    
    LDNode* customAttributes = LDNodeCreateHash();
    LDNodeAddString(&customAttributes, "iModelName", iModelName);
    LDNodeAddString(&customAttributes, "ConnectProjectGuid", guid);
    
    //return LDUserSetCustomAttributesJSON(m_user, BeRapidJsonUtilities::ToString(json).c_str()) ? SUCCESS : ERROR;

    LDUserSetCustomAttributes(m_user, customAttributes);
    return SUCCESS;
    }