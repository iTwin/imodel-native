/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "../BackDoor/PublicAPI/BackDoor/ECDb/BackDoor.h"

// #define DUMP_CHANGE_SUMMARY 1

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   12/16
//=======================================================================================
struct TestChangeSet : BeSQLite::ChangeSet
    {
    ConflictResolution _OnConflict(ConflictCause cause, BeSQLite::Changes::Change iter) override { BeAssert(false && "Unexpected conflict"); return ConflictResolution::Skip; }

    JsonValue ToJson(Db const& db) const
        {
        JsonValue json(Json::ValueType::arrayValue);

        bmap<Utf8String, Json::Value> changedValuesPerTableMap;
        std::vector<Utf8String> tablesAsOrderedInChangeset;

        for (Changes::Change change : const_cast<TestChangeSet*>(this)->GetChanges())
            {
            Utf8CP tableName = nullptr;
            DbOpcode opCode;
            int nCols = -1;
            int indirect = -1;
            if (BE_SQLITE_OK != change.GetOperation(&tableName, &nCols, &opCode, &indirect))
                return JsonValue();

            bvector<Utf8String> cols;
            if (!db.GetColumns(cols, tableName))
                return JsonValue();

            Utf8String tblName(tableName);
            auto valueItor = changedValuesPerTableMap.find(tblName);
            if (valueItor == changedValuesPerTableMap.end())
                {
                valueItor = changedValuesPerTableMap.insert(make_bpair(tblName, Json::Value(Json::ValueType::objectValue))).first;
                valueItor->second["table"] = Json::Value(tblName);
                tablesAsOrderedInChangeset.push_back(tableName);

                Byte* pkCols;
                int pkColCount;
                if (BE_SQLITE_OK != change.GetPrimaryKeyColumns(&pkCols, &pkColCount))
                    return JsonValue();

                Json::Value pkColumns(Json::ValueType::arrayValue);
                for (size_t i = 0; i < pkColCount; i++)
                    {
                    if (pkCols[i] == 1)
                        {
                        Utf8StringCR col = cols.at(i);
                        pkColumns.append(Json::Value(col.c_str()));
                        }
                    }

                valueItor->second["pk"] = pkColumns;
                valueItor->second["rows"] = Json::Value(Json::ValueType::arrayValue);
                }

            Json::Value rowChange = Json::Value(Json::ValueType::objectValue);
            if (opCode == DbOpcode::Delete)
                rowChange["op"] = Json::Value("delete");
            else if (opCode == DbOpcode::Insert)
                rowChange["op"] = Json::Value("insert");
            else
                rowChange["op"] = Json::Value("update");

            rowChange["indirect"] = Json::Value(indirect != 0);
            rowChange["values"] = Json::Value(Json::ValueType::arrayValue);
            for (int i = 0; i < nCols; i++)
                {
                Utf8StringCR col = cols.at((size_t) i);
                Json::Value changedColValues(Json::ValueType::objectValue);
                Json::Value colValueChange(Json::ValueType::objectValue);
                if (DbOpcode::Insert == opCode)
                    colValueChange["after"] = TestUtilities::DbValueToJson(change.GetValue(i, Changes::Change::Stage::New));
                else if (DbOpcode::Delete == opCode)
                    colValueChange["before"] = TestUtilities::DbValueToJson(change.GetValue(i, Changes::Change::Stage::Old));
                else
                    {
                    DbValue oldValue = change.GetOldValue(i);
                    DbValue newValue = change.GetNewValue(i);
                    if (oldValue.IsValid())
                        colValueChange["before"] = TestUtilities::DbValueToJson(oldValue);

                    if (newValue.IsValid())
                        colValueChange["after"] = TestUtilities::DbValueToJson(newValue);
                    }

                if (!colValueChange.empty())
                    {
                    changedColValues[col.c_str()] = colValueChange;
                    rowChange["values"].append(changedColValues);
                    }
                }

            valueItor->second["rows"].append(rowChange);
            }

        for (Utf8StringCR tableName : tablesAsOrderedInChangeset)
            {
            auto it = changedValuesPerTableMap.find(tableName);
            if (it == changedValuesPerTableMap.end())
                return JsonValue();

            json.m_value.append(it->second);
            }

        return json;
        }
    };

//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   12/16
//=======================================================================================
struct TestChangeTracker : BeSQLite::ChangeTracker
    {
    TestChangeTracker(BeSQLite::DbR db) { SetDb(&db); }

    OnCommitStatus _OnCommit(bool isCommit, Utf8CP operation) override { return OnCommitStatus::Continue; }
    };


//=======================================================================================
// @bsiclass                                               Krischan.Eberle  11/17
//=======================================================================================
struct ChangeSummaryTestFixture : public ECDbTestFixture
    {
    protected:
        DbResult AttachCache() const { return m_ecdb.AttachChangeCache(ECDb::GetDefaultChangeCachePath(m_ecdb.GetDbFileName())); }

        int GetInstanceChangeCount(ECInstanceId changeSummaryId) const
            {
            ECSqlStatement stmt;
            if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, "SELECT count(*) FROM change.InstanceChange WHERE Summary.Id=?"))
                return -1;

            stmt.BindId(1, changeSummaryId);

            if (stmt.Step() != BE_SQLITE_ROW)
                return -1;

            return stmt.GetValueInt(0);
            }

        int GetPropertyValueChangeCount(ECInstanceId instanceChangeId = ECInstanceId()) const
            {
            Utf8CP ecsql = instanceChangeId.IsValid() ? "SELECT count(*) FROM change.PropertyValueChange WHERE InstanceChange.Id=?" :
                "SELECT count(*) FROM change.PropertyValueChange";
            ECSqlStatement stmt;
            if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, ecsql))
                return -1;

            stmt.BindId(1, instanceChangeId);

            if (stmt.Step() != BE_SQLITE_ROW)
                return -1;

            return stmt.GetValueInt(0);
            }

        bool ContainsChange(ECInstanceId changeSummaryId, ECInstanceId changedInstanceId, Utf8CP changedInstanceSchemaName, Utf8CP changedInstanceClassName, ChangeOpCode opCode) const
            {
            ECClassId classIdOfChangedInstance = m_ecdb.Schemas().GetClassId(changedInstanceSchemaName, changedInstanceClassName);
            if (!classIdOfChangedInstance.IsValid())
                return false;

            ECSqlStatement stmt;
            if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, "SELECT 1 FROM change.InstanceChange WHERE Summary.Id=? AND ChangedInstance.Id=? AND ChangedInstance.ClassId=? AND OpCode=?"))
                return false;

            stmt.BindId(1, changeSummaryId);
            stmt.BindId(2, changedInstanceId);
            stmt.BindId(3, classIdOfChangedInstance);
            stmt.BindInt(4, (int) opCode);

            return stmt.Step() == BE_SQLITE_ROW;
            }

    void DumpChangeSummary(ECInstanceKey const& changeSummaryKey, Utf8CP label) const
        {
#ifdef DUMP_CHANGE_SUMMARY
        printf("%s\r\n", label);
        printf("-----------------\r\n");

        auto valueToString = [] (IECSqlValue const& val, Utf8CP sqliteType)
            {
            if (val.IsNull())
                return Utf8String("null");

            Utf8String str;

            if (BeStringUtilities::StricmpAscii("integer", sqliteType) == 0)
                str.Sprintf("%" PRIi64, val.GetInt64());
            else if (BeStringUtilities::StricmpAscii("real", sqliteType) == 0)
                str.Sprintf("%.6f", val.GetDouble());
            else if (BeStringUtilities::StricmpAscii("text", sqliteType) == 0)
                str.Sprintf("%s", val.GetText());
            else if (BeStringUtilities::StricmpAscii("blob", sqliteType) == 0)
                {
                int blobSize = -1;
                val.GetBlob(&blobSize);
                str.Sprintf("blob(%d bytes)", blobSize);
                }

            return str;
            };

        ECSqlStatement instanceChangeStmt;
        ASSERT_EQ(ECSqlStatus::Success, instanceChangeStmt.Prepare(m_ecdb, "SELECT ECInstanceId, ChangedInstance.Id, ChangedInstance.ClassId, OpCode, IsIndirect FROM change.InstanceChange WHERE Summary.Id=?"));
        instanceChangeStmt.BindId(1, changeSummaryKey.GetInstanceId();

        ECSqlStatement propValueChangeStmt;
        ASSERT_EQ(ECSqlStatus::Success, propValueChangeStmt.Prepare(m_ecdb, "SELECT AccessString,RawOldValue,RawNewValue,TYPEOF(RawOldValue),TYPEOF(RawNewValue) FROM change.PropertyValueChange WHERE InstanceChange.Id=?"));

        printf("ChangeSummary (Id: %s, ClassId: %s\r\n)", changeSummaryKey.GetInstanceId().ToString().c_str(), changeSummaryKey.GetClassId().ToString().c_str());
        printf("-----------------------------------\r\n");
        printf("ChangedInstance Id|ChangedInstance Class|ChangedInstance ClassId|OpCode|IsIndirect\r\n");

        while (BE_SQLITE_ROW == instanceChangeStmt.Step())
            {
            ECInstanceId changeId = instanceChangeStmt.GetValueId<ECInstanceId>(0);

            ECClassCP ecClass = m_ecdb.Schemas().GetClass(instanceChangeStmt.GetValueId<ECClassId>(2));
            ASSERT_TRUE(ecClass != nullptr) << "ECClassId: " << instanceChangeStmt.GetValueId<ECClassId>(2).ToString();

            printf("%s|%s|%s|%s|%s\r\n", 
                   instanceChangeStmt.GetValueId<ECInstanceId>(1).ToString().c_str(),
                   ecClass->GetFullName(), ecClass->GetId().ToString().c_str(), 
                   ChangedOpCodeToString((ChangeOpCode) instanceChangeStmt.GetValueInt(3)), 
                   instanceChangeStmt.GetValueBoolean(4) ? "yes" : "no");


            propValueChangeStmt.BindId(1, changeId);

            printf("\tAccessString|OldValue|NewValue\r\n");

            while (BE_SQLITE_ROW == propValueChangeStmt.Step())
                {
                Utf8CP accessString = propValueChangeStmt.GetValueText(0);

                IECSqlValue const& oldVal = propValueChangeStmt.GetValue(1);
                IECSqlValue const& newVal = propValueChangeStmt.GetValue(2);
                Utf8CP oldValueType = propValueChangeStmt.GetValueText(3);
                Utf8CP newValueType = propValueChangeStmt.GetValueText(4);

                Utf8String oldValStr = valueToString(oldVal, oldValueType);
                Utf8String newValStr = valueToString(newVal, newValueType);

                printf("\t%s|%s|%s\r\n", accessString, oldValStr.c_str(), newValStr.c_str());
                }

            propValueChangeStmt.Reset();
            propValueChangeStmt.ClearBindings();
            }
#endif
        }

    static Utf8CP ChangedOpCodeToString(ChangeOpCode opCode)
        {
        switch (opCode)
            {
                case ChangeOpCode::Insert:
                    return "Insert";
                case ChangeOpCode::Update:
                    return "Update";
                case ChangeOpCode::Delete:
                    return "Delete";

                default:
                    BeAssert(false);
                    return "invalid ChangedOpCode";
            }
        }
    };

