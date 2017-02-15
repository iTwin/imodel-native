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
    mutable BeMutex m_mutex;
    // unused - ECDbCR m_ecdb;
    ECDb::IIssueListener const* m_issueListener;

public:
    explicit IssueReporter(ECDbCR ecdb) : /* unused - m_ecdb(ecdb), */m_issueListener(nullptr) {}
    ~IssueReporter() {}

    BentleyStatus AddListener(ECDb::IIssueListener const&);
    void RemoveListener();

    void Report(Utf8CP message, ...) const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE