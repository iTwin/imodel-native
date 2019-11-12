/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "IssueReporter.h"

USING_NAMESPACE_ECPRESENTATIONTESTS

//---------------------------------------------------------------------------------------
// @bsimethod                                               Emily.Pazienza    10/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus IssueReporter::AddListener(IIssueListener& issueListener)
    {
    if (m_issueListener != nullptr)
        return ERROR;

    m_issueListener = &issueListener;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Emily.Pazienza    10/2016
//+---------------+---------------+---------------+---------------+---------------+------
void IssueReporter::RemoveListener()
    {
    m_issueListener = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Emily.Pazienza    10/2016
//+---------------+---------------+---------------+---------------+---------------+------
void IssueReporter::Report(Utf8CP message, ...) const
    {
    if (Utf8String::IsNullOrEmpty(message))
        return;

    va_list args;
    va_start(args, message);

    Utf8String formattedMessage;
    formattedMessage.VSprintf(message, args);

    if (m_issueListener != nullptr)
        m_issueListener->_OnIssueReported(formattedMessage.c_str());

    }

//----------------------------------------------------------------------------------
// @bsimethod                                       David.Le                10/2016
//----------------------------------------------------------------------------------
void StandardIssueListener::_OnIssueReported(Utf8CP message)
    {
    printf("%s", message);
    }

//----------------------------------------------------------------------------------
// @bsimethod                                       David.Le                10/2016
//----------------------------------------------------------------------------------
NativeLoggingIssueListener::NativeLoggingIssueListener(WCharCP namespaceUsed, NativeLogging::SEVERITY severity)
    {
    m_namespace = namespaceUsed;
    m_severity = severity;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                       David.Le                10/2016
//----------------------------------------------------------------------------------
void NativeLoggingIssueListener::_OnIssueReported(Utf8CP message)
    {
    NativeLogging::LoggingManager::GetLogger(m_namespace)->message(m_severity, message);
    }
