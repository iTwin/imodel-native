/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/InsertPerformance_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BeSQLitePublishedTests.h"

#define TABLE_IntPrimaryKey         "test_IntPrimaryKey"
#define TABLE_IntPrimaryKeyShared16 "test_IntPrimaryKeyShared16"
#define TABLE_BlobPrimaryKey        "test_BlobPrimaryKey"
#define TABLE_StringPrimaryKey      "test_StringPrimaryKey"

#define DDL_SharedColumns16         ", [sc01], [sc02], [sc03], [sc04], [sc05], [sc06], [sc07], [sc08], [sc09], [sc10], [sc11], [sc12], [sc13], [sc14], [sc15], [sc16]"
#define DDL_ExtraColumns            ", [X] DOUBLE, [Y] DOUBLE, [Z] DOUBLE, [A] TEXT COLLATE NOCASE, [B] TEXT COLLATE NOCASE, [C] TEXT COLLATE NOCASE, [I] INTEGER, [J] INTEGER, [K] INTEGER"
#define INTO_ExtraColumns           ",[X],[Y],[Z],[A],[B],[C],[I],[J],[K]"
#define VALUES_ExtraColumns         ",1.1,2.2,3.3,'AAAAAAAAAA','BBBBBBBBBB','CCCCCCCCCC',1,2,3"

//=======================================================================================
// @bsiclass                                    Shaun.Sewall                    09/2016
//=======================================================================================
struct InsertPerformanceTests : public ::testing::Test
{
protected:
    Db m_db;
    Statement m_statement;

    static uint64_t s_fileSizeNoSharedColumns;
    static uint64_t s_fileSizeSharedColumns;

public:
    static void SetUpTestCase();
    static void TearDownTestCase();

    void SetUpDb(Utf8CP dbName);
    int GetNumInserts() const {return 500 * 1000;}
    void SaveChanges(Utf8CP changesetName=nullptr) {m_db.SaveChanges(changesetName);}

    void CreateTableIntPrimaryKey();
    void CreateTableIntPrimaryKeyShared16();
    void CreateTableBlobPrimaryKey();
    void CreateTableStringPrimaryKey();

    void InsertRow(int64_t id, BeGuidCR guid, Utf8CP label);
    void InsertRow(BeGuidCR guid, Utf8CP label);
    void InsertRow(Utf8CP guid, Utf8CP label);

    int GetParameterIndexId() {return m_statement.GetParameterIndex(":Id");}
    int GetParameterIndexGuid() {return m_statement.GetParameterIndex(":Guid");}
    int GetParameterIndexLabel() {return m_statement.GetParameterIndex(":Label");}

    int64_t GenerateSnowflakeId(int);
    int64_t GenerateRandomId() {int64_t id; BeSQLiteLib::Randomness(sizeof(id), &id); return id;}
    BeBriefcaseBasedId GenerateBriefcaseBasedId(int i) {return BeBriefcaseBasedId(BeBriefcaseId(i%10+2), i);}
    Utf8String GenerateLabel(Utf8CP prefix, int i) {return Utf8PrintfString("%d", i);} // don't include prefix for now so file sizes are comparable
};

uint64_t InsertPerformanceTests::s_fileSizeNoSharedColumns = 0;
uint64_t InsertPerformanceTests::s_fileSizeSharedColumns = 0;

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
    LOGTODB("File Size", "No SharedColumns", 0, "", (int)s_fileSizeNoSharedColumns);
    LOGTODB("File Size", "16 SharedColumns", 0, "", (int)s_fileSizeSharedColumns);
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
void InsertPerformanceTests::CreateTableIntPrimaryKey()
    {
    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable(TABLE_IntPrimaryKey, "[Id] INTEGER PRIMARY KEY, [Guid] BLOB UNIQUE, [Label] TEXT COLLATE NOCASE" DDL_ExtraColumns));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Prepare(m_db, "INSERT INTO " TABLE_IntPrimaryKey " ([Id],[Guid],[Label]" INTO_ExtraColumns ") VALUES (:Id,:Guid,:Label" VALUES_ExtraColumns ")"));
    SaveChanges("CreateTableIntPrimaryKey");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::CreateTableIntPrimaryKeyShared16()
    {
    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable(TABLE_IntPrimaryKeyShared16, "[Id] INTEGER PRIMARY KEY, [Guid] BLOB UNIQUE, [Label] TEXT COLLATE NOCASE" DDL_ExtraColumns DDL_SharedColumns16));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Prepare(m_db, "INSERT INTO " TABLE_IntPrimaryKeyShared16 " ([Id],[Guid],[Label]" INTO_ExtraColumns ") VALUES (:Id,:Guid,:Label" VALUES_ExtraColumns ")"));
    SaveChanges("CreateTableIntPrimaryKeyShared16");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::CreateTableBlobPrimaryKey()
    {
    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable(TABLE_BlobPrimaryKey, "[Guid] BLOB UNIQUE PRIMARY KEY, [Label] TEXT COLLATE NOCASE" DDL_ExtraColumns));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Prepare(m_db, "INSERT INTO " TABLE_BlobPrimaryKey " ([Guid],[Label]" INTO_ExtraColumns ") VALUES (:Guid,:Label" VALUES_ExtraColumns ")"));
    SaveChanges("CreateTableBlobPrimaryKey");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
