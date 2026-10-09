/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "../../ECDb/ChangeUnifierMap.h"
#include <BeSQLite/ChangesetFile.h>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <map>
#include <set>
#include <tuple>

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// Writes the events of ChangeUnifier::Step into a BeJsValue, so tests can compare instances as JSON.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct BeJsValueInstanceWriter final : ChangeUnifier::IInstanceWriter {
private:
    BeJsValue m_root;
    std::vector<std::pair<BeJsValue, bool>> m_containers; //!< open objects and arrays; second is true for arrays
    Utf8String m_key;

    BeJsValue Target() {
        if (m_containers.empty())
            return m_root;
        auto& top = m_containers.back();
        return top.second ? top.first.appendValue() : top.first[m_key.c_str()];
    }

public:
    explicit BeJsValueInstanceWriter(BeJsValue root) : m_root(root) {}
    void StartObject() override {
        BeJsValue target = Target();
        target.SetEmptyObject();
        m_containers.push_back(std::make_pair(target, false));
    }
    void Key(Utf8CP name, size_t length, uint32_t) override { m_key.assign(name, length); }
    void EndObject() override { m_containers.pop_back(); }
    void StartArray() override {
        BeJsValue target = Target();
        target.SetEmptyArray();
        m_containers.push_back(std::make_pair(target, true));
    }
    void EndArray() override { m_containers.pop_back(); }
    void Null() override { Target().SetNull(); }
    void Bool(bool value) override { Target() = value; }
    void Int64(int64_t value) override { Target() = value; }
    void UInt64(uint64_t value) override { Target() = value; }
    void Double(double value) override { Target() = value; }
    void String(Utf8CP value, size_t length, uint32_t) override { Target() = Utf8String(value, length).c_str(); }
    void Binary(Byte const* data, size_t size) override { Target().SetBinary(data, size); }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static DbResult StepInto(ChangeUnifier& unifier, BeJsValue instance)
    {
    BeJsValueInstanceWriter writer(instance);
    return unifier.Step(writer);
    }

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

    //! Runs @p mutate with change tracking on and writes the captured changes to a changeset file.
    BeFileName Capture(Utf8CP fileName, std::function<void()> const& mutate)
        {
        UnifierTestChangeTracker tracker(m_ecdb);
        tracker.EnableTracking(true);
        mutate();
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

    DbResult Append(ChangeUnifier& unifier, BeFileNameCR changesetFile, std::vector<Utf8String> const& tableFilters = {})
        {
        ChangesetReader reader;
        DbResult rc = reader.OpenChangesetFile(m_ecdb, changesetFile.GetNameUtf8(), false, ChangesetReader::PropertyFilter::All);
        if (rc != BE_SQLITE_OK)
            return rc;
        if (!tableFilters.empty())
            EXPECT_EQ(SUCCESS, reader.SetTableFilters(tableFilters));
        rc = unifier.AppendFrom(reader, JsReadOptions());
        EXPECT_EQ(SUCCESS, reader.Close());
        return rc;
        }

    UnifiedInstances UnifyWithBudget(std::vector<BeFileName> const& files, ChangeUnifier::Options options, uint64_t budget, std::vector<Utf8String> const& tableFilters)
        {
        options.m_memoryBudgetBytes = budget;
        ChangeUnifier unifier(options);
        for (BeFileNameCR file : files)
            EXPECT_EQ(BE_SQLITE_OK, Append(unifier, file, tableFilters)) << unifier.GetLastError().c_str();
        UnifiedInstances out;
        for (;;)
            {
            auto doc = std::make_unique<BeJsDocument>();
            DbResult rc = StepInto(unifier, *doc);
            if (rc != BE_SQLITE_ROW)
                {
                EXPECT_EQ(BE_SQLITE_DONE, rc) << unifier.GetLastError().c_str();
                break;
                }
            out.push_back(std::move(doc));
            }
        unifier.Close();
        return out;
        }

    //! Unifies @p files in memory and returns the result. Also unifies them with tiny memory budgets, which force
    //! spilling to disk, and expects identical output.
    UnifiedInstances Unify(std::vector<BeFileName> const& files, ChangeUnifier::Options const& options = {}, std::vector<Utf8String> const& tableFilters = {})
        {
        UnifiedInstances inMemory = UnifyWithBudget(files, options, 0, tableFilters);
        std::vector<Utf8String> expected = Stringify(inMemory);
        for (uint64_t budget : {(uint64_t) 1, (uint64_t) 512})
            EXPECT_EQ(expected, Stringify(UnifyWithBudget(files, options, budget, tableFilters))) << "budget " << budget;
        return inMemory;
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

    static void ExpectMeta(BeJsConst inst, Utf8CP op, Utf8CP stage, std::vector<Utf8String> const& tables)
        {
        BeJsConst meta = inst["$meta"];
        EXPECT_STREQ(op, meta["op"].asString().c_str());
        EXPECT_STREQ(stage, meta["stage"].asString().c_str());
        EXPECT_EQ(tables, StringArray(meta["tables"]));
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
    BeFileName cs = Capture("cu_joined.changeset", [&] { key = Insert("INSERT INTO tu.JChild(BaseCode, P, Q) VALUES('base', 7, 'q')"); });

    UnifiedInstances instances = Unify({cs});
    ASSERT_EQ(1, (int) instances.size());
    BeJsConst inst = *instances[0];
    ExpectMeta(inst, "Inserted", "New", {"tu_JBase", "tu_JChild"});
    EXPECT_STREQ(key.GetInstanceId().ToHexStr().c_str(), inst["ECInstanceId"].asString().c_str());
    EXPECT_STREQ(ClassId("JChild").ToHexStr().c_str(), inst["ECClassId"].asString().c_str());
    EXPECT_STREQ("base", inst["BaseCode"].asString().c_str());
    EXPECT_EQ(7, inst["P"].asInt());
    EXPECT_STREQ("q", inst["Q"].asString().c_str());

    BeJsConst meta = inst["$meta"];
    EXPECT_FALSE(meta["isIndirectChange"].asBool(true));
    EXPECT_STREQ(ExpectedInstanceKey(key.GetInstanceId(), ClassId("JChild")).c_str(), meta["instanceKey"].asString().c_str());
    ASSERT_EQ(2u, meta["changeIndexes"].size());
    EXPECT_EQ(1, meta["changeIndexes"][0u].asInt());
    EXPECT_EQ(2, meta["changeIndexes"][1u].asInt());
    auto fetched = StringArray(meta["changeFetchedPropNames"]);
    for (Utf8CP name : {"ECInstanceId", "BaseCode", "P", "Q"})
        EXPECT_NE(fetched.end(), std::find(fetched.begin(), fetched.end(), Utf8String(name))) << name;
    }

//---------------------------------------------------------------------------------------
// Values that push the scratch render document past its byte limit, including one larger than the limit, come back
// intact, and so do the values rendered after the document is dropped.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangeUnifierTests, LargeValues_SurviveScratchReset)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("cu_large.ecdb", SchemaItem(GetSchema())));
    const Utf8String big1(700 * 1024, 'x'), big2(1500 * 1024, 'y');
    BeFileName cs = Capture("cu_large.changeset", [&]
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO tu.BigThing(A, B) VALUES(?, ?)"));
        for (Utf8StringCP big : {&big1, &big2})
            {
            ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, big->c_str(), IECSqlBinder::MakeCopy::No));
            ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, "after", IECSqlBinder::MakeCopy::No));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Reset();
            stmt.ClearBindings();
            }
        });

    UnifiedInstances instances = Unify({cs});
    ASSERT_EQ(2, (int) instances.size());
    EXPECT_TRUE(big1 == (*instances[0])["A"].asString());
    EXPECT_TRUE(big2 == (*instances[1])["A"].asString());
    for (auto const& doc : instances)
        EXPECT_STREQ("after", (*doc)["B"].asString().c_str());
    }

