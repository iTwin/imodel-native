/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <BeSQLite/ChangesetFile.h>
#include <algorithm>
#include <cstdlib>
#include <map>
#include <tuple>

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct UnifierTestChangeSet : BeSQLite::ChangeSet
    {
    ConflictResolution _OnConflict(ConflictCause, BeSQLite::Changes::Change) override
        { return ConflictResolution::Skip; }
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct UnifierTestChangeTracker : BeSQLite::ChangeTracker
    {
    UnifierTestChangeTracker(BeSQLite::DbR db) { SetDb(&db); }
    OnCommitStatus _OnCommit(bool, Utf8CP) override { return OnCommitStatus::Commit; }
    };

typedef std::vector<std::unique_ptr<BeJsDocument>> UnifiedInstances;

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ChangeUnifierTests : ECDbTestFixture
    {
    //! Three class families:
    //!  - JBase/JChild: joined table (tu_JBase + tu_JChild)
    //!  - Entity/BigThing: shared columns with overflow (tu_Entity + tu_Entity_Overflow)
    //!  - Gadget: a single table
    Utf8CP GetSchema() const
        {
        return R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="TestUnifier" alias="tu" version="01.00.00"
                    xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="JBase" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
                </ECCustomAttributes>
                <ECProperty propertyName="BaseCode" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="JChild" modifier="Sealed">
                <BaseClass>JBase</BaseClass>
                <ECProperty propertyName="P" typeName="int"/>
                <ECProperty propertyName="Q" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="Entity" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <ApplyToSubclassesOnly>true</ApplyToSubclassesOnly>
                        <MaxSharedColumnsBeforeOverflow>3</MaxSharedColumnsBeforeOverflow>
                    </ShareColumns>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="BigThing" modifier="Sealed">
                <BaseClass>Entity</BaseClass>
                <ECProperty propertyName="A" typeName="string"/>
                <ECProperty propertyName="B" typeName="string"/>
                <ECProperty propertyName="C" typeName="string"/>
                <ECProperty propertyName="D" typeName="string"/>
                <ECProperty propertyName="E" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="Gadget" modifier="Sealed">
                <ECProperty propertyName="Name" typeName="string"/>
                <ECProperty propertyName="Weight" typeName="double"/>
            </ECEntityClass>
            </ECSchema>)xml";
        }

    ECClassId ClassId(Utf8CP className) { return m_ecdb.Schemas().GetClassId("TestUnifier", className); }

    ECInstanceKey Insert(Utf8CP ecsql)
        {
        ECInstanceKey key;
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql)) << ecsql;
        EXPECT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecsql;
        return key;
        }

    void Execute(Utf8StringCR ecsql)
        {
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
        EXPECT_EQ(BE_SQLITE_DONE, stmt.Step()) << ecsql.c_str();
        }

    //! Writes the changes captured by @p tracker to a changeset file and returns its path.
    BeFileName WriteChangeset(UnifierTestChangeTracker& tracker, Utf8CP fileName)
        {
        UnifierTestChangeSet cs;
        EXPECT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
        BeFileName path = BuildECDbPath(fileName);
        BeSQLite::ChangeGroup group(m_ecdb);
        EXPECT_EQ(BE_SQLITE_OK, cs.AddToChangeGroup(group));
        BeSQLite::DdlChanges emptyDdl;
        BeSQLite::ChangesetFileWriter writer(path, false, emptyDdl, &m_ecdb);
        EXPECT_EQ(BE_SQLITE_OK, writer.Initialize());
        EXPECT_EQ(BE_SQLITE_OK, writer.FromChangeGroup(group));
        return path;
        }

    DbResult Append(ChangeUnifier& unifier, BeFileNameCR changesetFile, std::vector<Utf8String> const* tableFilters = nullptr)
        {
        ChangesetReader reader;
        DbResult rc = reader.OpenChangesetFile(m_ecdb, changesetFile.GetNameUtf8(), false, ChangesetReader::PropertyFilter::All);
        if (rc != BE_SQLITE_OK)
            return rc;
        if (tableFilters != nullptr)
            EXPECT_EQ(SUCCESS, reader.SetTableFilters(*tableFilters));
        rc = unifier.AppendFrom(reader, JsReadOptions());
        EXPECT_EQ(SUCCESS, reader.Close());
        return rc;
        }

    static UnifiedInstances Drain(ChangeUnifier& unifier)
        {
        UnifiedInstances out;
        for (;;)
            {
            auto doc = std::make_unique<BeJsDocument>();
            DbResult rc = unifier.Step(*doc);
            if (rc == BE_SQLITE_DONE)
                break;
            EXPECT_EQ(BE_SQLITE_ROW, rc) << unifier.GetLastError().c_str();
            if (rc != BE_SQLITE_ROW)
                break;
            out.push_back(std::move(doc));
            }
        return out;
        }

    static std::vector<Utf8String> Stringify(UnifiedInstances const& instances)
        {
        std::vector<Utf8String> out;
        for (auto const& doc : instances)
            out.push_back(doc->Stringify());
        return out;
        }

    static std::vector<Utf8String> MemberNames(BeJsConst obj)
        {
        std::vector<Utf8String> names;
        obj.ForEachProperty([&](Utf8CP name, BeJsConst) { names.push_back(name); return false; });
        return names;
        }

    static bool HasMember(BeJsConst obj, Utf8CP name)
        {
        auto names = MemberNames(obj);
        return std::find(names.begin(), names.end(), Utf8String(name)) != names.end();
        }

    static std::vector<Utf8String> StringArray(BeJsConst arr)
        {
        std::vector<Utf8String> out;
        for (uint32_t i = 0; i < arr.size(); ++i)
            out.push_back(arr[i].asString());
        return out;
        }

    static uint64_t ParseHexId(BeJsConst val) { return std::strtoull(val.asString().c_str(), nullptr, 16); }

    static Utf8String ExpectedInstanceKey(ECInstanceId id, ECClassId classId)
        {
        return Utf8PrintfString("%s-%s", id.ToHexStr().c_str(), classId.ToHexStr().c_str());
        }
    };

