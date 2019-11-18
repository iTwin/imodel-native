/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#define BEGIN_BENTLEY_IMODELHUB_NAMESPACE      BEGIN_BENTLEY_NAMESPACE namespace iModel { namespace Hub {
#define END_BENTLEY_IMODELHUB_NAMESPACE        } } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_IMODELHUB      using namespace BentleyApi::iModel::Hub;
#define LOGGER_NAMESPACE_IMODELHUB             "iModelHub"

#ifdef __iModelHubClient_DLL_BUILD__
#define IMODELHUBCLIENT_EXPORT EXPORT_ATTRIBUTE
#else
#define IMODELHUBCLIENT_EXPORT IMPORT_ATTRIBUTE
#endif