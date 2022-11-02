/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

#define ECDB_FILEINFO_SCHEMA_NAME "ECDbFileInfo"

#define TEST_SCHEMA_XML R"xml(<?xml version="1.0" encoding="utf-8" ?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
              <ECEntityClass typeName="Foo" >
                <ECProperty propertyName="Name" typeName="string" />
              </ECEntityClass>
              <ECEntityClass typeName="FooChild" >
                <BaseClass>Foo</BaseClass>
                <ECProperty propertyName="Label" typeName="string" />
              </ECEntityClass>
              <ECEntityClass typeName="Goo" >
                <ECProperty propertyName="Name" typeName="string" />
              </ECEntityClass>
            </ECSchema>)xml"

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct FileInfoTestFixture : ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileInfoTestFixture, EmptyECDbHasFileInfoSchema)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbfileinfo.ecdb", SchemaItem(TEST_SCHEMA_XML)));
    m_ecdb.Schemas().CreateClassViewsInDb();

    SchemaManager const& schemaManager = m_ecdb.Schemas();

    ECSchemaCP fileinfoSchema = schemaManager.GetSchema(ECDB_FILEINFO_SCHEMA_NAME, false);
    ASSERT_TRUE(fileinfoSchema != nullptr) << "Empty ECDb file is expected to contain the ECDbFileInfo ECSchema.";

    ECClassCP ecClass = schemaManager.GetClass(ECDB_FILEINFO_SCHEMA_NAME, "FileInfo");
    ASSERT_TRUE(ecClass != nullptr);

    ECEnumerationCP rootFolderEnum = schemaManager.GetEnumeration(ECDB_FILEINFO_SCHEMA_NAME, "StandardRootFolderType");
    ASSERT_TRUE(rootFolderEnum != nullptr);
    ASSERT_FALSE(rootFolderEnum->GetIsStrict());

    ecClass = schemaManager.GetClass(ECDB_FILEINFO_SCHEMA_NAME, "ExternalFileInfo");
    ASSERT_TRUE(ecClass != nullptr);

    ecClass = schemaManager.GetClass(ECDB_FILEINFO_SCHEMA_NAME, "EmbeddedFileInfo");
    ASSERT_TRUE(ecClass != nullptr);
    ecClass = schemaManager.GetClass(ECDB_FILEINFO_SCHEMA_NAME, "FileInfoOwnership");
    ASSERT_TRUE(ecClass != nullptr);

    ASSERT_TRUE(GetHelper().TableExists("be_EmbedFile")) << "Empty ECDb file is expected to contain the table 'be_EmbedFile'.";

    ECSqlStatement insertStmt;
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.Prepare(m_ecdb, "INSERT INTO ecdbf.ExternalFileInfo (Name, Size, LastModified) VALUES ('myexternalfile.pdf', 1024, DATE '2014-09-25')"));
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step());

    ECSqlStatement selStmt;
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "SELECT * FROM ecdbf.ExternalFileInfo"));

    int rowCount = 0;
    while (selStmt.Step() == BE_SQLITE_ROW)
        {
        rowCount++;
        }

    ASSERT_EQ(1, rowCount);
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileInfoTestFixture, PolymorphicQueryRightAfterCreation)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbfileinfo.ecdb", SchemaItem(TEST_SCHEMA_XML)));

    ECSqlStatement selStmt0;
    ASSERT_EQ(ECSqlStatus::Success, selStmt0.Prepare(m_ecdb, "SELECT * FROM ecdbf.EmbeddedFileInfo"));

    ECSqlStatement selStmt;
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "SELECT * FROM ecdbf.FileInfo"));
    }


