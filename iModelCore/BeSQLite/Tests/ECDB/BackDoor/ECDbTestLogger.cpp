/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/ECDB/BackDoor/ECDbTestLogger.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitTests/BackDoor/ECDb/ECDbTests.h>


BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    03/2014
//---------------------------------------------------------------------------------------
//static member initialization
Bentley::NativeLogging::ILogger* ECDbTestLogger::s_logger = nullptr;


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    03/2014
//---------------------------------------------------------------------------------------
//static
Bentley::NativeLogging::ILogger& ECDbTestLogger::Get ()
    {
    if (s_logger == nullptr)
        s_logger = Bentley::NativeLogging::LoggingManager::GetLogger (L"ECDbTests");

    return *s_logger;
    }

END_ECDBUNITTESTS_NAMESPACE
