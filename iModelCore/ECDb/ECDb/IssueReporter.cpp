/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  09/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus IssueReporter::AddListener(ECDb::IIssueListener const& issueListener)
    {
    BeMutexHolder lock(m_mutex);
    if (m_issueListener != nullptr)
        return ERROR;

    m_issueListener = &issueListener;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  09/2015
//+---------------+---------------+---------------+---------------+---------------+------
void IssueReporter::RemoveListener()
    {
    BeMutexHolder lock(m_mutex);
    m_issueListener = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  09/2015
//+---------------+---------------+---------------+---------------+---------------+------
void IssueReporter::Report(Utf8CP message) const
    {
    if (Utf8String::IsNullOrEmpty(message))
        return;

    BeMutexHolder lock(m_mutex);
    bool isLogSeverityEnabled = false;
    if (IsEnabled(&isLogSeverityEnabled))
        DoReport(message, isLogSeverityEnabled);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  09/2015
//+---------------+---------------+---------------+---------------+---------------+------
void IssueReporter::ReportV(Utf8CP message, ...) const
    {
    if (Utf8String::IsNullOrEmpty(message))
        return;

    BeMutexHolder lock(m_mutex);
    bool isLogSeverityEnabled = false;
    if (IsEnabled(&isLogSeverityEnabled))
        {
        va_list args;
        va_start(args, message);

        Utf8String formattedMessage;
        formattedMessage.VSprintf(message, args);

        DoReport(formattedMessage.c_str(), isLogSeverityEnabled);

        va_end(args);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  09/2015
//+---------------+---------------+---------------+---------------+---------------+------
void IssueReporter::DoReport(Utf8CP message, bool isLogSeverityEnabled) const
    {
    if (Utf8String::IsNullOrEmpty(message))
        return;

    if (m_issueListener != nullptr)
        m_issueListener->ReportIssue(message);

    if (isLogSeverityEnabled)
        LOG.message(s_logSeverity, message);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2017
//+---------------+---------------+---------------+---------------+---------------+------
bool IssueReporter::IsEnabled(bool* isLogEnabled /*= nullptr*/) const
    {
    const bool canLog = LOG.isSeverityEnabled(s_logSeverity);
    if (isLogEnabled != nullptr)
        *isLogEnabled = canLog;

    return m_issueListener != nullptr || canLog;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2018
//+---------------+---------------+---------------+---------------+---------------+------
ScopedIssueReporter::ScopedIssueReporter(ECDbCR ecdb, bool logErrors) : IIssueReporter(), m_issues(ecdb.GetImpl().Issues()), m_logErrors(logErrors) {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2018
//+---------------+---------------+---------------+---------------+---------------+------
void ScopedIssueReporter::Report(Utf8CP message) const
    {
    if (!m_logErrors)
        return;

    m_issues.Report(message);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2018
//+---------------+---------------+---------------+---------------+---------------+------
void ScopedIssueReporter::ReportV(Utf8CP message, ...) const
    {
    if (!m_logErrors)
        return;

    va_list args;
    va_start(args, message);

    Utf8String formattedMessage;
    formattedMessage.VSprintf(message, args);
    m_issues.Report(formattedMessage.c_str());
    va_end(args);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
