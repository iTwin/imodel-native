/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/IssueReporter.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
bool IssueReporter::IsSeverityEnabled(ECDbIssueSeverity severity) const
    {
    return m_issueListener != nullptr || LOG.isSeverityEnabled(ToLogSeverity(severity));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  09/2015
//+---------------+---------------+---------------+---------------+---------------+------
void IssueReporter::Report(ECDbIssueSeverity severity, Utf8CP message, ...) const
    {
    if (Utf8String::IsNullOrEmpty(message))
        return;

    const NativeLogging::SEVERITY logSeverity = ToLogSeverity(severity);
    const bool isLogSeverityEnabled = LOG.isSeverityEnabled(logSeverity);

    BeMutexHolder lock(m_mutex);

    if (m_issueListener != nullptr || isLogSeverityEnabled)
        {
        va_list args;
        va_start(args, message);

        Utf8String formattedMessage;
        formattedMessage.VSprintf(message, args);

        if (m_issueListener != nullptr)
            m_issueListener->ReportIssue(severity, formattedMessage.c_str());

        if (isLogSeverityEnabled)
            LOG.message(logSeverity, formattedMessage.c_str());

        va_end(args);
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2014
//+---------------+---------------+---------------+---------------+---------------+------
void IssueReporter::ReportSqliteIssue(ECDbIssueSeverity sev, DbResult sqliteStat, Utf8CP messageHeader) const
    {
    if (BE_SQLITE_OK != sqliteStat && IsSeverityEnabled(sev))
        {
        if (messageHeader == nullptr)
            messageHeader = "SQLite error:";

        Utf8CP dbResultStr = ECDb::InterpretDbResult(sqliteStat);

        Utf8String lastSqliteErrorMsg = m_ecdb.GetLastError();
        //ECDb sometimes returns DbResult errors on its own. In that case there is no SQLite error to output
        if (lastSqliteErrorMsg.empty())
            Report(sev, "%s %s", messageHeader, dbResultStr);
        else
            Report(sev, "%s %s: %s", messageHeader, dbResultStr, lastSqliteErrorMsg.c_str());
        }
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
