/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/PublicAPI/NonPublished/BeSQLite/BeSQLiteTests.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <Bentley/BeTimeUtilities.h>
#include <Logging/bentleylogging.h>

#include <BeSQLite/BeSQLite.h>

USING_NAMESPACE_BENTLEY_SQLITE 
USING_NAMESPACE_BENTLEY_LOGGING

#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger (L"BeSQLiteTest"))