//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileInfoTestFixture, SubclassingExternalFileInfo)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("subclassingexternalfileinfo.ecdb",
                                 SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbFileInfo" version="02.01" alias="ecdbf"/>
                <ECEntityClass typeName="MyExternalFileInfo" modifier="Sealed">
                    <BaseClass>ecdbf:ExternalFileInfo</BaseClass>
                    <ECProperty propertyName="MyExtraInformation" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="YourExternalFileInfo">
                    <BaseClass>ecdbf:ExternalFileInfo</BaseClass>
                    <ECProperty propertyName="YourExtraInformation" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="YourSpecialExternalFileInfo">
                    <BaseClass>YourExternalFileInfo</BaseClass>
                    <ECProperty propertyName="YourSpecialExtraInformation" typeName="string" />
                </ECEntityClass>
            </ECSchema>)xml")));

    ASSERT_FALSE(GetHelper().TableExists("_ecdbf_FileInfo")) << "FileInfo is abstract but as subclass with strategy existing and therefore is not updatable and therefore should not have an updatable view";
    ASSERT_TRUE(GetHelper().TableExists("ecdbf_ExternalFileInfo"));
    ASSERT_FALSE(GetHelper().TableExists("ts_MyExternalFileInfo"));
    ASSERT_FALSE(GetHelper().TableExists("ts_YourExternalFileInfo"));
    ASSERT_FALSE(GetHelper().TableExists("ts_YourSpecialExternalFileInfo"));

    ASSERT_EQ(ExpectedColumn("ecdbf_ExternalFileInfo", "ps1"), GetHelper().GetPropertyMapColumn(AccessString("ts", "MyExternalFileInfo", "MyExtraInformation")));
    ASSERT_EQ(ExpectedColumn("ecdbf_ExternalFileInfo", "ps1"), GetHelper().GetPropertyMapColumn(AccessString("ts", "YourExternalFileInfo", "YourExtraInformation")));
    ASSERT_EQ(ExpectedColumn("ecdbf_ExternalFileInfo", "ps1"), GetHelper().GetPropertyMapColumn(AccessString("ts", "YourSpecialExternalFileInfo", "YourExtraInformation")));
    ASSERT_EQ(ExpectedColumn("ecdbf_ExternalFileInfo", "ps2"), GetHelper().GetPropertyMapColumn(AccessString("ts", "YourSpecialExternalFileInfo", "YourSpecialExtraInformation")));

    ECInstanceKey extFileKey, myExtFileKey, yourExtFileKey, yourSpecialExtFileKey;
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecdbf.ExternalFileInfo(Name, RootFolder) VALUES('file1',1)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(extFileKey));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.MyExternalFileInfo(Name, RootFolder, MyExtraInformation) VALUES('file2',2,'my extra')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(myExtFileKey));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.YourExternalFileInfo(Name, RootFolder, YourExtraInformation) VALUES('file3',3,'your extra')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(yourExtFileKey));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.YourSpecialExternalFileInfo(Name, RootFolder, YourExtraInformation, YourSpecialExtraInformation) VALUES('file4',3,'your extra', 'special extra')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(yourSpecialExtFileKey));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId,Name FROM ecdbf.FileInfo"));
    while (BE_SQLITE_ROW == stmt.Step())
        {
        ECInstanceId actualId = stmt.GetValueId<ECInstanceId>(0);
        Utf8CP actualFileName = stmt.GetValueText(1);
        if (actualId == extFileKey.GetInstanceId())
            ASSERT_STREQ("file1", actualFileName) << actualId.ToString().c_str();
        else if (actualId == myExtFileKey.GetInstanceId())
            ASSERT_STREQ("file2", actualFileName) << actualId.ToString().c_str();
        else if (actualId == yourExtFileKey.GetInstanceId())
            ASSERT_STREQ("file3", actualFileName) << actualId.ToString().c_str();
        else if (actualId == yourSpecialExtFileKey.GetInstanceId())
            ASSERT_STREQ("file4", actualFileName) << actualId.ToString().c_str();
        else
            FAIL() << "FileInfo with ECInstanceId " << actualId.ToString().c_str() << " should not exist";
        }

    stmt.Finalize();

    //update
    //non-polymorphic
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ONLY ecdbf.ExternalFileInfo SET Description='I am a file info'"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ecdbf.FileInfo WHERE Description = 'I am a file info'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(1, stmt.GetValueInt(0)) << stmt.GetECSql();
    stmt.Finalize();

    //polymorphic
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ecdbf.ExternalFileInfo SET Description='I am a file info'"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ecdbf.FileInfo WHERE Description = 'I am a file info'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(4, stmt.GetValueInt(0)) << stmt.GetECSql();
    stmt.Finalize();

    //delete
    //non-polymorphic
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "DELETE FROM ONLY ecdbf.ExternalFileInfo"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ecdbf.FileInfo"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(3, stmt.GetValueInt(0)) << stmt.GetECSql();
    stmt.Finalize();

    //polymorphic
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "DELETE FROM ecdbf.ExternalFileInfo"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ecdbf.FileInfo"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(0, stmt.GetValueInt(0)) << stmt.GetECSql();
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileInfoTestFixture, ECFEmbeddedFileBackedInstanceSupport)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbfileinfo.ecdb", SchemaItem(TEST_SCHEMA_XML)));

    //test file
    Utf8CP testFileName = "ECSqlTest.01.00.00.ecschema.xml";
    WString testFileNameW(testFileName, BentleyCharEncoding::Utf8);

    BeFileName testFilePath;
    BeTest::GetHost().GetDocumentsRoot(testFilePath);
    testFilePath.AppendToPath(L"ECDb");
    testFilePath.AppendToPath(L"Schemas");
    testFilePath.AppendToPath(testFileNameW.c_str());


    //Insert Foo instance
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Foo (Name) VALUES (?)"));
    stmt.BindText(1, "Foo1", IECSqlBinder::MakeCopy::Yes);
    ECInstanceKey fooKey;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(fooKey));
    stmt.Finalize();

    ECClassCP embeddedFileInfoClass = m_ecdb.Schemas().GetClass(ECDB_FILEINFO_SCHEMA_NAME, "EmbeddedFileInfo");
    ASSERT_TRUE(embeddedFileInfoClass != nullptr);

    //INSERT scenario
    DbEmbeddedFileTable& embeddedFileTable = m_ecdb.EmbeddedFiles();
    DbResult stat = BE_SQLITE_OK;
    DateTime expectedLastModified = DateTime::GetCurrentTimeUtc();
    double expectedLastModifiedJd = 0.0;
    ASSERT_EQ(SUCCESS, expectedLastModified.ToJulianDay(expectedLastModifiedJd));

    BeBriefcaseBasedId embeddedFileId = embeddedFileTable.Import(&stat, true, testFileName, testFilePath.GetNameUtf8().c_str(), &expectedLastModified);
    ASSERT_EQ(BE_SQLITE_OK, stat);
    ASSERT_TRUE(embeddedFileId.IsValid());

    //insert ownership
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecdbf.FileInfoOwnership (OwnerId, OwnerECClassId, FileInfoId, FileInfoECClassId) VALUES (?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, fooKey.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, embeddedFileId));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, embeddedFileInfoClass->GetId()));

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //RETRIEVE scenario
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT fi.Name, fi.LastModified, fi.ECInstanceId FROM ecdbf.FileInfo fi JOIN ecdbf.FileInfoOwnership o ON fi.ECInstanceId=o.FileInfoId AND fi.ECClassId=o.FileInfoECClassId WHERE o.OwnerId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    Utf8CP actualFileName = stmt.GetValueText(0);
    ASSERT_STREQ(testFileName, actualFileName);
    DateTime actualLastModified = stmt.GetValueDateTime(1);
    double actualLastModifiedJd = 0.0;
    ASSERT_EQ(SUCCESS, actualLastModified.ToJulianDay(actualLastModifiedJd));
    EXPECT_DOUBLE_EQ(expectedLastModifiedJd, actualLastModifiedJd);
    ECInstanceId actualFileId = stmt.GetValueId<ECInstanceId>(2);
    ASSERT_EQ(embeddedFileId.GetValue(), actualFileId.GetValue());

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Only 1 entry expected in FileInfoOwnership for given OwnerId";

    BeFileName exportFilePath;
    BeTest::GetHost().GetOutputRoot(exportFilePath);
    exportFilePath.AppendToPath(testFileNameW.c_str());
    if (BeFileName::DoesPathExist(exportFilePath))
        {
        // Delete any previously exported file
        BeFileNameStatus fileDeleteStatus = BeFileName::BeDeleteFile(exportFilePath);
        ASSERT_EQ(BeFileNameStatus::Success, fileDeleteStatus);
        }


    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Export(exportFilePath.GetNameUtf8().c_str(), testFileName));

    stmt.Finalize();
    m_ecdb.SaveChanges();
    //DELETE scenario -> FileInfo is not implicitly deleted when the owner is deleted
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "DELETE FROM ONLY ts.Foo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ecdbf.FileInfo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, embeddedFileId));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(1, stmt.GetValueInt(0)) << "FileInfo is expected to still contain the one instance inserted";
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT count(*) FROM ecdbf.FileInfoOwnership WHERE OwnerId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(1, stmt.GetValueInt(0)) << "FileInfoOwnership is expected to still contain the ownership for the deleted Foo instance";
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "DELETE FROM ONLY ecdbf.EmbeddedFileInfo WHERE ECInstanceId=?")) << "Deletion not allowed for ECClasses that map to existing tables";
    stmt.Finalize();
    }


BeFileName SearchTestFile(Utf8CP testFileName)
    {
    WString testFileNameW(testFileName, BentleyCharEncoding::Utf8);

    BeFileName testFilePath;
    BeTest::GetHost().GetDocumentsRoot(testFilePath);
    testFilePath.AppendToPath(L"ECDb");
    testFilePath.AppendToPath(L"Schemas");
    testFilePath.AppendToPath(testFileNameW.c_str());
    return testFilePath;
    }


//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileInfoTestFixture, FileInfoOwnershipConstraints)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbfileinfo.ecdb", SchemaItem(TEST_SCHEMA_XML)));

    //Constraint: None of the properties can be null
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecdbf.FileInfoOwnership(OwnerId, OwnerECClassId, FileInfoId, FileInfoECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, ECInstanceId(UINT64_C(1))));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, ECClassId(UINT64_C(1))));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, ECInstanceId(UINT64_C(2))));
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_NOTNULL, stmt.Step());

    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, ECInstanceId(UINT64_C(1))));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, ECClassId(UINT64_C(1))));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, ECClassId(UINT64_C(2))));
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_NOTNULL, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, ECInstanceId(UINT64_C(1))));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, ECInstanceId(UINT64_C(2))));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, ECClassId(UINT64_C(2))));
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_NOTNULL, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, ECClassId(UINT64_C(1))));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, ECInstanceId(UINT64_C(2))));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, ECClassId(UINT64_C(2))));
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_NOTNULL, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, ECInstanceId(UINT64_C(1))));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, ECClassId(UINT64_C(1))));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, ECInstanceId(UINT64_C(2))));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, ECClassId(UINT64_C(2))));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    //Ensure no duplicates
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, ECInstanceId(UINT64_C(1))));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, ECClassId(UINT64_C(1))));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, ECInstanceId(UINT64_C(2))));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, ECClassId(UINT64_C(2))));
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, stmt.Step()) << "Inserting same ownership twice is expected to fail";
    stmt.Reset();
    stmt.ClearBindings();

    //One owner cannot have more than one file info
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, ECInstanceId(UINT64_C(1))));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, ECClassId(UINT64_C(1))));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, ECInstanceId(UINT64_C(10))));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, ECClassId(UINT64_C(2))));
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, stmt.Step()) << "One owner cannot have more than one file info";
    stmt.Reset();
    stmt.ClearBindings();

    //One file cannot have more than one file infos
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, ECInstanceId(UINT64_C(20))));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, ECClassId(UINT64_C(1))));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, ECInstanceId(UINT64_C(2))));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, ECClassId(UINT64_C(2))));
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, stmt.Step()) << "One file cannot have more than one file infos";
    stmt.Reset();
    stmt.ClearBindings();

    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void AssertPurge(ECDbCR ecdb, std::vector<std::pair<ECInstanceKey, ECInstanceKey>>& expectedOwnerships, std::vector<ECInstanceKey>& expectedFileInfos)
    {
    struct CompareECInstanceKeyPair
        {
        bool operator()(std::pair<ECInstanceKey, ECInstanceKey> const& lhs, std::pair<ECInstanceKey, ECInstanceKey> const& rhs) const
            {
            ECInstanceKey lhsFirstKey = lhs.first;
            ECInstanceKey lhsSecondKey = lhs.second;
            ECInstanceKey rhsFirstKey = rhs.first;
            ECInstanceKey rhsSecondKey = rhs.second;

            return (lhsFirstKey < rhsFirstKey) || (lhsFirstKey == rhsFirstKey && lhsSecondKey < rhsSecondKey);
            }
        };

    CompareECInstanceKeyPair compare;
    std::sort(expectedOwnerships.begin(), expectedOwnerships.end(), compare);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT OwnerECClassId, OwnerId, FileInfoECClassId, FileInfoId FROM ecdbf.FileInfoOwnership ORDER BY OwnerECClassId, OwnerId, FileInfoECClassId, FileInfoId"));

    size_t i = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        const ECClassId expectedOwnerClassId = expectedOwnerships[i].first.GetClassId();
        const ECInstanceId expectedOwnerId = expectedOwnerships[i].first.GetInstanceId();
        const ECClassId expectedFileInfoClassId = expectedOwnerships[i].second.GetClassId();
        const ECInstanceId expectedFileInfoId = expectedOwnerships[i].second.GetInstanceId();

        ECClassId actualOwnerClassId = stmt.GetValueId<ECClassId>(0);
        ASSERT_EQ(expectedOwnerClassId, actualOwnerClassId);
        ECInstanceId actualOwnerId = stmt.GetValueId<ECInstanceId>(1);
        ASSERT_EQ(expectedOwnerId.GetValue(), actualOwnerId.GetValue());
        ECClassId actualFileInfoClassId = stmt.GetValueId<ECClassId>(2);
        ASSERT_EQ(expectedFileInfoClassId, actualFileInfoClassId);
        ECInstanceId actualFileInfoId = stmt.GetValueId<ECInstanceId>(3);
        ASSERT_EQ(expectedFileInfoId.GetValue(), actualFileInfoId.GetValue());

        i++;
        }

    stmt.Finalize();

    std::sort(expectedFileInfos.begin(), expectedFileInfos.end());
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECClassId, ECInstanceId FROM ecdbf.FileInfo ORDER BY ECClassId, ECInstanceId"));

    i = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        const ECClassId expectedFileInfoClassId = expectedFileInfos[i].GetClassId();
        const ECInstanceId expectedFileInfoId = expectedFileInfos[i].GetInstanceId();

        ECClassId actualFileInfoClassId = stmt.GetValueId<ECClassId>(0);
        ASSERT_EQ(expectedFileInfoClassId, actualFileInfoClassId);
        ECInstanceId actualFileInfoId = stmt.GetValueId<ECInstanceId>(1);
        ASSERT_EQ(expectedFileInfoId.GetValue(), actualFileInfoId.GetValue());

        i++;
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileInfoTestFixture, Purge)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbfileinfo.ecdb", SchemaItem(TEST_SCHEMA_XML)));

    ECInstanceKey fooKey;
    ECInstanceKey fooChildKey;
    ECInstanceKey gooKey;
    ECInstanceKey fooExternalFileInfoKey;
    ECInstanceKey gooExternalFileInfoKey;
    ECInstanceKey fooChildEmbeddedFileInfoKey;
    ECInstanceKey orphanEmbeddedFileInfoKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Foo (Name) VALUES ('Foo')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(fooKey)) << m_ecdb.GetLastError().c_str();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.FooChild (Name, Label) VALUES ('FooChild', 'My Foo child')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(fooChildKey)) << m_ecdb.GetLastError().c_str();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Goo (Name) VALUES ('Goo')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(gooKey)) << m_ecdb.GetLastError().c_str();
    stmt.Finalize();

    //ExternalFileInfos
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecdbf.ExternalFileInfo(Name, RootFolder, RelativePath) VALUES(?,1,'mydocuments/private/')"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "ExternalFile1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(fooExternalFileInfoKey)) << m_ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "ExternalFile2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(gooExternalFileInfoKey)) << m_ecdb.GetLastError().c_str();
    stmt.Finalize();

    //EmbeddedFileInfos
    ECClassId embeddedFileInfoClassId = m_ecdb.Schemas().GetClassId("ECDbFileInfo", "EmbeddedFileInfo");
    ASSERT_TRUE(embeddedFileInfoClassId.IsValid());

    Utf8CP testFileName = "ECSqlTest.01.00.00.ecschema.xml";

    BeFileName testFilePath;
    BeTest::GetHost().GetDocumentsRoot(testFilePath);
    testFilePath.AppendToPath(L"ECDb");
    testFilePath.AppendToPath(L"Schemas");
    testFilePath.AppendToPath(WString(testFileName, BentleyCharEncoding::Utf8).c_str());

    DbEmbeddedFileTable& embeddedFileTable = m_ecdb.EmbeddedFiles();
    DbResult stat = BE_SQLITE_OK;
    DateTime lastModified = DateTime::GetCurrentTimeUtc();
    BeBriefcaseBasedId id = embeddedFileTable.Import(&stat, true, testFileName, testFilePath.GetNameUtf8().c_str(), &lastModified);
    ASSERT_EQ(BE_SQLITE_OK, stat);
    ASSERT_TRUE(id.IsValid());
    fooChildEmbeddedFileInfoKey = ECInstanceKey(embeddedFileInfoClassId, ECInstanceId(id.GetValue()));

    testFileName = "Copy of ECSqlTest.01.00.00.ecschema.xml";
    lastModified = DateTime::GetCurrentTimeUtc();
    id = embeddedFileTable.Import(&stat,true, testFileName, testFilePath.GetNameUtf8().c_str(),  &lastModified);
    ASSERT_EQ(BE_SQLITE_OK, stat);
    ASSERT_TRUE(id.IsValid());
    orphanEmbeddedFileInfoKey = ECInstanceKey(embeddedFileInfoClassId, ECInstanceId(id.GetValue()));

    ASSERT_EQ(SUCCESS, m_ecdb.Purge(ECDb::PurgeMode::FileInfoOwnerships)) << "Purge if nothing is to purge should succeed";

    //Ownership
    //Foo - ExternalFile1
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecdbf.FileInfoOwnership(OwnerId, OwnerECClassId, FileInfoId, FileInfoECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, fooKey.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, fooExternalFileInfoKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, fooExternalFileInfoKey.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << m_ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();

    //FooChild - EmbeddedFile1
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooChildKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, fooChildKey.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, fooChildEmbeddedFileInfoKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, fooChildEmbeddedFileInfoKey.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << m_ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();

    //Goo - ExternalFile2
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, gooKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, gooKey.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, gooExternalFileInfoKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, gooExternalFileInfoKey.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << m_ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();

    m_ecdb.SaveChanges();
    }

    //Check file infos before doing anything
    std::vector<std::pair<ECInstanceKey, ECInstanceKey>> expectedOwnerships;
    expectedOwnerships.push_back(std::make_pair(fooKey, fooExternalFileInfoKey));
    expectedOwnerships.push_back(std::make_pair(fooChildKey, fooChildEmbeddedFileInfoKey));
    expectedOwnerships.push_back(std::make_pair(gooKey, gooExternalFileInfoKey));

    std::vector<ECInstanceKey> expectedFileInfos;
    expectedFileInfos.push_back(fooExternalFileInfoKey);
    expectedFileInfos.push_back(gooExternalFileInfoKey);
    expectedFileInfos.push_back(fooChildEmbeddedFileInfoKey);
    expectedFileInfos.push_back(orphanEmbeddedFileInfoKey);

    AssertPurge(m_ecdb, expectedOwnerships, expectedFileInfos);

    //Scenario 1: Delete owners and purge
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("ECDbFileInfo Purge> START");
    ASSERT_EQ(SUCCESS, m_ecdb.Purge(ECDb::PurgeMode::FileInfoOwnerships));
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("ECDbFileInfo Purge> END");

    AssertPurge(m_ecdb, expectedOwnerships, expectedFileInfos);

    //Now delete Owner Foo
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "DELETE FROM ONLY ts.Foo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << m_ecdb.GetLastError().c_str();
    stmt.Finalize();
    ASSERT_EQ(SUCCESS, m_ecdb.Purge(ECDb::PurgeMode::FileInfoOwnerships));

    expectedOwnerships.clear();
    expectedOwnerships.push_back(std::make_pair(fooChildKey, fooChildEmbeddedFileInfoKey));
    expectedOwnerships.push_back(std::make_pair(gooKey, gooExternalFileInfoKey));

    AssertPurge(m_ecdb, expectedOwnerships, expectedFileInfos);

    //Now delete Owner FooChild
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "DELETE FROM ONLY ts.FooChild WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooChildKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << m_ecdb.GetLastError().c_str();
    stmt.Finalize();
    ASSERT_EQ(SUCCESS, m_ecdb.Purge(ECDb::PurgeMode::FileInfoOwnerships));

    expectedOwnerships.clear();
    expectedOwnerships.push_back(std::make_pair(gooKey, gooExternalFileInfoKey));

    AssertPurge(m_ecdb, expectedOwnerships, expectedFileInfos);

    //Now delete Owner Goo
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "DELETE FROM ONLY ts.Goo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, gooKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << m_ecdb.GetLastError().c_str();
    stmt.Finalize();
    ASSERT_EQ(SUCCESS, m_ecdb.Purge(ECDb::PurgeMode::FileInfoOwnerships));

    expectedOwnerships.clear();
    AssertPurge(m_ecdb, expectedOwnerships, expectedFileInfos);

    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AbandonChanges());

    //Scenario 2: Delete fileinfos and purge
    //delete EmbeddedFiles
    //Embedded files cannot be deleted via ECSQL because they map to an existing table.
    //EmbeddedFile API cannot be used either as it does not allow deleting by id.
    //Therefore use SQLite directly which is not a problem for this test.
    Statement sqliteStmt;
    ASSERT_EQ(BE_SQLITE_OK, sqliteStmt.Prepare(m_ecdb, "DELETE FROM " BEDB_TABLE_EmbeddedFile " WHERE Id=?"));
    ASSERT_EQ(BE_SQLITE_OK, sqliteStmt.BindId(1, fooChildEmbeddedFileInfoKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, sqliteStmt.Step());
    sqliteStmt.Finalize();

    ASSERT_EQ(SUCCESS, m_ecdb.Purge(ECDb::PurgeMode::FileInfoOwnerships));
    expectedOwnerships.clear();
    expectedOwnerships.push_back(std::make_pair(fooKey, fooExternalFileInfoKey));
    expectedOwnerships.push_back(std::make_pair(gooKey, gooExternalFileInfoKey));

    expectedFileInfos.clear();
    expectedFileInfos.push_back(fooExternalFileInfoKey);
    expectedFileInfos.push_back(gooExternalFileInfoKey);
    expectedFileInfos.push_back(orphanEmbeddedFileInfoKey);

    AssertPurge(m_ecdb, expectedOwnerships, expectedFileInfos);

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "DELETE FROM ONLY ecdbf.ExternalFileInfo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooExternalFileInfoKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << m_ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(SUCCESS, m_ecdb.Purge(ECDb::PurgeMode::FileInfoOwnerships));

    expectedOwnerships.clear();
    expectedOwnerships.push_back(std::make_pair(gooKey, gooExternalFileInfoKey));

    expectedFileInfos.clear();
    expectedFileInfos.push_back(gooExternalFileInfoKey);
    expectedFileInfos.push_back(orphanEmbeddedFileInfoKey);

    AssertPurge(m_ecdb, expectedOwnerships, expectedFileInfos);

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, gooExternalFileInfoKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << m_ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(SUCCESS, m_ecdb.Purge(ECDb::PurgeMode::FileInfoOwnerships));

    expectedOwnerships.clear();
    expectedFileInfos.clear();
    expectedFileInfos.push_back(orphanEmbeddedFileInfoKey);

    AssertPurge(m_ecdb, expectedOwnerships, expectedFileInfos);
    m_ecdb.AbandonChanges();
    }


END_ECDBUNITTESTS_NAMESPACE
