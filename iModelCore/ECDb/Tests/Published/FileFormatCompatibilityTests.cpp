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
#include <Bentley/BeNumerical.h>
#include <BeSQLite/BeBriefcaseBasedIdSequence.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

#define PROFILETABLE_SELECT_Schema "SELECT Name,DisplayLabel,Description,Alias,VersionDigit1,VersionDigit2,VersionDigit3 FROM ec_Schema ORDER BY Name"
#define PROFILETABLE_SELECT_SchemaReference "SELECT s.Name,ref.Name FROM ec_SchemaReference sr JOIN ec_Schema s ON sr.SchemaId=s.Id JOIN ec_Schema ref ON sr.ReferencedSchemaId=ref.Id ORDER BY s.Name,ref.Name"
#define PROFILETABLE_SELECT_Class "SELECT s.Name,c.Name,c.DisplayLabel,c.Description,c.Type,c.Modifier,c.RelationshipStrength,c.RelationshipStrengthDirection,c.CustomAttributeContainertype FROM ec_Class c JOIN ec_Schema s ON s.Id=c.SchemaId ORDER BY s.Name, c.Name"
#define PROFILETABLE_SELECT_ClassHasBaseClasses "SELECT s.Name, c.Name, baseS.Name, baseC.Name FROM ec_ClassHasBaseClasses bc JOIN ec_Class c ON bc.ClassId=c.Id JOIN ec_Schema s ON s.Id=c.SchemaId " \
                                                "JOIN ec_Class baseC ON bc.BaseClassId=baseC.Id JOIN ec_Schema baseS ON baseS.Id=baseC.SchemaId " \
                                                "ORDER BY s.Name, c.Name, baseS.Name, baseC.Name, bc.Ordinal"
#define PROFILETABLE_SELECT_Enumeration "SELECT s.Name, e.Name, e.DisplayLabel,e.Description,e.UnderlyingPrimitiveType,e.IsStrict,e.EnumValues FROM ec_Enumeration e JOIN ec_Schema s ON s.Id=e.SchemaId ORDER BY s.Name,e.Name"
#define PROFILETABLE_SELECT_KindOfQunatity "SELECT s.Name, koq.Name, koq.DisplayLabel,koq.Description,koq.PersistenceUnit,koq.RelativeError,koq.PresentationUnits FROM ec_KindOfQuantity koq JOIN ec_Schema s ON s.Id=koq.SchemaId ORDER BY s.Name,koq.Name"
#define PROFILETABLE_SELECT_PropertyCategory "SELECT s.Name, pc.Name, pc.DisplayLabel,pc.Description,pc.Priority FROM ec_PropertyCategory pc JOIN ec_Schema s ON s.Id=pc.SchemaId ORDER BY s.Name,pc.Name"
#define PROFILETABLE_SELECT_Property "SELECT s.Name, p.Name, p.DisplayLabel,p.Description,p.IsReadonly,p.Priority,p.Ordinal,p.Kind,p.PrimitiveType,p.PrimitiveTypeMinLength,p.PrimitiveTypeMaxLength,p.PrimitiveTypeMinValue,p.PrimitiveTypeMaxValue, " \
                                    "enumS.Name, enum.Name, structS.Name,struct.Name, p.ExtendedTypeName, koqS.Name,koq.Name,catS.Name,cat.Name,p.ArrayMinOccurs,p.ArrayMaxOccurs, " \
                                    "navRelS.Name,navRel.Name, p.NavigationDirection FROM ec_Property p " \
                                    "JOIN ec_Class c ON p.ClassId=c.Id JOIN ec_Schema s ON s.Id=c.SchemaId " \
                                    "JOIN ec_Enumeration enum ON enum.Id=p.EnumerationId JOIN ec_Schema enumS ON enumS.Id=enum.SchemaId " \
                                    "JOIN ec_Class struct ON struct.Id=p.StructClassId JOIN ec_Schema structS ON structS.Id=struct.SchemaId " \
                                    "JOIN ec_KindOfQuantity koq ON koq.Id=p.KindOfQuantityId JOIN ec_Schema koqS ON koqS.Id=koq.SchemaId " \
                                    "JOIN ec_PropertyCategory cat ON cat.Id=p.CategoryId JOIN ec_Schema catS ON catS.Id=cat.SchemaId " \
                                    "JOIN ec_Class navRel ON navRel.Id=p.NavigationRelationshipClassId JOIN ec_Schema navRelS ON navRelS.Id=navRel.SchemaId " \
                                    "ORDER BY s.Name,c.Name,p.Name"
#define PROFILETABLE_SELECT_RelationshipConstraint "SELECT relS.Name, rel.Name, rc.RelationshipEnd,rc.MultiplicityLowerLimit,rc.MultiplicityUpperLimit,rc.IsPolymorphic,rc.RoleLabel,abstractConstraintS.Name, abstractConstraint.Name FROM ec_RelationshipConstraint rc " \
                                    "JOIN ec_Class rel ON rel.Id=rc.RelationshipClassId JOIN ec_Schema relS ON relS.Id=rel.SchemaId " \
                                    "JOIN ec_Class abstractConstraint ON rc.AbstractConstraintClassId=abstractConstraint.Id JOIN ec_Schema abstractConstraintS ON abstractConstraintS.Id=abstractConstraint.SchemaId " \
                                    "ORDER BY relS.Name, rel.Name, rc.RelationshipEnd"
#define PROFILETABLE_SELECT_RelationshipConstraintClass "SELECT relS.Name,rel.Name,rc.RelationshipEnd,constraintS.Name,constraintC.Name FROM ec_RelationshipConstraintClass rcc " \
                                    "JOIN ec_RelationshipConstraint rc ON rc.Id=rcc.ConstraintId JOIN ec_Class rel ON rel.Id=rc.RelationshipClassId JOIN ec_Schema relS ON relS.Id=rel.SchemaId " \
                                    "JOIN ec_Class constraintC ON rcc.ClassId=constraintC.Id JOIN ec_Schema constraintS ON constraintS.Id=constraintC.SchemaId " \
                                    "ORDER BY relS.Name,rel.Name,rc.RelationshipEnd,constraintS.Name,constraintC.Name"
#define PROFILETABLE_SELECT_CustomAttribute "SELECT Instance,Ordinal,ContainerType FROM ec_CustomAttribute ORDER BY ClassId,ContainerId,ContainerType,Ordinal"
#define PROFILETABLE_SELECT_ClassMap "SELECT s.Name, c.Name, cm.MapStrategy,cm.ShareColumnsMode,cm.MaxSharedColumnsBeforeOverflow,cm.JoinedTableInfo FROM ec_ClassMap cm JOIN ec_Class c ON cm.ClassId=c.Id JOIN ec_Schema s ON s.Id=c.SchemaId ORDER BY s.Name,c.Name"
#define PROFILETABLE_SELECT_PropertyPath "SELECT rootPropS.Name, rootPropC.Name, rootProp.Name, pp.AccessString FROM ec_PropertyPath pp " \
                                    "JOIN ec_Property rootProp ON rootProp.Id=pp.RootPropertyId JOIN ec_Class rootPropC ON rootPropC.Id=rootProp.ClassId JOIN ec_Schema rootPropS ON rootPropS.Id=rootPropC.SchemaId " \
                                    "ORDER BY rootPropS.Name,rootPropC.Name, rootProp.Name,pp.AccessString"
#define PROFILETABLE_SELECT_PropertyMap "SELECT s.Name, c.Name, rootPropS.Name,rootPropC.Name,rootProp.Name,t.Name,col.Name FROM ec_PropertyMap pm " \
                                    "JOIN ec_Class c ON c.Id=pm.ClassId JOIN ec_Schema s ON s.Id=c.SchemaId " \
                                    "JOIN ec_PropertyPath pp ON pp.Id=pm.PropertyPathId JOIN ec_Property rootProp ON rootProp.Id=pp.RootPropertyId JOIN ec_Class rootPropC ON rootPropC.Id=rootProp.ClassId JOIN ec_Schema rootPropS ON rootPropS.Id=rootPropC.SchemaId " \
                                    "JOIN ec_Column col ON col.Id=pm.ColumnId JOIN ec_Table t ON t.Id=col.TableId " \
                                    "ORDER BY s.Name,c.Name,rootPropS.Name,rootPropC.Name,rootProp.Name,t.Name,col.Name"
