/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelDmsSupport/PWWorkspaceHelper.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
        bool        InitPwApi();
        virtual bool _Initialize() override;
        virtual bool _UnInitialize() override;
        virtual bool _InitializeSession(WStringCR pwMoniker) override;
        virtual bool _UnInitializeSession() override;
        virtual bool _InitializeSessionFromDataSource(WStringCR dataSource) override;
    public:
        PWWorkspaceHelper(DmsSession& session);
        ~PWWorkspaceHelper();
        virtual StatusInt _FetchWorkspace(BeFileNameR workspaceCfgFile, int folderId, int documentId, BeFileNameCR destination, bool isv8i) override;
        StatusInt GetFolderIdFromMoniker(int& folderId, int& documentId, WStringCR pwMoniker);
        virtual StatusInt _FetchWorkspace(BeFileNameR workspaceCfgFile, WStringCR pWMoniker, BeFileNameCR workspaceDir, bool isv8i) override;
        virtual  void SetApplicationResourcePath(BeFileNameCR applicationResourcePath) override;
        virtual Bentley::DgnPlatform::DgnDocumentManager* _GetDgnDocumentManager() override;
    };

END_BENTLEY_DGN_NAMESPACE