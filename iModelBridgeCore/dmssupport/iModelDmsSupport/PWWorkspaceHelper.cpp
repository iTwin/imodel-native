/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PWWorkspaceHelper.h"
#include <iModelDmsSupport/DmsSession.h>
#include <ProjectWise_InternalSDK/Include/Workspace/ManagedWorkspace.h>
#include <Bentley/Desktop/FileSystem.h>

#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger(L"iModelBridge"))

USING_NAMESPACE_BENTLEY_DGN

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool   PWWorkspaceHelper::_Initialize()
    {
    if (m_initDone)
        return true;
    
    if (!InitPwApi())
        return false;

    BOOL status =  workspace_Initialize();
    if (status)
        m_initDone = true;
    else
        LOG.errorv("Problems initializing workspace support.");
    return m_initDone ? true : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool   PWWorkspaceHelper::_UnInitialize()
    {
    if (m_initDone)
        workspace_UnInitialize();
    m_initDone = false;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PWWorkspaceHelper::StageDocument(long folderId, long documentId, BeFileNameCR dirPath)
    {
    wchar_t filePath[1024] = { 0 };
    if (!aaApi_GiveOutDocument(folderId, documentId, dirPath.c_str(), filePath, 1024))
        {
        int statusCodeAfter = aaApi_GetLastErrorId();
        LOG.errorv("Unable to fetch workspace for file. Status Code: %d", statusCodeAfter);
        LOG.errorv("%ls", aaApi_GetLastErrorMessage());
        LOG.errorv("%ls", aaApi_GetLastErrorDetail());
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PWWorkspaceHelper::_FetchWorkspace(BeFileNameR workspaceCfgFile, int folderId, int documentId, BeFileNameCR destination, bool isv8i, bvector<WString> const& additonalFilePatterns)
    {
    int statusCodeBefore = aaApi_GetLastErrorId();
    LOG.tracev("Generating workspace configuration file. %d", statusCodeBefore);
    m_activeWorkspaceDir = destination;
    StatusInt status = SUCCESS;
    wchar_t workspaceFilePath[1024] = {0};
    if (!workspace_GenerateMSConfigurationFile3(0,
                                                (long)folderId,
                                                (long)documentId,
                                                destination.c_str(),//workspaceDir,
                                                NULL, // additionalCfg
                                                m_session.GetApplicationResourcePath(isv8i).c_str(), // path to MSTN
                                                NULL,//m_session.GetDefaultConfigPath(isv8i).c_str(), // defaultCfgFile
                                                NULL, //commandLineArgs,
                                                NULL, // fnCallback
                                                NULL, // callbackData
                                                workspaceFilePath,
                                                1024))
        {
        int statusCodeAfter = aaApi_GetLastErrorId();
        LOG.errorv("Unable to fetch workspace for file. Status Code: %d", statusCodeAfter);
        LOG.errorv("%ls", aaApi_GetLastErrorMessage());
        LOG.errorv("%ls", aaApi_GetLastErrorDetail());
        status = statusCodeAfter;
        }

    LOG.tracev("Finished workspace configuration file: %ls", workspaceFilePath);
    workspaceCfgFile = BeFileName(workspaceFilePath);

    if (!additonalFilePatterns.empty())
        {
        BeFileName dirPath = BeFileName(m_inputFile).GetDirectoryName();

        for (auto pattern: additonalFilePatterns)
            {
            int result = aaApi_SelectDocumentsByNameProp((long)folderId, pattern.c_str(), NULL, NULL, NULL);
            if (result > 0)
                {
                for (int index = 0; index < result; ++index)
                    {
                    long docId = aaApi_GetDocumentId(index);
                    if (docId != 0)
                        StageDocument(folderId, docId, dirPath);
                    }
                }
            }
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
PWWorkspaceHelper::PWWorkspaceHelper(DmsSession& session)
    :m_initDone(false),m_session(session), m_initPwAppDone(false)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
PWWorkspaceHelper::~PWWorkspaceHelper()
    {
    _UnInitialize();
    _UnInitializeSession();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PWWorkspaceHelper::_FetchWorkspace(BeFileNameR workspaceCfgFile, WStringCR pwMoniker, BeFileNameCR workspaceDir, bool isv8i, bvector<WString> const& additonalFilePatterns)
    {
    _Initialize();

    int folderId, documentId;
    if (SUCCESS != GetFolderIdFromMoniker(folderId, documentId, pwMoniker))
        return ERROR;

    return _FetchWorkspace(workspaceCfgFile, folderId, documentId, workspaceDir, isv8i, additonalFilePatterns);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PWWorkspaceHelper::GetFolderIdFromMoniker(int& folderId, int& documentId, WStringCR pwMoniker)
    {
    HMONIKER moniker = NULL;
    DWORD monikerFlags = AAMONIKERF_USE_EXISTING_LOGIN| AAMONIKERF_RESOURCE_LOCATION;
    LPCWSTR monikerArray = &pwMoniker[0];
    if (!aaApi_StringsToMonikers(1, &moniker, &monikerArray, monikerFlags))
        {
        LOG.errorv("aaApi_StringsToMonikers failed for document %ls", pwMoniker.c_str());
        return ERROR;
        }

    LPCGUID guid = aaApi_GetDocumentGuidFromMoniker(moniker);
    if (NULL == guid)
        {
        LOG.errorv("aaApi_GetDocumentGuidFromMoniker failed for document %ls", pwMoniker.c_str());
        aaApi_Free(moniker);
        return ERROR;
        }

    AADOC_ITEM docItem = { 0 };
    if (!aaApi_GetDocumentIdsByGUIDs(1, guid, &docItem))
        {
        LOG.errorv("aaApi_GetDocumentIdsByGUIDs failed for document %ls", pwMoniker.c_str());
        aaApi_Free((void*)guid);
        aaApi_Free(moniker);
        return ERROR;
        }
    folderId = docItem.lProjectId;
    documentId = docItem.lDocumentId;
    aaApi_Free(moniker);
    //aaApi_Free((void*)guid);
    //aaApi_GetDatasourceNameFromMoniker
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PWWorkspaceHelper::InitPwApi()
    {
    if (m_initPwAppDone)
        return true;

    m_initPwAppDone = true;
    return m_session.InitPwLibraries(BeFileName(L"C:\\Program Files\\Bentley\\ProjectWise\\bin"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PWWorkspaceHelper::_InitializeSession(WStringCR pwMoniker)
    {
    m_inputFileMoniker = pwMoniker;

    if (!InitPwApi())
        return false;
    
    HMONIKER moniker = NULL;
    LPCWSTR monikerArray = &pwMoniker[0];
    if (!aaApi_StringsToMonikers(1, &moniker, &monikerArray, AAMONIKERF_DONT_VALIDATE))
        {
        LOG.errorv(L"Cannot call  aaApi_StringsToMonikers for document %ls", pwMoniker.c_str());
        return false;
        }
    
    LPCWSTR datasourceName = aaApi_GetDatasourceNameFromMoniker(moniker);
    if (NULL == datasourceName)
        {
        LOG.errorv(L"Cannot call  aaApi_GetDatasourceNameFromMoniker for document %ls", pwMoniker.c_str());
        aaApi_Free(moniker);
        return false;
        }

    _InitializeSessionFromDataSource(datasourceName);

    aaApi_Free(moniker);
    aaApi_Free((void*)datasourceName);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PWWorkspaceHelper::_UnInitializeSession()
    {
    m_session.UnInitialize();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void            PWWorkspaceHelper::SetApplicationResourcePath(BeFileNameCR applicationResourcePath)
    {
    m_session.SetApplicationResourcePath(applicationResourcePath);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PWWorkspaceHelper::_InitializeSessionFromDataSource(WStringCR dataSource)
    {
    if (!InitPwApi())
        return false;

    m_session.SetDataSource(Utf8String(dataSource));

    if (!m_session.Initialize())
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PWWorkspaceHelper::_StageInputFile(BeFileNameCR fileLocation)
    {
    m_inputFile = fileLocation;
    if (fileLocation.DoesPathExist())
        return true;

    int folderId, documentId;
    if (SUCCESS != GetFolderIdFromMoniker(folderId, documentId, m_inputFileMoniker))
        return false;

    BeFileName dirLocation = fileLocation.GetDirectoryName();
    if (SUCCESS == StageDocument(folderId, documentId, dirLocation))
        return true;

    return false;
    }