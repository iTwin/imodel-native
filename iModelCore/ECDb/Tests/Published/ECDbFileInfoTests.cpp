/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbFileInfoTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "string.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

#define ECDB_FILEINFO_SCHEMA_NAME "ECDb_FileInfo"

Utf8CP GetTestSchemaXml();

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  5/15
//+---------------+---------------+---------------+---------------+---------------+------
void deleteExistingFile (BeFileName filePath)
    {
    if (BeFileName::DoesPathExist (filePath.GetName ()))
        {
        // Delete any previously exported file
        BeFileNameStatus fileDeleteStatus = BeFileName::BeDeleteFile (filePath.GetName ());
        ASSERT_EQ (BeFileNameStatus::Success, fileDeleteStatus);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbFileInfo, EmptyECDbHasFileInfoSchema)
{
    ECDbTestProject testProject;
    auto& ecdb = testProject.Create("ecdbfileinfo.ecdb");

    auto const& schemaManager = ecdb. Schemas ();

    ECSchemaCP fileinfoSchema = schemaManager.GetECSchema (ECDB_FILEINFO_SCHEMA_NAME, false);
    ASSERT_TRUE (fileinfoSchema != nullptr) << "Empty ECDb file is expected to contain the ECDb_FileInfo ECSchema.";

    ECClassCP ecClass = schemaManager.GetECClass (ECDB_FILEINFO_SCHEMA_NAME, "FileInfo");
    ASSERT_TRUE (ecClass != nullptr);
    ecClass = schemaManager.GetECClass (ECDB_FILEINFO_SCHEMA_NAME, "ExternalFileInfo");
    ASSERT_TRUE (ecClass != nullptr);
    ecClass = schemaManager.GetECClass (ECDB_FILEINFO_SCHEMA_NAME, "EmbeddedFileInfo");
    ASSERT_TRUE (ecClass != nullptr);
    ecClass = schemaManager.GetECClass (ECDB_FILEINFO_SCHEMA_NAME, "InstanceHasFileInfo");
    ASSERT_TRUE (ecClass != nullptr);

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
TEST(ECDbFileInfo, PolymorphicQueryRightAfterCreation)
{
    ECDbTestProject testProject;
    auto& ecdb = testProject.Create("ecdbfileinfo.ecdb");

    ECSqlStatement selStmt0;
    ASSERT_EQ(ECSqlStatus::Success, selStmt0.Prepare(ecdb, "SELECT * FROM ecdbf.EmbeddedFileInfo"));

    ECSqlStatement selStmt;
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(ecdb, "SELECT * FROM ecdbf.FileInfo"));
}

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbFileInfo, ECFEmbeddedFileBackedInstanceSupport)
{
    BeFileName ecdbPath;

    {
        ECDbTestProject testProject;
        auto& ecdb = testProject.Create("ecdbfileinfo.ecdb");
        ecdbPath = BeFileName(ecdb.GetDbFileName());

        BeFileName schemaFolder;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(schemaFolder);
        schemaFolder.AppendToPath(L"ECSchemas");
        schemaFolder.AppendToPath(L"ECDb");

        auto schemaContext = ECSchemaReadContext::CreateContext();
        schemaContext->AddSchemaLocater (ecdb. GetSchemaLocater ());
        schemaContext->AddSchemaPath(schemaFolder.GetName());

        ASSERT_EQ(SUCCESS, ECDbTestUtility::ReadECSchemaFromString(schemaContext, GetTestSchemaXml()));

        ASSERT_EQ (SUCCESS, ecdb. Schemas ().ImportECSchemas (schemaContext->GetCache ()));
    }

    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbPath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));

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
    ASSERT_EQ((int)BE_SQLITE_DONE, (int)stmt.Step(fooKey));
    stmt.Finalize();

    ECClassCP embeddedFileInfoClass = ecdb. Schemas ().GetECClass (ECDB_FILEINFO_SCHEMA_NAME, "EmbeddedFileInfo");
    ASSERT_TRUE (embeddedFileInfoClass != nullptr);

    //INSERT scenario
    DbEmbeddedFileTable& embeddedFileTable = ecdb.EmbeddedFiles();
    DbResult stat = BE_SQLITE_OK;
    DateTime expectedLastModified = DateTime::GetCurrentTimeUtc();
    double expectedLastModifiedJd = 0.0;
    ASSERT_EQ(SUCCESS, expectedLastModified.ToJulianDay(expectedLastModifiedJd));

    BeRepositoryBasedId embeddedFileId = embeddedFileTable.Import(&stat, testFileName, testFilePath.GetNameUtf8().c_str(), "JSON", nullptr, &expectedLastModified);
    ASSERT_EQ(BE_SQLITE_OK, stat);
    ASSERT_TRUE(embeddedFileId.IsValid());

    //insert InstanceHasFileInfo relationship
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ecdbf.InstanceHasFileInfo (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKey.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(2, fooKey.GetECClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, embeddedFileId));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(4, embeddedFileInfoClass->GetId()));

    ASSERT_EQ((int)BE_SQLITE_DONE, (int)stmt.Step());
    stmt.Finalize();
    
    //RETRIEVE scenario
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT fi.Name, fi.LastModified, fi.ECInstanceId FROM ts.Foo f JOIN ecdbf.EmbeddedFileInfo fi USING ecdbf.InstanceHasFileInfo "
        "WHERE f.ECInstanceId = ?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKey.GetECInstanceId()));
    ASSERT_EQ((int)BE_SQLITE_ROW, (int)stmt.Step());
    Utf8CP actualFileName = stmt.GetValueText(0);
    DateTime actualLastModified = stmt.GetValueDateTime(1);
    double actualLastModifiedJd = 0.0;
    ASSERT_EQ(SUCCESS, actualLastModified.ToJulianDay(actualLastModifiedJd));
    EXPECT_DOUBLE_EQ(expectedLastModifiedJd, actualLastModifiedJd);
    ECInstanceId actualFileId = stmt.GetValueId<ECInstanceId>(2);

    ASSERT_STREQ(testFileName, actualFileName);
    ASSERT_EQ(embeddedFileId.GetValue(), actualFileId.GetValue());

    BeFileName exportFilePath;
    BeTest::GetHost ().GetOutputRoot (exportFilePath);
    exportFilePath.AppendToPath(testFileNameW.c_str());
    deleteExistingFile (exportFilePath);

    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Export(exportFilePath.GetNameUtf8().c_str(), testFileName));

    stmt.Finalize();
    ecdb.SaveChanges ();
    //DELETE scenario
    //TODO_ROWAFFECTED
    //stmt.EnableDefaultEventHandler();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "DELETE FROM ONLY ts.Foo WHERE ECInstanceId = ?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKey.GetECInstanceId()));
    ASSERT_EQ((int)BE_SQLITE_DONE, (int)stmt.Step());
    //check referential integrity
    //TODO_ROWAFFECTED
    //ASSERT_EQ(3, stmt.GetDefaultEventHandler()->GetInstancesAffectedCount()); //1 Foo, 1 EmbeddedFileInfo, 1 InstanceHasFileInfo
    stmt.Finalize();
 
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT NULL FROM ecdbf.InstanceHasFileInfo LIMIT 1"));
    ASSERT_EQ((int)BE_SQLITE_DONE, (int)stmt.Step());
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT NULL FROM ecdbf.FileInfo LIMIT 1"));
    ASSERT_EQ((int)BE_SQLITE_DONE, (int)stmt.Step());
}

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  12/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbFileInfo, ReplaceExistingEmbeddedFile)
{
    BeFileName ecdbPath;

    {
        ECDbTestProject testProject;
        auto& ecdb = testProject.Create("ecdbfileinfo.ecdb");
        ecdbPath = BeFileName(ecdb.GetDbFileName());
    }

    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbPath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));

    //test file
    //  Using a much larger file so I could check that the embedded blobs were removed from the BE_Prop table.
    Utf8CP testFileNameOld = "embeddedFile.i.idgndb";
    WString testFileNameOldW(testFileNameOld, BentleyCharEncoding::Utf8);

    BeFileName testFilePathOld;
    BeTest::GetHost().GetDocumentsRoot(testFilePathOld);
    testFilePathOld.AppendToPath(L"DgnDb");
    testFilePathOld.AppendToPath(testFileNameOldW.c_str());

    //INSERT scenario
    DbEmbeddedFileTable& embeddedFileTable = ecdb.EmbeddedFiles();
    DbResult stat = BE_SQLITE_OK;
    DateTime expectedLastModified = DateTime::GetCurrentTimeUtc();
    double expectedLastModifiedJd = 0.0;
    ASSERT_EQ(SUCCESS, expectedLastModified.ToJulianDay(expectedLastModifiedJd));

    BeRepositoryBasedId embeddedFileId = embeddedFileTable.Import(&stat, testFileNameOld, testFilePathOld.GetNameUtf8().c_str(), "i.idgndb", nullptr, &expectedLastModified);
    ASSERT_EQ(BE_SQLITE_OK, stat);
    ASSERT_TRUE(embeddedFileId.IsValid());

    BeFileName testFilePath;
    BeTest::GetHost().GetDocumentsRoot(testFilePath);
    testFilePath.AppendToPath(L"DgnDb");
    testFilePath.AppendToPath(L"ECDb");
    testFilePath.AppendToPath(testFileNameOldW.c_str());
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Replace(testFileNameOld, testFilePath.GetNameUtf8().c_str()));

    //Query to check that embedded blobs were removed form BE_Prop
    Statement stmt;
    DbResult dbr = stmt.Prepare(ecdb, "SELECT * FROM " BEDB_TABLE_Property " WHERE Id=? AND SubId>0");
    ASSERT_EQ(BE_SQLITE_OK, dbr);
    stmt.BindId(1, embeddedFileId);
    dbr = stmt.Step();
    ASSERT_EQ(BE_SQLITE_DONE, dbr);

    BeFileName exportFilePath;
    BeTest::GetHost().GetOutputRoot(exportFilePath);
    exportFilePath.AppendToPath(testFileNameOldW.c_str());
    deleteExistingFile(exportFilePath);
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Export(exportFilePath.GetNameUtf8().c_str(), testFileNameOld));
}

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  12/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbFileInfo, ReadAddNewEntrySaveEmbeddedFile)
{
    BeFileName ecdbPath;
    {
        ECDbTestProject testProject;
        auto& ecdb = testProject.Create("ecdbfileinfo.ecdb");
        ecdbPath = BeFileName(ecdb.GetDbFileName());
    }
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbPath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));

    //test file
    //  Used a fairly large file for this to verify that it correctly handles files that are larger than one blob.
    Utf8CP testFileName = "embeddedFile.i.idgndb";
    WString testFileNameW(testFileName, BentleyCharEncoding::Utf8);

    BeFileName testFilePath;
    BeTest::GetHost().GetDocumentsRoot(testFilePath);
    testFilePath.AppendToPath(L"DgnDb");
    testFilePath.AppendToPath(testFileNameW.c_str());

    //INSERT scenario
    DbEmbeddedFileTable& embeddedFileTable = ecdb.EmbeddedFiles();
    DbResult stat = BE_SQLITE_OK;
    DateTime expectedLastModified = DateTime::GetCurrentTimeUtc();
    double expectedLastModifiedJd = 0.0;
    ASSERT_EQ(SUCCESS, expectedLastModified.ToJulianDay(expectedLastModifiedJd));

    BeRepositoryBasedId embeddedFileId = embeddedFileTable.Import(&stat, testFileName, testFilePath.GetNameUtf8().c_str(), ".i.idgndb", nullptr, &expectedLastModified);
    ASSERT_EQ(BE_SQLITE_OK, stat);
    ASSERT_TRUE(embeddedFileId.IsValid());

    Utf8CP NewFileName = "Copy_EmbeddedFile.i.idgndb";
    WString NewFileNameW(NewFileName, BentleyCharEncoding::Utf8);
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.AddEntry(NewFileName, "i.idgndb"));

    uint64_t size = 0;
    ASSERT_EQ(embeddedFileId, embeddedFileTable.QueryFile(testFileName, &size));
    ASSERT_TRUE(size > 0);

    bvector<Byte> buffer;
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Read(buffer, testFileName));
    ASSERT_TRUE(size == buffer.size());
    //Save the data with compression and than read again to verify that the data is unchanged.
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Save(buffer.data(), size, NewFileName));
    bvector<Byte> buffer2;
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Read(buffer2, NewFileName));
    ASSERT_TRUE(buffer.size() == buffer2.size());
    ASSERT_EQ(0, memcmp(&buffer[0], &buffer2[0], buffer.size()));

    //Now save data without compression and read it again and read it again to verify that the data is unchanged.
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Save(buffer.data(), size, NewFileName, false));
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Read(buffer2, NewFileName));
    ASSERT_TRUE(buffer.size() == buffer2.size());
    ASSERT_EQ(0, memcmp(&buffer[0], &buffer2[0], buffer.size()));

    BeFileName exportFilePathOld;
    BeTest::GetHost().GetOutputRoot(exportFilePathOld);
    exportFilePathOld.AppendToPath(testFileNameW.c_str());
    deleteExistingFile(exportFilePathOld);

    BeFileName exportFilePath;
    BeTest::GetHost().GetOutputRoot(exportFilePath);
    exportFilePath.AppendToPath(NewFileNameW.c_str());
    deleteExistingFile(exportFilePath);

    //NewFileName now refers to a file that was embedded without compression. Verify that Export works for this file and for the original file aswell.
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Export(exportFilePathOld.GetNameUtf8().c_str(), testFileName));
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Export(exportFilePath.GetNameUtf8().c_str(), NewFileName));
}

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  12/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbFileInfo, ImportExportEmptyFile)
{
    BeFileName ecdbPath;
    {
        ECDbTestProject testProject;
        auto& ecdb = testProject.Create("ecdbfileinfo.ecdb");
        ecdbPath = BeFileName(ecdb.GetDbFileName());
    }
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbPath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));

    //test file
    Utf8CP testFileName = "EmptyFile.txt";
    BeFileName testFilePath;
    BeTest::GetHost().GetOutputRoot(testFilePath);
    testFilePath.AppendToPath(WString(testFileName, BentleyCharEncoding::Utf8).c_str());
    deleteExistingFile(testFilePath);

    BeFile testFile;
    ASSERT_EQ(BeFileStatus::Success, testFile.Create(testFilePath.c_str()));

    //INSERT scenario
    DbEmbeddedFileTable& embeddedFileTable = ecdb.EmbeddedFiles();
    DbResult stat = BE_SQLITE_OK;
    DateTime expectedLastModified = DateTime::GetCurrentTimeUtc();
    double expectedLastModifiedJd = 0.0;
    ASSERT_EQ(SUCCESS, expectedLastModified.ToJulianDay(expectedLastModifiedJd));

    BeRepositoryBasedId embeddedFileId = embeddedFileTable.Import(&stat, testFileName, testFilePath.GetNameUtf8().c_str(), "txt", nullptr, &expectedLastModified);
    ASSERT_EQ(BE_SQLITE_OK, stat);
    ASSERT_TRUE(embeddedFileId.IsValid());

    BeFileName testFileOutPath;
    BeTest::GetHost().GetOutputRoot(testFileOutPath);
    testFileOutPath.AppendToPath(L"EmptyFileOut.txt");
    deleteExistingFile(testFileOutPath);
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Export(testFileOutPath.GetNameUtf8().c_str(), testFileName));
}

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  12/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbFileInfo, EmbedFileWithInvalidPath)
{
    ECDbTestProject testProject;
    auto& ecdb = testProject.Create("ecdbfileinfo.ecdb");

    //test file
    Utf8CP testFileName = "StartupCompany.json";
    WString testFileNameW(testFileName, BentleyCharEncoding::Utf8);

    //Test File Path
    BeFileName testFilePath;
    testFilePath.AppendToPath(testFileNameW.c_str());

    //INSERT scenario
    DbEmbeddedFileTable& embeddedFileTable = ecdb.EmbeddedFiles();
    DbResult stat = BE_SQLITE_OK;
    DateTime expectedLastModified = DateTime::GetCurrentTimeUtc();
    double expectedLastModifiedJd = 0.0;
    ASSERT_EQ(SUCCESS, expectedLastModified.ToJulianDay(expectedLastModifiedJd));

    BeRepositoryBasedId embeddedFileId = embeddedFileTable.Import(&stat, testFileName, testFilePath.GetNameUtf8().c_str(), "JSON", nullptr, &expectedLastModified);
    ASSERT_EQ(BE_SQLITE_ERROR_FileNotFound, stat);
    ASSERT_FALSE(embeddedFileId.IsValid());
}

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/14
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP GetTestSchemaXml()
{
    return "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"Foo\" >"
        "    <ECProperty propertyName=\"Name\" typeName=\"string\" isDomainClass=\"True\" />"
        "  </ECClass>"
        "</ECSchema>";
}

END_ECDBUNITTESTS_NAMESPACE