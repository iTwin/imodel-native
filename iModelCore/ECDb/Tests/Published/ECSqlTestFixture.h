/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/ECDB/Published/ECSqlTestFixture.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECSqlCrudAsserter.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================    
// @bsiclass                                                 Krischan.Eberle     04/2013
//=======================================================================================    
struct ECSqlTestFixture : public ::testing::Test
    {
public:
    struct EventHandler : ECSqlEventHandler
        {
    private:
        int m_rowsAffected;

        virtual void _OnEvent (EventType eventType, ECSqlEventArgs const& args) override
            {
            m_rowsAffected = (int) args.GetInstanceKeys ().size ();
            }

    public:
        EventHandler ()
            : ECSqlEventHandler (), m_rowsAffected (-1)
            {}

        ~EventHandler ()
            {}

        int GetRowsAffected () const { return m_rowsAffected; }
        };

private:
    std::unique_ptr<ECDbTestProject> m_testProject;

    virtual ECDbR _SetUp (Utf8CP ecdbFileName, WCharCP schemaECXmlFileName, ECDb::OpenParams openParams, int perClassRowCount);
    virtual ECDbTestProject& _GetTestProject () const;

protected:
    static std::unique_ptr<ECDbTestProject> CreateTestProject (Utf8CP ecdbFileName, WCharCP schemaECXmlFileName, ECDb::OpenParams openParams, int perClassRowCount);
    ECDbR SetUp (Utf8CP ecdbFileName, WCharCP schemaECXmlFileName, ECDb::OpenParams openParams, int perClassRowCount);

    static void BindFromJson (BentleyStatus& succeeded, ECSqlStatement const& statement, JsonValueCR jsonValue, IECSqlBinder& structBinder);
    static void VerifyECSqlValue (ECSqlStatement const& statement, JsonValueCR expectedValue, IECSqlValue const& ecsqlValue);

    void SetTestProject (std::unique_ptr<ECDbTestProject> testProject);
    ECDbTestProject& GetTestProject () const;

public:
    ECSqlTestFixture ();
    virtual ~ECSqlTestFixture ();
    virtual void SetUp () override {}
    virtual void TearDown () override {}
    };


END_ECDBUNITTESTS_NAMESPACE