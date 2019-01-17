/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/iModelBridgeLdClient.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "iModelBridgeLdClient.h"
#include <LaunchDarkly/ldapi.h>
#include <mutex>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeLdClient::Init(CharCP authKey)
    {
    m_config = LDConfigNew(authKey);
    if (NULL == m_config)
        return ERROR;

    LDConfigSetStreaming(m_config, false);

    m_user = LDUserNew(NULL);//TODO check whether we need the user key.
    if (NULL == m_user)
        {
        LDFree(m_config);
        m_config = NULL;
        return ERROR;
        }

    unsigned int maxwaitmilliseconds = 60 * 1000;
    m_client = LDClientInit(m_config, m_user, maxwaitmilliseconds);
    if (NULL == m_client)
        {
        LDFree(m_config);
        m_config = NULL;

        LDFree(m_user);
        m_user = NULL;

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
        LDClientClose(m_client);
        LDFree(m_client);
        m_client = NULL;
        }

    if (NULL != m_user)
        {
        LDFree(m_user);
        m_user = NULL;
        }

    if (NULL != m_config)
        {
        LDFree(m_config);
        m_config = NULL;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeLdClient::~iModelBridgeLdClient()
    {
    Close();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeLdClient::IsFeatureOn(bool& flag, CharCP featureName)
    {
    static int timeOut = 60 * 1000;
    if (!LDClientAwaitInitialized(m_client,timeOut))
        return ERROR;

    flag = LDBoolVariation (m_client,featureName, flag);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeLdClient& iModelBridgeLdClient::GetInstance(WebServices::UrlProvider::Environment environment)
    {
    static iModelBridgeLdClient s_instance;
    static std::once_flag s_initOnce;
    std::call_once(s_initOnce, [&]
        {
        CharCP key = GetSDKKey(environment);
        BeAssert(NULL != key);
        s_instance.Init(key);
    });
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
            return "sdk-1bf2601d-6ba9-406c-919b-882e3c5c7694";
        case WebServices::UrlProvider::Qa:
            return "sdk-959d4e68-a773-4c2a-9a03-cef74473b61b";
        case WebServices::UrlProvider::Release:
            return "sdk-b30b05ae-73c9-4303-b43d-38db8fe6f8d8";
        case WebServices::UrlProvider::Perf:
        default:
            return NULL;
        }
    }