//---------------------------------------------------------------------------------------
// Rows of the main and the joined table of one instance merge into one instance.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangeUnifierTests, JoinedTable_InsertMergesRowsFromAllTables)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("cu_joined.ecdb", SchemaItem(GetSchema())));

    ECInstanceKey key;
    BeFileName csFile;
    {
    UnifierTestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    key = Insert("INSERT INTO tu.JChild(BaseCode, P, Q) VALUES('base', 7, 'q')");
    csFile = WriteChangeset(tracker, "cu_joined.changeset");
    }

    ChangeUnifier unifier;
    ASSERT_EQ(BE_SQLITE_OK, Append(unifier, csFile));
    UnifiedInstances instances = Drain(unifier);
    ASSERT_EQ(1, (int) instances.size());

    BeJsConst inst = *instances[0];
    EXPECT_STREQ(key.GetInstanceId().ToHexStr().c_str(), inst["ECInstanceId"].asString().c_str());
    EXPECT_STREQ(ClassId("JChild").ToHexStr().c_str(), inst["ECClassId"].asString().c_str());
    EXPECT_STREQ("base", inst["BaseCode"].asString().c_str());
    EXPECT_EQ(7, inst["P"].asInt());
    EXPECT_STREQ("q", inst["Q"].asString().c_str());

    BeJsConst meta = inst["$meta"];
    EXPECT_STREQ("Inserted", meta["op"].asString().c_str());
    EXPECT_STREQ("New", meta["stage"].asString().c_str());
    EXPECT_FALSE(meta["isIndirectChange"].asBool(true));
    EXPECT_STREQ(ExpectedInstanceKey(key.GetInstanceId(), ClassId("JChild")).c_str(), meta["instanceKey"].asString().c_str());
    EXPECT_EQ((std::vector<Utf8String>{"tu_JBase", "tu_JChild"}), StringArray(meta["tables"]));
    ASSERT_EQ(2u, meta["changeIndexes"].size());
    EXPECT_EQ(1, meta["changeIndexes"][0u].asInt());
    EXPECT_EQ(2, meta["changeIndexes"][1u].asInt());
    auto fetched = StringArray(meta["changeFetchedPropNames"]);
    for (Utf8CP name : {"ECInstanceId", "BaseCode", "P", "Q"})
        EXPECT_NE(fetched.end(), std::find(fetched.begin(), fetched.end(), Utf8String(name))) << name;
    }

//---------------------------------------------------------------------------------------
// An update yields an Old and a New instance, Old first.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangeUnifierTests, Update_ProducesOldAndNewInstances)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("cu_update.ecdb", SchemaItem(GetSchema())));
    ECInstanceKey key = Insert("INSERT INTO tu.Gadget(Name, Weight) VALUES('a', 1.5)");

    BeFileName csFile;
    {
    UnifierTestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    Execute(Utf8PrintfString("UPDATE tu.Gadget SET Name='b' WHERE ECInstanceId=%" PRIu64, key.GetInstanceId().GetValue()));
    csFile = WriteChangeset(tracker, "cu_update.changeset");
    }

    ChangeUnifier unifier;
    ASSERT_EQ(BE_SQLITE_OK, Append(unifier, csFile));
    UnifiedInstances instances = Drain(unifier);
    ASSERT_EQ(2, (int) instances.size());

    BeJsConst oldInst = *instances[0];
    BeJsConst newInst = *instances[1];
    EXPECT_STREQ("Old", oldInst["$meta"]["stage"].asString().c_str());
    EXPECT_STREQ("New", newInst["$meta"]["stage"].asString().c_str());
    EXPECT_STREQ("Updated", oldInst["$meta"]["op"].asString().c_str());
    EXPECT_STREQ("Updated", newInst["$meta"]["op"].asString().c_str());
    EXPECT_STREQ("a", oldInst["Name"].asString().c_str());
    EXPECT_STREQ("b", newInst["Name"].asString().c_str());
    EXPECT_STREQ(oldInst["$meta"]["instanceKey"].asString().c_str(), newInst["$meta"]["instanceKey"].asString().c_str());
    EXPECT_EQ((std::vector<Utf8String>{"tu_Gadget"}), StringArray(newInst["$meta"]["tables"]));
    }

//---------------------------------------------------------------------------------------
// When only overflow-table rows contribute, op is "Updated".
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangeUnifierTests, OverflowOnly_ReportedAsUpdated)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("cu_overflow.ecdb", SchemaItem(GetSchema())));

    // Insert touches main + overflow table.
    BeFileName insertFile;
    ECInstanceKey key;
    {
    UnifierTestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    key = Insert("INSERT INTO tu.BigThing(A, B, C, D, E) VALUES('a', 'b', 'c', 'd', 'e')");
    insertFile = WriteChangeset(tracker, "cu_overflow_insert.changeset");
    }

    // Update touches the overflow table only.
    BeFileName updateFile;
    {
    UnifierTestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    Execute(Utf8PrintfString("UPDATE tu.BigThing SET D='d2' WHERE ECInstanceId=%" PRIu64, key.GetInstanceId().GetValue()));
    updateFile = WriteChangeset(tracker, "cu_overflow_update.changeset");
    }

    // Both tables: op comes from the main table.
        {
        ChangeUnifier unifier;
        ASSERT_EQ(BE_SQLITE_OK, Append(unifier, insertFile));
        UnifiedInstances instances = Drain(unifier);
        ASSERT_EQ(1, (int) instances.size());
        BeJsConst inst = *instances[0];
        EXPECT_STREQ("Inserted", inst["$meta"]["op"].asString().c_str());
        EXPECT_EQ((std::vector<Utf8String>{"tu_Entity", "tu_Entity_Overflow"}), StringArray(inst["$meta"]["tables"]));
        for (Utf8CP name : {"A", "B", "C", "D", "E"})
            EXPECT_TRUE(HasMember(inst, name)) << name;
        EXPECT_STREQ(ClassId("BigThing").ToHexStr().c_str(), inst["ECClassId"].asString().c_str());
        }

    // Insert with the main table filtered out: only the overflow row contributes.
        {
        std::vector<Utf8String> filters{"tu_Entity_Overflow"};
        ChangeUnifier unifier;
        ASSERT_EQ(BE_SQLITE_OK, Append(unifier, insertFile, &filters));
        UnifiedInstances instances = Drain(unifier);
        ASSERT_EQ(1, (int) instances.size());
        BeJsConst inst = *instances[0];
        EXPECT_STREQ("Updated", inst["$meta"]["op"].asString().c_str());
        EXPECT_STREQ("New", inst["$meta"]["stage"].asString().c_str());
        EXPECT_EQ((std::vector<Utf8String>{"tu_Entity_Overflow"}), StringArray(inst["$meta"]["tables"]));
        EXPECT_FALSE(HasMember(inst, "A"));
        EXPECT_STREQ("d", inst["D"].asString().c_str());
        }

    // Overflow-only update.
        {
        ChangeUnifier unifier;
        ASSERT_EQ(BE_SQLITE_OK, Append(unifier, updateFile));
        UnifiedInstances instances = Drain(unifier);
        ASSERT_EQ(2, (int) instances.size());
        EXPECT_STREQ("Old", (*instances[0])["$meta"]["stage"].asString().c_str());
        EXPECT_STREQ("New", (*instances[1])["$meta"]["stage"].asString().c_str());
        for (auto const& doc : instances)
            {
            EXPECT_STREQ("Updated", (*doc)["$meta"]["op"].asString().c_str());
            EXPECT_EQ((std::vector<Utf8String>{"tu_Entity_Overflow"}), StringArray((*doc)["$meta"]["tables"]));
            }
        EXPECT_STREQ("d", (*instances[0])["D"].asString().c_str());
        EXPECT_STREQ("d2", (*instances[1])["D"].asString().c_str());
        }
    }

//---------------------------------------------------------------------------------------
// Only the requested properties plus ECInstanceId/ECClassId are returned.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangeUnifierTests, Projection_KeepsOnlyRequestedProperties)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("cu_projection.ecdb", SchemaItem(GetSchema())));

    BeFileName csFile;
    {
    UnifierTestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    Insert("INSERT INTO tu.Gadget(Name, Weight) VALUES('g', 2.5)");
    Insert("INSERT INTO tu.JChild(BaseCode, P, Q) VALUES('base', 1, 'q')");
    csFile = WriteChangeset(tracker, "cu_projection.changeset");
    }

    ChangeUnifier::Options options;
    options.m_propNames = {"Name", "Q"};
    ChangeUnifier unifier(options);
    ASSERT_EQ(BE_SQLITE_OK, Append(unifier, csFile));
    UnifiedInstances instances = Drain(unifier);
    ASSERT_EQ(2, (int) instances.size());

    for (auto const& doc : instances)
        {
        BeJsConst inst = *doc;
        const bool isGadget = inst["ECClassId"].asString() == ClassId("Gadget").ToHexStr();
        std::vector<Utf8String> expected{"ECInstanceId", "ECClassId", isGadget ? "Name" : "Q", "$meta"};
        auto names = MemberNames(inst);
        std::sort(names.begin(), names.end());
        std::sort(expected.begin(), expected.end());
        EXPECT_EQ(expected, names);
        }
    }

