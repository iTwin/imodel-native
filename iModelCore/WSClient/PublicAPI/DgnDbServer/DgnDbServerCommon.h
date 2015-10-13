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
struct DgnDbServerError;
template<typename AnyValue> using DgnDbServerResult = MobileDgn::Utils::AsyncResult<AnyValue, WebServices::WSError>;
using DgnDbServerTaskPtr = MobileDgn::Utils::AsyncTaskPtr<DgnDbServerResult<void>>;
END_BENTLEY_DGNDBSERVER_NAMESPACE
