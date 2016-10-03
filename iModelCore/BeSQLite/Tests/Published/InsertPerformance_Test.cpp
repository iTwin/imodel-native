/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/InsertPerformance_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BeSQLitePublishedTests.h"

#define TABLE_IntBlob           "test_IntBlob"
#define TABLE_IntBlobShared16   "test_IntBlobShared16"
#define TABLE_IntInt            "test_IntInt"
#define TABLE_IntIntShared16    "test_IntIntShared16"
#define TABLE_Blob              "test_Blob"
#define TABLE_BlobShared16      "test_BlobShared16"
#define TABLE_Text              "test_Text"
#define TABLE_TextShared16      "test_TextShared16"

#define DDL_SharedColumns16     ", [sc01], [sc02], [sc03], [sc04], [sc05], [sc06], [sc07], [sc08], [sc09], [sc10], [sc11], [sc12], [sc13], [sc14], [sc15], [sc16]"
#define DDL_ExtraColumns        ", [X] DOUBLE, [Y] DOUBLE, [Z] DOUBLE, [A] TEXT COLLATE NOCASE, [B] TEXT COLLATE NOCASE, [C] TEXT COLLATE NOCASE, [I] INTEGER, [J] INTEGER, [K] INTEGER"
#define INTO_ExtraColumns       ",[X],[Y],[Z],[A],[B],[C],[I],[J],[K]"
#define VALUES_ExtraColumns     ",1.1,2.2,3.3,'AAAAAAAAAA','BBBBBBBBBB','CCCCCCCCCC',1,2,3"

//=======================================================================================
// @bsiclass                                    Shaun.Sewall                    09/2016
//=======================================================================================
struct InsertPerformanceTests : public ::testing::Test
{
private:
    Db m_db;
    Statement m_statement;
    StopWatch m_timer;
    static bmap<Utf8String, uint64_t> s_fileSizeMap;

public:
    static void SetUpTestCase();
    static void TearDownTestCase();

    void SetUpDb(Utf8CP dbName);
    int GetNumInserts() const {return 500 * 1000;}
    void StartTest() {m_timer.Start();}
    void StopTest(Utf8CP testName);

    void CreateTableIntBlob();
    void CreateTableIntBlobShared16();
    void CreateTableIntInt();
    void CreateTableIntIntShared16();
    void CreateTableBlob();
    void CreateTableBlobShared16();
    void CreateTableText();
    void CreateTableTextShared16();

    void InsertRow(int64_t id, BeGuidCR guid, Utf8CP label);
    void InsertRow(int64_t id, int64_t id2, Utf8CP label);
    void InsertRow(BeGuidCR guid, Utf8CP label);
    void InsertRow(Utf8CP guid, Utf8CP label);

    int GetParameterIndexId() {return m_statement.GetParameterIndex(":Id");}
    int GetParameterIndexId2() {return m_statement.GetParameterIndex(":Id2");}
    int GetParameterIndexGuid() {return m_statement.GetParameterIndex(":Guid");}
    int GetParameterIndexLabel() {return m_statement.GetParameterIndex(":Label");}

    int64_t GenerateSnowflakeId(int);
    int64_t GenerateRandomId() {int64_t id; BeSQLiteLib::Randomness(sizeof(id), &id); return id;}
    BeBriefcaseBasedId GenerateBriefcaseBasedId(int i) {return BeBriefcaseBasedId(BeBriefcaseId(i%10+2), i);}
    Utf8String GenerateLabel(Utf8CP prefix, int i) {return Utf8PrintfString("%d", i);} // don't include prefix for now so file sizes are comparable
};

