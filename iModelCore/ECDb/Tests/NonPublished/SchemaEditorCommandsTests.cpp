/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <ECDb/SchemaEditor.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

struct SchemaEditorCommandsTests : ECDbTestFixture {
    Utf8String Describe(SchemaCommandResult const& result) {
        BeJsDocument json;
        result.ToJson(json);
        return json.Stringify();
    }
};

TEST_F(SchemaEditorCommandsTests, PropertyGroupsRetainHierarchyValidationAndRollback) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorPropertyGroup.ecdb"));
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->CreateSchema({{"name", "Grouped"}, {"alias", "gr"}, {"version", "01.00.00"}}).IsOk());
        BeJsDocument commands(R"json([
            {"command":"CreateEntityClass","args":{"schema":"gr","name":"Base","mapStrategy":"TablePerHierarchy"}},
            {"command":"AddProperty","args":{"class":"gr:Base","name":"A","type":"double"}},
            {"command":"AddProperty","args":{"class":"gr:Base","name":"B","type":"double"}},
            {"command":"CreateEntityClass","args":{"schema":"gr","name":"Child","baseClass":"gr:Base"}},
            {"command":"AddProperty","args":{"class":"gr:Child","name":"C","type":"double"}}
        ])json");
        auto result = editor->ExecuteGroup(commands);
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO gr.Child(A,B,C) VALUES(1.25,2.5,3.75)"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        BeJsDocument commands(R"json([
            {"command":"AddProperty","args":{"class":"gr:Base","name":"Transient","type":"double"}},
            {"command":"CreateEntityClass","args":{"schema":"gr","name":"Grandchild","baseClass":"gr:Child"}},
            {"command":"AddProperty","args":{"class":"gr:Grandchild","name":"A","type":"string"}}
        ])json");
        auto result = editor->ExecuteGroup(commands);
        ASSERT_FALSE(result.IsOk());
        BeJsDocument diagnostic;
        result.ToJson(diagnostic);
        EXPECT_STREQ("AddProperty", diagnostic["failedCommand"].asCString());
        EXPECT_EQ(2, diagnostic["failedCommandIndex"].asUInt64());
        EXPECT_EQ(nullptr, m_ecdb.Schemas().GetClass("Grouped", "Grandchild"));
        EXPECT_EQ(nullptr, m_ecdb.Schemas().GetClass("Grouped", "Base")->GetPropertyP("Transient"));
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT A,B,C FROM gr.Child"));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    EXPECT_DOUBLE_EQ(1.25, statement.GetValueDouble(0));
    EXPECT_DOUBLE_EQ(2.5, statement.GetValueDouble(1));
    EXPECT_DOUBLE_EQ(3.75, statement.GetValueDouble(2));
    EXPECT_EQ(BE_SQLITE_DONE, statement.Step());
}

TEST_F(SchemaEditorCommandsTests, PropertyWorkingGraphRefreshesAfterOtherCommands) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorPropertyWorkingGraph.ecdb"));
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->CreateSchema({{"name", "Cached"}, {"alias", "ch"}, {"version", "01.00.00"}}).IsOk());
        BeJsDocument commands(R"json([
            {"command":"CreateEntityClass","args":{"schema":"ch","name":"Item"}},
            {"command":"AddProperty","args":{"class":"ch:Item","name":"A","type":"double"}},
            {"command":"AddProperty","args":{"class":"ch:Item","name":"B","type":"double"}},
            {"command":"CreateStructClass","args":{"schema":"ch","name":"Payload"}},
            {"command":"AddProperty","args":{"class":"ch:Payload","name":"X","type":"double"}},
            {"command":"AddProperty","args":{"class":"ch:Item","name":"Payload","kind":"struct","struct":"ch:Payload"}},
            {"command":"AddProperty","args":{"class":"ch:Item","name":"Labels","kind":"primitiveArray","type":"string"}}
        ])json");
        auto result = editor->ExecuteGroup(commands);
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        SchemaCommandArgs args{{"class", "ch:Item"}, {"name", "Transient"}, {"type", "string"}};
        ASSERT_TRUE(editor->Validate("AddProperty", args).IsOk());
        ASSERT_TRUE(editor->Validate("AddProperty", args).IsOk());
        EXPECT_EQ(nullptr, m_ecdb.Schemas().GetClass("Cached", "Item")->GetPropertyP("Transient"));
        BeJsDocument commands(R"json([
            {"command":"AddProperty","args":{"class":"ch:Item","name":"Transient","type":"string"}},
            {"command":"AddProperty","args":{"class":"ch:Item","name":"Next","type":"string"}},
            {"command":"AddProperty","args":{"class":"ch:Item","name":"Transient","type":"string"}}
        ])json");
        auto result = editor->ExecuteGroup(commands);
        ASSERT_FALSE(result.IsOk());
        BeJsDocument diagnostic;
        result.ToJson(diagnostic);
        EXPECT_EQ(2, diagnostic["failedCommandIndex"].asUInt64());
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    auto item = m_ecdb.Schemas().GetClass("Cached", "Item");
    ASSERT_NE(nullptr, item);
    EXPECT_EQ(nullptr, item->GetPropertyP("Transient"));
    EXPECT_EQ(nullptr, item->GetPropertyP("Next"));
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ch.Item(A,B,Payload.X) VALUES(1.25,2.5,3.75)"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT A,B,Payload.X FROM ch.Item"));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    EXPECT_DOUBLE_EQ(1.25, statement.GetValueDouble(0));
    EXPECT_DOUBLE_EQ(2.5, statement.GetValueDouble(1));
    EXPECT_DOUBLE_EQ(3.75, statement.GetValueDouble(2));
    EXPECT_EQ(BE_SQLITE_DONE, statement.Step());
}

TEST_F(SchemaEditorCommandsTests, ReferencePruningKeepsPinnedReferencesAndRemovesUnusedImplicitReferences) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorPinnedReferences.ecdb"));
    auto hasReference = [&](Utf8CP referenced) {
        Statement statement;
        EXPECT_EQ(BE_SQLITE_OK, statement.Prepare(m_ecdb,
            "SELECT 1 FROM ec_SchemaReference r JOIN ec_Schema s ON s.Id=r.SchemaId"
            " JOIN ec_Schema t ON t.Id=r.ReferencedSchemaId WHERE s.Name='Consumers' AND t.Name=?"));
        EXPECT_EQ(BE_SQLITE_OK, statement.BindText(1, referenced, Statement::MakeCopy::No));
        auto status = statement.Step();
        EXPECT_TRUE(status == BE_SQLITE_ROW || status == BE_SQLITE_DONE);
        return status == BE_SQLITE_ROW;
    };
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->CreateSchema({{"name", "Keep"}, {"alias", "kp"}, {"version", "01.00.00"}}).IsOk());
        ASSERT_TRUE(editor->CreateSchema({{"name", "Values"}, {"alias", "val"}, {"version", "01.00.00"}}).IsOk());
        BeJsDocument enumeration(R"json({"schema":"val","name":"State","backingType":"int","enumerators":[{"name":"Off","value":0}]})json");
        auto result = editor->Execute("CreateEnumeration", enumeration);
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        ASSERT_TRUE(editor->CreateSchema({{"name", "Consumers"}, {"alias", "con"}, {"version", "01.00.00"}}).IsOk());
        ASSERT_TRUE(editor->AddSchemaReference({{"schema", "con"}, {"reference", "Keep"}}).IsOk());
        EXPECT_TRUE(hasReference("Keep"));
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "con"}, {"name", "Item"}}).IsOk());
        ASSERT_TRUE(editor->AddProperty({{"class", "con:Item"}, {"name", "State"}, {"enum", "val:State"}}).IsOk());
        EXPECT_TRUE(hasReference("Keep"));
        EXPECT_TRUE(hasReference("Values"));
        result = editor->DropProperty({{"class", "con:Item"}, {"property", "State"}});
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        EXPECT_TRUE(hasReference("Keep"));
        EXPECT_FALSE(hasReference("Values"));
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    EXPECT_TRUE(hasReference("Keep"));
    EXPECT_FALSE(hasReference("Values"));
}

TEST_F(SchemaEditorCommandsTests, ClearPropertyBoundsIndependently) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorClearLength.ecdb"));
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->CreateSchema({{"name", "LengthTest"}, {"alias", "lt"}, {"version", "01.00.00"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "lt"}, {"name", "Item"}}).IsOk());
        ASSERT_TRUE(editor->AddProperty({{"class", "lt:Item"}, {"name", "Text"}, {"type", "string"}, {"minLength", "1"}, {"maxLength", "100"}}).IsOk());
        ASSERT_TRUE(editor->AddProperty({{"class", "lt:Item"}, {"name", "Number"}, {"type", "int"}, {"minValue", "1"}, {"maxValue", "100"}}).IsOk());
        BeJsDocument invalid(R"json({"class":"lt:Item","property":"Number","minLength":null})json");
        EXPECT_FALSE(editor->Validate("AlterProperty", invalid).IsOk());
        BeJsDocument args(R"json({"class":"lt:Item","property":"Text","minLength":null})json");
        auto result = editor->Validate("AlterProperty", args);
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        EXPECT_EQ(1, m_ecdb.Schemas().GetClass("LengthTest", "Item")->GetPropertyP("Text")->GetMinimumLength());
        result = editor->Execute("AlterProperty", args);
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        args.Parse(R"json({"class":"lt:Item","property":"Number","minValue":null})json");
        result = editor->Execute("AlterProperty", args);
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    ReopenECDb();
    auto property = m_ecdb.Schemas().GetClass("LengthTest", "Item")->GetPropertyP("Text");
    EXPECT_FALSE(property->IsMinimumLengthDefined());
    EXPECT_EQ(100, property->GetMaximumLength());
    auto number = m_ecdb.Schemas().GetClass("LengthTest", "Item")->GetPropertyP("Number");
    EXPECT_FALSE(number->IsMinimumValueDefined());
    ECValue value;
    ASSERT_EQ(ECObjectsStatus::Success, number->GetMaximumValue(value));
    EXPECT_EQ(100, value.GetInteger());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        BeJsDocument args(R"json({"class":"lt:Item","property":"Text","maxLength":null})json");
        auto result = editor->Execute("AlterProperty", args);
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        args.Parse(R"json({"class":"lt:Item","property":"Number","maxValue":null})json");
        result = editor->Execute("AlterProperty", args);
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    ReopenECDb();
    property = m_ecdb.Schemas().GetClass("LengthTest", "Item")->GetPropertyP("Text");
    EXPECT_FALSE(property->IsMinimumLengthDefined());
    EXPECT_FALSE(property->IsMaximumLengthDefined());
    number = m_ecdb.Schemas().GetClass("LengthTest", "Item")->GetPropertyP("Number");
    EXPECT_FALSE(number->IsMinimumValueDefined());
    EXPECT_FALSE(number->IsMaximumValueDefined());
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT PrimitiveTypeMinLength,PrimitiveTypeMaxLength,PrimitiveTypeMinValue,PrimitiveTypeMaxValue FROM ec_Property WHERE ClassId=? AND Name IN ('Text','Number')"));
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindId(1, m_ecdb.Schemas().GetClassId("LengthTest", "Item")));
    for (int i = 0; i < 2; ++i) {
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        for (int column = 0; column < 4; ++column)
            EXPECT_TRUE(stmt.IsColumnNull(column));
    }
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step());
}

TEST_F(SchemaEditorCommandsTests, RequiredPropertyChecksExistingInstancesIncludingDerivedClasses) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorRequiredProperty.ecdb"));
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->CreateSchema({{"name", "RequiredTest"}, {"alias", "rt"}, {"version", "01.00.00"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "rt"}, {"name", "Base"}, {"mapStrategy", "TablePerHierarchy"}}).IsOk());
        auto result = editor->AddProperty({{"class", "rt:Base"}, {"name", "Name"}, {"type", "string"}, {"isNullable", "false"}});
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "rt"}, {"name", "Child"}, {"baseClass", "rt:Base"}}).IsOk());
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO rt.Child(Name) VALUES('keep')"));
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    auto result = editor->Validate("AddProperty",
                                   SchemaCommandArgs{{"class", "rt:Base"}, {"name", "Required"}, {"type", "string"}, {"isNullable", "false"}});
    EXPECT_EQ(SchemaCommandResult::Status::ERROR_DATA_DELETION_REQUIRED, result.GetStatus()) << Describe(result);
    EXPECT_FALSE(result.GetBlockingInstances().empty());
    BeJsDocument args(R"json({"class":"rt:Base","name":"Required","type":"string",
        "customAttributes":[{"className":"ECDbMap:PropertyMap","IsNullable":false}]})json");
    result = editor->Execute("AddProperty", args);
    EXPECT_EQ(SchemaCommandResult::Status::ERROR_DATA_DELETION_REQUIRED, result.GetStatus()) << Describe(result);
    EXPECT_FALSE(result.GetBlockingInstances().empty());
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetClass("RequiredTest", "Base")->GetPropertyP("Required"));
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Name FROM rt.Child"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_STREQ("keep", stmt.GetValueText(0));
}

