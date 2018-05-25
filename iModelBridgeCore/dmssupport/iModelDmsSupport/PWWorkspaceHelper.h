/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelDmsSupport/PWWorkspaceHelper.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <iModelDmsSupport/iModelDmsSupport.h>


BEGIN_BENTLEY_DGN_NAMESPACE

struct DmsSession;
struct PWWorkspaceHelper
    {
    private:
        bool Initialize();
        void UnInitialize();

    public:
        IMODEL_DMSSUPPORT_EXPORT static bool FetchWorkspace(DmsSession& session, int folderId, int documentId, BeFileNameCR destination);
    };

END_BENTLEY_DGN_NAMESPACE