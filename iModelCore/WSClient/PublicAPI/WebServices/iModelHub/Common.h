/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Common.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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


#define BEGIN_BENTLEY_IMODELHUB_NAMESPACE      BEGIN_BENTLEY_NAMESPACE namespace iModel { namespace Hub {
#define END_BENTLEY_IMODELHUB_NAMESPACE        } } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_IMODELHUB      using namespace BentleyApi::iModel::Hub;
#define LOGGER_NAMESPACE_IMODELHUB             "iModelHub"

#ifdef __iModelHubClient_DLL_BUILD__
#define IMODELHUBCLIENT_EXPORT EXPORT_ATTRIBUTE
#else
#define IMODELHUBCLIENT_EXPORT IMPORT_ATTRIBUTE
#endif