//=======================================================================================
//!Test fixture for the deprecated ChangeSummary API
// @bsiclass                                                 Ramanujam.Raman   12/16
//=======================================================================================
struct ChangeSummaryTestFixtureV1 : public ECDbTestFixture
    {
    protected:
        void DumpChangeSummary(ChangeSummary const& changeSummary, Utf8CP label)
            {
#ifdef DUMP_CHANGE_SUMMARY
            printf("\t%s:\n", label);
            changeSummary.Dump();
#endif
            }

        bool ChangeSummaryContainsInstance(ECDbCR ecdb, ChangeSummary const& changeSummary, ECInstanceId instanceId, Utf8CP schemaName, Utf8CP className, DbOpcode dbOpcode)
            {
            Utf8String tableName = changeSummary.GetInstancesTableName();
            ECClassId classId = ecdb.Schemas().GetClassId(schemaName, className);

            Utf8PrintfString sql("SELECT NULL FROM %s WHERE ClassId=? AND InstanceId=? AND DbOpcode=?", tableName.c_str());
            CachedStatementPtr statement = ecdb.GetCachedStatement(sql.c_str());
            BeAssert(statement.IsValid());

            statement->BindId(1, classId);
            statement->BindId(2, instanceId);
            statement->BindInt(3, (int) dbOpcode);

            DbResult result = statement->Step();
            return (result == BE_SQLITE_ROW);
            }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                  11/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, SchemaAndApiConsistency)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("changesummary_schemaandapiconsistency.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, AttachCache());

    //verify that the expected change summary cache file alias
    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "pragma database_list"));
    int attachedTableSpaceCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        Utf8CP tableSpaceName = stmt.GetValueText(1);
        if (BeStringUtilities::StricmpAscii("main", tableSpaceName) == 0 || BeStringUtilities::StricmpAscii("temp", tableSpaceName) == 0)
            continue;

        attachedTableSpaceCount++;
        ASSERT_STREQ("ecchange", tableSpaceName);
       }
    ASSERT_EQ(1, attachedTableSpaceCount) << "Only ecchange table space is expected to be attached";
    }

    ECEnumeration const* opCodeECEnum = m_ecdb.Schemas().GetEnumeration("ECDbChange", "OpCode");
    ASSERT_TRUE(opCodeECEnum != nullptr);

    ASSERT_EQ(3, (int) opCodeECEnum->GetEnumeratorCount());
    ASSERT_TRUE(opCodeECEnum->GetIsStrict());
    ASSERT_EQ(PRIMITIVETYPE_Integer, opCodeECEnum->GetType());

    for (ECEnumerator const* opCodeECEnumVal : opCodeECEnum->GetEnumerators())
        {
        if (opCodeECEnumVal->GetDisplayLabel().EqualsIAscii("insert"))
            ASSERT_EQ((int) ChangeOpCode::Insert, opCodeECEnumVal->GetInteger()) << opCodeECEnumVal->GetDisplayLabel();
        else if (opCodeECEnumVal->GetDisplayLabel().EqualsIAscii("update"))
            ASSERT_EQ((int) ChangeOpCode::Update, opCodeECEnumVal->GetInteger()) << opCodeECEnumVal->GetDisplayLabel();
        else if (opCodeECEnumVal->GetDisplayLabel().EqualsIAscii("delete"))
            ASSERT_EQ((int) ChangeOpCode::Delete, opCodeECEnumVal->GetInteger()) << opCodeECEnumVal->GetDisplayLabel();
        else
            FAIL() << opCodeECEnumVal->GetDisplayLabel() << " : " << opCodeECEnumVal->GetInteger();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                             1/18
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, sqlite_stat1)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("change_statistic1.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECEntityClass typeName="Foo1">
                <ECProperty propertyName="S" typeName="string" />
                <ECProperty propertyName="I" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Foo2">
                <ECProperty propertyName="Dt" typeName="dateTime" />
                <ECProperty propertyName="Origin" typeName="Point2d" />
            </ECEntityClass>
        </ECSchema>)xml")));
    
    ReopenECDb();
    //Enusre only on system table exist by default
    ASSERT_TRUE(m_ecdb.TableExists("sqlite_stat1"));
    ASSERT_FALSE(m_ecdb.TableExists("sqlite_stat2"));
    ASSERT_FALSE(m_ecdb.TableExists("sqlite_stat3"));
    ASSERT_FALSE(m_ecdb.TableExists("sqlite_stat4"));
    //and that table is empty by default
    {
    auto stmt = m_ecdb.GetCachedStatement("SELECT COUNT(*) FROM sqlite_stat1");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(0, stmt->GetValueInt(0));
    }
    //Make sure change is that system table can be tracked.
    TestChangeSet changeset1;
    {
    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    m_ecdb.ExecuteSql("analyze");
    tracker.EnableTracking(false);
    //Make we captured some changes
    ASSERT_TRUE(tracker.HasChanges());
    ASSERT_EQ(BE_SQLITE_OK, changeset1.FromChangeTrack(tracker)); 
    }

    m_ecdb.CloseDb();
    ASSERT_EQ(SUCCESS, SetupECDb("change_statistic2.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECEntityClass typeName="Foo1">
                <ECProperty propertyName="S" typeName="string" />
                <ECProperty propertyName="I" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Foo2">
                <ECProperty propertyName="Dt" typeName="dateTime" />
                <ECProperty propertyName="Origin" typeName="Point2d" />
            </ECEntityClass>
        </ECSchema>)xml")));

    {
    auto stmt = m_ecdb.GetCachedStatement("SELECT COUNT(*) FROM sqlite_stat1");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(0, stmt->GetValueInt(0));
    }
    //printf("\n%s\n", changeset1.ToJson(m_ecdb).ToString().c_str());

    //apply the change set to a new db
    ASSERT_EQ(BE_SQLITE_OK, changeset1.ApplyChanges(m_ecdb));
    m_ecdb.SaveChanges();
    {
    auto stmt = m_ecdb.GetCachedStatement("SELECT COUNT(*) FROM sqlite_stat1");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(48, stmt->GetValueInt(0));
    }

    //printf("%s", changeset1.ToJson(m_ecdb).ToString().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                  05/18
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, ChangesFunctionOptimizations)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ChangesFunctionOptimizations.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECEntityClass typeName="Foo">
                <ECProperty propertyName="I" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml")));
    ASSERT_EQ(BE_SQLITE_OK, AttachCache());

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.Foo(I) VALUES(123)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.Foo(I) VALUES(222)"));

    TestChangeSet changeset1;
    ASSERT_EQ(BE_SQLITE_OK, changeset1.FromChangeTrack(tracker));

    ECInstanceKey summary1Key;
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(summary1Key, ChangeSetArg(changeset1)));

    tracker.Restart();

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("UPDATE ts.Foo SET I=124 WHERE I=123"));

    TestChangeSet changeset2;
    ASSERT_EQ(BE_SQLITE_OK, changeset2.FromChangeTrack(tracker));

    ECInstanceKey summary2Key;
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(summary2Key, ChangeSetArg(changeset2)));

    tracker.Restart();

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("DELETE FROM ts.Foo WHERE I=222"));

    TestChangeSet changeset3;
    ASSERT_EQ(BE_SQLITE_OK, changeset3.FromChangeTrack(tracker));

    ECInstanceKey summary3Key;
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(summary3Key, ChangeSetArg(changeset3)));

    tracker.EndTracking();
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());

    //unoptimized case
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT I FROM ts.Foo.Changes(?,?)"));
    Utf8String nativeSql(stmt.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("LEFT JOIN")) << stmt.GetECSql() << " Native SQL: " << nativeSql;
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT I FROM ts.Foo.Changes(?,'BeforeUpdate')"));
    nativeSql.assign(stmt.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("LEFT JOIN")) << stmt.GetECSql() << " Native SQL: " << nativeSql;
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT I FROM ts.Foo.Changes(?,'AfterUpdate')"));
    nativeSql.assign(stmt.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("LEFT JOIN")) << stmt.GetECSql() << " Native SQL: " << nativeSql;
    stmt.Finalize();

    //optimized case
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT I FROM ts.Foo.Changes(?,'AfterInsert')"));
    nativeSql.assign(stmt.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("JOIN")) << stmt.GetECSql() << " Native SQL: " << nativeSql;
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, Utf8PrintfString("SELECT I FROM ts.Foo.Changes(?,%d)", (int) ChangedValueState::AfterInsert).c_str()));
    nativeSql.assign(stmt.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("JOIN")) << stmt.GetECSql() << " Native SQL: " << nativeSql;
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT I FROM ts.Foo.Changes(?,'BeforeDelete')"));
    nativeSql.assign(stmt.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("JOIN")) << stmt.GetECSql() << " Native SQL: " << nativeSql;
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, Utf8PrintfString("SELECT I FROM ts.Foo.Changes(?,%d)", (int) ChangedValueState::BeforeDelete).c_str()));
    nativeSql.assign(stmt.GetNativeSql());
    ASSERT_FALSE(nativeSql.ContainsI("JOIN")) << stmt.GetECSql() << " Native SQL: " << nativeSql;
    stmt.Finalize();

    //not yet optimized cases
    Utf8String summary2IdStr = summary2Key.GetInstanceId().ToString();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, Utf8PrintfString("SELECT I FROM ts.Foo.Changes(%s,'BeforeUpdate')", summary2IdStr.c_str()).c_str()));
    nativeSql.assign(stmt.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("LEFT JOIN")) << stmt.GetECSql() << " Native SQL: " << nativeSql;
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, Utf8PrintfString("SELECT I FROM ts.Foo.Changes(%s,%d)", summary2IdStr.c_str(), (int) ChangedValueState::BeforeUpdate).c_str()));
    nativeSql.assign(stmt.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("LEFT JOIN")) << stmt.GetECSql() << " Native SQL: " << nativeSql;
    stmt.Finalize();
 
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, Utf8PrintfString("SELECT I FROM ts.Foo.Changes(%s,'AfterUpdate')", summary2IdStr.c_str()).c_str()));
    nativeSql.assign(stmt.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("LEFT JOIN")) << stmt.GetECSql() << " Native SQL: " << nativeSql;
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, Utf8PrintfString("SELECT I FROM ts.Foo.Changes(%s,%d)", summary2IdStr.c_str(), (int) ChangedValueState::AfterUpdate).c_str()));
    nativeSql.assign(stmt.GetNativeSql());
    ASSERT_TRUE(nativeSql.ContainsI("LEFT JOIN")) << stmt.GetECSql() << " Native SQL: " << nativeSql;
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                  12/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, ChangesFunctionInput)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ChangesFunctionInput.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECEntityClass typeName="Foo1">
                <ECProperty propertyName="S" typeName="string" />
                <ECProperty propertyName="I" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Foo2">
                <ECProperty propertyName="Dt" typeName="dateTime" />
                <ECProperty propertyName="Origin" typeName="Point2d" />
            </ECEntityClass>
        </ECSchema>)xml")));
    ASSERT_EQ(BE_SQLITE_OK, AttachCache());

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.Foo1(S,I) VALUES('hello',123)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.Foo2(Dt,Origin.X,Origin.Y) VALUES(CURRENT_TIMESTAMP,1.0,1.0)"));

    TestChangeSet changeset1;
    ASSERT_EQ(BE_SQLITE_OK, changeset1.FromChangeTrack(tracker));

    ECInstanceKey summary1Key;
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(summary1Key, ChangeSetArg(changeset1)));

    tracker.Restart();

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("UPDATE ts.Foo1 SET I=124"));

    TestChangeSet changeset2;
    ASSERT_EQ(BE_SQLITE_OK, changeset2.FromChangeTrack(tracker));

    ECInstanceKey summary2Key;
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(summary2Key, ChangeSetArg(changeset2)));

    tracker.Restart();

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("DELETE FROM ts.Foo2"));

    TestChangeSet changeset3;
    ASSERT_EQ(BE_SQLITE_OK, changeset3.FromChangeTrack(tracker));

    ECInstanceKey summary3Key;
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(summary3Key, ChangeSetArg(changeset3)));

    tracker.EndTracking();
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());

    ECSqlStatement foo1ChangesStmt, foo2ChangesStmt;
    ASSERT_EQ(ECSqlStatus::Success, foo1ChangesStmt.Prepare(m_ecdb, "SELECT * FROM ts.Foo1.Changes(?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, foo2ChangesStmt.Prepare(m_ecdb, "SELECT * FROM ts.Foo2.Changes(?,?)"));

    std::vector<ChangedValueState> changedValueStates {ChangedValueState::AfterInsert, ChangedValueState::BeforeUpdate, ChangedValueState::AfterUpdate, ChangedValueState::BeforeDelete};
    std::vector<Utf8CP> changedValueStateStrings {"AfterInsert", "BeforeUpdate", "AfterUpdate", "BeforeDelete"};

    for (ChangedValueState state : changedValueStates)
        {
        //summary id unbound
        ASSERT_EQ(ECSqlStatus::Success, foo1ChangesStmt.BindInt(2, (int) state));
        ASSERT_EQ(BE_SQLITE_DONE, foo1ChangesStmt.Step());
        foo1ChangesStmt.Reset();
        foo1ChangesStmt.ClearBindings();
        ASSERT_EQ(ECSqlStatus::Success, foo2ChangesStmt.BindInt(2, (int) state));
        ASSERT_EQ(BE_SQLITE_DONE, foo2ChangesStmt.Step());
        foo2ChangesStmt.Reset();
        foo2ChangesStmt.ClearBindings();

        //summary id which does not exist
        ASSERT_EQ(ECSqlStatus::Success, foo1ChangesStmt.BindId(1, ECInstanceId((uint64_t) 1111111111)));
        ASSERT_EQ(ECSqlStatus::Success, foo1ChangesStmt.BindInt(2, (int) state));
        ASSERT_EQ(BE_SQLITE_DONE, foo1ChangesStmt.Step());
        foo1ChangesStmt.Reset();
        foo1ChangesStmt.ClearBindings();
        ASSERT_EQ(ECSqlStatus::Success, foo2ChangesStmt.BindId(1, ECInstanceId((uint64_t) 1111111111)));
        ASSERT_EQ(ECSqlStatus::Success, foo2ChangesStmt.BindInt(2, (int) state));
        ASSERT_EQ(BE_SQLITE_DONE, foo2ChangesStmt.Step());
        foo2ChangesStmt.Reset();
        foo2ChangesStmt.ClearBindings();
        }

    for (Utf8CP state : changedValueStateStrings)
        {
        //summary id unbound
        ASSERT_EQ(ECSqlStatus::Success, foo1ChangesStmt.BindText(2, state, IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(BE_SQLITE_DONE, foo1ChangesStmt.Step());
        foo1ChangesStmt.Reset();
        foo1ChangesStmt.ClearBindings();
        ASSERT_EQ(ECSqlStatus::Success, foo2ChangesStmt.BindText(2, state, IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(BE_SQLITE_DONE, foo2ChangesStmt.Step());
        foo2ChangesStmt.Reset();
        foo2ChangesStmt.ClearBindings();

        //summary id which does not exist
        ASSERT_EQ(ECSqlStatus::Success, foo1ChangesStmt.BindId(1, ECInstanceId((uint64_t) 1111111111)));
        ASSERT_EQ(ECSqlStatus::Success, foo1ChangesStmt.BindText(2, state, IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(BE_SQLITE_DONE, foo1ChangesStmt.Step());
        foo1ChangesStmt.Reset();
        foo1ChangesStmt.ClearBindings();
        ASSERT_EQ(ECSqlStatus::Success, foo2ChangesStmt.BindId(1, ECInstanceId((uint64_t) 1111111111)));
        ASSERT_EQ(ECSqlStatus::Success, foo2ChangesStmt.BindText(2, state, IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(BE_SQLITE_DONE, foo2ChangesStmt.Step());
        foo2ChangesStmt.Reset();
        foo2ChangesStmt.ClearBindings();
        }

    //Invalid states
    //first try with a valid state to ensure that the statement does return something with correct input
    ASSERT_EQ(ECSqlStatus::Success, foo1ChangesStmt.BindId(1, summary1Key.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, foo1ChangesStmt.BindInt(2, (int) ChangedValueState::AfterInsert));
    ASSERT_EQ(BE_SQLITE_ROW, foo1ChangesStmt.Step());
    foo1ChangesStmt.Reset();
    foo1ChangesStmt.ClearBindings();

    //first try with a valid state to ensure that the statement does return something with correct input
    ASSERT_EQ(ECSqlStatus::Success, foo1ChangesStmt.BindId(1, summary1Key.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, foo1ChangesStmt.BindText(2, "AfterInsert", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, foo1ChangesStmt.Step());
    foo1ChangesStmt.Reset();
    foo1ChangesStmt.ClearBindings();

    //now try invalid states
    ASSERT_EQ(ECSqlStatus::Success, foo1ChangesStmt.BindId(1, summary1Key.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, foo1ChangesStmt.BindInt(2, 5555555));
    ASSERT_EQ(BE_SQLITE_ERROR, foo1ChangesStmt.Step());
    foo1ChangesStmt.Reset();
    foo1ChangesStmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, foo1ChangesStmt.BindId(1, summary1Key.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, foo1ChangesStmt.BindText(2, "Insert", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ERROR, foo1ChangesStmt.Step());
    foo1ChangesStmt.Reset();
    foo1ChangesStmt.ClearBindings();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                  12/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, ChangesFunctionOnlyForSelect)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ChangesFunctionOnlyForSelect.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECEntityClass typeName="Foo">
                <ECProperty propertyName="S" typeName="string" />
                <ECProperty propertyName="I" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml")));
    ASSERT_EQ(BE_SQLITE_OK, AttachCache());

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.Foo(S,I) VALUES('hello',123)"));

    TestChangeSet changeset;
    ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));
    tracker.EndTracking();

    ECInstanceKey summaryKey;
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(summaryKey, ChangeSetArg(changeset)));
    Utf8String summaryId = summaryKey.GetInstanceId().ToString();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, Utf8PrintfString("SELECT * FROM ts.Foo.Changes(%s,'AfterInsert')", summaryId.c_str()).c_str()));
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();

    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, Utf8PrintfString("INSERT INTO ts.Foo.Changes(%s,'AfterInsert') VALUES(?,?)", summaryId.c_str()).c_str()));
    stmt.Finalize();

    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "INSERT INTO ts.Foo.Changes(I) VALUES(?)"));
    stmt.Finalize();

    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "INSERT INTO main.ts.Foo.Changes(I) VALUES(?)"));
    stmt.Finalize();

    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "INSERT INTO ts.Foo.Changes(1,'AfterInsert')(I) VALUES(?)"));
    stmt.Finalize();

    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "INSERT INTO main.ts.Foo.Changes(1,'AfterInsert')(I) VALUES(?)"));
    stmt.Finalize();

    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, Utf8PrintfString("UPDATE ts.Foo.Changes(%s,'AfterInsert') SET I=123", summaryId.c_str()).c_str()));
    stmt.Finalize();

    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, Utf8PrintfString("DELETE FROM ts.Foo.Changes(%s,'AfterInsert')", summaryId.c_str()).c_str()));
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                  11/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, ValidCache_InvalidCache)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ValidCache_InvalidCache.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECEntityClass typeName="Foo1">
                <ECProperty propertyName="S" typeName="string" />
                <ECProperty propertyName="I" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Foo2">
                <ECProperty propertyName="Dt" typeName="dateTime" />
                <ECProperty propertyName="Origin" typeName="Point2d" />
            </ECEntityClass>
        </ECSchema>)xml")));

    auto assertCache = [] (ECDbCR ecdb, bool expectedIsValidCache, Utf8CP assertMessage)
        {
        EXPECT_TRUE(ecdb.IsTransactionActive()) << "Transaction is expected to be restarted after attach/detach";
        EXPECT_EQ(expectedIsValidCache, ecdb.IsChangeCacheAttached()) << assertMessage;
        EXPECT_EQ(expectedIsValidCache, ecdb.Schemas().ContainsSchema("ECDbChange")) << assertMessage;
        EXPECT_EQ(expectedIsValidCache, ecdb.Schemas().ContainsSchema("ECDbChange", SchemaLookupMode::ByName, "ecchange")) << assertMessage;
        EXPECT_EQ(expectedIsValidCache, ecdb.Schemas().GetClass("ECDbChange", "ChangeSummary") != nullptr) << assertMessage;
        EXPECT_EQ(expectedIsValidCache, ecdb.Schemas().GetClass("ECDbChange", "ChangeSummary", SchemaLookupMode::ByName, "ecchange") != nullptr) << assertMessage;
        EXPECT_EQ(expectedIsValidCache, ecdb.Schemas().GetEnumeration("ECDbChange", "OpCode") != nullptr) << assertMessage;
        EXPECT_EQ(expectedIsValidCache, ecdb.Schemas().GetEnumeration("ECDbChange", "OpCode", SchemaLookupMode::ByName, "ecchange") != nullptr) << assertMessage;

        ECSqlStatement stmt;
        if (expectedIsValidCache)
            EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT * FROM change.InstanceChange")) << assertMessage;
        else
            EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(ecdb, "SELECT * FROM change.InstanceChange")) << assertMessage;

        stmt.Finalize();

        if (expectedIsValidCache)
            EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT * FROM ecchange.change.InstanceChange")) << assertMessage;
        else
            EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(ecdb, "SELECT * FROM ecchange.change.InstanceChange")) << assertMessage;

        stmt.Finalize();

        if (expectedIsValidCache)
            EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT * FROM ts.Foo1.Changes(1,'AfterInsert')")) << assertMessage;
        else
            EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(ecdb, "SELECT * FROM ts.Foo1.Changes(1,'AfterInsert')")) << assertMessage;

        stmt.Finalize();
        };

    BeFileName ecdbPath(m_ecdb.GetDbFileName());
    BeFileName cachePath = ECDb::GetDefaultChangeCachePath(m_ecdb.GetDbFileName());

    assertCache(m_ecdb, false, "Cache does not exist and is not attached");
    ASSERT_EQ(BE_SQLITE_OK, AttachCache());
    assertCache(m_ecdb, true, "Cache has been attached");
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.DetachChangeCache());
    assertCache(m_ecdb, false, "Cache has been detached");
    ASSERT_EQ(BE_SQLITE_OK, AttachCache());
    assertCache(m_ecdb, true, "Cache has been reattached");

    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb(ECDb::OpenParams(ECDb::OpenMode::ReadWrite)));
    assertCache(m_ecdb, false, "Opened readwrite");

    CloseECDb();
    ASSERT_EQ(BeFileNameStatus::Success, cachePath.BeDeleteFile());
    ASSERT_EQ(BE_SQLITE_OK, OpenECDb(ecdbPath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite)));
    assertCache(m_ecdb, false, "Opened where cache does not exist");

    CloseECDb();

    //attach cache with plain SQL command
    ASSERT_EQ(BE_SQLITE_OK, OpenECDb(ecdbPath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite, DefaultTxn::No)));
    Savepoint sp(m_ecdb, "");
    {
    ECDb cache;
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.CreateChangeCache(cache, cachePath));
    }
    sp.Cancel();
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.TryExecuteSql(Utf8PrintfString("ATTACH '%s' AS ecchange", cachePath.GetNameUtf8().c_str()).c_str()));
    sp.Begin();
    assertCache(m_ecdb, false, "Attached change cache with SQL command (expected to not be recognized by ECDb)");
    sp.Cancel();
    CloseECDb();
    ASSERT_EQ(BeFileNameStatus::Success, cachePath.BeDeleteFile());

    //create non-Change non-ECDb file
    {
    ASSERT_FALSE(cachePath.DoesPathExist());
    Db db;
    ASSERT_EQ(BE_SQLITE_OK, db.CreateNewDb(cachePath));
    }

    ASSERT_EQ(BE_SQLITE_OK, OpenECDb(ecdbPath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite)));
    {
    ScopedDisableFailOnAssertion failOnAssertion;
    ASSERT_EQ(BE_SQLITE_ERROR, AttachCache()) << "Non-ECDb file with same path exists";
    }
    assertCache(m_ecdb, false, "Attach failed because non-ECDb file with same path exists");
    CloseECDb();
    //create non-Change ECDb file
    {
    ASSERT_EQ(BeFileNameStatus::Success, cachePath.BeDeleteFile());

    ECDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.CreateNewDb(cachePath));
    }

    ASSERT_EQ(BE_SQLITE_OK, OpenECDb(ecdbPath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite)));
    ASSERT_EQ(BE_SQLITE_ERROR, AttachCache()) << "Non-Change ECDb file with same path exists";
    assertCache(m_ecdb, false, "Attach failed because Non-Change ECDb file with same path exists");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                  12/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, CloseClearCacheDestroyWithAttachedCache)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("CloseClearCacheDestroyWithAttachedCache.ecdb"));
    BeFileName ecdbPath(m_ecdb.GetDbFileName());
    BeFileName cachePath = ECDb::GetDefaultChangeCachePath(m_ecdb.GetDbFileName());

    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AttachChangeCache(cachePath));
    ASSERT_TRUE(m_ecdb.IsTransactionActive()) << "Transaction is expected to be restarted after attach/detach";
    ASSERT_TRUE(m_ecdb.Schemas().GetClass("ECDbFileInfo", "ExternalFileInfo") != nullptr);
    ASSERT_TRUE(m_ecdb.Schemas().GetClass("ECDbChange", "InstanceChange") != nullptr);
    m_ecdb.CloseDb();
    ASSERT_EQ(BeFileNameStatus::Success, ecdbPath.BeDeleteFile());
    ASSERT_EQ(BeFileNameStatus::Success, cachePath.BeDeleteFile());

    {
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.CreateNewDb(ecdbPath));
    ASSERT_EQ(BE_SQLITE_OK, ecdb.AttachChangeCache(cachePath));
    ASSERT_TRUE(ecdb.IsTransactionActive()) << "Transaction is expected to be restarted after attach/detach";
    ASSERT_TRUE(ecdb.Schemas().GetClass("ECDbFileInfo", "ExternalFileInfo") != nullptr);
    ASSERT_TRUE(ecdb.Schemas().GetClass("ECDbChange", "InstanceChange") != nullptr);
    }
    ASSERT_EQ(BeFileNameStatus::Success, ecdbPath.BeDeleteFile());
    ASSERT_EQ(BeFileNameStatus::Success, cachePath.BeDeleteFile());

    {
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.CreateNewDb(ecdbPath));
    ASSERT_EQ(BE_SQLITE_OK, ecdb.AttachChangeCache(cachePath));
    ASSERT_TRUE(ecdb.IsTransactionActive()) << "Transaction is expected to be restarted after attach/detach";
    ASSERT_TRUE(ecdb.Schemas().GetClass("ECDbFileInfo", "ExternalFileInfo") != nullptr);
    ASSERT_TRUE(ecdb.Schemas().GetClass("ECDbChange", "InstanceChange") != nullptr);
    ecdb.ClearECDbCache();
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                  01/18
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, AttachChangeCacheMethodInput)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("AttachChangeCacheMethodInput.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECEntityClass typeName="Foo1">
                <ECProperty propertyName="S" typeName="string" />
                <ECProperty propertyName="I" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Foo2">
                <ECProperty propertyName="Dt" typeName="dateTime" />
                <ECProperty propertyName="Origin" typeName="Point2d" />
            </ECEntityClass>
        </ECSchema>)xml")));

    BeFileName cachePath = ECDb::GetDefaultChangeCachePath(m_ecdb.GetDbFileName());

    ASSERT_FALSE(m_ecdb.IsChangeCacheAttached());

    ASSERT_EQ(BE_SQLITE_ERROR, m_ecdb.AttachChangeCache(BeFileName())) << "Change cache path must not be empty";
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AttachChangeCache(cachePath));
    ASSERT_TRUE(m_ecdb.IsTransactionActive()) << "Transaction is expected to be restarted after attach/detach";
    ASSERT_TRUE(m_ecdb.IsChangeCacheAttached());

    {
    ECDb cacheFile;
    ASSERT_EQ(BE_SQLITE_ERROR, m_ecdb.CreateChangeCache(cacheFile, cachePath)) << "Cache already exists";
    }

    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb(ECDb::OpenParams(ECDb::OpenMode::Readonly)));
    ASSERT_FALSE(m_ecdb.IsChangeCacheAttached());

    {
    ASSERT_EQ(BeFileNameStatus::Success, cachePath.BeDeleteFile());
    ECDb cacheFile;
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.CreateChangeCache(cacheFile, cachePath));
    }

    ASSERT_FALSE(m_ecdb.IsChangeCacheAttached());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AttachChangeCache(cachePath));
    ASSERT_TRUE(m_ecdb.IsTransactionActive()) << "Transaction is expected to be restarted after attach/detach";
    ASSERT_TRUE(m_ecdb.IsChangeCacheAttached());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                  01/18
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, IsChangeCacheAttached)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("IsChangeCacheAttached.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECEntityClass typeName="Foo1">
                <ECProperty propertyName="S" typeName="string" />
                <ECProperty propertyName="I" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml")));

    ASSERT_FALSE(m_ecdb.IsChangeCacheAttached());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                  12/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, InMemoryPrimaryECDb)
    {
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.CreateNewDb(":memory:"));
    TestHelper helper(m_ecdb);

    ASSERT_EQ(SUCCESS, helper.ImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECEntityClass typeName="Foo1">
                <ECProperty propertyName="S" typeName="string" />
                <ECProperty propertyName="I" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Foo2">
                <ECProperty propertyName="Dt" typeName="dateTime" />
                <ECProperty propertyName="Origin" typeName="Point2d" />
            </ECEntityClass>
        </ECSchema>)xml")));

    ASSERT_EQ(BE_SQLITE_ERROR, AttachCache()) << "cannot create a change cache for an in-memory primary file";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                  12/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, NonDefaultCachePath)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("ChangeSummaryNonDefaultCachePath.ecdb"));

    BeFileName ecdbPath(m_ecdb.GetDbFileName());

    BeFileName cacheFilePath = ecdbPath.GetDirectoryName();
    cacheFilePath.AppendToPath(L"mycachefolder");
    
    if (!cacheFilePath.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(cacheFilePath.GetName()));

    cacheFilePath.AppendToPath(L"ChangeSummaryNonDefaultCachePath.mycachefile");
    if (cacheFilePath.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, cacheFilePath.BeDeleteFile());

    ECDb cacheFile;
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.CreateChangeCache(cacheFile, cacheFilePath));
    cacheFile.CloseDb();

    ASSERT_FALSE(m_ecdb.IsChangeCacheAttached());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AttachChangeCache(cacheFilePath));
    ASSERT_TRUE(m_ecdb.IsChangeCacheAttached());
    ASSERT_EQ(BE_SQLITE_ERROR, m_ecdb.AttachChangeCache(cacheFilePath)) << "already attached";
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.DetachChangeCache());

    ASSERT_EQ(BE_SQLITE_OK, AttachCache()) << "No cache at default location -> another cache is created";
    ASSERT_TRUE(m_ecdb.IsChangeCacheAttached());
    ASSERT_EQ(BE_SQLITE_ERROR, AttachCache()) << "already attached";

    ASSERT_EQ(BeFileNameStatus::Success, cacheFilePath.BeDeleteFile());

    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb(ECDb::OpenParams(ECDb::OpenMode::Readonly)));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AttachChangeCache(cacheFilePath));
    ASSERT_TRUE(m_ecdb.IsChangeCacheAttached());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                  12/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, ChangeSummaryExtendedProps)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("ChangeSummaryExtendedProps.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, AttachCache());

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key, "INSERT INTO ecdbf.ExternalFileInfo(Name) VALUES('MyLogFile')"));
    ASSERT_TRUE(tracker.HasChanges());
    TestChangeSet changeset;
    ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));
    tracker.EndTracking();

    std::vector<ChangeSetArg> args {ChangeSetArg(changeset),
        ChangeSetArg(changeset, R"({"ChangeSetId":"1-0-0-1"})"),
        ChangeSetArg(changeset, R"({"ChangeSetId":"1-0-0-2", "ParentChangeSetId":"2-0-0-1", "PushDate" : "2017-12-15T12:24Z"})"),
        ChangeSetArg(changeset, R"({"ChangeSetId":"1-0-0-3", "ParentChangeSetId":"2-0-0-2", "PushDate" : "2017-12-15T12:24Z", "CreatedBy":{"Id":"5-5-5-5", "EMail":"Audrey Winter"}})")
        };

    std::vector<ECInstanceId> changeSummaryIds;

    for (ChangeSetArg const& arg : args)
        {
        ECInstanceKey changeSummaryKey;
        ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummaryKey, arg)) << arg.GetExtendedPropertiesJson();
        changeSummaryIds.push_back(changeSummaryKey.GetInstanceId());
        }

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ExtendedProperties FROM change.ChangeSummary WHERE ECInstanceId=?"));

    for (size_t i = 0; i < args.size(); i++)
        {
        ChangeSetArg const& arg = args[i];
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, changeSummaryIds[i])) << arg.GetExtendedPropertiesJson();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << arg.GetExtendedPropertiesJson();
        if (arg.GetExtendedPropertiesJson().empty())
            ASSERT_TRUE(stmt.IsValueNull(0));
        else
            ASSERT_STRCASEEQ(arg.GetExtendedPropertiesJson().c_str(), stmt.GetValueText(0));

        stmt.Reset();
        stmt.ClearBindings();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                  12/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, ChangeSummaryWithCustomMetaData)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("ChangeSummaryWithCustomMetaData.ecdb"));

    BeFileName cacheFilePath = ECDb::GetDefaultChangeCachePath(m_ecdb.GetDbFileName());
    ECDb cacheFile;
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.CreateChangeCache(cacheFile, cacheFilePath));
    ASSERT_FALSE(m_ecdb.IsChangeCacheAttached());
    //add a custom schema to the change file
    ASSERT_EQ(SUCCESS, TestHelper(cacheFile).ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
         <ECSchema schemaName="ChangeSets" alias="cset" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
          <ECSchemaReference name="ECDbChange" version="01.00.01" alias="change"/>
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
          <ECEntityClass typeName="ChangeSet" modifier="Sealed">
              <ECNavigationProperty propertyName="Summary" relationshipName="ChangeSummaryExtractedFromChangeSet" direction="Backward"/>
              <ECProperty propertyName="ChangeSetHubId" typeName="string" />
              <ECProperty propertyName="PushDate" typeName="dateTime" >
                <ECCustomAttributes>
                    <DateTimeInfo xmlns="CoreCustomAttributes.01.00">
                        <DateTimeKind>Utc</DateTimeKind>
                    </DateTimeInfo>
                </ECCustomAttributes>
              </ECProperty>
              <ECProperty propertyName="CreatedBy" typeName="string" />
          </ECEntityClass>
          <ECRelationshipClass typeName="ChangeSummaryExtractedFromChangeSet" modifier="Sealed" strength="referencing">
                <Source multiplicity="(0..1)" roleLabel="is extracted from" polymorphic="false">
                    <Class class="change:ChangeSummary"/>
                </Source>
                <Target multiplicity="(0..1)" roleLabel="refers to" polymorphic="false">
                    <Class class="ChangeSet"/>
                </Target>
           </ECRelationshipClass>
         </ECSchema>
             )xml")));

    cacheFile.CloseDb();
    ASSERT_FALSE(m_ecdb.IsChangeCacheAttached());

    //do some changes and create a changeset
    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key, "INSERT INTO ecdbf.ExternalFileInfo(Name) VALUES('MyLogFile')"));
    ASSERT_TRUE(tracker.HasChanges());
    TestChangeSet changeset;
    ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));
    tracker.EndTracking();

    //extract the change summary (with a separately opened cache file so that we can add more information to the extracted change summary)
    ASSERT_EQ(BE_SQLITE_OK, cacheFile.OpenBeSQLiteDb(cacheFilePath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite)));
    ECInstanceKey changeSummaryKey;
    ASSERT_EQ(SUCCESS, ECDb::ExtractChangeSummary(changeSummaryKey, cacheFile, m_ecdb, ChangeSetArg(changeset)));
    ASSERT_FALSE(m_ecdb.IsChangeCacheAttached());

    //now add additional meta data to the change summary
    ECSqlStatement insertChangeSetStmt;
    ASSERT_EQ(ECSqlStatus::Success, insertChangeSetStmt.Prepare(cacheFile, "INSERT INTO cset.ChangeSet(Summary,ChangeSetHubId,PushDate,CreatedBy) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, insertChangeSetStmt.BindNavigationValue(1, changeSummaryKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, insertChangeSetStmt.BindText(2, "ec1efd72621d42a0642bf135bdca409e94a454ed", IECSqlBinder::MakeCopy::No));
    DateTime pushDate(DateTime::Kind::Utc, 2017, 12, 20, 11, 23);
    ASSERT_EQ(ECSqlStatus::Success, insertChangeSetStmt.BindDateTime(3, pushDate));
    ASSERT_EQ(ECSqlStatus::Success, insertChangeSetStmt.BindText(4, "john.smith@acme.com", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, insertChangeSetStmt.Step());
    insertChangeSetStmt.Finalize();
    cacheFile.SaveChanges();
    cacheFile.CloseDb();

    ASSERT_EQ(SUCCESS, m_ecdb.AttachChangeCache(cacheFilePath));
    ASSERT_TRUE(m_ecdb.IsChangeCacheAttached());
    JsonValue csum = GetHelper().ExecuteSelectECSql("SELECT ECInstanceId,ExtendedProperties FROM change.ChangeSummary");
    ASSERT_EQ(1, csum.m_value.size());
    ASSERT_STREQ(changeSummaryKey.GetInstanceId().ToHexStr().c_str(), csum.m_value[0][ECJsonUtilities::json_id()].asCString());
    ASSERT_FALSE(csum.m_value[0].isMember("ExtendedProperties")) << "Not expected to be populated";

    JsonValue cset = GetHelper().ExecuteSelectECSql("SELECT Summary,ChangeSetHubId,PushDate,CreatedBy FROM cset.ChangeSet");
    ASSERT_EQ(1, cset.m_value.size());
    ASSERT_STREQ(changeSummaryKey.GetInstanceId().ToHexStr().c_str(), cset.m_value[0]["Summary"][ECJsonUtilities::json_id()].asCString());
    ASSERT_STREQ("ec1efd72621d42a0642bf135bdca409e94a454ed", cset.m_value[0]["ChangeSetHubId"].asCString());
    ASSERT_STREQ(pushDate.ToString().c_str(), cset.m_value[0]["PushDate"].asCString());
    ASSERT_STREQ("john.smith@acme.com", cset.m_value[0]["CreatedBy"].asCString());

    ASSERT_EQ(JsonValue("[{\"cnt\": 1}]"), GetHelper().ExecuteSelectECSql("SELECT count(*) cnt FROM change.InstanceChange WHERE OpCode=change.OpCode.[Insert]"));
    ASSERT_EQ(JsonValue("[{\"cnt\": 0}]"), GetHelper().ExecuteSelectECSql("SELECT count(*) cnt FROM change.InstanceChange WHERE OpCode=change.OpCode.[Update]"));
    ASSERT_EQ(JsonValue("[{\"cnt\": 0}]"), GetHelper().ExecuteSelectECSql("SELECT count(*) cnt FROM change.InstanceChange WHERE OpCode=change.OpCode.[Delete]"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                  11/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, SimpleWorkflow)
    {
    std::map<Utf8String, std::unique_ptr<SchemaItem>> testSchemas;
    testSchemas["default mapping"] = std::make_unique<SchemaItem>(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECEntityClass typeName="NamedElement">
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Person">
                <BaseClass>NamedElement</BaseClass>
                <ECProperty propertyName="Age" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml");

    testSchemas["joined table"] = std::make_unique<SchemaItem>(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="NamedElement">
                 <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00"/>
                    </ECCustomAttributes>
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Person">
                <BaseClass>NamedElement</BaseClass>
                <ECProperty propertyName="Age" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml");

    testSchemas["shared columns"] = std::make_unique<SchemaItem>(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="NamedElement">
                 <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <ShareColumns xmlns="ECDbMap.02.00">
                            <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                        </ShareColumns>
                    </ECCustomAttributes>
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Person">
                <BaseClass>NamedElement</BaseClass>
                <ECProperty propertyName="Age" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml");

    testSchemas["overflow table"] = std::make_unique<SchemaItem>(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="NamedElement">
                 <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <ShareColumns xmlns="ECDbMap.02.00">
                            <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                            <MaxSharedColumnsBeforeOverflow>1</MaxSharedColumnsBeforeOverflow>
                        </ShareColumns>
                    </ECCustomAttributes>
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Person">
                <BaseClass>NamedElement</BaseClass>
                <ECProperty propertyName="Age" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml");

    testSchemas["joined table and shared columns"] = std::make_unique<SchemaItem>(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="NamedElement">
                 <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00"/>
                        <ShareColumns xmlns="ECDbMap.02.00">
                            <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                        </ShareColumns>
                    </ECCustomAttributes>
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Person">
                <BaseClass>NamedElement</BaseClass>
                <ECProperty propertyName="Age" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml");

    testSchemas["joined table and overflow table"] = std::make_unique<SchemaItem>(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="NamedElement">
                 <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00"/>
                        <ShareColumns xmlns="ECDbMap.02.00">
                            <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            <MaxSharedColumnsBeforeOverflow>1</MaxSharedColumnsBeforeOverflow>
                        </ShareColumns>
                    </ECCustomAttributes>
                <ECProperty propertyName="Code" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Person">
                <BaseClass>NamedElement</BaseClass>
                <ECProperty propertyName="Name" typeName="string" />
                <ECProperty propertyName="Age" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml");

    for (auto const& kvPair : testSchemas)
        {
        Utf8StringCR scenario = kvPair.first;
        ASSERT_EQ(SUCCESS, SetupECDb("SimpleWorkflow.ecdb", *kvPair.second)) << scenario;
        ASSERT_EQ(BE_SQLITE_OK, AttachCache()) << scenario;

        TestChangeTracker tracker(m_ecdb);
        tracker.EnableTracking(true);

        ECInstanceKey maryKey, samKey;

        //Changeset 1
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(maryKey, "INSERT INTO ts.Person(Name,Age) VALUES('Mery', 20)")) << scenario;
        ASSERT_TRUE(tracker.HasChanges()) << scenario;
        TestChangeSet changeset1;
        ASSERT_EQ(BE_SQLITE_OK, changeset1.FromChangeTrack(tracker)) << scenario;
        //printf("Changeset 1: %s\r\n", changeset1.ToJson(m_ecdb).ToString().c_str());
        tracker.Restart();
        ECInstanceKey changeSummary1Key;
        ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummary1Key, ChangeSetArg(changeset1))) << scenario;

        //Changeset 2
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("UPDATE ts.Person SET Name='Mary'")) << scenario;
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(samKey, "INSERT INTO ts.Person(Name,Age) VALUES('Sam',30)")) << scenario;
        ASSERT_TRUE(tracker.HasChanges()) << scenario;
        TestChangeSet changeset2;
        ASSERT_EQ(BE_SQLITE_OK, changeset2.FromChangeTrack(tracker)) << scenario;
        //printf("Changeset 2: %s\r\n", changeset2.ToJson(m_ecdb).ToString().c_str());
        ECInstanceKey changeSummary2Key;
        ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummary2Key, ChangeSetArg(changeset2))) << scenario;
        tracker.Restart();

        //Changeset 3
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("DELETE FROM ts.Person WHERE Name='Mary'")) << scenario;
        ASSERT_TRUE(tracker.HasChanges()) << scenario;
        TestChangeSet changeset3;
        ASSERT_EQ(BE_SQLITE_OK, changeset3.FromChangeTrack(tracker)) << scenario;
        tracker.EndTracking();
        //printf("Changeset 3: %s\r\n", changeset3.ToJson(m_ecdb).ToString().c_str());
        ECInstanceKey changeSummary3Key;
        ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummary3Key, ChangeSetArg(changeset3))) << scenario;

        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "className":"ECDbChange.ChangeSummary"}])json", changeSummary1Key.GetInstanceId().ToHexStr().c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,ECClassId FROM change.ChangeSummary WHERE ECInstanceId=%s", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str())) << scenario;


        Utf8String maryIdStr = maryKey.GetInstanceId().ToHexStr();
        Utf8String samIdStr = samKey.GetInstanceId().ToHexStr();

        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"Sam", "Age":30}])json", samIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql("SELECT ECInstanceId,Name,Age FROM ts.Person")) << scenario;

        //Changeset 1
        Utf8String summary1IdStr = changeSummary1Key.GetInstanceId().ToString();
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"Mery", "Age":20}])json", maryIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Age FROM ts.Person.Changes(%s,'AfterInsert')", summary1IdStr.c_str()).c_str())) << scenario;
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Age FROM ts.Person.Changes(%s,'BeforeUpdate')", summary1IdStr.c_str()).c_str())) << scenario;
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Age FROM ts.Person.Changes(%s,'AfterUpdate')", summary1IdStr.c_str()).c_str())) << scenario;
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Age FROM ts.Person.Changes(%s,'BeforeDelete')", summary1IdStr.c_str()).c_str())) << scenario;


        //Changeset 2
        Utf8String summary2IdStr = changeSummary2Key.GetInstanceId().ToString();
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"Sam", "Age":30}])json", samIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Age FROM ts.Person.Changes(%s,'AfterInsert')", summary2IdStr.c_str()).c_str())) << scenario;
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"Mery"}])json", maryIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Age FROM ts.Person.Changes(%s,'BeforeUpdate')", summary2IdStr.c_str()).c_str())) << scenario;
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"Mary"}])json", maryIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Age FROM ts.Person.Changes(%s,'AfterUpdate')", summary2IdStr.c_str()).c_str())) << scenario;
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Age FROM ts.Person.Changes(%s,'BeforeDelete')", summary2IdStr.c_str()).c_str())) << scenario;

        //Changeset 3
        Utf8String summary3IdStr = changeSummary3Key.GetInstanceId().ToString();
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Age FROM ts.Person.Changes(%s,'AfterInsert')", summary3IdStr.c_str()).c_str())) << scenario;
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Age FROM ts.Person.Changes(%s,'BeforeUpdate')", summary3IdStr.c_str()).c_str())) << scenario;
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Age FROM ts.Person.Changes(%s,'AfterUpdate')", summary3IdStr.c_str()).c_str())) << scenario;
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"Mary", "Age":20}])json", maryIdStr.c_str())), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Age FROM ts.Person.Changes(%s,'BeforeDelete')", summary3IdStr.c_str()).c_str())) << scenario;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                  11/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, SimpleWorkflowWithPointProp)
    {
    std::map<Utf8String, std::unique_ptr<SchemaItem>> testSchemas;
    testSchemas["default mapping"] = std::make_unique<SchemaItem>(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECEntityClass typeName="NamedElement">
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="POI">
                <BaseClass>NamedElement</BaseClass>
                <ECProperty propertyName="Location" typeName="Point2d" />
            </ECEntityClass>
        </ECSchema>)xml");

    testSchemas["joined table"] = std::make_unique<SchemaItem>(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="NamedElement">
                 <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00"/>
                    </ECCustomAttributes>
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="POI">
                <BaseClass>NamedElement</BaseClass>
                <ECProperty propertyName="Location" typeName="Point2d" />
            </ECEntityClass>
        </ECSchema>)xml");

    testSchemas["shared columns"] = std::make_unique<SchemaItem>(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="NamedElement">
                 <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <ShareColumns xmlns="ECDbMap.02.00">
                            <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                        </ShareColumns>
                    </ECCustomAttributes>
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="POI">
                <BaseClass>NamedElement</BaseClass>
                <ECProperty propertyName="Location" typeName="Point2d" />
            </ECEntityClass>
        </ECSchema>)xml");

    testSchemas["overflow table"] = std::make_unique<SchemaItem>(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="NamedElement">
                 <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <ShareColumns xmlns="ECDbMap.02.00">
                            <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                            <MaxSharedColumnsBeforeOverflow>1</MaxSharedColumnsBeforeOverflow>
                        </ShareColumns>
                    </ECCustomAttributes>
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="POI">
                <BaseClass>NamedElement</BaseClass>
                <ECProperty propertyName="Location" typeName="Point2d" />
            </ECEntityClass>
        </ECSchema>)xml");

    testSchemas["joined table and shared columns"] = std::make_unique<SchemaItem>(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="NamedElement">
                 <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00"/>
                        <ShareColumns xmlns="ECDbMap.02.00">
                            <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                        </ShareColumns>
                    </ECCustomAttributes>
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="POI">
                <BaseClass>NamedElement</BaseClass>
                <ECProperty propertyName="Location" typeName="Point2d" />
            </ECEntityClass>
        </ECSchema>)xml");

    testSchemas["joined table and overflow table"] = std::make_unique<SchemaItem>(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="NamedElement">
                 <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00"/>
                        <ShareColumns xmlns="ECDbMap.02.00">
                            <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            <MaxSharedColumnsBeforeOverflow>1</MaxSharedColumnsBeforeOverflow>
                        </ShareColumns>
                    </ECCustomAttributes>
                <ECProperty propertyName="Code" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="POI">
                <BaseClass>NamedElement</BaseClass>
                <ECProperty propertyName="Name" typeName="string" />
                <ECProperty propertyName="Location" typeName="Point2d" />
            </ECEntityClass>
        </ECSchema>)xml");

    for (auto const& kvPair : testSchemas)
        {
        Utf8StringCR scenario = kvPair.first;
        ASSERT_EQ(SUCCESS, SetupECDb("SimpleWorkflowWithPointProp.ecdb", *kvPair.second)) << scenario;
        ASSERT_EQ(BE_SQLITE_OK, AttachCache()) << scenario;

        TestChangeTracker tracker(m_ecdb);
        tracker.EnableTracking(true);

        ECInstanceKey hallKey, stationKey, castleKey, lakeKey;

        ECSqlStatement insertStmt;
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.Prepare(m_ecdb, "INSERT INTO ts.POI(Name,Location) VALUES(?,?)")) << scenario;

        //Changeset 1
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindText(1, "City Hall", IECSqlBinder::MakeCopy::No)) << scenario;
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindPoint2d(2, DPoint2d::From(100.0, 100.0))) << scenario;
        ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step(hallKey)) << scenario;
        insertStmt.ClearBindings();
        insertStmt.Reset();
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindText(1, "Station", IECSqlBinder::MakeCopy::No)) << scenario;
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindPoint2d(2, DPoint2d::From(200.0, 200.0))) << scenario;
        ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step(stationKey)) << scenario;
        insertStmt.ClearBindings();
        insertStmt.Reset();
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindText(1, "Castle", IECSqlBinder::MakeCopy::No)) << scenario;
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindPoint2d(2, DPoint2d::From(300.0, 300.0))) << scenario;
        ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step(castleKey)) << scenario;
        insertStmt.Finalize();

        ASSERT_TRUE(tracker.HasChanges()) << scenario;
        TestChangeSet changeset1;
        ASSERT_EQ(BE_SQLITE_OK, changeset1.FromChangeTrack(tracker)) << scenario;
        //printf("Changeset 1: %s\r\n", changeset1.ToJson(m_ecdb).ToString().c_str());
        ECInstanceKey changeSummary1Key;
        ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummary1Key, ChangeSetArg(changeset1))) << scenario;

        //Changeset 2
        tracker.Restart();

        ASSERT_EQ(ECSqlStatus::Success, insertStmt.Prepare(m_ecdb, "INSERT INTO ts.POI(Name,Location) VALUES(?,?)")) << scenario;
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindText(1, "Lake", IECSqlBinder::MakeCopy::No)) << scenario;
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindPoint2d(2, DPoint2d::From(400.0, 400.0))) << scenario;
        ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step(lakeKey)) << scenario;
        insertStmt.Finalize();

        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("DELETE FROM ts.POI WHERE Name='Station'")) << scenario;
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("UPDATE ts.POI SET Name='County Hall', Location.X=150 WHERE Name='City Hall'")) << scenario;
        ASSERT_TRUE(tracker.HasChanges()) << scenario;
        TestChangeSet changeset2;
        ASSERT_EQ(BE_SQLITE_OK, changeset2.FromChangeTrack(tracker)) << scenario;
        //printf("Changeset 2: %s\r\n", changeset2.ToJson(m_ecdb).ToString().c_str());
        ECInstanceKey changeSummary2Key;
        ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummary2Key, ChangeSetArg(changeset2))) << scenario;
        tracker.EndTracking();

        Utf8String hallIdStr = hallKey.GetInstanceId().ToHexStr();
        Utf8String stationIdStr = stationKey.GetInstanceId().ToHexStr();
        Utf8String castleIdStr = castleKey.GetInstanceId().ToHexStr();
        Utf8String lakeIdStr = lakeKey.GetInstanceId().ToHexStr();

        //Current state
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"County Hall", "Location": {"x": 150.0, "y":100.0}},
                                                 {"id":"%s", "Name":"Castle", "Location": {"x": 300.0, "y":300.0}},
                                                 {"id":"%s", "Name":"Lake", "Location": {"x": 400.0, "y":400.0}}])json",
                                             hallIdStr.c_str(), castleIdStr.c_str(), lakeIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql("SELECT ECInstanceId,Name,Location FROM ts.POI")) << scenario;

        //Changeset 1
        Utf8String summary1IdStr = changeSummary1Key.GetInstanceId().ToString();

        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"City Hall", "Location": {"x":100.0, "y":100.0}},
                                                 {"id":"%s", "Name":"Station", "Location": {"x":200.0, "y":200.0}},
                                                 {"id":"%s", "Name":"Castle", "Location": {"x":300.0, "y":300.0}}])json",
                                             hallIdStr.c_str(), stationIdStr.c_str(), castleIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Location FROM ts.POI.Changes(%s,'AfterInsert')", summary1IdStr.c_str()).c_str())) << scenario;
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Location FROM ts.POI.Changes(%s,'BeforeUpdate')", summary1IdStr.c_str()).c_str())) << scenario;
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Location FROM ts.POI.Changes(%s,'AfterUpdate')", summary1IdStr.c_str()).c_str())) << scenario;
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Location FROM ts.POI.Changes(%s,'BeforeDelete')", summary1IdStr.c_str()).c_str())) << scenario;


        //Changeset 2

        //insert

        Utf8String summary2IdStr = changeSummary2Key.GetInstanceId().ToString();
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"Lake", "Location":{"x": 400.0, "y":400.0}}])json", lakeIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Location FROM ts.POI.Changes(%s,'AfterInsert')", summary2IdStr.c_str()).c_str())) << scenario;

        //update where only one component has changed

        //before update:

        //test with plain ECSQL to avoid JSON adapter processing.
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, Utf8PrintfString("SELECT ECInstanceId,Name,Location.X,Location.Y FROM ts.POI.Changes(%s,'BeforeUpdate')", summary2IdStr.c_str()).c_str())) << scenario;
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " Scenario: " << scenario;
        EXPECT_EQ(hallKey.GetInstanceId(), stmt.GetValueId<ECInstanceId>(0)) << stmt.GetECSql() << " Scenario: " << scenario;
        EXPECT_STREQ("City Hall", stmt.GetValueText(1)) << stmt.GetECSql() << " Scenario: " << scenario;
        EXPECT_DOUBLE_EQ(100.0, stmt.GetValueDouble(2)) << stmt.GetECSql() << " Scenario: " << scenario;
        EXPECT_DOUBLE_EQ(100.0, stmt.GetValueDouble(3)) << "Expected to be unchanged and therefore current value must be returned. " << stmt.GetECSql() << " Scenario: " << scenario;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Only one row expected for " << stmt.GetECSql() << " Scenario: " << scenario;
        stmt.Finalize();

        //now with JSON adapter
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"City Hall", "Location":{"x": 100.0, "y":100.0}}])json", hallIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Location FROM ts.POI.Changes(%s,'BeforeUpdate')", summary2IdStr.c_str()).c_str())) << scenario;
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"City Hall", "Location.X":100.0, "Location.Y":100.0}])json", hallIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Location.X,Location.Y FROM ts.POI.Changes(%s,'BeforeUpdate')", summary2IdStr.c_str()).c_str())) << scenario;


        //after update:

        //test with plain ECSQL to avoid JSON adapter processing.
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, Utf8PrintfString("SELECT ECInstanceId,Name,Location.X,Location.Y FROM ts.POI.Changes(%s,'AfterUpdate')", summary2IdStr.c_str()).c_str())) << scenario;
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
        EXPECT_EQ(hallKey.GetInstanceId(), stmt.GetValueId<ECInstanceId>(0)) << stmt.GetECSql() << " Scenario: " << scenario;
        EXPECT_STREQ("County Hall", stmt.GetValueText(1)) << stmt.GetECSql() << " Scenario: " << scenario;
        EXPECT_DOUBLE_EQ(150.0, stmt.GetValueDouble(2)) << stmt.GetECSql() << " Scenario: " << scenario;
        EXPECT_DOUBLE_EQ(100.0, stmt.GetValueDouble(3)) << "Expected to be unchanged and therefore current value must be returned. " << stmt.GetECSql() << " Scenario: " << scenario;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Only one row expected for " << stmt.GetECSql() << " Scenario: " << scenario;
        stmt.Finalize();

        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"County Hall", "Location":{"x": 150.0, "y":100.0}}])json", hallIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Location FROM ts.POI.Changes(%s,'AfterUpdate')", summary2IdStr.c_str()).c_str())) << scenario;
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"County Hall", "Location.X":150.0, "Location.Y":100.0}])json", hallIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Location.X,Location.Y FROM ts.POI.Changes(%s,'AfterUpdate')", summary2IdStr.c_str()).c_str())) << scenario;

        //delete
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"Station", "Location":{"x": 200.0, "y":200.0}}])json", stationIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Location FROM ts.POI.Changes(%s,'BeforeDelete')", summary2IdStr.c_str()).c_str())) << scenario;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                  12/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, SimpleWorkflowWithStructProp)
    {
    std::map<Utf8String, std::unique_ptr<SchemaItem>> testSchemas;
    testSchemas["default mapping"] = std::make_unique<SchemaItem>(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECStructClass typeName="Location" modifier="Sealed">
                <ECProperty propertyName="City" typeName="string" />
                <ECProperty propertyName="Zip" typeName="int" />
            </ECStructClass>
            <ECEntityClass typeName="NamedElement">
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="POI">
                <BaseClass>NamedElement</BaseClass>
                <ECStructProperty propertyName="Address" typeName="Location" />
            </ECEntityClass>
        </ECSchema>)xml");

    testSchemas["joined table"] = std::make_unique<SchemaItem>(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECStructClass typeName="Location" modifier="Sealed">
                <ECProperty propertyName="City" typeName="string" />
                <ECProperty propertyName="Zip" typeName="int" />
            </ECStructClass>
            <ECEntityClass typeName="NamedElement">
                 <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00"/>
                    </ECCustomAttributes>
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="POI">
                <BaseClass>NamedElement</BaseClass>
                <ECStructProperty propertyName="Address" typeName="Location" />
            </ECEntityClass>
        </ECSchema>)xml");

    testSchemas["shared columns"] = std::make_unique<SchemaItem>(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECStructClass typeName="Location" modifier="Sealed">
                <ECProperty propertyName="City" typeName="string" />
                <ECProperty propertyName="Zip" typeName="int" />
            </ECStructClass>
            <ECEntityClass typeName="NamedElement">
                 <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <ShareColumns xmlns="ECDbMap.02.00">
                            <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                        </ShareColumns>
                    </ECCustomAttributes>
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="POI">
                <BaseClass>NamedElement</BaseClass>
                <ECStructProperty propertyName="Address" typeName="Location" />
            </ECEntityClass>
        </ECSchema>)xml");

    testSchemas["overflow table"] = std::make_unique<SchemaItem>(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECStructClass typeName="Location" modifier="Sealed">
                <ECProperty propertyName="City" typeName="string" />
                <ECProperty propertyName="Zip" typeName="int" />
            </ECStructClass>
            <ECEntityClass typeName="NamedElement">
                 <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <ShareColumns xmlns="ECDbMap.02.00">
                            <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                            <MaxSharedColumnsBeforeOverflow>1</MaxSharedColumnsBeforeOverflow>
                        </ShareColumns>
                    </ECCustomAttributes>
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="POI">
                <BaseClass>NamedElement</BaseClass>
                <ECStructProperty propertyName="Address" typeName="Location" />
            </ECEntityClass>
        </ECSchema>)xml");

    testSchemas["joined table and shared columns"] = std::make_unique<SchemaItem>(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECStructClass typeName="Location" modifier="Sealed">
                <ECProperty propertyName="City" typeName="string" />
                <ECProperty propertyName="Zip" typeName="int" />
            </ECStructClass>
            <ECEntityClass typeName="NamedElement">
                 <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00"/>
                        <ShareColumns xmlns="ECDbMap.02.00">
                            <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                        </ShareColumns>
                    </ECCustomAttributes>
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="POI">
                <BaseClass>NamedElement</BaseClass>
                <ECStructProperty propertyName="Address" typeName="Location" />
            </ECEntityClass>
        </ECSchema>)xml");

    testSchemas["joined table and overflow table"] = std::make_unique<SchemaItem>(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECStructClass typeName="Location" modifier="Sealed">
                <ECProperty propertyName="City" typeName="string" />
                <ECProperty propertyName="Zip" typeName="int" />
            </ECStructClass>
            <ECEntityClass typeName="NamedElement">
                 <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00"/>
                        <ShareColumns xmlns="ECDbMap.02.00">
                            <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            <MaxSharedColumnsBeforeOverflow>1</MaxSharedColumnsBeforeOverflow>
                        </ShareColumns>
                    </ECCustomAttributes>
                <ECProperty propertyName="Code" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="POI">
                <BaseClass>NamedElement</BaseClass>
                <ECProperty propertyName="Name" typeName="string" />
                <ECStructProperty propertyName="Address" typeName="Location" />
            </ECEntityClass>
        </ECSchema>)xml");

    for (auto const& kvPair : testSchemas)
        {
        Utf8StringCR scenario = kvPair.first;
        ASSERT_EQ(SUCCESS, SetupECDb("SimpleWorkflowWithStructProp.ecdb", *kvPair.second)) << scenario;
        ASSERT_EQ(BE_SQLITE_OK, AttachCache()) << scenario;

        TestChangeTracker tracker(m_ecdb);
        tracker.EnableTracking(true);

        ECInstanceKey hallKey, stationKey, castleKey;

        ECSqlStatement insertStmt;
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.Prepare(m_ecdb, "INSERT INTO ts.POI(Name,Address.City,Address.Zip) VALUES(?,?,?)")) << scenario;

        //Changeset 1
        //Entry with intentionally wrong values which gets fixed in subsequent changesets
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindText(1, "Cty Hll", IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindText(2, "Lndn", IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindInt(3, 10001));
        ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step(hallKey)) << scenario;
        insertStmt.ClearBindings();
        insertStmt.Reset();
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindText(1, "Station", IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindText(2, "Paris", IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindInt(3, 20000));
        ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step(stationKey)) << scenario;
        insertStmt.Finalize();

        ASSERT_TRUE(tracker.HasChanges()) << scenario;
        TestChangeSet changeset1;
        ASSERT_EQ(BE_SQLITE_OK, changeset1.FromChangeTrack(tracker)) << scenario;
        //printf("Changeset 1: %s\r\n", changeset1.ToJson(m_ecdb).ToString().c_str());
        ECInstanceKey changeSummary1Key;
        ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummary1Key, ChangeSetArg(changeset1))) << scenario;

        //Changeset 2
        tracker.Restart();
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.Prepare(m_ecdb, "INSERT INTO ts.POI(Name,Address.City,Address.Zip) VALUES(?,?,?)")) << scenario;
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindText(1, "Castle", IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindText(2, "Heidelberg", IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindInt(3, 30000));
        ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step(castleKey)) << scenario;
        insertStmt.Finalize();

        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("DELETE FROM ts.POI WHERE Name='Station'")) << scenario;
        //fix Name and Location.Zip of Hall entry
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("UPDATE ts.POI SET Name='City Hall', Address.Zip=10000 WHERE Name='Cty Hll'")) << scenario;
        ASSERT_TRUE(tracker.HasChanges()) << scenario;
        TestChangeSet changeset2;
        ASSERT_EQ(BE_SQLITE_OK, changeset2.FromChangeTrack(tracker)) << scenario;
        //printf("Changeset 2: %s\r\n", changeset2.ToJson(m_ecdb).ToString().c_str());
        ECInstanceKey changeSummary2Key;
        ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummary2Key, ChangeSetArg(changeset2))) << scenario;

        //Changeset 3
        tracker.Restart();
        //fix location name of Hall
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("UPDATE ts.POI SET Address.City='London' WHERE Address.City='Lndn'")) << scenario;
        ASSERT_TRUE(tracker.HasChanges()) << scenario;
        TestChangeSet changeset3;
        ASSERT_EQ(BE_SQLITE_OK, changeset3.FromChangeTrack(tracker)) << scenario;
        //printf("Changeset 3: %s\r\n", changeset3.ToJson(m_ecdb).ToString().c_str());
        ECInstanceKey changeSummary3Key;
        ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummary3Key, ChangeSetArg(changeset3))) << scenario;
        tracker.EndTracking();

        Utf8String hallIdStr = hallKey.GetInstanceId().ToHexStr();
        Utf8String stationIdStr = stationKey.GetInstanceId().ToHexStr();
        Utf8String castleIdStr = castleKey.GetInstanceId().ToHexStr();

        //Current state
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"City Hall", "Address": {"City": "London", "Zip":10000}},
                                                 {"id":"%s", "Name":"Castle", "Address": {"City": "Heidelberg", "Zip":30000}}])json",
                                             hallIdStr.c_str(), castleIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql("SELECT ECInstanceId,Name,Address FROM ts.POI")) << scenario;

        //Changeset 1
        Utf8String summary1IdStr = changeSummary1Key.GetInstanceId().ToString();

        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"Cty Hll", "Address": {"City": "Lndn", "Zip":10001}},
                                                 {"id":"%s", "Name":"Station", "Address": {"City": "Paris", "Zip":20000}}])json",
                                             hallIdStr.c_str(), stationIdStr.c_str(), castleIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Address FROM ts.POI.Changes(%s,'AfterInsert')", summary1IdStr.c_str()).c_str())) << scenario;
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Address FROM ts.POI.Changes(%s,'BeforeUpdate')", summary1IdStr.c_str()).c_str())) << scenario;
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Address FROM ts.POI.Changes(%s,'AfterUpdate')", summary1IdStr.c_str()).c_str())) << scenario;
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Address FROM ts.POI.Changes(%s,'BeforeDelete')", summary1IdStr.c_str()).c_str())) << scenario;


        //Changeset 2

        //insert

        Utf8String summary2IdStr = changeSummary2Key.GetInstanceId().ToString();
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"Castle", "Address":{"City": "Heidelberg", "Zip":30000}}])json", castleIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Address FROM ts.POI.Changes(%s,'AfterInsert')", summary2IdStr.c_str()).c_str())) << scenario;

        //update where only one component has changed

        //before update:

        //test with plain ECSQL to avoid JSON adapter processing.
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, Utf8PrintfString("SELECT ECInstanceId,Name,Address.City,Address.Zip FROM ts.POI.Changes(%s,'BeforeUpdate')", summary2IdStr.c_str()).c_str())) << scenario;
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " Scenario: " << scenario;
        EXPECT_EQ(hallKey.GetInstanceId(), stmt.GetValueId<ECInstanceId>(0)) << stmt.GetECSql() << " Scenario: " << scenario;
        EXPECT_STREQ("Cty Hll", stmt.GetValueText(1)) << stmt.GetECSql() << " Scenario: " << scenario;
        EXPECT_STREQ("London", stmt.GetValueText(2)) << "Expected to be unchanged and therefore current value must be returned. " << stmt.GetECSql() << " Scenario: " << scenario;
        EXPECT_EQ(10001, stmt.GetValueInt(3)) << stmt.GetECSql() << " Scenario: " << scenario;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Only one row expected for " << stmt.GetECSql() << " Scenario: " << scenario;
        stmt.Finalize();

        //now with JSON adapter
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"Cty Hll", "Address":{"City": "London", "Zip":10001}}])json", hallIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Address FROM ts.POI.Changes(%s,'BeforeUpdate')", summary2IdStr.c_str()).c_str())) << scenario;
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"Cty Hll", "Address.City":"London", "Address.Zip":10001}])json", hallIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Address.City,Address.Zip FROM ts.POI.Changes(%s,'BeforeUpdate')", summary2IdStr.c_str()).c_str())) << scenario;


        //after update:

        //test with plain ECSQL to avoid JSON adapter processing.
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, Utf8PrintfString("SELECT ECInstanceId,Name,Address.City,Address.Zip FROM ts.POI.Changes(%s,'AfterUpdate')", summary2IdStr.c_str()).c_str())) << scenario;
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " Scenario: " << scenario;
        EXPECT_EQ(hallKey.GetInstanceId(), stmt.GetValueId<ECInstanceId>(0)) << stmt.GetECSql() << " Scenario: " << scenario;
        EXPECT_STREQ("City Hall", stmt.GetValueText(1)) << stmt.GetECSql() << " Scenario: " << scenario;
        EXPECT_STREQ("London", stmt.GetValueText(2)) << "Expected to be unchanged and therefore current value must be returned. " << stmt.GetECSql() << " Scenario: " << scenario;
        EXPECT_EQ(10000, stmt.GetValueInt(3)) << stmt.GetECSql() << " Scenario: " << scenario;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Only one row expected for " << stmt.GetECSql() << " Scenario: " << scenario;
        stmt.Finalize();

        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"City Hall", "Address":{"City": "London", "Zip":10000}}])json", hallIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Address FROM ts.POI.Changes(%s,'AfterUpdate')", summary2IdStr.c_str()).c_str())) << scenario;
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"City Hall", "Address.City":"London", "Address.Zip":10000}])json", hallIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Address.City,Address.Zip FROM ts.POI.Changes(%s,'AfterUpdate')", summary2IdStr.c_str()).c_str())) << scenario;

        //delete
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"Station", "Address":{"City": "Paris", "Zip":20000}}])json", stationIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Address FROM ts.POI.Changes(%s,'BeforeDelete')", summary2IdStr.c_str()).c_str())) << scenario;

        //Changeset 3

        Utf8String summary3IdStr = changeSummary3Key.GetInstanceId().ToString();

        //before update:

        //test with plain ECSQL to avoid JSON adapter processing.
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, Utf8PrintfString("SELECT ECInstanceId,Name,Address.City,Address.Zip FROM ts.POI.Changes(%s,'BeforeUpdate')", summary3IdStr.c_str()).c_str())) << scenario;
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " Scenario: " << scenario;
        EXPECT_EQ(hallKey.GetInstanceId(), stmt.GetValueId<ECInstanceId>(0)) << stmt.GetECSql() << " Scenario: " << scenario;
        EXPECT_STREQ("City Hall", stmt.GetValueText(1)) << "Expected to be unchanged and therefore current value must be returned. " << stmt.GetECSql() << " Scenario: " << scenario;
        EXPECT_STREQ("Lndn", stmt.GetValueText(2)) << stmt.GetECSql() << " Scenario: " << scenario;
        EXPECT_EQ(10000, stmt.GetValueInt(3)) << "Expected to be unchanged and therefore current value must be returned. " << stmt.GetECSql() << " Scenario: " << scenario;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Only one row expected for " << stmt.GetECSql() << " Scenario: " << scenario;
        stmt.Finalize();

        //now with JSON adapter
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"City Hall", "Address":{"City": "Lndn", "Zip":10000}}])json", hallIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Address FROM ts.POI.Changes(%s,'BeforeUpdate')", summary3IdStr.c_str()).c_str())) << scenario;
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"City Hall", "Address.City":"Lndn", "Address.Zip":10000}])json", hallIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Address.City,Address.Zip FROM ts.POI.Changes(%s,'BeforeUpdate')", summary3IdStr.c_str()).c_str())) << scenario;


        //after update:

        //test with plain ECSQL to avoid JSON adapter processing.
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, Utf8PrintfString("SELECT ECInstanceId,Name,Address.City,Address.Zip FROM ts.POI.Changes(%s,'AfterUpdate')", summary3IdStr.c_str()).c_str())) << scenario;
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " Scenario: " << scenario;
        EXPECT_EQ(hallKey.GetInstanceId(), stmt.GetValueId<ECInstanceId>(0)) << stmt.GetECSql() << " Scenario: " << scenario;
        EXPECT_STREQ("City Hall", stmt.GetValueText(1)) << "Expected to be unchanged and therefore current value must be returned. " << stmt.GetECSql() << " Scenario: " << scenario;
        EXPECT_STREQ("London", stmt.GetValueText(2)) << stmt.GetECSql() << " Scenario: " << scenario;
        EXPECT_EQ(10000, stmt.GetValueInt(3)) << "Expected to be unchanged and therefore current value must be returned. " << stmt.GetECSql() << " Scenario: " << scenario;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Only one row expected for " << stmt.GetECSql() << " Scenario: " << scenario;
        stmt.Finalize();

        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"City Hall", "Address":{"City": "London", "Zip":10000}}])json", hallIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Address FROM ts.POI.Changes(%s,'AfterUpdate')", summary3IdStr.c_str()).c_str())) << scenario;
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"City Hall", "Address.City":"London", "Address.Zip":10000}])json", hallIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Address.City,Address.Zip FROM ts.POI.Changes(%s,'AfterUpdate')", summary3IdStr.c_str()).c_str())) << scenario;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                  12/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, SimpleWorkflowWithNavPropLogicalForeignKey_NonVirtualRelECClassId)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("SimpleWorkflowWithNavPropLogicalForeignKey_NonVirtualRelECClassId.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECEntityClass typeName="Parent" modifier="Sealed">
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Child" modifier="Sealed">
                <ECProperty propertyName="Name" typeName="string" />
                <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward"/>
            </ECEntityClass>
            <ECRelationshipClass typeName="Rel" modifier="None" strength="embedding">
                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="is parent of">
                    <Class class="Parent"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is child of">
                    <Class class="Child" />
                </Target>
        </ECRelationshipClass>
        </ECSchema>)xml")));
    ASSERT_EQ(BE_SQLITE_OK, AttachCache());

    ECClassId relClassId = m_ecdb.Schemas().GetClassId("TestSchema", "Rel");
    ASSERT_TRUE(relClassId.IsValid());
    TestChangeTracker tracker(m_ecdb);

    ECInstanceKey parentKey, child1Key, child2Key;

    //changeset 1
    tracker.EnableTracking(true);
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(parentKey, "INSERT INTO ts.Parent(Name) VALUES('Parent 1')"));
    Utf8String parentIdStr = parentKey.GetInstanceId().ToHexStr();
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(child1Key, Utf8PrintfString("INSERT INTO ts.Child(Name,Parent.Id,Parent.RelECClassId) VALUES('Child 1',%s,%s)", parentIdStr.c_str(), relClassId.ToString().c_str()).c_str()));
    Utf8String child1IdStr = child1Key.GetInstanceId().ToHexStr();
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(child2Key, Utf8PrintfString("INSERT INTO ts.Child(Name,Parent.Id,Parent.RelECClassId) VALUES('Child 2',%s,%s)", parentIdStr.c_str(), relClassId.ToString().c_str()).c_str()));
    Utf8String child2IdStr = child2Key.GetInstanceId().ToHexStr();

    TestChangeSet changeset1;
    ASSERT_EQ(BE_SQLITE_OK, changeset1.FromChangeTrack(tracker));
    //printf("Changeset 1: %s\r\n", changeset1.ToJson(m_ecdb).ToString().c_str());
    ECInstanceKey changeSummary1Key;
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummary1Key, ChangeSetArg(changeset1)));


    //changeset 2
    tracker.Restart();
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql(Utf8PrintfString("UPDATE ts.Child SET Parent.Id=NULL WHERE ECInstanceId=%s", child1IdStr.c_str()).c_str()));
    TestChangeSet changeset2;
    ASSERT_EQ(BE_SQLITE_OK, changeset2.FromChangeTrack(tracker));
    //printf("Changeset 2: %s\r\n", changeset2.ToJson(m_ecdb).ToString().c_str());
    ECInstanceKey changeSummary2Key;
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummary2Key, ChangeSetArg(changeset2)));

    //changeset 3
    tracker.Restart();
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("DELETE FROM ts.Parent"));
    TestChangeSet changeset3;
    ASSERT_EQ(BE_SQLITE_OK, changeset3.FromChangeTrack(tracker));
    //printf("Changeset 3: %s\r\n", changeset3.ToJson(m_ecdb).ToString().c_str());
    ECInstanceKey changeSummary3Key;
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummary3Key, ChangeSetArg(changeset3)));
    tracker.EndTracking();
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());

    EXPECT_EQ(JsonValue(R"json([{"indirectcount":0}])json"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT count(*) indirectcount FROM change.InstanceChange WHERE Summary.Id=%s AND IsIndirect=True", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()))
        << "Expect no indirect changes because of logical Foreign key";

    //Verify current state
    //Parent: 0
    //Child: Child1, Child2
    //Rel: {Parent,Child2} (because Parent-Child1 was deleted explicitly and parent delete does not delete other rels)
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql("SELECT ECInstanceId FROM ts.Parent")) << "expected to be deleted in changeset 3";
    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s"}, {"id":"%s"}])json", child1IdStr.c_str(), child2IdStr.c_str())), GetHelper().ExecuteSelectECSql("SELECT ECInstanceId FROM ts.Child ORDER BY Name"))
        << "No cascade delete";
    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"sourceId":"%s", "targetId":"%s"}])json", parentIdStr.c_str(), child2IdStr.c_str())), GetHelper().ExecuteSelectECSql("SELECT SourceECInstanceId,TargetECInstanceId FROM ts.Rel")) << "Parent is deleted, relationships remain because of logical FK";

    //Verify change set 1
    //Parent: 1 added
    //Child: Child1 added, Child2 added
    //Rel: {Parent,Child1} added {Parent,Child2} added
    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s"}])json", parentIdStr.c_str())), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'AfterInsert')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
    //WIP: RelECClassId is not handled correctly if it is virtual.
    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"Child 1", "Parent":{"id":"%s","relClassName":"TestSchema.Rel"}},{"id":"%s", "Name":"Child 2", "Parent":{"id":"%s","relClassName":"TestSchema.Rel"}}])json", child1IdStr.c_str(), parentIdStr.c_str(), child2IdStr.c_str(), parentIdStr.c_str())),
              GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId, Name, Parent FROM ts.Child.Changes(%s,'AfterInsert') ORDER BY Name", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"sourceId":"%s", "targetId":"%s"},{"sourceId":"%s", "targetId":"%s"}])json",
                                         parentIdStr.c_str(), child1IdStr.c_str(), parentIdStr.c_str(), child2IdStr.c_str())),
              GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT SourceECInstanceId,TargetECInstanceId FROM ts.Rel.Changes(%s,'AfterInsert') ORDER BY TargetECInstanceId", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'BeforeUpdate')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'BeforeUpdate')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Rel.Changes(%s,'AfterUpdate')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'AfterUpdate')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'AfterUpdate')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Rel.Changes(%s,'AfterUpdate')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'BeforeDelete')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'BeforeDelete')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Rel.Changes(%s,'BeforeDelete')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));

    //Verify change set 2
    //Parent: 1
    //Child: Child1 modified Parent.Id=NULL, Child2 unmodified
    //Rel: {Parent, Child2}   [{Parent,Child1} deleted (because of setting Parent.Id=NULL)]
    //after insert
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'AfterInsert')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'AfterInsert')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Rel.Changes(%s,'AfterInsert')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()));

    //before update 
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'BeforeUpdate')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()));

    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"Child 1", "Parent":{"id":"%s"}}])json", child1IdStr.c_str(), parentIdStr.c_str())),
              GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Parent FROM ts.Child.Changes(%s,'BeforeUpdate')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()))
        << "Expected: Parent before being nulled out; Name is unchanged -> current value";
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT SourceECInstanceId,TargetECInstanceId FROM ts.Rel.Changes(%s,'BeforeUpdate') ORDER BY TargetECInstanceId", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()))
        << "Nav prop was updated to null which means the rel was deleted and not updated";

    //after update 
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'AfterUpdate')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()));

    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"Child 1"}])json", child1IdStr.c_str())),
              GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Parent FROM ts.Child.Changes(%s,'AfterUpdate')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()))
        << "Expected: Parent being nulled out; Name is unchanged -> current value";

    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT SourceECInstanceId,TargetECInstanceId FROM ts.Rel.Changes(%s,'AfterUpdate') ORDER BY TargetECInstanceId", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()))
        << "Nav prop was updated to null which means the rel was deleted and not updated";

    //before delete
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'BeforeDelete')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'BeforeDelete')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"sourceId":"%s", "targetId":"%s"}])json", parentIdStr.c_str(), child1IdStr.c_str(), parentIdStr.c_str(), child2IdStr.c_str())),
              GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT SourceECInstanceId,TargetECInstanceId FROM ts.Rel.Changes(%s,'BeforeDelete')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()));

    //Verify change set 3
    //Parent: 0
    //Child: Child1, Child2
    //Rel: {Parent, Child2}
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'AfterInsert')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'AfterInsert')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Rel.Changes(%s,'AfterInsert')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'BeforeUpdate')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'BeforeUpdate')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Rel.Changes(%s,'BeforeUpdate')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'AfterUpdate')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'AfterUpdate')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Rel.Changes(%s,'AfterUpdate')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));

    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s"}])json", parentIdStr.c_str())), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'BeforeDelete')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'BeforeDelete')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT SourceECInstanceId,TargetECInstanceId FROM ts.Rel.Changes(%s,'BeforeDelete') ORDER BY TargetECInstanceId", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                  12/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, SimpleWorkflowWithNavProp_MandatoryRelClassIdIsOmitted)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("SimpleWorkflowWithNavProp_MandatoryRelClassIdIsOmitted.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECEntityClass typeName="Parent" modifier="Sealed">
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Child" modifier="Sealed">
                <ECProperty propertyName="Name" typeName="string" />
                <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward"/>
            </ECEntityClass>
            <ECRelationshipClass typeName="Rel" modifier="None" strength="embedding">
                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="is parent of">
                    <Class class="Parent"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is child of">
                    <Class class="Child" />
                </Target>
        </ECRelationshipClass>
        </ECSchema>)xml")));
    ASSERT_EQ(BE_SQLITE_OK, AttachCache());

    TestChangeTracker tracker(m_ecdb);

    //changeset 1
    tracker.EnableTracking(true);
    ECInstanceKey parentKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(parentKey, "INSERT INTO ts.Parent(Name) VALUES('Parent 1')"));
    Utf8String parentIdStr = parentKey.GetInstanceId().ToHexStr();
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql(Utf8PrintfString("INSERT INTO ts.Child(Name,Parent.Id) VALUES('Child 1',%s)", parentIdStr.c_str()).c_str()));

    TestChangeSet changeset;
    ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));
    //printf("Changeset: %s\r\n", changeset.ToJson(m_ecdb).ToString().c_str());
    ECInstanceKey changeSummaryKey;
    BeTest::SetFailOnAssert(false);
    ASSERT_EQ(ERROR, m_ecdb.ExtractChangeSummary(changeSummaryKey, ChangeSetArg(changeset))) << "Expected to fail because RelClassId wasn't inserted along with Nav id";
    BeTest::SetFailOnAssert(true);
    tracker.EndTracking();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                  12/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, SimpleWorkflowWithNavPropLogicalForeignKey_VirtualRelECClassId)
    {
        ASSERT_EQ(SUCCESS, SetupECDb("SimpleWorkflowWithNavPropLogicalForeignKey_VirtualRelECClassId.ecdb", SchemaItem(
            R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECEntityClass typeName="Parent" modifier="Sealed">
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Child" modifier="Sealed">
                <ECProperty propertyName="Name" typeName="string" />
                <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward"/>
            </ECEntityClass>
            <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="embedding">
                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="is parent of">
                    <Class class="Parent"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is child of">
                    <Class class="Child" />
                </Target>
        </ECRelationshipClass>
        </ECSchema>)xml")));
        ASSERT_EQ(BE_SQLITE_OK, AttachCache());

        TestChangeTracker tracker(m_ecdb);

        ECInstanceKey parentKey, child1Key, child2Key;

        //changeset 1
        tracker.EnableTracking(true);
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(parentKey, "INSERT INTO ts.Parent(Name) VALUES('Parent 1')"));
        Utf8String parentIdStr = parentKey.GetInstanceId().ToHexStr();
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(child1Key, Utf8PrintfString("INSERT INTO ts.Child(Name,Parent.Id) VALUES('Child 1',%s)", parentIdStr.c_str()).c_str()));
        Utf8String child1IdStr = child1Key.GetInstanceId().ToHexStr();
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(child2Key, Utf8PrintfString("INSERT INTO ts.Child(Name,Parent.Id) VALUES('Child 2',%s)", parentIdStr.c_str()).c_str()));
        Utf8String child2IdStr = child2Key.GetInstanceId().ToHexStr();

        TestChangeSet changeset1;
        ASSERT_EQ(BE_SQLITE_OK, changeset1.FromChangeTrack(tracker));
        //printf("Changeset 1: %s\r\n", changeset1.ToJson(m_ecdb).ToString().c_str());
        ECInstanceKey changeSummary1Key;
        ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummary1Key, ChangeSetArg(changeset1)));


        //changeset 2
        tracker.Restart();
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql(Utf8PrintfString("UPDATE ts.Child SET Parent.Id=NULL WHERE ECInstanceId=%s", child1IdStr.c_str()).c_str()));
        TestChangeSet changeset2;
        ASSERT_EQ(BE_SQLITE_OK, changeset2.FromChangeTrack(tracker));
        //printf("Changeset 2: %s\r\n", changeset2.ToJson(m_ecdb).ToString().c_str());
        ECInstanceKey changeSummary2Key;
        ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummary2Key, ChangeSetArg(changeset2)));

        //changeset 3
        tracker.Restart();
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("DELETE FROM ts.Parent"));
        TestChangeSet changeset3;
        ASSERT_EQ(BE_SQLITE_OK, changeset3.FromChangeTrack(tracker));
        //printf("Changeset 3: %s\r\n", changeset3.ToJson(m_ecdb).ToString().c_str());
        ECInstanceKey changeSummary3Key;
        ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummary3Key, ChangeSetArg(changeset3)));
        tracker.EndTracking();
        ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());

        EXPECT_EQ(JsonValue(R"json([{"indirectcount":0}])json"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT count(*) indirectcount FROM change.InstanceChange WHERE Summary.Id=%s AND IsIndirect=True", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()))
            << "Expect no indirect changes because of logical Foreign key";

        //Verify current state
        //Parent: 0
        //Child: Child1, Child2
        //Rel: {Parent,Child2} (because Parent-Child1 was deleted explicitly and parent delete does not delete other rels)
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql("SELECT ECInstanceId FROM ts.Parent")) << "expected to be deleted in changeset 3";
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s"}, {"id":"%s"}])json", child1IdStr.c_str(), child2IdStr.c_str())), GetHelper().ExecuteSelectECSql("SELECT ECInstanceId FROM ts.Child ORDER BY Name"))
            << "No cascade delete";
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"sourceId":"%s", "targetId":"%s"}])json", parentIdStr.c_str(), child2IdStr.c_str())), GetHelper().ExecuteSelectECSql("SELECT SourceECInstanceId,TargetECInstanceId FROM ts.Rel")) << "Parent is deleted, relationships remain because of logical FK";

        //Verify change set 1
        //Parent: 1 added
        //Child: Child1 added, Child2 added
        //Rel: {Parent,Child1} added {Parent,Child2} added
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s"}])json", parentIdStr.c_str())), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'AfterInsert')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
        //WIP: RelECClassId is not handled correctly if it is virtual.
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"Child 1", "Parent":{"id":"%s", "relClassName":"TestSchema.Rel"}},{"id":"%s", "Name":"Child 2", "Parent":{"id":"%s","relClassName":"TestSchema.Rel"}}])json", child1IdStr.c_str(), parentIdStr.c_str(), child2IdStr.c_str(), parentIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId, Name, Parent FROM ts.Child.Changes(%s,'AfterInsert') ORDER BY Name", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"sourceId":"%s", "targetId":"%s"},{"sourceId":"%s", "targetId":"%s"}])json",
                                             parentIdStr.c_str(), child1IdStr.c_str(), parentIdStr.c_str(), child2IdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT SourceECInstanceId,TargetECInstanceId FROM ts.Rel.Changes(%s,'AfterInsert') ORDER BY TargetECInstanceId", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'BeforeUpdate')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'BeforeUpdate')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Rel.Changes(%s,'AfterUpdate')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'AfterUpdate')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'AfterUpdate')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Rel.Changes(%s,'AfterUpdate')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'BeforeDelete')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'BeforeDelete')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Rel.Changes(%s,'BeforeDelete')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));

        //Verify change set 2
        //Parent: 1
        //Child: Child1 modified Parent.Id=NULL, Child2 unmodified
        //Rel: {Parent, Child2}   [{Parent,Child1} deleted (because of setting Parent.Id=NULL)]
        //after insert
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'AfterInsert')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()));
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'AfterInsert')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()));
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Rel.Changes(%s,'AfterInsert')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()));

        //before update 
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'BeforeUpdate')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()));

        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"Child 1", "Parent":{"id":"%s", "relClassName":"TestSchema.Rel"}}])json", child1IdStr.c_str(), parentIdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Parent FROM ts.Child.Changes(%s,'BeforeUpdate')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()))
            << "Expected: Parent before being nulled out; Name is unchanged -> current value";
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT SourceECInstanceId,TargetECInstanceId FROM ts.Rel.Changes(%s,'BeforeUpdate') ORDER BY TargetECInstanceId", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()))
            << "Nav prop was updated to null which means the rel was deleted and not updated";

        //after update 
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'AfterUpdate')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()));

        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"Child 1"}])json", child1IdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Parent FROM ts.Child.Changes(%s,'AfterUpdate')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()))
            << "Expected: Parent being nulled out; Name is unchanged -> current value";

        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT SourceECInstanceId,TargetECInstanceId FROM ts.Rel.Changes(%s,'AfterUpdate') ORDER BY TargetECInstanceId", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()))
            << "Nav prop was updated to null which means the rel was deleted and not updated";

        //before delete
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'BeforeDelete')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()));
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'BeforeDelete')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()));
        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"sourceId":"%s", "targetId":"%s"}])json", parentIdStr.c_str(), child1IdStr.c_str(), parentIdStr.c_str(), child2IdStr.c_str())),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT SourceECInstanceId,TargetECInstanceId FROM ts.Rel.Changes(%s,'BeforeDelete')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()));

        //Verify change set 3
        //Parent: 0
        //Child: Child1, Child2
        //Rel: {Parent, Child2}
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'AfterInsert')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'AfterInsert')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Rel.Changes(%s,'AfterInsert')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'BeforeUpdate')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'BeforeUpdate')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Rel.Changes(%s,'BeforeUpdate')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'AfterUpdate')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'AfterUpdate')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Rel.Changes(%s,'AfterUpdate')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));

        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s"}])json", parentIdStr.c_str())), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'BeforeDelete')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'BeforeDelete')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
        EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT SourceECInstanceId,TargetECInstanceId FROM ts.Rel.Changes(%s,'BeforeDelete') ORDER BY TargetECInstanceId", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
        }

