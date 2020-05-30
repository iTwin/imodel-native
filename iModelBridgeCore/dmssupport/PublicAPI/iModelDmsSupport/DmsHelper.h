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
#include <iModelBridge/iModelBridge.h>
USING_NAMESPACE_BENTLEY_IMODELHUB
BEGIN_BENTLEY_DGN_NAMESPACE

struct DmsHelper : public IDmsSupport, iMBridgeDocPropertiesAccessor
    {
    private:
        Utf8String m_repositoryUrl;
        Utf8String m_callbackUrl;
        Utf8String m_accessToken;
        Utf8String m_repositoryType;
        Utf8String m_datasource;
        bool m_skipAssignmentCheck;
        Utf8String m_projectShareUrl;
        Utf8String m_inputFileMoniker;
        Utf8String m_inputFileParentId;
        BeFileName m_inputFileLocation;
        WString m_cfgfilePath;
        const int m_guidLength = 36;
        bmap<WString, WString> m_fileIdToFolderIdMap;
        bmap<WString, WString> m_fileNameToFileIdMap;
        AzureBlobStorageHelper* m_azureHelper = nullptr;
        IConnectTokenProvider* m_tokenProvider = nullptr;
        int m_maxRetries;
        DmsClient* m_dmsClient = nullptr;
        Utf8String GetToken();
        bool CreateCFGFile(BeFileNameCR fileLocation, DmsResponseData fileData);
        WString GetDocumentGuid(WStringCR fileName);
        bool ValidateMoniker(Utf8StringCR pwMoniker);
        IMODEL_DMSSUPPORT_EXPORT virtual bool _Initialize() override;
        IMODEL_DMSSUPPORT_EXPORT virtual bool _UnInitialize() override;
        IMODEL_DMSSUPPORT_EXPORT virtual bool _UnInitializeSession() override;

        IMODEL_DMSSUPPORT_EXPORT virtual bool _InitializeSessionFromDataSource(WStringCR dataSource) override
            {
            return true;
            }

    public:
        IMODEL_DMSSUPPORT_EXPORT DmsHelper(Utf8StringCR callBackurl, Utf8StringCR accessToken, int maxRetries, Utf8StringCR repositoryType = Utf8String(), bool skipAssignmentCheck = false, Utf8StringCR datasource = Utf8String());
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
        IMODEL_DMSSUPPORT_EXPORT WString _GetFolderIdByFileId(WStringCR pwMoniker = WString());
        IMODEL_DMSSUPPORT_EXPORT WString _GetFileIdByFileName(WStringCR pwMoniker);

        void _SetDependecy(IConnectTokenProvider* tokenProvider, DmsClient* dmsClient, AzureBlobStorageHelper* azureHelper)
            {
            m_tokenProvider = tokenProvider;
            m_dmsClient = dmsClient;
            m_azureHelper = azureHelper;
            }
        Utf8String GetRepositoryType();

        IMODEL_DMSSUPPORT_EXPORT virtual iMBridgeDocPropertiesAccessor* _GetDocumentPropertiersAccessor() override;


        //! Check if the specified file is assigned to the specified bridge.
        //! @param fn   The name of the file that is to be converted
        //! @param bridgeRegSubKey The registry subkey that identifies the bridge
        //! @return true if the specified bridge should convert the specified file
        virtual bool _IsFileAssignedToBridge(BeFileNameCR fn, wchar_t const* bridgeRegSubKey) { return m_skipAssignmentCheck; }

        //! Get the URN and other properties for a document from the host document control system (e.g., ProjectWise)
        //! @param[in] fn   The name of the file that is to be converted
        //! @param[out] props Properties that may be assigned to a document by its home document control system (DCS)
        //! @return non-zero error status if doc properties could not be found for this file.
        virtual BentleyStatus _GetDocumentProperties(iModelBridgeDocumentProperties& props, BeFileNameCR fn);

        //! Look up a document's properties by its document GUID.
        //! @param[out] props Properties that may be assigned to a document by its home document control system (DCS)
        //! @param[out] localFilePath The local filepath of the staged document
        //! @param[in] docGuid  The document's GUID in its home DCS
        //! @return non-zero error status if the GUID is not in the table of registered documents.
        virtual BentleyStatus _GetDocumentPropertiesByGuid(iModelBridgeDocumentProperties& props, BeFileNameR localFilePath, BeSQLite::BeGuid const& docGuid);

        //! Assign a new file to the Bridge Registry database. This is for tracking files which might exist outside the Document Management system but still is critical
        //! in succesfully converting data inside a file.
        //! @param fn   The name of the file that is to be converted
        //! @param bridgeRegSubKey The registry subkey that identifies the bridge
        //! @param guid The the default docguid should docprops not exist for fn
        //! @return non-zero error status if assignment of this file to the registry database failed.
        virtual BentleyStatus _AssignFileToBridge(BeFileNameCR fn, wchar_t const* bridgeRegSubKey, BeSQLite::BeGuidCP guid) { return ERROR; }

        //! Query all files assigned to this bridge.
        //! @param fns   The names of all files that are assignd to this bridge.
        //! @param bridgeRegSubKey The registry subkey that identifies the bridge
        virtual void _QueryAllFilesAssignedToBridge(bvector<BeFileName>& fns, wchar_t const* bridgeRegSubKey) {}

    };

END_BENTLEY_DGN_NAMESPACE