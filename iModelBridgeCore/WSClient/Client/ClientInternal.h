/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/ClientInternal.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef __CLIENTINTERNAL_H__
#define __CLIENTINTERNAL_H__

#include <Bentley/BeDebugLog.h>
#include <Bentley/Bentley.h>
#include <DgnClientFx/DgnClientFxCommon.h>
#include <DgnClientFx/Utils/Utils.h>

#include <WebServices/Client/ObjectId.h>
#include <WebServices/Client/WebServicesClient.h>
#include <WebServices/Client/WSClient.h>
#include <WebServices/Client/WSError.h>
#include <WebServices/Client/WSInfo.h>
#include <WebServices/Client/WSQuery.h>
#include <WebServices/Client/WSRepository.h>
#include <WebServices/Client/WSRepositoryClient.h>

#include "ChunkedUploadRequest.h"
#include "ClientConfiguration.h"
#include "ClientConnection.h"
#include "Logging.h"
#include "ServerInfoProvider.h"
#include "Utils.h"

USING_NAMESPACE_BENTLEY_DGNCLIENTFX
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS
USING_NAMESPACE_BENTLEY_WEBSERVICES

#endif // __CLIENTINTERNAL_H__
