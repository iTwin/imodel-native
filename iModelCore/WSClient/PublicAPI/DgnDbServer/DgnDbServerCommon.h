/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/DgnDbServerCommon.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <WebServices/WebServices.h>
#include <WebServices/Client/WSError.h>
#include <MobileDgn/Utils/Threading/AsyncResult.h>
#include <MobileDgn/Utils/Threading/AsyncTask.h>

#define BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE    BEGIN_BENTLEY_NAMESPACE namespace DgnDbServer {
#define END_BENTLEY_DGNDBSERVER_NAMESPACE      } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_DGNDBSERVER    using namespace BentleyApi::DgnDbServer;

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
struct DgnDbServerError : public MobileDgn::Utils::AsyncError
    {
public:
    DgnDbServerError() {}
    DgnDbServerError(WebServices::WSErrorCR error) {}
    DgnDbServerError(BeSQLite::DbResult error) {}
    DgnDbServerError(BentleyStatus status) {}
    };
template<typename AnyValue> using DgnDbServerResult = MobileDgn::Utils::AsyncResult<AnyValue, DgnDbServerError>;
template<typename AnyValue> using DgnDbServerResultPtr = std::shared_ptr<MobileDgn::Utils::AsyncResult<AnyValue, DgnDbServerError>>;
template<typename AnyValue> using DgnDbServerTask = MobileDgn::Utils::PackagedAsyncTask<MobileDgn::Utils::AsyncResult<AnyValue, DgnDbServerError>>;
template<typename AnyValue> using DgnDbServerTaskPtr = std::shared_ptr<MobileDgn::Utils::PackagedAsyncTask<MobileDgn::Utils::AsyncResult<AnyValue, DgnDbServerError>>>;
END_BENTLEY_DGNDBSERVER_NAMESPACE


#ifdef __DgnDbServerClient_DLL_BUILD__
#define DGNDBSERVERCLIENT_EXPORT EXPORT_ATTRIBUTE
#else
#define DGNDBSERVERCLIENT_EXPORT IMPORT_ATTRIBUTE
#endif