//---------------------------------------------------------------------------------------
// When only overflow-table rows contribute, op is "Updated".
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangeUnifierTests, OverflowOnly_ReportedAsUpdated)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("cu_overflow.ecdb", SchemaItem(GetSchema())));
    ECInstanceKey key;
    BeFileName insertCs = Capture("cu_overflow_insert.changeset", [&] { key = Insert("INSERT INTO tu.BigThing(A, B, C, D, E) VALUES('a', 'b', 'c', 'd', 'e')"); });
    BeFileName updateCs = Capture("cu_overflow_update.changeset", [&] { Execute(Utf8PrintfString("UPDATE tu.BigThing SET D='d2' WHERE ECInstanceId=%" PRIu64, key.GetInstanceId().GetValue())); });

    // Main + overflow row: op comes from the main table.
    UnifiedInstances both = Unify({insertCs});
    ASSERT_EQ(1, (int) both.size());
    ExpectMeta(*both[0], "Inserted", "New", {"tu_Entity", "tu_Entity_Overflow"});
    for (Utf8CP name : {"A", "B", "C", "D", "E"})
        EXPECT_TRUE(HasMember(*both[0], name)) << name;
    EXPECT_STREQ(ClassId("BigThing").ToHexStr().c_str(), (*both[0])["ECClassId"].asString().c_str());

    // Insert with the main table filtered out: only the overflow row contributes.
    UnifiedInstances overflowOnly = Unify({insertCs}, {}, {"tu_Entity_Overflow"});
    ASSERT_EQ(1, (int) overflowOnly.size());
    ExpectMeta(*overflowOnly[0], "Updated", "New", {"tu_Entity_Overflow"});
    EXPECT_FALSE(HasMember(*overflowOnly[0], "A"));
    EXPECT_STREQ("d", (*overflowOnly[0])["D"].asString().c_str());

    // Update that touches the overflow table only.
    UnifiedInstances updated = Unify({updateCs});
    ASSERT_EQ(2, (int) updated.size());
    ExpectMeta(*updated[0], "Updated", "Old", {"tu_Entity_Overflow"});
    ExpectMeta(*updated[1], "Updated", "New", {"tu_Entity_Overflow"});
    EXPECT_STREQ("d", (*updated[0])["D"].asString().c_str());
    EXPECT_STREQ("d2", (*updated[1])["D"].asString().c_str());
    }