#define PROFILETABLE_SELECT_Table "SELECT parentT.Name, t.Name, t.Type, exclusiveRootClassS.Name, exclusiveRootClass.Name, t.UpdatableViewName FROM ec_table t " \
                                    "JOIN ec_Table parentT ON parentT.Id=t.ParentTableId JOIN ec_Class exclusiveRootClass ON exclusiveRootClass.Id=t.ExclusiveRootClassId JOIN ec_Schema exclusiveRootClassS ON exclusiveRootClassS.Id=exclusiveRootClass.SchemaId " \
                                    "ORDER BY t.Name"
#define PROFILETABLE_SELECT_Column "SELECT t.Name, c.Name,c.Type,c.IsVirtual,c.Ordinal,c.NotNullConstraint,c.UniqueConstraint,c.CheckConstraint,c.DefaultConstraint,c.CollationConstraint,c.OrdinalInPrimaryKey,c.ColumnKind FROM ec_Column c " \
                                    "JOIN ec_Table t ON t.Id=c.TableId ORDER BY t.Name, c.Name"
#define PROFILETABLE_SELECT_Index "SELECT i.Name, t.Name, i.IsUnique,i.AddNotNullWhereExp,i.IsAutoGenerated,s.Name,c.Name,i.AppliesToSubclassesIfPartial FROM ec_Index i " \
                                    "JOIN ec_Table t ON t.Id=i.TableId JOIN ec_Class c ON c.Id=i.ClassId JOIN ec_Schema s ON s.Id=c.SchemaId ORDER BY i.Name"