TEST_F(SchemaEditorCommandsTests, CustomAttributeReorderingPreservesImmutableValuesAndUniqueOrdinals) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorAttributeOrder.ecdb"));
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->CreateSchema({{"name", "OrderTest"}, {"alias", "ot"}, {"version", "01.00.00"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "ot"}, {"name", "Item"}, {"mapStrategy", "TablePerHierarchy"}}).IsOk());
        BeJsDocument hidden(R"json({"container":"ot:Item","caClass":"CoreCustomAttributes:HiddenClass","instance":{}})json");
        auto result = editor->Execute("SetCustomAttribute", hidden);
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    BeJsDocument reorder(R"json({"container":"ot:Item","caClass":"ECDbMap:ClassMap","instance":{"MapStrategy":"TablePerHierarchy"},"ordinal":1})json");
    auto result = editor->Execute("SetCustomAttribute", reorder);
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    reorder["instance"]["MapStrategy"] = "NotMapped";
    result = editor->Validate("SetCustomAttribute", reorder);
    EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, result.GetStatus()) << Describe(result);
    reorder["instance"]["MapStrategy"] = "TablePerHierarchy";
    reorder["ordinal"] = 2;
    result = editor->Validate("SetCustomAttribute", reorder);
    EXPECT_EQ(SchemaCommandResult::Status::ERROR_INVALID_ARGS, result.GetStatus()) << Describe(result);
    ASSERT_TRUE(editor->SaveChanges().IsOk());
    editor.reset();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb,
                                         "SELECT k.Name,x.Ordinal FROM ec_CustomAttribute x JOIN ec_Class k ON k.Id=x.ClassId"
                                         " WHERE x.ContainerType=30 AND x.ContainerId=(SELECT Id FROM ec_Class WHERE Name='Item') ORDER BY x.Ordinal"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_STREQ("HiddenClass", stmt.GetValueText(0));
    EXPECT_EQ(0, stmt.GetValueInt(1));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_STREQ("ClassMap", stmt.GetValueText(0));
    EXPECT_EQ(1, stmt.GetValueInt(1));
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step());
}

TEST_F(SchemaEditorCommandsTests, PersistsClassesPropertiesAndDataAcrossReopen) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorCommandPersistence.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    auto result = editor->CreateSchema({{"name", "CommandSchema"}, {"alias", "cmd"}, {"version", "01.00.00"}});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    result = editor->CreateEntityClass({{"schema", "cmd"}, {"name", "Item"}});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    result = editor->AddProperty({{"class", "cmd:Item"}, {"name", "Title"}, {"kind", "primitive"}, {"type", "string"}});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    // Fix persisted tests using reserved SQLite keywords (cmd, dt, rel, md) in ECSQLINSERT/SELECT/UPDATE
    result = editor->AddProperty({{"class", "cmd:Item"}, {"name", "Count"}, {"kind", "primitive"}, {"type", "long"}});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO cmd.Item([Title],[Count]) VALUES('persistent',42)"));
    result = editor->SaveChanges();
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    editor.reset();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT [Title],[Count] FROM cmd.Item"));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    EXPECT_STREQ("persistent", statement.GetValueText(0));
    EXPECT_EQ(42, statement.GetValueInt64(1));
    EXPECT_EQ(BE_SQLITE_DONE, statement.Step());
    auto ecClass = m_ecdb.Schemas().GetClass("CommandSchema", "Item");
    ASSERT_NE(nullptr, ecClass);
    ASSERT_NE(nullptr, ecClass->GetPropertyP("Title", false));
    ASSERT_NE(nullptr, ecClass->GetPropertyP("Count", false));
}

TEST_F(SchemaEditorCommandsTests, ValidateDoesNotPersistAndRejectsMalformedVersion) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorCommandValidate.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    auto result = editor->Validate("CreateSchema", SchemaCommandArgs{{"name", "DryRun"}, {"alias", "dry"}, {"version", "01.00.00"}});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetSchema("DryRun"));
    for (auto version : {"1.0", "1.0.0junk", "-1.0.0", "4294967296.0.0", "1..0", "1.0.0.0"}) {
        result = editor->Validate("CreateSchema", SchemaCommandArgs{{"name", "DryRun"}, {"alias", "dry"}, {"version", version}});
        EXPECT_EQ(SchemaCommandResult::Status::ERROR_INVALID_ARGS, result.GetStatus()) << version << ": " << Describe(result);
    }
    result = editor->CreateSchema({{"name", "DryRun"}, {"alias", "dry"}, {"version", "01.00.00"}});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    result = editor->Validate("CreateEntityClass", SchemaCommandArgs{{"schema", "dry"}, {"name", "Unwritten"}});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetClass("DryRun", "Unwritten"));
    result = editor->SaveChanges();
    ASSERT_TRUE(result.IsOk()) << Describe(result);
}

TEST_F(SchemaEditorCommandsTests, NonNullPropertyDropRollsBackEarlierCommands) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorCommandRefuseDataDeletion.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->CreateSchema({{"name", "DropTest"}, {"alias", "dt"}, {"version", "01.00.00"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "dt"}, {"name", "Item"}}).IsOk());
        ASSERT_TRUE(editor->AddProperty({{"class", "dt:Item"}, {"name", "Value"}, {"type", "string"}}).IsOk());
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO dt.Item([Value]) VALUES('keep')"));
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    auto result = editor->AlterSchema({{"schema", "dt"}, {"version", "02.00.00"}, {"label", "Must roll back"}});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    result = editor->DropProperty({{"class", "dt:Item"}, {"property", "Value"}});
    ASSERT_EQ(SchemaCommandResult::Status::ERROR_DATA_DELETION_REQUIRED, result.GetStatus()) << Describe(result);
    EXPECT_FALSE(result.GetBlockingInstances().empty());
    auto schema = m_ecdb.Schemas().GetSchema("DropTest");
    ASSERT_NE(nullptr, schema);
    EXPECT_EQ(1, schema->GetVersionRead());
    EXPECT_FALSE(schema->GetIsDisplayLabelDefined());
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT [Value] FROM dt.Item"));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    EXPECT_STREQ("keep", statement.GetValueText(0));
}

TEST_F(SchemaEditorCommandsTests, EmptyPropertyDropRequiresExplicitMajorUpgrade) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorCommandMajorVersion.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->CreateSchema({{"name", "VersionTest"}, {"alias", "vt"}, {"version", "01.00.00"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "vt"}, {"name", "Item"}}).IsOk());
        ASSERT_TRUE(editor->AddProperty({{"class", "vt:Item"}, {"name", "Empty"}, {"type", "int"}}).IsOk());
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        auto result = editor->DropProperty({{"class", "vt:Item"}, {"property", "Empty"}});
        EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, result.GetStatus()) << Describe(result);
    }
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    ASSERT_TRUE(editor->AlterSchema({{"schema", "vt"}, {"version", "02.00.00"}}).IsOk());
    auto result = editor->DropProperty({{"class", "vt:Item"}, {"property", "Empty"}});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ASSERT_TRUE(editor->SaveChanges().IsOk());
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    auto ecClass = m_ecdb.Schemas().GetClass("VersionTest", "Item");
    ASSERT_NE(nullptr, ecClass);
    EXPECT_EQ(nullptr, ecClass->GetPropertyP("Empty"));
}

TEST_F(SchemaEditorCommandsTests, EnumerationReferencesArePersistedAndPreventCascadeDrop) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorCommandEnumeration.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->CreateSchema({{"name", "Values"}, {"alias", "val"}, {"version", "01.00.00"}}).IsOk());
        BeJsDocument enumeration(R"({"schema":"val","name":"State","backingType":"int","enumerators":[{"name":"Off","value":0},{"name":"On","value":1,"label":"Enabled"}]})");
        auto result = editor->Execute("CreateEnumeration", enumeration);
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        ASSERT_TRUE(editor->AddEnumerator({{"enum", "val:State"}, {"enumerator", "Unknown"}, {"value", "-1"}}).IsOk());
        ASSERT_TRUE(editor->CreateSchema({{"name", "Consumers"}, {"alias", "con"}, {"version", "01.00.00"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "con"}, {"name", "Item"}}).IsOk());
        result = editor->AddProperty({{"class", "con:Item"}, {"name", "State"}, {"enum", "val:State"}});
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    auto enumeration = m_ecdb.Schemas().GetEnumeration("Values", "State");
    ASSERT_NE(nullptr, enumeration);
    EXPECT_EQ(3, enumeration->GetEnumeratorCount());
    ASSERT_NE(nullptr, enumeration->FindEnumeratorByName("Unknown"));
    auto schema = m_ecdb.Schemas().GetSchema("Consumers");
    ASSERT_NE(nullptr, schema);
    EXPECT_FALSE(schema->GetReferencedSchemas().empty());
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    ASSERT_TRUE(editor->AlterSchema({{"schema", "val"}, {"version", "02.00.00"}}).IsOk());
    auto result = editor->DropEnumeration({{"enum", "val:State"}});
    EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, result.GetStatus()) << Describe(result);
    ASSERT_NE(nullptr, m_ecdb.Schemas().GetClass("Consumers", "Item")->GetPropertyP("State"));
}

TEST_F(SchemaEditorCommandsTests, StructAndArrayMetadataRoundTrip) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorCommandStruct.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    ASSERT_TRUE(editor->CreateSchema({{"name", "Structures"}, {"alias", "st"}, {"version", "01.00.00"}}).IsOk());
    ASSERT_TRUE(editor->CreateStructClass({{"schema", "st"}, {"name", "Location"}}).IsOk());
    ASSERT_TRUE(editor->AddProperty({{"class", "st:Location"}, {"name", "X"}, {"type", "double"}}).IsOk());
    ASSERT_TRUE(editor->CreateEntityClass({{"schema", "st"}, {"name", "Item"}}).IsOk());
    auto result = editor->AddProperty({{"class", "st:Item"}, {"name", "Position"}, {"kind", "struct"}, {"struct", "st:Location"}});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    result = editor->AddProperty({{"class", "st:Item"}, {"name", "Samples"}, {"kind", "primitiveArray"}, {"type", "double"}, {"minOccurs", "0"}, {"maxOccurs", "8"}});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO st.Item(Position.X) VALUES(12.5)"));
    result = editor->SaveChanges();
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    editor.reset();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    auto ecClass = m_ecdb.Schemas().GetClass("Structures", "Item");
    ASSERT_NE(nullptr, ecClass);
    auto samples = ecClass->GetPropertyP("Samples")->GetAsArrayProperty();
    ASSERT_NE(nullptr, samples);
    EXPECT_EQ(8, samples->GetStoredMaxOccurs());
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Position.X FROM st.Item"));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    EXPECT_DOUBLE_EQ(12.5, statement.GetValueDouble(0));
}

TEST_F(SchemaEditorCommandsTests, RelationshipAndNavigationAreMappedTogether) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorCommandForeignKey.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    ASSERT_TRUE(editor->CreateSchema({{"name", "Relationships"}, {"alias", "rel"}, {"version", "01.00.00"}}).IsOk());
    ASSERT_TRUE(editor->CreateEntityClass({{"schema", "rel"}, {"name", "Parent"}}).IsOk());
    ASSERT_TRUE(editor->CreateEntityClass({{"schema", "rel"}, {"name", "Child"}}).IsOk());
    BeJsDocument relationship(R"json({
        "schema":"rel","name":"Owns","strength":"Embedding","direction":"Forward","mapAs":"foreignKey",
        "source":{"classes":["rel:Parent"],"multiplicity":"(0..1)","roleLabel":"parent","polymorphic":true},
        "target":{"classes":["rel:Child"],"multiplicity":"(0..*)","roleLabel":"children","polymorphic":true},
        "navProperty":{"class":"rel:Child","name":"Parent","direction":"Backward"},"fkOnDelete":"Cascade"
    })json");
    auto result = editor->Execute("CreateRelationshipClass", relationship);
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO rel.[Parent](ECInstanceId) VALUES(1)"));
    {
        ECSqlStatement insertChild;
        ASSERT_EQ(ECSqlStatus::Success, insertChild.Prepare(m_ecdb, "INSERT INTO rel.Child([Parent]) VALUES(?)"));
        ASSERT_EQ(ECSqlStatus::Success, insertChild.BindNavigationValue(1, ECInstanceId(UINT64_C(1)), m_ecdb.Schemas().GetClassId("Relationships", "Owns")));
        ASSERT_EQ(BE_SQLITE_DONE, insertChild.Step());
    }
    ASSERT_TRUE(editor->SaveChanges().IsOk());
    editor.reset();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT SourceECInstanceId,TargetECInstanceId FROM rel.Owns"));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    EXPECT_EQ(1, statement.GetValueInt64(0));
    EXPECT_GT(statement.GetValueInt64(1), 0);
}

