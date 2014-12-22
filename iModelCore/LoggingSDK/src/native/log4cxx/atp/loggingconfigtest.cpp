/*--------------------------------------------------------------------------------------+
|
|     $Source: logging/native/log4cxx/atp/loggingconfigtest.cpp $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <cppunittest/TestHarness.h>
#include <windows.h>
#include <bentley/bentley.h>
#include <log4cxx.h>

USING_NAMESPACE_CPPUNITTEST;
USING_NAMESPACE_BENTLEY_LOGGING;

TEST (LoggingConfig, configurationSUCCESS)
        {
        CHECK ( false == LoggingConfig::IsProviderActive() );

        CHECK ( SUCCESS == LoggingConfig::ActivateProvider(LOG4CXX_LOGGING_PROVIDER) );

        CHECK ( true == LoggingConfig::IsProviderActive() );

        CHECK ( SUCCESS == LoggingConfig::DeactivateProvider() );
        }

