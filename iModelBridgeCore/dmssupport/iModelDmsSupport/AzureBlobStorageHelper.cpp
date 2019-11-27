/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "AzureBlobStorageHelper.h"
#include <iModelDmsSupport/DmsSession.h>
#include <Bentley/Desktop/FileSystem.h>

#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger(L"iModelBridge"))

USING_NAMESPACE_BENTLEY_DGN

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool   AzureBlobStorageHelper::_Initialize()
    {
    if (nullptr != m_client)
        return true;
    
    m_client = WebServices::AzureBlobStorageClient::Create();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool   AzureBlobStorageHelper::_UnInitialize()
    {
    m_client = nullptr;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AzureBlobStorageHelper::AzureBlobStorageHelper()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AzureBlobStorageHelper::~AzureBlobStorageHelper()
    {
    _UnInitialize();
    _UnInitializeSession();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool            AzureBlobStorageHelper::_InitializeSession(WStringCR pwMoniker)
    {
    m_sasUrl = Utf8String (pwMoniker);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool            AzureBlobStorageHelper::_UnInitializeSession()
    {
    m_sasUrl.clear();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool            AzureBlobStorageHelper::_StageInputFile(BeFileNameCR fileLocation)
    {
    auto result = _AsyncStageInputFile(fileLocation);
    if (result == nullptr)
        {
        LOG.errorv("Error sas url empty");
        _UnInitialize();
        return false;
        }

    auto response = result->GetResult();
    if (response.IsSuccess())
        {
        LOG.tracev("Successfully download sas url to input file location");
        _UnInitialize();
        return true;
        }

    LOG.errorv("Error getting sas url Error : %s.", response.GetError().GetCode().c_str());
    _UnInitialize();
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<AzureResult>            AzureBlobStorageHelper::_AsyncStageInputFile(BeFileNameCR fileLocation)
    {
    _Initialize();
    if (m_sasUrl.empty())
        return nullptr;

    return m_client->SendGetFileRequest(m_sasUrl, fileLocation);
    }