TEST_F(SchemaEditorCommandsTests, UnknownMappingOptionsAndExistingQueryViewChangesFailExplicitly) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorCommandUnsupported.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    ASSERT_TRUE(editor->CreateSchema({{"name", "Options"}, {"alias", "opt"}, {"version", "01.00.00"}}).IsOk());
    auto result = editor->Validate("CreateEntityClass", SchemaCommandArgs{{"schema", "opt"}, {"name", "Item"}, {"mapStrategy", "madeUp"}});
    EXPECT_EQ(SchemaCommandResult::Status::ERROR_INVALID_ARGS, result.GetStatus()) << Describe(result);
    ASSERT_TRUE(editor->CreateEntityClass({{"schema", "opt"}, {"name", "Item"}}).IsOk());
    result = editor->Validate("AlterEntityClass", SchemaCommandArgs{{"class", "opt:Item"}, {"queryView", "SELECT 1"}});
    EXPECT_EQ(SchemaCommandResult::Status::ERROR_INVALID_ARGS, result.GetStatus()) << Describe(result);
    result = editor->Validate("AddProperty", SchemaCommandArgs{{"class", "opt:Item"}, {"name", "Bad"}, {"kind", "unknown"}, {"type", "string"}});
    EXPECT_EQ(SchemaCommandResult::Status::ERROR_INVALID_ARGS, result.GetStatus()) << Describe(result);
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetClass("Options", "Item")->GetPropertyP("Bad"));
    editor->AbandonChanges();
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetSchema("Options"));
}

TEST_F(SchemaEditorCommandsTests, MixinPropertiesPersistAndNonNullRemovalIsRefused) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorCommandMixin.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->CreateSchema({{"name", "Mixins"}, {"alias", "mx"}, {"version", "01.00.00"}}).IsOk());
        auto result = editor->CreateEntityClass({{"schema", "mx"}, {"name", "Base"}, {"mapStrategy", "TablePerHierarchy"}});
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        result = editor->CreateMixin({{"schema", "mx"}, {"name", "Named"}, {"appliesTo", "mx:Base"}});
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        result = editor->AddProperty({{"class", "mx:Named"}, {"name", "Name"}, {"type", "string"}});
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        result = editor->CreateEntityClass({{"schema", "mx"}, {"name", "Item"}, {"baseClass", "mx:Base"}, {"mixins", "mx:Named"}});
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "mx"}, {"name", "Descendant"}, {"baseClass", "mx:Item"}}).IsOk());
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO mx.Descendant(Name) VALUES('preserved mixin value')"));
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    ASSERT_TRUE(editor->AlterSchema({{"schema", "mx"}, {"version", "02.00.00"}}).IsOk());
    auto result = editor->Validate("DropProperty", SchemaCommandArgs{{"class", "mx:Named"}, {"property", "Name"}});
    ASSERT_EQ(SchemaCommandResult::Status::ERROR_DATA_DELETION_REQUIRED, result.GetStatus()) << Describe(result);
    EXPECT_FALSE(result.GetBlockingInstances().empty());
    result = editor->RemoveMixin({{"class", "mx:Item"}, {"mixin", "mx:Named"}});
    ASSERT_EQ(SchemaCommandResult::Status::ERROR_DATA_DELETION_REQUIRED, result.GetStatus()) << Describe(result);
    EXPECT_FALSE(result.GetBlockingInstances().empty());
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Name FROM mx.Item"));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    EXPECT_STREQ("preserved mixin value", statement.GetValueText(0));
}

TEST_F(SchemaEditorCommandsTests, CrossSchemaDryRunDoesNotMutateLoadedInheritance) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorCommandDetachedCopy.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    ASSERT_TRUE(editor->CreateSchema({{"name", "BaseSchema"}, {"alias", "bs"}, {"version", "01.00.00"}}).IsOk());
    auto result = editor->CreateEntityClass({{"schema", "bs"}, {"name", "Base"}, {"mapStrategy", "TablePerHierarchy"}});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ASSERT_TRUE(editor->CreateSchema({{"name", "OtherSchema"}, {"alias", "os"}, {"version", "01.00.00"}}).IsOk());
    auto base = m_ecdb.Schemas().GetClass("BaseSchema", "Base");
    ASSERT_NE(nullptr, base);
    auto count = base->GetDerivedClasses().size();
    result = editor->Validate("CreateEntityClass", SchemaCommandArgs{{"schema", "os"}, {"name", "Unwritten"}, {"baseClass", "bs:Base"}});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    EXPECT_EQ(count, base->GetDerivedClasses().size());
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetClass("OtherSchema", "Unwritten"));
    ASSERT_TRUE(editor->SaveChanges().IsOk());
}

TEST_F(SchemaEditorCommandsTests, EnumeratorDeletionRewritesNewMetadataButHonorsPersistedUpgradeRules) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorCommandEnumeratorDeletion.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->CreateSchema({{"name", "EnumeratorRules"}, {"alias", "er"}, {"version", "01.00.00"}}).IsOk());
        BeJsDocument definition(R"({"schema":"er","name":"State","backingType":"int","isStrict":false,"enumerators":[{"name":"Keep","value":1},{"name":"Remove","value":2}]})");
        ASSERT_TRUE(editor->Execute("CreateEnumeration", definition).IsOk());
        auto result = editor->DropEnumerator({{"enum", "er:State"}, {"enumerator", "Remove"}});
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    auto persisted = m_ecdb.Schemas().GetEnumeration("EnumeratorRules", "State");
    ASSERT_NE(nullptr, persisted);
    ASSERT_EQ(1, persisted->GetEnumeratorCount());
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    auto result = editor->Validate("DropEnumerator", SchemaCommandArgs{{"enum", "er:State"}, {"enumerator", "Keep"}});
    EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, result.GetStatus()) << Describe(result);
    ASSERT_TRUE(editor->AlterSchema({{"schema", "er"}, {"version", "02.00.00"}}).IsOk());
    result = editor->Validate("DropEnumerator", SchemaCommandArgs{{"enum", "er:State"}, {"enumerator", "Keep"}});
    EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, result.GetStatus()) << Describe(result);
    ASSERT_TRUE(editor->SaveChanges().IsOk());
    editor.reset();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    auto enumeration = m_ecdb.Schemas().GetEnumeration("EnumeratorRules", "State");
    ASSERT_NE(nullptr, enumeration);
    EXPECT_EQ(1, enumeration->GetEnumeratorCount());
    EXPECT_NE(nullptr, enumeration->FindEnumeratorByName("Keep"));
    EXPECT_EQ(nullptr, enumeration->FindEnumeratorByName("Remove"));
}

TEST_F(SchemaEditorCommandsTests, StructMemberDropChecksNestedArrayValuesAndCleansFlattenedPaths) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorCommandStructMemberDrop.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->CreateSchema({{"name", "MemberDrop"}, {"alias", "md"}, {"version", "01.00.00"}}).IsOk());
        ASSERT_TRUE(editor->CreateStructClass({{"schema", "md"}, {"name", "Value"}}).IsOk());
        ASSERT_TRUE(editor->AddProperty({{"class", "md:Value"}, {"name", "Removed"}, {"type", "string"}}).IsOk());
        ASSERT_TRUE(editor->AddProperty({{"class", "md:Value"}, {"name", "Kept"}, {"type", "string"}}).IsOk());
        ASSERT_TRUE(editor->CreateStructClass({{"schema", "md"}, {"name", "Wrapper"}}).IsOk());
        ASSERT_TRUE(editor->AddProperty({{"class", "md:Wrapper"}, {"name", "Inner"}, {"kind", "struct"}, {"struct", "md:Value"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "md"}, {"name", "Item"}}).IsOk());
        ASSERT_TRUE(editor->AddProperty({{"class", "md:Item"}, {"name", "Scalar"}, {"kind", "struct"}, {"struct", "md:Wrapper"}}).IsOk());
        ASSERT_TRUE(editor->AddProperty({{"class", "md:Item"}, {"name", "Items"}, {"kind", "structArray"}, {"struct", "md:Wrapper"}}).IsOk());
        ECSqlStatement insert;
        ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(m_ecdb, "INSERT INTO md.Item([Scalar].[Inner].Kept,Items) VALUES('scalar retained',?)"));
        auto& element = insert.GetBinder(1).AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Success, element["Inner"]["Removed"].BindText("array blocker", IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(ECSqlStatus::Success, element["Inner"]["Kept"].BindText("array retained", IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(BE_SQLITE_DONE, insert.Step());
        insert.Finalize();
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    ASSERT_TRUE(editor->AlterSchema({{"schema", "md"}, {"version", "02.00.00"}}).IsOk());
    auto result = editor->Validate("DropProperty", SchemaCommandArgs{{"class", "md:Value"}, {"property", "Removed"}});
    ASSERT_EQ(SchemaCommandResult::Status::ERROR_DATA_DELETION_REQUIRED, result.GetStatus()) << Describe(result);
    EXPECT_FALSE(result.GetBlockingInstances().empty());
    {
        ECSqlStatement update;
        ASSERT_EQ(ECSqlStatus::Success, update.Prepare(m_ecdb, "UPDATE md.Item SET Items=?"));
        auto& element = update.GetBinder(1).AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Success, element["Inner"]["Kept"].BindText("array retained", IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(BE_SQLITE_DONE, update.Step());
    }
    result = editor->DropProperty({{"class", "md:Value"}, {"property", "Removed"}});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ASSERT_TRUE(editor->SaveChanges().IsOk());
    editor.reset();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetClass("MemberDrop", "Value")->GetPropertyP("Removed"));
    ECSqlStatement values;
    ASSERT_EQ(ECSqlStatus::Success, values.Prepare(m_ecdb, "SELECT [Scalar].[Inner].Kept,Items FROM md.Item"));
    ASSERT_EQ(BE_SQLITE_ROW, values.Step());
    EXPECT_STREQ("scalar retained", values.GetValueText(0));
    EXPECT_EQ(1, values.GetValue(1).GetArrayLength());
    for (auto const& element : values.GetValue(1).GetArrayIterable())
        EXPECT_STREQ("array retained", element["Inner"]["Kept"].GetText());
    Statement paths;
    ASSERT_EQ(BE_SQLITE_OK, paths.Prepare(m_ecdb, "SELECT COUNT(*) FROM ec_PropertyPath WHERE AccessString='Scalar.Inner.Removed'"));
    ASSERT_EQ(BE_SQLITE_ROW, paths.Step());
    EXPECT_EQ(0, paths.GetValueInt(0));
}

TEST_F(SchemaEditorCommandsTests, RemovingEmptyMixinPropertiesPreservesOtherConsumers) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorCommandEmptyMixinRemoval.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->CreateSchema({{"name", "MixinRemoval"}, {"alias", "mr"}, {"version", "01.00.00"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "mr"}, {"name", "Base"}, {"mapStrategy", "TablePerHierarchy"}, {"shareColumns", "true"}, {"applyToSubclassesOnly", "true"}, {"maxSharedColumnsBeforeOverflow", "8"}}).IsOk());
        ASSERT_TRUE(editor->AddProperty({{"class", "mr:Base"}, {"name", "Kept"}, {"type", "string"}}).IsOk());
        ASSERT_TRUE(editor->CreateMixin({{"schema", "mr"}, {"name", "Named"}, {"appliesTo", "mr:Base"}}).IsOk());
        ASSERT_TRUE(editor->AddProperty({{"class", "mr:Named"}, {"name", "Name"}, {"type", "string"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "mr"}, {"name", "Target"}, {"baseClass", "mr:Base"}, {"mixins", "mr:Named"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "mr"}, {"name", "Descendant"}, {"baseClass", "mr:Target"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "mr"}, {"name", "Other"}, {"baseClass", "mr:Base"}, {"mixins", "mr:Named"}}).IsOk());
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO mr.Descendant(Kept) VALUES('target retained')"));
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO mr.Other(Kept,Name) VALUES('other retained','mixin retained')"));
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    ASSERT_TRUE(editor->AlterSchema({{"schema", "mr"}, {"version", "02.00.00"}}).IsOk());
    auto result = editor->RemoveMixin({{"class", "mr:Target"}, {"mixin", "mr:Named"}});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ASSERT_TRUE(editor->SaveChanges().IsOk());
    editor.reset();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    auto target = m_ecdb.Schemas().GetClass("MixinRemoval", "Target");
    auto descendant = m_ecdb.Schemas().GetClass("MixinRemoval", "Descendant");
    auto other = m_ecdb.Schemas().GetClass("MixinRemoval", "Other");
    ASSERT_NE(nullptr, target);
    ASSERT_NE(nullptr, descendant);
    ASSERT_NE(nullptr, other);
    EXPECT_EQ(nullptr, target->GetPropertyP("Name"));
    EXPECT_EQ(nullptr, descendant->GetPropertyP("Name"));
    EXPECT_NE(nullptr, other->GetPropertyP("Name"));
    ECSqlStatement values;
    ASSERT_EQ(ECSqlStatus::Success, values.Prepare(m_ecdb, "SELECT Kept FROM mr.Descendant"));
    ASSERT_EQ(BE_SQLITE_ROW, values.Step());
    EXPECT_STREQ("target retained", values.GetValueText(0));
    values.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, values.Prepare(m_ecdb, "SELECT Kept,Name FROM mr.Other"));
    ASSERT_EQ(BE_SQLITE_ROW, values.Step());
    EXPECT_STREQ("other retained", values.GetValueText(0));
    EXPECT_STREQ("mixin retained", values.GetValueText(1));
}

