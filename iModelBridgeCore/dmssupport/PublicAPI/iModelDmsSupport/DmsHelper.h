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
#include "DmsClient.h"
USING_NAMESPACE_BENTLEY_IMODELHUB
BEGIN_BENTLEY_DGN_NAMESPACE

struct DmsHelper : public IDmsSupport
    {
    private:
        Utf8String m_repositoryUrl;
        Utf8String m_callbackUrl;
        Utf8String m_accessToken;
        Utf8String m_repositoryType;
        Utf8String m_datasource;
        WString m_cfgfilePath;
        bmap<WString, WString> m_fileFolderIds;
        AzureBlobStorageHelper* m_azureHelper = nullptr;
        IConnectTokenProvider* m_tokenProvider = nullptr;
        int m_maxRetries;
        DmsClient* m_dmsClient = nullptr;
        Utf8String GetToken();
        bool CreateCFGFile(BeFileNameCR fileLocation, DmsResponseData fileData);
        IMODEL_DMSSUPPORT_EXPORT virtual bool _Initialize() override;
        IMODEL_DMSSUPPORT_EXPORT virtual bool _UnInitialize() override;
        IMODEL_DMSSUPPORT_EXPORT virtual bool _UnInitializeSession() override;

        IMODEL_DMSSUPPORT_EXPORT virtual bool _InitializeSessionFromDataSource(WStringCR dataSource) override
            {
            return true;
            }

    public:
        IMODEL_DMSSUPPORT_EXPORT DmsHelper(Utf8StringCR callBackurl, Utf8StringCR accessToken, int maxRetries, Utf8StringCR repositoryType = Utf8String(), Utf8StringCR datasource = Utf8String());
        IMODEL_DMSSUPPORT_EXPORT ~DmsHelper();
        IMODEL_DMSSUPPORT_EXPORT virtual bool _InitializeSession(WStringCR pwMoniker) override;
        virtual StatusInt _FetchWorkspace(BeFileNameR workspaceCfgFile, int folderId, int documentId, BeFileNameCR destination, bool isv8i, bvector<WString> const& additonalFilePatterns) override
            {
            return SUCCESS;
            }
        IMODEL_DMSSUPPORT_EXPORT virtual StatusInt _FetchWorkspace(BeFileNameR workspaceCfgFile, WStringCR pWMoniker, BeFileNameCR workspaceDir, bool isv8i, bvector<WString> const& additonalFilePatterns) override;
        virtual  void SetApplicationResourcePath(BeFileNameCR applicationResourcePath) override
            {}
        IMODEL_DMSSUPPORT_EXPORT virtual Bentley::DgnPlatform::DgnDocumentManager* _GetDgnDocumentManager() override;
        IMODEL_DMSSUPPORT_EXPORT virtual bool _StageInputFile(BeFileNameCR fileLocation) override;
        IMODEL_DMSSUPPORT_EXPORT virtual bool _StageDocuments(BeFileNameR fileLocation, bool downloadWS = false, bool downloadRef = false);
        IMODEL_DMSSUPPORT_EXPORT WString _GetFolderId(WStringCR pwMoniker = WString());

        void _SetDependecy(IConnectTokenProvider* tokenProvider, DmsClient* dmsClient, AzureBlobStorageHelper* azureHelper)
            {
            m_tokenProvider = tokenProvider;
            m_dmsClient = dmsClient;
            m_azureHelper = azureHelper;
            }
        Utf8String GetRepositoryType();
    };

END_BENTLEY_DGN_NAMESPACE