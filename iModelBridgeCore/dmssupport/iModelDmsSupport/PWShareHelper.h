/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <iModelDmsSupport/iModelDmsSupport.h>
#include <BeHttp/HttpRequest.h>
#include <BeHttp/HttpClient.h>
#include <WebServices/iModelHub/Client/OidcTokenProvider.h>
#include "AzureBlobStorageHelper.h"

USING_NAMESPACE_BENTLEY_IMODELHUB
BEGIN_BENTLEY_DGN_NAMESPACE

struct PWShareHelper : public IDmsSupport
    {
    private:
        Utf8String m_repositoryUrl;
        Utf8String m_callbackUrl;
        Utf8String m_accessToken;
        AzureBlobStorageHelper* m_azureHelper = nullptr;
        IConnectTokenProvider* m_tokenProvider = nullptr;
        virtual bool _Initialize() override;
        virtual bool _UnInitialize() override;
        virtual bool _InitializeSession(WStringCR pwMoniker) override;
        virtual bool _UnInitializeSession() override;

        virtual bool _InitializeSessionFromDataSource(WStringCR dataSource) override
            {
            return true;
            }


    public:
        PWShareHelper(Utf8StringCR callBackurl, Utf8StringCR accessToken);
        ~PWShareHelper();
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
        virtual bool _StageInputFile(BeFileNameCR fileLocation) override;
    };

END_BENTLEY_DGN_NAMESPACE