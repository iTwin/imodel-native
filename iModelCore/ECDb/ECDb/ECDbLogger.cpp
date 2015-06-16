/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbLogger.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECDbLogger.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    03/2014
//---------------------------------------------------------------------------------------
//static member initialization
BentleyApi::NativeLogging::ILogger* ECDbLogger::s_logger = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    03/2014
//---------------------------------------------------------------------------------------
//static
BentleyApi::NativeLogging::ILogger& ECDbLogger::Get ()
    {
    if (s_logger == nullptr)
        s_logger = NativeLogging::LoggingManager::GetLogger (L"ECDb");

    return *s_logger;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
