/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/iModelDmsSupport/iModelDmsSupport.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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

#include <Logging/bentleylogging.h>
#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger(L"iModelBridge"))

BEGIN_BENTLEY_DGN_NAMESPACE
struct DmsSession;

struct IDmsSupport
    {
    virtual bool _Initialize() = 0;
    virtual bool _UnInitialize() = 0;
    virtual StatusInt _FetchWorkspace(Utf8StringCR pWMoniker, BeFileNameCR workspaceDir) = 0;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod  
+---------------+---------------+---------------+---------------+---------------+------*/
struct iModelDmsSupport
    {
    enum SessionType
        {
        PWDI = 0,
        PWShare = 1
        };

    IMODEL_DMSSUPPORT_EXPORT static IDmsSupport* GetInstance(SessionType sessionType, Utf8StringCR userName, Utf8StringCR password, Utf8StringCR dataSource);

    };
END_BENTLEY_DGN_NAMESPACE

extern "C"
    {
    IMODEL_DMSSUPPORT_EXPORT BentleyApi::Dgn::IDmsSupport*   iModelDmsSupport_getInstance(int sessionType, Utf8StringCR userName, Utf8StringCR password, Utf8StringCR dataSource);
    }


