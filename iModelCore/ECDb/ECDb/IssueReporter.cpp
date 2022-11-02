/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

IssueType IssueType::ECDbIssue = "ECDbIssue";

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void IssueDataSource::SetOldStyleListener(ECN::IIssueListener const* listener) const {
    BeMutexHolder lock(m_issueEvent.GetMutex());
    if (m_issueListenerCancel != nullptr ) {
        m_issueListenerCancel();
    }
    if (listener == nullptr)
        return;

    m_issueListenerCancel = OnIssueReported().AddListener([=](ECN::IssueSeverity severity, ECN::IssueCategory category, ECN::IssueType type, Utf8CP message) -> void{
        //! dangerous use of ptr
        listener->ReportIssue(severity, category, type, message);
    });
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void IssueDataSource::SetIn(IssueDataSource& source) const {
    if (&source == this) {
        BeAssert(false && "cannot set this class as source");
        return;
    }
    ClearIn();
    m_sourceCancel = source.OnIssueReported().AddListener([&](ECN::IssueSeverity severity, ECN::IssueCategory category,ECN::IssueType type, Utf8CP message) -> void {
        Report(severity, category, type, message);
    });
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void IssueDataSource::ClearIn() const {
    if (m_sourceCancel != nullptr) {
        m_sourceCancel();
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void IssueDataSource::SetFilter(filter_callback_t filterCallback) const {
    BeMutexHolder lock(m_issueEvent.GetMutex());
    m_filterCallback = filterCallback;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void IssueDataSource::ClearFilter() const {
    BeMutexHolder lock(m_issueEvent.GetMutex());
    m_filterCallback= nullptr;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void IssueDataSource::Report(ECN::IssueSeverity severity, ECN::IssueCategory category, ECN::IssueType type, Utf8CP message)  const {
    BeMutexHolder lock(m_issueEvent.GetMutex());
    if (m_filterCallback != nullptr) {
        if (m_filterCallback(severity, category, type, message) == FilterAction::Ignore) {
            return;
        }
    }
    m_issueEvent.RaiseEvent(severity, category, type, message);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
cancel_callback_type IssueDataSource::AppendLogSink(IssueDataSource& source, Utf8CP loggerName) {
    NativeLogging::CategoryLogger logger(loggerName);
    return source.OnIssueReported().AddListener([=](ECN::IssueSeverity severity, ECN::IssueCategory category, ECN::IssueType type, Utf8CP message){
        const auto loggerServerity = IssueDataSource::s_serverityMap [severity];
        logger.message(loggerServerity, message);
    });
}

IssueDataSource::FilterScope::FilterScope(IssueDataSource const& source, filter_callback_t filterCallback ):m_source(source) { m_source.SetFilter(filterCallback); }
IssueDataSource::FilterScope::~FilterScope(){ m_source.ClearFilter();}

std::map<IssueSeverity,NativeLogging::SEVERITY> IssueDataSource::s_serverityMap = std::map<IssueSeverity,NativeLogging::SEVERITY> {
        {ECN::IssueSeverity::Info, NativeLogging::SEVERITY::LOG_INFO},
        {ECN::IssueSeverity::Warning, NativeLogging::SEVERITY::LOG_WARNING},
        {ECN::IssueSeverity::CriticalWarning, NativeLogging::SEVERITY::LOG_WARNING},
        {ECN::IssueSeverity::Error, NativeLogging::SEVERITY::LOG_ERROR},
        {ECN::IssueSeverity::Fatal, NativeLogging::SEVERITY::LOG_ERROR},
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
