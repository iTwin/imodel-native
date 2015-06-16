/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/BackDoor/PublicAPI/BackDoor/ECDb/ECDbTests.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ECDb/ECDbApi.h>
#include <Bentley/BeTest.h>
#include <Logging/bentleylogging.h>

#define BEGIN_ECDBUNITTESTS_NAMESPACE BEGIN_BENTLEY_SQLITE_EC_NAMESPACE namespace Tests {
#define END_ECDBUNITTESTS_NAMESPACE } END_BENTLEY_SQLITE_EC_NAMESPACE

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
//! Provides the logger for the ECDb ATP logging category
// @bsiclass                                                Krischan.Eberle      03/2014
//+===============+===============+===============+===============+===============+======
struct ECDbTestLogger
    {
    private:
        static BentleyApi::NativeLogging::ILogger* s_logger;

        ECDbTestLogger ();
        ~ECDbTestLogger ();

    public:
        static BentleyApi::NativeLogging::ILogger& Get ();
    };

#define LOG (ECDbTestLogger::Get())

END_ECDBUNITTESTS_NAMESPACE