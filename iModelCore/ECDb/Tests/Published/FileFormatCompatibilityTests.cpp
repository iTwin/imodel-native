/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/FileFormatCompatibilityTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/BeTextFile.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/17
//+---------------+---------------+---------------+---------------+---------------+------
struct FileFormatCompatibilityTests : ECDbTestFixture
    {
    private:
        BentleyStatus CreateFakeBimFile(Utf8CP fileName, BeFileNameCR bisSchemaFolder);
        BentleyStatus ImportSchemasFromFolder(BeFileName const& schemaFolder);

    protected:
        BentleyStatus SetupDomainBimFile(Utf8CP fileName, BeFileNameCR benchmarkFolder);

        static BeFileName GetBenchmarkFolder();
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileFormatCompatibilityTests, CompareDdl)
    {
    BeFileName benchmarkFolder = GetBenchmarkFolder();
    ASSERT_EQ(SUCCESS, SetupDomainBimFile("bim02fileformatcompatibilitytest.ecdb", benchmarkFolder));

    Db benchmarkFile;
    BeFileName benchmarkFilePath(benchmarkFolder);
    benchmarkFilePath.AppendToPath(L"imodel2.ecdb");
    ASSERT_EQ(BE_SQLITE_OK, benchmarkFile.OpenBeSQLiteDb(benchmarkFilePath, Db::OpenParams(Db::OpenMode::Readonly))) << benchmarkFilePath;

    int benchmarkMasterTableRowCount = 0;
    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(benchmarkFile, "SELECT count(*) FROM sqlite_master"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    benchmarkMasterTableRowCount = stmt.GetValueInt(0);
    }

    Statement benchmarkDdlLookupStmt;
    ASSERT_EQ(BE_SQLITE_OK, benchmarkDdlLookupStmt.Prepare(benchmarkFile, "SELECT sql FROM sqlite_master WHERE name=?"));


    Statement actualDdlStmt;
    ASSERT_EQ(BE_SQLITE_OK, actualDdlStmt.Prepare(m_ecdb, "SELECT name, sql FROM sqlite_master ORDER BY name"));
    int actualMasterTableRowCount = 0;
    while (BE_SQLITE_ROW == actualDdlStmt.Step())
        {
        actualMasterTableRowCount++;
        Utf8CP actualName = actualDdlStmt.GetValueText(0);
        Utf8CP actualDdl = actualDdlStmt.GetValueText(1);

        ASSERT_EQ(BE_SQLITE_OK, benchmarkDdlLookupStmt.BindText(1, actualName, Statement::MakeCopy::No)) << actualName;
        ASSERT_EQ(BE_SQLITE_ROW, benchmarkDdlLookupStmt.Step()) << "DB object in actual file not found: " << actualName;
        Utf8CP benchmarkDdl = benchmarkDdlLookupStmt.GetValueText(0);
        EXPECT_STREQ(benchmarkDdl, actualDdl) << "DB object in actual file has different DDL than in benchmark file: " << actualName;
        benchmarkDdlLookupStmt.Reset();
        benchmarkDdlLookupStmt.ClearBindings();
        }

    ASSERT_EQ(benchmarkMasterTableRowCount, actualMasterTableRowCount) << benchmarkFilePath;
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileFormatCompatibilityTests, CompareProfileTables)
    {
    BeFileName benchmarkFolder = GetBenchmarkFolder();
    ASSERT_EQ(SUCCESS, SetupDomainBimFile("bim02fileformatcompatibilitytest.ecdb", benchmarkFolder));

    Db benchmarkFile;
    BeFileName benchmarkFilePath(benchmarkFolder);
    benchmarkFilePath.AppendToPath(L"imodel2.ecdb");
    ASSERT_EQ(BE_SQLITE_OK, benchmarkFile.OpenBeSQLiteDb(benchmarkFilePath, Db::OpenParams(Db::OpenMode::Readonly))) << benchmarkFilePath;

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT name FROM sqlite_master WHERE name LIKE 'ec_%%' ORDER BY name"));
    while (BE_SQLITE_ROW == stmt.Step())
        {
        Utf8CP tableName = stmt.GetValueText(0);

        //compare row count
        {
        Utf8String sql;
        sql.Sprintf("SELECT count(*) from %s", tableName);

        Statement benchmarkStmt, actualStmt;
        ASSERT_EQ(BE_SQLITE_OK, benchmarkStmt.Prepare(benchmarkFile, sql.c_str())) << sql;
        ASSERT_EQ(BE_SQLITE_ROW, benchmarkStmt.Step()) << sql;
        ASSERT_EQ(BE_SQLITE_OK, actualStmt.Prepare(m_ecdb, sql.c_str())) << sql;
        ASSERT_EQ(BE_SQLITE_ROW, actualStmt.Step()) << sql;

        const int benchmarkRowCount = benchmarkStmt.GetValueInt(0);
        const int actualRowCount = actualStmt.GetValueInt(0);
        ASSERT_EQ(benchmarkRowCount, actualRowCount) << tableName;
        }

        Utf8String sql("SELECT ");
        int colCount = 0;
        {
        Statement colInfosStmt;
        Utf8String pragma;
        pragma.Sprintf("pragma table_info('%s')", tableName);
        ASSERT_EQ(BE_SQLITE_OK, colInfosStmt.Prepare(m_ecdb, pragma.c_str())) << pragma;
        bool isFirstCol = true;
        while (BE_SQLITE_ROW == colInfosStmt.Step())
            {
            colCount++;
            if (!isFirstCol)
                sql.append(",");

            sql.append(colInfosStmt.GetValueText(1));
            isFirstCol = false;
            }

        sql.append(" FROM ").append(tableName);
        }
        ASSERT_NE(0, colCount) << "table_info returned no columns for table " << tableName;

        if (0 == BeStringUtilities::StricmpAscii(tableName, "ec_index") || 
            0 == BeStringUtilities::StricmpAscii(tableName, "ec_indexcolumn"))
            continue;

        Statement benchmarkTableStmt;
        ASSERT_EQ(BE_SQLITE_OK, benchmarkTableStmt.Prepare(benchmarkFile, sql.c_str())) << sql;

        Statement actualTableStmt;
        ASSERT_EQ(BE_SQLITE_OK, actualTableStmt.Prepare(m_ecdb, sql.c_str())) << sql;

        int rowCount = 0;
        while (BE_SQLITE_ROW == benchmarkTableStmt.Step())
            {
            ASSERT_EQ(BE_SQLITE_ROW, actualTableStmt.Step()) << "Differing number of rows in table " << tableName;

            rowCount++;

            for (int i = 0; i < colCount; i++)
                {
                Utf8CP colName = benchmarkTableStmt.GetColumnName(i);

                const DbValueType benchmarkColType = benchmarkTableStmt.GetColumnType(i);
                const DbValueType actualColType = actualTableStmt.GetColumnType(i);
                ASSERT_EQ(benchmarkColType, actualColType) << "Table: " << tableName << " Col: " << colName << " Row: " << rowCount;

                switch (benchmarkColType)
                    {
                        case DbValueType::NullVal:
                        {
                        EXPECT_EQ(benchmarkTableStmt.IsColumnNull(i), actualTableStmt.IsColumnNull(i)) << "Table: " << tableName << " Col: " << colName << " Row: " << rowCount;
                        break;
                        }

                        case DbValueType::BlobVal:
                        {
                        const int benchmarkBlobSize = benchmarkTableStmt.GetColumnBytes(i);
                        const int actualBlobSize = actualTableStmt.GetColumnBytes(i);
                        ASSERT_EQ(benchmarkBlobSize, actualBlobSize) << "Table: " << tableName << " Col: " << colName << " Row: " << rowCount;
                        void const* benchmarkValue = benchmarkTableStmt.GetValueBlob(i);
                        void const* actualValue = actualTableStmt.GetValueBlob(i);
                        ASSERT_EQ(0, memcmp(benchmarkValue, actualValue, (size_t) benchmarkBlobSize)) << "Table: " << tableName << " Col: " << colName << " Row: " << rowCount;
                        break;
                        }

                        case DbValueType::FloatVal:
                        {
                        double benchmarkValue = benchmarkTableStmt.GetValueDouble(i);
                        double actualValue = actualTableStmt.GetValueDouble(i);

                        EXPECT_DOUBLE_EQ(benchmarkValue, actualValue) << "Table: " << tableName << " Col: " << colName << " Row: " << rowCount;
                        break;
                        }

                        case DbValueType::IntegerVal:
                        {
                        int64_t benchmarkValue = benchmarkTableStmt.GetValueInt64(i);
                        int64_t actualValue = actualTableStmt.GetValueInt64(i);

                        EXPECT_EQ(benchmarkValue, actualValue) << "Table: " << tableName << " Col: " << colName << " Row: " << rowCount;
                        break;
                        }

                        case DbValueType::TextVal:
                        {
                        Utf8CP benchmarkValue = benchmarkTableStmt.GetValueText(i);
                        Utf8CP actualValue = actualTableStmt.GetValueText(i);

                        EXPECT_STREQ(benchmarkValue, actualValue) << "Table: " << tableName << " Col: " << colName << " Row: " << rowCount;
                        break;
                        }

                        default:
                            FAIL() << "Unknown DbValueType";
                            break;
                    }
                }
            }
        }
    }

