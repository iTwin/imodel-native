/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelDmsSupport/PWWorkspaceHelper.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PWWorkspaceHelper::_FetchWorkspace(BeFileNameR workspaceCfgFile, int folderId, int documentId, BeFileNameCR destination, bool isv8i)
    {
    int statusCodeBefore = aaApi_GetLastErrorId();
    LOG.tracev("Generating workspace configuration file. %d", statusCodeBefore);
    StatusInt status = SUCCESS;
    wchar_t workspaceFilePath[1024] = {0};
    if (!workspace_GenerateMSConfigurationFile3(0,
                                                folderId,
                                                documentId,
                                                destination.c_str(),//workspaceDir,
                                                NULL, // additionalCfg
                                                m_session.GetApplicationResourcePath(isv8i).c_str(), // path to MSTN
                                                NULL, // defaultCfgFile
                                                NULL, //commandLineArgs,
                                                NULL, // fnCallback
                                                NULL, // callbackData
                                                workspaceFilePath,
                                                1024))
        {
        int statusCodeAfter = aaApi_GetLastErrorId();
        LOG.errorv("Unable to fetch workspace for file. Status Code: %d", statusCodeAfter);
        LOG.errorv(aaApi_GetLastErrorMessage());
        LOG.errorv(aaApi_GetLastErrorDetail());
        status = statusCodeAfter;
        }

    LOG.tracev("Finished workspace configuration file: %S", workspaceFilePath);
    workspaceCfgFile = BeFileName(workspaceFilePath);
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
StatusInt   PWWorkspaceHelper::_FetchWorkspace(BeFileNameR workspaceCfgFile, WStringCR pwMoniker, BeFileNameCR workspaceDir, bool isv8i)
    {
    _Initialize();

    int folderId, documentId;
    if (SUCCESS != GetFolderIdFromMoniker(folderId, documentId, pwMoniker))
        return ERROR;

    return _FetchWorkspace(workspaceCfgFile, folderId, documentId, workspaceDir, isv8i);
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
        return ERROR;

    LPCGUID guid = aaApi_GetDocumentGuidFromMoniker(moniker);
    if (NULL == guid)
        {
        aaApi_Free(moniker);
        return ERROR;
        }

    AADOC_ITEM docItem = { 0 };
    if (!aaApi_GetDocumentIdsByGUIDs(1, guid, &docItem))
        {
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
    if (!InitPwApi())
        return false;
    
    HMONIKER moniker = NULL;
    LPCWSTR monikerArray = &pwMoniker[0];
    if (!aaApi_StringsToMonikers(1, &moniker, &monikerArray, AAMONIKERF_DONT_VALIDATE))
        return false;
    
    LPCWSTR datasourceName = aaApi_GetDatasourceNameFromMoniker(moniker);
    if (NULL == datasourceName)
        {
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
