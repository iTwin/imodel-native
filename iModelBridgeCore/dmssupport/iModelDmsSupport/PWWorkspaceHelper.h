/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <iModelDmsSupport/iModelDmsSupport.h>
#include <iModelDmsSupport/DmsSession.h>

BEGIN_BENTLEY_DGN_NAMESPACE

struct DmsSession;
struct PWWorkspaceHelper : public IDmsSupport
    {
    private:
        bool        m_initPwAppDone;
        bool        m_initDone;
        DmsSession  m_session;
        BeFileName  m_inputFile;

        bool        InitPwApi();
        virtual bool _Initialize() override;
        virtual bool _UnInitialize() override;
        StatusInt _FetchWorkspace(BeFileNameR workspaceCfgFile, int folderId, int documentId, BeFileNameCR destination, bool isv8i, bvector<WString> const & additonalFilePatterns);
        virtual bool _InitializeSession(WStringCR pwMoniker) override;
        virtual bool _UnInitializeSession() override;
        virtual bool _InitializeSessionFromDataSource(WStringCR dataSource) override;
        virtual bool _StageInputFile(BeFileNameCR fileLocation) override;
        
        StatusInt  StageDocument(long folderId, long documentId, BeFileNameCR dirPath);
    public:
        PWWorkspaceHelper(DmsSession& session);
        ~PWWorkspaceHelper();
        StatusInt GetFileName(WString fileName, int folderId, int documentId);
        StatusInt GetFolderIdFromMoniker(int& folderId, int& documentId, WStringCR pwMoniker);
        virtual StatusInt _FetchWorkspace(BeFileNameR workspaceCfgFile, WStringCR pWMoniker, BeFileNameCR workspaceDir, bool isv8i, bvector<WString> const & additonalFilePatterns) override;
        virtual  void SetApplicationResourcePath(BeFileNameCR applicationResourcePath) override;
        virtual Bentley::DgnPlatform::DgnDocumentManager* _GetDgnDocumentManager() override;
    };

END_BENTLEY_DGN_NAMESPACE