//*****************************************************************************************
// FileFormatCompatibilityTests
//*****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
BentleyStatus FileFormatCompatibilityTests::SetupDomainBimFile(Utf8CP fileName, BeFileNameCR benchmarkFolder)
    {
    BeFileName bisSchemaFolder(benchmarkFolder);
    bisSchemaFolder.AppendToPath(L"schemas").AppendToPath(L"dgndb");

    BeFileName domainSchemaFolder(benchmarkFolder);
    domainSchemaFolder.AppendToPath(L"schemas").AppendToPath(L"domains");

    if (SUCCESS != CreateFakeBimFile(fileName, bisSchemaFolder))
        return ERROR;

    if (BE_SQLITE_OK != ReopenECDb())
        return ERROR;

    PERFLOG_START("ECDb ATP", "BIS domain schema import");
    const BentleyStatus stat = ImportSchemasFromFolder(domainSchemaFolder);
    PERFLOG_FINISH("ECDb ATP", "BIS domain schema import");

    if (SUCCESS != stat)
        return ERROR;

    if (BE_SQLITE_OK != ReopenECDb())
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
BentleyStatus FileFormatCompatibilityTests::CreateFakeBimFile(Utf8CP fileName, BeFileNameCR bisSchemaFolder)
    {
    if (BE_SQLITE_OK != SetupECDb(fileName))
        return ERROR;

    //BIS ECSchema needs this table to pre-exist
    if (BE_SQLITE_OK != m_ecdb.ExecuteSql("CREATE VIRTUAL TABLE dgn_SpatialIndex USING rtree(ElementId,MinX,MaxX,MinY,MaxY,MinZ,MaxZ)"))
        return ERROR;

    PERFLOG_START("ECDb ATP", "BIS schema import");
    const BentleyStatus stat = ImportSchemasFromFolder(bisSchemaFolder);
    PERFLOG_FINISH("ECDb ATP", "BIS schema import");
    return stat;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
BentleyStatus FileFormatCompatibilityTests::ImportSchemasFromFolder(BeFileName const& schemaFolder)
    {
    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext(false, true);
    ctx->AddSchemaLocater(m_ecdb.GetSchemaLocater());
    ctx->AddSchemaPath(schemaFolder);

    BeFileName ecdbSchemaSearchPath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(ecdbSchemaSearchPath);
    ecdbSchemaSearchPath.AppendToPath(L"ECSchemas").AppendToPath(L"ECDb");
    ctx->AddSchemaPath(ecdbSchemaSearchPath);

    bvector<BeFileName> schemaPaths;
    BeDirectoryIterator::WalkDirsAndMatch(schemaPaths, schemaFolder, L"*.ecschema.xml", false);

    if (schemaPaths.empty())
        return ERROR;

    for (BeFileName const& schemaXmlFile : schemaPaths)
        {
        ECN::ECSchemaPtr ecSchema = nullptr;
        const SchemaReadStatus stat = ECN::ECSchema::ReadFromXmlFile(ecSchema, schemaXmlFile.GetName(), *ctx);
        //duplicate schema error is ok, as the ReadFromXmlFile reads schema references implicitly.
        if (SchemaReadStatus::Success != stat && SchemaReadStatus::DuplicateSchema != stat)
            return ERROR;
        }

    if (SUCCESS != m_ecdb.Schemas().ImportSchemas(ctx->GetCache().GetSchemas()))
        {
        m_ecdb.AbandonChanges();
        return ERROR;
        }

    m_ecdb.SaveChanges();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2017
//---------------------------------------------------------------------------------------
//static
BeFileName FileFormatCompatibilityTests::GetBenchmarkFolder()
    {
    BeFileName benchmarkFilesDir;
    BeTest::GetHost().GetDocumentsRoot(benchmarkFilesDir);
    benchmarkFilesDir.AppendToPath(L"ECDb").AppendToPath(L"fileformatbenchmark");
    return benchmarkFilesDir;
    }

END_ECDBUNITTESTS_NAMESPACE

