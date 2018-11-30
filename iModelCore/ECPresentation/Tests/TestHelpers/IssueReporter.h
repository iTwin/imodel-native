/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/TestHelpers/IssueReporter.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECPresentationTest.h"

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

//=======================================================================================
// @bsiclass                                                  Emily.Pazienza     10/2016
//=======================================================================================
struct IIssueListener
    {
    //! Called for each test assertion failure.
    //! @param[in] message The test error message.
    virtual void _OnIssueReported(Utf8CP message) = 0;
    };

//=======================================================================================
// @bsiclass                                                  Emily.Pazienza     10/2016
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
// @bsiclass                                                    David.Le           10/2016
// Standard issue listener, send messages to printf
//==========================================================================================
struct StandardIssueListener : IIssueListener
    {
    public:
        StandardIssueListener() {};
        void _OnIssueReported(Utf8CP message) override;
    };

//============================================================================================
// @bsiclass                                                    David.Le            10/2016
// Issue Listener using Native Logging      
//============================================================================================
struct NativeLoggingIssueListener : IIssueListener
    {
    private:
        WCharCP m_namespace;
        NativeLogging::SEVERITY m_severity;

    public:
        NativeLoggingIssueListener(WCharCP namespaceUsed, NativeLogging::SEVERITY severity);
        void _OnIssueReported(Utf8CP message) override;
    };

END_ECPRESENTATIONTESTS_NAMESPACE
