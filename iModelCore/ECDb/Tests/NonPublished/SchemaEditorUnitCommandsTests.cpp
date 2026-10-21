/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <ECDb/SchemaEditor.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

struct SchemaEditorUnitCommandsTests : ECDbTestFixture {
    static Utf8CP SeedXml() {
        return R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="Seed" alias="seed" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                <KindOfQuantity typeName="SeedLength" persistenceUnit="u:M" relativeError="0.1" presentationUnits="f:DefaultRealU(2)[u:M]"/>
            </ECSchema>)xml";
    }

    static Utf8String Describe(SchemaCommandResult const& result) {
        BeJsDocument json;
        result.ToJson(json);
        return json.Stringify();
    }

    SchemaCommandResult Run(SchemaEditor& editor, Utf8CP command, Utf8CP json) {
        BeJsDocument args(json);
        return editor.Execute(command, args);
    }

    // A failed Execute aborts the session, so rejections are checked with Validate.
    SchemaCommandResult Check(SchemaEditor& editor, Utf8CP command, Utf8CP json) {
        BeJsDocument args(json);
        return editor.Validate(command, args);
    }

    static Utf8String Arg(SchemaCommandResult const& result) { return result.GetIssues().empty() ? Utf8String() : result.GetIssues().front().arg; }

    static void ExpectFailure(SchemaCommandResult const& result, SchemaCommandResult::Status status, Utf8CP arg) {
        EXPECT_EQ(status, result.GetStatus()) << Describe(result);
        EXPECT_STREQ(arg, Arg(result).c_str()) << Describe(result);
    }

    Utf8String Query(Utf8CP sql) {
        Statement stmt;
        EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, sql)) << sql;
        Utf8String out;
        while (BE_SQLITE_ROW == stmt.Step()) {
            for (int i = 0; i < stmt.GetColumnCount(); ++i) {
                out.append(i == 0 ? "" : "|");
                out.append(stmt.IsColumnNull(i) ? "<null>" : stmt.GetValueText(i));
            }
            out.append("\n");
        }
        return out;
    }

    // Persisted rows of schema Q with foreign keys translated to qualified names.
    Utf8String Dump() {
        Utf8String dump;
        dump.append(Query("SELECT r.Name FROM ec_SchemaReference sr JOIN ec_Schema s ON s.Id=sr.SchemaId JOIN ec_Schema r ON r.Id=sr.ReferencedSchemaId WHERE s.Name='Q' ORDER BY r.Name"));
        dump.append(Query("SELECT x.Name,x.DisplayLabel,x.Description FROM ec_UnitSystem x JOIN ec_Schema s ON s.Id=x.SchemaId WHERE s.Name='Q' ORDER BY x.Name"));
        dump.append(Query("SELECT x.Name,x.DisplayLabel,x.Description,x.Definition FROM ec_Phenomenon x JOIN ec_Schema s ON s.Id=x.SchemaId WHERE s.Name='Q' ORDER BY x.Name"));
        dump.append(Query("SELECT x.Name,x.DisplayLabel,x.Description,"
                          "(SELECT ps.Name||':'||p.Name FROM ec_Phenomenon p JOIN ec_Schema ps ON ps.Id=p.SchemaId WHERE p.Id=x.PhenomenonId),"
                          "(SELECT us.Name||':'||u.Name FROM ec_UnitSystem u JOIN ec_Schema us ON us.Id=u.SchemaId WHERE u.Id=x.UnitSystemId),"
                          "x.Definition,x.Numerator,x.Denominator,x.Offset,x.IsConstant,"
                          "(SELECT is_.Name||':'||i.Name FROM ec_Unit i JOIN ec_Schema is_ ON is_.Id=i.SchemaId WHERE i.Id=x.InvertingUnitId) "
                          "FROM ec_Unit x JOIN ec_Schema s ON s.Id=x.SchemaId WHERE s.Name='Q' ORDER BY x.Name"));
        dump.append(Query("SELECT x.Name,x.DisplayLabel,x.Description,x.NumericSpec,x.CompositeSpec FROM ec_Format x JOIN ec_Schema s ON s.Id=x.SchemaId WHERE s.Name='Q' ORDER BY x.Name"));
        dump.append(Query("SELECT f.Name,c.Label,us.Name||':'||u.Name,c.Ordinal FROM ec_FormatCompositeUnit c JOIN ec_Format f ON f.Id=c.FormatId "
                          "JOIN ec_Schema s ON s.Id=f.SchemaId JOIN ec_Unit u ON u.Id=c.UnitId JOIN ec_Schema us ON us.Id=u.SchemaId WHERE s.Name='Q' ORDER BY f.Name,c.Ordinal"));
        dump.append(Query("SELECT x.Name,x.DisplayLabel,x.Description,x.RelativeError,x.PersistenceUnit,x.PresentationUnits FROM ec_KindOfQuantity x JOIN ec_Schema s ON s.Id=x.SchemaId WHERE s.Name='Q' ORDER BY x.Name"));
        dump.append(Query("SELECT x.Name,x.DisplayLabel,x.Description,x.Priority FROM ec_PropertyCategory x JOIN ec_Schema s ON s.Id=x.SchemaId WHERE s.Name='Q' ORDER BY x.Name"));
        return dump;
    }

    Utf8String SchemaJson() {
        auto schema = m_ecdb.Schemas().GetSchema("Q", true);
        EXPECT_NE(nullptr, schema);
        if (schema == nullptr)
            return "";
        BeJsDocument json;
        EXPECT_TRUE(schema->WriteToJsonValue(json));
        return json.Stringify(StringifyFormat::Indented);
    }

    // Creates the schema Q with the same items as NativeXml() through editor commands.
    void CreateItems(SchemaEditor& editor) {
        auto result = editor.CreateSchema({{"name", "Q"}, {"alias", "q"}, {"version", "01.00.00"}});
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        for (auto const& command : std::vector<std::pair<Utf8CP, Utf8CP>>{
                 {"CreateUnitSystem", R"({"schema":"q","name":"MySystem","label":"My System","description":"A unit system"})"},
                 {"CreateUnitSystem", R"({"schema":"Q","name":"OtherSystem"})"},
                 {"CreatePhenomenon", R"({"schema":"q","name":"MyLength","definition":"LENGTH","label":"My Length"})"},
                 {"CreateUnit", R"({"schema":"q","name":"MyMeter","phenomenon":"q:MyLength","unitSystem":"q:MySystem","definition":"u:M","numerator":2.0,"label":"My Meter"})"},
                 {"CreateUnit", R"({"schema":"q","name":"MyScaled","phenomenon":"Q.MyLength","unitSystem":"Q.MySystem","definition":"MyMeter","numerator":"1.5","denominator":3,"offset":2,"description":"Scaled"})"},
                 {"CreateUnit", R"({"schema":"q","name":"MyFoot","phenomenon":"Units.LENGTH","unitSystem":"Units.USCUSTOM","definition":"u:FT"})"},
                 {"CreateUnit", R"({"schema":"q","name":"MyPi","isConstant":true,"phenomenon":"u:ANGLE","definition":"ONE","numerator":3.5,"denominator":2,"label":"Pi"})"},
                 {"CreateUnit", R"({"schema":"q","name":"MyInverted","invertsUnit":"u:VERTICAL_PER_HORIZONTAL","unitSystem":"u:INTERNATIONAL","label":"inv"})"},
                 {"CreateFormat", R"({"schema":"q","name":"MyFmt","label":"My Format","type":"decimal","precision":4,"formatTraits":["keepSingleZero","showUnitLabel"],"decimalSeparator":",","thousandSeparator":".","uomSeparator":" ","roundFactor":0.5,"minWidth":3,"showSignOption":"signAlways"})"},
                 {"CreateFormat", R"({"schema":"q","name":"MyComposite","type":"fractional","precision":8,"formatTraits":"keepSingleZero|showUnitLabel","composite":{"spacer":"-","includeZero":false,"units":[{"name":"Units.FT","label":"'"},{"name":"Units.IN","label":"\""}]}})"},
                 {"CreateFormat", R"({"schema":"q","name":"MyStation","type":"station","precision":2,"stationOffsetSize":2,"stationSeparator":"+","description":"Station"})"},
                 {"CreateFormat", R"({"schema":"q","name":"MyScientific","type":"scientific","scientificType":"normalized","precision":3,"composite":{"units":["q:MyMeter"]}})"},
                 {"CreateKindOfQuantity", R"({"schema":"q","name":"MyKoq","persistenceUnit":"Units.M","relativeError":0.001,"presentationUnits":["Formats.DefaultRealU(4)[Units.M]","Q.MyFmt[Units.FT|feet]","q:MyComposite"],"label":"Len"})"},
                 {"CreateKindOfQuantity", R"({"schema":"q","name":"OwnKoq","persistenceUnit":"q:MyMeter","relativeError":"0.1","presentationUnits":"q:MyStation[q:MyMeter];q:MyScientific","description":"Own"})"},
                 {"CreatePropertyCategory", R"({"schema":"q","name":"MyCat","priority":5,"label":"Cat","description":"Category"})"},
                 {"CreatePropertyCategory", R"({"schema":"q","name":"OtherCat"})"}}) {
            result = Run(editor, command.first, command.second);
            ASSERT_TRUE(result.IsOk()) << command.first << " " << command.second << ": " << Describe(result);
        }
    }

    static Utf8CP NativeXml() {
        return R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="Q" alias="q" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                <UnitSystem typeName="MySystem" displayLabel="My System" description="A unit system"/>
                <UnitSystem typeName="OtherSystem"/>
                <Phenomenon typeName="MyLength" definition="LENGTH" displayLabel="My Length"/>
                <Unit typeName="MyMeter" phenomenon="MyLength" unitSystem="MySystem" definition="u:M" numerator="2.0" displayLabel="My Meter"/>
                <Unit typeName="MyScaled" phenomenon="MyLength" unitSystem="MySystem" definition="MyMeter" numerator="1.5" denominator="3" offset="2" description="Scaled"/>
                <Unit typeName="MyFoot" phenomenon="u:LENGTH" unitSystem="u:USCUSTOM" definition="u:FT"/>
                <Constant typeName="MyPi" phenomenon="u:ANGLE" definition="ONE" numerator="3.5" denominator="2" displayLabel="Pi"/>
                <InvertedUnit typeName="MyInverted" invertsUnit="u:VERTICAL_PER_HORIZONTAL" unitSystem="u:INTERNATIONAL" displayLabel="inv"/>
                <Format typeName="MyFmt" displayLabel="My Format" type="decimal" precision="4" formatTraits="keepSingleZero|showUnitLabel" decimalSeparator="," thousandSeparator="." uomSeparator=" " roundFactor="0.5" minWidth="3" showSignOption="signAlways"/>
                <Format typeName="MyComposite" type="fractional" precision="8" formatTraits="keepSingleZero|showUnitLabel">
                    <Composite spacer="-" includeZero="false">
                        <Unit label="'">u:FT</Unit>
                        <Unit label="&quot;">u:IN</Unit>
                    </Composite>
                </Format>
                <Format typeName="MyStation" type="station" precision="2" stationOffsetSize="2" stationSeparator="+" description="Station"/>
                <Format typeName="MyScientific" type="scientific" scientificType="normalized" precision="3">
                    <Composite>
                        <Unit>MyMeter</Unit>
                    </Composite>
                </Format>
                <KindOfQuantity typeName="MyKoq" persistenceUnit="u:M" relativeError="0.001" presentationUnits="f:DefaultRealU(4)[u:M];MyFmt[u:FT|feet];MyComposite" displayLabel="Len"/>
                <KindOfQuantity typeName="OwnKoq" persistenceUnit="MyMeter" relativeError="0.1" presentationUnits="MyStation[MyMeter];MyScientific" description="Own"/>
                <PropertyCategory typeName="MyCat" priority="5" displayLabel="Cat" description="Category"/>
                <PropertyCategory typeName="OtherCat"/>
            </ECSchema>)xml";
    }

    void SetupEditorDb(Utf8CP fileName) {
        ASSERT_EQ(SUCCESS, SetupECDb(fileName, SchemaItem(SeedXml())));
        ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        CreateItems(*editor);
        auto result = editor->SaveChanges();
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    }
};

