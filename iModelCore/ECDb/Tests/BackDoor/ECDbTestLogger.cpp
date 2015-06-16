/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/BackDoor/ECDbTestLogger.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitTests/BackDoor/ECDb/ECDbTests.h>


BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    03/2014
//---------------------------------------------------------------------------------------
//static member initialization
BentleyApi::NativeLogging::ILogger* ECDbTestLogger::s_logger = nullptr;


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    03/2014
//---------------------------------------------------------------------------------------
//static
BentleyApi::NativeLogging::ILogger& ECDbTestLogger::Get ()
    {
    if (s_logger == nullptr)
        s_logger = BentleyApi::NativeLogging::LoggingManager::GetLogger (L"ECDbTests");

    return *s_logger;
    }

END_ECDBUNITTESTS_NAMESPACE
