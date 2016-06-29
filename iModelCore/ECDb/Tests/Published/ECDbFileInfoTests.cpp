/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbFileInfoTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

#define ECDB_FILEINFO_SCHEMA_NAME "ECDb_FileInfo"

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
struct ECDbFileInfoTests : ECDbTestFixture
    {
    protected:
        static Utf8CP GetTestSchemaXml();
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbFileInfoTests, EmptyECDbHasFileInfoSchema)
    {
    ECDbR ecdb = SetupECDb("ecdbfileinfo.ecdb", SchemaItem(GetTestSchemaXml()));
    ASSERT_TRUE(ecdb.IsDbOpen());

    auto const& schemaManager = ecdb.Schemas();

    ECSchemaCP fileinfoSchema = schemaManager.GetECSchema(ECDB_FILEINFO_SCHEMA_NAME, false);
    ASSERT_TRUE(fileinfoSchema != nullptr) << "Empty ECDb file is expected to contain the ECDb_FileInfo ECSchema.";

    ECClassCP ecClass = schemaManager.GetECClass(ECDB_FILEINFO_SCHEMA_NAME, "FileInfo");
    ASSERT_TRUE(ecClass != nullptr);

    ECEnumerationCP rootFolderEnum = schemaManager.GetECEnumeration(ECDB_FILEINFO_SCHEMA_NAME, "StandardRootFolderType");
    ASSERT_TRUE(rootFolderEnum != nullptr);
    ASSERT_FALSE(rootFolderEnum->GetIsStrict());

    ecClass = schemaManager.GetECClass(ECDB_FILEINFO_SCHEMA_NAME, "ExternalFileInfo");
    ASSERT_TRUE(ecClass != nullptr);

    ecClass = schemaManager.GetECClass(ECDB_FILEINFO_SCHEMA_NAME, "EmbeddedFileInfo");
    ASSERT_TRUE(ecClass != nullptr);
    ecClass = schemaManager.GetECClass(ECDB_FILEINFO_SCHEMA_NAME, "FileInfoOwnership");
    ASSERT_TRUE(ecClass != nullptr);

    ASSERT_TRUE(ecdb.TableExists("be_EmbedFile")) << "Empty ECDb file is expected to contain the table 'be_EmbedFile'.";

    ECSqlStatement insertStmt;
    ASSERT_EQ(ECSqlStatus::Success, insertStmt.Prepare(ecdb, "INSERT INTO ecdbf.ExternalFileInfo (Name, Size, LastModified) VALUES ('myexternalfile.pdf', 1024, DATE '2014-09-25')"));
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step());

    ECSqlStatement selStmt;
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(ecdb, "SELECT * FROM ecdbf.ExternalFileInfo"));

    int rowCount = 0;
    while (selStmt.Step() == BE_SQLITE_ROW)
        {
        rowCount++;
        }

    ASSERT_EQ(1, rowCount);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbFileInfoTests, PolymorphicQueryRightAfterCreation)
    {
    ECDbR ecdb = SetupECDb("ecdbfileinfo.ecdb", SchemaItem(GetTestSchemaXml()));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECSqlStatement selStmt0;
    ASSERT_EQ(ECSqlStatus::Success, selStmt0.Prepare(ecdb, "SELECT * FROM ecdbf.EmbeddedFileInfo"));

    ECSqlStatement selStmt;
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(ecdb, "SELECT * FROM ecdbf.FileInfo"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbFileInfoTests, ECFEmbeddedFileBackedInstanceSupport)
    {
    ECDbR ecdb = SetupECDb("ecdbfileinfo.ecdb", SchemaItem(GetTestSchemaXml()));
    ASSERT_TRUE(ecdb.IsDbOpen());

    //test file
    Utf8CP testFileName = "StartupCompany.json";
    WString testFileNameW(testFileName, BentleyCharEncoding::Utf8);

    BeFileName testFilePath;
    BeTest::GetHost().GetDocumentsRoot(testFilePath);
    testFilePath.AppendToPath(L"ECDb");
    testFilePath.AppendToPath(testFileNameW.c_str());


    //Insert Foo instance
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.Foo (Name) VALUES (?)"));
    stmt.BindText(1, "Foo1", IECSqlBinder::MakeCopy::Yes);
    ECInstanceKey fooKey;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(fooKey));
    stmt.Finalize();

    ECClassCP embeddedFileInfoClass = ecdb.Schemas().GetECClass(ECDB_FILEINFO_SCHEMA_NAME, "EmbeddedFileInfo");
    ASSERT_TRUE(embeddedFileInfoClass != nullptr);

    //INSERT scenario
    DbEmbeddedFileTable& embeddedFileTable = ecdb.EmbeddedFiles();
    DbResult stat = BE_SQLITE_OK;
    DateTime expectedLastModified = DateTime::GetCurrentTimeUtc();
    double expectedLastModifiedJd = 0.0;
    ASSERT_EQ(SUCCESS, expectedLastModified.ToJulianDay(expectedLastModifiedJd));

    BeBriefcaseBasedId embeddedFileId = embeddedFileTable.Import(&stat, testFileName, testFilePath.GetNameUtf8().c_str(), "JSON", nullptr, &expectedLastModified);
    ASSERT_EQ(BE_SQLITE_OK, stat);
    ASSERT_TRUE(embeddedFileId.IsValid());

    //insert ownership
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ecdbf.FileInfoOwnership (OwnerId, OwnerECClassId, FileInfoId, FileInfoECClassId) VALUES (?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKey.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, fooKey.GetECClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, embeddedFileId));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, embeddedFileInfoClass->GetId()));

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //RETRIEVE scenario
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT fi.Name, fi.LastModified, fi.ECInstanceId FROM ecdbf.FileInfo fi JOIN ecdbf.FileInfoOwnership o ON fi.ECInstanceId=o.FileInfoId AND fi.GetECClassId()=o.FileInfoECClassId WHERE o.OwnerId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKey.GetECInstanceId()));
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
    ecdb.SaveChanges();
    //DELETE scenario -> FileInfo is not implicitly deleted when the owner is deleted
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "DELETE FROM ONLY ts.Foo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT count(*) FROM ecdbf.FileInfo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, embeddedFileId));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(1, stmt.GetValueInt(0)) << "FileInfo is expected to still contain the one instance inserted";
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT count(*) FROM ecdbf.FileInfoOwnership WHERE OwnerId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(1, stmt.GetValueInt(0)) << "FileInfoOwnership is expected to still contain the ownership for the deleted Foo instance";
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(ecdb, "DELETE FROM ONLY ecdbf.EmbeddedFileInfo WHERE ECInstanceId=?")) << "Deletion not allowed for ECClasses that map to existing tables";
    stmt.Finalize();
    }


