/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/ClientInternal.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>
#include <Bentley/BeDebugLog.h>
#include <MobileDgn/MobileDgnCommon.h>
#include <MobileDgn/Utils/Utils.h>

#include <WebServices/Client/WebServicesClient.h>
#include <WebServices/Client/ChunkedUploadRequest.h>
#include <WebServices/Client/ObjectId.h>
#include <WebServices/Client/WSClient.h>
#include <WebServices/Client/WSError.h>
#include <WebServices/Client/WSInfo.h>
#include <WebServices/Client/WSQuery.h>
#include <WebServices/Client/WSRepository.h>
#include <WebServices/Client/WSRepositoryClient.h>

#include "ClientConfiguration.h"
#include "ClientConnection.h"
#include "ServerInfoProvider.h"
#include "Logging.h"
#include "Utils.h"

USING_NAMESPACE_BENTLEY_MOBILEDGN
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS
USING_NAMESPACE_BENTLEY_WEBSERVICES