//---------------------------------------------------------------------------------------
// Only the requested properties plus ECInstanceId/ECClassId are returned.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangeUnifierTests, Projection_KeepsOnlyRequestedProperties)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("cu_projection.ecdb", SchemaItem(GetSchema())));
    BeFileName cs = Capture("cu_projection.changeset", [&]
        {
        Insert("INSERT INTO tu.Gadget(Name, Weight) VALUES('g', 2.5)");
        Insert("INSERT INTO tu.JChild(BaseCode, P, Q) VALUES('base', 1, 'q')");
        });

    ChangeUnifier::Options options;
    options.m_propNames = {"Name", "Q"};
    UnifiedInstances instances = Unify({cs}, options);
    ASSERT_EQ(2, (int) instances.size());
    for (auto const& doc : instances)
        {
        const bool isGadget = (*doc)["ECClassId"].asString() == ClassId("Gadget").ToHexStr();
        std::vector<Utf8String> expected{"$meta", "ECClassId", "ECInstanceId", isGadget ? "Name" : "Q"};
        auto names = MemberNames(*doc);
        std::sort(names.begin(), names.end());
        std::sort(expected.begin(), expected.end());
        EXPECT_EQ(expected, names);
        }
    }

//---------------------------------------------------------------------------------------
// Output is sorted numerically by (root class id, ECInstanceId, stage) with Old before New. An update yields an
// Old and a New instance; a delete yields an Old instance.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangeUnifierTests, Output_SortedByRootClassInstanceIdAndStage)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("cu_sort.ecdb", SchemaItem(GetSchema())));
    std::vector<ECInstanceKey> g;
    for (Utf8CP name : {"g0", "g1", "g2"})
        g.push_back(Insert(Utf8PrintfString("INSERT INTO tu.Gadget(Name) VALUES('%s')", name).c_str()));

    BeFileName cs = Capture("cu_sort.changeset", [&]
        {
        Insert("INSERT INTO tu.JChild(BaseCode, P, Q) VALUES('base', 1, 'q')");
        Execute(Utf8PrintfString("UPDATE tu.Gadget SET Name='g2b' WHERE ECInstanceId=%" PRIu64, g[2].GetInstanceId().GetValue()));
        Execute(Utf8PrintfString("DELETE FROM tu.Gadget WHERE ECInstanceId=%" PRIu64, g[1].GetInstanceId().GetValue()));
        Execute(Utf8PrintfString("UPDATE tu.Gadget SET Name='g0b' WHERE ECInstanceId=%" PRIu64, g[0].GetInstanceId().GetValue()));
        Insert("INSERT INTO tu.Gadget(Name) VALUES('g3')");
        Insert("INSERT INTO tu.BigThing(A, D) VALUES('a', 'd')");
        });

    std::map<uint64_t, uint64_t> rootOf{
        {ClassId("JChild").GetValue(), ClassId("JBase").GetValue()},
        {ClassId("BigThing").GetValue(), ClassId("Entity").GetValue()},
        {ClassId("Gadget").GetValue(), ClassId("Gadget").GetValue()},
    };

    UnifiedInstances instances = Unify({cs});
    // JChild New, BigThing New, g0 Old/New, g1 Old, g2 Old/New, g3 New
    ASSERT_EQ(8, (int) instances.size());

    std::vector<std::tuple<uint64_t, uint64_t, int>> keys;
    for (auto const& doc : instances)
        {
        BeJsConst inst = *doc;
        auto it = rootOf.find(ParseHexId(inst["ECClassId"]));
        ASSERT_NE(rootOf.end(), it);
        const bool isOld = inst["$meta"]["stage"].asString() == "Old";
        const uint64_t id = ParseHexId(inst["ECInstanceId"]);
        keys.push_back(std::make_tuple(it->second, id, isOld ? 0 : 1));
        if (id == g[1].GetInstanceId().GetValue())
            {
            ExpectMeta(inst, "Deleted", "Old", {"tu_Gadget"});
            EXPECT_STREQ("g1", inst["Name"].asString().c_str());
            }
        if (id == g[0].GetInstanceId().GetValue())
            {
            ExpectMeta(inst, "Updated", isOld ? "Old" : "New", {"tu_Gadget"});
            EXPECT_STREQ(isOld ? "g0" : "g0b", inst["Name"].asString().c_str());
            EXPECT_STREQ(ExpectedInstanceKey(g[0].GetInstanceId(), ClassId("Gadget")).c_str(), inst["$meta"]["instanceKey"].asString().c_str());
            }
        }
    for (size_t i = 1; i < keys.size(); ++i)
        EXPECT_TRUE(keys[i - 1] < keys[i]) << "instance " << i << " is out of order";
    }

//---------------------------------------------------------------------------------------
// A larger mix of joined, overflow and single-table changes. Unify() checks that tiny budgets which force spilling
// match the in-memory output; the default budget must match too.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangeUnifierTests, Spill_TinyBudgetMatchesInMemory)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("cu_spill.ecdb", SchemaItem(GetSchema())));
    std::vector<ECInstanceKey> gadgets;
    for (int i = 0; i < 20; ++i)
        gadgets.push_back(Insert(Utf8PrintfString("INSERT INTO tu.Gadget(Name, Weight) VALUES('g%d', %d.5)", i, i).c_str()));

    BeFileName cs = Capture("cu_spill.changeset", [&]
        {
        for (int i = 0; i < 30; ++i)
            Insert(Utf8PrintfString("INSERT INTO tu.JChild(BaseCode, P, Q) VALUES('b%d', %d, 'q%d')", i, i, i).c_str());
        for (int i = 0; i < 10; ++i)
            Insert(Utf8PrintfString("INSERT INTO tu.BigThing(A, B, C, D, E) VALUES('a%d', 'b', 'c', 'd%d', 'e')", i, i).c_str());
        for (auto const& g : gadgets)
            Execute(Utf8PrintfString("UPDATE tu.Gadget SET Name=Name || 'x' WHERE ECInstanceId=%" PRIu64, g.GetInstanceId().GetValue()));
        });

    std::vector<Utf8String> inMemory = Stringify(Unify({cs}));
    ASSERT_EQ(30 + 10 + 2 * 20, (int) inMemory.size());
    EXPECT_EQ(inMemory, Stringify(UnifyWithBudget({cs}, {}, ChangeUnifier::DefaultMemoryBudgetBytes, {})));
    }

//---------------------------------------------------------------------------------------
// Several readers can be appended; rows of later readers win on conflicting values.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangeUnifierTests, MultipleAppendFrom_MergesAcrossReaders)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("cu_multi.ecdb", SchemaItem(GetSchema())));
    ECInstanceKey key;
    BeFileName cs1 = Capture("cu_multi_1.changeset", [&] { key = Insert("INSERT INTO tu.JChild(BaseCode, P, Q) VALUES('base', 1, 'v1')"); });
    BeFileName cs2 = Capture("cu_multi_2.changeset", [&] { Execute(Utf8PrintfString("UPDATE tu.JChild SET Q='v2' WHERE ECInstanceId=%" PRIu64, key.GetInstanceId().GetValue())); });

    UnifiedInstances instances = Unify({cs1, cs2});
    ASSERT_EQ(2, (int) instances.size());
    BeJsConst oldInst = *instances[0];
    BeJsConst newInst = *instances[1];
    EXPECT_STREQ("Old", oldInst["$meta"]["stage"].asString().c_str());
    EXPECT_STREQ("Updated", oldInst["$meta"]["op"].asString().c_str());
    EXPECT_STREQ("v1", oldInst["Q"].asString().c_str());
    EXPECT_STREQ("New", newInst["$meta"]["stage"].asString().c_str());
    EXPECT_STREQ("Inserted", newInst["$meta"]["op"].asString().c_str());
    EXPECT_EQ(3u, newInst["$meta"]["tables"].size());
    EXPECT_STREQ("v2", newInst["Q"].asString().c_str());
    EXPECT_STREQ("base", newInst["BaseCode"].asString().c_str());
    EXPECT_STREQ(ExpectedInstanceKey(key.GetInstanceId(), ClassId("JChild")).c_str(), newInst["$meta"]["instanceKey"].asString().c_str());
    }

//---------------------------------------------------------------------------------------
// AppendFrom after Step is misuse; Step after exhaustion keeps returning DONE; Close is idempotent.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangeUnifierTests, Lifecycle)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("cu_lifecycle.ecdb", SchemaItem(GetSchema())));
    BeFileName cs = Capture("cu_lifecycle.changeset", [&] { Insert("INSERT INTO tu.Gadget(Name) VALUES('g')"); });

    ChangeUnifier unifier;
    ASSERT_EQ(BE_SQLITE_OK, Append(unifier, cs));
    BeJsDocument doc;
    ASSERT_EQ(BE_SQLITE_ROW, StepInto(unifier, doc));
    EXPECT_EQ(BE_SQLITE_MISUSE, Append(unifier, cs));
    EXPECT_FALSE(unifier.GetLastError().empty());
    EXPECT_EQ(BE_SQLITE_DONE, StepInto(unifier, doc));
    EXPECT_EQ(BE_SQLITE_DONE, StepInto(unifier, doc));
    unifier.Close();
    unifier.Close();

    ChangeUnifier empty;
    BeJsDocument emptyDoc;
    EXPECT_EQ(BE_SQLITE_DONE, StepInto(empty, emptyDoc));
    }

