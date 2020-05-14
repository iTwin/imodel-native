/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "math.h" //To workaround projectwise API header issues
#include <Bentley/Bentley.h>
#include  "CommonDefinition.h"

//Forward declaration for DgnDocumentManager
namespace Bentley {
    namespace DgnPlatform {
        struct DgnDocumentManager;
    }
}

BEGIN_BENTLEY_DGN_NAMESPACE
struct DmsSession;
struct iMBridgeDocPropertiesAccessor;
struct IDmsSupport
    {
    virtual bool _InitializeSession(WStringCR pwMoniker) = 0;
    virtual bool _InitializeSessionFromDataSource(WStringCR dataSource) = 0;
    virtual bool _UnInitializeSession() = 0;
    virtual bool _Initialize() = 0;
    virtual bool _UnInitialize() = 0;
    virtual bool _StageInputFile(BeFileNameCR fileLocation) = 0;
    virtual StatusInt _FetchWorkspace(BeFileNameR workspaceCfgFile, WStringCR pWMoniker, BeFileNameCR workspaceDir, bool isv8i, bvector<WString> const& additonalFilePatterns) = 0;
    virtual StatusInt _FetchWorkspace(BeFileNameR workspaceCfgFile, int folderId, int documentId, BeFileNameCR workspaceDir, bool isv8i, bvector<WString> const& additonalFilePatterns) = 0;
    virtual void SetApplicationResourcePath(BeFileNameCR applicationResourcePath) = 0;
    virtual Bentley::DgnPlatform::DgnDocumentManager* _GetDgnDocumentManager() = 0;
    virtual iMBridgeDocPropertiesAccessor* _GetDocumentPropertiersAccessor() = 0;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod  
+---------------+---------------+---------------+---------------+---------------+------*/
struct iModelDmsSupport
    {
    enum SessionType
        {
        PWDI = 1,
        PWShare = 2,
        AzureBlobStorage = 3,
        PWDIDMS = 4,
        };

    IMODEL_DMSSUPPORT_EXPORT static IDmsSupport* GetInstance(SessionType sessionType, Utf8StringCR userName, Utf8StringCR password, 
		Utf8StringCR callBackurl, Utf8StringCR accessToken, Utf8StringCR datasource, int maxReties, unsigned long  productId);
    };

END_BENTLEY_DGN_NAMESPACE

extern "C"
    {
    typedef BentleyApi::Dgn::IDmsSupport* T_iModelDmsSupport_getInstance(int sessionType, BentleyApi::Utf8StringCR userName, BentleyApi::Utf8StringCR password,
        BentleyApi::Utf8StringCR callBackurl, BentleyApi::Utf8StringCR accessToken, BentleyApi::Utf8StringCR dataSource, int maxReties, unsigned long productId);
    }


