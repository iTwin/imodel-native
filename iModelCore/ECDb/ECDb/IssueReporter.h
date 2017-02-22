/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/IssueReporter.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ECDb/ECDb.h>
#include <Logging/bentleylogging.h>
#include <Bentley/BeThread.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      09/2015
//+===============+===============+===============+===============+===============+======
struct IssueReporter : NonCopyableClass
    {
private:
    static const NativeLogging::SEVERITY s_logSeverity = NativeLogging::LOG_ERROR;
    mutable BeMutex m_mutex;
    ECDb::IIssueListener const* m_issueListener = nullptr;

public:
    IssueReporter() {}
    ~IssueReporter() {}

    BentleyStatus AddListener(ECDb::IIssueListener const&);
    void RemoveListener();

    void Report(Utf8CP message, ...) const;

    bool IsEnabled(bool* isLogEnabled = nullptr) const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE