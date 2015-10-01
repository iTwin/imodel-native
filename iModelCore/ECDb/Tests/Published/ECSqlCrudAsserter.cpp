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
    _Assert (testItem);
    LogECSqlSupport (testItem);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlCrudAsserter::LogECSqlSupport (ECSqlTestItem const& testItem) const
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
        GetLogger ().debug (logMessage.c_str ());
        }
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