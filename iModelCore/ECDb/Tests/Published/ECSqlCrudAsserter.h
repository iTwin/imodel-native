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

    ECDb& m_ecdb;

    virtual void _Assert (ECSqlTestItem const& testItem) const = 0;
    virtual Utf8CP _GetTargetOperationName () const = 0;

    void LogECSqlSupport (ECSqlTestItem const& testItem) const;
    static BentleyApi::NativeLogging::ILogger& GetLogger ();

protected:

    ECDb& GetECDb() const { return m_ecdb; }

public:
    explicit ECSqlCrudAsserter (ECDb& ecdb) : m_ecdb(ecdb) {}
    virtual ~ECSqlCrudAsserter () {}

    void Assert (ECSqlTestItem const& testItem) const;
    Utf8CP GetTargetOperationName () const { return _GetTargetOperationName(); }
    };

typedef std::vector<std::unique_ptr<ECSqlCrudAsserter>> ECSqlCrudAsserterList;

END_ECDBUNITTESTS_NAMESPACE