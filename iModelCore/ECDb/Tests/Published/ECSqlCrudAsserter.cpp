/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlCrudAsserter.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlCrudAsserter.h"

BEGIN_ECDBUNITTESTS_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  03/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
NativeLogging::ILogger* ECSqlCrudAsserter::s_logger = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlCrudAsserter::Assert (ECSqlTestItem const& testItem) const
    {
    Utf8String statementErrorMessage;
    _Assert (statementErrorMessage, testItem);
    LogECSqlSupport (testItem, statementErrorMessage.c_str ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlCrudAsserter::LogECSqlSupport (ECSqlTestItem const& testItem, Utf8CP statementErrorMessage) const
    {
    if (GetLogger ().isSeverityEnabled (NativeLogging::LOG_DEBUG))
        {
        const auto expectedResultCategory = testItem.GetExpectedResultCategory ();
        const auto expectedResultCategoryStr = IECSqlExpectedResult::CategoryToString (expectedResultCategory);

        Utf8String logMessage;
        if (GetTargetOperationName () != nullptr)
            logMessage.append (GetTargetOperationName ()).append ("> ");
        
        logMessage.append (expectedResultCategoryStr.c_str ()).append (": ");

        auto description = testItem.GetExpectedResultDescription ();
        if (!Utf8String::IsNullOrEmpty (description))
            logMessage.append (description).append (" ");

        logMessage.append (testItem.GetECSql ().c_str ());

        if (!Utf8String::IsNullOrEmpty (statementErrorMessage))
            logMessage.append (" - ").append (statementErrorMessage);
        
        GetLogger ().debug (logMessage.c_str ());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP ECSqlCrudAsserter::GetTargetOperationName () const
    {
    return _GetTargetOperationName ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECDbR ECSqlCrudAsserter::GetDgnDb () const
    {
    return m_testProject.GetECDb ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  03/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
NativeLogging::ILogger& ECSqlCrudAsserter::GetLogger ()
    {
    if (s_logger == nullptr)
        {
        s_logger = NativeLogging::LoggingManager::GetLogger (L"ECSqlSupport");
        }

    return *s_logger;
    }


END_ECDBUNITTESTS_NAMESPACE