TEST_F(SchemaEditorCommandsTests, NavigationDropReportsDataBeforeTheForeignKeyUnmappingLimit) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorCommandNavigationDrop.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->CreateSchema({{"name", "NavigationDrop"}, {"alias", "nd"}, {"version", "01.00.00"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "nd"}, {"name", "Parent"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "nd"}, {"name", "Child"}}).IsOk());
        BeJsDocument relationship(R"json({
        "schema":"nd","name":"Owns","strength":"Referencing","direction":"Forward","mapAs":"foreignKey",
        "source":{"classes":["nd:Parent"],"multiplicity":"(0..1)","roleLabel":"parent","polymorphic":true},
        "target":{"classes":["nd:Child"],"multiplicity":"(0..*)","roleLabel":"children","polymorphic":true},
        "navProperty":{"class":"nd:Child","name":"Parent","direction":"Backward"}
    })json");
        ASSERT_TRUE(editor->Execute("CreateRelationshipClass", relationship).IsOk());
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO nd.Parent(ECInstanceId) VALUES(1)"));
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO nd.Child(Parent.Id) VALUES(1)"));
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    ASSERT_TRUE(editor->AlterSchema({{"schema", "nd"}, {"version", "02.00.00"}}).IsOk());
    auto result = editor->Validate("DropProperty", SchemaCommandArgs{{"class", "nd:Child"}, {"property", "Parent"}});
    ASSERT_EQ(SchemaCommandResult::Status::ERROR_DATA_DELETION_REQUIRED, result.GetStatus()) << Describe(result);
    EXPECT_FALSE(result.GetBlockingInstances().empty());
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("UPDATE nd.Child SET Parent.Id=NULL"));
    result = editor->Validate("DropProperty", SchemaCommandArgs{{"class", "nd:Child"}, {"property", "Parent"}});
    EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, result.GetStatus()) << Describe(result);
    ASSERT_FALSE(result.GetIssues().empty());
    EXPECT_NE(Utf8String::npos, result.GetIssues().front().message.find("unmapping collector"));
    EXPECT_NE(nullptr, m_ecdb.Schemas().GetClass("NavigationDrop", "Child")->GetPropertyP("Parent"));
}

TEST_F(SchemaEditorCommandsTests, MappingCustomAttributeRulesAndIndexModification) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorMappingCARules.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->CreateSchema({{"name", "MapRules"}, {"alias", "mr"}, {"version", "01.00.00"}}).IsOk());
        ASSERT_FALSE(editor->Validate("CreateEntityClass", SchemaCommandArgs{{"schema", "mr"}, {"name", "Base"}, {"mapStrategy", "TablePerHierarchy"}, {"indexes", "ix_base_code:Code:unique:where=IndexedColumnsAreNotNull"}}).IsOk()); // Code not added yet

        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "mr"}, {"name", "Base"}, {"mapStrategy", "TablePerHierarchy"}}).IsOk());
        ASSERT_TRUE(editor->AddProperty({{"class", "mr:Base"}, {"name", "Code"}, {"type", "string"}}).IsOk());
        ASSERT_TRUE(editor->AddProperty({{"class", "mr:Base"}, {"name", "Tag"}, {"type", "string"}}).IsOk());

        // Cannot change ClassMap / undo TablePerHierarchy via SetCustomAttribute or RemoveCustomAttribute
        BeJsDocument classMapCa(R"json({"container":"mr:Base","caClass":"ECDbMap:ClassMap","instance":{"MapStrategy":"OwnTable"}})json");
        auto res = editor->Validate("SetCustomAttribute", classMapCa);
        EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, res.GetStatus()) << Describe(res);
        res = editor->Validate("RemoveCustomAttribute", SchemaCommandArgs{{"container", "mr:Base"}, {"caClass", "ECDbMap:ClassMap"}});
        EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, res.GetStatus()) << Describe(res);

        // Can add indexes via AlterEntityClass using KV string syntax
        res = editor->AlterEntityClass({{"class", "mr:Base"}, {"indexes", "ix_base_code:Code:unique:where=IndexedColumnsAreNotNull"}});
        ASSERT_TRUE(res.IsOk()) << Describe(res);

        // Can add a second index and modify properties on the first index, but cannot remove or rename existing index
        res = editor->Validate("AlterEntityClass", SchemaCommandArgs{{"class", "mr:Base"}, {"indexes", "ix_renamed:Code:unique:where=IndexedColumnsAreNotNull"}});
        EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, res.GetStatus()) << Describe(res);

        res = editor->AlterEntityClass({{"class", "mr:Base"},
                                        {"indexes", "ix_base_code:Code+Tag:unique:where=IndexedColumnsAreNotNull; ix_base_tag:Tag"}});
        ASSERT_TRUE(res.IsOk()) << Describe(res);
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM main.ec_Index WHERE Name IN ('ix_base_code','ix_base_tag')"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(2, stmt.GetValueInt(0));
}

TEST_F(SchemaEditorCommandsTests, AddBasePropertyWithIncompatibleDerivedPropertyTypeFails) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorBasePropIncompatible.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->CreateSchema({{"name", "Hierarchy"}, {"alias", "h"}, {"version", "01.00.00"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "h"}, {"name", "Base"}, {"mapStrategy", "TablePerHierarchy"}}).IsOk());
        ASSERT_TRUE(editor->AddProperty({{"class", "h:Base"}, {"name", "RootTag"}, {"type", "string"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "h"}, {"name", "Child"}, {"baseClass", "h:Base"}}).IsOk());
        ASSERT_TRUE(editor->AddProperty({{"class", "h:Child"}, {"name", "Prop"}, {"type", "point3d"}}).IsOk());
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    auto editor = SchemaEditor::Begin(m_ecdb, SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade);
    ASSERT_NE(nullptr, editor);
    ASSERT_TRUE(editor->AlterSchema({{"schema", "h"}, {"version", "02.00.00"}}).IsOk());
    // Adding string Prop on Base when Child has point3d Prop must fail validation
    auto res = editor->Validate("AddProperty", SchemaCommandArgs{{"class", "h:Base"}, {"name", "Prop"}, {"type", "string"}});
    EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, res.GetStatus()) << Describe(res);
}

TEST_F(SchemaEditorCommandsTests, AddBasePropertyWithCompatibleDerivedTypeConvertsAndMovesData) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorBasePropTypeConvert.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->CreateSchema({{"name", "Hierarchy"}, {"alias", "h"}, {"version", "01.00.00"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "h"}, {"name", "Base"}, {"mapStrategy", "TablePerHierarchy"}}).IsOk());
        ASSERT_TRUE(editor->AddProperty({{"class", "h:Base"}, {"name", "RootTag"}, {"type", "string"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "h"}, {"name", "Child"}, {"baseClass", "h:Base"}}).IsOk());
        ASSERT_TRUE(editor->AddProperty({{"class", "h:Child"}, {"name", "Code"}, {"type", "int"}}).IsOk());
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO h.Child(ECInstanceId, Code) VALUES(1, 42)"));
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    // Without major version bump on Child's schema, compatible primitive type change fails validation
    {
        auto editor = SchemaEditor::Begin(m_ecdb, SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade);
        ASSERT_NE(nullptr, editor);
        auto res = editor->Validate("AddProperty", SchemaCommandArgs{{"class", "h:Base"}, {"name", "Code"}, {"type", "string"}});
        EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, res.GetStatus()) << Describe(res);
        editor->AbandonChanges();
    }
    // With major version bump and AllowDataTransformDuringSchemaUpgrade, int -> string converts and moves data to new TEXT column
    {
        auto editor = SchemaEditor::Begin(m_ecdb, SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->AlterSchema({{"schema", "h"}, {"version", "02.00.00"}}).IsOk());
        auto res = editor->AddProperty({{"class", "h:Base"}, {"name", "Code"}, {"type", "string"}});
        ASSERT_TRUE(res.IsOk()) << Describe(res);
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Code FROM h.Base WHERE ECInstanceId=1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_STREQ("42", stmt.GetValueText(0));
}

TEST_F(SchemaEditorCommandsTests, AddBasePropertyWithSameTypeAndColumnCollisionMovesData) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorBasePropColumnCollision.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->CreateSchema({{"name", "Hierarchy"}, {"alias", "h"}, {"version", "01.00.00"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "h"}, {"name", "Base"}, {"mapStrategy", "TablePerHierarchy"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "h"}, {"name", "Child1"}, {"baseClass", "h:Base"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "h"}, {"name", "Child2"}, {"baseClass", "h:Base"}}).IsOk());
        // Child1 claims physical column 'Tag' for an integer property 'Tag'
        ASSERT_TRUE(editor->AddProperty({{"class", "h:Child1"}, {"name", "Tag"}, {"type", "int"}}).IsOk());
        // Child2 adds string property 'Tag' (which cannot use column 'Tag' because Child1's int Tag uses it, so it gets mapped to a prefixed column)
        ASSERT_TRUE(editor->AddProperty({{"class", "h:Child2"}, {"name", "Tag"}, {"type", "string"}}).IsOk());
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO h.Child1(ECInstanceId, Tag) VALUES(1, 100)"));
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO h.Child2(ECInstanceId, Tag) VALUES(2, 'moved-tag-val')"));
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    // Without AllowDataTransformDuringSchemaUpgrade, adding string 'Tag' to Base must fail with ERROR_DATA_TRANSFORM_REQUIRED
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->AlterSchema({{"schema", "h"}, {"version", "02.00.00"}}).IsOk());
        auto res = editor->AddProperty({{"class", "h:Base"}, {"name", "Tag"}, {"type", "string"}});
        EXPECT_EQ(SchemaCommandResult::Status::ERROR_DATA_TRANSFORM_REQUIRED, res.GetStatus()) << Describe(res);
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    // With AllowDataTransformDuringSchemaUpgrade, adding string 'Tag' to Base allocates a new TEXT column for Base::Tag
    // and moves both Child1::Tag (converted to TEXT) and Child2::Tag (same exact type, moved from its prefixed column) into Base::Tag's column
    {
        auto editor = SchemaEditor::Begin(m_ecdb, SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->AlterSchema({{"schema", "h"}, {"version", "02.00.00"}}).IsOk());
        auto res = editor->AddProperty({{"class", "h:Base"}, {"name", "Tag"}, {"type", "string"}});
        ASSERT_TRUE(res.IsOk()) << Describe(res);
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Tag FROM h.Base WHERE ECInstanceId=2"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_STREQ("moved-tag-val", stmt.GetValueText(0));
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Tag FROM h.Child1 WHERE ECInstanceId=1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_STREQ("100", stmt.GetValueText(0));
}