#define PROFILETABLE_SELECT_IndexColumn "SELECT i.Name, c.Name, t.Name, ic.Ordinal FROM ec_IndexColumn ic " \
                                    "JOIN ec_Index i ON i.Id=ic.IndexId JOIN ec_Column c ON c.Id=ic.ColumnId JOIN ec_Table t ON t.Id=c.TableId ORDER BY i.Name, ic.Ordinal"

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/17
//+---------------+---------------+---------------+---------------+---------------+------
struct FileFormatCompatibilityTests : ECDbTestFixture
    {
    private:
        BentleyStatus CreateFakeBimFile(Utf8CP fileName, BeFileNameCR bisSchemaFolder, DbCR benchmarkFile);
        BentleyStatus ImportSchemasFromFolder(BeFileName const& schemaFolder);

    protected:
        BentleyStatus SetupTestFile(Utf8CP fileName, BeFileNameCR benchmarkFolder, DbCR benchmarkFile);
        static bool CompareTable(DbCR benchmark, DbCR actual, Utf8CP tableName, Utf8CP selectSql);

        static BeFileName GetBenchmarkFolder();
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileFormatCompatibilityTests, CompareDdl_NewFile)
    {
    BeFileName benchmarkFolder = GetBenchmarkFolder();

    Db benchmarkFile;
    BeFileName benchmarkFilePath(benchmarkFolder);
    benchmarkFilePath.AppendToPath(L"imodel2.ecdb");
    ASSERT_EQ(BE_SQLITE_OK, benchmarkFile.OpenBeSQLiteDb(benchmarkFilePath, Db::OpenParams(Db::OpenMode::Readonly))) << benchmarkFilePath;

    ASSERT_EQ(SUCCESS, SetupTestFile("imodel2fileformatcompatibilitytest.ecdb", benchmarkFolder, benchmarkFile));

    BeFileName artefactOutDir;
    BeTest::GetHost().GetOutputRoot(artefactOutDir);

    BeFileStatus stat = BeFileStatus::Success;


    int benchmarkMasterTableRowCount = 0;
    {
    BeFileName benchmarkDdlDumpFilePath(artefactOutDir);
    benchmarkDdlDumpFilePath.AppendToPath(L"benchmarkddl.txt");

    BeTextFilePtr benchmarkDdlDumpFile = BeTextFile::Open(stat, benchmarkDdlDumpFilePath, TextFileOpenType::Write, TextFileOptions::KeepNewLine, TextFileEncoding::Utf8);
    ASSERT_EQ(BeFileStatus::Success, stat) << "Creating file " << benchmarkDdlDumpFilePath;

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(benchmarkFile, "SELECT sql FROM sqlite_master ORDER BY name"));
    while (BE_SQLITE_ROW == stmt.Step())
        {
        benchmarkMasterTableRowCount++;
        benchmarkDdlDumpFile->PutLine(WString(stmt.GetValueText(0), BentleyCharEncoding::Utf8).c_str(), true);
        }
    }


    BeFileName actualDdlDumpFilePath(artefactOutDir);
    actualDdlDumpFilePath.AppendToPath(L"actualddl.txt");
    BeTextFilePtr actualDdlDumpFile = BeTextFile::Open(stat, actualDdlDumpFilePath, TextFileOpenType::Write, TextFileOptions::KeepNewLine, TextFileEncoding::Utf8);
    ASSERT_EQ(BeFileStatus::Success, stat) << "Creating file " << actualDdlDumpFilePath;

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

        actualDdlDumpFile->PutLine(WString(actualDdl, BentleyCharEncoding::Utf8).c_str(), true);

        benchmarkDdlLookupStmt.BindText(1, actualName, Statement::MakeCopy::No);
        if (BE_SQLITE_ROW == benchmarkDdlLookupStmt.Step())
            {
            Utf8CP benchmarkDdl = benchmarkDdlLookupStmt.GetValueText(0);
            EXPECT_STREQ(benchmarkDdl, actualDdl) << "DB object in actual file has different DDL than in benchmark file: " << actualName;
            }
        else
            EXPECT_TRUE(false) << "DB object in actual file not found: " << actualName;

        benchmarkDdlLookupStmt.Reset();
        benchmarkDdlLookupStmt.ClearBindings();
        }

    ASSERT_EQ(benchmarkMasterTableRowCount, actualMasterTableRowCount) << benchmarkFilePath;
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileFormatCompatibilityTests, CompareDdl_UpgradedFile)
    {
    BeFileName benchmarkFolder = GetBenchmarkFolder();

    BeFileName benchmarkFilePath(benchmarkFolder);
    benchmarkFilePath.AppendToPath(L"imodel2.ecdb");
    Db benchmarkFile;
    ASSERT_EQ(BE_SQLITE_OK, benchmarkFile.OpenBeSQLiteDb(benchmarkFilePath, Db::OpenParams(Db::OpenMode::Readonly))) << benchmarkFilePath;

    BeFileName artefactOutDir;
    BeTest::GetHost().GetOutputRoot(artefactOutDir);
    if (!artefactOutDir.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(artefactOutDir));

    BeFileName upgradedFilePath(artefactOutDir);
    upgradedFilePath.AppendToPath(L"upgradedimodel2.ecdb");
    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(benchmarkFilePath, upgradedFilePath));
    ECDb upgradedFile;
    ASSERT_EQ(BE_SQLITE_OK, upgradedFile.OpenBeSQLiteDb(upgradedFilePath, ECDb::OpenParams(ECDb::OpenMode::Readonly)));
    
    
    BeFileStatus stat = BeFileStatus::Success;
    
    int benchmarkMasterTableRowCount = 0;
    {
    BeFileName benchmarkDdlDumpFilePath(artefactOutDir);
    benchmarkDdlDumpFilePath.AppendToPath(L"benchmarkddl.txt");

    BeTextFilePtr benchmarkDdlDumpFile = BeTextFile::Open(stat, benchmarkDdlDumpFilePath, TextFileOpenType::Write, TextFileOptions::KeepNewLine, TextFileEncoding::Utf8);
    ASSERT_EQ(BeFileStatus::Success, stat) << "Creating file " << benchmarkDdlDumpFilePath;

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(benchmarkFile, "SELECT sql FROM sqlite_master ORDER BY name"));
    while (BE_SQLITE_ROW == stmt.Step())
        {
        benchmarkMasterTableRowCount++;
        benchmarkDdlDumpFile->PutLine(WString(stmt.GetValueText(0), BentleyCharEncoding::Utf8).c_str(), true);
        }
    }


    BeFileName actualDdlDumpFilePath(artefactOutDir);
    actualDdlDumpFilePath.AppendToPath(L"upgradedfileddl.txt");
    BeTextFilePtr actualDdlDumpFile = BeTextFile::Open(stat, actualDdlDumpFilePath, TextFileOpenType::Write, TextFileOptions::KeepNewLine, TextFileEncoding::Utf8);
    ASSERT_EQ(BeFileStatus::Success, stat) << "Creating file " << actualDdlDumpFilePath;

    Statement benchmarkDdlLookupStmt;
    ASSERT_EQ(BE_SQLITE_OK, benchmarkDdlLookupStmt.Prepare(benchmarkFile, "SELECT sql FROM sqlite_master WHERE name=?"));


    Statement actualDdlStmt;
    ASSERT_EQ(BE_SQLITE_OK, actualDdlStmt.Prepare(upgradedFile, "SELECT name, sql FROM sqlite_master ORDER BY name"));
    int actualMasterTableRowCount = 0;
    while (BE_SQLITE_ROW == actualDdlStmt.Step())
        {
        actualMasterTableRowCount++;
        Utf8CP actualName = actualDdlStmt.GetValueText(0);
        Utf8CP actualDdl = actualDdlStmt.GetValueText(1);

        actualDdlDumpFile->PutLine(WString(actualDdl, BentleyCharEncoding::Utf8).c_str(), true);

        benchmarkDdlLookupStmt.BindText(1, actualName, Statement::MakeCopy::No);
        if (BE_SQLITE_ROW == benchmarkDdlLookupStmt.Step())
            {
            Utf8CP benchmarkDdl = benchmarkDdlLookupStmt.GetValueText(0);
            EXPECT_STREQ(benchmarkDdl, actualDdl) << "DB object in upgraded file has different DDL than in benchmark file: " << actualName;
            }
        else
            EXPECT_TRUE(false) << "DB object in upgraded file not found: " << actualName;

        benchmarkDdlLookupStmt.Reset();
        benchmarkDdlLookupStmt.ClearBindings();
        }

    ASSERT_EQ(benchmarkMasterTableRowCount, actualMasterTableRowCount) << benchmarkFilePath;
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileFormatCompatibilityTests, CompareProfileTables_NewFile)
    {
    BeFileName benchmarkFolder = GetBenchmarkFolder();

    Db benchmarkFile;
    BeFileName benchmarkFilePath(benchmarkFolder);
    benchmarkFilePath.AppendToPath(L"imodel2.ecdb");
    ASSERT_EQ(BE_SQLITE_OK, benchmarkFile.OpenBeSQLiteDb(benchmarkFilePath, Db::OpenParams(Db::OpenMode::Readonly))) << benchmarkFilePath;

    ASSERT_EQ(SUCCESS, SetupTestFile("imodel2fileformatcompatibilitytest.ecdb", benchmarkFolder, benchmarkFile));
    
    //profile table count check
    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, R"sql(SELECT count(*) FROM sqlite_master WHERE name LIKE 'ec\_%' ESCAPE '\' ORDER BY name COLLATE NOCASE)sql"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetSql();
    ASSERT_EQ(20, stmt.GetValueInt(0)) << "ECDb profile table count";
    }

    //schema profile tables
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_Schema", PROFILETABLE_SELECT_Schema));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_SchemaReference", PROFILETABLE_SELECT_SchemaReference));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_Class", PROFILETABLE_SELECT_Class));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_ClassHasBaseClasses", PROFILETABLE_SELECT_ClassHasBaseClasses));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_Enumeration", PROFILETABLE_SELECT_Enumeration));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_KindOfQuantity", PROFILETABLE_SELECT_KindOfQunatity));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_PropertyCategory", PROFILETABLE_SELECT_PropertyCategory));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_Property", PROFILETABLE_SELECT_Property));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_RelationshipConstraint", PROFILETABLE_SELECT_RelationshipConstraint));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_RelationshipConstraintClass", PROFILETABLE_SELECT_RelationshipConstraintClass));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_CustomAttribute", PROFILETABLE_SELECT_CustomAttribute));

    //mapping profile tables
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_ClassMap", PROFILETABLE_SELECT_ClassMap));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_PropertyPath", PROFILETABLE_SELECT_PropertyPath));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_PropertyMap", PROFILETABLE_SELECT_PropertyMap));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_Table", PROFILETABLE_SELECT_Table));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_Column", PROFILETABLE_SELECT_Column));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_Index", PROFILETABLE_SELECT_Index));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_IndexColumn", PROFILETABLE_SELECT_IndexColumn));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileFormatCompatibilityTests, CompareProfileTables_UpgradedFile)
    {
    BeFileName benchmarkFolder = GetBenchmarkFolder();

    BeFileName benchmarkFilePath(benchmarkFolder);
    benchmarkFilePath.AppendToPath(L"imodel2.ecdb");
    Db benchmarkFile;
    ASSERT_EQ(BE_SQLITE_OK, benchmarkFile.OpenBeSQLiteDb(benchmarkFilePath, Db::OpenParams(Db::OpenMode::Readonly))) << benchmarkFilePath;

    BeFileName artefactOutDir;
    BeTest::GetHost().GetOutputRoot(artefactOutDir);
    if (!artefactOutDir.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(artefactOutDir));

    BeFileName upgradedFilePath(artefactOutDir);
    upgradedFilePath.AppendToPath(L"upgradedimodel2.ecdb");
    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(benchmarkFilePath, upgradedFilePath));
    ECDb upgradedFile;
    ASSERT_EQ(BE_SQLITE_OK, upgradedFile.OpenBeSQLiteDb(upgradedFilePath, ECDb::OpenParams(ECDb::OpenMode::Readonly)));

    //profile table count check
    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(upgradedFile, R"sql(SELECT count(*) FROM sqlite_master WHERE name LIKE 'ec\_%' ESCAPE '\' ORDER BY name COLLATE NOCASE)sql"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetSql();
    ASSERT_EQ(20, stmt.GetValueInt(0)) << "ECDb profile table count";
    }

    //schema profile tables
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_Schema", PROFILETABLE_SELECT_Schema));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_SchemaReference", PROFILETABLE_SELECT_SchemaReference));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_Class", PROFILETABLE_SELECT_Class));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_ClassHasBaseClasses", PROFILETABLE_SELECT_ClassHasBaseClasses));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_Enumeration", PROFILETABLE_SELECT_Enumeration));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_KindOfQuantity", PROFILETABLE_SELECT_KindOfQunatity));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_PropertyCategory", PROFILETABLE_SELECT_PropertyCategory));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_Property", PROFILETABLE_SELECT_Property));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_RelationshipConstraint", PROFILETABLE_SELECT_RelationshipConstraint));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_RelationshipConstraintClass", PROFILETABLE_SELECT_RelationshipConstraintClass));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_CustomAttribute", PROFILETABLE_SELECT_CustomAttribute));

    //mapping profile tables
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_ClassMap", PROFILETABLE_SELECT_ClassMap));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_PropertyPath", PROFILETABLE_SELECT_PropertyPath));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_PropertyMap", PROFILETABLE_SELECT_PropertyMap));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_Table", PROFILETABLE_SELECT_Table));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_Column", PROFILETABLE_SELECT_Column));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_Index", PROFILETABLE_SELECT_Index));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_IndexColumn", PROFILETABLE_SELECT_IndexColumn));
    }