//---------------------------------------------------------------------------------------
// Output is sorted numerically by (root class id, ECInstanceId, stage) with Old before New.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangeUnifierTests, Output_SortedByRootClassInstanceIdAndStage)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("cu_sort.ecdb", SchemaItem(GetSchema())));
    ECInstanceKey g1 = Insert("INSERT INTO tu.Gadget(Name) VALUES('g1')");
    ECInstanceKey g2 = Insert("INSERT INTO tu.Gadget(Name) VALUES('g2')");
    ECInstanceKey g3 = Insert("INSERT INTO tu.Gadget(Name) VALUES('g3')");

    BeFileName csFile;
    {
    UnifierTestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    Insert("INSERT INTO tu.JChild(BaseCode, P, Q) VALUES('base', 1, 'q')");
    Execute(Utf8PrintfString("UPDATE tu.Gadget SET Name='g3b' WHERE ECInstanceId=%" PRIu64, g3.GetInstanceId().GetValue()));
    Execute(Utf8PrintfString("DELETE FROM tu.Gadget WHERE ECInstanceId=%" PRIu64, g2.GetInstanceId().GetValue()));
    Execute(Utf8PrintfString("UPDATE tu.Gadget SET Name='g1b' WHERE ECInstanceId=%" PRIu64, g1.GetInstanceId().GetValue()));
    Insert("INSERT INTO tu.Gadget(Name) VALUES('g4')");
    Insert("INSERT INTO tu.BigThing(A, D) VALUES('a', 'd')");
    csFile = WriteChangeset(tracker, "cu_sort.changeset");
    }

    std::map<uint64_t, uint64_t> rootOf{
        {ClassId("JChild").GetValue(), ClassId("JBase").GetValue()},
        {ClassId("BigThing").GetValue(), ClassId("Entity").GetValue()},
        {ClassId("Gadget").GetValue(), ClassId("Gadget").GetValue()},
    };

    ChangeUnifier unifier;
    ASSERT_EQ(BE_SQLITE_OK, Append(unifier, csFile));
    UnifiedInstances instances = Drain(unifier);
    // JChild New, BigThing New, g1 Old/New, g2 Old, g3 Old/New, g4 New
    ASSERT_EQ(8, (int) instances.size());

    std::vector<std::tuple<uint64_t, uint64_t, int>> keys;
    for (auto const& doc : instances)
        {
        BeJsConst inst = *doc;
        auto it = rootOf.find(ParseHexId(inst["ECClassId"]));
        ASSERT_NE(rootOf.end(), it);
        const int stage = inst["$meta"]["stage"].asString() == "Old" ? 0 : 1;
        keys.push_back(std::make_tuple(it->second, ParseHexId(inst["ECInstanceId"]), stage));
        if (ParseHexId(inst["ECInstanceId"]) == g2.GetInstanceId().GetValue())
            {
            EXPECT_STREQ("Deleted", inst["$meta"]["op"].asString().c_str());
            EXPECT_STREQ("g2", inst["Name"].asString().c_str());
            }
        }
    for (size_t i = 1; i < keys.size(); ++i)
        EXPECT_TRUE(keys[i - 1] < keys[i]) << "instance " << i << " is out of order";
    }

//---------------------------------------------------------------------------------------
// Spilling sorted runs to disk produces exactly the same output as merging in memory.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangeUnifierTests, Spill_TinyBudgetMatchesInMemory)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("cu_spill.ecdb", SchemaItem(GetSchema())));
    std::vector<ECInstanceKey> gadgets;
    for (int i = 0; i < 20; ++i)
        gadgets.push_back(Insert(Utf8PrintfString("INSERT INTO tu.Gadget(Name, Weight) VALUES('g%d', %d.5)", i, i).c_str()));

    BeFileName csFile;
    {
    UnifierTestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    for (int i = 0; i < 30; ++i)
        Insert(Utf8PrintfString("INSERT INTO tu.JChild(BaseCode, P, Q) VALUES('b%d', %d, 'q%d')", i, i, i).c_str());
    for (int i = 0; i < 10; ++i)
        Insert(Utf8PrintfString("INSERT INTO tu.BigThing(A, B, C, D, E) VALUES('a%d', 'b', 'c', 'd%d', 'e')", i, i).c_str());
    for (auto const& g : gadgets)
        Execute(Utf8PrintfString("UPDATE tu.Gadget SET Name=Name || 'x' WHERE ECInstanceId=%" PRIu64, g.GetInstanceId().GetValue()));
    csFile = WriteChangeset(tracker, "cu_spill.changeset");
    }

    auto run = [&](uint64_t budget)
        {
        ChangeUnifier::Options options;
        options.m_memoryBudgetBytes = budget;
        ChangeUnifier unifier(options);
        EXPECT_EQ(BE_SQLITE_OK, Append(unifier, csFile));
        auto out = Stringify(Drain(unifier));
        unifier.Close();
        return out;
        };

    std::vector<Utf8String> inMemory = run(0);
    ASSERT_EQ(30 + 10 + 2 * 20, (int) inMemory.size());
    EXPECT_EQ(inMemory, run(1));
    EXPECT_EQ(inMemory, run(512));
    EXPECT_EQ(inMemory, run(ChangeUnifier::DefaultMemoryBudgetBytes));
    }