TEST_F(SchemaEditorCommandsTests, AlterRelationshipClassAndCrossSchemaNavProperty) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorAlterRelationship.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->CreateSchema({{"name", "Core"}, {"alias", "c"}, {"version", "01.00.00"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "c"}, {"name", "Base"}, {"mapStrategy", "TablePerHierarchy"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "c"}, {"name", "Parent"}, {"baseClass", "c:Base"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "c"}, {"name", "Child"}, {"baseClass", "c:Base"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "c"}, {"name", "Unrelated"}}).IsOk());

        // Create a second schema that owns the child entity with a cross-schema navigation property
        ASSERT_TRUE(editor->CreateSchema({{"name", "Domain"}, {"alias", "d"}, {"version", "01.00.00"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "d"}, {"name", "Leaf"}, {"baseClass", "c:Child"}}).IsOk());

        // Create relationship in Domain referencing Core:Parent, with navigation property declared on Domain:Leaf
        auto res = editor->CreateRelationshipClass({{"schema", "d"}, {"name", "ParentOwnsLeaf"}, {"strength", "embedding"}, {"direction", "forward"}, {"mapAs", "foreignKey"}, {"source.classes", "c:Parent"}, {"source.multiplicity", "(0..1)"}, {"source.polymorphic", "false"}, {"source.roleLabel", "owns"}, {"target.classes", "d:Leaf"}, {"target.multiplicity", "(0..2)"}, {"target.polymorphic", "true"}, {"target.roleLabel", "owned by"}, {"navProperty.class", "d:Leaf"}, {"navProperty.name", "OwnerParent"}, {"navProperty.direction", "Backward"}});
        ASSERT_TRUE(res.IsOk()) << Describe(res);

        // Create a third schema that defines a relationship between Core:Parent -> Core:Child while placing the navigation property on Domain:Leaf (rejected by Validate because Domain:Leaf is not a constraint class of BaseLinksChild)
        ASSERT_TRUE(editor->CreateSchema({{"name", "RelSchema"}, {"alias", "rs"}, {"version", "01.00.00"}}).IsOk());
        res = editor->Validate("CreateRelationshipClass", SchemaCommandArgs{
                                                              {"schema", "rs"}, {"name", "BaseLinksChild"}, {"strength", "referencing"}, {"direction", "forward"}, {"mapAs", "foreignKey"}, {"source.classes", "c:Parent"}, {"source.multiplicity", "(0..1)"}, {"source.polymorphic", "false"}, {"source.roleLabel", "refs"}, {"target.classes", "c:Child"}, {"target.multiplicity", "(0..*)"}, {"target.polymorphic", "true"}, {"target.roleLabel", "ref by"}, {"navProperty.class", "d:Leaf"}, {"navProperty.name", "RefParent"}, {"navProperty.direction", "Backward"}});
        EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, res.GetStatus());

        // Placing the navigation property on c:Parent when the relationship is in RelSchema (which references Core) would create a reference cycle (rejected by Validate):
        res = editor->Validate("CreateRelationshipClass", SchemaCommandArgs{
                                                              {"schema", "rs"}, {"name", "ChildHasSingleParent"}, {"strength", "referencing"}, {"direction", "forward"}, {"mapAs", "foreignKey"}, {"source.classes", "c:Parent"}, {"source.multiplicity", "(0..1)"}, {"source.polymorphic", "false"}, {"source.roleLabel", "parent"}, {"target.classes", "c:Child"}, {"target.multiplicity", "(0..1)"}, {"target.polymorphic", "false"}, {"target.roleLabel", "child"}, {"navProperty.class", "c:Parent"}, {"navProperty.name", "SingleChild"}, {"navProperty.direction", "Forward"}});
        EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, res.GetStatus());

        // Create a derived relationship class in Domain inheriting from c:ParentLinksChild (relationship inheritance)
        res = editor->CreateRelationshipClass({{"schema", "c"}, {"name", "ParentLinksChild"}, {"strength", "referencing"}, {"direction", "forward"}, {"mapAs", "linkTable"}, {"source.classes", "c:Parent"}, {"source.multiplicity", "(0..1)"}, {"source.polymorphic", "false"}, {"source.roleLabel", "links"}, {"target.classes", "c:Child"}, {"target.multiplicity", "(0..2)"}, {"target.polymorphic", "true"}, {"target.roleLabel", "linked by"}});
        ASSERT_TRUE(res.IsOk()) << Describe(res);

        res = editor->CreateRelationshipClass({{"schema", "d"}, {"name", "LeafLinksParent"}, {"baseClass", "c:ParentLinksChild"}, {"strength", "referencing"}, {"direction", "forward"}, {"mapAs", "linkTable"}, {"source.classes", "c:Parent"}, {"source.multiplicity", "(0..1)"}, {"source.polymorphic", "false"}, {"source.roleLabel", "links"}, {"target.classes", "d:Leaf"}, {"target.multiplicity", "(0..2)"}, {"target.polymorphic", "false"}, {"target.roleLabel", "linked by"}});
        ASSERT_TRUE(res.IsOk()) << Describe(res);
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);

        // 1. Changing strength or direction is rejected
        auto res = editor->Validate("AlterRelationshipClass", SchemaCommandArgs{{"class", "c:ParentLinksChild"}, {"strength", "holding"}});
        EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, res.GetStatus());
        res = editor->Validate("AlterRelationshipClass", SchemaCommandArgs{{"class", "c:ParentLinksChild"}, {"direction", "backward"}});
        EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, res.GetStatus());

        // 2. Changing multiplicity lower bound or decreasing upper bound is rejected
        res = editor->Validate("AlterRelationshipClass", SchemaCommandArgs{{"class", "c:ParentLinksChild"}, {"target.multiplicity", "(1..2)"}});
        EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, res.GetStatus());
        res = editor->Validate("AlterRelationshipClass", SchemaCommandArgs{{"class", "c:ParentLinksChild"}, {"target.multiplicity", "(0..1)"}});
        EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, res.GetStatus());

        // 3. On FK relationship in target table, changing source.multiplicity is rejected, but increasing target.multiplicity is allowed
        res = editor->Validate("AlterRelationshipClass", SchemaCommandArgs{{"class", "d:ParentOwnsLeaf"}, {"source.multiplicity", "(0..*)"}});
        EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, res.GetStatus());
        res = editor->AlterRelationshipClass({{"class", "d:ParentOwnsLeaf"}, {"target.multiplicity", "(0..*)"}});
        ASSERT_TRUE(res.IsOk()) << Describe(res);

        // 4. Changing polymorphic true -> false is rejected, but false -> true is allowed
        res = editor->Validate("AlterRelationshipClass", SchemaCommandArgs{{"class", "d:ParentOwnsLeaf"}, {"target.polymorphic", "false"}});
        EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, res.GetStatus());

        // 5. Generalizing constraint class to an unrelated class is rejected, but generalizing to Base (with polymorphic=true) is allowed
        res = editor->Validate("AlterRelationshipClass", SchemaCommandArgs{{"class", "c:ParentLinksChild"}, {"source.classes", "c:Unrelated"}, {"source.polymorphic", "true"}});
        EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, res.GetStatus());

        res = editor->AlterRelationshipClass({{"class", "c:ParentLinksChild"},
                                              {"source.classes", "c:Base"},
                                              {"source.polymorphic", "true"},
                                              {"source.multiplicity", "(0..*)"},
                                              {"source.roleLabel", "generalized links"}});
        ASSERT_TRUE(res.IsOk()) << Describe(res);
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    auto rel = m_ecdb.Schemas().GetClass("Core", "ParentLinksChild")->GetRelationshipClassCP();
    ASSERT_NE(nullptr, rel);
    EXPECT_TRUE(rel->GetSource().GetIsPolymorphic());
    EXPECT_EQ(1u, rel->GetSource().GetConstraintClasses().size());
    EXPECT_STREQ("Base", rel->GetSource().GetConstraintClasses()[0]->GetName().c_str());
    EXPECT_STREQ("generalized links", rel->GetSource().GetRoleLabel().c_str());
    EXPECT_TRUE(rel->GetSource().GetMultiplicity().IsUpperLimitUnbounded());
}

TEST_F(SchemaEditorCommandsTests, IsMixinAppliesToGeneralizationAndArrayBoundsValidation) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorMixinGeneralize.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->CreateSchema({{"name", "MixTest"}, {"alias", "mt"}, {"version", "01.00.00"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "mt"}, {"name", "Base"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "mt"}, {"name", "Child"}, {"baseClass", "mt:Base"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "mt"}, {"name", "Other"}}).IsOk());
        ASSERT_TRUE(editor->CreateMixin({{"schema", "mt"}, {"name", "IMix"}, {"appliesTo", "mt:Child"}}).IsOk());
        ASSERT_TRUE(editor->AddProperty({{"class", "mt:Child"}, {"name", "Tags"}, {"kind", "primitiveArray"}, {"type", "string"}, {"minOccurs", "1"}, {"maxOccurs", "5"}}).IsOk());
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);

        // Changing array minOccurs/maxOccurs via AlterProperty must fail with ERROR_VALIDATION
        auto res = editor->Validate("AlterProperty", SchemaCommandArgs{{"class", "mt:Child"}, {"property", "Tags"}, {"maxOccurs", "10"}});
        EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, res.GetStatus()) << Describe(res);

        // Changing IsMixin.AppliesToEntityClass to an unrelated class fails validation
        BeJsDocument badMixinCa(R"json({"container":"mt:IMix","caClass":"CoreCustomAttributes:IsMixin","instance":{"AppliesToEntityClass":"mt:Other"}})json");
        res = editor->Validate("SetCustomAttribute", badMixinCa);
        EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, res.GetStatus()) << Describe(res);

        // Generalizing IsMixin.AppliesToEntityClass from Child to Base succeeds
        BeJsDocument goodMixinCa(R"json({"container":"mt:IMix","caClass":"CoreCustomAttributes:IsMixin","instance":{"AppliesToEntityClass":"mt:Base"}})json");
        res = editor->Execute("SetCustomAttribute", goodMixinCa);
        ASSERT_TRUE(res.IsOk()) << Describe(res);
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    auto mixClass = m_ecdb.Schemas().GetClass("MixTest", "IMix");
    ASSERT_NE(nullptr, mixClass);
    auto ca = mixClass->GetCustomAttributeLocal("CoreCustomAttributes", "IsMixin");
    ASSERT_TRUE(ca.IsValid());
    ECValue v;
    ASSERT_EQ(ECObjectsStatus::Success, ca->GetValue(v, "AppliesToEntityClass"));
    EXPECT_STREQ("mt:Base", v.GetUtf8CP());
}

TEST_F(SchemaEditorCommandsTests, CustomAttributeClassesConstraintContainersTimeStampAndBaseMixinOverrideRules) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorCAAndBaseOverrideRules.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->CreateSchema({{"name", "RuleTest"}, {"alias", "rt"}, {"version", "01.00.00"}}).IsOk());

        // 1. Create a CustomAttributeClass and add properties to it
        auto res = editor->CreateCustomAttributeClass({{"schema", "rt"}, {"name", "ConstraintTag"}, {"appliesTo", "SourceRelationshipConstraint,TargetRelationshipConstraint,EntityClass"}, {"label", "Constraint Tag"}});
        ASSERT_TRUE(res.IsOk()) << Describe(res);
        ASSERT_TRUE(editor->AddProperty({{"class", "rt:ConstraintTag"}, {"name", "Tag"}, {"kind", "primitive"}, {"type", "string"}}).IsOk());

        // Create an unused CustomAttributeClass to test Alter and Drop
        ASSERT_TRUE(editor->CreateCustomAttributeClass({{"schema", "rt"}, {"name", "TempCA"}, {"appliesTo", "Schema"}}).IsOk());
        ASSERT_TRUE(editor->AlterCustomAttributeClass({{"class", "rt:TempCA"}, {"label", "Updated Temp CA"}, {"appliesTo", "Schema,EntityClass"}}).IsOk());

        // 2. Create entity hierarchy with ClassHasCurrentTimeStampProperty on Root (inherited from BaseWithLastMod)
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "rt"}, {"name", "BaseWithLastMod"}, {"modifier", "Abstract"}, {"mapStrategy", "TablePerHierarchy"}}).IsOk());
        ASSERT_TRUE(editor->AddProperty({{"class", "rt:BaseWithLastMod"}, {"name", "LastMod"}, {"kind", "primitive"}, {"type", "dateTime"}, {"readOnly", "true"}}).IsOk());

        BeJsDocument createRoot(R"json({
        "schema":"rt","name":"Root","modifier":"Abstract","baseClass":"rt:BaseWithLastMod",
        "shareColumns":true,"maxSharedColumnsBeforeOverflow":32,
        "customAttributes":[{"className":"CoreCustomAttributes:ClassHasCurrentTimeStampProperty","PropertyName":"LastMod"}]
    })json");
        res = editor->Execute("CreateEntityClass", createRoot);
        ASSERT_TRUE(res.IsOk()) << Describe(res);

        // ClassHasCurrentTimeStampProperty cannot be added, modified, or removed via SetCustomAttribute/RemoveCustomAttribute after creation
        BeJsDocument tsCa(R"json({"container":"rt:Root","caClass":"CoreCustomAttributes:ClassHasCurrentTimeStampProperty","instance":{"PropertyName":"LastMod"}})json");
        res = editor->Validate("SetCustomAttribute", tsCa);
        EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, res.GetStatus());
        res = editor->Validate("RemoveCustomAttribute", SchemaCommandArgs{{"container", "rt:Root"}, {"caClass", "CoreCustomAttributes:ClassHasCurrentTimeStampProperty"}});
        EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, res.GetStatus());

        // AlterProperty setting readOnly=false on LastMod is rejected when ClassHasCurrentTimeStampProperty references it
        res = editor->Validate("AlterProperty", SchemaCommandArgs{{"class", "rt:BaseWithLastMod"}, {"property", "LastMod"}, {"readOnly", "false"}});
        EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, res.GetStatus());

        // 3. Create relationship and apply ConstraintTag custom attribute to source and target constraints
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "rt"}, {"name", "Node"}, {"baseClass", "rt:Root"}}).IsOk());
        ASSERT_TRUE(editor->CreateRelationshipClass({{"schema", "rt"}, {"name", "NodeLinksNode"}, {"strength", "referencing"}, {"direction", "forward"}, {"mapAs", "linkTable"}, {"source.classes", "rt:Node"}, {"source.multiplicity", "(0..*)"}, {"source.polymorphic", "true"}, {"source.roleLabel", "from"}, {"target.classes", "rt:Node"}, {"target.multiplicity", "(0..*)"}, {"target.polymorphic", "true"}, {"target.roleLabel", "to"}}).IsOk());

        BeJsDocument srcCa(R"json({"container":"rt:NodeLinksNode.source","caClass":"rt:ConstraintTag","instance":{"Tag":"src-tag"}})json");
        ASSERT_TRUE(editor->Execute("SetCustomAttribute", srcCa).IsOk());
        BeJsDocument tgtCa(R"json({"container":"rt:NodeLinksNode.target","caClass":"rt:ConstraintTag","instance":{"Tag":"tgt-tag"}})json");
        ASSERT_TRUE(editor->Execute("SetCustomAttribute", tgtCa).IsOk());

        // 4. Create Mid (with Code:string) and Leaf (with Code:int) deriving from Root, plus Mixin with Score:string and ScoreEntity with Score:int
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "rt"}, {"name", "Mid"}, {"baseClass", "rt:Root"}}).IsOk());
        ASSERT_TRUE(editor->AddProperty({{"class", "rt:Mid"}, {"name", "Code"}, {"kind", "primitive"}, {"type", "string"}}).IsOk());
        ASSERT_TRUE(editor->CreateEntityClass({{"schema", "rt"}, {"name", "Leaf"}, {"baseClass", "rt:Root"}}).IsOk());
        ASSERT_TRUE(editor->AddProperty({{"class", "rt:Leaf"}, {"name", "Code"}, {"kind", "primitive"}, {"type", "int"}}).IsOk());
        ASSERT_TRUE(editor->AddProperty({{"class", "rt:Leaf"}, {"name", "Score"}, {"kind", "primitive"}, {"type", "int"}}).IsOk());

        ASSERT_TRUE(editor->CreateMixin({{"schema", "rt"}, {"name", "IScored"}, {"appliesTo", "rt:Root"}}).IsOk());
        ASSERT_TRUE(editor->AddProperty({{"class", "rt:IScored"}, {"name", "Score"}, {"kind", "primitive"}, {"type", "string"}}).IsOk());

        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }

    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO RuleTest.Leaf(Code, Score) VALUES(42, 99)"));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        stmt.Finalize();
        ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    }

    {
        auto editor = SchemaEditor::Begin(m_ecdb, SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade);
        ASSERT_NE(nullptr, editor);

        // Specializing Leaf's baseClass from Root to Mid (which has Code:string while Leaf has Code:int) requires major version bump:
        auto res = editor->Validate("AlterEntityClass", SchemaCommandArgs{{"class", "rt:Leaf"}, {"baseClass", "rt:Mid"}});
        EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, res.GetStatus());

        // Applying mixin IScored (which has Score:string while Leaf has Score:int) also requires major version bump:
        res = editor->Validate("AddMixin", SchemaCommandArgs{{"class", "rt:Leaf"}, {"mixin", "rt:IScored"}});
        EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, res.GetStatus());

        // Bump major version:
        ASSERT_TRUE(editor->AlterSchema({{"schema", "rt"}, {"version", "02.00.00"}}).IsOk());

        // Now specializing Leaf's baseClass to Mid and applying IScored both succeed and convert/move data:
        res = editor->AlterEntityClass({{"class", "rt:Leaf"}, {"baseClass", "rt:Mid"}});
        ASSERT_TRUE(res.IsOk()) << Describe(res);
        res = editor->AddMixin({{"class", "rt:Leaf"}, {"mixin", "rt:IScored"}});
        ASSERT_TRUE(res.IsOk()) << Describe(res);

        // Remove constraint custom attribute from target and drop TempCA
        ASSERT_TRUE(editor->RemoveCustomAttribute({{"container", "rt:NodeLinksNode.target"}, {"caClass", "rt:ConstraintTag"}}).IsOk());
        ASSERT_TRUE(editor->DropCustomAttributeClass({{"class", "rt:TempCA"}}).IsOk());

        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }

    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetClass("RuleTest", "TempCA"));
    auto rel = m_ecdb.Schemas().GetClass("RuleTest", "NodeLinksNode")->GetRelationshipClassCP();
    ASSERT_NE(nullptr, rel);
    EXPECT_TRUE(rel->GetSource().GetCustomAttributeLocal("RuleTest", "ConstraintTag").IsValid());
    EXPECT_FALSE(rel->GetTarget().GetCustomAttributeLocal("RuleTest", "ConstraintTag").IsValid());

    ECSqlStatement select;
    ASSERT_EQ(ECSqlStatus::Success, select.Prepare(m_ecdb, "SELECT Code, Score FROM RuleTest.Leaf"));
    ASSERT_EQ(BE_SQLITE_ROW, select.Step());
    EXPECT_STREQ("42", select.GetValueText(0));
    EXPECT_STREQ("99", select.GetValueText(1));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaEditorCommandsTests, CreateClassesAcceptInlineProperties) {
    // Like CREATE TABLE: a create command defines the class and its properties in one step; indexes may
    // reference inline properties, and ordinals follow the declared order.
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorInlineProperties.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->CreateSchema({{"name", "Inline"}, {"alias", "il"}, {"version", "01.00.00"}}).IsOk());
        BeJsDocument item(R"json({"schema":"il","name":"Item","mapStrategy":"TablePerHierarchy","indexes":"ix_item_code:Code:unique","properties":[
            {"name":"Code","type":"string"},
            {"name":"Count","type":"long"},
            {"name":"Tags","kind":"primitiveArray","type":"string"}]})json");
        auto result = editor->Execute("CreateEntityClass", item);
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        ASSERT_TRUE(editor->Execute("CreateMixin", BeJsDocument(R"json({"schema":"il","name":"INamed","appliesTo":"il:Item",
            "properties":[{"name":"Nickname","type":"string"}]})json")).IsOk());
        result = editor->Execute("CreateCustomAttributeClass", BeJsDocument(R"json({"schema":"il","name":"Note","appliesTo":"Any",
            "properties":[{"name":"Text","type":"string"}]})json"));
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO il.Item([Code],[Count]) VALUES('a',1)"));
        EXPECT_NE(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO il.Item([Code],[Count]) VALUES('a',2)")) << "unique index on the inline property";
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    Statement ordinals;
    ASSERT_EQ(BE_SQLITE_OK, ordinals.Prepare(m_ecdb, "SELECT p.Name,p.Ordinal FROM main.ec_Property p JOIN main.ec_Class c ON c.Id=p.ClassId"
                                                     " WHERE c.Name='Item' ORDER BY p.Ordinal"));
    for (auto name : {"Code", "Count", "Tags"}) {
        ASSERT_EQ(BE_SQLITE_ROW, ordinals.Step());
        EXPECT_STREQ(name, ordinals.GetValueText(0));
    }
    EXPECT_EQ(BE_SQLITE_DONE, ordinals.Step());
    ECSqlStatement select;
    ASSERT_EQ(ECSqlStatus::Success, select.Prepare(m_ecdb, "SELECT [Code],[Count] FROM il.Item"));
    ASSERT_EQ(BE_SQLITE_ROW, select.Step());
    EXPECT_STREQ("a", select.GetValueText(0));
    EXPECT_EQ(1, select.GetValueInt64(1));
    EXPECT_NE(nullptr, m_ecdb.Schemas().GetClass("Inline", "INamed")->GetPropertyP("Nickname", false));
    EXPECT_NE(nullptr, m_ecdb.Schemas().GetClass("Inline", "Note")->GetPropertyP("Text", false));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaEditorCommandsTests, InlinePropertyErrorsNameTheElementAndPersistNothing) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorInlinePropertyErrors.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    ASSERT_TRUE(editor->CreateSchema({{"name", "InlineErrors"}, {"alias", "ie"}, {"version", "01.00.00"}}).IsOk());
    auto expectFailure = [&](Utf8CP properties, Utf8CP arg) {
        BeJsDocument args(R"json({"schema":"ie","name":"Item"})json");
        args["properties"].From(BeJsDocument(properties));
        auto result = editor->Validate("CreateEntityClass", args);
        ASSERT_FALSE(result.IsOk());
        ASSERT_FALSE(result.GetIssues().empty());
        EXPECT_STREQ(arg, result.GetIssues().front().arg.c_str()) << Describe(result);
    };
    expectFailure(R"json([{"name":"Ok","type":"int"},{"name":"Bad","type":"notAType"}])json", "properties[1].type");
    expectFailure(R"json([{"name":"Ok","type":"int"},{"name":"ECClassId","type":"int"}])json", "properties[1].name");
    expectFailure(R"json([{"name":"Ok","type":"int"},{"name":"ok","type":"int"}])json", "properties[1].name");
    expectFailure(R"json([{"name":"Ok","type":"int","class":"ie:Item"}])json", "properties[0].class");
    expectFailure(R"json({"name":"Ok"})json", "properties");
    expectFailure(R"json([{"name":"Parent","kind":"navigation","relationship":"ie:Missing","direction":"Backward"}])json", "properties[0].relationship");

    BeJsDocument bad(R"json({"schema":"ie","name":"Item","properties":[{"name":"Ok","type":"int"},{"name":"Bad","type":"notAType"}]})json");
    auto result = editor->Execute("CreateEntityClass", bad);
    ASSERT_FALSE(result.IsOk());
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetClass("InlineErrors", "Item"));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaEditorCommandsTests, InlineNavigationMatchesAddPropertyValidation) {
    // An inline navigation is prepared by the same code as AddProperty and must reach the same verdict.
    // A new class is never one of an existing relationship's constraint classes, so it could only override
    // the inherited navigation, which the editor rejects in both forms.
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorInlineNavigation.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    ASSERT_TRUE(editor->CreateSchema({{"name", "InlineNav"}, {"alias", "inav"}, {"version", "01.00.00"}}).IsOk());
    ASSERT_TRUE(editor->CreateEntityClass({{"schema", "inav"}, {"name", "Parent"}}).IsOk());
    ASSERT_TRUE(editor->CreateEntityClass({{"schema", "inav"}, {"name", "Child"}, {"mapStrategy", "TablePerHierarchy"}}).IsOk());
    auto result = editor->Execute("CreateRelationshipClass", BeJsDocument(R"json({
        "schema":"inav","name":"Owns","strength":"Referencing","direction":"Forward","mapAs":"foreignKey",
        "source":{"classes":["inav:Parent"],"multiplicity":"(0..1)","roleLabel":"parent","polymorphic":true},
        "target":{"classes":["inav:Child"],"multiplicity":"(0..*)","roleLabel":"children","polymorphic":true},
        "navProperty":{"class":"inav:Child","name":"Parent","direction":"Backward"}})json"));
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ASSERT_TRUE(editor->CreateEntityClass({{"schema", "inav"}, {"name", "ChildA"}, {"baseClass", "inav:Child"}}).IsOk());
    auto separate = editor->Validate("AddProperty", BeJsDocument(R"json({"class":"inav:ChildA","name":"Parent","kind":"navigation",
        "relationship":"inav:Owns","direction":"Backward"})json"));
    auto inlined = editor->Validate("CreateEntityClass", BeJsDocument(R"json({"schema":"inav","name":"ChildB","baseClass":"inav:Child",
        "properties":[{"name":"Label","type":"string"},{"name":"Parent","kind":"navigation","relationship":"inav:Owns","direction":"Backward"}]})json"));
    ASSERT_FALSE(separate.IsOk());
    ASSERT_FALSE(inlined.IsOk());
    EXPECT_EQ(separate.GetStatus(), inlined.GetStatus());
    ASSERT_FALSE(separate.GetIssues().empty());
    ASSERT_FALSE(inlined.GetIssues().empty());
    EXPECT_STREQ(separate.GetIssues().front().message.c_str(), inlined.GetIssues().front().message.c_str());
    EXPECT_STREQ(("properties[1]." + separate.GetIssues().front().arg).c_str(), inlined.GetIssues().front().arg.c_str()) << Describe(inlined);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaEditorCommandsTests, StandaloneCommandsKeepClassCachesCurrent) {
    // Created classes append their hierarchy rows, commands that write no base-class rows skip the
    // rebuild, and Map refreshes class-has-table rows of the classes it maps. Compare both caches
    // against a full rebuild after every command, before SaveChanges repopulates them anyway.
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorLeafHierarchy.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    auto expectCacheCurrent = [&](Utf8CP step) {
        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb,
            "WITH RECURSIVE h(ClassId,BaseClassId) AS (SELECT Id,Id FROM main.ec_Class UNION "
            "SELECT h.ClassId,b.BaseClassId FROM h JOIN main.ec_ClassHasBaseClasses b ON b.ClassId=h.BaseClassId), "
            "t(ClassId,TableId) AS (SELECT DISTINCT cm.ClassId,c.TableId FROM main.ec_PropertyMap pm "
            "JOIN main.ec_Column c ON c.Id=pm.ColumnId JOIN main.ec_ClassMap cm ON cm.ClassId=pm.ClassId "
            "WHERE c.ColumnKind & 2=0 AND cm.MapStrategy NOT IN(10,11)) "
            "SELECT (SELECT COUNT(*) FROM (SELECT ClassId,BaseClassId FROM h EXCEPT SELECT ClassId,BaseClassId FROM main.ec_cache_ClassHierarchy)),"
            "(SELECT COUNT(*) FROM (SELECT ClassId,BaseClassId FROM main.ec_cache_ClassHierarchy EXCEPT SELECT ClassId,BaseClassId FROM h)),"
            "(SELECT COUNT(*)-COUNT(DISTINCT ClassId||'-'||BaseClassId) FROM main.ec_cache_ClassHierarchy),"
            "(SELECT COUNT(*) FROM (SELECT ClassId,TableId FROM t EXCEPT SELECT ClassId,TableId FROM main.ec_cache_ClassHasTables)),"
            "(SELECT COUNT(*) FROM (SELECT ClassId,TableId FROM main.ec_cache_ClassHasTables EXCEPT SELECT ClassId,TableId FROM t))"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        EXPECT_EQ(0, stmt.GetValueInt(0)) << "Missing hierarchy rows after " << step;
        EXPECT_EQ(0, stmt.GetValueInt(1)) << "Stale hierarchy rows after " << step;
        EXPECT_EQ(0, stmt.GetValueInt(2)) << "Duplicate hierarchy rows after " << step;
        EXPECT_EQ(0, stmt.GetValueInt(3)) << "Missing class-has-table rows after " << step;
        EXPECT_EQ(0, stmt.GetValueInt(4)) << "Stale class-has-table rows after " << step;
    };
    auto run = [&](Utf8CP command, Utf8CP json) {
        auto result = editor->Execute(command, BeJsDocument(json));
        ASSERT_TRUE(result.IsOk()) << command << ": " << Describe(result);
        expectCacheCurrent(command);
    };
    ASSERT_TRUE(editor->CreateSchema({{"name", "LeafHierarchy"}, {"alias", "lh"}, {"version", "01.00.00"}}).IsOk());
    run("CreateEntityClass", R"json({"schema":"lh","name":"Base","mapStrategy":"TablePerHierarchy"})json");
    run("CreateEntityClass", R"json({"schema":"lh","name":"Mid","baseClass":"lh:Base"})json");
    run("CreateMixin", R"json({"schema":"lh","name":"Named","appliesTo":"lh:Base","properties":[{"name":"Name","type":"string"}]})json");
    run("CreateEntityClass", R"json({"schema":"lh","name":"Item","baseClass":"lh:Mid","mixins":"lh:Named"})json");
    run("CreateStructClass", R"json({"schema":"lh","name":"Payload"})json");
    run("CreateCustomAttributeClass", R"json({"schema":"lh","name":"Note","appliesTo":"Any"})json");
    run("CreateRelationshipClass", R"json({"schema":"lh","name":"Links","strength":"referencing","direction":"forward","mapAs":"linkTable",
        "source":{"classes":["lh:Base"],"multiplicity":"(0..*)","roleLabel":"links","polymorphic":true},
        "target":{"classes":["lh:Base"],"multiplicity":"(0..*)","roleLabel":"linked by","polymorphic":true}})json");
    run("CreateRelationshipClass", R"json({"schema":"lh","name":"ItemLinks","baseClass":"lh:Links","strength":"referencing","direction":"forward","mapAs":"linkTable",
        "source":{"classes":["lh:Item"],"multiplicity":"(0..*)","roleLabel":"links","polymorphic":true},
        "target":{"classes":["lh:Item"],"multiplicity":"(0..*)","roleLabel":"linked by","polymorphic":true}})json");
    // Mapped edits that write no base-class rows.
    run("AddProperty", R"json({"class":"lh:Base","name":"Code","type":"string"})json");
    run("SetCustomAttribute", R"json({"container":"lh:Item","caClass":"lh:Note","instance":{}})json");
    // Hierarchy edits on existing classes still rebuild.
    run("CreateMixin", R"json({"schema":"lh","name":"Tagged","appliesTo":"lh:Base"})json");
    run("CreateEntityClass", R"json({"schema":"lh","name":"Other","baseClass":"lh:Base"})json");
    run("AddMixin", R"json({"class":"lh:Other","mixin":"lh:Tagged"})json");
    run("DropEntityClass", R"json({"class":"lh:Other"})json");

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO lh.Item(Name) VALUES('leaf')"));
    for (auto query : {"SELECT COUNT(*) FROM lh.Base", "SELECT COUNT(*) FROM lh.Mid", "SELECT COUNT(*) FROM lh.Named WHERE Name='leaf'"}) {
        ECSqlStatement statement;
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, query)) << query;
        ASSERT_EQ(BE_SQLITE_ROW, statement.Step()) << query;
        EXPECT_EQ(1, statement.GetValueInt(0)) << query;
    }
    auto result = editor->SaveChanges();
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    expectCacheCurrent("SaveChanges");
}

TEST_F(SchemaEditorCommandsTests, GroupedRelationshipsReuseTheWorkingGraph) {
    // Relationships in a group continue the working graph published by the previous create instead of
    // reloading the schema; the persisted result and later validation must be unaffected.
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorGroupedRelationships.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->CreateSchema({{"name", "GroupedRel"}, {"alias", "grel"}, {"version", "01.00.00"}}).IsOk());
        BeJsDocument commands(R"json([
            {"command":"CreateEntityClass","args":{"schema":"grel","name":"Parent","properties":[{"name":"Code","type":"string"}]}},
            {"command":"CreateEntityClass","args":{"schema":"grel","name":"Child","mapStrategy":"TablePerHierarchy","properties":[{"name":"Name","type":"string"}]}},
            {"command":"CreateRelationshipClass","args":{"schema":"grel","name":"Owns","strength":"Embedding","direction":"Forward","mapAs":"foreignKey",
                "source":{"classes":["grel:Parent"],"multiplicity":"(0..1)","roleLabel":"parent","polymorphic":true},
                "target":{"classes":["grel:Child"],"multiplicity":"(0..*)","roleLabel":"children","polymorphic":true},
                "navProperty":{"class":"grel:Child","name":"Parent","direction":"Backward"},"fkOnDelete":"Cascade"}},
            {"command":"CreateEntityClass","args":{"schema":"grel","name":"Leaf","baseClass":"grel:Child"}},
            {"command":"CreateRelationshipClass","args":{"schema":"grel","name":"Links","strength":"Referencing","direction":"Forward","mapAs":"linkTable",
                "source":{"classes":["grel:Parent"],"multiplicity":"(0..*)","roleLabel":"links","polymorphic":true},
                "target":{"classes":["grel:Leaf"],"multiplicity":"(0..*)","roleLabel":"linked by","polymorphic":true}}},
            {"command":"AddProperty","args":{"class":"grel:Leaf","name":"Extra","type":"int"}}
        ])json");
        auto result = editor->ExecuteGroup(commands);
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        result = editor->SaveChanges();
        ASSERT_TRUE(result.IsOk()) << Describe(result);
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    auto leaf = m_ecdb.Schemas().GetClass("GroupedRel", "Leaf");
    ASSERT_NE(nullptr, leaf);
    for (auto name : {"Name", "Parent", "Extra"})
        EXPECT_NE(nullptr, leaf->GetPropertyP(name)) << name;
    ASSERT_NE(nullptr, leaf->GetPropertyP("Parent")->GetAsNavigationProperty());
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO grel.Parent(ECInstanceId,Code) VALUES(1,'p')"));
    {
        ECSqlStatement insert;
        ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(m_ecdb, "INSERT INTO grel.Leaf(ECInstanceId,Name,Parent,Extra) VALUES(2,'l',?,7)"));
        ASSERT_EQ(ECSqlStatus::Success, insert.BindNavigationValue(1, ECInstanceId(UINT64_C(1)), m_ecdb.Schemas().GetClassId("GroupedRel", "Owns")));
        ASSERT_EQ(BE_SQLITE_DONE, insert.Step());
    }
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO grel.Links(SourceECInstanceId,TargetECInstanceId) VALUES(1,2)"));
    {
        ECSqlStatement query;
        ASSERT_EQ(ECSqlStatus::Success, query.Prepare(m_ecdb, "SELECT c.Name,c.Extra FROM grel.Leaf c JOIN grel.Parent p ON p.ECInstanceId=c.Parent.Id JOIN grel.Links l ON l.TargetECInstanceId=c.ECInstanceId WHERE p.Code='p'"));
        ASSERT_EQ(BE_SQLITE_ROW, query.Step());
        EXPECT_STREQ("l", query.GetValueText(0));
        EXPECT_EQ(7, query.GetValueInt(1));
    }
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        // A navigation colliding with a property of the reused graph is still rejected and rolled back.
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        BeJsDocument commands(R"json([
            {"command":"CreateEntityClass","args":{"schema":"grel","name":"Other"}},
            {"command":"CreateRelationshipClass","args":{"schema":"grel","name":"Names","strength":"Referencing","direction":"Forward","mapAs":"foreignKey",
                "source":{"classes":["grel:Other"],"multiplicity":"(0..1)","roleLabel":"names","polymorphic":true},
                "target":{"classes":["grel:Child"],"multiplicity":"(0..*)","roleLabel":"named by","polymorphic":true},
                "navProperty":{"class":"grel:Child","name":"Name","direction":"Backward"}}}
        ])json");
        auto result = editor->ExecuteGroup(commands);
        ASSERT_FALSE(result.IsOk());
        BeJsDocument diagnostic;
        result.ToJson(diagnostic);
        EXPECT_STREQ("CreateRelationshipClass", diagnostic["failedCommand"].asCString());
        EXPECT_EQ(1, diagnostic["failedCommandIndex"].asUInt64());
        EXPECT_EQ(nullptr, m_ecdb.Schemas().GetClass("GroupedRel", "Other"));
        EXPECT_EQ(nullptr, m_ecdb.Schemas().GetClass("GroupedRel", "Names"));
    }
}


TEST_F(SchemaEditorCommandsTests, GroupedPropertyAlterationsReuseTheWorkingGraph) {
    // Metadata alterations in a group continue the working graph published by earlier property commands
    // instead of reloading the schema. Later children must see their changes, and failures roll back.
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaEditorGroupedAlterations.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        BeJsDocument setup(R"json([
            {"command":"CreateSchema","args":{"name":"GroupedAlter","alias":"galt","version":"01.00.00"}},
            {"command":"CreatePropertyCategory","args":{"schema":"galt","name":"Cat"}},
            {"command":"CreateEntityClass","args":{"schema":"galt","name":"Base","mapStrategy":"TablePerHierarchy",
                "properties":[{"name":"A","type":"string"},{"name":"B","type":"int"}]}},
            {"command":"CreateEntityClass","args":{"schema":"galt","name":"Derived","baseClass":"galt:Base"}}
        ])json");
        setup.ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst command) {
            auto result = editor->Execute(command);
            EXPECT_TRUE(result.IsOk()) << Describe(result);
            return false;
        });
        ASSERT_FALSE(HasFailure());
        BeJsDocument commands(R"json([
            {"command":"AddProperty","args":{"class":"galt:Base","name":"C","type":"string"}},
            {"command":"AlterProperty","args":{"class":"galt:Base","property":"A","priority":5,"label":"Alpha","minLength":2}},
            {"command":"AddProperty","args":{"class":"galt:Derived","name":"D","type":"double"}},
            {"command":"AlterProperty","args":{"class":"galt:Base","property":"B","category":"galt:Cat","readOnly":true}},
            {"command":"AlterProperty","args":{"class":"galt:Base","property":"C","description":"added earlier in the group"}},
            {"command":"AlterProperty","args":{"class":"galt:Base","property":"A","maxLength":10}}
        ])json");
        auto result = editor->ExecuteGroup(commands);
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        result = editor->SaveChanges();
        ASSERT_TRUE(result.IsOk()) << Describe(result);
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    auto base = m_ecdb.Schemas().GetClass("GroupedAlter", "Base");
    ASSERT_NE(nullptr, base);
    auto a = base->GetPropertyP("A", false);
    ASSERT_NE(nullptr, a);
    EXPECT_EQ(5, a->GetPriority());
    EXPECT_STREQ("Alpha", a->GetInvariantDisplayLabel().c_str());
    EXPECT_EQ(2u, a->GetMinimumLength());
    EXPECT_EQ(10u, a->GetMaximumLength());
    auto b = base->GetPropertyP("B", false);
    ASSERT_NE(nullptr, b);
    ASSERT_NE(nullptr, b->GetCategory());
    EXPECT_STREQ("Cat", b->GetCategory()->GetName().c_str());
    EXPECT_TRUE(b->GetIsReadOnly());
    ASSERT_NE(nullptr, base->GetPropertyP("C", false));
    EXPECT_STREQ("added earlier in the group", base->GetPropertyP("C", false)->GetInvariantDescription().c_str());
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO galt.Derived(ECInstanceId,A,B,C,D) VALUES(1,'abc',2,'c',1.5)"));
    {
        ECSqlStatement query;
        ASSERT_EQ(ECSqlStatus::Success, query.Prepare(m_ecdb, "SELECT A,C,D FROM galt.Derived"));
        ASSERT_EQ(BE_SQLITE_ROW, query.Step());
        EXPECT_STREQ("abc", query.GetValueText(0));
        EXPECT_STREQ("c", query.GetValueText(1));
        EXPECT_EQ(1.5, query.GetValueDouble(2));
    }
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        // The reused graph carries minLength=8 from the first child, so the second child's maxLength=4 is rejected
        // and the whole group, including the added property, rolls back.
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        BeJsDocument commands(R"json([
            {"command":"AlterProperty","args":{"class":"galt:Base","property":"C","minLength":8}},
            {"command":"AddProperty","args":{"class":"galt:Base","name":"E","type":"int"}},
            {"command":"AlterProperty","args":{"class":"galt:Base","property":"C","maxLength":4}}
        ])json");
        auto result = editor->ExecuteGroup(commands);
        ASSERT_FALSE(result.IsOk());
        BeJsDocument diagnostic;
        result.ToJson(diagnostic);
        EXPECT_STREQ("AlterProperty", diagnostic["failedCommand"].asCString());
        EXPECT_EQ(2, diagnostic["failedCommandIndex"].asUInt64());
        base = m_ecdb.Schemas().GetClass("GroupedAlter", "Base");
        ASSERT_NE(nullptr, base);
        EXPECT_EQ(nullptr, base->GetPropertyP("E", false));
        EXPECT_FALSE(base->GetPropertyP("C", false)->IsMinimumLengthDefined());
    }
}


