/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DmsHelper.h"
#include <iModelDmsSupport/DmsSession.h>
#include <Bentley/Desktop/FileSystem.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <rapidjson/document.h>

#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger(L"iModelBridge"))

USING_NAMESPACE_BENTLEY_DGN

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Suvik.Rahane                    11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool   DmsHelper::_Initialize()
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
bool   DmsHelper::_UnInitialize()
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
DmsHelper::DmsHelper(Utf8StringCR callBackurl, Utf8StringCR accessToken, Utf8StringCR repositoryType, Utf8StringCR datasource)
    {
    m_callbackUrl = Utf8String(callBackurl);
    m_accessToken = Utf8String(accessToken);
    m_repositoryType = Utf8String(repositoryType);
    m_datasource = Utf8String(datasource);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Suvik.Rahane                    11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DmsHelper::~DmsHelper()
    {
    _UnInitialize();
    _UnInitializeSession();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Suvik.Rahane                    11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DmsHelper::_InitializeSession(WStringCR repositoryUrl)
    {
    m_repositoryUrl = Utf8String(repositoryUrl);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Suvik.Rahane                    11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DmsHelper::_UnInitializeSession()
    {
    m_repositoryUrl.clear();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String       DmsHelper::GetToken()
    {
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
        return Utf8String();
        }

    token.append(tokenStr);
    return token;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Suvik.Rahane                    11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DmsHelper::_StageInputFile(BeFileNameCR fileLocation)
    {
    _Initialize();

    //Parse URL
    DmsClient dmsClient;
    if (!dmsClient._InitializeSession(m_repositoryUrl, m_repositoryType))
        {
        LOG.errorv("Error while parsing url");
        dmsClient._UnInitializeSession();
        return false;
        }
    //Get OIDC token
    Utf8String token = GetToken();
    if (token.empty())
        return false;
    //Get download URLs
    bvector<DmsResponseData> downloadUrls = dmsClient._GetDownloadURLs(token, m_datasource);

    if (downloadUrls.empty())
        {
        LOG.errorv("Error while getting download urls");
        dmsClient._UnInitializeSession();
        return false;
        }
    dmsClient._UnInitializeSession();

    //Download files
    bvector<AsyncTaskPtr<AzureResult>> stageFileRequests;
    for (bvector<DmsResponseData>::iterator itr = downloadUrls.begin(); itr != downloadUrls.end(); ++itr)
        {
        WString fileName(itr->fileName);
        WString downloadURL(itr->downloadURL);
        WString parentId(itr->parentFolderId);
        WString fileId(itr->fileId);

        WString dirPath(BeFileName::GetDirectoryName(fileLocation));
        BeFileName fullDirPath;
        if (!fileId.empty() && !parentId.empty())
            {
            bvector<WString> paths;
            BeStringUtilities::Split(dirPath.c_str(), L"\\", paths);
            fullDirPath.append(paths[0].c_str());
            for (int itr = 1; itr != paths.size(); itr++)
                {
                if (paths[itr].EqualsI(parentId))
                    break;

                fullDirPath.AppendToPath(paths[itr].c_str());
                }
            fullDirPath.AppendToPath(parentId.c_str());
            if (!fullDirPath.DoesPathExist())
                BeFileName::CreateNewDirectory(fullDirPath);

            // Added folderId into collection
            m_fileFolderIds.Insert(fileId, parentId);
            }

        fullDirPath.AppendToPath(fileName.c_str());

        if (m_azureHelper->_InitializeSession(downloadURL))
            {
            auto result = m_azureHelper->_AsyncStageInputFile(fullDirPath);
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
            LOG.tracev("Successfully download file url to input file location");
            continue;
            }
        LOG.errorv("Error getting file url Error : %s.", result.GetError().GetCode().c_str());
        return false;
        }
    LOG.tracev("Successfully download all file urls to input file location");
    _UnInitialize();
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
WString   DmsHelper::_GetFolderId(WStringCR pwMoniker)
    {
    Utf8String fileId = Utf8String();
    WString folderId = WString();
    if (pwMoniker.empty())
        {
        return m_fileFolderIds.begin()->second;
        }
    bvector<Utf8String> guids;
    if (m_repositoryType.EqualsI(PWREPOSITORYTYPE))
        {
        BeStringUtilities::Split(Utf8String(pwMoniker).c_str(), "{", guids);
        if (guids.size() < 2)
            return  folderId;
        fileId = guids[1].Trim("}");
        }
    else
        {
        BeStringUtilities::Split(Utf8String(pwMoniker).c_str(), "/", guids);
        if (guids.size() < 3)
            return  folderId;
        fileId = guids[2];
        }
    if (m_fileFolderIds.count(WString(fileId.c_str(), 0)) != 0)
        {
        auto nodeItr = m_fileFolderIds.find(WString(fileId.c_str(), 0));
        folderId = nodeItr->second;
        return folderId;
        }
    return folderId;
    }