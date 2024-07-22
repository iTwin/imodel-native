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


END_ECDBUNITTESTS_NAMESPACE
