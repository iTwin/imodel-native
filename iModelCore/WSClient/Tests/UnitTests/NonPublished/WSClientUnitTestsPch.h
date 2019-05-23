/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#ifndef __WSCLIENTUNITTESTSPCH_H__
#define __WSCLIENTUNITTESTSPCH_H__

#include "Utils/WebServicesUnitTests.h"
#include "Utils/WSClientBaseTest.h"
#include "Utils/WebServicesTestsHelper.h"

#include "WebServices/Cache/CachingTestsHelper.h"

#include <Bentley/Base64Utilities.h>
#include <Bentley/BeDebugLog.h>
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/BeFile.h>
#include <Bentley/BeFileName.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeThread.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/BeVersion.h>
#include <Bentley/CancellationToken.h>
#include <Bentley/DateTime.h>
#include <Bentley/Tasks/AsyncError.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/Tasks/WorkerThread.h>
#include <Bentley/WString.h>

#include <BeHttp/Credentials.h>
#include <BeHttp/HttpClient.h>
#include <BeHttp/HttpHeaders.h>
#include <BeHttp/HttpRequest.h>
#include <BeHttp/HttpResponse.h>
#include <BeHttp/HttpStatus.h>
#include <BeHttp/HttpStatusHelper.h>
#include <BeHttp/IHttpHandler.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <BeRapidJson/BeRapidJson.h>
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/L10N.h>
#include <BeSecurity/SecureStore.h>
#include <ECDb/ECDb.h>
#include <ECDb/ECDbApi.h>
#include <ECObjects/ECObjects.h>
#include <Logging/bentleylogging.h>

#include <rapidjson/prettywriter.h>

#include <algorithm>
#include <atomic>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <ostream>
#include <regex>
#include <sstream>
#include <string>

#endif

