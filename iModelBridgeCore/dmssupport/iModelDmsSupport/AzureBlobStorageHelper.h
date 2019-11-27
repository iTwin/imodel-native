/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <iModelDmsSupport/iModelDmsSupport.h>
#include <BeHttp/HttpRequest.h>
#include <WebServices/Azure/AzureBlobStorageClient.h>

BEGIN_BENTLEY_DGN_NAMESPACE
USING_NAMESPACE_BENTLEY_WEBSERVICES

struct DmsSession;
struct AzureBlobStorageHelper : public IDmsSupport
    {
    private:
        WebServices::AzureBlobStorageClientPtr m_client;
        Utf8String m_sasUrl;

        virtual bool _InitializeSessionFromDataSource(WStringCR dataSource) override
            {
            return true;
            }

    public:
        AzureBlobStorageHelper();
        ~AzureBlobStorageHelper();
        virtual bool _Initialize() override;
        virtual bool _UnInitialize() override;
        virtual bool _InitializeSession(WStringCR pwMoniker) override;
        virtual bool _UnInitializeSession() override;
        virtual bool _StageInputFile(BeFileNameCR fileLocation) override;
        AsyncTaskPtr<AzureResult> _AsyncStageInputFile(BeFileNameCR fileLocation);
        virtual StatusInt _FetchWorkspace(BeFileNameR workspaceCfgFile, int folderId, int documentId, BeFileNameCR destination, bool isv8i, bvector<WString> const& additonalFilePatterns) override
            {
            return SUCCESS;
            }
        virtual StatusInt _FetchWorkspace(BeFileNameR workspaceCfgFile, WStringCR pWMoniker, BeFileNameCR workspaceDir, bool isv8i, bvector<WString> const& additonalFilePatterns) override
            {
            return SUCCESS;
            }
        virtual  void SetApplicationResourcePath(BeFileNameCR applicationResourcePath) override
            {}
        virtual Bentley::DgnPlatform::DgnDocumentManager* _GetDgnDocumentManager() override
            {
            return NULL;
            }
    };

END_BENTLEY_DGN_NAMESPACE