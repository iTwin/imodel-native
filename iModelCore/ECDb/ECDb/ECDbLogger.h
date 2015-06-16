/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbLogger.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <ECDb/ECDbTypes.h>
#include <Logging/bentleylogging.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Provides the logger for the ECDb logging category
// Note: The static was needed especially for iOS use cases - just getting the logger repeatedly caused a drain
// in performance since the logger was looked up with a naked wchar_t pointer. 
// @bsiclass                                                Krischan.Eberle      03/2014
//+===============+===============+===============+===============+===============+======
struct ECDbLogger
    {
private:
    static BentleyApi::NativeLogging::ILogger* s_logger;

    ECDbLogger ();
    ~ECDbLogger ();

public:
    static BentleyApi::NativeLogging::ILogger& Get ();
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