TEST_F(SchemaEditorUnitCommandsTests, CreatedItemsMatchNativeImport) {
    ASSERT_EQ(SUCCESS, SetupECDb("SchemaEditorUnitsNative.ecdb", SchemaItem(NativeXml())));
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    auto nativeDump = Dump();
    auto nativeJson = SchemaJson();
    ASSERT_FALSE(nativeJson.empty());
    for (auto expected : {"MySystem|My System|A unit system", "MyInverted|inv|<null>|Units:SLOPE|Units:INTERNATIONAL|<null>", "MyComposite|'|Units:FT|0",
                          "MyKoq|Len|<null>|0.001|u:M|", "MyCat|Cat|Category|5"})
        EXPECT_NE(Utf8String::npos, nativeDump.find(expected)) << expected << "\n"
                                                               << nativeDump;

    SetupEditorDb("SchemaEditorUnitsEditor.ecdb");
    EXPECT_STREQ(nativeDump.c_str(), Dump().c_str());
    EXPECT_STREQ(nativeJson.c_str(), SchemaJson().c_str());

    auto unit = m_ecdb.Schemas().GetUnit("Q", "MyInverted");
    ASSERT_NE(nullptr, unit);
    EXPECT_TRUE(unit->IsInvertedUnit());
    auto constant = m_ecdb.Schemas().GetUnit("Q", "MyPi");
    ASSERT_NE(nullptr, constant);
    EXPECT_TRUE(constant->IsConstant());
    auto format = m_ecdb.Schemas().GetFormat("Q", "MyComposite");
    ASSERT_NE(nullptr, format);
    ASSERT_TRUE(format->HasComposite());
    EXPECT_EQ(2, format->GetCompositeSpec()->GetUnitCount());
    auto koq = m_ecdb.Schemas().GetKindOfQuantity("Q", "MyKoq");
    ASSERT_NE(nullptr, koq);
    EXPECT_EQ(3, koq->GetPresentationFormats().size());
}

TEST_F(SchemaEditorUnitCommandsTests, RejectsInvalidCreates) {
    ASSERT_EQ(SUCCESS, SetupECDb("SchemaEditorUnitsInvalidCreate.ecdb", SchemaItem(SeedXml())));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    ASSERT_TRUE(editor->CreateSchema({{"name", "Q"}, {"alias", "q"}, {"version", "01.00.00"}}).IsOk());
    ExpectFailure(Check(*editor, "CreateUnitSystem", R"({"schema":"Units","name":"Nope"})"), SchemaCommandResult::Status::ERROR_VALIDATION, "schema");
    ExpectFailure(Check(*editor, "CreateUnitSystem", R"({"schema":"q","name":"Bad Name"})"), SchemaCommandResult::Status::ERROR_INVALID_ARGS, "name");
    ExpectFailure(Check(*editor, "CreateUnitSystem", R"({"schema":"q","name":"S","bogus":1})"), SchemaCommandResult::Status::ERROR_INVALID_ARGS, "bogus");
    ExpectFailure(Check(*editor, "CreatePhenomenon", R"({"schema":"q","name":"P"})"), SchemaCommandResult::Status::ERROR_INVALID_ARGS, "definition");
    ExpectFailure(Check(*editor, "CreateUnit", R"({"schema":"q","name":"U","phenomenon":"u:NOPE","unitSystem":"u:SI","definition":"M"})"), SchemaCommandResult::Status::ERROR_NOT_FOUND, "phenomenon");
    ExpectFailure(Check(*editor, "CreateUnit", R"({"schema":"q","name":"U","phenomenon":"u:LENGTH","unitSystem":"u:SI","definition":"M","numerator":0})"), SchemaCommandResult::Status::ERROR_VALIDATION, "numerator");
    ExpectFailure(Check(*editor, "CreateUnit", R"({"schema":"q","name":"U","isConstant":true,"phenomenon":"u:LENGTH","definition":"M"})"), SchemaCommandResult::Status::ERROR_INVALID_ARGS, "numerator");
    ExpectFailure(Check(*editor, "CreateUnit", R"({"schema":"q","name":"U","isConstant":true,"phenomenon":"u:LENGTH","unitSystem":"u:SI","definition":"M","numerator":1})"), SchemaCommandResult::Status::ERROR_INVALID_ARGS, "unitSystem");
    ExpectFailure(Check(*editor, "CreateUnit", R"({"schema":"q","name":"U","invertsUnit":"u:M","unitSystem":"u:SI","definition":"M"})"), SchemaCommandResult::Status::ERROR_INVALID_ARGS, "definition");
    ExpectFailure(Check(*editor, "CreateFormat", R"({"schema":"q","name":"F","type":"decimal"})"), SchemaCommandResult::Status::ERROR_INVALID_ARGS, "precision");
    ExpectFailure(Check(*editor, "CreateFormat", R"({"schema":"q","name":"F","type":"bogus","precision":2})"), SchemaCommandResult::Status::ERROR_INVALID_ARGS, "type");
    ExpectFailure(Check(*editor, "CreateFormat", R"({"schema":"q","name":"F","type":"decimal","precision":13})"), SchemaCommandResult::Status::ERROR_INVALID_ARGS, "precision");
    ExpectFailure(Check(*editor, "CreateFormat", R"({"schema":"q","name":"F","type":"fractional","precision":3})"), SchemaCommandResult::Status::ERROR_INVALID_ARGS, "precision");
    ExpectFailure(Check(*editor, "CreateFormat", R"({"schema":"q","name":"F","type":"station","precision":2})"), SchemaCommandResult::Status::ERROR_INVALID_ARGS, "stationOffsetSize");
    ExpectFailure(Check(*editor, "CreateFormat", R"({"schema":"q","name":"F","type":"decimal","precision":2,"decimalSeparator":",,"})"), SchemaCommandResult::Status::ERROR_INVALID_ARGS, "decimalSeparator");
    ExpectFailure(Check(*editor, "CreateFormat", R"({"schema":"q","name":"F","type":"decimal","precision":2,"formatTraits":"bogusTrait"})"), SchemaCommandResult::Status::ERROR_INVALID_ARGS, "formatTraits");
    ExpectFailure(Check(*editor, "CreateFormat", R"({"schema":"q","name":"F","type":"decimal","precision":2,"composite":{"units":["u:M","u:CM","u:MM","u:UM","u:NM"]}})"), SchemaCommandResult::Status::ERROR_INVALID_ARGS, "composite.units");
    ExpectFailure(Check(*editor, "CreateKindOfQuantity", R"({"schema":"q","name":"K","persistenceUnit":"u:M"})"), SchemaCommandResult::Status::ERROR_INVALID_ARGS, "relativeError");
    ExpectFailure(Check(*editor, "CreateKindOfQuantity", R"({"schema":"q","name":"K","persistenceUnit":"u:M","relativeError":0.1,"presentationUnits":["f:Nope"]})"), SchemaCommandResult::Status::ERROR_NOT_FOUND, "presentationUnits[0]");
    ExpectFailure(Check(*editor, "CreateKindOfQuantity", R"({"schema":"q","name":"K","persistenceUnit":"u:M","relativeError":0.1,"presentationUnits":["f:DefaultRealU[u:KG]"]})"), SchemaCommandResult::Status::ERROR_VALIDATION, "presentationUnits[0]");
    ExpectFailure(Check(*editor, "CreatePropertyCategory", R"({"schema":"q","name":"C","priority":-1})"), SchemaCommandResult::Status::ERROR_INVALID_ARGS, "priority");
    auto result = editor->SaveChanges();
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    EXPECT_STREQ("", Query("SELECT r.Name FROM ec_SchemaReference sr JOIN ec_Schema s ON s.Id=sr.SchemaId JOIN ec_Schema r ON r.Id=sr.ReferencedSchemaId WHERE s.Name='Q' ORDER BY r.Name").c_str())
        << "Failed commands must not leave references behind";
}