//---------------------------------------------------------------------------------------
// Step gives every name from the unifier's string table the same non-zero id in every instance,
// so writers can convert each name once.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangeUnifierTests, Writer_StableStringIds)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("cu_writer.ecdb", SchemaItem(GetSchema())));
    BeFileName cs = Capture("cu_writer.changeset", [&]
        {
        Insert("INSERT INTO tu.Gadget(Name) VALUES('a')");
        Insert("INSERT INTO tu.Gadget(Name) VALUES('b')");
        });

    struct KeyIdWriter final : ChangeUnifier::IInstanceWriter
        {
        std::map<Utf8String, std::set<uint32_t>> m_keyIds;
        void StartObject() override {}
        void Key(Utf8CP name, size_t length, uint32_t stringId) override { m_keyIds[Utf8String(name, length)].insert(stringId); }
        void EndObject() override {}
        void StartArray() override {}
        void EndArray() override {}
        void Null() override {}
        void Bool(bool) override {}
        void Int64(int64_t) override {}
        void UInt64(uint64_t) override {}
        void Double(double) override {}
        void String(Utf8CP, size_t, uint32_t) override {}
        void Binary(Byte const*, size_t) override {}
        };
    KeyIdWriter writer;
    ChangeUnifier unifier;
    ASSERT_EQ(BE_SQLITE_OK, Append(unifier, cs));
    int count = 0;
    while (unifier.Step(writer) == BE_SQLITE_ROW)
        ++count;
    EXPECT_EQ(2, count);
    EXPECT_EQ(1u, writer.m_keyIds.count("Name"));
    EXPECT_EQ(1u, writer.m_keyIds.count("$meta"));
    for (auto const& entry : writer.m_keyIds)
        {
        EXPECT_EQ(1u, entry.second.size()) << entry.first.c_str();
        EXPECT_NE(0u, *entry.second.begin()) << entry.first.c_str();
        }
    }

//---------------------------------------------------------------------------------------
// The merge map keeps finding every key across many regrowths, also when all keys share one hash.
// TakeEntries returns insertion order and leaves an empty, reusable map.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(ChangeUnifierFlatMapTests, GrowthCollisionsAndTake)
    {
    struct SameHash final { size_t operator()(uint64_t) const { return 42; } };
    auto check = [](auto& map, uint64_t count)
        {
        EXPECT_EQ(nullptr, map.Find(0));
        for (uint64_t i = 1; i <= count; ++i)
            map.Insert(i, i * 10);
        ASSERT_EQ(count, map.Size());
        for (uint64_t i = 1; i <= count; ++i)
            {
            uint64_t* value = map.Find(i);
            ASSERT_NE(nullptr, value) << i;
            EXPECT_EQ(i * 10, *value);
            }
        EXPECT_EQ(nullptr, map.Find(count + 1));
        *map.Find(1) = 7;

        auto taken = map.TakeEntries();
        ASSERT_EQ(count, taken.size());
        for (uint64_t i = 0; i < count; ++i)
            EXPECT_EQ(i + 1, taken[i].first);
        EXPECT_EQ(7u, taken[0].second);
        EXPECT_TRUE(map.IsEmpty());
        EXPECT_EQ(nullptr, map.Find(1));
        map.Insert(1, 1);
        EXPECT_EQ(1u, *map.Find(1));
        };
    ChangeUnifierFlatMap<uint64_t, uint64_t, std::hash<uint64_t>> spread;
    check(spread, 10000);
    ChangeUnifierFlatMap<uint64_t, uint64_t, SameHash> clashing;
    check(clashing, 200);
    }

END_ECDBUNITTESTS_NAMESPACE
