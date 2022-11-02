/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECDb/ECDb.h>
#include <ECObjects/IssueReporter.h>
#include <Bentley/BeEvent.h>
#include "ECDbLogger.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

using ECN::IssueSeverity;
using ECN::IssueCategory;

struct ECDB_EXPORT IssueType : ECN::IssueType
    {
    using ECN::IssueType::IssueType;

    static IssueType ECDbIssue;
    };

//---------------------------------------------------------------------------------------
// @bsistruct
//---------------------------------------------------------------------------------------
struct IssueDataSource final {
    enum class FilterAction {
        Ignore,
        Forward,
    };
    typedef BeEvent<ECN::IssueSeverity,ECN::IssueCategory,ECN::IssueType,Utf8CP> listener_t;
    typedef std::function<FilterAction(ECN::IssueSeverity,ECN::IssueCategory,ECN::IssueType,Utf8CP)> filter_callback_t;

    struct FilterScope final {
        private:
            IssueDataSource const& m_source;
        public:
            FilterScope(IssueDataSource const&, filter_callback_t );
            ~FilterScope();
            IssueDataSource const& Source() const {return m_source;}
    };

    private:
        mutable listener_t m_issueEvent;
        mutable filter_callback_t m_filterCallback;
        mutable cancel_callback_type m_sourceCancel;
        mutable cancel_callback_type m_issueListenerCancel;
        static std::map<IssueSeverity,NativeLogging::SEVERITY> s_serverityMap;
    public:
        IssueDataSource(){}
        IssueDataSource(IssueDataSource const&) = delete;
        IssueDataSource& operator=(IssueDataSource const&) = delete;
        // set null to remove last listener
        void SetOldStyleListener(ECN::IIssueListener const* listener) const;
        // Subscribe issue from another source and raise them locally
        void SetIn(IssueDataSource& source) const;
        // Clear the in;
        void ClearIn() const;
        // Allow to subscribe to events
        listener_t& OnIssueReported() const { return m_issueEvent;}
        // Set filter that would can prevent message from getting propagated
        void SetFilter(filter_callback_t filterCallback) const;
        // Clear filter callback
        void ClearFilter() const;
        // Report issue which will be propagated 
        void Report(ECN::IssueSeverity severity, ECN::IssueCategory category, ECN::IssueType type, Utf8CP message) const;
        template<typename ...FmtArgs>
        void ReportV(ECN::IssueSeverity severity, ECN::IssueCategory category, ECN::IssueType type, Utf8CP message, FmtArgs&& ...fmtArgs) const {
            Utf8String formattedMessage;
            formattedMessage.Sprintf(message, std::forward<FmtArgs>(fmtArgs)...);
            Report(severity, category, type, formattedMessage.c_str());
        }
        ~IssueDataSource() { ClearIn(); }
        // A default implementation of adding native logger sink to a issue data source
        static cancel_callback_type AppendLogSink(IssueDataSource& source, Utf8CP loggerName);
};


END_BENTLEY_SQLITE_EC_NAMESPACE