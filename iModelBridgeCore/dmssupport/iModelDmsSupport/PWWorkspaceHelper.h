/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <iModelDmsSupport/iModelDmsSupport.h>
#include <iModelDmsSupport/DmsSession.h>
#include <iModelBridge/iModelBridge.h>
BEGIN_BENTLEY_DGN_NAMESPACE

struct DmsSession;


struct PWWorkspaceHelper : public IDmsSupport, iMBridgeDocPropertiesAccessor
    {
    private:
        bool        m_initPwAppDone;
        bool        m_initDone;
        DmsSession* m_session;
        BeFileName  m_inputFile;
        BeFileName  m_activeWorkspaceDir;
        WString     m_inputFileMoniker;
        Utf8String  m_documentGuid;
        bool        m_skipAssignmentCheck;
        bool        InitPwApi();
        static Utf8String GetDocumentGuid(WStringCR inputMoniker);
        virtual bool _Initialize() override;
        virtual bool _UnInitialize() override;
        StatusInt _FetchWorkspace(BeFileNameR workspaceCfgFile, int folderId, int documentId, BeFileNameCR destination, bool isv8i, bvector<WString> const & additonalFilePatterns);
        virtual bool _InitializeSession(WStringCR pwMoniker) override;
        virtual bool _UnInitializeSession() override;
        virtual bool _InitializeSessionFromDataSource(WStringCR dataSource) override;
        virtual bool _StageInputFile(BeFileNameCR fileLocation) override;
        
        StatusInt  StageDocument(long folderId, long documentId, BeFileNameCR dirPath);

        //! Check if the specified file is assigned to the specified bridge.
        //! @param fn   The name of the file that is to be converted
        //! @param bridgeRegSubKey The registry subkey that identifies the bridge
        //! @return true if the specified bridge should convert the specified file
        virtual bool _IsFileAssignedToBridge(BeFileNameCR fn, wchar_t const* bridgeRegSubKey) { return m_skipAssignmentCheck; }

        //! Get the URN and other properties for a document from the host document control system (e.g., ProjectWise)
        //! @param[in] fn   The name of the file that is to be converted
        //! @param[out] props Properties that may be assigned to a document by its home document control system (DCS)
        //! @return non-zero error status if doc properties could not be found for this file.
        virtual BentleyStatus _GetDocumentProperties(iModelBridgeDocumentProperties& props, BeFileNameCR fn) ;

        //! Look up a document's properties by its document GUID.
        //! @param[out] props Properties that may be assigned to a document by its home document control system (DCS)
        //! @param[out] localFilePath The local filepath of the staged document
        //! @param[in] docGuid  The document's GUID in its home DCS
        //! @return non-zero error status if the GUID is not in the table of registered documents.
        virtual BentleyStatus _GetDocumentPropertiesByGuid(iModelBridgeDocumentProperties& props, BeFileNameR localFilePath, BeSQLite::BeGuid const& docGuid) ;

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

    public:
        PWWorkspaceHelper(DmsSession& session, bool skipAssignmentCheck = false);
        ~PWWorkspaceHelper();
        StatusInt GetFileName(WString fileName, int folderId, int documentId);
        StatusInt GetFolderIdFromMoniker(int& folderId, int& documentId, WStringCR pwMoniker);
        virtual StatusInt _FetchWorkspace(BeFileNameR workspaceCfgFile, WStringCR pWMoniker, BeFileNameCR workspaceDir, bool isv8i, bvector<WString> const & additonalFilePatterns) override;
        virtual  void SetApplicationResourcePath(BeFileNameCR applicationResourcePath) override;
        virtual Bentley::DgnPlatform::DgnDocumentManager* _GetDgnDocumentManager() override;
        virtual iMBridgeDocPropertiesAccessor* _GetDocumentPropertiersAccessor() override;
        BeFileName const& GetActiveWorkspaceDir() 
            {
            return m_activeWorkspaceDir;
            }
    };

END_BENTLEY_DGN_NAMESPACE