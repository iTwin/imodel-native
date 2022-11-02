/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <set>
#include <ECObjects/SchemaComparer.h>
#include "../BackDoor/PublicAPI/BackDoor/ECDb/BackDoor.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct SchemaChangesetTestFixture : public ECDbTestFixture
    {
    std::vector<Utf8String> m_updatedDbs;
    protected:

        //---------------------------------------------------------------------------------------
        // @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        void CloseReOpenECDb()
            {
            Utf8CP dbFileName = m_ecdb.GetDbFileName();
            BeFileName dbPath(dbFileName);
            m_ecdb.CloseDb();
            ASSERT_FALSE(m_ecdb.IsDbOpen());
            ASSERT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(dbPath, ECDb::OpenParams(ECDb::OpenMode::Readonly)));
            ASSERT_TRUE(m_ecdb.IsDbOpen());
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        DbResult OpenBesqliteDb(Utf8CP dbPath) { return m_ecdb.OpenBeSQLiteDb(dbPath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite)); }
    };

#define ASSERT_ECSQL(ECDB_OBJ, PREPARESTATUS, STEPSTATUS, ECSQL)   {\
                                                                    ECSqlStatement stmt;\
                                                                    ASSERT_EQ(PREPARESTATUS, stmt.Prepare(ECDB_OBJ, ECSQL));\
                                                                    if (PREPARESTATUS == ECSqlStatus::Success)\
                                                                        ASSERT_EQ(STEPSTATUS, stmt.Step());\
                                                                   }

struct SchemaChangesetTestChangeTracker : BeSQLite::ChangeTracker
    {
    SchemaChangesetTestChangeTracker(BeSQLite::DbR db) { SetDb(&db); }

    OnCommitStatus _OnCommit(bool isCommit, Utf8CP operation) override { return OnCommitStatus::Commit; }
    };

