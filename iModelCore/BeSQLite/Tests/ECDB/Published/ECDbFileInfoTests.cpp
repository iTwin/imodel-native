/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ECDB/Published/ECDbFileInfoTests.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "string.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

#define ECDB_FILEINFO_SCHEMA_NAME "ECDb_FileInfo"

Utf8CP GetTestSchemaXml ();

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbFileInfo, EmptyECDbHasFileInfoSchema)
    {
    ECDbTestProject testProject;
    auto& ecdb = testProject.Create ("ecdbfileinfo.ecdb");

    auto const& schemaManager = ecdb.GetEC ().GetSchemaManager ();

    ECSchemaP fileinfoSchema = nullptr;
    auto stat = schemaManager.GetECSchema (fileinfoSchema, ECDB_FILEINFO_SCHEMA_NAME, false);
    ASSERT_EQ (SUCCESS, stat) << "Empty ECDb file is expected to contain the ECDb_FileInfo ECSchema.";

    ECClassP ecClass = nullptr;
    ASSERT_EQ (SUCCESS, schemaManager.GetECClass (ecClass, ECDB_FILEINFO_SCHEMA_NAME, "FileInfo"));
    ASSERT_EQ (SUCCESS, schemaManager.GetECClass (ecClass, ECDB_FILEINFO_SCHEMA_NAME, "ExternalFileInfo"));
    ASSERT_EQ (SUCCESS, schemaManager.GetECClass (ecClass, ECDB_FILEINFO_SCHEMA_NAME, "EmbeddedFileInfo"));
    ASSERT_EQ (SUCCESS, schemaManager.GetECClass (ecClass, ECDB_FILEINFO_SCHEMA_NAME, "InstanceHasFileInfo"));

    ASSERT_TRUE (ecdb.TableExists ("be_EmbedFile")) << "Empty ECDb file is expected to contain the table 'be_EmbedFile'.";

    ECSqlStatement insertStmt;
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) insertStmt.Prepare (ecdb, "INSERT INTO ecdbf.ExternalFileInfo (Name, Size, LastModified) VALUES ('myexternalfile.pdf', 1024, DATE '2014-09-25')"));
    ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) insertStmt.Step ());

    ECSqlStatement selStmt;
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) selStmt.Prepare (ecdb, "SELECT * FROM ecdbf.ExternalFileInfo"));

    int rowCount = 0;
    while (selStmt.Step () == ECSqlStepStatus::HasRow)
        {
        rowCount++;
        }

    ASSERT_EQ (1, rowCount);

    }

    

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbFileInfo, PolymorphicQueryRightAfterCreation)
    {
    ECDbTestProject testProject;
    auto& ecdb = testProject.Create ("ecdbfileinfo.ecdb");

    ECSqlStatement selStmt0;
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) selStmt0.Prepare (ecdb, "SELECT * FROM ecdbf.EmbeddedFileInfo"));

    ECSqlStatement selStmt;
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) selStmt.Prepare (ecdb, "SELECT * FROM ecdbf.FileInfo"));
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbFileInfo, ECFEmbeddedFileBackedInstanceSupport)
    {
    BeFileName ecdbPath;

        {
        ECDbTestProject testProject;
        auto& ecdb = testProject.Create ("ecdbfileinfo.ecdb");
        ecdbPath = BeFileName (ecdb.GetDbFileName ());

        BeFileName schemaFolder;
        BeTest::GetHost ().GetDgnPlatformAssetsDirectory (schemaFolder);
        schemaFolder.AppendToPath (L"ECSchemas");
        schemaFolder.AppendToPath (L"ECDb");

        auto schemaContext = ECSchemaReadContext::CreateContext ();
        schemaContext->AddSchemaLocater (ecdb.GetEC ().GetSchemaLocater ());
        schemaContext->AddSchemaPath (schemaFolder.GetName ());

        ASSERT_EQ (SUCCESS, ECDbTestUtility::ReadECSchemaFromString (schemaContext, GetTestSchemaXml ()));

        ASSERT_EQ (SUCCESS, ecdb.GetEC ().GetSchemaManager ().ImportECSchemas (schemaContext->GetCache ()));
        }

    ECDb ecdb;
    ASSERT_EQ (BE_SQLITE_OK, ecdb.OpenBeSQLiteDb (ecdbPath, ECDb::OpenParams (Db::OPEN_ReadWrite)));

    //test file
    Utf8CP testFileName = "StartupCompany.json";
    WString testFileNameW (testFileName, BentleyCharEncoding::Utf8);
        
    BeFileName testFilePath;
    BeTest::GetHost ().GetDocumentsRoot (testFilePath);
    testFilePath.AppendToPath (L"DgnDb");
    testFilePath.AppendToPath (testFileNameW.c_str ());


    //Insert Foo instance
    ECSqlStatement stmt;
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.Prepare (ecdb, "INSERT INTO ts.Foo (Name) VALUES (?)"));
    stmt.BindText (1, "Foo1", IECSqlBinder::MakeCopy::Yes);
    ECInstanceKey fooKey;
    ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) stmt.Step (fooKey));
    stmt.Finalize ();

    ECClassP embeddedFileInfoClass = nullptr;
    ASSERT_EQ (SUCCESS, ecdb.GetEC ().GetSchemaManager ().GetECClass (embeddedFileInfoClass, ECDB_FILEINFO_SCHEMA_NAME, "EmbeddedFileInfo"));
    
    //INSERT scenario
    DbEmbeddedFileTable& embeddedFileTable = ecdb.EmbeddedFiles ();
    DbResult stat = BE_SQLITE_OK;
    DateTime expectedLastModified = DateTime::GetCurrentTimeUtc ();
    double expectedLastModifiedJd = 0.0;
    ASSERT_EQ (SUCCESS, expectedLastModified.ToJulianDay (expectedLastModifiedJd));

    BeRepositoryBasedId embeddedFileId = embeddedFileTable.Import (&stat, testFileName, testFilePath.GetNameUtf8 ().c_str (), "JSON", nullptr, &expectedLastModified);
    ASSERT_EQ (BE_SQLITE_OK, stat);
    ASSERT_TRUE (embeddedFileId.IsValid ());

    //insert InstanceHasFileInfo relationship
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.Prepare (ecdb, "INSERT INTO ecdbf.InstanceHasFileInfo (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?,?,?,?)"));
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.BindId (1, fooKey.GetECInstanceId ()));
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.BindInt64 (2, fooKey.GetECClassId ()));
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.BindId (3, embeddedFileId));
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.BindInt64 (4, embeddedFileInfoClass->GetId ()));

    ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) stmt.Step ());
    stmt.Finalize ();

    //RETRIEVE scenario
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.Prepare (ecdb, "SELECT fi.Name, fi.LastModified, fi.ECInstanceId FROM ts.Foo f JOIN ecdbf.EmbeddedFileInfo fi USING ecdbf.InstanceHasFileInfo "
                                                                      "WHERE f.ECInstanceId = ?"));
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.BindId (1, fooKey.GetECInstanceId ()));
    ASSERT_EQ ((int) ECSqlStepStatus::HasRow, (int) stmt.Step ());
    Utf8CP actualFileName = stmt.GetValueText (0);
    DateTime actualLastModified = stmt.GetValueDateTime (1);
    double actualLastModifiedJd = 0.0;
    ASSERT_EQ (SUCCESS, actualLastModified.ToJulianDay (actualLastModifiedJd));
    EXPECT_DOUBLE_EQ (expectedLastModifiedJd, actualLastModifiedJd);
    ECInstanceId actualFileId = stmt.GetValueId<ECInstanceId> (2);

    ASSERT_STREQ (testFileName, actualFileName);
    ASSERT_EQ (embeddedFileId.GetValue (), actualFileId.GetValue ());

    BeFileName exportFilePath;
    BeTest::GetHost ().GetTempDir (exportFilePath);
    exportFilePath.AppendToPath (testFileNameW.c_str ());

    ASSERT_EQ (BE_SQLITE_OK, embeddedFileTable.Export (exportFilePath.GetNameUtf8 ().c_str (), testFileName));

    stmt.Finalize ();

    //DELETE scenario
    struct EventHandler : ECSqlEventHandler 
        {
    private:
        size_t m_rowsAffected;

        virtual void _OnEvent (EventType eventType, ECSqlEventArgs const& args) override
            {
            m_rowsAffected = args.GetInstanceKeys ().size ();
            }

    public:
        EventHandler () : ECSqlEventHandler () {}

        size_t GetRowsAffected () const
            {
            return m_rowsAffected;
            }
        };

    EventHandler handler;
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.RegisterEventHandler (handler));
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.Prepare (ecdb, "DELETE FROM ONLY ts.Foo WHERE ECInstanceId = ?"));
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.BindId (1, fooKey.GetECInstanceId ()));
    ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) stmt.Step ());
    stmt.Finalize ();

    //check referential integrity
    ASSERT_EQ (3, handler.GetRowsAffected ()); //1 Foo, 1 EmbeddedFileInfo, 1 InstanceHasFileInfo

    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.Prepare (ecdb, "SELECT NULL FROM ecdbf.InstanceHasFileInfo LIMIT 1"));
    ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) stmt.Step ());
    stmt.Finalize ();
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.Prepare (ecdb, "SELECT NULL FROM ecdbf.FileInfo LIMIT 1"));
    ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) stmt.Step ());
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
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbPath, ECDb::OpenParams(Db::OPEN_ReadWrite)));

    //test file
    //  Using a much larger file so I could check that the embedded blobs were removed from the BE_Prop table.
    Utf8CP testFileNameOld = "79_Main.i.idgndb";
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
    stmt.BindId(1,embeddedFileId);
    dbr = stmt.Step();
    ASSERT_EQ(BE_SQLITE_DONE, dbr);

    BeFileName exportFilePath;
    BeTest::GetHost().GetTempDir(exportFilePath);
    exportFilePath.AppendToPath(testFileNameOldW.c_str());

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
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbPath, ECDb::OpenParams(Db::OPEN_ReadWrite)));

    //test file
    //  Used a fairly large file for this to verify that it correctly handles files that are larger than one blob.
    Utf8CP testFileName = "79_Main.i.idgndb";
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

    Utf8CP NewFileName = "CopyOf79_Main.i.idgndb";
    WString NewFileNameW(NewFileName, BentleyCharEncoding::Utf8);
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.AddEntry(NewFileName, "i.idgndb"));

    UInt64 size = 0;
    ASSERT_EQ(embeddedFileId, embeddedFileTable.QueryFile(testFileName, &size));
    ASSERT_TRUE(size>0);

    bvector<byte> buffer;
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Read(buffer, testFileName));
    ASSERT_TRUE(size == buffer.size());
    //Save the data with compression and than read again to verify that the data is unchanged.
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Save(buffer.data(), size, NewFileName));
    bvector<byte> buffer2;
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Read(buffer2, NewFileName));
    ASSERT_TRUE(buffer.size() == buffer2.size());
    ASSERT_EQ(0, memcmp(&buffer[0], &buffer2[0], buffer.size()));

    //Now save data without compression and read it again and read it again to verify that the data is unchanged.
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Save(buffer.data(), size, NewFileName, false));
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Read(buffer2, NewFileName));
    ASSERT_TRUE(buffer.size() == buffer2.size());
    ASSERT_EQ(0, memcmp(&buffer[0], &buffer2[0], buffer.size()));

    BeFileName exportFilePathOld;
    BeTest::GetHost().GetTempDir(exportFilePathOld);
    exportFilePathOld.AppendToPath(testFileNameW.c_str());
    BeFileName exportFilePath;
    BeTest::GetHost().GetTempDir(exportFilePath);
    exportFilePath.AppendToPath(NewFileNameW.c_str());

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
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbPath, ECDb::OpenParams(Db::OPEN_ReadWrite)));

    //test file
    Utf8CP testFileName = "EmptyFile.txt";
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

    BeRepositoryBasedId embeddedFileId = embeddedFileTable.Import(&stat, testFileName, testFilePath.GetNameUtf8().c_str(), "txt", nullptr, &expectedLastModified);
    ASSERT_EQ(BE_SQLITE_OK, stat);
    ASSERT_TRUE(embeddedFileId.IsValid());

    BeFileName exportFilePath;
    BeTest::GetHost().GetTempDir(exportFilePath);
    exportFilePath.AppendToPath(testFileNameW.c_str());

    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Export(exportFilePath.GetNameUtf8().c_str(), testFileName));
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
Utf8CP GetTestSchemaXml ()
    {
    return "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"Foo\" >"
        "    <ECProperty propertyName=\"Name\" typeName=\"string\" isDomainClass=\"True\" />"
        "  </ECClass>"
        "</ECSchema>";
    }

END_ECDBUNITTESTS_NAMESPACE