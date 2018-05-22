/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/iModelBridge/Dms/PWWorkspaceHelper.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "iModelDmsSupport.h"


BEGIN_BENTLEY_DGN_NAMESPACE

struct PWSession;
struct PWWorkspaceHelper
    {
    private:
        bool Initialize();
        void UnInitialize();

    public:
        IMODEL_DMSSUPPORT_EXPORT static bool FetchWorkspace(PWSession& session, int folderId, int documentId, BeFileNameCR destination);
    };

END_BENTLEY_DGN_NAMESPACE