bmap<Utf8String, uint64_t> InsertPerformanceTests::s_fileSizeMap;

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::SetUpTestCase()
    {
#if defined (NDEBUG)
    LOGTODB("Optimized Build", "", 0);
#else
    LOGTODB("DEBUG Build!!!", "", 0);
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::TearDownTestCase()
    {
    LOGTODB("", "", 0);
    for (auto entry : s_fileSizeMap)
        LOGTODB("InsertPerformanceTests", entry.first.c_str(), 0, "FileSize:", (int)entry.second);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::SetUpDb(Utf8CP dbName)
    {
    BeFileName outputRootDir;
    BeTest::GetHost().GetOutputRoot(outputRootDir);
    BeSQLiteLib::Initialize(outputRootDir);

    WString dbNameW;
    dbNameW.AssignUtf8(dbName);

    BeFileName dbFileName(outputRootDir);
    dbFileName.AppendToPath(dbNameW.c_str());
    dbFileName.AppendExtension(L"db3");

    if (BeFileName::DoesPathExist(dbFileName))
        BeFileName::BeDeleteFile(dbFileName);

    DbResult result = m_db.CreateNewDb(dbFileName.GetNameUtf8().c_str());
    ASSERT_EQ(BE_SQLITE_OK, result) << "Db Creation failed";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::StopTest(Utf8CP testName)
    {
    if (m_db.IsDbOpen())
        m_db.SaveChanges(testName);

    m_timer.Stop();
    LOGTODB("InsertPerformanceTests", testName, m_timer.GetElapsedSeconds(), "", GetNumInserts());

    if (m_db.IsDbOpen())
        {
        uint64_t fileSize;
        BeFileName dbFileName(m_db.GetDbFileName(), BentleyCharEncoding::Utf8);
        ASSERT_EQ(BeFileNameStatus::Success, dbFileName.GetFileSize(fileSize));
        s_fileSizeMap[testName] = fileSize;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::CreateTableIntBlob()
    {
    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable(TABLE_IntBlob, "[Id] INTEGER PRIMARY KEY, [Guid] BLOB UNIQUE, [Label] TEXT COLLATE NOCASE" DDL_ExtraColumns));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Prepare(m_db, "INSERT INTO " TABLE_IntBlob " ([Id],[Guid],[Label]" INTO_ExtraColumns ") VALUES (:Id,:Guid,:Label" VALUES_ExtraColumns ")"));
    m_db.SaveChanges("CreateTableIntBlob");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::CreateTableIntBlobShared16()
    {
    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable(TABLE_IntBlobShared16, "[Id] INTEGER PRIMARY KEY, [Guid] BLOB UNIQUE, [Label] TEXT COLLATE NOCASE" DDL_ExtraColumns DDL_SharedColumns16));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Prepare(m_db, "INSERT INTO " TABLE_IntBlobShared16 " ([Id],[Guid],[Label]" INTO_ExtraColumns ") VALUES (:Id,:Guid,:Label" VALUES_ExtraColumns ")"));
    m_db.SaveChanges("CreateTableIntBlobShared16");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::CreateTableIntInt()
    {
    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable(TABLE_IntInt, "[Id] INTEGER PRIMARY KEY, [Id2] INTEGER, [Label] TEXT COLLATE NOCASE" DDL_ExtraColumns));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Prepare(m_db, "INSERT INTO " TABLE_IntInt " ([Id],[Id2],[Label]" INTO_ExtraColumns ") VALUES (:Id,:Id2,:Label" VALUES_ExtraColumns ")"));
    m_db.SaveChanges("CreateTableIntInt");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::CreateTableIntIntShared16()
    {
    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable(TABLE_IntIntShared16, "[Id] INTEGER PRIMARY KEY, [Id2] INTEGER, [Label] TEXT COLLATE NOCASE" DDL_ExtraColumns DDL_SharedColumns16));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Prepare(m_db, "INSERT INTO " TABLE_IntIntShared16 " ([Id],[Id2],[Label]" INTO_ExtraColumns ") VALUES (:Id,:Id2,:Label" VALUES_ExtraColumns ")"));
    m_db.SaveChanges("CreateTableIntIntShared16");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::CreateTableBlob()
    {
    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable(TABLE_Blob, "[Guid] BLOB UNIQUE PRIMARY KEY, [Label] TEXT COLLATE NOCASE" DDL_ExtraColumns));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Prepare(m_db, "INSERT INTO " TABLE_Blob " ([Guid],[Label]" INTO_ExtraColumns ") VALUES (:Guid,:Label" VALUES_ExtraColumns ")"));
    m_db.SaveChanges("CreateTableBlob");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::CreateTableBlobShared16()
    {
    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable(TABLE_BlobShared16, "[Guid] BLOB UNIQUE PRIMARY KEY, [Label] TEXT COLLATE NOCASE" DDL_ExtraColumns DDL_SharedColumns16));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Prepare(m_db, "INSERT INTO " TABLE_BlobShared16 " ([Guid],[Label]" INTO_ExtraColumns ") VALUES (:Guid,:Label" VALUES_ExtraColumns ")"));
    m_db.SaveChanges("CreateTableBlobShared16");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::CreateTableText()
    {
    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable(TABLE_Text, "[Guid] TEXT UNIQUE PRIMARY KEY, [Label] TEXT COLLATE NOCASE" DDL_ExtraColumns));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Prepare(m_db, "INSERT INTO " TABLE_Text " ([Guid],[Label]" INTO_ExtraColumns ") VALUES (:Guid,:Label" VALUES_ExtraColumns ")"));
    m_db.SaveChanges("CreateTableText");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::CreateTableTextShared16()
    {
    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable(TABLE_TextShared16, "[Guid] TEXT UNIQUE PRIMARY KEY, [Label] TEXT COLLATE NOCASE" DDL_ExtraColumns DDL_SharedColumns16));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Prepare(m_db, "INSERT INTO " TABLE_TextShared16 " ([Guid],[Label]" INTO_ExtraColumns ") VALUES (:Guid,:Label" VALUES_ExtraColumns ")"));
    m_db.SaveChanges("CreateTableTextShared16");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::InsertRow(int64_t id, BeGuidCR guid, Utf8CP label)
    {
    static int parameterIndexId;
    static int parameterIndexGuid;
    static int parameterIndexLabel;

    if (0 == parameterIndexId) parameterIndexId = GetParameterIndexId();
    if (0 == parameterIndexGuid) parameterIndexGuid = GetParameterIndexGuid();
    if (0 == parameterIndexLabel) parameterIndexLabel = GetParameterIndexLabel();

    ASSERT_EQ(BE_SQLITE_OK, m_statement.BindInt64(parameterIndexId, id));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.BindGuid(parameterIndexGuid, guid));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.BindText(parameterIndexLabel, label, Statement::MakeCopy::No));

    DbResult result = m_statement.Step();
    ASSERT_EQ(BE_SQLITE_DONE, result);
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Reset());
    ASSERT_EQ(BE_SQLITE_OK, m_statement.ClearBindings());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::InsertRow(int64_t id, int64_t id2, Utf8CP label)
    {
    static int parameterIndexId;
    static int parameterIndexId2;
    static int parameterIndexLabel;

    if (0 == parameterIndexId) parameterIndexId = GetParameterIndexId();
    if (0 == parameterIndexId2) parameterIndexId2 = GetParameterIndexId2();
    if (0 == parameterIndexLabel) parameterIndexLabel = GetParameterIndexLabel();

    ASSERT_EQ(BE_SQLITE_OK, m_statement.BindInt64(parameterIndexId, id));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.BindInt64(parameterIndexId2, id2));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.BindText(parameterIndexLabel, label, Statement::MakeCopy::No));

    DbResult result = m_statement.Step();
    ASSERT_EQ(BE_SQLITE_DONE, result);
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Reset());
    ASSERT_EQ(BE_SQLITE_OK, m_statement.ClearBindings());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::InsertRow(BeGuidCR guid, Utf8CP label)
    {
    static int parameterIndexGuid;
    static int parameterIndexLabel;

    if (0 == parameterIndexGuid) parameterIndexGuid = GetParameterIndexGuid();
    if (0 == parameterIndexLabel) parameterIndexLabel = GetParameterIndexLabel();

    ASSERT_EQ(BE_SQLITE_OK, m_statement.BindGuid(parameterIndexGuid, guid));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.BindText(parameterIndexLabel, label, Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, m_statement.Step());
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Reset());
    ASSERT_EQ(BE_SQLITE_OK, m_statement.ClearBindings());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::InsertRow(Utf8CP guid, Utf8CP label)
    {
    static int parameterIndexGuid;
    static int parameterIndexLabel;

    if (0 == parameterIndexGuid) parameterIndexGuid = GetParameterIndexGuid();
    if (0 == parameterIndexLabel) parameterIndexLabel = GetParameterIndexLabel();

    ASSERT_EQ(BE_SQLITE_OK, m_statement.BindText(parameterIndexGuid, guid, Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.BindText(parameterIndexLabel, label, Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, m_statement.Step());
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Reset());
    ASSERT_EQ(BE_SQLITE_OK, m_statement.ClearBindings());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
int64_t InsertPerformanceTests::GenerateSnowflakeId(int counter)
    {
    int64_t part1 = BeTimeUtilities::QueryMillisecondsCounter() << 12;
    int64_t part2 = counter & 0xFFF;
    return part1 + part2;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlob_SequentialId_NoGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlob();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, BeGuid(), GenerateLabel(TEST_NAME, i).c_str());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlob_SequentialId_QuarterGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlob();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, BeGuid(0 == i%4 ? true : false), GenerateLabel(TEST_NAME, i).c_str());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlob_SequentialId_AllGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlob();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, BeGuid(true), GenerateLabel(TEST_NAME, i).c_str());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlob_RandomId_NoGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlob();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(GenerateRandomId(), BeGuid(), GenerateLabel(TEST_NAME, i).c_str());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlob_RandomId_QuarterGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlob();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(GenerateRandomId(), BeGuid(0 == i%4 ? true : false), GenerateLabel(TEST_NAME, i).c_str());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlob_RandomId_AllGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlob();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(GenerateRandomId(), BeGuid(true), GenerateLabel(TEST_NAME, i).c_str());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlob_AlternatingBriefcaseId_NoGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlob();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(GenerateBriefcaseBasedId(i).GetValue(), BeGuid(), GenerateLabel(TEST_NAME, i).c_str());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlob_AlternatingBriefcaseId_QuarterGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlob();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(GenerateBriefcaseBasedId(i).GetValue(), BeGuid(0 == i%4 ? true : false), GenerateLabel(TEST_NAME, i).c_str());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlob_AlternatingBriefcaseId_AllGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlob();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(GenerateBriefcaseBasedId(i).GetValue(), BeGuid(true), GenerateLabel(TEST_NAME, i).c_str());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlobShared16_AlternatingBriefcaseId_NoGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlobShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(GenerateBriefcaseBasedId(i).GetValue(), BeGuid(), GenerateLabel(TEST_NAME, i).c_str());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlobShared16_AlternatingBriefcaseId_QuarterGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlobShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(GenerateBriefcaseBasedId(i).GetValue(), BeGuid(0 == i%4 ? true : false), GenerateLabel(TEST_NAME, i).c_str());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlobShared16_AlternatingBriefcaseId_AllGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlobShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(GenerateBriefcaseBasedId(i).GetValue(), BeGuid(true), GenerateLabel(TEST_NAME, i).c_str());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlobShared16_SnowflakeId_NoGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlobShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(GenerateSnowflakeId(i), BeGuid(), GenerateLabel(TEST_NAME, i).c_str());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlobShared16_SnowflakeId_QuarterGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlobShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(GenerateSnowflakeId(i), BeGuid(0 == i%4 ? true : false), GenerateLabel(TEST_NAME, i).c_str());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlobShared16_SnowflakeId_AllGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlobShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(GenerateSnowflakeId(i), BeGuid(true), GenerateLabel(TEST_NAME, i).c_str());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntInt_SnowflakeId_AllInternalGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntInt();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(GenerateSnowflakeId(i), GenerateRandomId(), GenerateLabel(TEST_NAME, i).c_str());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntIntShared16_SnowflakeId_AllInternalGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntIntShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(GenerateSnowflakeId(i), GenerateRandomId(), GenerateLabel(TEST_NAME, i).c_str());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntIntShared16_SnowflakeId_QuarterExternalGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntIntShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        {
        if (0 == i%4)
            {
            BeGuid externalGuid(true); // simulate an external guid being supplied
            InsertRow(externalGuid.m_guid.u[0], externalGuid.m_guid.u[1], GenerateLabel(TEST_NAME, i).c_str());
            }
        else
            {
            InsertRow(GenerateSnowflakeId(i), GenerateRandomId(), GenerateLabel(TEST_NAME, i).c_str());
            }
        }

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntIntShared16_SnowflakeId_AllExternalGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntIntShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        {
        BeGuid externalGuid(true); // simulate an external guid being supplied
        InsertRow(externalGuid.m_guid.u[0], externalGuid.m_guid.u[1], GenerateLabel(TEST_NAME, i).c_str());
        }

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, Blob_AllGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableBlob();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(BeGuid(true), GenerateLabel(TEST_NAME, i).c_str());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, BlobShared16_AllGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableBlobShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(BeGuid(true), GenerateLabel(TEST_NAME, i).c_str());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, Text_AllGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableText();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(BeGuid(true).ToString().c_str(), GenerateLabel(TEST_NAME, i).c_str());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, TextShared16_AllGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableTextShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(BeGuid(true).ToString().c_str(), GenerateLabel(TEST_NAME, i).c_str());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, BeGuidCreate)
    {
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        {
        BeGuid guid(true);
        UNUSED_VARIABLE(guid);
        }

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, BeGuidCreateAndToString)
    {
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        {
        Utf8String guid = BeGuid(true).ToString();
        UNUSED_VARIABLE(guid);
        }

    StopTest(TEST_NAME);
    }
