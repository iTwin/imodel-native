#/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PrivateAPI/ProfilesLogging.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Profiles\ProfilesDefinitions.h>
#include <Logging\bentleylogging.h>

#define PROFILES_LOG (*NativeLogging::LoggingManager::GetLogger (PRF_LOGGER_NAMESPACE))
