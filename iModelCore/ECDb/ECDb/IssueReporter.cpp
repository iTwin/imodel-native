/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/IssueReporter.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
void IssueReporter::Report(Utf8CP message, ...) const
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

        if (m_issueListener != nullptr)
            m_issueListener->ReportIssue(formattedMessage.c_str());

        if (isLogSeverityEnabled)
            LOG.message(s_logSeverity, formattedMessage.c_str());

        va_end(args);
        }
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

END_BENTLEY_SQLITE_EC_NAMESPACE
