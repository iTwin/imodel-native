/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlStatementEvents_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlTestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 02/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlTestFixture, ECSqlStatement_DefaultEventHandler)
    {
    // Create and populate a sample project
    auto& ecdb = SetUp("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams(Db::OpenMode::ReadWrite), false);

        {
        ECSqlStatement stmt;
        // Enable/Disable
        ASSERT_TRUE(stmt.GetDefaultEventHandler() == nullptr) << "GetDefaultEventHandler is expected to return nullptr if it isn't enabled.";

        stmt.EnableDefaultEventHandler();
        ASSERT_TRUE(stmt.GetDefaultEventHandler() != nullptr) << "GetDefaultEventHandler is expected to not return nullptr if it is enabled.";

        ASSERT_EQ(-1, stmt.GetDefaultEventHandler()->GetInstancesAffectedCount());
        ASSERT_TRUE(stmt.GetDefaultEventHandler()->GetArgs () == nullptr);

        stmt.DisableDefaultEventHandler();
        ASSERT_TRUE(stmt.GetDefaultEventHandler() == nullptr) << "GetDefaultEventHandler is expected to return nullptr if it was disabled.";

        //Call UnregisterEventHandler with default handler
        stmt.EnableDefaultEventHandler();
        DefaultECSqlEventHandler const* defaultEh = stmt.GetDefaultEventHandler();
        ASSERT_TRUE(defaultEh != nullptr) << "GetDefaultEventHandler is expected to not return nullptr if it is enabled.";

        ASSERT_EQ((int) ECSqlStatus::Success, (int) stmt.UnregisterEventHandler(const_cast<DefaultECSqlEventHandler&> (*defaultEh)));
        ASSERT_TRUE(stmt.GetDefaultEventHandler() == nullptr) << "GetDefaultEventHandler is expected to return nullptr if it was unregistered via UnregisterEventHandler.";

        //Call UnregisterAllEventHandlers
        stmt.EnableDefaultEventHandler();
        ASSERT_TRUE(stmt.GetDefaultEventHandler() != nullptr) << "GetDefaultEventHandler is expected to not return nullptr if it is enabled.";
        stmt.UnregisterAllEventHandlers();
        ASSERT_TRUE(stmt.GetDefaultEventHandler() == nullptr) << "GetDefaultEventHandler is expected to return nullptr after a call to UnregisterAllEventHandlers.";

        //Across finalize
        stmt.EnableDefaultEventHandler();
        ASSERT_TRUE(stmt.GetDefaultEventHandler() != nullptr) << "GetDefaultEventHandler is expected to not return nullptr if it is enabled.";
        stmt.Finalize();
        ASSERT_TRUE(stmt.GetDefaultEventHandler() != nullptr) << "GetDefaultEventHandler is expected to not return nullptr after a call to Finalize.";
        }

    //now test that handler state gets cleared after two subsequent executions

        {
        //set up test data
        ECSqlStatement stmt;
        ASSERT_EQ((int) ECSqlStatus::Success, (int) stmt.Prepare(ecdb, "INSERT INTO ecsql.PSA (I) VALUES (?)"));
        stmt.BindInt(1, 1);
        ASSERT_EQ((int) ECSqlStepStatus::Done, (int) stmt.Step());
        stmt.Reset();
        stmt.BindInt(1, 2);
        ASSERT_EQ((int) ECSqlStepStatus::Done, (int) stmt.Step());
        stmt.Reset();
        stmt.BindInt(1, 3);
        ASSERT_EQ((int) ECSqlStepStatus::Done, (int) stmt.Step());
        stmt.Reset();
        stmt.BindInt(1, 4);
        ASSERT_EQ((int) ECSqlStepStatus::Done, (int) stmt.Step());
        ecdb.SaveChanges();
        //test table:
        // I
        // --
        // 1
        // 2
        // 3
        // 4
        }

        {
        ECSqlStatement stmt;
        stmt.EnableDefaultEventHandler();
        ASSERT_EQ(-1, stmt.GetDefaultEventHandler()->GetInstancesAffectedCount());
        ASSERT_TRUE(stmt.GetDefaultEventHandler()->GetArgs() == nullptr);

        ASSERT_EQ((int) ECSqlStatus::Success, (int) stmt.Prepare(ecdb, "UPDATE ONLY ecsql.PSA SET I = (-1)*I WHERE I BETWEEN ? AND ?"));
        stmt.BindInt(1, 1);
        stmt.BindInt(2, 1);
        ASSERT_EQ((int) ECSqlStepStatus::Done, (int) stmt.Step());

        //test table now:
        // I
        // --
        // -1
        // 2
        // 3
        // 4

        ASSERT_EQ((int) ECSqlEventHandler::EventType::Update, (int) stmt.GetDefaultEventHandler()->GetEventType());
        ASSERT_EQ(1, stmt.GetDefaultEventHandler()->GetInstancesAffectedCount());
        ASSERT_TRUE(stmt.GetDefaultEventHandler()->GetArgs() != nullptr);

        stmt.Reset();
        stmt.ClearBindings();
        ASSERT_EQ(1, stmt.GetDefaultEventHandler()->GetInstancesAffectedCount()) << "Reset doesn't reset the default event handler";
        ASSERT_TRUE(stmt.GetDefaultEventHandler()->GetArgs() != nullptr) << "Reset doesn't reset the default event handler";

        stmt.BindInt(1, 3);
        stmt.BindInt(2, 4);
        ASSERT_EQ((int) ECSqlStepStatus::Done, (int) stmt.Step());
        //test table now:
        // I
        // --
        // -1
        // 2
        // -3
        // -4

        ASSERT_EQ((int) ECSqlEventHandler::EventType::Update, (int) stmt.GetDefaultEventHandler()->GetEventType());
        ASSERT_EQ(2, stmt.GetDefaultEventHandler()->GetInstancesAffectedCount());
        ASSERT_TRUE(stmt.GetDefaultEventHandler()->GetArgs() != nullptr);

        stmt.Reset();
        stmt.ClearBindings();

        stmt.BindInt(1, -10);
        stmt.BindInt(2, 10);
        ASSERT_EQ((int) ECSqlStepStatus::Done, (int) stmt.Step());
        //test table now:
        // I
        // --
        // -1
        // -2
        // -3
        // -4
        ASSERT_EQ((int) ECSqlEventHandler::EventType::Update, (int) stmt.GetDefaultEventHandler()->GetEventType());
        ASSERT_EQ(4, stmt.GetDefaultEventHandler()->GetInstancesAffectedCount());
        ASSERT_TRUE(stmt.GetDefaultEventHandler()->GetArgs() != nullptr);
        }
    }

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
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::ReadWrite), perClassRowCount);

    ECSqlStatement statement;
    TestEventHandler eventHandler;
    statement.RegisterEventHandler (eventHandler);
    //Registering event handler twice should succeed (and not register it again)
    statement.RegisterEventHandler(eventHandler);

    TestEventHandler eventHandler2;
    statement.RegisterEventHandler (eventHandler2);

    ECSqlStatus stat = statement.UnregisterEventHandler (eventHandler);
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Unregistering a registered event handler is expected to work.";

    //prepare somewhere in the middle to test that this does not affect event handler management
    stat = statement.Prepare (ecdb, "INSERT INTO ecsql.P (I, S) VALUES (9999, 'Event handler test')");
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Preparation failed unexpectedly";

    statement.RegisterEventHandler (eventHandler);
    stat = statement.UnregisterEventHandler (eventHandler2);
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Unregistering a registered event handler is expected to work.";

    statement.UnregisterAllEventHandlers ();
    //calling UnregisterAllEventHandlers twice is expected to work
    statement.UnregisterAllEventHandlers ();

    //unregister a handler which is not registered should fail
    TestEventHandler eventHandler3;
    stat = statement.UnregisterEventHandler(eventHandler3);
    ASSERT_EQ((int) ECSqlStatus::UserError, (int) stat) << "Unregistering an event handler which is not registered is expected to fail.";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_EventHandlingAcrossFinalize)
    {
    const auto perClassRowCount = 2;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::ReadWrite), perClassRowCount);

    ECSqlStatement statement;
    TestEventHandler eventHandler;
    statement.RegisterEventHandler (eventHandler);

    TestEventHandler eventHandler2;
    statement.RegisterEventHandler (eventHandler2);

    ECSqlStatus stat = statement.Prepare (ecdb, "INSERT INTO ecsql.P (I, S) VALUES (9999, 'Event handler test')");
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
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::ReadWrite), perClassRowCount);
    auto ecsql = "DELETE FROM  ONLY ecsql.PSA WHERE I = ?";

    ECSqlStatement statement;

    statement.EnableDefaultEventHandler();

    TestEventHandler deleteEventHandler;
    statement.RegisterEventHandler (deleteEventHandler);

    auto stat = statement.Prepare (ecdb, ecsql);
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat) << "Preparation of '" << ecsql << "' failed: " << statement.GetLastStatusMessage ();

    statement.BindInt (1, 123);

    auto stepStatus = statement.Step ();
    ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) stepStatus) << "Step for '" << ecsql << "' failed: " << statement.GetLastStatusMessage ();

    ASSERT_EQ ((int) ECSqlEventHandler::EventType::Delete, (int) deleteEventHandler.GetEventType ());
    ASSERT_EQ (1, deleteEventHandler.GetTimesEventOccurred ());
    ASSERT_EQ (2, deleteEventHandler.GetRowsAffected ());


    ECClassCP psaClass = ecdb. Schemas ().GetECClass ("ECSqlTest", "PSA");
    ASSERT_TRUE (psaClass != nullptr);
    for (auto const& instanceKey : deleteEventHandler.GetInstanceKeys ())
        {
        ASSERT_EQ (psaClass->GetId (), instanceKey.GetECClassId ()) << "Unexpected ECClassIds in event args.";
        }

    //default event handler should report the same
    ASSERT_TRUE(statement.GetDefaultEventHandler() != nullptr);
    ASSERT_EQ((int) deleteEventHandler.GetEventType(), (int) statement.GetDefaultEventHandler()->GetEventType());
    ASSERT_EQ(deleteEventHandler.GetRowsAffected(), statement.GetDefaultEventHandler()->GetInstancesAffectedCount());
    for (auto const& instanceKey : statement.GetDefaultEventHandler()->GetArgs ()->GetInstanceKeys())
        {
        ASSERT_EQ(psaClass->GetId(), instanceKey.GetECClassId()) << "Unexpected ECClassIds in event args of default event handler";
        }

    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                 01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlTestFixture, ECSqlStatement_InsertEvent)
    {
    const auto perClassRowCount = 2;
    // Create and populate a sample project
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::ReadWrite), perClassRowCount);
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
    auto& ecdb = SetUp ("ecsqlstatementtests.ecdb", L"ECSqlTest.01.00.ecschema.xml", ECDb::OpenParams (Db::OpenMode::ReadWrite), perClassRowCount);
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

    ECClassCP pClass = ecdb. Schemas ().GetECClass ("ECSqlTest", "P");
    ASSERT_TRUE (pClass != nullptr);

    for (auto const& instanceKey : updateEventHandler.GetInstanceKeys ())
        {
        ASSERT_EQ (pClass->GetId (), instanceKey.GetECClassId ()) << "Unexpected ECClassIds in event args.";
        }
    }

END_ECDBUNITTESTS_NAMESPACE
