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

USING_NAMESPACE_BENTLEY_DGN

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool   PWWorkspaceHelper::_Initialize()
    {
    if (m_initDone)
        return true;

    //TODO: Lookup the projectwise binray or ship it.
    if (SUCCESS != m_session.Initialize(BeFileName(L"C:\\Program Files\\Bentley\\ProjectWise\\bin")))
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
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PWWorkspaceHelper::FetchWorkspace( int folderId, int documentId, BeFileNameCR destination)
    {
    _Initialize();
    
    int statusCodeBefore = aaApi_GetLastErrorId();
    LOG.tracev("Generating workspace configuration file. %d", statusCodeBefore);
    StatusInt status = SUCCESS;
    wchar_t workspaceFilePath[1024] = {0};
    if (!workspace_GenerateMSConfigurationFile3(0,
                                                folderId,
                                                documentId,
                                                destination.c_str(),//workspaceDir,
                                                NULL, // additionalCfg
                                                m_session.GetApplicationResourcePath().c_str(), // path to MSTN
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
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
PWWorkspaceHelper::PWWorkspaceHelper(DmsSession& session)
    :m_initDone(false),m_session(session)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
PWWorkspaceHelper::~PWWorkspaceHelper()
    {
    _UnInitialize();
    m_session.UnInitialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PWWorkspaceHelper::_FetchWorkspace(Utf8StringCR pwMoniker, BeFileNameCR workspaceDir)
    {
    int folderId, documentId;
    if (SUCCESS != GetFolderIdFromMoniker(folderId, documentId, pwMoniker))
        return ERROR;

    return FetchWorkspace(folderId, documentId, workspaceDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PWWorkspaceHelper::GetFolderIdFromMoniker(int& folderId, int& documentId, Utf8StringCR pwMoniker)
    {
    return SUCCESS;
    }