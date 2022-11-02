/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#ifdef RUN_PERFORMANCE_TESTS

#include "BeSQLiteNonPublishedTests.h"

#define TABLE_IntBlob                   "test_IntBlob"
#define TABLE_IntBlobShared16           "test_IntBlobShared16"
#define TABLE_IntBlobNoIndexShared16    "test_IntBlobShared16"
#define TABLE_IntIntShared16            "test_IntIntShared16"
#define TABLE_IntIntUniqueShared16      "test_IntIntUniqueShared16"
#define TABLE_IntIntBlobShared16        "test_IntIntBlobShared16"
#define TABLE_BlobShared16              "test_BlobShared16"
#define TABLE_TextShared16              "test_TextShared16"

#define DDL_SharedColumns16     ", [sc01], [sc02], [sc03], [sc04], [sc05], [sc06], [sc07], [sc08], [sc09], [sc10], [sc11], [sc12], [sc13], [sc14], [sc15], [sc16]"
#define DDL_ExtraColumns        ", [X] DOUBLE, [Y] DOUBLE, [Z] DOUBLE, [A] TEXT COLLATE NOCASE, [B] TEXT COLLATE NOCASE, [C] TEXT COLLATE NOCASE, [I] INTEGER, [J] INTEGER, [K] INTEGER"
#define INTO_ExtraColumns       ",[X],[Y],[Z],[A],[B],[C],[I],[J],[K]"
#define VALUES_ExtraColumns     ",1.1,2.2,3.3,'AAAAAAAAAA','BBBBBBBBBB','CCCCCCCCCC',1,2,3"

//=======================================================================================
// @bsiclass
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

    static int GetNumInserts() {return 500 * 1000;}
    static int GetNumInsertsPerCommit() {return 500 * 1000;}

    void SetUpDb(Utf8CP dbName);
    void StartTest() {m_timer.Start();}
    void StopTest(Utf8CP testName);

    void CreateTableIntBlob();
    void CreateTableIntBlobShared16();
    void CreateTableIntBlobNoIndexShared16();
    void CreateTableIntIntShared16();
    void CreateTableIntIntUniqueShared16();
    void CreateTableIntIntBlobShared16();
    void CreateTableBlobShared16();
    void CreateTableTextShared16();
    void CreateUniqueIndex(Utf8CP tableName, Utf8CP columnName, Utf8CP whereClause=nullptr);

    void InsertRow(int counter, int64_t id, BeGuidCR guid);
    void InsertRow(int counter, int64_t id, int64_t id2);
    void InsertRow(int counter, int64_t id, int64_t id2, BeGuidCR guid);
    void InsertRow(int counter, BeGuidCR guid);
    void InsertRow(int counter, Utf8CP guid);
    void CommitAtInsertInterval(int counter, Utf8CP message) {if (0 == counter%GetNumInsertsPerCommit()) m_db.SaveChanges(message);}

    int GetParameterIndexId() {return m_statement.GetParameterIndex(":Id");}
    int GetParameterIndexId2() {return m_statement.GetParameterIndex(":Id2");}
    int GetParameterIndexGuid() {return m_statement.GetParameterIndex(":Guid");}
    int GetParameterIndexLabel() {return m_statement.GetParameterIndex(":Label");}

    int64_t GenerateTimeBasedId(int);
    int64_t GenerateRandomId() {int64_t id; BeSQLiteLib::Randomness(sizeof(id), &id); return id;}
    BeBriefcaseBasedId GenerateBriefcaseBasedId(int i) {return BeBriefcaseBasedId(BeBriefcaseId((i/100)%10+2), i);}
    Utf8String GenerateLabel(int i) {return Utf8PrintfString("%d", i);}
};

