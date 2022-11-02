/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "IssueReporter.h"

USING_NAMESPACE_ECPRESENTATIONTESTS

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus IssueReporter::AddListener(IIssueListener& issueListener)
    {
    if (m_issueListener != nullptr)
        return ERROR;

    m_issueListener = &issueListener;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void IssueReporter::RemoveListener()
    {
    m_issueListener = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//----------------------------------------------------------------------------------
void StandardIssueListener::_OnIssueReported(Utf8CP message)
    {
    printf("%s", message);
    }

//----------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------
NativeLoggingIssueListener::NativeLoggingIssueListener(CharCP namespaceUsed, NativeLogging::SEVERITY severity)
    {
    m_namespace = namespaceUsed;
    m_severity = severity;
    }

//----------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------
void NativeLoggingIssueListener::_OnIssueReported(Utf8CP message)
    {
    NativeLogging::CategoryLogger(m_namespace).message(m_severity, message);
    }