//Specific for file search in Document Root only.
BeFileName SearchTestFile (Utf8CP testFileName)
    {
    WString testFileNameW (testFileName, BentleyCharEncoding::Utf8);

    BeFileName testFilePath;
    BeTest::GetHost ().GetDocumentsRoot (testFilePath);
    testFilePath.AppendToPath (L"ECDb");
    testFilePath.AppendToPath (testFileNameW.c_str ());
    return testFilePath;
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Maha Nasir                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECDbFileInfoTests, IterateThroughEmbeddedFiles)
    {
    ECDbR ecdb = SetupECDb ("ecdbfileinfo.ecdb");
    ASSERT_TRUE (ecdb.IsDbOpen ());

    DbEmbeddedFileTable& embeddedFileTable = ecdb.EmbeddedFiles ();

        {
        //Test file 1
        BeFileName testFilePath = SearchTestFile ("StartupCompany.json");
        ASSERT_TRUE (testFilePath.DoesPathExist ());

        DbResult stat = BE_SQLITE_OK;
        DateTime expectedLastModified = DateTime::GetCurrentTimeUtc ();
        double expectedLastModifiedJd = 0.0;
        ASSERT_EQ (SUCCESS, expectedLastModified.ToJulianDay (expectedLastModifiedJd));

        //Imports the file into the db.
        BeBriefcaseBasedId embeddedFileId = embeddedFileTable.Import (&stat, "StartupCompany.json", testFilePath.GetNameUtf8 ().c_str (), "JSON", nullptr, &expectedLastModified);
        ASSERT_EQ (BE_SQLITE_OK, stat);
        ASSERT_TRUE (embeddedFileId.IsValid ());
        }

        {
        //test file 2
        BeFileName testFilePath = SearchTestFile ("CommonGeometry.json");
        ASSERT_TRUE (testFilePath.DoesPathExist ());

        //Imports the file into the db.
        DbResult stat = BE_SQLITE_OK;
        BeBriefcaseBasedId embeddedFileId = embeddedFileTable.Import (&stat, "CommonGeometry.json", testFilePath.GetNameUtf8 ().c_str (), "JSON", "Geometry");
        ASSERT_EQ (BE_SQLITE_OK, stat);
        ASSERT_TRUE (embeddedFileId.IsValid ());
        }

    DbEmbeddedFileTable::Iterator iter = embeddedFileTable.MakeIterator ();
    ASSERT_EQ (2, iter.QueryCount ());

    DbEmbeddedFileTable::Iterator::Entry file = iter.begin ();
    for (auto const& file : iter)
        {
        if (Utf8String (file.GetNameUtf8 ()) == "StartupCompany.json")
            {
            ASSERT_EQ (1, file.GetId ().GetValue());
            ASSERT_STREQ ("JSON", file.GetTypeUtf8 ());
            ASSERT_EQ (9059, file.GetFileSize ());
            ASSERT_EQ (NULL, file.GetDescriptionUtf8 ());
            ASSERT_EQ (524288, file.GetChunkSize ());
            }
        else if (Utf8String (file.GetNameUtf8 ()) == "CommonGeometry.json")
            {
            ASSERT_EQ (2, file.GetId ().GetValue());
            ASSERT_STREQ ("JSON", file.GetTypeUtf8 ());
            ASSERT_EQ (765, file.GetFileSize ());
            ASSERT_STREQ ("Geometry", file.GetDescriptionUtf8 ());
            ASSERT_EQ (524288, file.GetChunkSize ());
            }
        }
    iter.end ();
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Maha Nasir                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECDbFileInfoTests, VerifyEmbeddedFileSize)
    {
    ECDbR ecdb = SetupECDb ("ecdbfileinfo.ecdb");
    ASSERT_TRUE (ecdb.IsDbOpen ());

    DbEmbeddedFileTable& embeddedFileTable = ecdb.EmbeddedFiles ();

    //embed test file
    Utf8CP testFileName = "CommonGeometry.json";
    uint64_t size = 0;
        {
        BeFileName testFilePath = SearchTestFile (testFileName);
        ASSERT_TRUE (testFilePath.DoesPathExist ());

        //Imports the test file into the db.
        DbResult stat = BE_SQLITE_OK;
        BeBriefcaseBasedId embeddedFileId = embeddedFileTable.Import (&stat, testFileName, testFilePath.GetNameUtf8 ().c_str (), "JSON", "Geometry");
        ASSERT_EQ (BE_SQLITE_OK, stat);
        ASSERT_TRUE (embeddedFileId.IsValid ());

        //Query the values for a file.
        BeBriefcaseBasedId id = embeddedFileTable.QueryFile (testFileName, &size);
        ASSERT_TRUE (id.IsValid ());
        ASSERT_TRUE (size > 0);
        }

    //Read existing embedded file, AddEntry, Save and verify the size. 
        {
        Utf8CP newfileName = "CopyCommonGeometry.json";

        //Creates a new entry in the embedded file table with the specified name.
        ASSERT_EQ (BE_SQLITE_OK, embeddedFileTable.AddEntry (newfileName, "JSON"));
        bvector<Byte> buffer;
        ASSERT_EQ (BE_SQLITE_OK, embeddedFileTable.Read (buffer, testFileName));

        //Now save data without compression and read it again to verify that the data is unchanged.
        ASSERT_EQ (BE_SQLITE_OK, embeddedFileTable.Save (buffer.data (), size, newfileName, nullptr, false));
        bvector<Byte> newBuffer;
        ASSERT_EQ (BE_SQLITE_OK, embeddedFileTable.Read (newBuffer, newfileName));
        ASSERT_TRUE (buffer.size () == newBuffer.size ());
        ASSERT_EQ (0, memcmp (&buffer[0], &newBuffer[0], buffer.size ()));
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbFileInfoTests, FileInfoOwnershipConstraints)
    {
    ECDbR ecdb = SetupECDb("ecdbfileinfo.ecdb", SchemaItem(GetTestSchemaXml()));
    ASSERT_TRUE(ecdb.IsDbOpen());

    //Constraint: None of the properties can be null
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ecdbf.FileInfoOwnership(OwnerId, OwnerECClassId, FileInfoId, FileInfoECClassId) VALUES(?,?,?,?)"));
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
// @bsimethod                                  Krischan.Eberle                  11/15
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
        const ECClassId expectedOwnerClassId = expectedOwnerships[i].first.GetECClassId();
        const ECInstanceId expectedOwnerId = expectedOwnerships[i].first.GetECInstanceId();
        const ECClassId expectedFileInfoClassId = expectedOwnerships[i].second.GetECClassId();
        const ECInstanceId expectedFileInfoId = expectedOwnerships[i].second.GetECInstanceId();

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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT GetECClassId(), ECInstanceId FROM ecdbf.FileInfo ORDER BY GetECClassId(), ECInstanceId"));

    i = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        const ECClassId expectedFileInfoClassId = expectedFileInfos[i].GetECClassId();
        const ECInstanceId expectedFileInfoId = expectedFileInfos[i].GetECInstanceId();

        ECClassId actualFileInfoClassId = stmt.GetValueId<ECClassId>(0);
        ASSERT_EQ(expectedFileInfoClassId, actualFileInfoClassId);
        ECInstanceId actualFileInfoId = stmt.GetValueId<ECInstanceId>(1);
        ASSERT_EQ(expectedFileInfoId.GetValue(), actualFileInfoId.GetValue());

        i++;
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbFileInfoTests, Purge)
    {
    ECDbR ecdb = SetupECDb("ecdbfileinfo.ecdb", SchemaItem(GetTestSchemaXml()));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECInstanceKey fooKey;
    ECInstanceKey fooChildKey;
    ECInstanceKey gooKey;
    ECInstanceKey fooExternalFileInfoKey;
    ECInstanceKey gooExternalFileInfoKey;
    ECInstanceKey fooChildEmbeddedFileInfoKey;
    ECInstanceKey orphanEmbeddedFileInfoKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.Foo (Name) VALUES ('Foo')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(fooKey)) << ecdb.GetLastError().c_str();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.FooChild (Name, Label) VALUES ('FooChild', 'My Foo child')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(fooChildKey)) << ecdb.GetLastError().c_str();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.Goo (Name) VALUES ('Goo')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(gooKey)) << ecdb.GetLastError().c_str();
    stmt.Finalize();

    //ExternalFileInfos
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ecdbf.ExternalFileInfo(Name, RootFolder, RelativePath) VALUES(?,1,'mydocuments/private/')"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "ExternalFile1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(fooExternalFileInfoKey)) << ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "ExternalFile2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(gooExternalFileInfoKey)) << ecdb.GetLastError().c_str();
    stmt.Finalize();

    //EmbeddedFileInfos
    ECClassId embeddedFileInfoClassId = ecdb.Schemas().GetECClassId("ECDb_FileInfo", "EmbeddedFileInfo");
    ASSERT_TRUE(embeddedFileInfoClassId.IsValid());

    Utf8CP testFileName = "StartupCompany.json";

    BeFileName testFilePath;
    BeTest::GetHost().GetDocumentsRoot(testFilePath);
    testFilePath.AppendToPath(L"ECDb");
    testFilePath.AppendToPath(WString(testFileName, BentleyCharEncoding::Utf8).c_str());

    DbEmbeddedFileTable& embeddedFileTable = ecdb.EmbeddedFiles();
    DbResult stat = BE_SQLITE_OK;
    DateTime lastModified = DateTime::GetCurrentTimeUtc();
    BeBriefcaseBasedId id = embeddedFileTable.Import(&stat, testFileName, testFilePath.GetNameUtf8().c_str(), "JSON", nullptr, &lastModified);
    ASSERT_EQ(BE_SQLITE_OK, stat);
    ASSERT_TRUE(id.IsValid());
    fooChildEmbeddedFileInfoKey = ECInstanceKey(embeddedFileInfoClassId, ECInstanceId(id.GetValue()));

    testFileName = "Copy of StartupCompany.json";
    lastModified = DateTime::GetCurrentTimeUtc();
    id = embeddedFileTable.Import(&stat, testFileName, testFilePath.GetNameUtf8().c_str(), "JSON", nullptr, &lastModified);
    ASSERT_EQ(BE_SQLITE_OK, stat);
    ASSERT_TRUE(id.IsValid());
    orphanEmbeddedFileInfoKey = ECInstanceKey(embeddedFileInfoClassId, ECInstanceId(id.GetValue()));

    //Ownership
    //Foo - ExternalFile1
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ecdbf.FileInfoOwnership(OwnerId, OwnerECClassId, FileInfoId, FileInfoECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKey.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, fooKey.GetECClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, fooExternalFileInfoKey.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, fooExternalFileInfoKey.GetECClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();

    //FooChild - EmbeddedFile1
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooChildKey.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, fooChildKey.GetECClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, fooChildEmbeddedFileInfoKey.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, fooChildEmbeddedFileInfoKey.GetECClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();

    //Goo - ExternalFile2
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, gooKey.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, gooKey.GetECClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, gooExternalFileInfoKey.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, gooExternalFileInfoKey.GetECClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();

    ecdb.SaveChanges();
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

    AssertPurge(ecdb, expectedOwnerships, expectedFileInfos);

    //Scenario 1: Delete owners and purge
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("ECDbFileInfo Purge> START");
    ASSERT_EQ(SUCCESS, ecdb.Purge(ECDb::PurgeMode::FileInfoOwnerships));
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("ECDbFileInfo Purge> END");

    AssertPurge(ecdb, expectedOwnerships, expectedFileInfos);
    
    //Now delete Owner Foo
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "DELETE FROM ONLY ts.Foo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << ecdb.GetLastError().c_str();
    stmt.Finalize();
    ASSERT_EQ(SUCCESS, ecdb.Purge(ECDb::PurgeMode::FileInfoOwnerships));

    expectedOwnerships.clear();
    expectedOwnerships.push_back(std::make_pair(fooChildKey, fooChildEmbeddedFileInfoKey));
    expectedOwnerships.push_back(std::make_pair(gooKey, gooExternalFileInfoKey));

    AssertPurge(ecdb, expectedOwnerships, expectedFileInfos);

    //Now delete Owner FooChild
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "DELETE FROM ONLY ts.FooChild WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooChildKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << ecdb.GetLastError().c_str();
    stmt.Finalize();
    ASSERT_EQ(SUCCESS, ecdb.Purge(ECDb::PurgeMode::FileInfoOwnerships));

    expectedOwnerships.clear();
    expectedOwnerships.push_back(std::make_pair(gooKey, gooExternalFileInfoKey));

    AssertPurge(ecdb, expectedOwnerships, expectedFileInfos);

    //Now delete Owner Goo
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "DELETE FROM ONLY ts.Goo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, gooKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << ecdb.GetLastError().c_str();
    stmt.Finalize();
    ASSERT_EQ(SUCCESS, ecdb.Purge(ECDb::PurgeMode::FileInfoOwnerships));

    expectedOwnerships.clear();
    AssertPurge(ecdb, expectedOwnerships, expectedFileInfos);

    ASSERT_EQ(BE_SQLITE_OK, ecdb.AbandonChanges());

    //Scenario 2: Delete fileinfos and purge
    //delete EmbeddedFiles
    //Embedded files cannot be deleted via ECSQL because they map to an existing table.
    //EmbeddedFile API cannot be used either as it does not allow deleting by id.
    //Therefore use SQLite directly which is not a problem for this test.
    Statement sqliteStmt;
    ASSERT_EQ(BE_SQLITE_OK, sqliteStmt.Prepare(ecdb, "DELETE FROM " BEDB_TABLE_EmbeddedFile " WHERE Id=?"));
    ASSERT_EQ(BE_SQLITE_OK, sqliteStmt.BindId(1, fooChildEmbeddedFileInfoKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, sqliteStmt.Step());
    sqliteStmt.Finalize();

    ASSERT_EQ(SUCCESS, ecdb.Purge(ECDb::PurgeMode::FileInfoOwnerships));
    expectedOwnerships.clear();
    expectedOwnerships.push_back(std::make_pair(fooKey, fooExternalFileInfoKey));
    expectedOwnerships.push_back(std::make_pair(gooKey, gooExternalFileInfoKey));

    expectedFileInfos.clear();
    expectedFileInfos.push_back(fooExternalFileInfoKey);
    expectedFileInfos.push_back(gooExternalFileInfoKey);
    expectedFileInfos.push_back(orphanEmbeddedFileInfoKey);

    AssertPurge(ecdb, expectedOwnerships, expectedFileInfos);

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "DELETE FROM ONLY ecdbf.ExternalFileInfo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooExternalFileInfoKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(SUCCESS, ecdb.Purge(ECDb::PurgeMode::FileInfoOwnerships));

    expectedOwnerships.clear();
    expectedOwnerships.push_back(std::make_pair(gooKey, gooExternalFileInfoKey));

    expectedFileInfos.clear();
    expectedFileInfos.push_back(gooExternalFileInfoKey);
    expectedFileInfos.push_back(orphanEmbeddedFileInfoKey);

    AssertPurge(ecdb, expectedOwnerships, expectedFileInfos);

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, gooExternalFileInfoKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(SUCCESS, ecdb.Purge(ECDb::PurgeMode::FileInfoOwnerships));

    expectedOwnerships.clear();
    expectedFileInfos.clear();
    expectedFileInfos.push_back(orphanEmbeddedFileInfoKey);

    AssertPurge(ecdb, expectedOwnerships, expectedFileInfos);
    ecdb.AbandonChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP ECDbFileInfoTests::GetTestSchemaXml()
{
    return "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
        "  <ECEntityClass typeName=\"Foo\" >"
        "    <ECProperty propertyName=\"Name\" typeName=\"string\" />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName=\"FooChild\" >"
        "    <BaseClass>Foo</BaseClass>"
        "    <ECProperty propertyName=\"Label\" typeName=\"string\" />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName=\"Goo\" >"
        "    <ECProperty propertyName=\"Name\" typeName=\"string\"  />"
        "  </ECEntityClass>"
        "</ECSchema>";
}

END_ECDBUNITTESTS_NAMESPACE
