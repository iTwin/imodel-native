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
        bool        m_initDone;
        DmsSession  m_session;
        virtual bool _Initialize() override;
        virtual bool _UnInitialize() override;

    public:
        PWWorkspaceHelper(DmsSession& session);
        ~PWWorkspaceHelper();
        StatusInt FetchWorkspace(int folderId, int documentId, BeFileNameCR destination);
        StatusInt GetFolderIdFromMoniker(int& folderId, int& documentId, Utf8StringCR pwMoniker);
        virtual StatusInt _FetchWorkspace(Utf8StringCR pWMoniker, BeFileNameCR workspaceDir) override;
    };

END_BENTLEY_DGN_NAMESPACE