void InsertPerformanceTests::CreateTableStringPrimaryKey()
    {
    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable(TABLE_StringPrimaryKey, "[Guid] TEXT UNIQUE PRIMARY KEY, [Label] TEXT COLLATE NOCASE" DDL_ExtraColumns));
    ASSERT_EQ(BE_SQLITE_OK, m_statement.Prepare(m_db, "INSERT INTO " TABLE_StringPrimaryKey " ([Guid],[Label]" INTO_ExtraColumns ") VALUES (:Guid,:Label" VALUES_ExtraColumns ")"));
    SaveChanges("CreateTableStringPrimaryKey");
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
TEST_F(InsertPerformanceTests, IntPrimaryKeySequentialNoGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntPrimaryKey();
    StopWatch timer(TEST_NAME, true);

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, BeGuid(), GenerateLabel(TEST_NAME, i).c_str());

    SaveChanges(TEST_NAME);
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), TEST_NAME, GetNumInserts());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntPrimaryKeySequentialQuarterGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntPrimaryKey();
    StopWatch timer(TEST_NAME, true);

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, BeGuid(0 == i%4 ? true : false), GenerateLabel(TEST_NAME, i).c_str());

    SaveChanges(TEST_NAME);
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), TEST_NAME, GetNumInserts());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntPrimaryKeySequentialAllGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntPrimaryKey();
    StopWatch timer(TEST_NAME, true);

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(i, BeGuid(true), GenerateLabel(TEST_NAME, i).c_str());

    SaveChanges(TEST_NAME);
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), TEST_NAME, GetNumInserts());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntPrimaryKeyRandomNoGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntPrimaryKey();
    StopWatch timer(TEST_NAME, true);

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(GenerateRandomId(), BeGuid(), GenerateLabel(TEST_NAME, i).c_str());

    SaveChanges(TEST_NAME);
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), TEST_NAME, GetNumInserts());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntPrimaryKeyRandomQuarterGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntPrimaryKey();
    StopWatch timer(TEST_NAME, true);

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(GenerateRandomId(), BeGuid(0 == i%4 ? true : false), GenerateLabel(TEST_NAME, i).c_str());

    SaveChanges(TEST_NAME);
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), TEST_NAME, GetNumInserts());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntPrimaryKeyRandomAllGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntPrimaryKey();
    StopWatch timer(TEST_NAME, true);

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(GenerateRandomId(), BeGuid(true), GenerateLabel(TEST_NAME, i).c_str());

    SaveChanges(TEST_NAME);
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), TEST_NAME, GetNumInserts());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntPrimaryKeyAlternatingBriefcaseIdNoGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntPrimaryKey();
    StopWatch timer(TEST_NAME, true);

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(GenerateBriefcaseBasedId(i).GetValue(), BeGuid(), GenerateLabel(TEST_NAME, i).c_str());

    SaveChanges(TEST_NAME);
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), TEST_NAME, GetNumInserts());
    
    BeFileName dbFileName(m_db.GetDbFileName(), BentleyCharEncoding::Utf8);
    dbFileName.GetFileSize(s_fileSizeNoSharedColumns);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntPrimaryKeyAlternatingBriefcaseIdQuarterGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntPrimaryKey();
    StopWatch timer(TEST_NAME, true);

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(GenerateBriefcaseBasedId(i).GetValue(), BeGuid(0 == i%4 ? true : false), GenerateLabel(TEST_NAME, i).c_str());

    SaveChanges(TEST_NAME);
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), TEST_NAME, GetNumInserts());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntPrimaryKeyAlternatingBriefcaseIdAllGuids)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntPrimaryKey();
    StopWatch timer(TEST_NAME, true);

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(GenerateBriefcaseBasedId(i).GetValue(), BeGuid(true), GenerateLabel(TEST_NAME, i).c_str());

    SaveChanges(TEST_NAME);
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), TEST_NAME, GetNumInserts());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntPrimaryKeyAlternatingBriefcaseIdNoGuidsShared16)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntPrimaryKeyShared16();
    StopWatch timer(TEST_NAME, true);

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(GenerateBriefcaseBasedId(i).GetValue(), BeGuid(), GenerateLabel(TEST_NAME, i).c_str());

    SaveChanges(TEST_NAME);
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), TEST_NAME, GetNumInserts());
    
    BeFileName dbFileName(m_db.GetDbFileName(), BentleyCharEncoding::Utf8);
    dbFileName.GetFileSize(s_fileSizeSharedColumns);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, IntPrimaryKeySnowflakeIdNoGuidsShared16)
    {
    SetUpDb(TEST_NAME);
    CreateTableIntPrimaryKeyShared16();
    StopWatch timer(TEST_NAME, true);

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(GenerateSnowflakeId(i), BeGuid(), GenerateLabel(TEST_NAME, i).c_str());

    SaveChanges(TEST_NAME);
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), TEST_NAME, GetNumInserts());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, BlobPrimaryKey)
    {
    SetUpDb(TEST_NAME);
    CreateTableBlobPrimaryKey();
    StopWatch timer(TEST_NAME, true);

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(BeGuid(true), GenerateLabel(TEST_NAME, i).c_str());

    SaveChanges(TEST_NAME);
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), TEST_NAME, GetNumInserts());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, StringPrimaryKey)
    {
    SetUpDb(TEST_NAME);
    CreateTableStringPrimaryKey();
    StopWatch timer(TEST_NAME, true);

    for (int i=1; i<=GetNumInserts(); i++)
        InsertRow(BeGuid(true).ToString().c_str(), GenerateLabel(TEST_NAME, i).c_str());

    SaveChanges(TEST_NAME);
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), TEST_NAME, GetNumInserts());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, BeGuidCreate)
    {
    StopWatch timer(TEST_NAME, true);

    for (int i=1; i<=GetNumInserts(); i++)
        {
        BeGuid guid(true);
        UNUSED_VARIABLE(guid);
        }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), TEST_NAME, GetNumInserts());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(InsertPerformanceTests, BeGuidCreateAndToString)
    {
    StopWatch timer(TEST_NAME, true);

    for (int i=1; i<=GetNumInserts(); i++)
        {
        Utf8String guid = BeGuid(true).ToString();
        UNUSED_VARIABLE(guid);
        }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), TEST_NAME, GetNumInserts());
    }
