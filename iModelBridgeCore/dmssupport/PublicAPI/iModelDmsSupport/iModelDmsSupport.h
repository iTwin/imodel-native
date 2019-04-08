/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/iModelDmsSupport/iModelDmsSupport.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "math.h" //To workaround projectwise API header issues
#include <Bentley/Bentley.h>


#ifdef __IMODEL_DMSSUPPORT_BUILD__
#define IMODEL_DMSSUPPORT_EXPORT EXPORT_ATTRIBUTE
#else
#define IMODEL_DMSSUPPORT_EXPORT IMPORT_ATTRIBUTE
#endif

#ifndef BEGIN_BENTLEY_DGN_NAMESPACE
#define BEGIN_BENTLEY_DGN_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace Dgn {
#define END_BENTLEY_DGN_NAMESPACE   } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_DGN  using namespace BentleyApi::Dgn;
#endif

//Forward declaration for DgnDocumentManager
namespace Bentley {
    namespace DgnPlatform {
        struct DgnDocumentManager;
    }
}
#include <Logging/bentleylogging.h>


BEGIN_BENTLEY_DGN_NAMESPACE
struct DmsSession;

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
        };

    IMODEL_DMSSUPPORT_EXPORT static IDmsSupport* GetInstance(SessionType sessionType, Utf8StringCR userName, Utf8StringCR password);
    };

END_BENTLEY_DGN_NAMESPACE

extern "C"
    {
    typedef BentleyApi::Dgn::IDmsSupport* T_iModelDmsSupport_getInstance(int sessionType, BentleyApi::Utf8StringCR userName, BentleyApi::Utf8StringCR password);
    }


