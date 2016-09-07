/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/DgnDbServerCommon.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <WebServices/WebServices.h>
#include <WebServices/Client/WSError.h>
#include <Bentley/Tasks/AsyncResult.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <DgnPlatform/DgnPlatformApi.h>

#define BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE    BEGIN_BENTLEY_NAMESPACE namespace DgnDbServer {
#define END_BENTLEY_DGNDBSERVER_NAMESPACE      } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_DGNDBSERVER    using namespace BentleyApi::DgnDbServer;
#define LOGGER_NAMESPACE_DGNDBSERVER           "DgnDbServer"

#ifdef __DgnDbServerClient_DLL_BUILD__
#define DGNDBSERVERCLIENT_EXPORT EXPORT_ATTRIBUTE
#else
#define DGNDBSERVERCLIENT_EXPORT IMPORT_ATTRIBUTE
#endif
