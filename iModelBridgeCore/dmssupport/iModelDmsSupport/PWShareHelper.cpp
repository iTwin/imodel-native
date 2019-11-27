/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PWShareHelper.h"
#include "PWShareDmsSupport.h"
#include <iModelDmsSupport/DmsSession.h>
#include <Bentley/Desktop/FileSystem.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <rapidjson/document.h>

#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger(L"iModelBridge"))

USING_NAMESPACE_BENTLEY_DGN

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Suvik.Rahane                    11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool   PWShareHelper::_Initialize()
    {
    if (m_azureHelper != nullptr && m_tokenProvider != nullptr)
        return true;

    m_azureHelper = new AzureBlobStorageHelper();
    m_azureHelper->_Initialize();

    if (!m_callbackUrl.empty())
        m_tokenProvider = new OidcTokenProvider(m_callbackUrl);
    else
        m_tokenProvider = new OidcStaticTokenProvider(m_accessToken);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Suvik.Rahane                    11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool   PWShareHelper::_UnInitialize()
    {
    if (m_azureHelper != nullptr)
        {
        m_azureHelper->_UnInitialize();
        delete m_azureHelper;
        m_azureHelper = nullptr;
        }
    if (m_tokenProvider != nullptr)
        {
        delete m_tokenProvider;
        m_tokenProvider = nullptr;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Suvik.Rahane                    11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
PWShareHelper::PWShareHelper(Utf8StringCR callBackurl, Utf8StringCR accessToken)
    {
    m_callbackUrl = Utf8String(callBackurl);
    m_accessToken = Utf8String(accessToken);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Suvik.Rahane                    11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
PWShareHelper::~PWShareHelper()
    {
    _UnInitialize();
    _UnInitializeSession();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Suvik.Rahane                    11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PWShareHelper::_InitializeSession(WStringCR repositoryUrl)
    {
    m_repositoryUrl = Utf8String(repositoryUrl);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Suvik.Rahane                    11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PWShareHelper::_UnInitializeSession()
    {
    m_repositoryUrl.clear();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Suvik.Rahane                    11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PWShareHelper::_StageInputFile(BeFileNameCR fileLocation)
    {
    _Initialize();

    //Parse URL
    PWShareDmsSupport pwShareDmsSupport;
    if (!pwShareDmsSupport._InitializeSession(m_repositoryUrl))
        {
        LOG.errorv("Error while parsing url");
        pwShareDmsSupport._UnInitializeSession();
        return false;
        }

    //Get OIDC token
    Utf8String token = Utf8String("Bearer ");
    Utf8String tokenStr = Utf8String();
    WebServices::ISecurityTokenPtr tokenPtr = nullptr;

    tokenPtr = m_tokenProvider->GetToken();
    if (tokenPtr == nullptr)
        tokenPtr = m_tokenProvider->UpdateToken()->GetResult();

    if (tokenPtr != nullptr)
        {
        tokenStr = tokenPtr->ToAuthorizationString();
        }
    else
        {
        LOG.errorv("Error while getting authorization token");
        return false;
        }

    if (!m_callbackUrl.empty())
        token.append(tokenStr);
    else
        token = tokenStr;

    //Get download URLs
    bmap<WString, WString> downloadUrls = pwShareDmsSupport._GetDownloadURLs(token);
    if (downloadUrls.empty())
        {
        LOG.errorv("Error while getting download urls");
        pwShareDmsSupport._UnInitializeSession();
        return false;
        }
    pwShareDmsSupport._UnInitializeSession();

    //Download files
    bvector<AsyncTaskPtr<AzureResult>> stageFileRequests;
    for (bmap<WString, WString>::iterator itr = downloadUrls.begin(); itr != downloadUrls.end(); ++itr)
        {
        WString fileName(itr->first);
        WString downloadURL(itr->second);

        WString dirPath(BeFileName::GetDirectoryName(fileLocation));
        BeFileName::AppendToPath(dirPath, fileName.c_str());

        if (m_azureHelper->_InitializeSession(downloadURL))
            {
            auto result = m_azureHelper->_AsyncStageInputFile(BeFileName(dirPath));
            if (result == nullptr)
                {
                LOG.errorv("Error sas url empty");
                m_azureHelper->_UnInitializeSession();
                return false;
                }
            stageFileRequests.push_back(result);
            }
        m_azureHelper->_UnInitializeSession();
        }

    for (int itr = 0; itr != stageFileRequests.size(); itr++)
        {
        auto result = stageFileRequests[itr]->GetResult();
        if (result.IsSuccess())
            {
            LOG.tracev("Successfully download sas url to input file location");
            continue;
            }
        LOG.errorv("Error getting sas url Error : %s.", result.GetError().GetCode().c_str());
        return false;
        }

    LOG.tracev("Successfully download all sas url to input file location");
    _UnInitialize();
    return true;
    }