TEST_F(SchemaEditorCommandsTests, KindOfQuantityCanBeAddedBeforeUpdatingDerivedOverride) {
    ASSERT_EQ(SUCCESS, SetupECDb("SchemaEditorNullKoqOverride.ecdb", SchemaItem(R"xml(
        <ECSchema schemaName="NullKoq" alias="nk" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <KindOfQuantity typeName="Length" persistenceUnit="u:M" relativeError="0.0001"/>
            <ECEntityClass typeName="Equipment">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="Width" typeName="double"/>
            </ECEntityClass>
            <ECEntityClass typeName="ReservedArea"><BaseClass>Equipment</BaseClass></ECEntityClass>
            <ECEntityClass typeName="ReservedAreaPar">
                <BaseClass>ReservedArea</BaseClass>
                <ECProperty propertyName="Width" typeName="double"/>
            </ECEntityClass>
        </ECSchema>)xml")));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO nk.ReservedAreaPar(Width) VALUES(12.5)"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        auto result = editor->AlterProperty({{"class", "nk:Equipment"}, {"property", "Width"}, {"koq", "nk:Length"}});
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        result = editor->SaveChanges();
        ASSERT_TRUE(result.IsOk()) << Describe(result);
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    auto derived = m_ecdb.Schemas().GetClass("NullKoq", "ReservedAreaPar");
    ASSERT_NE(nullptr, derived);
    auto width = derived->GetPropertyP("Width", false);
    ASSERT_NE(nullptr, width);
    EXPECT_FALSE(width->IsKindOfQuantityDefinedLocally());
    ASSERT_NE(nullptr, width->GetKindOfQuantity());
    EXPECT_STREQ("Length", width->GetKindOfQuantity()->GetName().c_str());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        auto result = editor->AlterProperty({{"class", "nk:ReservedAreaPar"}, {"property", "Width"}, {"koq", "nk:Length"}});
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        result = editor->SaveChanges();
        ASSERT_TRUE(result.IsOk()) << Describe(result);
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    derived = m_ecdb.Schemas().GetClass("NullKoq", "ReservedAreaPar");
    ASSERT_NE(nullptr, derived);
    width = derived->GetPropertyP("Width", false);
    ASSERT_NE(nullptr, width);
    EXPECT_TRUE(width->IsKindOfQuantityDefinedLocally());
    ASSERT_NE(nullptr, width->GetKindOfQuantity());
    EXPECT_STREQ("Length", width->GetKindOfQuantity()->GetName().c_str());
    ECSqlStatement query;
    ASSERT_EQ(ECSqlStatus::Success, query.Prepare(m_ecdb, "SELECT Width FROM nk.ReservedAreaPar"));
    ASSERT_EQ(BE_SQLITE_ROW, query.Step());
    EXPECT_DOUBLE_EQ(12.5, query.GetValueDouble(0));
    EXPECT_EQ(BE_SQLITE_DONE, query.Step());
}

TEST_F(SchemaEditorCommandsTests, InheritedOverridesPreserveCompoundMapsAndDataAcrossCommits) {
    ASSERT_EQ(SUCCESS, SetupECDb("SchemaEditorInheritedOverrides.ecdb", SchemaItem(R"xml(
        <ECSchema schemaName="Overrides" alias="ov" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECStructClass typeName="Payload">
                <ECProperty propertyName="X" typeName="double"/>
                <ECProperty propertyName="Y" typeName="string"/>
            </ECStructClass>
            <ECEntityClass typeName="Root">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>20</MaxSharedColumnsBeforeOverflow></ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="Code" typeName="string"/>
                <ECProperty propertyName="Location" typeName="point3d"/>
                <ECStructProperty propertyName="Content" typeName="Payload"/>
                <ECArrayProperty propertyName="Values" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="Middle"><BaseClass>Root</BaseClass></ECEntityClass>
            <ECEntityClass typeName="Leaf"><BaseClass>Middle</BaseClass></ECEntityClass>
            <ECEntityClass typeName="Grandchild"><BaseClass>Leaf</BaseClass></ECEntityClass>
            <ECEntityClass typeName="Sibling"><BaseClass>Root</BaseClass></ECEntityClass>
        </ECSchema>)xml")));
    int id = 0;
    for (auto className : {"Root", "Middle", "Leaf", "Grandchild", "Sibling"}) {
        ECSqlStatement insert;
        auto sql = Utf8PrintfString("INSERT INTO ov.%s(ECInstanceId,Code,Location,Content.X,Content.Y,[Values]) VALUES(%d,'code',?,2.5,'payload',?)", className, ++id);
        ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(m_ecdb, sql.c_str()));
        ASSERT_EQ(ECSqlStatus::Success, insert.BindPoint3d(1, DPoint3d::From(1.25, 2.5, 3.75)));
        ASSERT_EQ(ECSqlStatus::Success, insert.GetBinder(2).AddArrayElement().BindText("array", IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(BE_SQLITE_DONE, insert.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    for (auto className : {"ov:Middle", "ov:Leaf"}) {
        for (auto propertyName : {"Code", "Location", "Content", "Values"}) {
            SCOPED_TRACE(testing::Message() << className << "." << propertyName);
            auto editor = SchemaEditor::Begin(m_ecdb);
            ASSERT_NE(nullptr, editor);
            SchemaCommandArgs args;
            if (strcmp(propertyName, "Content") == 0)
                args = {{"class", className}, {"name", propertyName}, {"kind", "struct"}, {"struct", "ov:Payload"}};
            else
                args = {{"class", className}, {"name", propertyName}, {"type", strcmp(propertyName, "Location") == 0 ? "point3d" : "string"},
                        {"kind", strcmp(propertyName, "Values") == 0 ? "primitiveArray" : "primitive"}};
            auto result = editor->AddProperty(args);
            ASSERT_TRUE(result.IsOk()) << Describe(result);
            result = editor->SaveChanges();
            ASSERT_TRUE(result.IsOk()) << Describe(result);
            editor.reset();
            ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

            Statement duplicate;
            ASSERT_EQ(BE_SQLITE_OK, duplicate.Prepare(m_ecdb,
                "SELECT m.ClassId,pp.AccessString FROM ec_PropertyMap m JOIN ec_PropertyPath pp ON pp.Id=m.PropertyPathId "
                "JOIN ec_Class c ON c.Id=m.ClassId JOIN ec_Schema s ON s.Id=c.SchemaId "
                "WHERE s.Name='Overrides' AND pp.AccessString NOT IN ('ECInstanceId','ECClassId') "
                "GROUP BY m.ClassId,pp.AccessString HAVING COUNT(*)<>1"));
            EXPECT_EQ(BE_SQLITE_DONE, duplicate.Step());

            ECSqlStatement query;
            ASSERT_EQ(ECSqlStatus::Success, query.Prepare(m_ecdb,
                "SELECT ECInstanceId,Code,Location,Content.X,Content.Y,[Values] FROM ov.Root ORDER BY ECInstanceId"));
            for (int expectedId = 1; expectedId <= 5; ++expectedId) {
                ASSERT_EQ(BE_SQLITE_ROW, query.Step());
                EXPECT_EQ(expectedId, query.GetValueInt(0));
                EXPECT_STREQ("code", query.GetValueText(1));
                EXPECT_EQ(DPoint3d::From(1.25, 2.5, 3.75), query.GetValuePoint3d(2));
                EXPECT_DOUBLE_EQ(2.5, query.GetValueDouble(3));
                EXPECT_STREQ("payload", query.GetValueText(4));
                EXPECT_EQ(1, query.GetValue(5).GetArrayLength());
                for (auto const& element : query.GetValue(5).GetArrayIterable())
                    EXPECT_STREQ("array", element.GetText());
            }
            EXPECT_EQ(BE_SQLITE_DONE, query.Step());
        }
    }
}

TEST_F(SchemaEditorCommandsTests, GroupedKindOfQuantityAlterationsReuseTheWorkingGraph) {
    // KoQ alterations in a group continue the published working graph like other metadata alterations,
    // while incompatible overrides are still rejected and roll back the group.
    ASSERT_EQ(SUCCESS, SetupECDb("SchemaEditorGroupedKoq.ecdb", SchemaItem(R"xml(
        <ECSchema schemaName="GroupedKoq" alias="gkoq" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
            <KindOfQuantity typeName="Len" persistenceUnit="u:M" relativeError="0.1" presentationUnits="f:DefaultRealU(2)[u:M]"/>
            <KindOfQuantity typeName="Len2" persistenceUnit="u:M" relativeError="0.01" presentationUnits="f:DefaultRealU(4)[u:M]"/>
            <KindOfQuantity typeName="Mass" persistenceUnit="u:KG" relativeError="0.1"/>
            <ECEntityClass typeName="Base">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="L" typeName="double" kindOfQuantity="Len"/>
                <ECProperty propertyName="P" typeName="double"/>
                <ECProperty propertyName="Q" typeName="double"/>
            </ECEntityClass>
            <ECEntityClass typeName="Derived">
                <BaseClass>Base</BaseClass>
                <ECProperty propertyName="Q" typeName="double" kindOfQuantity="Len"/>
            </ECEntityClass>
        </ECSchema>)xml")));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        BeJsDocument commands(R"json([
            {"command":"AddProperty","args":{"class":"gkoq:Base","name":"C","type":"double","koq":"gkoq:Len"}},
            {"command":"AlterProperty","args":{"class":"gkoq:Base","property":"L","koq":"gkoq:Len2"}},
            {"command":"AlterProperty","args":{"class":"gkoq:Base","property":"P","koq":"gkoq:Mass","label":"Weight"}},
            {"command":"AlterProperty","args":{"class":"gkoq:Base","property":"C","koq":"gkoq:Len2"}},
            {"command":"AlterProperty","args":{"class":"gkoq:Base","property":"L","description":"after the KoQ change"}}
        ])json");
        auto result = editor->ExecuteGroup(commands);
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        result = editor->SaveChanges();
        ASSERT_TRUE(result.IsOk()) << Describe(result);
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    auto base = m_ecdb.Schemas().GetClass("GroupedKoq", "Base");
    ASSERT_NE(nullptr, base);
    auto koqName = [&](Utf8CP name) {
        auto property = base->GetPropertyP(name, false);
        return property == nullptr || property->GetKindOfQuantity() == nullptr ? Utf8String() : property->GetKindOfQuantity()->GetName();
    };
    EXPECT_STREQ("Len2", koqName("L").c_str());
    EXPECT_STREQ("Mass", koqName("P").c_str());
    EXPECT_STREQ("Len2", koqName("C").c_str());
    EXPECT_STREQ("Weight", base->GetPropertyP("P", false)->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("after the KoQ change", base->GetPropertyP("L", false)->GetInvariantDescription().c_str());
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO gkoq.Derived(ECInstanceId,L,P,Q,C) VALUES(1,1.5,2.5,3.5,4.5)"));
    {
        ECSqlStatement query;
        ASSERT_EQ(ECSqlStatus::Success, query.Prepare(m_ecdb, "SELECT L,C FROM gkoq.Base"));
        ASSERT_EQ(BE_SQLITE_ROW, query.Step());
        EXPECT_EQ(1.5, query.GetValueDouble(0));
        EXPECT_EQ(4.5, query.GetValueDouble(1));
    }
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        // Derived overrides Q with a metre KoQ, so giving the base Q a kilogram KoQ is rejected and the
        // earlier children roll back.
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        BeJsDocument commands(R"json([
            {"command":"AlterProperty","args":{"class":"gkoq:Base","property":"L","label":"Length"}},
            {"command":"AddProperty","args":{"class":"gkoq:Base","name":"E","type":"int"}},
            {"command":"AlterProperty","args":{"class":"gkoq:Base","property":"Q","koq":"gkoq:Mass"}}
        ])json");
        auto result = editor->ExecuteGroup(commands);
        ASSERT_FALSE(result.IsOk());
        BeJsDocument diagnostic;
        result.ToJson(diagnostic);
        EXPECT_STREQ("AlterProperty", diagnostic["failedCommand"].asCString());
        EXPECT_EQ(2, diagnostic["failedCommandIndex"].asUInt64());
        base = m_ecdb.Schemas().GetClass("GroupedKoq", "Base");
        ASSERT_NE(nullptr, base);
        EXPECT_EQ(nullptr, base->GetPropertyP("E", false));
        EXPECT_FALSE(base->GetPropertyP("L", false)->GetIsDisplayLabelDefined());
        EXPECT_EQ(nullptr, base->GetPropertyP("Q", false)->GetKindOfQuantity());
    }
}

END_ECDBUNITTESTS_NAMESPACE
