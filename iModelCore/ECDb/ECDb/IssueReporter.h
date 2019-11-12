/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECDb/ECDb.h>
#include <Logging/bentleylogging.h>
#include <Bentley/BeThread.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      01/2018
//+===============+===============+===============+===============+===============+======
struct IIssueReporter
    {
    protected:
        IIssueReporter() {}
    public:
        virtual void Report(Utf8CP message) const = 0;
        virtual void ReportV(Utf8CP message, ...) const = 0;
        virtual ~IIssueReporter() {}
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      09/2015
//+===============+===============+===============+===============+===============+======
struct IssueReporter final : IIssueReporter
    {
private:
    static const NativeLogging::SEVERITY s_logSeverity = NativeLogging::LOG_ERROR;
    mutable BeMutex m_mutex;
    ECDb::IIssueListener const* m_issueListener = nullptr;

    //not copyable
    IssueReporter(IssueReporter const&) = delete;
    IssueReporter& operator=(IssueReporter const&) = delete;

    void DoReport(Utf8CP message, bool isLogSeverityEnabled) const;

public:
    IssueReporter() : IIssueReporter() {}
    ~IssueReporter() {}

    BentleyStatus AddListener(ECDb::IIssueListener const&);
    void RemoveListener();

    void Report(Utf8CP message) const override;
    void ReportV(Utf8CP message, ...) const override;

    bool IsEnabled(bool* isLogEnabled = nullptr) const;
    };


//=======================================================================================
//! Helper class that can be used to use an IssueReporter that can be turned on/off for a given
//! scope
// @bsiclass                                                Krischan.Eberle      01/2018
//+===============+===============+===============+===============+===============+======
struct ScopedIssueReporter final : IIssueReporter
    {
    private:
        IssueReporter const& m_issues;
        bool m_logErrors = true;

    public:
        ScopedIssueReporter(ECDbCR, bool logErrors);
        ~ScopedIssueReporter() {}
        void Report(Utf8CP message) const override;
        void ReportV(Utf8CP message, ...) const override;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE