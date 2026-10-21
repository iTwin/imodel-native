/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <ECDb/SchemaEditor.h>
#include <BeSQLite/ChangeSet.h>
#include <set>

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

struct SchemaEditorTests : ECDbTestFixture {};

TEST_F(SchemaEditorTests, CommitsAndReloadsCommandMetadataAndMapping) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorCommit.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    ASSERT_TRUE(editor->Execute("CreateSchema", {{"name", "Atomic"}, {"alias", "a"}, {"version", "01.00.00"}}).IsOk());
    ASSERT_TRUE(editor->Execute("CreateEntityClass", {{"schema", "Atomic"}, {"name", "Item"}}).IsOk());
    ASSERT_TRUE(editor->Execute("AddProperty", {{"class", "a:Item"}, {"name", "Code"}, {"kind", "primitive"}, {"type", "string"}}).IsOk());
    ASSERT_TRUE(editor->SaveChanges().IsOk());
    editor.reset();

    m_ecdb.ClearECDbCache();
    auto ecClass = m_ecdb.Schemas().GetClass("Atomic", "Item");
    ASSERT_NE(nullptr, ecClass);
    ASSERT_NE(nullptr, ecClass->GetPropertyP("Code"));
    ECSqlStatement insert;
    ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(m_ecdb, "INSERT INTO Atomic.Item(Code) VALUES('persisted')"));
    ASSERT_EQ(BE_SQLITE_DONE, insert.Step());
    insert.Finalize();
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AbandonChanges());
    ECSqlStatement select;
    ASSERT_EQ(ECSqlStatus::Success, select.Prepare(m_ecdb, "SELECT Code FROM Atomic.Item"));
    ASSERT_EQ(BE_SQLITE_ROW, select.Step());
    EXPECT_STREQ("persisted", select.GetValueText(0));
    EXPECT_EQ(BE_SQLITE_DONE, select.Step());
}

TEST_F(SchemaEditorTests, FailedCommandAbandonsEverythingAndClosesSession) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorAbort.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    ASSERT_TRUE(editor->Execute("CreateSchema", {{"name", "Atomic"}, {"alias", "a"}, {"version", "01.00.00"}}).IsOk());
    ASSERT_TRUE(editor->Execute("CreateEntityClass", {{"schema", "Atomic"}, {"name", "Item"}}).IsOk());
    auto result = editor->Execute("CreateEntityClass", {{"schema", "Atomic"}, {"name", "Item"}});
    EXPECT_FALSE(result.IsOk());
    EXPECT_FALSE(result.GetIssues().empty());
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetSchema("Atomic"));
    EXPECT_EQ(SchemaCommandResult::Status::ERROR_SESSION,
              editor->Execute("CreateSchema", {{"name", "Other"}, {"alias", "o"}, {"version", "01.00.00"}}).GetStatus());
    EXPECT_EQ(SchemaCommandResult::Status::ERROR_SESSION, editor->SaveChanges().GetStatus());

    auto next = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, next);
    next->AbandonChanges();
}

TEST_F(SchemaEditorTests, DryRunDoesNotWriteOrFailSession) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorValidate.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    SchemaCommandArgs valid = {{"name", "Atomic"}, {"alias", "a"}, {"version", "01.00.00"}};
    ASSERT_TRUE(editor->Validate("CreateSchema", valid).IsOk());
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetSchema("Atomic"));
    EXPECT_FALSE(editor->Validate("CreateSchema", {{"name", "Bad Name"}, {"alias", "b"}, {"version", "01.00.00"}}).IsOk());
    EXPECT_TRUE(editor->Execute("CreateSchema", valid).IsOk());
    editor->AbandonChanges();
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetSchema("Atomic"));
}

TEST_F(SchemaEditorTests, RejectsCompetingEditorsAndBulkImports) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorExclusive.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    SchemaCommandResult beginResult;
    EXPECT_EQ(nullptr, SchemaEditor::Begin(m_ecdb, SchemaImportOptions::None, nullptr, &beginResult));
    EXPECT_EQ(SchemaCommandResult::Status::ERROR_SESSION, beginResult.GetStatus());
    ECSchemaPtr schema;
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "Competing", "c", 1, 0, 0));
    EXPECT_NE(SUCCESS, m_ecdb.Schemas().ImportSchemas({schema.get()}));
    EXPECT_TRUE(editor->Execute("CreateSchema", {{"name", "Atomic"}, {"alias", "a"}, {"version", "01.00.00"}}).IsOk());
    EXPECT_TRUE(m_ecdb.Schemas().DropSchema("Atomic").IsError());
    editor->AbandonChanges();
    EXPECT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas({schema.get()}));
}

TEST_F(SchemaEditorTests, RequiresCleanTransactionAndDestructorAbandons) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorClean.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->Execute("CreateSchema", {{"name", "Atomic"}, {"alias", "a"}, {"version", "01.00.00"}}).IsOk());
    }
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetSchema("Atomic"));
    ECSchemaPtr schema;
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "Pending", "p", 1, 0, 0));
    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas({schema.get()}));
    SchemaCommandResult result;
    EXPECT_EQ(nullptr, SchemaEditor::Begin(m_ecdb, SchemaImportOptions::None, nullptr, &result));
    EXPECT_EQ(SchemaCommandResult::Status::ERROR_SESSION, result.GetStatus());
    EXPECT_NE(nullptr, m_ecdb.Schemas().GetSchema("Pending"));
}

TEST_F(SchemaEditorTests, BatchFailureRollsBackEarlierCommands) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorBatch.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    BeJsDocument batch(R"([
        {"command":"CreateSchema","args":{"name":"Atomic","alias":"a","version":"01.00.00"}},
        {"command":"CreateEntityClass","args":{"schema":"Atomic","name":"Item"}},
        {"command":"AddProperty","args":{"class":"a:Item","name":"Bad","kind":"primitive","type":"notAType"}}
    ])");
    auto result = editor->ExecuteBatch(batch);
    EXPECT_FALSE(result.IsOk());
    BeJsDocument json;
    result.ToJson(json);
    EXPECT_EQ(2, json["failedCommandIndex"].asInt());
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetSchema("Atomic"));
}

TEST_F(SchemaEditorTests, AbandonsTrackedDdlWithoutNestedSavepoints) {
    struct Tracker final : ChangeTracker {
        explicit Tracker(ECDbR db) { SetDb(&db); }
        OnCommitStatus _OnCommit(bool, Utf8CP) override {
            Restart();
            return OnCommitStatus::Commit;
        }
        Utf8String Ddl() const { return m_ddlChanges.ToString(); }
    };
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorTracked.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    RefCountedPtr<Tracker> tracker = new Tracker(m_ecdb);
    m_ecdb.SetChangeTracker(tracker.get());
    tracker->EnableTracking(true);
    ASSERT_TRUE(tracker->IsTracking());
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    ASSERT_TRUE(editor->Execute("CreateSchema", {{"name", "Atomic"}, {"alias", "a"}, {"version", "01.00.00"}}).IsOk());
    ASSERT_TRUE(editor->Execute("CreateEntityClass", {{"schema", "Atomic"}, {"name", "Item"}}).IsOk());
    EXPECT_FALSE(tracker->Ddl().empty());
    EXPECT_EQ(1, m_ecdb.GetCurrentSavepointDepth());
    EXPECT_FALSE(editor->Execute("NotACommand", SchemaCommandArgs{}).IsOk());
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetSchema("Atomic"));
    EXPECT_TRUE(tracker->Ddl().empty());
    tracker->EndTracking();
    m_ecdb.SetChangeTracker(nullptr);
}

TEST_F(SchemaEditorTests, SimplePropertyMappingsMatchBulkImport) {
    auto snapshot = [](ECDbCR db, bvector<Utf8String>& rows) -> DbResult {
        Statement statement;
        auto status = statement.Prepare(db,
                                        "SELECT 'map|'||p.AccessString||'|'||t.Name||'|'||c.Name||'|'||c.Type||'|'||c.IsVirtual "
                                        "FROM ec_PropertyMap m JOIN ec_PropertyPath p ON p.Id=m.PropertyPathId "
                                        "JOIN ec_Column c ON c.Id=m.ColumnId JOIN ec_Table t ON t.Id=c.TableId "
                                        "JOIN ec_Class cl ON cl.Id=m.ClassId JOIN ec_Schema s ON s.Id=cl.SchemaId "
                                        "WHERE s.Name='Atomic' AND cl.Name='Item' "
                                        "UNION ALL SELECT 'property|'||p.Name||'|'||p.Kind||'|'||p.PrimitiveType||'|'||p.Ordinal "
                                        "FROM ec_Property p JOIN ec_Class cl ON cl.Id=p.ClassId JOIN ec_Schema s ON s.Id=cl.SchemaId "
                                        "WHERE s.Name='Atomic' AND cl.Name='Item' ORDER BY 1");
        if (status != BE_SQLITE_OK)
            return status;
        while ((status = statement.Step()) == BE_SQLITE_ROW)
            rows.emplace_back(statement.GetValueText(0));
        return status == BE_SQLITE_DONE ? BE_SQLITE_OK : status;
    };
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorParityCommands.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    ASSERT_TRUE(editor->Execute("CreateSchema", {{"name", "Atomic"}, {"alias", "a"}, {"version", "01.00.00"}}).IsOk());
    ASSERT_TRUE(editor->Execute("CreateEntityClass", {{"schema", "Atomic"}, {"name", "Item"}}).IsOk());
    ASSERT_TRUE(editor->Execute("AddProperty", {{"class", "a:Item"}, {"name", "Code"}, {"kind", "primitive"}, {"type", "string"}}).IsOk());
    ASSERT_TRUE(editor->Execute("AddProperty", {{"class", "a:Item"}, {"name", "Count"}, {"kind", "primitive"}, {"type", "int"}}).IsOk());
    ASSERT_TRUE(editor->SaveChanges().IsOk());
    editor.reset();
    bvector<Utf8String> commands;
    ASSERT_EQ(BE_SQLITE_OK, snapshot(m_ecdb, commands));
    ASSERT_FALSE(commands.empty());
    CloseECDb();

    ASSERT_EQ(SUCCESS, SetupECDb("SchemaEditorParityImport.ecdb", SchemaItem(R"xml(
        <ECSchema schemaName="Atomic" alias="a" version="01.00.00"
          xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Item">
            <ECProperty propertyName="Code" typeName="string"/>
            <ECProperty propertyName="Count" typeName="int"/>
          </ECEntityClass>
        </ECSchema>)xml")));
    bvector<Utf8String> imported;
    ASSERT_EQ(BE_SQLITE_OK, snapshot(m_ecdb, imported));
    EXPECT_EQ(imported, commands);
}

TEST_F(SchemaEditorTests, PolicyValidationHandlesClassesWithoutPersistedIds) {
    ASSERT_EQ(SUCCESS, SetupECDb("SchemaEditorPolicy.ecdb", SchemaItem(R"xml(
        <ECSchema schemaName="PolicyOwner" alias="po" version="01.00.00"
          xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbSchemaPolicies" version="01.00.00" alias="ecdbpol"/>
          <ECCustomAttributes>
            <NoAdditionalRootEntityClasses xmlns="ECDbSchemaPolicies.01.00.00"/>
          </ECCustomAttributes>
          <ECEntityClass typeName="Base"/>
        </ECSchema>)xml")));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    ASSERT_TRUE(editor->Execute("CreateSchema", {{"name", "Other"}, {"alias", "o"}, {"version", "01.00.00"}}).IsOk());
    EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION,
              editor->Validate("CreateEntityClass", {{"schema", "Other"}, {"name", "Root"}}).GetStatus());
    EXPECT_TRUE(editor->Validate("CreateEntityClass", {{"schema", "Other"}, {"name", "Derived"}, {"baseClass", "po:Base"}}).IsOk());
    EXPECT_TRUE(editor->Validate("CreateEntityClass", {{"schema", "PolicyOwner"}, {"name", "AnotherRoot"}}).IsOk());
    EXPECT_FALSE(editor->Execute("CreateEntityClass", {{"schema", "Other"}, {"name", "Root"}}).IsOk());
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetSchema("Other"));
    EXPECT_NE(nullptr, m_ecdb.Schemas().GetClass("PolicyOwner", "Base"));
}

TEST_F(SchemaEditorTests, DropValidationRetainsMultipleBlockingInstancesOfOneClass) {
    ASSERT_EQ(SUCCESS, SetupECDb("SchemaEditorBlockingInstances.ecdb", SchemaItem(R"xml(
       <ECSchema schemaName="Atomic" alias="a" version="01.00.00"
         xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
         <ECEntityClass typeName="Item">
           <ECProperty propertyName="Code" typeName="string"/>
         </ECEntityClass>
       </ECSchema>)xml")));
    for (auto sql : {
             "INSERT INTO Atomic.Item(ECInstanceId,Code) VALUES(1,'one')",
             "INSERT INTO Atomic.Item(ECInstanceId,Code) VALUES(2,'two')"}) {
        ECSqlStatement insert;
        ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(m_ecdb, sql));
        ASSERT_EQ(BE_SQLITE_DONE, insert.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    auto ecClass = m_ecdb.Schemas().GetClass("Atomic", "Item");
    ASSERT_NE(nullptr, ecClass);
    auto classId = ecClass->GetId();
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    ASSERT_TRUE(editor->Execute("AlterSchema", {{"schema", "Atomic"}, {"version", "02.00.00"}}).IsOk());
    auto propertyDrop = editor->Validate("DropProperty", {{"class", "a:Item"}, {"property", "Code"}});
    auto classDrop = editor->Validate("DropEntityClass", {{"class", "a:Item"}});
    for (auto result : {&propertyDrop, &classDrop}) {
        ASSERT_EQ(SchemaCommandResult::Status::ERROR_DATA_DELETION_REQUIRED, result->GetStatus());
        auto const& instances = result->GetBlockingInstances();
        ASSERT_EQ(2, instances.size());
        ASSERT_EQ(2, instances.count(classId));
        std::set<ECInstanceId> ids;
        for (auto const& instance : instances)
            ids.insert(instance.second);
        EXPECT_EQ(1, ids.count(ECInstanceId(uint64_t(1))));
        EXPECT_EQ(1, ids.count(ECInstanceId(uint64_t(2))));
    }
}

END_ECDBUNITTESTS_NAMESPACE
