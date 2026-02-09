/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDbTypes.h>
#include <BeSQLite/BeSQLite.h>
#include <Bentley/Logging.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Provides the logger for the ECDb logging category
// Note: The static was needed especially for iOS use cases - just getting the logger repeatedly caused a drain
// in performance since the logger was looked up with a naked wchar_t pointer.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECDbLogger final
    {
private:
    static NativeLogging::CategoryLogger* s_logger;

    ECDbLogger ();
    ~ECDbLogger ();

public:
    static NativeLogging::CategoryLogger& Get();

    static void LogSqliteError(BeSQLite::Db const&, BeSQLite::DbResult sqliteStat, Utf8CP errorMessageHeader = nullptr);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