//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                  12/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, VirtualRelECClassId)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("VirtualRelECClassId.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECEntityClass typeName="Parent" modifier="Sealed">
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Child" modifier="Sealed">
                <ECProperty propertyName="Name" typeName="string" />
                <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward"/>
            </ECEntityClass>
            <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="embedding">
                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="is parent of">
                    <Class class="Parent"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is child of">
                    <Class class="Child" />
                </Target>
        </ECRelationshipClass>
        </ECSchema>)xml")));
    ASSERT_EQ(BE_SQLITE_OK, AttachCache());

    TestChangeTracker tracker(m_ecdb);

    ECInstanceKey parentKey, childKey;

    //changeset 1
    tracker.EnableTracking(true);
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(parentKey, "INSERT INTO ts.Parent(Name) VALUES('Parent 1')"));
    Utf8String parentIdStr = parentKey.GetInstanceId().ToHexStr();
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(childKey, Utf8PrintfString("INSERT INTO ts.Child(Name,Parent.Id) VALUES('Child 1',%s)", parentIdStr.c_str()).c_str()));
    Utf8String childIdStr = childKey.GetInstanceId().ToHexStr();

    TestChangeSet changeset;
    ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));
    //printf("Changeset: %s\r\n", changeset.ToJson(m_ecdb).ToString().c_str());
    ECInstanceKey changeSummaryKey;
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummaryKey, ChangeSetArg(changeset)));

    tracker.EndTracking();
    //now delete the child so that its unmodified values show up as NULL in the Changes function
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("DELETE FROM ts.Child"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, Utf8PrintfString("SELECT Parent.Id,Parent.RelECClassId FROM ts.Child.Changes(%s,'AfterInsert')", changeSummaryKey.GetInstanceId().ToString().c_str()).c_str()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(parentKey.GetInstanceId(), stmt.GetValueId<ECInstanceId>(0)) << stmt.GetECSql();
    ASSERT_EQ(m_ecdb.Schemas().GetClassId("TestSchema", "Rel"), stmt.GetValueId<ECClassId>(1));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                  12/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, SimpleWorkflowWithNavPropCascadeDelete)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("SimpleWorkflowWithNavPropCascadeDelete.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Parent" modifier="Sealed">
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Child" modifier="Sealed">
                <ECProperty propertyName="Name" typeName="string" />
                <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward">
                    <ECCustomAttributes>
                        <ForeignKeyConstraint xmlns="ECDbMap.02.00">
                            <OnDeleteAction>Cascade</OnDeleteAction>
                        </ForeignKeyConstraint>
                    </ECCustomAttributes>
                </ECNavigationProperty>
            </ECEntityClass>
            <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="embedding">
                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="is parent of">
                    <Class class="Parent"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is child of">
                    <Class class="Child" />
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml")));
    ASSERT_EQ(BE_SQLITE_OK, AttachCache());

    TestChangeTracker tracker(m_ecdb);

    ECInstanceKey parentKey, child1Key, child2Key;

    //changeset 1
    tracker.EnableTracking(true);
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(parentKey, "INSERT INTO ts.Parent(Name) VALUES('Parent 1')"));
    Utf8String parentIdStr = parentKey.GetInstanceId().ToHexStr();
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(child1Key, Utf8PrintfString("INSERT INTO ts.Child(Name,Parent.Id) VALUES('Child 1',%s)", parentIdStr.c_str()).c_str()));
    Utf8String child1IdStr = child1Key.GetInstanceId().ToHexStr();
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(child2Key, Utf8PrintfString("INSERT INTO ts.Child(Name,Parent.Id) VALUES('Child 2',%s)", parentIdStr.c_str()).c_str()));
    Utf8String child2IdStr = child2Key.GetInstanceId().ToHexStr();

    TestChangeSet changeset1;
    ASSERT_EQ(BE_SQLITE_OK, changeset1.FromChangeTrack(tracker));
    //printf("Changeset 1: %s\r\n", changeset1.ToJson(m_ecdb).ToString().c_str());
    ECInstanceKey changeSummary1Key;
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummary1Key, ChangeSetArg(changeset1)));


    //changeset 2
    tracker.Restart();
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql(Utf8PrintfString("UPDATE ts.Child SET Parent.Id=NULL WHERE ECInstanceId=%s", child1IdStr.c_str()).c_str()));
    TestChangeSet changeset2;
    ASSERT_EQ(BE_SQLITE_OK, changeset2.FromChangeTrack(tracker));
    //printf("Changeset 2: %s\r\n", changeset2.ToJson(m_ecdb).ToString().c_str());
    ECInstanceKey changeSummary2Key;
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummary2Key, ChangeSetArg(changeset2)));

    //changeset 3
    tracker.Restart();
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("DELETE FROM ts.Parent"));
    TestChangeSet changeset3;
    ASSERT_EQ(BE_SQLITE_OK, changeset3.FromChangeTrack(tracker));
    //printf("Changeset 3: %s\r\n", changeset3.ToJson(m_ecdb).ToString().c_str());
    ECInstanceKey changeSummary3Key;
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummary3Key, ChangeSetArg(changeset3)));
    tracker.EndTracking();
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());

    //Verify current state
    //Parent: 0
    //Child: Child1
    //Rel: 0

    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql("SELECT ECInstanceId FROM ts.Parent")) << "expected to be deleted in changeset 3";
    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s"}])json", child1IdStr.c_str())), GetHelper().ExecuteSelectECSql("SELECT ECInstanceId FROM ts.Child"))
        << "1 left, other cascade deleted in changeset 3";
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql("SELECT ECInstanceId FROM ts.Rel")) << "Rels are expected to be deleted when parent is deleted";

    //Verify change set 1
    //Parent: Parent1
    //Child: Child1, Child2
    //Rel: {Parent1,Child1} {Parent1,Child2}

    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s"}])json", parentIdStr.c_str())), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'AfterInsert')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"Child 1", "Parent":{"id":"%s", "relClassName":"TestSchema.Rel"}},{"id":"%s", "Name":"Child 2", "Parent":{"id":"%s", "relClassName":"TestSchema.Rel"}}])json", child1IdStr.c_str(), parentIdStr.c_str(), child2IdStr.c_str(), parentIdStr.c_str())),
              GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId, Name, Parent FROM ts.Child.Changes(%s,'AfterInsert') ORDER BY Name", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"sourceId":"%s", "targetId":"%s"},{"sourceId":"%s", "targetId":"%s"}])json", 
                                         parentIdStr.c_str(), child1IdStr.c_str(), parentIdStr.c_str(), child2IdStr.c_str())),
              GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT SourceECInstanceId,TargetECInstanceId FROM ts.Rel.Changes(%s,'AfterInsert') ORDER BY TargetECInstanceId", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'BeforeUpdate')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'BeforeUpdate')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Rel.Changes(%s,'AfterUpdate')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'AfterUpdate')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'AfterUpdate')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Rel.Changes(%s,'AfterUpdate')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'BeforeDelete')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'BeforeDelete')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Rel.Changes(%s,'BeforeDelete')", changeSummary1Key.GetInstanceId().ToString().c_str()).c_str()));

    //Verify change set 2
    //Parent: Parent1
    //Child: Child1 (modified), Child2
    //Rel: {Parent1,Child2}

    //after insert
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'AfterInsert')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'AfterInsert')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Rel.Changes(%s,'AfterInsert')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()));

    //before update 
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'BeforeUpdate')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()));

    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"Child 1", "Parent":{"id":"%s", "relClassName":"TestSchema.Rel"}}])json", child1IdStr.c_str(), parentIdStr.c_str())),
              GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Parent FROM ts.Child.Changes(%s,'BeforeUpdate')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()))
        << "Expected: Parent before being nulled out; Name is unchanged -> current value";
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT SourceECInstanceId,TargetECInstanceId FROM ts.Rel.Changes(%s,'BeforeUpdate') ORDER BY TargetECInstanceId", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()))
        << "Nav prop was updated to null which means the rel was deleted and not updated";

    //after update 
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'AfterUpdate')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()));

    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s", "Name":"Child 1"}])json", child1IdStr.c_str())),
              GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId,Name,Parent FROM ts.Child.Changes(%s,'AfterUpdate')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()))
        << "Expected: Parent being nulled out; Name is unchanged -> current value";

    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT SourceECInstanceId,TargetECInstanceId FROM ts.Rel.Changes(%s,'AfterUpdate') ORDER BY TargetECInstanceId", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()))
        << "Nav prop was updated to null which means the rel was deleted and not updated";

    //before delete
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'BeforeDelete')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'BeforeDelete')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"sourceId":"%s", "targetId":"%s"}])json", parentIdStr.c_str(), child1IdStr.c_str(), parentIdStr.c_str(), child2IdStr.c_str())),
              GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT SourceECInstanceId,TargetECInstanceId FROM ts.Rel.Changes(%s,'BeforeDelete')", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str()));

    //Verify change set 3
    //Parent: 0
    //Child: Child1 (modified)
    //Rel: 0

    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'AfterInsert')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'AfterInsert')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Rel.Changes(%s,'AfterInsert')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'BeforeUpdate')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'BeforeUpdate')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Rel.Changes(%s,'BeforeUpdate')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'AfterUpdate')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'AfterUpdate')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Rel.Changes(%s,'AfterUpdate')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));

    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s"}])json", parentIdStr.c_str())), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Parent.Changes(%s,'BeforeDelete')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id":"%s"}])json", child2IdStr.c_str())), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ECInstanceId FROM ts.Child.Changes(%s,'BeforeDelete')", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"sourceId":"%s", "targetId":"%s"}])json", parentIdStr.c_str(), child2IdStr.c_str())),
              GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT SourceECInstanceId,TargetECInstanceId FROM ts.Rel.Changes(%s,'BeforeDelete') ORDER BY TargetECInstanceId", changeSummary3Key.GetInstanceId().ToString().c_str()).c_str()));

    }