//*****************************************************************************************
// FileFormatCompatibilityTests
//*****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
//static
bool FileFormatCompatibilityTests::CompareTable(DbCR benchmarkFile, DbCR actualFile, Utf8CP tableName, Utf8CP selectSql)
    {
    //compare row count
            {
            Utf8String sql;
            sql.Sprintf("SELECT count(*) from %s", tableName);

            Statement benchmarkStmt;
            if (BE_SQLITE_OK != benchmarkStmt.Prepare(benchmarkFile, sql.c_str()) ||
                BE_SQLITE_ROW != benchmarkStmt.Step())
                {
                return false;
                }

            const int benchmarkRowCount = benchmarkStmt.GetValueInt(0);

            Statement actualStmt;
            if (BE_SQLITE_OK != actualStmt.Prepare(actualFile, sql.c_str()) ||
                BE_SQLITE_ROW != actualStmt.Step())
                {
                return false;
                }

            const int actualRowCount = actualStmt.GetValueInt(0);
            EXPECT_EQ(benchmarkRowCount, actualRowCount) << tableName;
            }

    Statement benchmarkTableStmt, actualTableStmt;
    if (BE_SQLITE_OK != benchmarkTableStmt.Prepare(benchmarkFile, selectSql) ||
        BE_SQLITE_OK != actualTableStmt.Prepare(actualFile, selectSql))
        return false;

    int rowCount = 0;
    while (BE_SQLITE_ROW == benchmarkTableStmt.Step())
        {
        if (BE_SQLITE_ROW != actualTableStmt.Step())
            return false;

        rowCount++;

        const int colCount = benchmarkTableStmt.GetColumnCount();
        if (colCount != actualTableStmt.GetColumnCount())
            return false;

        for (int i = 0; i < colCount; i++)
            {
            Utf8CP colName = benchmarkTableStmt.GetColumnName(i);

            const DbValueType benchmarkColType = benchmarkTableStmt.GetColumnType(i);
            const DbValueType actualColType = actualTableStmt.GetColumnType(i);
            EXPECT_EQ(benchmarkColType, actualColType) << "Table: " << tableName << " Col: " << colName << " Row: " << rowCount;

            switch (benchmarkColType)
                {
                    case DbValueType::NullVal:
                    {
                    if (benchmarkTableStmt.IsColumnNull(i) != actualTableStmt.IsColumnNull(i))
                        {
                        EXPECT_EQ(benchmarkTableStmt.IsColumnNull(i), actualTableStmt.IsColumnNull(i)) << "Table: " << tableName << " Col: " << colName << " Row: " << rowCount;
                        return false;
                        }

                    break;
                    }

                    case DbValueType::BlobVal:
                    {
                    const int benchmarkBlobSize = benchmarkTableStmt.GetColumnBytes(i);
                    const int actualBlobSize = actualTableStmt.GetColumnBytes(i);
                    if (benchmarkBlobSize != actualBlobSize)
                        {
                        EXPECT_EQ(benchmarkBlobSize, actualBlobSize) << "Table: " << tableName << " Col: " << colName << " Row: " << rowCount;
                        return false;
                        }

                    void const* benchmarkValue = benchmarkTableStmt.GetValueBlob(i);
                    void const* actualValue = actualTableStmt.GetValueBlob(i);
                    if (0 != memcmp(benchmarkValue, actualValue, (size_t) benchmarkBlobSize))
                        {
                        EXPECT_EQ(0, memcmp(benchmarkValue, actualValue, (size_t) benchmarkBlobSize)) << "Table: " << tableName << " Col: " << colName << " Row: " << rowCount;
                        return false;
                        }
                    break;
                    }

                    case DbValueType::FloatVal:
                    {
                    double benchmarkValue = benchmarkTableStmt.GetValueDouble(i);
                    double actualValue = actualTableStmt.GetValueDouble(i);

                    if (fabs(benchmarkValue - actualValue) > BeNumerical::ComputeComparisonTolerance(benchmarkValue, actualValue))
                        {
                        EXPECT_DOUBLE_EQ(benchmarkValue, actualValue) << "Table: " << tableName << " Col: " << colName << " Row: " << rowCount;
                        return false;
                        }
                    break;
                    }

                    case DbValueType::IntegerVal:
                    {
                    int64_t benchmarkValue = benchmarkTableStmt.GetValueInt64(i);
                    int64_t actualValue = actualTableStmt.GetValueInt64(i);

                    if (benchmarkValue != actualValue)
                        {
                        EXPECT_EQ(benchmarkValue, actualValue) << "Table: " << tableName << " Col: " << colName << " Row: " << rowCount;
                        return false;
                        }
                    break;
                    }

                    case DbValueType::TextVal:
                    {
                    Utf8CP benchmarkValue = benchmarkTableStmt.GetValueText(i);
                    Utf8CP actualValue = actualTableStmt.GetValueText(i);

                    if (0 != strcmp(benchmarkValue, actualValue))
                        {
                        EXPECT_STREQ(benchmarkValue, actualValue) << "Table: " << tableName << " Col: " << colName << " Row: " << rowCount;
                        return false;
                        }

                    break;
                    }

                    default:
                        return false;
                }
            }
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
BentleyStatus FileFormatCompatibilityTests::SetupTestFile(Utf8CP fileName, BeFileNameCR benchmarkFolder, DbCR benchmarkFile)
    {
    BeFileName bisSchemaFolder(benchmarkFolder);
    bisSchemaFolder.AppendToPath(L"schemas").AppendToPath(L"dgndb");

    BeFileName domainSchemaFolder(benchmarkFolder);
    domainSchemaFolder.AppendToPath(L"schemas").AppendToPath(L"domains");

    if (SUCCESS != CreateFakeBimFile(fileName, bisSchemaFolder, benchmarkFile))
        return ERROR;

    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    {
    Db db;
    if (BE_SQLITE_OK != db.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite)))
        return ERROR;

    uint64_t initialId;
    if (BE_SQLITE_ROW != benchmarkFile.QueryProperty(&initialId, sizeof(uint64_t), PropertySpec("InitialClassId_BisDomains", "ECDb_FileFormatCompatiblity_Test")))
        {
        EXPECT_TRUE(false) << "Benchmark file " << benchmarkFile.GetDbFileName() << " must contain BeProp 'ECDb_FileFormatCompatiblity_Test:InitialClassId_BisDomains";
        return ERROR;
        }

    if (BE_SQLITE_DONE != db.SaveBriefcaseLocalValue("ec_classidsequence", initialId))
        return ERROR;

    if (BE_SQLITE_OK != db.SaveChanges())
        return ERROR;
    }

    if (BE_SQLITE_OK != OpenECDb(filePath))
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
BentleyStatus FileFormatCompatibilityTests::CreateFakeBimFile(Utf8CP fileName, BeFileNameCR bisSchemaFolder, DbCR benchmarkFile)
    {
    if (BE_SQLITE_OK != SetupECDb(fileName))
        return ERROR;

    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    {
    Db db;
    if (BE_SQLITE_OK != db.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite)))
        return ERROR;

    uint64_t initialId;
    if (BE_SQLITE_ROW != benchmarkFile.QueryProperty(&initialId, sizeof(uint64_t), PropertySpec("InitialClassId_BisCore", "ECDb_FileFormatCompatiblity_Test")))
        {
        EXPECT_TRUE(false) << "Benchmark file " << benchmarkFile.GetDbFileName() << " must contain BeProp 'ECDb_FileFormatCompatiblity_Test:InitialClassId_BisCore";
        return ERROR;
        }

    if (BE_SQLITE_DONE != db.SaveBriefcaseLocalValue("ec_classidsequence", initialId))
        return ERROR;

    if (BE_SQLITE_OK != db.SaveChanges())
        return ERROR;
    }

    if (BE_SQLITE_OK != OpenECDb(filePath))
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

