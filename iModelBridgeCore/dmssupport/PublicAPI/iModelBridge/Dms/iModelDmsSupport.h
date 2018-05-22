/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/iModelBridge/Dms/iModelDmsSupport.h $
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