//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                  07/18
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, DeletedLinkTableRow)
    {
    auto resetFile = [this] (ECInstanceKey& aKey, ECInstanceKey& bKey, ECInstanceKey& relKey, TestChangeTracker& tracker)
        {
        ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
        tracker.EndTracking();
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("DELETE FROM ts.A"));
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("DELETE FROM ts.B"));
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("DELETE FROM ts.Rel"));

        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(aKey, "INSERT INTO ts.A(Name) VALUES('A 1')"));
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(bKey, "INSERT INTO ts.B(Name) VALUES('B 1')"));
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(relKey, Utf8PrintfString("INSERT INTO ts.Rel(SourceECInstanceId,TargetECInstanceId) VALUES(%s,%s)", aKey.GetInstanceId().ToString().c_str(), bKey.GetInstanceId().ToString().c_str()).c_str()));
        ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
        tracker.EnableTracking(true);
        };

    // Case 1: Link table has FKs into end tables
            { 
            // Classes must have a class id column -> TPH and not sealed
            ASSERT_EQ(SUCCESS, SetupECDb("DeletedLinkTableRow.ecdb", SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?> 
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="A">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Name" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="B">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Name" typeName="string" />
                    </ECEntityClass>
                    <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="referencing">
                        <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="A"/>
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                            <Class class="B" />
                        </Target>
                    </ECRelationshipClass>
                </ECSchema>)xml")));
            ASSERT_EQ(BE_SQLITE_OK, AttachCache());

            ASSERT_EQ(ExpectedColumn("ts_A", "ECClassId", Virtual::No), GetHelper().GetPropertyMapColumn(AccessString("ts", "A", "ECClassId")));
            ASSERT_EQ(ExpectedColumn("ts_B", "ECClassId", Virtual::No), GetHelper().GetPropertyMapColumn(AccessString("ts", "B", "ECClassId")));

            TestChangeTracker tracker(m_ecdb);

            ECInstanceKey aKey, bKey, relKey;

            //Scenario: Just delete the rel
            resetFile(aKey, bKey, relKey, tracker);
            ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("DELETE FROM ts.Rel"));

            TestChangeSet changeset;
            ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));
            //printf("Changeset: %s\r\n", changeset.ToJson(m_ecdb).ToString().c_str());
            ECInstanceKey changeSummaryKey;
            ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummaryKey, ChangeSetArg(changeset)));

            ASSERT_EQ(JsonValue(Utf8PrintfString(R"json([{"ChangedInstance":{"Id":%s, "ClassId":%s},"OpCode":%d,"IsIndirect":false}])json",relKey.GetInstanceId().ToString().c_str(), relKey.GetClassId().ToString().c_str(), (int) ChangeOpCode::Delete)),
                      GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ChangedInstance,OpCode,IsIndirect FROM change.InstanceChange WHERE Summary.Id=%s", changeSummaryKey.GetInstanceId().ToString().c_str()).c_str())) << "After deleting just the rel";

            //Scenario: Delete the rel and a
            resetFile(aKey, bKey, relKey, tracker);
            ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("DELETE FROM ts.Rel"));
            ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("DELETE FROM ts.A"));

            changeset.Free();
            ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));
            //printf("Changeset: %s\r\n", changeset.ToJson(m_ecdb).ToString().c_str());
            ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummaryKey, ChangeSetArg(changeset)));
            ASSERT_EQ(JsonValue(Utf8PrintfString(R"json(
                    [{"ChangedInstance":{"Id":%s, "ClassId":%s},"OpCode":%d,"IsIndirect":false},
                     {"ChangedInstance":{"Id":%s, "ClassId":%s},"OpCode":%d,"IsIndirect":false}])json",
                      aKey.GetInstanceId().ToString().c_str(), aKey.GetClassId().ToString().c_str(), (int) ChangeOpCode::Delete,
                      relKey.GetInstanceId().ToString().c_str(), relKey.GetClassId().ToString().c_str(), (int) ChangeOpCode::Delete)),
                      GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ChangedInstance,OpCode,IsIndirect FROM change.InstanceChange WHERE Summary.Id=%s", changeSummaryKey.GetInstanceId().ToString().c_str()).c_str())) << "After deleting A and the rel";

            //Scenario: Delete just a (which should cascadingly delete the rel)
            resetFile(aKey, bKey, relKey, tracker);
            ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("DELETE FROM ts.A"));

            changeset.Free();
            ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));
            //printf("Changeset: %s\r\n", changeset.ToJson(m_ecdb).ToString().c_str());
            ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummaryKey, ChangeSetArg(changeset)));
            ASSERT_EQ(JsonValue(Utf8PrintfString(R"json(
                    [{"ChangedInstance":{"Id":%s, "ClassId":%s},"OpCode":%d,"IsIndirect":false},
                     {"ChangedInstance":{"Id":%s, "ClassId":%s},"OpCode":%d,"IsIndirect":true}])json",
                                                 aKey.GetInstanceId().ToString().c_str(), aKey.GetClassId().ToString().c_str(), (int) ChangeOpCode::Delete,
                                                 relKey.GetInstanceId().ToString().c_str(), relKey.GetClassId().ToString().c_str(), (int) ChangeOpCode::Delete)),
                      GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ChangedInstance,OpCode,IsIndirect FROM change.InstanceChange WHERE Summary.Id=%s", changeSummaryKey.GetInstanceId().ToString().c_str()).c_str())) << "After deleting A (which cascading deletes rel)";
            }

   // Case 2: Link table has no FKs into end tables

        {
        ASSERT_EQ(SUCCESS, SetupECDb("DeletedLinkTableRow_NoFks.ecdb", SchemaItem(
            R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
        <ECEntityClass typeName="A">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
            </ECCustomAttributes>
            <ECProperty propertyName="Name" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName="B">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
            </ECCustomAttributes>
            <ECProperty propertyName="Name" typeName="string" />
        </ECEntityClass>
        <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="referencing">
            <ECCustomAttributes>
                <LinkTableRelationshipMap xmlns="ECDbMap.02.00.00">
                    <CreateForeignKeyConstraints>false</CreateForeignKeyConstraints>
                </LinkTableRelationshipMap>
            </ECCustomAttributes>
            <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                <Class class="A"/>
            </Source>
            <Target multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                <Class class="B" />
            </Target>
        </ECRelationshipClass>
    </ECSchema>)xml")));
        ASSERT_EQ(BE_SQLITE_OK, AttachCache());

        ASSERT_EQ(ExpectedColumn("ts_A", "ECClassId", Virtual::No), GetHelper().GetPropertyMapColumn(AccessString("ts", "A", "ECClassId")));
        ASSERT_EQ(ExpectedColumn("ts_B", "ECClassId", Virtual::No), GetHelper().GetPropertyMapColumn(AccessString("ts", "B", "ECClassId")));

        TestChangeTracker tracker(m_ecdb);
        tracker.EnableTracking(true);

        ECInstanceKey aKey, bKey, relKey;

        //Scenario: Just delete the rel
        resetFile(aKey, bKey, relKey, tracker);
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("DELETE FROM ts.Rel"));

        TestChangeSet changeset;
        ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));
        //printf("Changeset: %s\r\n", changeset.ToJson(m_ecdb).ToString().c_str());
        ECInstanceKey changeSummaryKey;
        ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummaryKey, ChangeSetArg(changeset)));

        ASSERT_EQ(JsonValue(Utf8PrintfString(R"json([{"ChangedInstance":{"Id":%s, "ClassId":%s},"OpCode":%d,"IsIndirect":false}])json", relKey.GetInstanceId().ToString().c_str(), relKey.GetClassId().ToString().c_str(), (int) ChangeOpCode::Delete)),
                  GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ChangedInstance,OpCode,IsIndirect FROM change.InstanceChange WHERE Summary.Id=%s", changeSummaryKey.GetInstanceId().ToString().c_str()).c_str())) << "After deleting the rel";

        //Scenario: delete the rel and a
        resetFile(aKey, bKey, relKey, tracker);
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("DELETE FROM ts.Rel"));
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("DELETE FROM ts.A"));

        changeset.Free();
        ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));
        //printf("Changeset: %s\r\n", changeset.ToJson(m_ecdb).ToString().c_str());
        ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummaryKey, ChangeSetArg(changeset)));
        ASSERT_EQ(JsonValue(Utf8PrintfString(R"json(
            [{"ChangedInstance":{"Id":%s, "ClassId":%s},"OpCode":%d,"IsIndirect":false},
                {"ChangedInstance":{"Id":%s, "ClassId":%s},"OpCode":%d,"IsIndirect":false}])json",
                                                aKey.GetInstanceId().ToString().c_str(), aKey.GetClassId().ToString().c_str(), (int) ChangeOpCode::Delete,
                                                relKey.GetInstanceId().ToString().c_str(), relKey.GetClassId().ToString().c_str(), (int) ChangeOpCode::Delete)),
                    GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ChangedInstance,OpCode,IsIndirect FROM change.InstanceChange WHERE Summary.Id=%s", changeSummaryKey.GetInstanceId().ToString().c_str()).c_str())) << "After deleting the rel and A";

        //Scenario: delete a in one changeset and rel in second changeset
        resetFile(aKey, bKey, relKey, tracker);
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("DELETE FROM ts.A"));

        changeset.Free();
        ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));
        //printf("Changeset: %s\r\n", changeset.ToJson(m_ecdb).ToString().c_str());
        ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummaryKey, ChangeSetArg(changeset)));
        ASSERT_EQ(JsonValue(Utf8PrintfString(R"json(
            [{"ChangedInstance":{"Id":%s, "ClassId":%s},"OpCode":%d,"IsIndirect":false}])json",
                                                aKey.GetInstanceId().ToString().c_str(), aKey.GetClassId().ToString().c_str(), (int) ChangeOpCode::Delete).c_str()),
                    GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ChangedInstance,OpCode,IsIndirect FROM change.InstanceChange WHERE Summary.Id=%s", changeSummaryKey.GetInstanceId().ToString().c_str()).c_str())) << "After deleting A in first changeset";

        tracker.Restart();

        //now delete rel in separate changeset
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("DELETE FROM ts.Rel"));
        changeset.Free();
        ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));
        //printf("Changeset: %s\r\n", changeset.ToJson(m_ecdb).ToString().c_str());
        ECInstanceKey changeSummary2Key;
        ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummary2Key, ChangeSetArg(changeset)));
        ASSERT_EQ(JsonValue(Utf8PrintfString(R"json(
            [{"ChangedInstance":{"Id":%s, "ClassId":%s},"OpCode":%d,"IsIndirect":false}])json",
                                                relKey.GetInstanceId().ToString().c_str(), relKey.GetClassId().ToString().c_str(), (int) ChangeOpCode::Delete).c_str()),
                    GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT ChangedInstance,OpCode,IsIndirect FROM change.InstanceChange WHERE Summary.Id=%s", changeSummary2Key.GetInstanceId().ToString().c_str()).c_str())) << "After deleting Rel in second changeset";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    12/16
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, SchemaChange)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("invalidsummarytest.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, AttachCache());

    // Test1: Change to be_Prop table - should cause empty change summary without errors
    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    DbResult result = m_ecdb.SavePropertyString(PropertySpec("TestName", "TestNamespace"), "TestValue");
    ASSERT_EQ(BE_SQLITE_OK, result);

    TestChangeSet changeSet;
    result = changeSet.FromChangeTrack(tracker);
    ASSERT_EQ(BE_SQLITE_OK, result);

    ECInstanceKey summaryKey;
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(summaryKey, ChangeSetArg(changeSet), ECDb::ChangeSummaryExtractOptions(false)));

    tracker.Restart();

    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'> "
        "</ECSchema>")));

    changeSet.Free();
    ASSERT_EQ(BE_SQLITE_OK, changeSet.FromChangeTrack(tracker));
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(summaryKey, ChangeSetArg(changeSet), ECDb::ChangeSummaryExtractOptions(false)));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Maha.Nasir                    1/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, Crud)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("changeSummary_Crud.ecdb", SchemaItem(R"(<?xml version='1.0' encoding='utf-8'?>
            <ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />
                <ECEntityClass typeName='Foo' modifier='None'>
                    <ECCustomAttributes>
                        <ClassMap xmlns='ECDbMap.02.00'>
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <ShareColumns xmlns='ECDbMap.02.00'>
                            <MaxSharedColumnsBeforeOverflow>1</MaxSharedColumnsBeforeOverflow>
                            <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                        </ShareColumns>
                    </ECCustomAttributes>
                    <ECProperty propertyName='S' typeName='string'/>
                    <ECProperty propertyName='I' typeName='int'/>
                    <ECProperty propertyName='P2D' typeName='point2d'/>
                </ECEntityClass>
                <ECEntityClass typeName='Goo' modifier='None'>
                    <ECProperty propertyName='S' typeName='string'/>
                    <ECProperty propertyName='I' typeName='int'/>
                    <ECProperty propertyName='P2D' typeName='point2d'/>
                    <ECNavigationProperty propertyName='Foo' relationshipName='Rel' direction='Backward'/>
                </ECEntityClass>
                <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='embedding'>
                    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='embeds'>
                        <Class class='Foo'/>
                    </Source>
                    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='is embedded in'>
                        <Class class='Goo' />
                    </Target>
                </ECRelationshipClass>
            </ECSchema>)")));
    ASSERT_EQ(BE_SQLITE_OK, AttachCache());

    ECInstanceKey i1, i2, i3, i4, i5, i6;
    {//---------------------------------------------------->>>
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Foo(S,I,P2D.X,P2D.Y)VALUES('Night',100,10.33,30.34)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(i1));
    }//---------------------------------------------------->>>

    {//---------------------------------------------------->>>
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Foo(S,I,P2D.X,P2D.Y)VALUES('Dawn',200,30.42, -3.44)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(i2));
    }//---------------------------------------------------->>>


    {//---------------------------------------------------->>>
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Foo(S,I,P2D.X,P2D.Y)VALUES('Lion',300,-66.23, 12.33)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(i3));
    }//---------------------------------------------------->>>

    {//---------------------------------------------------->>>
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, Utf8PrintfString("INSERT INTO ts.Goo(S,I,P2D.X,P2D.Y,Foo.Id)VALUES('Ten',123,-32.23, 45.22, %s)", i1.GetInstanceId().ToString().c_str()).c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(i4));
    }//---------------------------------------------------->>>

    {//---------------------------------------------------->>>
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, Utf8PrintfString("INSERT INTO ts.Goo(S,I,P2D.X,P2D.Y,Foo.Id)VALUES('Zen',223,-12.23, 45.22, %s)", i2.GetInstanceId().ToString().c_str()).c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(i4));
    }//---------------------------------------------------->>>

    {//---------------------------------------------------->>>
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Goo(S,I,P2D.X,P2D.Y)VALUES('Tree',1213,-2332.23, 245.22)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(i4));
    }//---------------------------------------------------->>>


    m_ecdb.SaveChanges();
    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    {//---------------------------------------------------->>>
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Foo(S,I,P2D.X,P2D.Y)VALUES('Cat',400,-10.12,-23.56)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(i5));
    }//---------------------------------------------------->>>

    {//---------------------------------------------------->>>
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, Utf8PrintfString("INSERT INTO ts.Goo(S,I,P2D.X,P2D.Y,Foo.Id)VALUES('Blue',2233,-212.23, -215.44, %s)", i3.GetInstanceId().ToString().c_str()).c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(i6));
    }//---------------------------------------------------->>>

    {//---------------------------------------------------->>>
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "DELETE FROM ts.Foo WHERE S='Dawn'"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }//---------------------------------------------------->>>


    {//---------------------------------------------------->>>
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Foo SET S='Zoo', P2D.X=1000 WHERE S='Night'"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }//---------------------------------------------------->>>


    {//---------------------------------------------------->>>
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, Utf8PrintfString("UPDATE ts.Goo SET S='Plant',I=10000,P2D.X=0, P2D.Y=0, Foo.Id=%s WHERE S='Tree'", i5.GetInstanceId().ToString().c_str()).c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }//---------------------------------------------------->>>

    {//---------------------------------------------------->>>
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Foo SET P2D.Y=2000, I=666 WHERE S='Lion'"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }//---------------------------------------------------->>>

    TestChangeSet changeset;
    ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));

    ECInstanceKey changeSummaryKey;
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummaryKey, ChangeSetArg(changeset)));
    m_ecdb.SaveChanges();

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT S, P2D FROM ts.Foo.Changes(?, ?)"));
    stmt.BindId(1, changeSummaryKey.GetInstanceId());
    stmt.BindInt(2, (int) ChangedValueState::BeforeUpdate);
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Maha.Nasir                    1/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, PropertiesWithRegularColumns)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("overflowProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECStructClass typeName='ST1' modifier='None'>"
        "        <ECProperty propertyName='D1' typeName='double' readOnly='false'/>"
        "        <ECProperty propertyName='I1' typeName='int'/>"
        "    </ECStructClass>"
        "    <ECStructClass typeName='ST2' modifier='None'>"
        "        <ECProperty propertyName='D2' typeName='double' readOnly='false'/>"
        "        <ECProperty propertyName='P3D' typeName='point3d'/>"
        "        <ECStructProperty propertyName='ST2P' typeName='ST1'/>"
        "    </ECStructClass>"
        "    <ECEntityClass typeName='Element' modifier='None'>"
        "        <ECProperty propertyName='S' typeName='string'/>"
        "        <ECProperty propertyName='I' typeName='int'/>"
        "        <ECProperty propertyName='L' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='double'/>"
        "        <ECProperty propertyName='DT' typeName='dateTime'/>"
        "        <ECProperty propertyName='B' typeName='boolean'/>"
        "        <ECProperty propertyName='P2D' typeName='point2d'/>"
        "        <ECProperty propertyName='P3D' typeName='point3d'/>"
        "        <ECProperty propertyName='BIN' typeName='binary'/>"
        "        <ECProperty propertyName='Geom' typeName='Bentley.Geometry.Common.IGeometry'/>"
        "        <ECStructProperty propertyName='StructProp' typeName='ST1'/>"
        "        <ECArrayProperty propertyName='ArrayProp' typeName='double' minOccurs='0' maxOccurs='unbounded'/>"
        "        <ECArrayProperty propertyName='arrayOfP2d' typeName='point2d' minOccurs='0' maxOccurs='unbounded'/>"
        "        <ECArrayProperty propertyName='arrayOfP3d' typeName='point3d' minOccurs='0' maxOccurs='unbounded'/>"
        "        <ECStructArrayProperty propertyName='arrayOfST2' typeName='ST2' minOccurs='0' maxOccurs='unbounded'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));
    ASSERT_EQ(BE_SQLITE_OK, AttachCache());

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    int idx = 1;
    Utf8CP String = "TestVal";
    int Integer = 132;
    int64_t Long = 1235678901;
    double Double = 2341.432;
    DateTime DT = DateTime(DateTime::Kind::Unspecified, 2017, 1, 17, 0, 0);
    bool Boolean = false;
    DPoint2d P2D = DPoint2d::From(22.33, -21.34);
    DPoint3d P3D = DPoint3d::From(12.13, -42.34, -93.12);
    IGeometryPtr geom = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));
    std::vector<Utf8Char> Blob = {'H', 'e', 'l','l', 'o'};
    double Array[] = {123.3434, 345.223,-532.123};
    DPoint2d ArrayOfP2d[] = {DPoint2d::From(-21, 22.1),DPoint2d::From(-85.34, 35.36),DPoint2d::From(-31.34, 12.35)};
    DPoint3d ArrayOfP3d[] = {DPoint3d::From(-41.33, 41.13, -12.25), DPoint3d::From(-23.37, 53.54, -34.31), DPoint3d::From(-33.41, 11.13, -99.11)};
    double ArrayOfST1_D1[] = {123.3434, 345.223,-532.123};
    int ArrayOfST1_I1[] = {012, 456, 789};
    double ArrayOfST2_D2[] = {12.3, -45.72, -31.11};
    DPoint3d ArrayOfST2_P3D[] = {DPoint3d::From(-12.11, -74.1, 12.3),DPoint3d::From(-12.53, 21.76, -32.22),DPoint3d::From(-41.14, -22.45, -31.16)};

    ECSqlStatement stmt;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Element (S, I, L, D, DT, B, P2D, P3D, BIN, Geom, StructProp, ArrayProp , arrayOfP2d, arrayOfP3d, arrayOfST2) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(idx++, String, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(idx++, Integer));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(idx++, Long));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(idx++, Double));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(idx++, DT));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBoolean(idx++, Boolean));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(idx++, P2D));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(idx++, P3D));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBlob(idx++, &Blob, static_cast<int>(Blob.size()), IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGeometry(idx++, *geom));

    //Binding Struct property
    IECSqlBinder& StructBinder = stmt.GetBinder(idx++);
    ASSERT_EQ(ECSqlStatus::Success, StructBinder["D1"].BindDouble(Double));
    ASSERT_EQ(ECSqlStatus::Success, StructBinder["I1"].BindInt(123));

    //Binding Array property
    IECSqlBinder& ArrayBinder = stmt.GetBinder(idx++).AddArrayElement();
    for (size_t i = 0; i < 3; i++)
        {
        ASSERT_EQ(ECSqlStatus::Success, ArrayBinder.BindDouble(Array[i]));
        }

    //Binding Array of Point2d
    IECSqlBinder& arrayOfP2d = stmt.GetBinder(idx++);
    for (size_t i = 0; i < 3; i++)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayOfP2d.AddArrayElement().BindPoint2d(ArrayOfP2d[i]));
        }

    //Binding Array of Point3d
    IECSqlBinder& arrayOfP3d = stmt.GetBinder(idx++);
    for (size_t i = 0; i < 3; i++)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayOfP3d.AddArrayElement().BindPoint3d(ArrayOfP3d[i]));
        }

    //Binding Array of Struct
    IECSqlBinder& arrayOfST2 = stmt.GetBinder(idx++);
    for (size_t i = 0; i < 3; i++)
        {
        IECSqlBinder& elementBinder = arrayOfST2.AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["D2"].BindDouble(ArrayOfST2_D2[i]));
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["P3D"].BindPoint3d(ArrayOfST2_P3D[i]));
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["ST2P"]["D1"].BindDouble(ArrayOfST1_D1[i]));
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["ST2P"]["I1"].BindInt(ArrayOfST1_I1[i]));
        }

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    m_ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));

    ECInstanceKey changeSummaryKey;
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummaryKey, ChangeSetArg(changeset)));
    m_ecdb.SaveChanges();
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT S, P2D FROM ts.Element.Changes(?,?)"));

    stmt.BindId(1, changeSummaryKey.GetInstanceId());
    stmt.BindInt(2, (int) ChangedValueState::AfterInsert);
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    12/16
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, Overflow_PrimitiveProperties)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("overflowProperties.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Element" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00">
                      <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="Code" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="TestElement" modifier="None">
                <BaseClass>Element</BaseClass>
                <ECProperty propertyName="S" typeName="string"/>
                <ECProperty propertyName="I" typeName="int"/>
                <ECProperty propertyName="L" typeName="long"/>
                <ECProperty propertyName="D" typeName="double"/>
                <ECProperty propertyName="DT" typeName="dateTime"/>
                <ECProperty propertyName="B" typeName="boolean"/>
            </ECEntityClass>
        </ECSchema>)xml")));
    ASSERT_EQ(BE_SQLITE_OK, AttachCache());

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.TestElement(Code,S,I,L,D,DT,B) VALUES('C1', 'Str', 123, 12345, 23.5453, TIMESTAMP '2013-02-09T12:00:00', true)"));
    tracker.EnableTracking(false);
    m_ecdb.SaveChanges();
    ASSERT_TRUE(tracker.HasChanges());
    TestChangeSet changeset;
    ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));

    ECInstanceKey changeSummaryKey;
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummaryKey, ChangeSetArg(changeset)));

    /*
    BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
    AccessString;OldValue;NewValue
    0:1;TestSchema:TestElement:60;Insert;No
    B;NULL;1
    Code;NULL;"C1"
    D;NULL;23.5453
    DT;NULL;2.45633e+06
    I;NULL;123
    L;NULL;12345
    S;NULL;"Str"
    */
    DumpChangeSummary(changeSummaryKey, "Overflow_PrimitiveProperties");

    EXPECT_EQ(1, GetInstanceChangeCount(changeSummaryKey.GetInstanceId()));
    EXPECT_EQ(7, GetPropertyValueChangeCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Maha.Nasir                    1/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, Overflow_StructProperty)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("overflowProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECStructClass typeName='StructProp' modifier='None'>"
        "        <ECProperty propertyName='S' typeName='string' readOnly='false'/>"
        "        <ECProperty propertyName='I' typeName='int'/>"
        "    </ECStructClass>"
        "    <ECEntityClass typeName='TestElement' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECStructProperty propertyName='SP' typeName='StructProp'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));
    ASSERT_EQ(BE_SQLITE_OK, AttachCache());

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECSqlStatement stmt;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestElement (Code, SP) VALUES ('C1', ?)"));

    IECSqlBinder& Binder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, Binder["S"].BindText("TestVal", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, Binder["I"].BindInt(123));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    m_ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));

    ECInstanceKey changeSummaryKey;
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummaryKey, ChangeSetArg(changeset)));

    /*
    Code:"C1"
    SP.S:"TestVal"
    SP.I:123
    */

    EXPECT_EQ(1, GetInstanceChangeCount(changeSummaryKey.GetInstanceId()));
    EXPECT_EQ(3, GetPropertyValueChangeCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Maha.Nasir                    1/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, Overflow_ArrayProperty)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("overflowProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestElement' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECArrayProperty propertyName='ArrayProp' typeName='double' minOccurs='0' maxOccurs='unbounded'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));
    ASSERT_EQ(BE_SQLITE_OK, AttachCache());

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECSqlStatement stmt;
    double Array[] = {123.3434, 345.223,-532.123};
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestElement (Code, ArrayProp) VALUES ('C1', ?)"));

    IECSqlBinder& Binder = stmt.GetBinder(1).AddArrayElement();
    for (size_t i = 0; i < 3; i++)
        {
        ASSERT_EQ(ECSqlStatus::Success, Binder.BindDouble(Array[i]));
        }
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    m_ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));

    ECInstanceKey changeSummaryKey;
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummaryKey, ChangeSetArg(changeset)));

    /*
    Code:"C1"
    ArrayProp: { 123.3434, 345.223,-532.123 }
    */
    EXPECT_EQ(1, GetInstanceChangeCount(changeSummaryKey.GetInstanceId()));
    EXPECT_EQ(2, GetPropertyValueChangeCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Maha.Nasir                    1/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, Overflow_ComplexPropertyTypes)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("overflowProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestElement' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECProperty propertyName='P2D' typeName='point2d'/>"
        "        <ECProperty propertyName='P3D' typeName='point3d'/>"
        "        <ECProperty propertyName='BIN' typeName='binary'/>"
        "        <ECProperty propertyName='Geom' typeName='Bentley.Geometry.Common.IGeometry'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));
    ASSERT_EQ(BE_SQLITE_OK, AttachCache());

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECSqlStatement stmt;
    std::vector<Utf8Char> bin = {'H', 'e', 'l','l', 'o'};
    IGeometryPtr geom = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestElement (Code, P2D, P3D, Bin, Geom) VALUES ('C1', ?, ?, ?, ?)"));

    //Binding Point 2d & 3d
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(1, DPoint2d::From(-21, 22.1)));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(2, DPoint3d::From(-12.53, 21.76, -32.22)));
    //Binding Binary blob
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBlob(3, &bin, static_cast<int>(bin.size()), IECSqlBinder::MakeCopy::No));
    //Binding Geometry
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGeometry(4, *geom));

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    m_ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));

    ECInstanceKey changeSummaryKey;
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummaryKey, ChangeSetArg(changeset)));

    /*
    Code:"C1"
    P2D.X: -21
    P2D.Y: 22.1
    P3D.X: -12.53
    P3D.Y: 21.76
    P3D.Z: -32.22
    Bin: { 'H', 'e', 'l','l', 'o' }
    Geom: null
    */
    EXPECT_EQ(1, GetInstanceChangeCount(changeSummaryKey.GetInstanceId()));
    EXPECT_EQ(8, GetPropertyValueChangeCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Maha.Nasir                    1/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, Overflow_ArrayOfPoints)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("overflowProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestElement' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECArrayProperty propertyName='arrayOfP2d' typeName='point2d' minOccurs='0' maxOccurs='unbounded'/>"
        "        <ECArrayProperty propertyName='arrayOfP3d' typeName='point3d' minOccurs='0' maxOccurs='unbounded'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));
    ASSERT_EQ(BE_SQLITE_OK, AttachCache());

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECSqlStatement stmt;

    DPoint2d ArrayOfP2d[] = {DPoint2d::From(-21, 22.1),DPoint2d::From(-85.34, 35.36),DPoint2d::From(-31.34, 12.35)};
    DPoint3d ArrayOfP3d[] = {DPoint3d::From(-41.33, 41.13, -12.25), DPoint3d::From(-23.37, 53.54, -34.31), DPoint3d::From(-33.41, 11.13, -99.11)};

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestElement (Code, arrayOfP2d, arrayOfP3d) VALUES ('C1', ?, ?)"));

    //Binding Array of Point2d
    IECSqlBinder& arrayOfP2d = stmt.GetBinder(1);
    for (size_t i = 0; i < 3; i++)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayOfP2d.AddArrayElement().BindPoint2d(ArrayOfP2d[i]));
        }

    //Binding Array of Point3d
    IECSqlBinder& arrayOfP3d = stmt.GetBinder(2);
    for (size_t i = 0; i < 3; i++)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayOfP3d.AddArrayElement().BindPoint3d(ArrayOfP3d[i]));
        }

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    m_ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));

    ECInstanceKey changeSummaryKey;
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummaryKey, ChangeSetArg(changeset)));

    EXPECT_EQ(1, GetInstanceChangeCount(changeSummaryKey.GetInstanceId()));
    EXPECT_EQ(3, GetPropertyValueChangeCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Maha.Nasir                    1/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, Overflow_ArrayOfStructs)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("overflowProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECStructClass typeName='ST2' modifier='None'>"
        "        <ECProperty propertyName='D2' typeName='double' readOnly='false'/>"
        "        <ECProperty propertyName='P3D' typeName='point3d'/>"
        "    </ECStructClass>"
        "    <ECStructClass typeName='ST1' modifier='None'>"
        "        <ECProperty propertyName='D1' typeName='double' readOnly='false'/>"
        "        <ECProperty propertyName='P2D' typeName='point2d'/>"
        "        <ECStructProperty propertyName='ST2P' typeName='ST2'/>"
        "    </ECStructClass>"
        "    <ECEntityClass typeName='TestElement' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECStructArrayProperty propertyName='arrayOfST1' typeName='ST1' minOccurs='0' maxOccurs='unbounded'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));
    ASSERT_EQ(BE_SQLITE_OK, AttachCache());

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECSqlStatement stmt;

    //Binding array of Struct
    double ArrayOfST1_D1[] = {123.3434, 345.223,-532.123};
    DPoint2d ArrayOfST1_P2D[] = {DPoint2d::From(-21, 22.1),DPoint2d::From(-85.34, 35.36),DPoint2d::From(-31.34, 12.35)};
    double ArrayOfST1_D2[] = {12.3, -45.72, -31.11};
    DPoint3d ArrayOfST1_P3D[] = {DPoint3d::From(-12.11, -74.1, 12.3),DPoint3d::From(-12.53, 21.76, -32.22),DPoint3d::From(-41.14, -22.45, -31.16)};

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestElement (Code, arrayOfST1) VALUES ('C1', ?)"));

    //Binding Array of Struct
    IECSqlBinder& arrayOfST1 = stmt.GetBinder(1);
    for (size_t i = 0; i < 3; i++)
        {
        IECSqlBinder& elementBinder = arrayOfST1.AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["D1"].BindDouble(ArrayOfST1_D1[i]));
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["P2D"].BindPoint2d(ArrayOfST1_P2D[i]));
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["ST2P"]["D2"].BindDouble(ArrayOfST1_D2[i]));
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["ST2P"]["P3D"].BindPoint3d(ArrayOfST1_P3D[i]));
        }

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    m_ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));

    ECInstanceKey changeSummaryKey;
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummaryKey, ChangeSetArg(changeset)));

    EXPECT_EQ(1, GetInstanceChangeCount(changeSummaryKey.GetInstanceId()));
    EXPECT_EQ(2, GetPropertyValueChangeCount());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, RelationshipChangesFromCurrentTransaction)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("RelationshipChangesFromCurrentTransaction.ecdb", SchemaItem::CreateForFile("StartupCompany.02.00.00.ecschema.xml")));
    ASSERT_EQ(BE_SQLITE_OK, AttachCache());
    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECInstanceKey employeeKey, companyKey1, companyKey2, hardwareKey1, hardwareKey2;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(employeeKey, "INSERT INTO StartupCompany.Employee (FirstName,LastName) VALUES('John','Doe')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(companyKey1, "INSERT INTO StartupCompany.Company (Name) VALUES('AcmeWorks')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(companyKey2, "INSERT INTO StartupCompany.Company (Name) VALUES('CmeaWorks')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(hardwareKey1, "INSERT INTO StartupCompany.Hardware (Make,Model) VALUES('Tesla', 'Model-S')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(hardwareKey2, "INSERT INTO StartupCompany.Hardware (Make,Model) VALUES('Toyota', 'Prius')"));

    TestChangeSet changeset;
    ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));

    ECInstanceKey changeSummaryKey;
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummaryKey, ChangeSetArg(changeset)));

    DumpChangeSummary(changeSummaryKey, "ChangeSummary after inserting instances");

    /*
    ChangeSummary after inserting instances:
    BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
    AccessString;OldValue;NewValue
    0:1;StartupCompany:Employee:79;Insert;No
    FirstName;NULL;"John"
    LastName;NULL;"Doe"
    0:2;StartupCompany:Company:71;Insert;No
    Name;NULL;"AcmeWorks"
    0:3;StartupCompany:Company:71;Insert;No
    Name;NULL;"CmeaWorks"
    0:4;StartupCompany:Hardware:75;Insert;No
    Make;NULL;"Tesla"
    Model;NULL;"Model-S"
    0:5;StartupCompany:Hardware:75;Insert;No
    Make;NULL;"Toyota"
    Model;NULL;"Prius"
    */
    EXPECT_EQ(5, GetInstanceChangeCount(changeSummaryKey.GetInstanceId()));
    EXPECT_TRUE(ContainsChange(changeSummaryKey.GetInstanceId(), ECInstanceId(employeeKey.GetInstanceId().GetValueUnchecked()), "StartupCompany", "Employee", ChangeOpCode::Insert));
    EXPECT_TRUE(ContainsChange(changeSummaryKey.GetInstanceId(), ECInstanceId(companyKey1.GetInstanceId().GetValueUnchecked()), "StartupCompany", "Company", ChangeOpCode::Insert));
    EXPECT_TRUE(ContainsChange(changeSummaryKey.GetInstanceId(), ECInstanceId(companyKey2.GetInstanceId().GetValueUnchecked()), "StartupCompany", "Company", ChangeOpCode::Insert));
    EXPECT_TRUE(ContainsChange(changeSummaryKey.GetInstanceId(), ECInstanceId(hardwareKey1.GetInstanceId().GetValueUnchecked()), "StartupCompany", "Hardware", ChangeOpCode::Insert));
    EXPECT_TRUE(ContainsChange(changeSummaryKey.GetInstanceId(), ECInstanceId(hardwareKey2.GetInstanceId().GetValueUnchecked()), "StartupCompany", "Hardware", ChangeOpCode::Insert));

    m_ecdb.SaveChanges();
    tracker.Restart();

    ECSqlStatement statement;
    statement.Prepare(m_ecdb, "INSERT INTO StartupCompany.EmployeeHardware (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindId(1, employeeKey.GetClassId());
    statement.BindId(2, employeeKey.GetInstanceId());
    statement.BindId(3, hardwareKey1.GetClassId());
    statement.BindId(4, hardwareKey1.GetInstanceId());

    ECInstanceKey employeeHardwareKey;
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(employeeHardwareKey));

    changeset.Free();

    ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummaryKey, ChangeSetArg(changeset)));

    DumpChangeSummary(changeSummaryKey, "ChangeSummary after inserting relationships");

    /*
    ChangeSummary after inserting relationships:
    BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
    AccessString;OldValue;NewValue
    0:6;StartupCompany:EmployeeHardware:82;Insert;No
    SourceECClassId;NULL;StartupCompany:Employee:79
    SourceECInstanceId;NULL;0:1
    TargetECClassId;NULL;StartupCompany:Hardware:75
    TargetECInstanceId;NULL;0:4
    */
    EXPECT_EQ(1, GetInstanceChangeCount(changeSummaryKey.GetInstanceId()));
    EXPECT_TRUE(ContainsChange(changeSummaryKey.GetInstanceId(), ECInstanceId(employeeHardwareKey.GetInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", ChangeOpCode::Insert));

    m_ecdb.SaveChanges();
    tracker.Restart();

    /*
    * Note: ECDb doesn't support updates of relationships directly. Can only delete and re-insert relationships
    */
    statement.Finalize();
    statement.Prepare(m_ecdb, "DELETE FROM StartupCompany.EmployeeHardware WHERE EmployeeHardware.ECInstanceId=?");
    statement.BindId(1, employeeHardwareKey.GetInstanceId());
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

    statement.Finalize();
    statement.Prepare(m_ecdb, "INSERT INTO StartupCompany.EmployeeHardware (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindId(1, employeeKey.GetClassId());
    statement.BindId(2, employeeKey.GetInstanceId());
    statement.BindId(3, hardwareKey2.GetClassId());
    statement.BindId(4, hardwareKey2.GetInstanceId());

    ECInstanceKey employeeHardwareKey2;
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(employeeHardwareKey2));

    changeset.Free();
    ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummaryKey, ChangeSetArg(changeset)));

    DumpChangeSummary(changeSummaryKey, "ChangeSummary after updating (deleting and inserting different) relationships");

    /*
    ChangeSummary after updating (deleting and inserting different) relationships:
    BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
    AccessString;OldValue;NewValue
    0:6;StartupCompany:EmployeeHardware:82;Delete;No
    SourceECClassId;StartupCompany:Employee:79;NULL
    SourceECInstanceId;0:1;NULL
    TargetECClassId;StartupCompany:Hardware:75;NULL
    TargetECInstanceId;0:4;NULL
    0:7;StartupCompany:EmployeeHardware:82;Insert;No
    SourceECClassId;NULL;StartupCompany:Employee:79
    SourceECInstanceId;NULL;0:1
    TargetECClassId;NULL;StartupCompany:Hardware:75
    TargetECInstanceId;NULL;0:5
    */
    EXPECT_EQ(2, GetInstanceChangeCount(changeSummaryKey.GetInstanceId()));
    EXPECT_TRUE(ContainsChange(changeSummaryKey.GetInstanceId(), ECInstanceId(employeeHardwareKey.GetInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", ChangeOpCode::Delete));
    EXPECT_TRUE(ContainsChange(changeSummaryKey.GetInstanceId(), ECInstanceId(employeeHardwareKey2.GetInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", ChangeOpCode::Insert));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    12/16
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, OverflowTables)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("overflowTables.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='GrandParent' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='string' />"
        "        <ECProperty propertyName='B' typeName='string' />"
        "        <ECProperty propertyName='C' typeName='string' />"
        "        <ECProperty propertyName='D' typeName='string' />"
        "        <ECProperty propertyName='E' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <BaseClass>GrandParent</BaseClass>"
        "        <ECCustomAttributes>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.2.0'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='F' typeName='string'/>"
        "        <ECProperty propertyName='G' typeName='string'/>"
        "        <ECProperty propertyName='H' typeName='string'/>"
        "        <ECProperty propertyName='I' typeName='string'/>"
        "        <ECProperty propertyName='J' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Child' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='K' typeName='string'/>"
        "        <ECProperty propertyName='L' typeName='string'/>"
        "        <ECProperty propertyName='M' typeName='string'/>"
        "        <ECProperty propertyName='N' typeName='string'/>"
        "        <ECProperty propertyName='O' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='GrandChild' modifier='None'>"
        "        <BaseClass>Child</BaseClass>"
        "        <ECProperty propertyName='P' typeName='string'/>"
        "        <ECProperty propertyName='Q' typeName='string'/>"
        "        <ECProperty propertyName='R' typeName='string'/>"
        "        <ECProperty propertyName='S' typeName='string'/>"
        "        <ECProperty propertyName='T' typeName='string'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));
    ASSERT_EQ(BE_SQLITE_OK, AttachCache());

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECSqlStatement stmt;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.GrandChild (A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T) VALUES ('A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T')"));

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    m_ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));
    ECInstanceKey changeSummaryKey;
    ASSERT_EQ(SUCCESS, m_ecdb.ExtractChangeSummary(changeSummaryKey, ChangeSetArg(changeset)));

    /*
    BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
    AccessString;OldValue;NewValue
    0:1;TestSchema:GrandChild:53;Insert;No
    A;NULL;"A"
    B;NULL;"B"
    C;NULL;"C"
    D;NULL;"D"
    E;NULL;"E"
    F;NULL;"F"
    G;NULL;"G"
    H;NULL;"H"
    I;NULL;"I"
    J;NULL;"J"
    K;NULL;"K"
    L;NULL;"L"
    M;NULL;"M"
    N;NULL;"N"
    O;NULL;"O"
    P;NULL;"P"
    Q;NULL;"Q"
    R;NULL;"R"
    S;NULL;"S"
    T;NULL;"T"
    */
    DumpChangeSummary(changeSummaryKey, "OverflowTables");
    EXPECT_EQ(1, GetInstanceChangeCount(changeSummaryKey.GetInstanceId()));
    EXPECT_EQ(20, GetPropertyValueChangeCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                       11/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangeSummaryTestFixture, NoChangeTrackingInAttachedFile)
    {
    BeFileName primaryFilePath = ECDbTestFixture::BuildECDbPath("pri.db");
    BeFileName attachedFilePath = ECDbTestFixture::BuildECDbPath("sec.db");

    Db primaryDb, attachedDb;
    ASSERT_EQ(BE_SQLITE_OK, primaryDb.CreateNewDb(primaryFilePath));
    ASSERT_EQ(BE_SQLITE_OK, attachedDb.CreateNewDb(attachedFilePath));
    ASSERT_EQ(BE_SQLITE_OK, primaryDb.ExecuteSql("create table FooP(id integer primary key, prop text)"));
    ASSERT_EQ(BE_SQLITE_OK, primaryDb.ExecuteSql("create table Goo(id integer primary key, city text, pop int)"));
    ASSERT_EQ(BE_SQLITE_OK, attachedDb.ExecuteSql("create table FooS(id integer primary key, prop text)"));
    primaryDb.CloseDb();
    attachedDb.CloseDb();

    ASSERT_EQ(BE_SQLITE_OK, primaryDb.OpenBeSQLiteDb(primaryFilePath, Db::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::Yes)));
    primaryDb.AttachDb(attachedFilePath.GetNameUtf8().c_str(), "sec");

    TestChangeTracker tracker(primaryDb);
    tracker.EnableTracking(true);

    ASSERT_EQ(BE_SQLITE_OK, primaryDb.ExecuteSql("insert into fooP(id,prop) values(1, 'cat')"));
    ASSERT_EQ(BE_SQLITE_OK, primaryDb.ExecuteSql("insert into fooP(id,prop) values(2, 'bird')"));

    ASSERT_EQ(BE_SQLITE_OK, primaryDb.ExecuteSql("insert into Goo(id,city,pop) values(20, 'boston', 200000)"));
    ASSERT_EQ(BE_SQLITE_OK, primaryDb.ExecuteSql("insert into Goo(id,city,pop) values(30, 'paris', 300000)"));

    ASSERT_EQ(BE_SQLITE_OK, primaryDb.ExecuteSql("insert into fooS(id,prop) values(101, 'dino')"));
    ASSERT_EQ(BE_SQLITE_OK, primaryDb.ExecuteSql("insert into fooS(id,prop) values(102, 'zibra')"));

    //----------------------------------------------------------------------------------
    TestChangeSet rev0;
    rev0.FromChangeTrack(tracker, IChangeSet::SetType::Full);
    tracker.Restart();
    ASSERT_EQ(BE_SQLITE_OK, primaryDb.ExecuteSql("insert into fooP(id,prop) values(3, 'dog')"));
    ASSERT_EQ(BE_SQLITE_OK, primaryDb.ExecuteSql("update fooP set prop='crow' where id =2"));
    ASSERT_EQ(BE_SQLITE_OK, primaryDb.ExecuteSql("delete from fooP where id=1"));

    ASSERT_EQ(BE_SQLITE_OK, primaryDb.ExecuteSql("insert into Goo(id,city,pop) values(10, 'london', 100000)"));
    ASSERT_EQ(BE_SQLITE_OK, primaryDb.ExecuteSql("update Goo set city='peshawar', pop=400000 where id =30"));
    ASSERT_EQ(BE_SQLITE_OK, primaryDb.ExecuteSql("delete from Goo where id=20"));

    ASSERT_EQ(BE_SQLITE_OK, primaryDb.ExecuteSql("insert into fooS(id,prop) values(100, 'cow')"));
    ASSERT_EQ(BE_SQLITE_OK, primaryDb.ExecuteSql("update fooS set prop='tiger' where id =101"));
    ASSERT_EQ(BE_SQLITE_OK, primaryDb.ExecuteSql("delete from fooS where id=102"));

    //----------------------------------------------------------------------------------
    TestChangeSet rev1;
    rev1.FromChangeTrack(tracker, IChangeSet::SetType::Full);
    tracker.EnableTracking(false);

    //Verify
    //----------------------------------------------------------------------------------
    Json::Value expectedChangeset0Json, expectedChangeset1Json;
    ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(expectedChangeset0Json, R"json(
        [{"pk":["id"],"rows":
            [{"indirect":false,"op":"insert",
                "values":[{"id":{"after": 1}},
                          {"prop":{"after": "cat"}}]},
             {"indirect":false,"op":"insert",
                "values":[{"id":{"after": 2}},
                          {"prop":{"after": "bird"}}]}
              ], "table":"FooP"},
            {"pk":["id"],"rows":
            [{"indirect":false,"op":"insert",
               "values":[{"id": {"after": 30}},
                        {"city":{"after": "paris"}},
                        {"pop":{"after": 300000}}]},
            {"indirect":false,"op":"insert",
                "values":[{"id":{"after": 20}},
                          {"city":{"after": "boston"}},
                          {"pop":{"after": 200000}}]}
            ],
            "table":"Goo"}])json"));
    ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(expectedChangeset1Json, R"json(
        [{"pk":["id"],
        "rows": [{"indirect":false,"op":"delete",
                    "values": [{"id":{"before":1}}, {"prop":{"before":"cat"}}]},
                {"indirect":false,"op":"update",
                    "values": [{"id":{"before":2}}, {"prop":{"before":"bird","after":"crow"}}]},
                {"indirect":false,"op":"insert",
                    "values": [{"id":{"after":3}}, {"prop":{"after":"dog"}}]}],
         "table":"FooP"},
        {"pk":["id"],
         "rows": [{"indirect":false,"op":"insert",
                      "values": [{"id":{"after": 10}}, {"city":{"after": "london"}}, {"pop":{"after": 100000}}]},
                  {"indirect":false,"op":"update",
                      "values": [{"id":{"before": 30}}, {"city":{"before": "paris","after": "peshawar"}}, {"pop":{"before": 300000, "after": 400000}}]},
                 {"indirect":false,"op":"delete",
                     "values": [{"id":{"before":20}}, {"city":{"before":"boston"}}, {"pop":{"before":200000}}]}],
         "table":"Goo"}])json"));

    ASSERT_EQ(JsonValue(expectedChangeset0Json), rev0.ToJson(primaryDb));
    ASSERT_EQ(JsonValue(expectedChangeset1Json), rev1.ToJson(primaryDb));

    primaryDb.AbandonChanges();

    //But transaction does cover multiple files and canceling will loose change across multiple files
    ASSERT_EQ(BE_SQLITE_DONE, primaryDb.GetCachedStatement("select 1 from FooP")->Step());
    ASSERT_EQ(BE_SQLITE_DONE, primaryDb.GetCachedStatement("select 1 from Goo")->Step());
    ASSERT_EQ(BE_SQLITE_DONE, primaryDb.GetCachedStatement("select 1 from FooS")->Step());
    }


//*************************************
// Deprecated API tests
//*************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    12/16
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixtureV1, SchemaChange)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("invalidsummarytest.ecdb"));

    // Test1: Change to be_Prop table - should cause empty change summary without errors
    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SavePropertyString(PropertySpec("TestName", "TestNamespace"), "TestValue"));

    TestChangeSet changeSet;
    ASSERT_EQ(BE_SQLITE_OK, changeSet.FromChangeTrack(tracker));

    ChangeSummary changeSummary(m_ecdb);
    ASSERT_EQ(SUCCESS, changeSummary.FromChangeSet(changeSet));

    // Test2: Change to ec_ tables - should cause an error creating a change summary
    tracker.Restart();

    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'> "
        "</ECSchema>")));

    changeSet.Free();
    ASSERT_EQ(BE_SQLITE_OK, changeSet.FromChangeTrack(tracker));

    changeSummary.Free();
    ASSERT_EQ(SUCCESS, changeSummary.FromChangeSet(changeSet));
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    12/16
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixtureV1, Overflow_PrimitiveProperties)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("overflowProperties.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Element" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00">
                      <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="Code" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="TestElement" modifier="None">
                <BaseClass>Element</BaseClass>
                <ECProperty propertyName="S" typeName="string"/>
                <ECProperty propertyName="I" typeName="int"/>
                <ECProperty propertyName="L" typeName="long"/>
                <ECProperty propertyName="D" typeName="double"/>
                <ECProperty propertyName="DT" typeName="dateTime"/>
                <ECProperty propertyName="B" typeName="boolean"/>
            </ECEntityClass>
        </ECSchema>)xml")));

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.TestElement(Code,S,I,L,D,DT,B) VALUES('C1', 'Str', 123, 12345, 23.5453, TIMESTAMP '2013-02-09T12:00:00', true)"));
    m_ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));

    ChangeSummary changeSummary(m_ecdb);
    ASSERT_EQ(SUCCESS, changeSummary.FromChangeSet(changeset));

    /*
    BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
    AccessString;OldValue;NewValue
    0:1;TestSchema:TestElement:60;Insert;No
    B;NULL;1
    Code;NULL;"C1"
    D;NULL;23.5453
    DT;NULL;2.45633e+06
    I;NULL;123
    L;NULL;12345
    S;NULL;"Str"
    */
    DumpChangeSummary(changeSummary, "Overflow_PrimitiveProperties");

    ChangeSummary::InstanceIterator instIter = changeSummary.MakeInstanceIterator();
    EXPECT_EQ(1, instIter.QueryCount());

    ChangeSummary::ValueIterator valIter = instIter.begin().GetInstance().MakeValueIterator();
    EXPECT_EQ(7, valIter.QueryCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Maha.Nasir                    1/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixtureV1, Overflow_StructProperty)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("overflowProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECStructClass typeName='StructProp' modifier='None'>"
        "        <ECProperty propertyName='S' typeName='string' readOnly='false'/>"
        "        <ECProperty propertyName='I' typeName='int'/>"
        "    </ECStructClass>"
        "    <ECEntityClass typeName='TestElement' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECStructProperty propertyName='SP' typeName='StructProp'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECSqlStatement stmt;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestElement (Code, SP) VALUES ('C1', ?)"));

    IECSqlBinder& Binder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, Binder["S"].BindText("TestVal", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, Binder["I"].BindInt(123));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    m_ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    auto rc = changeset.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == rc);

    ChangeSummary changeSummary(m_ecdb);
    BentleyStatus status = changeSummary.FromChangeSet(changeset);
    ASSERT_TRUE(SUCCESS == status);

    /*
    Code:"C1"
    SP.S:"TestVal"
    SP.I:123
    */

    ChangeSummary::InstanceIterator instIter = changeSummary.MakeInstanceIterator();
    EXPECT_EQ(1, instIter.QueryCount());

    ChangeSummary::ValueIterator valIter = instIter.begin().GetInstance().MakeValueIterator();
    EXPECT_EQ(3, valIter.QueryCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Maha.Nasir                    1/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixtureV1, Overflow_ArrayProperty)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("overflowProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestElement' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECArrayProperty propertyName='ArrayProp' typeName='double' minOccurs='0' maxOccurs='unbounded'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECSqlStatement stmt;
    double Array[] = { 123.3434, 345.223,-532.123 };
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestElement (Code, ArrayProp) VALUES ('C1', ?)"));

    IECSqlBinder& Binder = stmt.GetBinder(1).AddArrayElement();
    for (size_t i = 0; i < 3; i++)
        {
        ASSERT_EQ(ECSqlStatus::Success, Binder.BindDouble(Array[i]));
        }
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    m_ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    auto rc = changeset.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == rc);

    ChangeSummary changeSummary(m_ecdb);
    BentleyStatus status = changeSummary.FromChangeSet(changeset);
    ASSERT_TRUE(SUCCESS == status);

    /*
    Code:"C1"
    ArrayProp: { 123.3434, 345.223,-532.123 }
    */

    ChangeSummary::InstanceIterator instIter = changeSummary.MakeInstanceIterator();
    EXPECT_EQ(1, instIter.QueryCount());

    ChangeSummary::ValueIterator valIter = instIter.begin().GetInstance().MakeValueIterator();
    EXPECT_EQ(2, valIter.QueryCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Maha.Nasir                    1/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixtureV1, Overflow_ComplexPropertyTypes)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("overflowProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestElement' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECProperty propertyName='P2D' typeName='point2d'/>"
        "        <ECProperty propertyName='P3D' typeName='point3d'/>"
        "        <ECProperty propertyName='BIN' typeName='binary'/>"
        "        <ECProperty propertyName='Geom' typeName='Bentley.Geometry.Common.IGeometry'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECSqlStatement stmt;
    std::vector<Utf8Char> bin = { 'H', 'e', 'l','l', 'o' };
    IGeometryPtr geom = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestElement (Code, P2D, P3D, Bin, Geom) VALUES ('C1', ?, ?, ?, ?)"));

    //Binding Point 2d & 3d
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(1, DPoint2d::From(-21, 22.1)));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(2, DPoint3d::From(-12.53, 21.76, -32.22)));
    //Binding Binary blob
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBlob(3, &bin, static_cast<int>(bin.size()), IECSqlBinder::MakeCopy::No));
    //Binding Geometry
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGeometry(4, *geom));

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    m_ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    auto rc = changeset.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == rc);

    ChangeSummary changeSummary(m_ecdb);
    BentleyStatus status = changeSummary.FromChangeSet(changeset);
    ASSERT_TRUE(SUCCESS == status);

    /*
    Code:"C1"
    P2D.X: -21
    P2D.Y: 22.1
    P3D.X: -12.53
    P3D.Y: 21.76
    P3D.Z: -32.22
    Bin: { 'H', 'e', 'l','l', 'o' }
    */

    ChangeSummary::InstanceIterator instIter = changeSummary.MakeInstanceIterator();
    EXPECT_EQ(1, instIter.QueryCount());

    ChangeSummary::ValueIterator valIter = instIter.begin().GetInstance().MakeValueIterator();
    EXPECT_EQ(8, valIter.QueryCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Maha.Nasir                    1/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixtureV1, Overflow_ArrayOfPoints)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("overflowProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestElement' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECArrayProperty propertyName='arrayOfP2d' typeName='point2d' minOccurs='0' maxOccurs='unbounded'/>"
        "        <ECArrayProperty propertyName='arrayOfP3d' typeName='point3d' minOccurs='0' maxOccurs='unbounded'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECSqlStatement stmt;

    DPoint2d ArrayOfP2d[] = { DPoint2d::From(-21, 22.1),DPoint2d::From(-85.34, 35.36),DPoint2d::From(-31.34, 12.35) };
    DPoint3d ArrayOfP3d[] = { DPoint3d::From(-41.33, 41.13, -12.25), DPoint3d::From(-23.37, 53.54, -34.31), DPoint3d::From(-33.41, 11.13, -99.11) };

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestElement (Code, arrayOfP2d, arrayOfP3d) VALUES ('C1', ?, ?)"));

    //Binding Array of Point2d
    IECSqlBinder& arrayOfP2d = stmt.GetBinder(1);
    for (size_t i = 0; i < 3; i++)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayOfP2d.AddArrayElement().BindPoint2d(ArrayOfP2d[i]));
        }

    //Binding Array of Point3d
    IECSqlBinder& arrayOfP3d = stmt.GetBinder(2);
    for (size_t i = 0; i < 3; i++)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayOfP3d.AddArrayElement().BindPoint3d(ArrayOfP3d[i]));
        }

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    m_ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    auto rc = changeset.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == rc);

    ChangeSummary changeSummary(m_ecdb);
    BentleyStatus status = changeSummary.FromChangeSet(changeset);
    ASSERT_TRUE(SUCCESS == status);

    ChangeSummary::InstanceIterator instIter = changeSummary.MakeInstanceIterator();
    EXPECT_EQ(1, instIter.QueryCount());

    ChangeSummary::ValueIterator valIter = instIter.begin().GetInstance().MakeValueIterator();
    EXPECT_EQ(3, valIter.QueryCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Maha.Nasir                    1/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixtureV1, Overflow_ArrayOfStructs)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("overflowProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECStructClass typeName='ST2' modifier='None'>"
        "        <ECProperty propertyName='D2' typeName='double' readOnly='false'/>"
        "        <ECProperty propertyName='P3D' typeName='point3d'/>"
        "    </ECStructClass>"
        "    <ECStructClass typeName='ST1' modifier='None'>"
        "        <ECProperty propertyName='D1' typeName='double' readOnly='false'/>"
        "        <ECProperty propertyName='P2D' typeName='point2d'/>"
        "        <ECStructProperty propertyName='ST2P' typeName='ST2'/>"
        "    </ECStructClass>"
        "    <ECEntityClass typeName='TestElement' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECStructArrayProperty propertyName='arrayOfST1' typeName='ST1' minOccurs='0' maxOccurs='unbounded'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECSqlStatement stmt;

    //Binding array of Struct
    double ArrayOfST1_D1[] = { 123.3434, 345.223,-532.123 };
    DPoint2d ArrayOfST1_P2D[] = { DPoint2d::From(-21, 22.1),DPoint2d::From(-85.34, 35.36),DPoint2d::From(-31.34, 12.35) };
    double ArrayOfST1_D2[] = { 12.3, -45.72, -31.11 };
    DPoint3d ArrayOfST1_P3D[] = { DPoint3d::From(-12.11, -74.1, 12.3),DPoint3d::From(-12.53, 21.76, -32.22),DPoint3d::From(-41.14, -22.45, -31.16) };

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestElement (Code, arrayOfST1) VALUES ('C1', ?)"));

    //Binding Array of Struct
    IECSqlBinder& arrayOfST1 = stmt.GetBinder(1);
    for (size_t i = 0; i < 3; i++)
        {
        IECSqlBinder& elementBinder = arrayOfST1.AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["D1"].BindDouble(ArrayOfST1_D1[i]));
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["P2D"].BindPoint2d(ArrayOfST1_P2D[i]));
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["ST2P"]["D2"].BindDouble(ArrayOfST1_D2[i]));
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["ST2P"]["P3D"].BindPoint3d(ArrayOfST1_P3D[i]));
        }

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    m_ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    auto rc = changeset.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == rc);

    ChangeSummary changeSummary(m_ecdb);
    BentleyStatus status = changeSummary.FromChangeSet(changeset);
    ASSERT_TRUE(SUCCESS == status);

    ChangeSummary::InstanceIterator instIter = changeSummary.MakeInstanceIterator();
    EXPECT_EQ(1, instIter.QueryCount());

    ChangeSummary::ValueIterator valIter = instIter.begin().GetInstance().MakeValueIterator();
    EXPECT_EQ(2, valIter.QueryCount());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixtureV1, RelationshipChangesFromCurrentTransaction)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("RelationshipChangesFromCurrentTransaction.ecdb", SchemaItem::CreateForFile("StartupCompany.02.00.00.ecschema.xml")));
    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    // Insert Employee - FirstName, LastName
    // Insert Company - Name
    // Insert Hardware - Make (String), Model (String)
    // Insert EmployeeCompany - End Table relationship (Company__trg_11_id)
    // Insert EmployeeHardware - Link Table relationship

    ECSqlStatement statement;
    statement.Prepare(m_ecdb, "INSERT INTO StartupCompany.Employee (FirstName,LastName) VALUES('John','Doe')");
    ECInstanceKey employeeKey;
    DbResult stepStatus = statement.Step(employeeKey);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(m_ecdb, "INSERT INTO StartupCompany.Company (Name) VALUES('AcmeWorks')");
    ECInstanceKey companyKey1;
    stepStatus = statement.Step(companyKey1);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(m_ecdb, "INSERT INTO StartupCompany.Company (Name) VALUES('CmeaWorks')");
    ECInstanceKey companyKey2;
    stepStatus = statement.Step(companyKey2);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(m_ecdb, "INSERT INTO StartupCompany.Hardware (Make,Model) VALUES('Tesla', 'Model-S')");
    ECInstanceKey hardwareKey1;
    stepStatus = statement.Step(hardwareKey1);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(m_ecdb, "INSERT INTO StartupCompany.Hardware (Make,Model) VALUES('Toyota', 'Prius')");
    ECInstanceKey hardwareKey2;
    stepStatus = statement.Step(hardwareKey2);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    TestChangeSet changeSet;
    DbResult result = changeSet.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == result);

    ChangeSummary changeSummary(m_ecdb);
    BentleyStatus status = changeSummary.FromChangeSet(changeSet);
    ASSERT_TRUE(SUCCESS == status);

    DumpChangeSummary(changeSummary, "ChangeSummary after inserting instances");

    /*
        ChangeSummary after inserting instances:
        BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
                AccessString;OldValue;NewValue
        0:1;StartupCompany:Employee:79;Insert;No
                FirstName;NULL;"John"
                LastName;NULL;"Doe"
        0:2;StartupCompany:Company:71;Insert;No
                Name;NULL;"AcmeWorks"
        0:3;StartupCompany:Company:71;Insert;No
                Name;NULL;"CmeaWorks"
        0:4;StartupCompany:Hardware:75;Insert;No
                Make;NULL;"Tesla"
                Model;NULL;"Model-S"
        0:5;StartupCompany:Hardware:75;Insert;No
                Make;NULL;"Toyota"
                Model;NULL;"Prius"
    */
    EXPECT_EQ(5, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(m_ecdb, changeSummary, ECInstanceId(employeeKey.GetInstanceId().GetValueUnchecked()), "StartupCompany", "Employee", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(m_ecdb, changeSummary, ECInstanceId(companyKey1.GetInstanceId().GetValueUnchecked()), "StartupCompany", "Company", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(m_ecdb, changeSummary, ECInstanceId(companyKey2.GetInstanceId().GetValueUnchecked()), "StartupCompany", "Company", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(m_ecdb, changeSummary, ECInstanceId(hardwareKey1.GetInstanceId().GetValueUnchecked()), "StartupCompany", "Hardware", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(m_ecdb, changeSummary, ECInstanceId(hardwareKey2.GetInstanceId().GetValueUnchecked()), "StartupCompany", "Hardware", DbOpcode::Insert));

    m_ecdb.SaveChanges();
    tracker.Restart();

    statement.Finalize();
    statement.Prepare(m_ecdb, "INSERT INTO StartupCompany.EmployeeHardware (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindId(1, employeeKey.GetClassId());
    statement.BindId(2, employeeKey.GetInstanceId());
    statement.BindId(3, hardwareKey1.GetClassId());
    statement.BindId(4, hardwareKey1.GetInstanceId());

    ECInstanceKey employeeHardwareKey;
    stepStatus = statement.Step(employeeHardwareKey);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    changeSummary.Free();
    changeSet.Free();

    result = changeSet.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == result);
    
    status = changeSummary.FromChangeSet(changeSet);
    ASSERT_TRUE(SUCCESS == status);

    DumpChangeSummary(changeSummary, "ChangeSummary after inserting relationships");

    /*
        ChangeSummary after inserting relationships:
        BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
                AccessString;OldValue;NewValue
        0:6;StartupCompany:EmployeeHardware:82;Insert;No
                SourceECClassId;NULL;StartupCompany:Employee:79
                SourceECInstanceId;NULL;0:1
                TargetECClassId;NULL;StartupCompany:Hardware:75
                TargetECInstanceId;NULL;0:4
    */
    EXPECT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(m_ecdb, changeSummary, ECInstanceId(employeeHardwareKey.GetInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Insert));

    m_ecdb.SaveChanges();
    tracker.Restart();

    /* 
    * Note: ECDb doesn't support updates of relationships directly. Can only delete and re-insert relationships
    */
    statement.Finalize();
    statement.Prepare(m_ecdb, "DELETE FROM StartupCompany.EmployeeHardware WHERE EmployeeHardware.ECInstanceId=?");
    statement.BindId(1, employeeHardwareKey.GetInstanceId());
    stepStatus = statement.Step();
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(m_ecdb, "INSERT INTO StartupCompany.EmployeeHardware (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindId(1, employeeKey.GetClassId());
    statement.BindId(2, employeeKey.GetInstanceId());
    statement.BindId(3, hardwareKey2.GetClassId());
    statement.BindId(4, hardwareKey2.GetInstanceId());

    ECInstanceKey employeeHardwareKey2;
    stepStatus = statement.Step(employeeHardwareKey2);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    changeSummary.Free();
    changeSet.Free();

    result = changeSet.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == result);

    status = changeSummary.FromChangeSet(changeSet);
    ASSERT_TRUE(SUCCESS == status);

    DumpChangeSummary(changeSummary, "ChangeSummary after updating (deleting and inserting different) relationships");

    /*
        ChangeSummary after updating (deleting and inserting different) relationships:
        BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
                AccessString;OldValue;NewValue
        0:6;StartupCompany:EmployeeHardware:82;Delete;No
                SourceECClassId;StartupCompany:Employee:79;NULL
                SourceECInstanceId;0:1;NULL
                TargetECClassId;StartupCompany:Hardware:75;NULL
                TargetECInstanceId;0:4;NULL
        0:7;StartupCompany:EmployeeHardware:82;Insert;No
                SourceECClassId;NULL;StartupCompany:Employee:79
                SourceECInstanceId;NULL;0:1
                TargetECClassId;NULL;StartupCompany:Hardware:75
                TargetECInstanceId;NULL;0:5
    */
    EXPECT_EQ(2, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(m_ecdb, changeSummary, ECInstanceId(employeeHardwareKey.GetInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Delete));
    EXPECT_TRUE(ChangeSummaryContainsInstance(m_ecdb, changeSummary, ECInstanceId(employeeHardwareKey2.GetInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Insert));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    12/16
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixtureV1, OverflowTables)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("overflowTables.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='GrandParent' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='string' />"
        "        <ECProperty propertyName='B' typeName='string' />"
        "        <ECProperty propertyName='C' typeName='string' />"
        "        <ECProperty propertyName='D' typeName='string' />"
        "        <ECProperty propertyName='E' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <BaseClass>GrandParent</BaseClass>"
        "        <ECCustomAttributes>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.2.0'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='F' typeName='string'/>"
        "        <ECProperty propertyName='G' typeName='string'/>"
        "        <ECProperty propertyName='H' typeName='string'/>"
        "        <ECProperty propertyName='I' typeName='string'/>"
        "        <ECProperty propertyName='J' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Child' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='K' typeName='string'/>"
        "        <ECProperty propertyName='L' typeName='string'/>"
        "        <ECProperty propertyName='M' typeName='string'/>"
        "        <ECProperty propertyName='N' typeName='string'/>"
        "        <ECProperty propertyName='O' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='GrandChild' modifier='None'>"
        "        <BaseClass>Child</BaseClass>"
        "        <ECProperty propertyName='P' typeName='string'/>"
        "        <ECProperty propertyName='Q' typeName='string'/>"
        "        <ECProperty propertyName='R' typeName='string'/>"
        "        <ECProperty propertyName='S' typeName='string'/>"
        "        <ECProperty propertyName='T' typeName='string'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECSqlStatement stmt;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.GrandChild (A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T) VALUES ('A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T')"));

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    m_ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    auto rc = changeset.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == rc);

    ChangeSummary changeSummary(m_ecdb);
    BentleyStatus status = changeSummary.FromChangeSet(changeset);
    ASSERT_TRUE(SUCCESS == status);

    /*
    BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
            AccessString;OldValue;NewValue
    0:1;TestSchema:GrandChild:53;Insert;No
            A;NULL;"A"
            B;NULL;"B"
            C;NULL;"C"
            D;NULL;"D"
            E;NULL;"E"
            F;NULL;"F"
            G;NULL;"G"
            H;NULL;"H"
            I;NULL;"I"
            J;NULL;"J"
            K;NULL;"K"
            L;NULL;"L"
            M;NULL;"M"
            N;NULL;"N"
            O;NULL;"O"
            P;NULL;"P"
            Q;NULL;"Q"
            R;NULL;"R"
            S;NULL;"S"
            T;NULL;"T"
    */
    DumpChangeSummary(changeSummary, "OverflowTables");

    ChangeSummary::InstanceIterator instIter = changeSummary.MakeInstanceIterator();
    EXPECT_EQ(1, instIter.QueryCount());

    ChangeSummary::ValueIterator valIter = instIter.begin().GetInstance().MakeValueIterator();
    EXPECT_EQ(20, valIter.QueryCount());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                Maha.Nasir                    1/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixtureV1, PropertiesWithRegularColumns)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("overflowProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECStructClass typeName='ST1' modifier='None'>"
        "        <ECProperty propertyName='D1' typeName='double' readOnly='false'/>"
        "        <ECProperty propertyName='I1' typeName='int'/>"
        "    </ECStructClass>"
        "    <ECStructClass typeName='ST2' modifier='None'>"
        "        <ECProperty propertyName='D2' typeName='double' readOnly='false'/>"
        "        <ECProperty propertyName='P3D' typeName='point3d'/>"
        "        <ECStructProperty propertyName='ST2P' typeName='ST1'/>"
        "    </ECStructClass>"
        "    <ECEntityClass typeName='Element' modifier='None'>"
        "        <ECProperty propertyName='S' typeName='string'/>"
        "        <ECProperty propertyName='I' typeName='int'/>"
        "        <ECProperty propertyName='L' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='double'/>"
        "        <ECProperty propertyName='DT' typeName='dateTime'/>"
        "        <ECProperty propertyName='B' typeName='boolean'/>"
        "        <ECProperty propertyName='P2D' typeName='point2d'/>"
        "        <ECProperty propertyName='P3D' typeName='point3d'/>"
        "        <ECProperty propertyName='BIN' typeName='binary'/>"
        "        <ECProperty propertyName='Geom' typeName='Bentley.Geometry.Common.IGeometry'/>"
        "        <ECStructProperty propertyName='StructProp' typeName='ST1'/>"
        "        <ECArrayProperty propertyName='ArrayProp' typeName='double' minOccurs='0' maxOccurs='unbounded'/>"
        "        <ECArrayProperty propertyName='arrayOfP2d' typeName='point2d' minOccurs='0' maxOccurs='unbounded'/>"
        "        <ECArrayProperty propertyName='arrayOfP3d' typeName='point3d' minOccurs='0' maxOccurs='unbounded'/>"
        "        <ECStructArrayProperty propertyName='arrayOfST2' typeName='ST2' minOccurs='0' maxOccurs='unbounded'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    int idx = 1;
    Utf8CP String = "TestVal";
    int Integer = 132;
    int64_t Long = 1235678901;
    double Double = 2341.432;
    DateTime DT = DateTime(DateTime::Kind::Unspecified, 2017, 1, 17, 0, 0);
    bool Boolean = false;
    DPoint2d P2D = DPoint2d::From(22.33, -21.34);
    DPoint3d P3D = DPoint3d::From(12.13, -42.34, -93.12);
    IGeometryPtr geom = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));
    std::vector<Utf8Char> Blob = {'H', 'e', 'l','l', 'o'};
    double Array[] = {123.3434, 345.223,-532.123};
    DPoint2d ArrayOfP2d[] = {DPoint2d::From(-21, 22.1),DPoint2d::From(-85.34, 35.36),DPoint2d::From(-31.34, 12.35)};
    DPoint3d ArrayOfP3d[] = {DPoint3d::From(-41.33, 41.13, -12.25), DPoint3d::From(-23.37, 53.54, -34.31), DPoint3d::From(-33.41, 11.13, -99.11)};
    double ArrayOfST1_D1[] = {123.3434, 345.223,-532.123};
    int ArrayOfST1_I1[] = {012, 456, 789};
    double ArrayOfST2_D2[] = {12.3, -45.72, -31.11};
    DPoint3d ArrayOfST2_P3D[] = {DPoint3d::From(-12.11, -74.1, 12.3),DPoint3d::From(-12.53, 21.76, -32.22),DPoint3d::From(-41.14, -22.45, -31.16)};

    ECSqlStatement stmt;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Element (S, I, L, D, DT, B, P2D, P3D, BIN, Geom, StructProp, ArrayProp , arrayOfP2d, arrayOfP3d, arrayOfST2) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(idx++, String, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(idx++, Integer));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(idx++, Long));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(idx++, Double));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(idx++, DT));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBoolean(idx++, Boolean));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(idx++, P2D));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(idx++, P3D));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBlob(idx++, &Blob, static_cast<int>(Blob.size()), IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGeometry(idx++, *geom));

    //Binding Struct property
    IECSqlBinder& StructBinder = stmt.GetBinder(idx++);
    ASSERT_EQ(ECSqlStatus::Success, StructBinder["D1"].BindDouble(Double));
    ASSERT_EQ(ECSqlStatus::Success, StructBinder["I1"].BindInt(123));

    //Binding Array property
    IECSqlBinder& ArrayBinder = stmt.GetBinder(idx++).AddArrayElement();
    for (size_t i = 0; i < 3; i++)
        {
        ASSERT_EQ(ECSqlStatus::Success, ArrayBinder.BindDouble(Array[i]));
        }

    //Binding Array of Point2d
    IECSqlBinder& arrayOfP2d = stmt.GetBinder(idx++);
    for (size_t i = 0; i < 3; i++)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayOfP2d.AddArrayElement().BindPoint2d(ArrayOfP2d[i]));
        }

    //Binding Array of Point3d
    IECSqlBinder& arrayOfP3d = stmt.GetBinder(idx++);
    for (size_t i = 0; i < 3; i++)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayOfP3d.AddArrayElement().BindPoint3d(ArrayOfP3d[i]));
        }

    //Binding Array of Struct
    IECSqlBinder& arrayOfST2 = stmt.GetBinder(idx++);
    for (size_t i = 0; i < 3; i++)
        {
        IECSqlBinder& elementBinder = arrayOfST2.AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["D2"].BindDouble(ArrayOfST2_D2[i]));
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["P3D"].BindPoint3d(ArrayOfST2_P3D[i]));
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["ST2P"]["D1"].BindDouble(ArrayOfST1_D1[i]));
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["ST2P"]["I1"].BindInt(ArrayOfST1_I1[i]));
        }

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    m_ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));

    ChangeSummary changeSummary(m_ecdb);
    ASSERT_EQ(SUCCESS, changeSummary.FromChangeSet(changeset));

    ChangeSummary::InstanceIterator instIter = changeSummary.MakeInstanceIterator();
    EXPECT_EQ(1, instIter.QueryCount());

    ChangeSummary::ValueIterator valIter = instIter.begin().GetInstance().MakeValueIterator();
    EXPECT_EQ(19, valIter.QueryCount());
    }


        
END_ECDBUNITTESTS_NAMESPACE
