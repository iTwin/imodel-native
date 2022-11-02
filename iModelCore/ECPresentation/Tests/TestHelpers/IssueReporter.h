/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ECPresentationTest.h"

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct IIssueListener
    {
    //! Called for each test assertion failure.
    //! @param[in] message The test error message.
    virtual void _OnIssueReported(Utf8CP message) = 0;
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct IssueReporter : NonCopyableClass
    {
    private:
        IIssueListener* m_issueListener;

    public:
        explicit IssueReporter() : m_issueListener(nullptr) {}
        ~IssueReporter() {}

        BentleyStatus AddListener(IIssueListener&);
        void RemoveListener();

        //! Sends the message to _OnIssueReported
        //! @param[in] message To be sent to the IIssueListener
        void Report(Utf8CP message, ...) const;
    };

//==========================================================================================
// @bsiclass
// Standard issue listener, send messages to printf
//==========================================================================================
struct StandardIssueListener : IIssueListener
    {
    public:
        StandardIssueListener() {};
        void _OnIssueReported(Utf8CP message) override;
    };

//============================================================================================
// @bsiclass
// Issue Listener using Native Logging
//============================================================================================
struct NativeLoggingIssueListener : IIssueListener
    {
    private:
        CharCP m_namespace;
        NativeLogging::SEVERITY m_severity;

    public:
        NativeLoggingIssueListener(CharCP namespaceUsed, NativeLogging::SEVERITY severity);
        void _OnIssueReported(Utf8CP message) override;
    };

END_ECPRESENTATIONTESTS_NAMESPACE
