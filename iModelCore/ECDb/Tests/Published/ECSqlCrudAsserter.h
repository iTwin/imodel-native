/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlCrudAsserter.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECSqlTestDataset.h"
#include <Logging/bentleylogging.h>

BEGIN_ECDBUNITTESTS_NAMESPACE
//=======================================================================================    
// @bsiclass                                                 Krischan.Eberle     04/2013
//=======================================================================================    
struct ECSqlCrudAsserter : NonCopyableClass
    {
protected:
    struct DisableBeAsserts
        {
    public:
        explicit DisableBeAsserts (bool disable = true)
            {
            BeTest::SetFailOnAssert (!disable);
            }

        ~DisableBeAsserts ()
            {
            BeTest::SetFailOnAssert (true);
            }
        };
    #define DISABLE_BEASSERTS DisableBeAsserts disableBeAsserts;

private:
    static BentleyApi::NativeLogging::ILogger* s_logger;

    ECDbTestProject& m_testProject;

    virtual void _Assert (ECSqlTestItem const& testItem) const = 0;
    virtual Utf8CP _GetTargetOperationName () const = 0;

    void LogECSqlSupport (ECSqlTestItem const& testItem) const;
    static BentleyApi::NativeLogging::ILogger& GetLogger ();

protected:

    ECDbTestProject& GetTestProject() const { return m_testProject; }
    ECDbR GetDgnDb () const;

public:
    explicit ECSqlCrudAsserter (ECDbTestProject& testProject) : m_testProject(testProject) {}
    virtual ~ECSqlCrudAsserter () {}

    void Assert (ECSqlTestItem const& testItem) const;
    Utf8CP GetTargetOperationName () const;
    };

typedef std::vector<std::unique_ptr<ECSqlCrudAsserter>> ECSqlCrudAsserterList;

END_ECDBUNITTESTS_NAMESPACE