struct SchemaChangesetTestChangeSet : BeSQLite::ChangeSet
    {
    ConflictResolution _OnConflict(ConflictCause cause, BeSQLite::Changes::Change iter) override
        {
        return ConflictResolution::Skip;
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaChangesetTestFixture, RevertAndReinstateSchemaChange)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="Class1">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="A" typeName="string" />
            <ECProperty propertyName="B" typeName="string" />
            <ECProperty propertyName="C" typeName="string" />
            <ECProperty propertyName="D" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="Class2">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="D" typeName="string" />
            <ECProperty propertyName="C" typeName="string" />
            <ECProperty propertyName="B" typeName="string" />
            <ECProperty propertyName="A" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="Class3">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="C" typeName="string" />
            <ECProperty propertyName="A" typeName="string" />
            <ECProperty propertyName="D" typeName="string" />
            <ECProperty propertyName="B" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("revertAndReinstateSchemaChange.ecdb", schemaItem));
    SchemaChangesetTestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    // CHANGESET 1------------------------------------------------------------------
    //Setting property values to <ClassNumber><PropertyName><#RevisionOfInstance>
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Class1 (A,B,C,D) VALUES ('1A1','1B1','1C1','1D1')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Class2 (A,B,C,D) VALUES ('2A1','2B1','2C1','2D1')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Class3 (A,B,C,D) VALUES ('3A1','3B1','3C1','3D1')");
    SchemaChangesetTestChangeSet changeset1;
    ASSERT_EQ(BE_SQLITE_OK, changeset1.FromChangeTrack(tracker));

    ASSERT_EQ(JsonValue(R"json([{"A":"1A1","B":"1B1","C":"1C1","D":"1D1"}])json"), GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class1"));
    ASSERT_EQ(JsonValue(R"json([{"A":"2A1","B":"2B1","C":"2C1","D":"2D1"}])json"), GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class2"));
    ASSERT_EQ(JsonValue(R"json([{"A":"3A1","B":"3B1","C":"3C1","D":"3D1"}])json"), GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class3"));

    tracker.Restart();
    // CHANGESET 2------------------------------------------------------------------
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "UPDATE TestSchema.Class1 SET A='1A2',B='1B2',C='1C2',D='1D2' WHERE A='1A1'");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "UPDATE TestSchema.Class2 SET A='2A2',B='2B2',C='2C2',D='2D2' WHERE A='2A1'");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "UPDATE TestSchema.Class3 SET A='3A2',B='3B2',C='3C2',D='3D2' WHERE A='3A1'");
    SchemaChangesetTestChangeSet changeset2;
    ASSERT_EQ(BE_SQLITE_OK, changeset2.FromChangeTrack(tracker));

    tracker.Restart();
    // CHANGESET 3------------------------------------------------------------------
    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="A" typeName="string" />
            <ECProperty propertyName="B" typeName="string" />
            <ECProperty propertyName="C" typeName="string" />
            <ECProperty propertyName="D" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="Class1">
            <BaseClass>MyBaseClass</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="Class2">
            <BaseClass>MyBaseClass</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="Class3">
            <BaseClass>MyBaseClass</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));

    SchemaChangesetTestChangeSet changeset3;
    ASSERT_EQ(BE_SQLITE_OK, changeset3.FromChangeTrack(tracker));

    ASSERT_EQ(JsonValue(R"json([{"A":"1A2","B":"1B2","C":"1C2","D":"1D2"}])json"), GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class1"));
    ASSERT_EQ(JsonValue(R"json([{"A":"2A2","B":"2B2","C":"2C2","D":"2D2"}])json"), GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class2"));
    ASSERT_EQ(JsonValue(R"json([{"A":"3A2","B":"3B2","C":"3C2","D":"3D2"}])json"), GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class3"));

    tracker.Restart();
    // CHANGESET 4------------------------------------------------------------------
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "UPDATE TestSchema.Class1 SET A='1A3',B='1B3',C='1C3',D='1D3' WHERE A='1A2'");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "UPDATE TestSchema.Class2 SET A='2A3',B='2B3',C='2C3',D='2D3' WHERE A='2A2'");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "UPDATE TestSchema.Class3 SET A='3A3',B='3B3',C='3C3',D='3D3' WHERE A='3A2'");
    SchemaChangesetTestChangeSet changeset4;
    ASSERT_EQ(BE_SQLITE_OK, changeset4.FromChangeTrack(tracker));
    tracker.EndTracking();

    SchemaChangesetTestChangeSet changeset4Inverted;
    changeset4.ToChangeSet(changeset4Inverted, true);

    SchemaChangesetTestChangeSet changeset3Inverted;
    changeset3.ToChangeSet(changeset3Inverted, true);

    SchemaChangesetTestChangeSet changeset2Inverted;
    changeset2.ToChangeSet(changeset2Inverted, true);

    ASSERT_EQ(BE_SQLITE_OK, changeset4Inverted.ApplyChanges(m_ecdb));

    ASSERT_EQ(JsonValue(R"json([{"A":"1A2","B":"1B2","C":"1C2","D":"1D2"}])json"), GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class1"));
    ASSERT_EQ(JsonValue(R"json([{"A":"2A2","B":"2B2","C":"2C2","D":"2D2"}])json"), GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class2"));
    ASSERT_EQ(JsonValue(R"json([{"A":"3A2","B":"3B2","C":"3C2","D":"3D2"}])json"), GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class3"));

    ASSERT_EQ(BE_SQLITE_OK, changeset3Inverted.ApplyChanges(m_ecdb));
    m_ecdb.AfterSchemaChangeSetApplied();

    ASSERT_EQ(JsonValue(R"json([{"A":"1A2","B":"1B2","C":"1C2","D":"1D2"}])json"), GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class1"));
    ASSERT_EQ(JsonValue(R"json([{"A":"2A2","B":"2B2","C":"2C2","D":"2D2"}])json"), GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class2"));
    ASSERT_EQ(JsonValue(R"json([{"A":"3A2","B":"3B2","C":"3C2","D":"3D2"}])json"), GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class3"));

    ASSERT_EQ(BE_SQLITE_OK, changeset2Inverted.ApplyChanges(m_ecdb));
    ASSERT_EQ(JsonValue(R"json([{"A":"1A1","B":"1B1","C":"1C1","D":"1D1"}])json"), GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class1"));
    ASSERT_EQ(JsonValue(R"json([{"A":"2A1","B":"2B1","C":"2C1","D":"2D1"}])json"), GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class2"));
    ASSERT_EQ(JsonValue(R"json([{"A":"3A1","B":"3B1","C":"3C1","D":"3D1"}])json"), GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class3"));

    ASSERT_EQ(BE_SQLITE_OK, changeset2.ApplyChanges(m_ecdb));
    ASSERT_EQ(JsonValue(R"json([{"A":"1A2","B":"1B2","C":"1C2","D":"1D2"}])json"), GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class1"));
    ASSERT_EQ(JsonValue(R"json([{"A":"2A2","B":"2B2","C":"2C2","D":"2D2"}])json"), GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class2"));
    ASSERT_EQ(JsonValue(R"json([{"A":"3A2","B":"3B2","C":"3C2","D":"3D2"}])json"), GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class3"));

    // This step is problematic. We don't officially support reinstate, because this will run into trying to delete rows which don't exist.
    // Has to do with rows getting deleted and inserted instead of updated during the revert step.
    // Since our Changeset skips over any conflicts, we will continue here and verify the data, but if this stops working at some point, don't bother fixing,
    // it's not something that we officially support.
    ASSERT_EQ(BE_SQLITE_OK, changeset3.ApplyChanges(m_ecdb));
    m_ecdb.AfterSchemaChangeSetApplied();
    ASSERT_EQ(JsonValue(R"json([{"A":"1A2","B":"1B2","C":"1C2","D":"1D2"}])json"), GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class1"));
    auto result = GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class2");
    ASSERT_EQ(JsonValue(R"json([{"A":"2A2","B":"2B2","C":"2C2","D":"2D2"}])json"), result);
    ASSERT_EQ(JsonValue(R"json([{"A":"3A2","B":"3B2","C":"3C2","D":"3D2"}])json"), GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class3"));

    ASSERT_EQ(BE_SQLITE_OK, changeset4.ApplyChanges(m_ecdb));
    ASSERT_EQ(JsonValue(R"json([{"A":"1A3","B":"1B3","C":"1C3","D":"1D3"}])json"), GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class1"));
    ASSERT_EQ(JsonValue(R"json([{"A":"2A3","B":"2B3","C":"2C3","D":"2D3"}])json"), GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class2"));
    ASSERT_EQ(JsonValue(R"json([{"A":"3A3","B":"3B3","C":"3C3","D":"3D3"}])json"), GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class3"));
    }
END_ECDBUNITTESTS_NAMESPACE
