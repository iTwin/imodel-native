/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/DgnCoreAPI.h>
#include <DgnPlatform/GenericDomain.h>

#include <LinearReferencing/LinearReferencingApi.h>
#include <RoadRailPhysical/RoadRailPhysicalApi.h>

#include <ECPresentation/RulesDriven/RuleSetEmbedder.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

#include "C3dImporter.h"
#include "C3dHelper.h"

#define LOG (*NativeLogging::LoggingManager::GetLogger(L"C3dImporter"))

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG
USING_NAMESPACE_BENTLEY_LINEARREFERENCING
USING_NAMESPACE_BENTLEY_ROADRAILALIGNMENT

