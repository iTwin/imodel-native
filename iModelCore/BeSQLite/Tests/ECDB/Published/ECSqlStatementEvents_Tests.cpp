/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ECDB/Published/ECSqlStatementEvents_Tests.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlTestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                 07/14
//+---------------+---------------+---------------+---------------+---------------+------
struct TestEventHandler : ECSqlEventHandler
    {
private:
    EventType m_eventType;
    size_t m_timesEventOccured;
    ECSqlEventArgs::ECInstanceKeyList m_list;
    virtual void _OnEvent (EventType eventType, ECSqlEventArgs const& args) override
        {
        m_eventType = eventType;
        m_timesEventOccured++;
        m_list = args.GetInstanceKeys ();
        }

public:
    TestEventHandler ()
        : ECSqlEventHandler (), m_timesEventOccured (0)
        {}

    ~TestEventHandler ()
        {}

    void Reset ()
        {
        m_timesEventOccured = 0;
        m_list.clear ();
        }

    ECSqlEventArgs::ECInstanceKeyList const& GetInstanceKeys () const { return m_list; }
    EventType GetEventType () const { return m_eventType; }
    size_t GetRowsAffected () const { return m_list.size(); }
    size_t GetTimesEventOccurred () const { return m_timesEventOccured; }
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_RegisterUnregisterEventHandler)
    {
    const auto perClassRowCount = 2;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);

    ECSqlStatement statement;
    TestEventHandler eventHandler;
    auto stat = statement.RegisterEventHandler (eventHandler);
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Registering an event handler before preparing the statement is expected to succeed.";

    stat = statement.RegisterEventHandler (eventHandler);
    ASSERT_EQ ((int) ECSqlStatus::UserError, (int) stat) << "Registering the same event handler twice is expected to fail.";

    TestEventHandler eventHandler2;
    stat = statement.RegisterEventHandler (eventHandler2);
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Registering another handler must succeed.";

    stat = statement.UnregisterEventHandler (eventHandler);
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Unregistering a registered event handler is expected to work.";

    //prepare somewhere in the middle to test that this does not affect event handler management
    stat = statement.Prepare (ecdb, "INSERT INTO ecsql.P (I, S) VALUES (9999, 'Event handler test')");
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Preparation failed unexpectedly";

    stat = statement.RegisterEventHandler (eventHandler);
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Re-registering an unregistered event handler is expected to work.";

    stat = statement.UnregisterEventHandler (eventHandler2);
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Unregistering a registered event handler is expected to work.";

    stat = statement.RegisterEventHandler (eventHandler2);
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Re-registering an unregistered event handler is expected to work.";

    stat = statement.UnregisterAllEventHandlers ();
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Unregistering all event handlers is expected to work.";

    stat = statement.UnregisterAllEventHandlers ();
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Calling UnregisterAllEventHandlers twice is expected to work.";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_EventHandlingAcrossFinalize)
    {
    const auto perClassRowCount = 2;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);

    ECSqlStatement statement;
    TestEventHandler eventHandler;
    auto stat = statement.RegisterEventHandler (eventHandler);
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Registering an event handler before preparing the statement is expected to succeed.";

    stat = statement.RegisterEventHandler (eventHandler);
    ASSERT_EQ ((int) ECSqlStatus::UserError, (int) stat) << "Registering the same event handler twice is expected to fail.";

    TestEventHandler eventHandler2;
    stat = statement.RegisterEventHandler (eventHandler2);
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Registering another handler must succeed.";

    stat = statement.Prepare (ecdb, "INSERT INTO ecsql.P (I, S) VALUES (9999, 'Event handler test')");
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Preparation failed unexpectedly";

    ECInstanceKey newInstanceKey;
    ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) statement.Step (newInstanceKey)) << "Step failed unexpectedly";

    ASSERT_EQ ((int) ECSqlEventHandler::EventType::Insert, (int) eventHandler.GetEventType ());
    ASSERT_EQ (1, eventHandler.GetRowsAffected ());
    ASSERT_EQ (1, eventHandler.GetTimesEventOccurred ());
    ASSERT_TRUE (eventHandler.GetInstanceKeys ().at (0) == newInstanceKey);

    //second event handler should yield same results
    ASSERT_EQ ((int) ECSqlEventHandler::EventType::Insert, (int) eventHandler2.GetEventType ());
    ASSERT_EQ (1, eventHandler2.GetRowsAffected ());
    ASSERT_EQ (1, eventHandler2.GetTimesEventOccurred ());
    ASSERT_TRUE (eventHandler.GetInstanceKeys ().at (0) == newInstanceKey);


    statement.Finalize ();

    //execute a new statement, in which we delete the inserted row again
    stat = statement.Prepare (ecdb, "UPDATE ONLY ecsql.P SET S = 'hello' WHERE ECInstanceId = ?");
    ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat) << "Preparation failed unexpectedly";
    statement.BindId (1, newInstanceKey.GetECInstanceId ());

    ASSERT_EQ ((int)ECSqlStepStatus::Done, (int)statement.Step ()) << "Step failed unexpectedly";

    ASSERT_EQ ((int)ECSqlEventHandler::EventType::Update, (int)eventHandler.GetEventType ());
    ASSERT_EQ (1, eventHandler.GetRowsAffected ());
    ASSERT_EQ (2, eventHandler.GetTimesEventOccurred ());

    //second event handler should yield same results
    ASSERT_EQ ((int)ECSqlEventHandler::EventType::Update, (int)eventHandler2.GetEventType ());
    ASSERT_EQ (1, eventHandler2.GetRowsAffected ());
    ASSERT_EQ (2, eventHandler2.GetTimesEventOccurred ());

    //now finalize statement, event handler should stay registered
    statement.Finalize ();

    //execute a new statement, in which we delete the inserted row again
    stat = statement.Prepare (ecdb, "DELETE FROM ONLY ecsql.P WHERE ECInstanceId = ?");
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Preparation failed unexpectedly";
    statement.BindId (1, newInstanceKey.GetECInstanceId ());

    ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) statement.Step ()) << "Step failed unexpectedly";

    ASSERT_EQ ((int) ECSqlEventHandler::EventType::Delete, (int) eventHandler.GetEventType ());
    ASSERT_EQ (1, eventHandler.GetRowsAffected ());
    ASSERT_EQ (3, eventHandler.GetTimesEventOccurred ());

    //second event handler should yield same results
    ASSERT_EQ ((int) ECSqlEventHandler::EventType::Delete, (int) eventHandler2.GetEventType ());
    ASSERT_EQ (1, eventHandler2.GetRowsAffected ());
    ASSERT_EQ (3, eventHandler2.GetTimesEventOccurred ());
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                 01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_DeleteEvent)
    {
    const auto perClassRowCount = 2;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);
    auto ecsql = "DELETE FROM  ONLY ecsql.PSA WHERE I = ?";

    ECSqlStatement statement;

    TestEventHandler deleteEventHandler;
    statement.RegisterEventHandler (deleteEventHandler);

    auto stat = statement.Prepare (ecdb, ecsql);
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Preparation of '" << ecsql << "' failed: " << statement.GetLastStatusMessage ();

    statement.BindInt (1, 123);

    auto stepStatus = statement.Step ();
    ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) stepStatus) << "Step for '" << ecsql << "' failed: " << statement.GetLastStatusMessage ();

    ASSERT_EQ ((int) ECSqlEventHandler::EventType::Delete, (int) deleteEventHandler.GetEventType ());
    ASSERT_EQ (deleteEventHandler.GetTimesEventOccurred (), 1);
    ASSERT_EQ (deleteEventHandler.GetRowsAffected (), 2);

    ECClassP psaClass = nullptr;
    ASSERT_EQ (SUCCESS, ecdb.GetEC ().GetSchemaManager ().GetECClass (psaClass, "ECSqlTest", "PSA"));
    for (auto const& instanceKey : deleteEventHandler.GetInstanceKeys ())
        {
        ASSERT_EQ (psaClass->GetId (), instanceKey.GetECClassId ()) << "Unexpected ECClassIds in event args.";
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                 01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_InsertEvent)
    {
    const auto perClassRowCount = 2;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);
    auto ecsql = "INSERT INTO ecsql.P (I, S) VALUES (9999, 'Event handler test')";

    ECSqlStatement statement;

    TestEventHandler insertEventHandler;
    statement.RegisterEventHandler (insertEventHandler);
    
    auto stat = statement.Prepare (ecdb, ecsql);
    ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat) << "Preparation of '" << ecsql << "' failed: " << statement.GetLastStatusMessage ();

    ECInstanceKey generatedKey;
    auto stepStatus = statement.Step (generatedKey);
    ASSERT_EQ ((int)ECSqlStepStatus::Done, (int)stepStatus) << "Step for '" << ecsql << "' failed: " << statement.GetLastStatusMessage ();

    ASSERT_EQ ((int)ECSqlEventHandler::EventType::Insert, (int)insertEventHandler.GetEventType ());
    ASSERT_EQ (insertEventHandler.GetTimesEventOccurred (), 1);
    ASSERT_EQ (insertEventHandler.GetRowsAffected (), 1);

    ECInstanceKeyCR eventArgsInstanceKey = insertEventHandler.GetInstanceKeys ()[0];
    ASSERT_TRUE (generatedKey == eventArgsInstanceKey) << "Unexpected ECInstanceId in event args.";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                 01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_UpdateEvent)
    {
    const auto perClassRowCount = 2;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OPEN_ReadWrite, DefaultTxn_Yes), perClassRowCount);
    auto ecsql = "UPDATE ONLY ecsql.P SET S = 'hello' WHERE I = ?";

    ECSqlStatement statement;

    TestEventHandler updateEventHandler;
    statement.RegisterEventHandler (updateEventHandler);

    auto stat = statement.Prepare (ecdb, ecsql);
    ASSERT_EQ ((int)ECSqlStatus::Success, (int)stat) << "Preparation of '" << ecsql << "' failed: " << statement.GetLastStatusMessage ();

    statement.BindInt (1, 123);

    auto stepStatus = statement.Step ();
    ASSERT_EQ ((int)ECSqlStepStatus::Done, (int)stepStatus) << "Step for '" << ecsql << "' failed: " << statement.GetLastStatusMessage ();

    ASSERT_EQ ((int)ECSqlEventHandler::EventType::Update, (int)updateEventHandler.GetEventType ());
    ASSERT_EQ (1, updateEventHandler.GetTimesEventOccurred ());
    ASSERT_EQ (2, updateEventHandler.GetRowsAffected ());

    ECClassP pClass = nullptr;
    ASSERT_EQ (SUCCESS, ecdb.GetEC ().GetSchemaManager ().GetECClass (pClass, "ECSqlTest", "P"));

    for (auto const& instanceKey : updateEventHandler.GetInstanceKeys ())
        {
        ASSERT_EQ (pClass->GetId (), instanceKey.GetECClassId ()) << "Unexpected ECClassIds in event args.";
        }
    }


END_ECDBUNITTESTS_NAMESPACE