TEST_F(SchemaEditorUnitCommandsTests, AllowedAlterations) {
    SetupEditorDb("SchemaEditorUnitsAlter.ecdb");
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    for (auto const& command : std::vector<std::pair<Utf8CP, Utf8CP>>{
             {"AlterUnitSystem", R"({"unitSystem":"q:MySystem","label":"","description":"Changed"})"},
             {"AlterPhenomenon", R"({"phenomenon":"q:MyLength","definition":"LENGTH","label":"Length 2"})"},
             {"AlterUnit", R"({"unit":"q:MyFoot","numerator":1,"denominator":"1.0","offset":0,"label":"Foot"})"},
             {"AlterUnit", R"({"unit":"q:MyMeter","numerator":2,"definition":"u:M","phenomenon":"q:MyLength","unitSystem":"q:MySystem","isConstant":false,"description":"Meter"})"},
             {"AlterFormat", R"({"format":"q:MyFmt","type":"decimal","precision":6,"formatTraits":["keepSingleZero"],"label":"Six"})"},
             {"AlterFormat", R"({"format":"q:MyComposite","composite":{"spacer":" ","units":[{"name":"u:FT","label":"ft"},{"name":"u:IN"}]}})"},
             {"AlterKindOfQuantity", R"({"koq":"q:MyKoq","relativeError":0.5,"persistenceUnit":"u:M","presentationUnits":["f:DefaultRealU(2)[u:M]"]})"},
             {"AlterKindOfQuantity", R"({"koq":"q:OwnKoq","presentationUnits":[],"label":"Own"})"},
             {"AlterPropertyCategory", R"({"category":"q:MyCat","priority":9,"description":""})"}}) {
        auto result = Run(*editor, command.first, command.second);
        ASSERT_TRUE(result.IsOk()) << command.first << " " << command.second << ": " << Describe(result);
    }
    auto result = editor->SaveChanges();
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    EXPECT_STREQ("<null>|Changed\n", Query("SELECT DisplayLabel,Description FROM ec_UnitSystem WHERE Name='MySystem'").c_str());
    EXPECT_STREQ("Length 2|LENGTH\n", Query("SELECT DisplayLabel,Definition FROM ec_Phenomenon WHERE Name='MyLength'").c_str());
    EXPECT_STREQ("Foot|1.0|1.0|0.0\n", Query("SELECT DisplayLabel,Numerator,Denominator,Offset FROM ec_Unit WHERE Name='MyFoot'").c_str());
    EXPECT_STREQ("Meter|2.0\n", Query("SELECT Description,Numerator FROM ec_Unit WHERE Name='MyMeter'").c_str());
    auto format = m_ecdb.Schemas().GetFormat("Q", "MyFmt");
    ASSERT_NE(nullptr, format);
    EXPECT_STREQ("Six", format->GetInvariantDisplayLabel().c_str());
    EXPECT_EQ(Formatting::DecimalPrecision::Precision6, format->GetNumericSpec()->GetDecimalPrecision());
    auto composite = m_ecdb.Schemas().GetFormat("Q", "MyComposite");
    ASSERT_NE(nullptr, composite);
    EXPECT_STREQ(" ", composite->GetCompositeSpec()->GetSpacer().c_str());
    EXPECT_STREQ("ft", composite->GetCompositeSpec()->GetMajorLabel().c_str());
    EXPECT_FALSE(composite->GetCompositeSpec()->HasMiddleLabel());
    auto koq = m_ecdb.Schemas().GetKindOfQuantity("Q", "MyKoq");
    ASSERT_NE(nullptr, koq);
    EXPECT_EQ(0.5, koq->GetRelativeError());
    ASSERT_EQ(1, koq->GetPresentationFormats().size());
    EXPECT_STREQ("<null>|Own\n", Query("SELECT PresentationUnits,DisplayLabel FROM ec_KindOfQuantity WHERE Name='OwnKoq'").c_str());
    EXPECT_STREQ("9|<null>\n", Query("SELECT Priority,Description FROM ec_PropertyCategory WHERE Name='MyCat'").c_str());
}

TEST_F(SchemaEditorUnitCommandsTests, RejectedAlterations) {
    SetupEditorDb("SchemaEditorUnitsRejectAlter.ecdb");
    auto before = Dump();
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    auto const validation = SchemaCommandResult::Status::ERROR_VALIDATION;
    ExpectFailure(Check(*editor, "AlterUnitSystem", R"({"unitSystem":"q:MySystem","name":"Renamed"})"), SchemaCommandResult::Status::ERROR_INVALID_ARGS, "name");
    ExpectFailure(Check(*editor, "AlterUnitSystem", R"({"unitSystem":"q:MySystem"})"), SchemaCommandResult::Status::ERROR_INVALID_ARGS, "unitSystem");
    ExpectFailure(Check(*editor, "AlterUnitSystem", R"({"unitSystem":"u:SI","label":"SI"})"), validation, "schema");
    ExpectFailure(Check(*editor, "AlterUnitSystem", R"({"unitSystem":"q:Missing","label":"x"})"), SchemaCommandResult::Status::ERROR_NOT_FOUND, "unitSystem");
    ExpectFailure(Check(*editor, "AlterPhenomenon", R"({"phenomenon":"q:MyLength","definition":"AREA"})"), validation, "definition");
    ExpectFailure(Check(*editor, "AlterUnit", R"({"unit":"q:MyMeter","numerator":3})"), validation, "numerator");
    ExpectFailure(Check(*editor, "AlterUnit", R"({"unit":"q:MyFoot","numerator":2})"), validation, "numerator");
    ExpectFailure(Check(*editor, "AlterUnit", R"({"unit":"q:MyMeter","definition":"u:CM"})"), validation, "definition");
    ExpectFailure(Check(*editor, "AlterUnit", R"({"unit":"q:MyMeter","unitSystem":"q:OtherSystem"})"), validation, "unitSystem");
    ExpectFailure(Check(*editor, "AlterUnit", R"({"unit":"q:MyMeter","phenomenon":"u:LENGTH"})"), validation, "phenomenon");
    ExpectFailure(Check(*editor, "AlterUnit", R"({"unit":"q:MyMeter","isConstant":true})"), validation, "isConstant");
    ExpectFailure(Check(*editor, "AlterUnit", R"({"unit":"q:MyInverted","invertsUnit":"u:M"})"), validation, "invertsUnit");
    ExpectFailure(Check(*editor, "AlterFormat", R"({"format":"q:MyComposite","composite":{"units":["u:FT","u:M"]}})"), validation, "composite.units");
    ExpectFailure(Check(*editor, "AlterFormat", R"({"format":"q:MyComposite","composite":{"units":["u:FT"]}})"), validation, "composite.units");
    ExpectFailure(Check(*editor, "AlterFormat", R"({"format":"q:MyFmt","composite":{"units":["u:FT"]}})"), validation, "composite");
    ExpectFailure(Check(*editor, "AlterFormat", R"({"format":"q:MyFmt","precision":2})"), SchemaCommandResult::Status::ERROR_INVALID_ARGS, "type");
    ExpectFailure(Check(*editor, "AlterFormat", R"({"format":"q:MyFmt","type":"decimal","precision":4,"formatTraits":["keepSingleZero","showUnitLabel"],"decimalSeparator":",","thousandSeparator":".","uomSeparator":" ","roundFactor":0.5,"minWidth":3,"showSignOption":"signAlways"})"),
                  SchemaCommandResult::Status::ERROR_INVALID_ARGS, "format");
    ExpectFailure(Check(*editor, "AlterKindOfQuantity", R"({"koq":"q:MyKoq","persistenceUnit":"u:CM"})"), validation, "persistenceUnit");
    ExpectFailure(Check(*editor, "AlterKindOfQuantity", R"({"koq":"q:MyKoq","relativeError":null})"), validation, "relativeError");
    ExpectFailure(Check(*editor, "AlterKindOfQuantity", R"({"koq":"q:MyKoq","presentationUnits":["f:DefaultRealU[u:KG]"]})"), validation, "presentationUnits[0]");
    ExpectFailure(Check(*editor, "AlterPropertyCategory", R"({"category":"q:MyCat","priority":5})"), SchemaCommandResult::Status::ERROR_INVALID_ARGS, "category");
    auto result = editor->SaveChanges();
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    EXPECT_STREQ(before.c_str(), Dump().c_str());
}

TEST_F(SchemaEditorUnitCommandsTests, DropKindOfQuantityAndPropertyCategory) {
    ASSERT_EQ(SUCCESS, SetupECDb("SchemaEditorUnitsDrop.ecdb", SchemaItem(SeedXml())));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    CreateItems(*editor);
    ASSERT_TRUE(editor->CreateEntityClass({{"schema", "q"}, {"name", "Item"}}).IsOk());
    auto result = editor->AddProperty({{"class", "q:Item"}, {"name", "Length"}, {"kind", "primitive"}, {"type", "double"}, {"koq", "q:MyKoq"}, {"category", "q:MyCat"}});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    result = editor->Validate("DropKindOfQuantity", SchemaCommandArgs{{"koq", "q:MyKoq"}});
    ExpectFailure(result, SchemaCommandResult::Status::ERROR_VALIDATION, "koq");
    EXPECT_NE(Utf8String::npos, result.GetIssues().front().message.find("Q:Item.Length")) << Describe(result);
    result = editor->Validate("DropPropertyCategory", SchemaCommandArgs{{"category", "q:MyCat"}});
    ExpectFailure(result, SchemaCommandResult::Status::ERROR_VALIDATION, "category");
    ExpectFailure(editor->Validate("DropKindOfQuantity", SchemaCommandArgs{{"koq", "seed:SeedLength"}, {"bogus", "1"}}), SchemaCommandResult::Status::ERROR_INVALID_ARGS, "bogus");
    ExpectFailure(editor->Validate("DropKindOfQuantity", SchemaCommandArgs{{"koq", "q:Missing"}}), SchemaCommandResult::Status::ERROR_NOT_FOUND, "koq");
    // A schema created in this session may lose items without a version bump.
    result = editor->DropKindOfQuantity({{"koq", "q:OwnKoq"}});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    result = editor->SaveChanges();
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetKindOfQuantity("Q", "OwnKoq"));

    editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    // SchemaWriter::DeleteKindOfQuantity requires a major version change; DeletePropertyCategory does not.
    ExpectFailure(editor->Validate("DropKindOfQuantity", SchemaCommandArgs{{"koq", "seed:SeedLength"}}), SchemaCommandResult::Status::ERROR_VALIDATION, "schema");
    result = editor->DropPropertyCategory({{"category", "q:OtherCat"}});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ASSERT_TRUE(editor->AlterSchema({{"schema", "seed"}, {"version", "02.00.00"}}).IsOk());
    result = editor->DropKindOfQuantity({{"koq", "seed:SeedLength"}});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    result = editor->SaveChanges();
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetKindOfQuantity("Seed", "SeedLength"));
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetPropertyCategory("Q", "OtherCat"));
    EXPECT_NE(nullptr, m_ecdb.Schemas().GetKindOfQuantity("Q", "MyKoq"));
}

