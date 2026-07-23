/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
#include <ECDb/ConcurrentQueryManager.h>

BEGIN_ECDBUNITTESTS_NAMESPACE

struct IntegrityCheckerFixture : ECDbTestFixture {

    DbResult OpenCopyOfDataFile(Utf8CP testFileName, Utf8CP asFileName , Db::OpenMode openMode = Db::OpenMode::Readonly) {
        auto getDataPath = []() {
            BeFileName docRoot;
            BeTest::GetHost().GetDocumentsRoot(docRoot);
            docRoot.AppendToPath(L"ECDb");
            return docRoot;
        };

        const auto bimPath = getDataPath().AppendToPath(WString(testFileName, true).c_str());
        if (!bimPath.DoesPathExist()) {
            EXPECT_TRUE(bimPath.DoesPathExist()) << "Test file" << bimPath.c_str() << "does not exist.";
            return BE_SQLITE_ERROR;
        }

        const auto clonedFilePath = BuildECDbPath(asFileName);
        if (clonedFilePath.DoesPathExist()) {
            clonedFilePath.BeDeleteFile();
        }
        BeFileName::BeCopyFile(bimPath.c_str(), clonedFilePath.c_str());
        if (m_ecdb.IsDbOpen()) {
            m_ecdb.CloseDb();
        }

        return m_ecdb.OpenBeSQLiteDb(clonedFilePath, Db::OpenParams(openMode));
    }
    Utf8String ParseJSON(Utf8CP json) const {
        BeJsDocument doc;
        doc.Parse(json);
        return doc.Stringify(StringifyFormat::Indented);
    }
};
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IntegrityCheckerFixture, experimental_check) {
    SetupECDb("test.ecdb");
    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus(BE_SQLITE_ERROR), stmt.Prepare(m_ecdb, "PRAGMA integrity_check"));
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IntegrityCheckerFixture, check_all) {
    auto runCheck = [&](ECDbCR db) -> Utf8String {
        BeJsDocument out;
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "PRAGMA integrity_check"));
        EXPECT_EQ(4, stmt.GetColumnCount());
        EXPECT_STRCASEEQ("sno", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("check", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("result", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("elapsed_sec", stmt.GetColumnInfo(3).GetProperty()->GetName().c_str());
        EXPECT_EQ(PRIMITIVETYPE_Integer, stmt.GetColumnInfo(0).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(1).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_Boolean, stmt.GetColumnInfo(2).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(3).GetDataType().GetPrimitiveType());

        out.SetEmptyArray();
        while (stmt.Step() == BE_SQLITE_ROW) {
            auto row = out.appendObject();
            row["sno"] = stmt.GetValueInt(0);
            row["check"] = stmt.GetValueText(1);
            row["result"] = stmt.GetValueText(2);
        }
        return out.Stringify(StringifyFormat::Indented);
    };
    auto executeTest = [&]() {
        auto expectedJSON = R"json(
            [
                {
                    "sno": 1,
                    "check": "check_data_columns",
                    "result": "true"
                },
                {
                    "sno": 2,
                    "check": "check_ec_profile",
                    "result": "true"
                },
                {
                    "sno": 3,
                    "check": "check_nav_class_ids",
                    "result": "true"
                },
                {
                    "sno": 4,
                    "check": "check_nav_ids",
                    "result": "true"
                },
                {
                    "sno": 5,
                    "check": "check_linktable_fk_class_ids",
                    "result": "true"
                },
                {
                    "sno": 6,
                    "check": "check_linktable_fk_ids",
                    "result": "true"
                },
                {
                    "sno": 7,
                    "check": "check_class_ids",
                    "result": "true"
                },
                {
                    "sno": 8,
                    "check": "check_data_schema",
                    "result": "true"
                },
                {
                    "sno": 9,
                    "check": "check_schema_load",
                    "result": "true"
                }
            ]
        )json";
        ASSERT_STREQ(ParseJSON(expectedJSON).c_str(), runCheck(m_ecdb).c_str()) << "Failed for " << m_ecdb.GetDbFileName();
        m_ecdb.AbandonChanges();
    };

    ASSERT_EQ(BE_SQLITE_OK, OpenCopyOfDataFile("test.bim", "check_all.bim", Db::OpenMode::ReadWrite));
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));
    executeTest();

}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IntegrityCheckerFixture, check_nav_class_ids) {
    auto runCheck = [&](ECDbCR db) -> Utf8String {
        BeJsDocument out;
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "PRAGMA integrity_check(check_nav_class_ids)"));
        EXPECT_EQ(6, stmt.GetColumnCount());
        EXPECT_STRCASEEQ("sno", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("id", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("class", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("property", stmt.GetColumnInfo(3).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("nav_id", stmt.GetColumnInfo(4).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("nav_classId", stmt.GetColumnInfo(5).GetProperty()->GetName().c_str());
        EXPECT_EQ(PRIMITIVETYPE_Integer, stmt.GetColumnInfo(0).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(1).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(2).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(3).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(4).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(5).GetDataType().GetPrimitiveType());

        out.SetEmptyArray();
        while (stmt.Step() == BE_SQLITE_ROW) {
            auto row = out.appendObject();
            row["sno"] = stmt.GetValueInt(0);
            row["id"] = stmt.GetValueText(1);
            row["class"] = stmt.GetValueText(2);
            row["property"] = stmt.GetValueText(3);
            row["nav_id"] = stmt.GetValueText(4);
            row["nav_classId"] = stmt.GetValueText(5);
        }
        return out.Stringify(StringifyFormat::Indented);
    };

    auto executeTest = [&]() {
        ASSERT_STREQ(ParseJSON("[]").c_str(), runCheck(m_ecdb).c_str()) << "expect this to pass";
        // RelECClassId does not exist in the iModel
        m_ecdb.ExecuteSql("UPDATE bis_GeometricElement3d SET TypeDefinitionId = 0x17, TypeDefinitionRelECClassId=0xffff WHERE ElementId = 0x3a");
        // RelECClassId does exist in the iModel but it does not represent valid ClassId for the relationship
        m_ecdb.ExecuteSql("UPDATE bis_GeometricElement3d SET TypeDefinitionId = 0x17, TypeDefinitionRelECClassId=0x43 WHERE ElementId = 0x3b");
        m_ecdb.ExecuteSql("UPDATE bis_GeometricElement3d SET TypeDefinitionId = 0x17, TypeDefinitionRelECClassId=0x43 WHERE ElementId = 0x39");
        auto expectedJSON = R"json(
            [
                {
                    "sno": 1,
                    "id": "0x39",
                    "class": "BisCore:GeometricElement3d",
                    "property": "TypeDefinition",
                    "nav_id": "0x17",
                    "nav_classId": "0x43"
                },
                {
                    "sno": 2,
                    "id": "0x3a",
                    "class": "BisCore:GeometricElement3d",
                    "property": "TypeDefinition",
                    "nav_id": "0x17",
                    "nav_classId": "0xffff"
                },
                {
                    "sno": 3,
                    "id": "0x3b",
                    "class": "BisCore:GeometricElement3d",
                    "property": "TypeDefinition",
                    "nav_id": "0x17",
                    "nav_classId": "0x43"
                }
            ]
        )json";
        ASSERT_STREQ(ParseJSON(expectedJSON).c_str(), runCheck(m_ecdb).c_str()) << "Failed for " << m_ecdb.GetDbFileName();
        m_ecdb.AbandonChanges();
    };

    ASSERT_EQ(BE_SQLITE_OK, OpenCopyOfDataFile("test.bim", "test.bim", Db::OpenMode::ReadWrite));
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));

    executeTest();
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IntegrityCheckerFixture, check_nav_ids) {
    auto runCheck = [&](ECDbCR db) -> Utf8String {
        BeJsDocument out;
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "PRAGMA integrity_check(check_nav_ids)"));
        EXPECT_EQ(6, stmt.GetColumnCount());
        EXPECT_STRCASEEQ("sno", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("id", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("class", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("property", stmt.GetColumnInfo(3).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("nav_id", stmt.GetColumnInfo(4).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("primary_class", stmt.GetColumnInfo(5).GetProperty()->GetName().c_str());
        EXPECT_EQ(PRIMITIVETYPE_Integer, stmt.GetColumnInfo(0).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(1).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(2).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(3).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(4).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(5).GetDataType().GetPrimitiveType());

        out.SetEmptyArray();
        while (stmt.Step() == BE_SQLITE_ROW) {
            auto row = out.appendObject();
            row["sno"] = stmt.GetValueInt(0);
            row["id"] = stmt.GetValueText(1);
            row["class"] = stmt.GetValueText(2);
            row["property"] = stmt.GetValueText(3);
            row["nav_id"] = stmt.GetValueText(4);
            row["primary_class"] = stmt.GetValueText(5);
        }
        return out.Stringify(StringifyFormat::Indented);
    };

    auto executeTest = [&]() {
        ASSERT_STREQ(ParseJSON("[]").c_str(), runCheck(m_ecdb).c_str()) << "expect this to pass";
        //m_ecdb.ExecuteSql("UPDATE bis_GeometricElement3d SET TypeDefinitionId = 47    ,  TypeDefinitionRelECClassId = 0xffff WHERE ElementId = 57");
        m_ecdb.ExecuteSql("UPDATE bis_GeometricElement3d SET TypeDefinitionId = 0x17 WHERE ElementId = 0x3a");
        auto expectedJSON = R"json(
            [
                {
                    "sno": 1,
                    "id": "0x3a",
                    "class": "BisCore:GeometricElement3d",
                    "property": "TypeDefinition",
                    "nav_id": "0x17",
                    "primary_class": "BisCore:TypeDefinitionElement"
                }
            ]
        )json";
        ASSERT_STREQ(ParseJSON(expectedJSON).c_str(), runCheck(m_ecdb).c_str()) << "Failed for " << m_ecdb.GetDbFileName();
        m_ecdb.AbandonChanges();
    };

    ASSERT_EQ(BE_SQLITE_OK, OpenCopyOfDataFile("test.bim", "test.bim", Db::OpenMode::ReadWrite));
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));
    executeTest();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IntegrityCheckerFixture, check_linktable_fk_ids) {
    auto runCheck = [&](ECDbCR db) -> Utf8String {
        BeJsDocument out;
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "PRAGMA integrity_check(check_linktable_fk_ids)"));
        EXPECT_EQ(6, stmt.GetColumnCount());
        EXPECT_STRCASEEQ("sno", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("id", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("relationship", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("property", stmt.GetColumnInfo(3).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("key_id", stmt.GetColumnInfo(4).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("primary_class", stmt.GetColumnInfo(5).GetProperty()->GetName().c_str());
        EXPECT_EQ(PRIMITIVETYPE_Integer, stmt.GetColumnInfo(0).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(1).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(2).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(3).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(4).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(5).GetDataType().GetPrimitiveType());

        out.SetEmptyArray();
        while (stmt.Step() == BE_SQLITE_ROW) {
            auto row = out.appendObject();
            row["sno"] = stmt.GetValueInt(0);
            row["id"] = stmt.GetValueText(1);
            row["relationship"] = stmt.GetValueText(2);
            row["property"] = stmt.GetValueText(3);
            row["key_id"] = stmt.GetValueText(4);
            row["primary_class"] = stmt.GetValueText(5);
        }
        return out.Stringify(StringifyFormat::Indented);
    };

    auto executeTest = [&]() {
        ASSERT_STREQ(ParseJSON("[]").c_str(), runCheck(m_ecdb).c_str()) << "expect this to pass";
        m_ecdb.ExecuteSql("UPDATE bis_ElementRefersToElements     SET TargetId = 0xffff WHERE Id = 4");
        m_ecdb.ExecuteSql("UPDATE bis_ElementRefersToElements     SET TargetId = 0xffff WHERE Id = 5");
        m_ecdb.ExecuteSql("UPDATE bis_ModelSelectorRefersToModels SET SourceId = 0xffff WHERE Id = 8");
        m_ecdb.ExecuteSql("UPDATE bis_ModelSelectorRefersToModels SET TargetId = 0xffff WHERE Id = 9");
        auto expectedJSON = R"json(
            [
                {
                    "sno": 1,
                    "id": "0x4",
                    "relationship": "BisCore:ElementRefersToElements",
                    "property": "TargetECInstanceId",
                    "key_id": "0xffff",
                    "primary_class": "BisCore:Element"
                },
                {
                    "sno": 2,
                    "id": "0x5",
                    "relationship": "BisCore:ElementRefersToElements",
                    "property": "TargetECInstanceId",
                    "key_id": "0xffff",
                    "primary_class": "BisCore:Element"
                },
                {
                    "sno": 3,
                    "id": "0x8",
                    "relationship": "BisCore:ModelSelectorRefersToModels",
                    "property": "SourceECInstanceId",
                    "key_id": "0xffff",
                    "primary_class": "BisCore:ModelSelector"
                },
                {
                    "sno": 4,
                    "id": "0x9",
                    "relationship": "BisCore:ModelSelectorRefersToModels",
                    "property": "TargetECInstanceId",
                    "key_id": "0xffff",
                    "primary_class": "BisCore:Model"
                }
            ]
        )json";
        ASSERT_STREQ(ParseJSON(expectedJSON).c_str(), runCheck(m_ecdb).c_str()) << "Failed for " << m_ecdb.GetDbFileName();
        m_ecdb.AbandonChanges();
    };

    ASSERT_EQ(BE_SQLITE_OK, OpenCopyOfDataFile("test.bim", "test.bim", Db::OpenMode::ReadWrite));
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));
    executeTest();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IntegrityCheckerFixture, check_linktable_fk_class_ids) {
    auto runCheck = [&](ECDbCR db) -> Utf8String {
        BeJsDocument out;
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "PRAGMA integrity_check(check_linktable_fk_class_ids)"));
        EXPECT_EQ(6, stmt.GetColumnCount());
        EXPECT_STRCASEEQ("sno", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("id", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("relationship", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("property", stmt.GetColumnInfo(3).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("key_id", stmt.GetColumnInfo(4).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("key_classId", stmt.GetColumnInfo(5).GetProperty()->GetName().c_str());
        EXPECT_EQ(PRIMITIVETYPE_Integer, stmt.GetColumnInfo(0).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(1).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(2).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(3).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(4).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(5).GetDataType().GetPrimitiveType());

        out.SetEmptyArray();
        while (stmt.Step() == BE_SQLITE_ROW) {
            auto row = out.appendObject();
            row["sno"] = stmt.GetValueInt(0);
            row["id"] = stmt.GetValueText(1);
            row["relationship"] = stmt.GetValueText(2);
            row["property"] = stmt.GetValueText(3);
            row["key_id"] = stmt.GetValueText(4);
            row["key_classId"] = stmt.GetValueText(4);
        }
        return out.Stringify(StringifyFormat::Indented);
    };

    auto executeTest = [&]() {
        ASSERT_STREQ(ParseJSON("[]").c_str(), runCheck(m_ecdb).c_str()) << "expect this to pass";
    };
    // Bis never have this case
    ASSERT_EQ(BE_SQLITE_OK, OpenCopyOfDataFile("test.bim", "test.bim", Db::OpenMode::ReadWrite));
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));
    executeTest();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IntegrityCheckerFixture, check_class_ids) {
    auto runCheck = [&](ECDbCR db) -> Utf8String {
        BeJsDocument out;
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "PRAGMA integrity_check(check_class_ids)"));
        EXPECT_EQ(5, stmt.GetColumnCount());
        EXPECT_STRCASEEQ("sno", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("class", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("id", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("class_id", stmt.GetColumnInfo(3).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("type", stmt.GetColumnInfo(4).GetProperty()->GetName().c_str());
        EXPECT_EQ(PRIMITIVETYPE_Integer, stmt.GetColumnInfo(0).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(1).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(2).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(3).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(4).GetDataType().GetPrimitiveType());

        out.SetEmptyArray();
        while (stmt.Step() == BE_SQLITE_ROW) {
            auto row = out.appendObject();
            row["sno"] = stmt.GetValueInt(0);
            row["class"] = stmt.GetValueText(1);
            row["id"] = stmt.GetValueText(2);
            row["class_id"] = stmt.GetValueText(3);
            row["type"] = stmt.GetValueText(4);
        }
        return out.Stringify(StringifyFormat::Indented);
    };

    auto executeTest = [&]() {
        ASSERT_STREQ(ParseJSON("[]").c_str(), runCheck(m_ecdb).c_str()) << "expect this to pass";
        m_ecdb.ExecuteSql("UPDATE bis_Element                 SET ECClassId = 0x3e8 WHERE        Id = 0x20");
        m_ecdb.ExecuteSql("UPDATE bis_GeometricElement3d      SET ECClassId = 0x3e8 WHERE ElementId = 0x3B");
        m_ecdb.ExecuteSql("UPDATE bis_ElementRefersToElements SET ECClassId = 0x3e8 WHERE        Id = 0x0e");
        m_ecdb.ExecuteSql("UPDATE bis_Model                   SET ECClassId = 0x3e8 WHERE        Id = 0x24");
        auto expectedJSON = R"json(
            [
                {
                    "sno": 1,
                    "class": "BisCore:Element",
                    "id": "0x20",
                    "class_id": "0x3e8",
                    "type": "primary"
                },
                {
                    "sno": 2,
                    "class": "BisCore:Model",
                    "id": "0x24",
                    "class_id": "0x3e8",
                    "type": "primary"
                },
                {
                    "sno": 3,
                    "class": "BisCore:ElementRefersToElements",
                    "id": "0xe",
                    "class_id": "0x3e8",
                    "type": "primary"
                },
                {
                    "sno": 4,
                    "class": "BisCore:GeometricElement3d",
                    "id": "0x3b",
                    "class_id": "0x3e8",
                    "type": "joined"
                }
            ]
        )json";
        ASSERT_STREQ(ParseJSON(expectedJSON).c_str(), runCheck(m_ecdb).c_str()) << "Failed for " << m_ecdb.GetDbFileName();
        m_ecdb.AbandonChanges();
    };

    ASSERT_EQ(BE_SQLITE_OK, OpenCopyOfDataFile("test.bim", "test.bim", Db::OpenMode::ReadWrite));
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));
    executeTest();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IntegrityCheckerFixture, check_missing_child_rows) {
    auto runCheck = [&](ECDbCR db) -> Utf8String {
        BeJsDocument out;
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "PRAGMA integrity_check(check_missing_child_rows)"));
        EXPECT_EQ(5, stmt.GetColumnCount());
        EXPECT_STRCASEEQ("sno", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("class", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("id", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("class_id", stmt.GetColumnInfo(3).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("MissingRowInTables", stmt.GetColumnInfo(4).GetProperty()->GetName().c_str());
        EXPECT_EQ(PRIMITIVETYPE_Integer, stmt.GetColumnInfo(0).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(1).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(2).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(3).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(4).GetDataType().GetPrimitiveType());

        out.SetEmptyArray();
        while (stmt.Step() == BE_SQLITE_ROW) {
            auto row = out.appendObject();
            row["sno"] = stmt.GetValueInt(0);
            row["class"] = stmt.GetValueText(1);
            row["id"] = stmt.GetValueText(2);
            row["class_id"] = stmt.GetValueText(3);
            row["MissingRowInTables"] = stmt.GetValueText(4);
        }
        return out.Stringify(StringifyFormat::Indented);
    };

    auto executeTest = [&]() {
        ASSERT_STREQ(ParseJSON("[]").c_str(), runCheck(m_ecdb).c_str()) << "expect this to pass";
        ASSERT_EQ(DbResult::BE_SQLITE_OK, m_ecdb.ExecuteSql("DELETE FROM bis_InformationReferenceElement WHERE ElementId = 0x1D"));
        ASSERT_EQ(DbResult::BE_SQLITE_OK, m_ecdb.ExecuteSql("DELETE FROM bis_GeometricElement3d WHERE ElementId = 0x3B"));
        auto expectedJSON = R"json(
            [
               {
                  "sno": 1,
                  "class": "BisCore:Element",
                  "id": "0x1d",
                  "class_id": "0xce",
                  "MissingRowInTables": "bis_InformationReferenceElement"
               },
                {
                   "sno": 2,
                   "class": "BisCore:Element",
                   "id": "0x3b",
                   "class_id": "0xe7",
                   "MissingRowInTables": "bis_GeometricElement3d"
                }
            ]
        )json";
        ASSERT_STREQ(ParseJSON(expectedJSON).c_str(), runCheck(m_ecdb).c_str()) << "Failed for " << m_ecdb.GetDbFileName();
        m_ecdb.AbandonChanges();
    };

    ASSERT_EQ(BE_SQLITE_OK, OpenCopyOfDataFile("test.bim", "test.bim", Db::OpenMode::ReadWrite));
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));
    executeTest();
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IntegrityCheckerFixture, check_data_columns) {
    auto runCheck = [&](ECDbCR db) -> Utf8String {
        BeJsDocument out;
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "PRAGMA integrity_check(check_data_columns)"));
        EXPECT_EQ(3, stmt.GetColumnCount());
        EXPECT_STRCASEEQ("sno", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("table", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("column", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        EXPECT_EQ(PRIMITIVETYPE_Integer, stmt.GetColumnInfo(0).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(1).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(2).GetDataType().GetPrimitiveType());
        out.SetEmptyArray();
        while (stmt.Step() == BE_SQLITE_ROW) {
            auto row = out.appendObject();
            row["sno"] = stmt.GetValueInt(0);
            row["table"] = stmt.GetValueText(1);
            row["column"] = stmt.GetValueText(2);
        }
        return out.Stringify(StringifyFormat::Indented);
    };

    auto executeTest = [&]() {
        ASSERT_STREQ(ParseJSON("[]").c_str(), runCheck(m_ecdb).c_str()) << "expect this to pass";
        m_ecdb.ExecuteSql("ALTER TABLE bis_ElementRefersToElements DROP COLUMN ps3");
        auto expectedJSON = R"json(
            [
                {
                    "sno": 1,
                    "table": "bis_ElementRefersToElements",
                    "column": "ps3"
                }
            ]
        )json";
        ASSERT_STREQ(ParseJSON(expectedJSON).c_str(), runCheck(m_ecdb).c_str()) << "Failed for " << m_ecdb.GetDbFileName();
        m_ecdb.AbandonChanges();
    };
    //! delete a data table 4000
    ASSERT_EQ(BE_SQLITE_OK, OpenCopyOfDataFile("fileformatbenchmark/4000/imodel2.ecdb", "4000.bim", Db::OpenMode::ReadWrite));
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));
    executeTest();

    //! delete a data table 4001
    ASSERT_EQ(BE_SQLITE_OK, OpenCopyOfDataFile("fileformatbenchmark/4001/imodel2.ecdb", "4001.bim", Db::OpenMode::ReadWrite));
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));
    executeTest();

    //! delete a profile table 4002
    ASSERT_EQ(BE_SQLITE_OK, OpenCopyOfDataFile("fileformatbenchmark/4002/imodel2.ecdb", "4002.bim", Db::OpenMode::ReadWrite));
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));

    executeTest();

    //! delete a data table 4003
    ASSERT_EQ(BE_SQLITE_OK, OpenCopyOfDataFile("fileformatbenchmark/4003/imodel2.ecdb", "4003.bim", Db::OpenMode::ReadWrite));
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));
    executeTest();
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IntegrityCheckerFixture, check_data_schema) {
    auto runCheck = [&](ECDbCR db) -> Utf8String {
        BeJsDocument out;
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "PRAGMA integrity_check(check_data_schema)"));
        EXPECT_EQ(3, stmt.GetColumnCount());
        EXPECT_STRCASEEQ("sno", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("type", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("name", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        EXPECT_EQ(PRIMITIVETYPE_Integer, stmt.GetColumnInfo(0).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(1).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(2).GetDataType().GetPrimitiveType());
        out.SetEmptyArray();
        while (stmt.Step() == BE_SQLITE_ROW) {
            auto row = out.appendObject();
            row["sno"] = stmt.GetValueInt(0);
            row["type"] = stmt.GetValueText(1);
            row["name"] = stmt.GetValueText(2);
        }
        return out.Stringify(StringifyFormat::Indented);
    };

    auto executeTest = [&]() {
        ASSERT_STREQ(ParseJSON("[]").c_str(), runCheck(m_ecdb).c_str()) << "expect this to pass";
        m_ecdb.ExecuteSql("DROP TABLE bis_DefinitionElement");
        auto expectedJSON = R"json(
            [
                {
                    "sno": 1,
                    "type": "table",
                    "name": "bis_DefinitionElement"
                },
                {
                    "sno": 2,
                    "type": "index",
                    "name": "ix_bis_DefinitionElement_ecclassid"
                }
            ]
        )json";
        ASSERT_STREQ(ParseJSON(expectedJSON).c_str(), runCheck(m_ecdb).c_str()) << "Failed for " << m_ecdb.GetDbFileName();
        m_ecdb.AbandonChanges();
    };
    //! delete a data table 4000
    ASSERT_EQ(BE_SQLITE_OK, OpenCopyOfDataFile("fileformatbenchmark/4000/imodel2.ecdb", "4000.bim", Db::OpenMode::ReadWrite));
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));

    executeTest();

    //! delete a data table 4001
    ASSERT_EQ(BE_SQLITE_OK, OpenCopyOfDataFile("fileformatbenchmark/4001/imodel2.ecdb", "4001.bim", Db::OpenMode::ReadWrite));
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));

    executeTest();

    //! delete a data table 4002 - skipping this test file as it already have issues.

    //! delete a data table 4003
    ASSERT_EQ(BE_SQLITE_OK, OpenCopyOfDataFile("fileformatbenchmark/4003/imodel2.ecdb", "4003.bim", Db::OpenMode::ReadWrite));
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));

    executeTest();
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IntegrityCheckerFixture, check_ec_profile) {
    auto runCheck = [&](ECDbCR db) -> Utf8String {
        BeJsDocument out;
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "PRAGMA integrity_check(check_ec_profile)"));
        EXPECT_EQ(4, stmt.GetColumnCount());
        EXPECT_STRCASEEQ("sno", stmt.GetColumnInfo(0).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("type", stmt.GetColumnInfo(1).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("name", stmt.GetColumnInfo(2).GetProperty()->GetName().c_str());
        EXPECT_STRCASEEQ("issue", stmt.GetColumnInfo(3).GetProperty()->GetName().c_str());
        EXPECT_EQ(PRIMITIVETYPE_Integer, stmt.GetColumnInfo(0).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(1).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(2).GetDataType().GetPrimitiveType());
        EXPECT_EQ(PRIMITIVETYPE_String, stmt.GetColumnInfo(3).GetDataType().GetPrimitiveType());
        out.SetEmptyArray();
        while (stmt.Step() == BE_SQLITE_ROW) {
            auto row = out.appendObject();
            row["sno"] = stmt.GetValueInt(0);
            row["type"] = stmt.GetValueText(1);
            row["name"] = stmt.GetValueText(2);
            row["issue"] = stmt.GetValueText(3);
        }
        return out.Stringify(StringifyFormat::Indented);
    };
    auto alreadyMissingTriggers = R"json(
        [
            {
                "sno": 1,
                "type": "trigger",
                "name": "dgn_fts_ad",
                "issue": "missing"
            },
            {
                "sno": 2,
                "type": "trigger",
                "name": "dgn_fts_ai",
                "issue": "missing"
            },
            {
                "sno": 3,
                "type": "trigger",
                "name": "dgn_fts_au",
                "issue": "missing"
            },
            {
                "sno": 4,
                "type": "trigger",
                "name": "dgn_prjrange_del",
                "issue": "missing"
            },
            {
                "sno": 5,
                "type": "trigger",
                "name": "dgn_rtree_ins",
                "issue": "missing"
            },
            {
                "sno": 6,
                "type": "trigger",
                "name": "dgn_rtree_upd",
                "issue": "missing"
            },
            {
                "sno": 7,
                "type": "trigger",
                "name": "dgn_rtree_upd1",
                "issue": "missing"
            }
        ]
    )json";
    auto executeTest = [&]() {
        ASSERT_STREQ(ParseJSON(alreadyMissingTriggers).c_str(), runCheck(m_ecdb).c_str()) << "expect this to pass";
        m_ecdb.ExecuteSql("DROP TABLE ec_Column");
        auto expectedJSON = R"json(
            [
                {
                    "sno": 1,
                    "type": "table",
                    "name": "ec_Column",
                    "issue": "missing"
                },
                {
                    "sno": 2,
                    "type": "index",
                    "name": "uix_ec_Column_TableId_Name",
                    "issue": "missing"
                },
                {
                    "sno": 3,
                    "type": "index",
                    "name": "uix_ec_Column_TableId_Ordinal",
                    "issue": "missing"
                },
                {
                    "sno": 4,
                    "type": "trigger",
                    "name": "dgn_fts_ad",
                    "issue": "missing"
                },
                {
                    "sno": 5,
                    "type": "trigger",
                    "name": "dgn_fts_ai",
                    "issue": "missing"
                },
                {
                    "sno": 6,
                    "type": "trigger",
                    "name": "dgn_fts_au",
                    "issue": "missing"
                },
                {
                    "sno": 7,
                    "type": "trigger",
                    "name": "dgn_prjrange_del",
                    "issue": "missing"
                },
                {
                    "sno": 8,
                    "type": "trigger",
                    "name": "dgn_rtree_ins",
                    "issue": "missing"
                },
                {
                    "sno": 9,
                    "type": "trigger",
                    "name": "dgn_rtree_upd",
                    "issue": "missing"
                },
                {
                    "sno": 10,
                    "type": "trigger",
                    "name": "dgn_rtree_upd1",
                    "issue": "missing"
                }
            ]
        )json";
        ASSERT_STREQ(ParseJSON(expectedJSON).c_str(), runCheck(m_ecdb).c_str()) << "Failed for " << m_ecdb.GetDbFileName();
        m_ecdb.AbandonChanges();
    };
    //! delete a profile table 4000
    ASSERT_EQ(BE_SQLITE_OK, OpenCopyOfDataFile("fileformatbenchmark/4000/imodel2.ecdb", "4000.bim", Db::OpenMode::ReadWrite));
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));

    executeTest();

    //! delete a profile table 4001
    ASSERT_EQ(BE_SQLITE_OK, OpenCopyOfDataFile("fileformatbenchmark/4001/imodel2.ecdb", "4001.bim", Db::OpenMode::ReadWrite));
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));

    executeTest();

    //! delete a profile table 4002
    ASSERT_EQ(BE_SQLITE_OK, OpenCopyOfDataFile("fileformatbenchmark/4002/imodel2.ecdb", "4002.bim", Db::OpenMode::ReadWrite));
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));

    executeTest();

    //! delete a profile table 4003
    ASSERT_EQ(BE_SQLITE_OK, OpenCopyOfDataFile("fileformatbenchmark/4003/imodel2.ecdb", "4003.bim", Db::OpenMode::ReadWrite));
    ASSERT_FALSE(IsECSqlExperimentalFeaturesEnabled(m_ecdb));
    ASSERT_TRUE(EnableECSqlExperimentalFeatures(m_ecdb, true));

    executeTest();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IntegrityCheckerFixture, check_linktable_invalid_classIds)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("check_linktable_invalid_classIds.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
	        <ECSchemaReference name="ECDbMap" version="2.0.0" alias="ecdbmap" />

            <ECEntityClass typeName="ClassA" modifier="None">
                <ECProperty propertyName="ClassAProp" typeName="string" displayLabel="ClassAProp"/>
            </ECEntityClass>
            <ECEntityClass typeName="ClassB" modifier="None">
                <ECProperty propertyName="ClassBProp" typeName="string" displayLabel="ClassBProp"/>
            </ECEntityClass>
            <ECEntityClass typeName="ClassC" modifier="None">
                <ECProperty propertyName="ClassCProp" typeName="string" displayLabel="ClassCProp"/>
            </ECEntityClass>

            <ECRelationshipClass typeName="RelA_B" modifier="Sealed" strength="referencing" strengthDirection="backward">
                <ECCustomAttributes>
                    <LinkTableRelationshipMap xmlns="ECDbMap.2.0.0">
                        <CreateForeignKeyConstraints>False</CreateForeignKeyConstraints>
                    </LinkTableRelationshipMap>
                </ECCustomAttributes>
                <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                    <Class class="ClassA"/>
                </Source>
                <Target multiplicity="(1..*)" roleLabel="is referenced by" polymorphic="true">
                    <Class class="ClassB"/>
                </Target>
            </ECRelationshipClass>

            <ECRelationshipClass typeName="RelB_A" modifier="Sealed" strength="referencing" strengthDirection="backward">
                <ECCustomAttributes>
                    <LinkTableRelationshipMap xmlns="ECDbMap.2.0.0">
                        <CreateForeignKeyConstraints>False</CreateForeignKeyConstraints>
                    </LinkTableRelationshipMap>
                </ECCustomAttributes>
                <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                    <Class class="ClassB"/>
                </Source>
                <Target multiplicity="(1..*)" roleLabel="is referenced by" polymorphic="true">
                    <Class class="ClassA"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml")));

    auto insertEntry = [&](Utf8CP className, Utf8CP propName)
        {
        ECSqlStatement stmt;
        ECInstanceKey outKey;
        if (ECSqlStatus::Success == stmt.Prepare(m_ecdb, Utf8PrintfString("INSERT INTO TestSchema.%s(%sProp) VALUES('%s')", className, className, propName).c_str()))
            stmt.Step(outKey);
        return outKey;
        };

    auto insertRelationship = [&](Utf8CP className, const ECInstanceKey& sourceKey, const ECInstanceKey& targetKey)
        {
        ECSqlStatement stmt;
        ECInstanceKey outKey;
        if (ECSqlStatus::Success == stmt.Prepare(m_ecdb, Utf8PrintfString("INSERT INTO TestSchema.%s(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(%s,%s,%s,%s)",
            className, sourceKey.GetInstanceId().ToString().c_str(), sourceKey.GetClassId().ToString().c_str(), targetKey.GetInstanceId().ToString().c_str(), targetKey.GetClassId().ToString().c_str()).c_str()))
            stmt.Step(outKey);
        return outKey;
        };

    // Insert class instances
    const auto classA1 = insertEntry("ClassA", "A1");
    const auto classA2 = insertEntry("ClassA", "A2");
    const auto classC1 = insertEntry("ClassC", "C1");
    const auto classB1 = insertEntry("ClassB", "B1");
    const auto classC2 = insertEntry("ClassC", "C2");
    const auto classB2 = insertEntry("ClassB", "B2");

    // Insert link table relationships
    EXPECT_TRUE(insertRelationship("RelA_B", classA1, classB1).IsValid()); // Valid
    EXPECT_TRUE(insertRelationship("RelB_A", classB2, classA2).IsValid()); // Valid
    EXPECT_TRUE(insertRelationship("RelA_B", classC1, classB2).IsValid()); // Invalid source, valid target
    EXPECT_TRUE(insertRelationship("RelA_B", classA1, classC1).IsValid()); // Valid source, invalid target
    EXPECT_TRUE(insertRelationship("RelA_B", classB1, classA1).IsValid()); // Invalid source, invalid target
    EXPECT_TRUE(insertRelationship("RelB_A", classA2, classB2).IsValid()); // Invalid source, invalid target

    BeJsDocument out;
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "pragma integrity_check(check_linktable_fk_ids) options enable_experimental_features"));
    EXPECT_EQ(6, stmt.GetColumnCount());

    out.SetEmptyArray();
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        auto row = out.appendObject();
        row["sno"] = stmt.GetValueInt(0);
        row["id"] = stmt.GetValueText(1);
        row["relationship"] = stmt.GetValueText(2);
        row["property"] = stmt.GetValueText(3);
        row["key_id"] = stmt.GetValueText(4);
        row["primary_class"] = stmt.GetValueText(5);
        }

    const auto expectedJSON = R"json(
        [
            {
               "sno": 1,
               "id": "0x9",
               "relationship": "TestSchema:RelA_B",
               "property": "SourceECInstanceId",
               "key_id": "0x3",
               "primary_class": "TestSchema:ClassA"
            },
            {
               "sno": 2,
               "id": "0xb",
               "relationship": "TestSchema:RelA_B",
               "property": "SourceECInstanceId",
               "key_id": "0x4",
               "primary_class": "TestSchema:ClassA"
            },
            {
               "sno": 3,
               "id": "0xb",
               "relationship": "TestSchema:RelA_B",
               "property": "TargetECInstanceId",
               "key_id": "0x1",
               "primary_class": "TestSchema:ClassB"
            },
            {
               "sno": 4,
               "id": "0xa",
               "relationship": "TestSchema:RelA_B",
               "property": "TargetECInstanceId",
               "key_id": "0x3",
               "primary_class": "TestSchema:ClassB"
            },
            {
               "sno": 5,
               "id": "0xc",
               "relationship": "TestSchema:RelB_A",
               "property": "SourceECInstanceId",
               "key_id": "0x2",
               "primary_class": "TestSchema:ClassB"
            },
            {
               "sno": 6,
               "id": "0xc",
               "relationship": "TestSchema:RelB_A",
               "property": "TargetECInstanceId",
               "key_id": "0x6",
               "primary_class": "TestSchema:ClassA"
            }
        ])json";
    ASSERT_STREQ(ParseJSON(expectedJSON).c_str(), out.Stringify(StringifyFormat::Indented).c_str()) << "Failed for " << m_ecdb.GetDbFileName();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IntegrityCheckerFixture, check_linktable_invalid_classIds_TPH)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("check_linktable_invalid_classIds_TPH.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
	        <ECSchemaReference name="ECDbMap" version="2.0.0" alias="ecdbmap" />

            <ECEntityClass typeName="BaseClassA" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.2.0.0">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="SubClassB" modifier="None">
                <BaseClass>BaseClassA</BaseClass>
                <ECProperty propertyName="SubClassBProp" typeName="string" displayLabel="ClassBProp"/>
            </ECEntityClass>
            <ECEntityClass typeName="SubClassC" modifier="None">
                <BaseClass>BaseClassA</BaseClass>
                <ECProperty propertyName="SubClassCProp" typeName="string" displayLabel="ClassCProp"/>
            </ECEntityClass>

            <ECEntityClass typeName="ClassD" modifier="None">
                <ECProperty propertyName="ClassDProp" typeName="string" displayLabel="ClassCProp"/>
            </ECEntityClass>

            <ECRelationshipClass typeName="RelA_A" modifier="Sealed" strength="referencing" strengthDirection="backward">
                <ECCustomAttributes>
                    <LinkTableRelationshipMap xmlns="ECDbMap.2.0.0">
                        <CreateForeignKeyConstraints>False</CreateForeignKeyConstraints>
                    </LinkTableRelationshipMap>
                </ECCustomAttributes>
                <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                    <Class class="BaseClassA"/>
                </Source>
                <Target multiplicity="(1..*)" roleLabel="is referenced by" polymorphic="true">
                    <Class class="BaseClassA"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml")));

    auto insertEntry = [&](Utf8CP className, Utf8CP propName)
        {
        ECSqlStatement stmt;
        ECInstanceKey outKey;
        if (ECSqlStatus::Success == stmt.Prepare(m_ecdb, Utf8PrintfString("INSERT INTO TestSchema.%s(%sProp) VALUES('%s')", className, className, propName).c_str()))
            stmt.Step(outKey);
        return outKey;
        };

    auto insertRelationship = [&](Utf8CP className, const ECInstanceKey& sourceKey, const ECInstanceKey& targetKey)
        {
        ECSqlStatement stmt;
        ECInstanceKey outKey;
        if (ECSqlStatus::Success == stmt.Prepare(m_ecdb, Utf8PrintfString("INSERT INTO TestSchema.%s(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(%s,%s,%s,%s)",
            className, sourceKey.GetInstanceId().ToString().c_str(), sourceKey.GetClassId().ToString().c_str(), targetKey.GetInstanceId().ToString().c_str(), targetKey.GetClassId().ToString().c_str()).c_str()))
            stmt.Step(outKey);
        return outKey;
        };

    // Insert class instances
    const auto classB1 = insertEntry("SubClassB", "B1");
    const auto classB2 = insertEntry("SubClassB", "B2");
    const auto classC1 = insertEntry("SubClassC", "C1");
    const auto classC2 = insertEntry("SubClassC", "C2");
    const auto classD1 = insertEntry("ClassD", "D1");
    const auto classD2 = insertEntry("ClassD", "D2");

    // Insert link table relationships
    EXPECT_TRUE(insertRelationship("RelA_A", classB1, classB2).IsValid()); // Valid source, valid target
    EXPECT_TRUE(insertRelationship("RelA_A", classB1, classC1).IsValid()); // Valid source, valid target
    EXPECT_TRUE(insertRelationship("RelA_A", classC1, classB1).IsValid()); // Valid source, valid target
    EXPECT_TRUE(insertRelationship("RelA_A", classB2, classD1).IsValid()); // Valid source, invalid target
    EXPECT_TRUE(insertRelationship("RelA_A", classD1, classB1).IsValid()); // Invalid source, valid target
    EXPECT_TRUE(insertRelationship("RelA_A", classD1, classD2).IsValid()); // Invalid source, invalid target

    BeJsDocument out;
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "pragma integrity_check(check_linktable_fk_ids) options enable_experimental_features"));
    EXPECT_EQ(6, stmt.GetColumnCount());

    out.SetEmptyArray();
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        auto row = out.appendObject();
        row["sno"] = stmt.GetValueInt(0);
        row["id"] = stmt.GetValueText(1);
        row["relationship"] = stmt.GetValueText(2);
        row["property"] = stmt.GetValueText(3);
        row["key_id"] = stmt.GetValueText(4);
        row["primary_class"] = stmt.GetValueText(5);
        }

    const auto expectedJSON = R"json(
        [
            {
               "sno": 1,
               "id": "0xb",
               "relationship": "TestSchema:RelA_A",
               "property": "SourceECInstanceId",
               "key_id": "0x5",
               "primary_class": "TestSchema:BaseClassA"
            },
            {
               "sno": 2,
               "id": "0xc",
               "relationship": "TestSchema:RelA_A",
               "property": "SourceECInstanceId",
               "key_id": "0x5",
               "primary_class": "TestSchema:BaseClassA"
            },
            {
               "sno": 3,
               "id": "0xa",
               "relationship": "TestSchema:RelA_A",
               "property": "TargetECInstanceId",
               "key_id": "0x5",
               "primary_class": "TestSchema:BaseClassA"
            },
            {
               "sno": 4,
               "id": "0xc",
               "relationship": "TestSchema:RelA_A",
               "property": "TargetECInstanceId",
               "key_id": "0x6",
               "primary_class": "TestSchema:BaseClassA"
            }
        ])json";
    ASSERT_STREQ(ParseJSON(expectedJSON).c_str(), out.Stringify(StringifyFormat::Indented).c_str()) << "Failed for " << m_ecdb.GetDbFileName();
    }

END_ECDBUNITTESTS_NAMESPACE