//---------------------------------------------------------------------------------------
// Several readers can be appended; rows of later readers win on conflicting values.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangeUnifierTests, MultipleAppendFrom_MergesAcrossReaders)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("cu_multi.ecdb", SchemaItem(GetSchema())));

    ECInstanceKey key;
    BeFileName cs1;
    {
    UnifierTestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    key = Insert("INSERT INTO tu.JChild(BaseCode, P, Q) VALUES('base', 1, 'v1')");
    cs1 = WriteChangeset(tracker, "cu_multi_1.changeset");
    }
    BeFileName cs2;
    {
    UnifierTestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    Execute(Utf8PrintfString("UPDATE tu.JChild SET Q='v2' WHERE ECInstanceId=%" PRIu64, key.GetInstanceId().GetValue()));
    cs2 = WriteChangeset(tracker, "cu_multi_2.changeset");
    }

    for (uint64_t budget : {(uint64_t) 0, (uint64_t) 1})
        {
        ChangeUnifier::Options options;
        options.m_memoryBudgetBytes = budget;
        ChangeUnifier unifier(options);
        ASSERT_EQ(BE_SQLITE_OK, Append(unifier, cs1));
        ASSERT_EQ(BE_SQLITE_OK, Append(unifier, cs2));
        UnifiedInstances instances = Drain(unifier);
        ASSERT_EQ(2, (int) instances.size());

        BeJsConst oldInst = *instances[0];
        EXPECT_STREQ("Old", oldInst["$meta"]["stage"].asString().c_str());
        EXPECT_STREQ("Updated", oldInst["$meta"]["op"].asString().c_str());
        EXPECT_STREQ("v1", oldInst["Q"].asString().c_str());

        BeJsConst newInst = *instances[1];
        EXPECT_STREQ("New", newInst["$meta"]["stage"].asString().c_str());
        EXPECT_STREQ("Inserted", newInst["$meta"]["op"].asString().c_str());
        EXPECT_STREQ("v2", newInst["Q"].asString().c_str());
        EXPECT_STREQ("base", newInst["BaseCode"].asString().c_str());
        EXPECT_EQ(3u, newInst["$meta"]["tables"].size());
        EXPECT_STREQ(ExpectedInstanceKey(key.GetInstanceId(), ClassId("JChild")).c_str(), newInst["$meta"]["instanceKey"].asString().c_str());
        }
    }

//---------------------------------------------------------------------------------------
// AppendFrom after Step is misuse; Step after exhaustion keeps returning DONE; Close is idempotent.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangeUnifierTests, Lifecycle)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("cu_lifecycle.ecdb", SchemaItem(GetSchema())));

    BeFileName csFile;
    {
    UnifierTestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    Insert("INSERT INTO tu.Gadget(Name) VALUES('g')");
    csFile = WriteChangeset(tracker, "cu_lifecycle.changeset");
    }

    ChangeUnifier unifier;
    ASSERT_EQ(BE_SQLITE_OK, Append(unifier, csFile));
    BeJsDocument doc;
    ASSERT_EQ(BE_SQLITE_ROW, unifier.Step(doc));
    EXPECT_EQ(BE_SQLITE_MISUSE, Append(unifier, csFile));
    EXPECT_FALSE(unifier.GetLastError().empty());
    EXPECT_EQ(BE_SQLITE_DONE, unifier.Step(doc));
    EXPECT_EQ(BE_SQLITE_DONE, unifier.Step(doc));
    unifier.Close();
    unifier.Close();

    ChangeUnifier empty;
    BeJsDocument emptyDoc;
    EXPECT_EQ(BE_SQLITE_DONE, empty.Step(emptyDoc));
    }

END_ECDBUNITTESTS_NAMESPACE