// Consecutive category creations reuse the session's working graph instead of re-reading the schema.
TEST_F(SchemaEditorUnitCommandsTests, ConsecutiveCategoryCreatesReuseWorkingGraph) {
    ASSERT_EQ(SUCCESS, SetupECDb("SchemaEditorCategoryReuse.ecdb", SchemaItem(SeedXml())));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    ASSERT_TRUE(editor->CreateSchema({{"name", "Q"}, {"alias", "q"}, {"version", "01.00.00"}}).IsOk());
    for (Utf8CP name : {"A", "B", "C"}) {
        auto result = Run(*editor, "CreatePropertyCategory", Utf8PrintfString(R"({"schema":"q","name":"%s","priority":1})", name).c_str());
        ASSERT_TRUE(result.IsOk()) << name << ": " << Describe(result);
    }
    ASSERT_TRUE(editor->CreateEntityClass({{"schema", "q"}, {"name", "Item"}}).IsOk());
    auto result = editor->AddProperty({{"class", "q:Item"}, {"name", "P"}, {"kind", "primitive"}, {"type", "int"}, {"category", "q:C"}});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    // The class creation published the graph that already holds the categories; a duplicate is rejected.
    result = Run(*editor, "CreatePropertyCategory", R"({"schema":"q","name":"B"})");
    EXPECT_EQ(SchemaCommandResult::Status::ERROR_VALIDATION, result.GetStatus()) << Describe(result);
    EXPECT_STREQ("name", Arg(result).c_str()) << Describe(result);
    EXPECT_STREQ("", Query("SELECT Name FROM ec_Schema WHERE Name='Q'").c_str()) << "A failed command abandons the session";

    editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    ASSERT_TRUE(editor->CreateSchema({{"name", "Q"}, {"alias", "q"}, {"version", "01.00.00"}}).IsOk());
    ASSERT_TRUE(Run(*editor, "CreatePropertyCategory", R"({"schema":"q","name":"A","priority":1})").IsOk());
    ASSERT_TRUE(Run(*editor, "CreatePropertyCategory", R"({"schema":"q","name":"B","priority":2})").IsOk());
    ASSERT_TRUE(editor->CreateEntityClass({{"schema", "q"}, {"name", "Item"}}).IsOk());
    ASSERT_TRUE(editor->AddProperty({{"class", "q:Item"}, {"name", "P"}, {"kind", "primitive"}, {"type", "int"}, {"category", "q:B"}}).IsOk());
    result = editor->SaveChanges();
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    EXPECT_STREQ("A|1\nB|2\n", Query("SELECT c.Name,c.Priority FROM ec_PropertyCategory c JOIN ec_Schema s ON s.Id=c.SchemaId WHERE s.Name='Q' ORDER BY 1").c_str());
    auto property = m_ecdb.Schemas().GetClass("Q", "Item")->GetPropertyP("P");
    ASSERT_NE(nullptr, property);
    ASSERT_NE(nullptr, property->GetCategory());
    EXPECT_STREQ("B", property->GetCategory()->GetName().c_str());
}

// Unit, format, KoQ and category commands do not touch class mappings, so they skip the mapper
// (observable through class views, which the mapper drops) while class edits still map.
TEST_F(SchemaEditorUnitCommandsTests, ItemCommandsSkipClassMapping) {
    ASSERT_EQ(SUCCESS, SetupECDb("SchemaEditorItemsSkipMapping.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="Seed" alias="seed" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
            <ECEntityClass typeName="Item"><ECProperty propertyName="Code" typeName="string"/></ECEntityClass>
        </ECSchema>)xml")));
    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().CreateClassViewsInDb());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    Utf8CP viewQuery = "SELECT name FROM sqlite_master WHERE type='view' AND name='seed.Item'";
    ASSERT_STREQ("seed.Item\n", Query(viewQuery).c_str());

    auto editor = SchemaEditor::Begin(m_ecdb);
    ASSERT_NE(nullptr, editor);
    auto result = Run(*editor, "CreatePropertyCategory", R"({"schema":"seed","name":"Cat","priority":1})");
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    result = Run(*editor, "CreateKindOfQuantity", R"({"schema":"seed","name":"Len","persistenceUnit":"u:M","relativeError":0.1,"presentationUnits":["f:DefaultRealU(2)[u:M]"]})");
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    result = Run(*editor, "CreateEnumeration", R"({"schema":"seed","name":"Color","backingType":"int","enumerators":[{"name":"Red","value":1}]})");
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    EXPECT_STREQ("seed.Item\n", Query(viewQuery).c_str()) << "Item commands must not run the class mapper";

    result = editor->AddProperty({{"class", "seed:Item"}, {"name", "Length"}, {"kind", "primitive"}, {"type", "double"}, {"koq", "seed:Len"}, {"category", "seed:Cat"}});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    EXPECT_STREQ("", Query(viewQuery).c_str()) << "Class edits still run the mapper";
    result = editor->SaveChanges();
    ASSERT_TRUE(result.IsOk()) << Describe(result);

    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    auto property = m_ecdb.Schemas().GetClass("Seed", "Item")->GetPropertyP("Length");
    ASSERT_NE(nullptr, property);
    ASSERT_NE(nullptr, property->GetCategory());
    EXPECT_STREQ("Cat", property->GetCategory()->GetName().c_str());
    ASSERT_NE(nullptr, property->GetKindOfQuantity());
    EXPECT_STREQ("Len", property->GetKindOfQuantity()->GetName().c_str());
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Code, Length FROM seed.Item"));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaEditorUnitCommandsTests, InlinePropertiesResolveKindOfQuantityAndCategory) {
    // Inline properties resolve items from schemas the working graph already references without the schema
    // reader; other schemas are loaded and referenced. Both must persist the same IDs and references.
    ASSERT_EQ(SUCCESS, SetupECDb("SchemaEditorInlineKoq.ecdb", SchemaItem(SeedXml())));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        ASSERT_TRUE(editor->CreateSchema({{"name", "Q"}, {"alias", "q"}, {"version", "01.00.00"}}).IsOk());
        ASSERT_TRUE(Run(*editor, "CreatePropertyCategory", R"({"schema":"q","name":"MyCat"})").IsOk());
        ExpectFailure(Check(*editor, "CreateEntityClass", R"({"schema":"q","name":"Bad","properties":[{"name":"L","type":"double","koq":"seed:Nope"}]})"),
                      SchemaCommandResult::Status::ERROR_NOT_FOUND, "properties[0].koq");
        // Seed is not referenced yet: loaded through the reader and added as a reference.
        auto result = Run(*editor, "CreateEntityClass", R"({"schema":"q","name":"First","properties":[
            {"name":"Length","type":"double","koq":"seed:SeedLength","category":"q:MyCat"}]})");
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        // Seed and Q are now in the working graph; schema name and alias forms resolve without the reader.
        result = Run(*editor, "CreateEntityClass", R"({"schema":"q","name":"Second","properties":[
            {"name":"Width","type":"double","koq":"Seed:SeedLength","category":"Q:MyCat"},
            {"name":"Depth","type":"double","koq":"seed:SeedLength"}]})");
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        ExpectFailure(Check(*editor, "CreateEntityClass", R"({"schema":"q","name":"Bad","properties":[{"name":"L","type":"double","category":"q:Nope"}]})"),
                      SchemaCommandResult::Status::ERROR_NOT_FOUND, "properties[0].category");
        ASSERT_TRUE(editor->SaveChanges().IsOk());
    }
    EXPECT_STREQ("Depth|Seed:SeedLength|<null>\nLength|Seed:SeedLength|Q:MyCat\nWidth|Seed:SeedLength|Q:MyCat\n",
                 Query("SELECT p.Name,(SELECT ks.Name||':'||k.Name FROM ec_KindOfQuantity k JOIN ec_Schema ks ON ks.Id=k.SchemaId WHERE k.Id=p.KindOfQuantityId),"
                       "(SELECT cs.Name||':'||c.Name FROM ec_PropertyCategory c JOIN ec_Schema cs ON cs.Id=c.SchemaId WHERE c.Id=p.CategoryId)"
                       " FROM ec_Property p JOIN ec_Class x ON x.Id=p.ClassId JOIN ec_Schema s ON s.Id=x.SchemaId WHERE s.Name='Q' ORDER BY p.Name").c_str());
    EXPECT_STREQ("Seed\n", Query("SELECT r.Name FROM ec_SchemaReference sr JOIN ec_Schema s ON s.Id=sr.SchemaId JOIN ec_Schema r ON r.Id=sr.ReferencedSchemaId"
                                 " WHERE s.Name='Q' ORDER BY r.Name").c_str());
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    auto width = m_ecdb.Schemas().GetClass("Q", "Second")->GetPropertyP("Width", false);
    ASSERT_NE(nullptr, width);
    ASSERT_NE(nullptr, width->GetKindOfQuantity());
    EXPECT_STREQ("SeedLength", width->GetKindOfQuantity()->GetName().c_str());
}

END_ECDBUNITTESTS_NAMESPACE
