/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/IssueReporter.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
    mutable BeMutex m_mutex;
    ECDbCR m_ecdb;
    ECDb::IIssueListener const* m_issueListener;

public:
    explicit IssueReporter(ECDbCR ecdb) : m_ecdb(ecdb), m_issueListener(nullptr) {}
    ~IssueReporter() {}

    BentleyStatus AddListener(ECDb::IIssueListener const&);
    void RemoveListener();

    bool IsSeverityEnabled(ECDbIssueSeverity) const;

    void Report(ECDbIssueSeverity, Utf8CP message, ...) const;
    void ReportSqliteIssue(ECDbIssueSeverity, DbResult, Utf8CP messageHeader = nullptr) const;

    static NativeLogging::SEVERITY ToLogSeverity(ECDbIssueSeverity sev) { return sev == ECDbIssueSeverity::Warning ? NativeLogging::LOG_WARNING : NativeLogging::LOG_ERROR; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE