/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelDmsSupport/PWWorkspaceHelper.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <iModelBridge/Dms/PWWorkspaceHelper.h>
#include <iModelBridge/Dms/PWSession.h>
#include <ProjectWise_InternalSDK/Include/Workspace/ManagedWorkspace.h>
#include <Bentley/Desktop/FileSystem.h>

USING_NAMESPACE_BENTLEY_DGN

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PWWorkspaceHelper::Initialize()
    {
    return workspace_Initialize() ? true : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void            PWWorkspaceHelper::UnInitialize()
    {
    workspace_UnInitialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PWWorkspaceHelper::FetchWorkspace(PWSession& session, int folderId, int documentId, BeFileNameCR destination)
    {
    PWWorkspaceHelper helper;
    if (!helper.Initialize())
        {
        LOG.errorv("Problems initializing workspace support.");
        }
    int statusCodeBefore = aaApi_GetLastErrorId();
    LOG.tracev("Generating workspace configuration file. %d", statusCodeBefore);
    bool status = true;
    wchar_t workspaceFilePath[1024] = {0};
    if (!workspace_GenerateMSConfigurationFile3(0,
                                                folderId,
                                                documentId,
                                                destination.c_str(),//workspaceDir,
                                                NULL, // additionalCfg
                                                session.GetApplicationResourcePath().c_str(), // path to MSTN
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
        
        status = false;
        }
    LOG.tracev("Finished workspace configuration file: %S", workspaceFilePath);
    helper.UnInitialize();
    return status;
    }