bmap<Utf8String, uint64_t> InsertPerformanceTests::s_fileSizeMap;

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::SetUpTestCase()
    {
#if defined (NDEBUG)
    LOGTODB("Optimized Build", "", 0);
#else
    LOGTODB("DEBUG Build!!!", "", 0);
#endif
    LOGTODB(Utf8PrintfString("NumInsertsPerCommit=%d", GetNumInsertsPerCommit()).c_str(), "", 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::TearDownTestCase()
    {
    LOGTODB("", "", 0);
    for (auto entry : s_fileSizeMap)
        LOGTODB("InsertPerformanceTests", entry.first.c_str(), 0, (int) entry.second, "FileSize:");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::StopTest(Utf8CP testName)
    {
    m_timer.Stop();
    LOGTODB("InsertPerformanceTests", testName, m_timer.GetElapsedSeconds(), GetNumInserts());

    if (m_db.IsDbOpen())
        {
        uint64_t fileSize;
        BeFileName dbFileName(m_db.GetDbFileName(), BentleyCharEncoding::Utf8);
        ASSERT_EQ(BeFileNameStatus::Success, dbFileName.GetFileSize(fileSize));
        s_fileSizeMap[testName] = fileSize;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::CreateTableIntBlob()
    {
    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable(TABLE_IntBlob, "[Id] INTEGER PRIMARY KEY, [Guid] BLOB UNIQUE, [Label] TEXT COLLATE NOCASE" DDL_ExtraColumns));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Prepare(m_db, "INSERT INTO " TABLE_IntBlob " ([Id],[Guid],[Label]" INTO_ExtraColumns ") VALUES (:Id,:Guid,:Label" VALUES_ExtraColumns ")"));
    m_db.SaveChanges("CreateTableIntBlob");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::CreateTableIntBlobShared16()
    {
    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable(TABLE_IntBlobShared16, "[Id] INTEGER PRIMARY KEY, [Guid] BLOB UNIQUE, [Label] TEXT COLLATE NOCASE" DDL_ExtraColumns DDL_SharedColumns16));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Prepare(m_db, "INSERT INTO " TABLE_IntBlobShared16 " ([Id],[Guid],[Label]" INTO_ExtraColumns ") VALUES (:Id,:Guid,:Label" VALUES_ExtraColumns ")"));
    m_db.SaveChanges("CreateTableIntBlobShared16");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::CreateTableIntBlobNoIndexShared16()
    {
    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable(TABLE_IntBlobNoIndexShared16, "[Id] INTEGER PRIMARY KEY, [Guid] BLOB, [Label] TEXT COLLATE NOCASE" DDL_ExtraColumns DDL_SharedColumns16));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Prepare(m_db, "INSERT INTO " TABLE_IntBlobNoIndexShared16 " ([Id],[Guid],[Label]" INTO_ExtraColumns ") VALUES (:Id,:Guid,:Label" VALUES_ExtraColumns ")"));
    m_db.SaveChanges("CreateTableIntBlobNoIndexShared16");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::CreateTableIntIntShared16()
    {
    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable(TABLE_IntIntShared16, "[Id] INTEGER PRIMARY KEY, [Id2] INTEGER, [Label] TEXT COLLATE NOCASE" DDL_ExtraColumns DDL_SharedColumns16));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Prepare(m_db, "INSERT INTO " TABLE_IntIntShared16 " ([Id],[Id2],[Label]" INTO_ExtraColumns ") VALUES (:Id,:Id2,:Label" VALUES_ExtraColumns ")"));
    m_db.SaveChanges("CreateTableIntIntShared16");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::CreateTableIntIntUniqueShared16()
    {
    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable(TABLE_IntIntUniqueShared16, "[Id] INTEGER, [Id2] INTEGER, [Label] TEXT COLLATE NOCASE" DDL_ExtraColumns DDL_SharedColumns16 ", PRIMARY KEY ([Id],[Id2])"));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Prepare(m_db, "INSERT INTO " TABLE_IntIntUniqueShared16 " ([Id],[Id2],[Label]" INTO_ExtraColumns ") VALUES (:Id,:Id2,:Label" VALUES_ExtraColumns ")"));
    m_db.SaveChanges("CreateTableIntIntUniqueShared16");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::CreateTableIntIntBlobShared16()
    {
    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable(TABLE_IntIntBlobShared16, "[Id] INTEGER PRIMARY KEY, [Id2] INTEGER, [Guid] BLOB UNIQUE, [Label] TEXT COLLATE NOCASE" DDL_ExtraColumns DDL_SharedColumns16));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Prepare(m_db, "INSERT INTO " TABLE_IntIntBlobShared16 " ([Id],[Id2],[Guid],[Label]" INTO_ExtraColumns ") VALUES (:Id,:Id2,:Guid,:Label" VALUES_ExtraColumns ")"));
    m_db.SaveChanges("CreateTableIntIntBlobShared16");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::CreateTableBlobShared16()
    {
    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable(TABLE_BlobShared16, "[Guid] BLOB UNIQUE PRIMARY KEY, [Label] TEXT COLLATE NOCASE" DDL_ExtraColumns DDL_SharedColumns16));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Prepare(m_db, "INSERT INTO " TABLE_BlobShared16 " ([Guid],[Label]" INTO_ExtraColumns ") VALUES (:Guid,:Label" VALUES_ExtraColumns ")"));
    m_db.SaveChanges("CreateTableBlobShared16");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::CreateTableTextShared16()
    {
    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable(TABLE_TextShared16, "[Guid] TEXT UNIQUE PRIMARY KEY, [Label] TEXT COLLATE NOCASE" DDL_ExtraColumns DDL_SharedColumns16));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Prepare(m_db, "INSERT INTO " TABLE_TextShared16 " ([Guid],[Label]" INTO_ExtraColumns ") VALUES (:Guid,:Label" VALUES_ExtraColumns ")"));
    m_db.SaveChanges("CreateTableTextShared16");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::CreateUniqueIndex(Utf8CP tableName, Utf8CP columnName, Utf8CP whereClause)
    {
    Utf8PrintfString sql("CREATE UNIQUE INDEX ix_%s_%s ON %s (%s) ", tableName, columnName, tableName, columnName);
    if (whereClause)
        sql.append(whereClause);

    ASSERT_EQ(BE_SQLITE_OK, m_db.ExecuteSql(sql.c_str()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::InsertRow(int counter, int64_t id, BeGuidCR guid)
    {
    Utf8String label = GenerateLabel(counter);
    ASSERT_EQ(BE_SQLITE_OK, m_statement.BindInt64(GetParameterIndexId(), id));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.BindGuid(GetParameterIndexGuid(), guid));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.BindText(GetParameterIndexLabel(), label, Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, m_statement.Step());
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Reset());
    ASSERT_EQ(BE_SQLITE_OK, m_statement.ClearBindings());
    CommitAtInsertInterval(counter, label.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::InsertRow(int counter, int64_t id, int64_t id2)
    {
    Utf8String label = GenerateLabel(counter);
    ASSERT_EQ(BE_SQLITE_OK, m_statement.BindInt64(GetParameterIndexId(), id));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.BindInt64(GetParameterIndexId2(), id2));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.BindText(GetParameterIndexLabel(), label, Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, m_statement.Step());
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Reset());
    ASSERT_EQ(BE_SQLITE_OK, m_statement.ClearBindings());
    CommitAtInsertInterval(counter, label.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::InsertRow(int counter, int64_t id, int64_t id2, BeGuidCR guid)
    {
    Utf8String label = GenerateLabel(counter);
    ASSERT_EQ(BE_SQLITE_OK, m_statement.BindInt64(GetParameterIndexId(), id));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.BindGuid(GetParameterIndexGuid(), guid));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.BindText(GetParameterIndexLabel(), label, Statement::MakeCopy::No));

    if (0 == id2)
        m_statement.BindNull(GetParameterIndexId2());
    else
        m_statement.BindInt64(GetParameterIndexId2(), id2);

    ASSERT_EQ(BE_SQLITE_DONE, m_statement.Step());
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Reset());
    ASSERT_EQ(BE_SQLITE_OK, m_statement.ClearBindings());
    CommitAtInsertInterval(counter, label.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::InsertRow(int counter, BeGuidCR guid)
    {
    Utf8String label = GenerateLabel(counter);
    ASSERT_EQ(BE_SQLITE_OK, m_statement.BindGuid(GetParameterIndexGuid(), guid));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.BindText(GetParameterIndexLabel(), label, Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, m_statement.Step());
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Reset());
    ASSERT_EQ(BE_SQLITE_OK, m_statement.ClearBindings());
    CommitAtInsertInterval(counter, label.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::InsertRow(int counter, Utf8CP guid)
    {
    Utf8String label = GenerateLabel(counter);
    ASSERT_EQ(BE_SQLITE_OK, m_statement.BindText(GetParameterIndexGuid(), guid, Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.BindText(GetParameterIndexLabel(), label, Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, m_statement.Step());
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Reset());
    ASSERT_EQ(BE_SQLITE_OK, m_statement.ClearBindings());
    CommitAtInsertInterval(counter, label.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int64_t InsertPerformanceTests::GenerateTimeBasedId(int counter)
    {
    int64_t part1 = BeTimeUtilities::QueryMillisecondsCounter() << 12;
    int64_t part2 = counter & 0xFFF;
    return part1 + part2;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlob_AlternatingBriefcaseId_NoGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlob();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, GenerateBriefcaseBasedId(i).GetValue(), BeGuid());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlob_AlternatingBriefcaseId_QuarterGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlob();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, GenerateBriefcaseBasedId(i).GetValue(), BeGuid(0 == i%4 ? true : false));

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlob_AlternatingBriefcaseId_AllGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlob();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, GenerateBriefcaseBasedId(i).GetValue(), BeGuid(true));

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlobShared16_AlternatingBriefcaseId_NoGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlobShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, GenerateBriefcaseBasedId(i).GetValue(), BeGuid());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlobShared16_AlternatingBriefcaseId_QuarterGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlobShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, GenerateBriefcaseBasedId(i).GetValue(), BeGuid(0 == i%4 ? true : false));

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlobShared16_AlternatingBriefcaseId_AllGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlobShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, GenerateBriefcaseBasedId(i).GetValue(), BeGuid(true));

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlobShared16_SequentialId_NoGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlobShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, BeGuid());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlobShared16_SequentialId_QuarterGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlobShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, BeGuid(0 == i%4 ? true : false));

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlobShared16_SequentialId_AllGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlobShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, BeGuid(true));

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlobShared16_RandomId_NoGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlobShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, GenerateRandomId(), BeGuid());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlobShared16_RandomId_QuarterGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlobShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, GenerateRandomId(), BeGuid(0 == i%4 ? true : false));

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlobShared16_RandomId_AllGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlobShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, GenerateRandomId(), BeGuid(true));

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlobShared16_TimeBasedId_NoGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlobShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, GenerateTimeBasedId(i), BeGuid());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlobShared16_TimeBasedId_QuarterGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlobShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, GenerateTimeBasedId(i), BeGuid(0 == i%4 ? true : false));

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlobShared16_TimeBasedId_AllGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlobShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, GenerateTimeBasedId(i), BeGuid(true));

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlobNoIndexShared16_TimeBasedId_NoGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlobNoIndexShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, GenerateTimeBasedId(i), BeGuid());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlobNoIndexShared16_TimeBasedId_QuarterGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlobNoIndexShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, GenerateTimeBasedId(i), BeGuid(0 == i%4 ? true : false));

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlobNoIndexShared16_TimeBasedId_AllGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlobNoIndexShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, GenerateTimeBasedId(i), BeGuid(true));

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntBlobPostIndexShared16_TimeBasedId_AllGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntBlobNoIndexShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, GenerateTimeBasedId(i), BeGuid(true));

    CreateUniqueIndex(TABLE_IntBlobNoIndexShared16, "Guid");
    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntIntShared16_TimeBasedId_AllInternalGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntIntShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, GenerateTimeBasedId(i), GenerateRandomId());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntIntShared16_TimeBasedId_QuarterExternalGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntIntShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        {
        if (0 == i%4)
            {
            BeGuid externalGuid(true); // simulate an external guid being supplied
            InsertRow(i, externalGuid.m_guid.u[0], externalGuid.m_guid.u[1]);
            }
        else
            {
            InsertRow(i, GenerateTimeBasedId(i), GenerateRandomId());
            }
        }

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntIntShared16_TimeBasedId_AllExternalGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntIntShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        {
        BeGuid externalGuid(true); // simulate an external guid being supplied
        InsertRow(i, externalGuid.m_guid.u[0], externalGuid.m_guid.u[1]);
        }

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntIntUniqueShared16_TimeBasedId_AllInternalGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntIntUniqueShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, GenerateTimeBasedId(i), GenerateRandomId());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntIntUniqueShared16_TimeBasedId_QuarterExternalGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntIntUniqueShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        {
        if (0 == i%4)
            {
            BeGuid externalGuid(true); // simulate an external guid being supplied
            InsertRow(i, externalGuid.m_guid.u[0], externalGuid.m_guid.u[1]);
            }
        else
            {
            InsertRow(i, GenerateTimeBasedId(i), GenerateRandomId());
            }
        }

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntIntUniqueShared16_TimeBasedId_AllExternalGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntIntUniqueShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        {
        BeGuid externalGuid(true); // simulate an external guid being supplied
        InsertRow(i, externalGuid.m_guid.u[0], externalGuid.m_guid.u[1]);
        }

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntIntBlobShared16_TimeBasedId_NoGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntIntBlobShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, GenerateTimeBasedId(i), GenerateRandomId(), BeGuid());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntIntBlobShared16_TimeBasedId_QuarterGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntIntBlobShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, GenerateTimeBasedId(i), 0 == i%4 ? 0 : GenerateRandomId(), BeGuid(0 == i%4 ? true : false));

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntIntBlobShared16_TimeBasedId_AllGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntIntBlobShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, GenerateTimeBasedId(i), 0, BeGuid(true));

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, BlobShared16_RandomGuid_AllGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableBlobShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, BeGuid(true));

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, BlobShared16_TimeBasedGuid_AllGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableBlobShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, BeGuid(GenerateTimeBasedId(i), GenerateRandomId()));

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, TextShared16_AllGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableTextShared16();
    StartTest();

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, BeGuid(true).ToString().c_str());

    StopTest(TEST_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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

#endif