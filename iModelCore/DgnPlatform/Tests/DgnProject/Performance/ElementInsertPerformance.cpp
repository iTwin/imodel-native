/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTestFixture.h"
#include "PerformanceElementsCRUDTests.h"
#include <array>

//=======================================================================================
// @bsiclass                                                     Krischan.Eberle      06/15
//=======================================================================================
struct PerformanceElementTestFixture : public DgnDbTestFixture
    {
    protected:
        static const CodeSpecId s_codeSpecId;
        static const int s_instanceCount = 50000;
        static Utf8CP const s_textVal;
        static const int64_t s_int64Val = 20000000LL;
        static const double s_doubleVal;
        Utf8CP const s_testSchemaXml =
            "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
            "  <ECSchemaReference name = 'BisCore' version = '01.00' prefix = 'bis' />"
            "  <ECClass typeName='Element1' >"
            "    <ECCustomAttributes>"
            "       <ClassHasHandler xmlns=\"BisCore.01.00\" />"
            "    </ECCustomAttributes>"
            "    <BaseClass>bis:PhysicalElement</BaseClass>"
            "    <ECProperty propertyName='Prop1_1' typeName='string' />"
            "    <ECProperty propertyName='Prop1_2' typeName='long' />"
            "    <ECProperty propertyName='Prop1_3' typeName='double' />"
            "  </ECClass>"
            "  <ECClass typeName='Element2' >"
            "    <ECCustomAttributes>"
            "       <ClassHasHandler xmlns=\"BisCore.01.00\" />"
            "    </ECCustomAttributes>"
            "    <BaseClass>Element1</BaseClass>"
            "    <ECProperty propertyName='Prop2_1' typeName='string' />"
            "    <ECProperty propertyName='Prop2_2' typeName='long' />"
            "    <ECProperty propertyName='Prop2_3' typeName='double' />"
            "  </ECClass>"
            "  <ECClass typeName='Element3' >"
            "    <ECCustomAttributes>"
            "       <ClassHasHandler xmlns=\"BisCore.01.00\" />"
            "    </ECCustomAttributes>"
            "    <BaseClass>Element2</BaseClass>"
            "    <ECProperty propertyName='Prop3_1' typeName='string' />"
            "    <ECProperty propertyName='Prop3_2' typeName='long' />"
            "    <ECProperty propertyName='Prop3_3' typeName='double' />"
            "  </ECClass>"
            "  <ECClass typeName='Element4' >"
            "    <ECCustomAttributes>"
            "       <ClassHasHandler xmlns=\"BisCore.01.00\" />"
            "    </ECCustomAttributes>"
            "    <BaseClass>Element3</BaseClass>"
            "    <ECProperty propertyName='Prop4_1' typeName='string' />"
            "    <ECProperty propertyName='Prop4_2' typeName='long' />"
            "    <ECProperty propertyName='Prop4_3' typeName='double' />"
            "  </ECClass>"
            "  <ECClass typeName='Element4b' >"
            "    <ECCustomAttributes>"
            "       <ClassHasHandler xmlns=\"BisCore.01.00\" />"
            "    </ECCustomAttributes>"
            "    <BaseClass>Element3</BaseClass>"
            "    <ECProperty propertyName='Prop4b_1' typeName='string' />"
            "    <ECProperty propertyName='Prop4b_2' typeName='long' />"
            "    <ECProperty propertyName='Prop4b_3' typeName='double' />"
            "    <ECProperty propertyName='Prop4b_4' typeName='point3d' />"
            "  </ECClass>"
            "  <ECClass typeName='SimpleElement'>"
            "    <ECCustomAttributes>"
            "       <ClassHasHandler xmlns=\"BisCore.01.00\" />"
            "    </ECCustomAttributes>"
            "    <BaseClass>bis:Element</BaseClass>"
            "  </ECClass>"
            "</ECSchema>";

        BentleyStatus ImportTestSchema() const;
    };

//static
const CodeSpecId PerformanceElementTestFixture::s_codeSpecId = CodeSpecId((uint64_t) 1);
Utf8CP const PerformanceElementTestFixture::s_textVal = "bla bla";
const double PerformanceElementTestFixture::s_doubleVal = -3.1415;

//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformanceElementTestFixture::ImportTestSchema() const
    {
    ECN::ECSchemaReadContextPtr schemaContext = ECN::ECSchemaReadContext::CreateContext();
    schemaContext->AddSchemaLocater(m_db->GetSchemaLocater());

    BeFileName searchDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(searchDir);
    searchDir.AppendToPath(L"ECSchemas").AppendToPath(L"Dgn");
    schemaContext->AddSchemaPath(searchDir.GetName());

    ECN::ECSchemaPtr schema = nullptr;
    if (ECN::SchemaReadStatus::Success != ECN::ECSchema::ReadFromXmlString(schema, s_testSchemaXml, *schemaContext))
        return ERROR;

    if (SchemaStatus::Success != m_db->ImportSchemas(schemaContext->GetCache().GetSchemas()))
        return ERROR;

    m_db->SaveChanges();
    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementTestFixture, ElementInsertInDbWithSingleInsertApproach)
    {
    SetupSeedProject();
    ASSERT_EQ(SUCCESS, ImportTestSchema());

    PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*m_db, "Instances");
    DgnModelId modelId = model->GetModelId();
    ASSERT_TRUE(modelId.IsValid());

    DgnCategoryId catid = DgnDbTestUtils::InsertSpatialCategory(*m_db, TEST_NAME);

    StopWatch timer(true);

    Utf8String code;
    for (int i = 0; i < s_instanceCount; i++)
        {
        //Call GetPreparedECSqlStatement for each instance (instead of once before) to insert as this is closer to the real world scenario
        CachedECSqlStatementPtr insertStmt = m_db->GetPreparedECSqlStatement("INSERT INTO ts.Element4 (ECInstanceId,Model.Id,CodeSpec.Id,CodeValue,CodeScope,Category.Id,"
                                                                             "Prop1_1,Prop1_2,Prop1_3,"
                                                                             "Prop2_1,Prop2_2,Prop2_3,"
                                                                             "Prop3_1,Prop3_2,Prop3_3,"
                                                                             "Prop4_1,Prop4_2,Prop4_3) "
                                                                             "VALUES (?,?,?,?,'',?,?,?,?,?,?,?,?,?,?,?,?,?)");
        ASSERT_TRUE(insertStmt != nullptr);
        CachedECSqlStatement& stmt = *insertStmt;

        const ECInstanceId id((uint64_t) (i + 100));
        stmt.BindId(1, id);
        stmt.BindId(2, modelId);
        stmt.BindId(3, s_codeSpecId);
        code.Sprintf("Id-%d", i);
        stmt.BindText(4, code.c_str(), IECSqlBinder::MakeCopy::No);
        stmt.BindId(5, catid);

        stmt.BindText(6, s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(7, s_int64Val);
        stmt.BindDouble(8, s_doubleVal);

        stmt.BindText(9, s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(10, s_int64Val);
        stmt.BindDouble(11, s_doubleVal);

        stmt.BindText(12, s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(13, s_int64Val);
        stmt.BindDouble(14, s_doubleVal);

        stmt.BindText(15, s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(16, s_int64Val);
        stmt.BindDouble(17, s_doubleVal);

        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

        stmt.Reset();
        stmt.ClearBindings();
        }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_instanceCount, "Single Insert (numeric parameters)");
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementTestFixture, ElementInsertInDbWithInsertUpdateApproach)
    {
    SetupSeedProject();
    ASSERT_EQ(SUCCESS, ImportTestSchema());

    PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*m_db, "Instances");
    DgnModelId modelId = model->GetModelId();
    ASSERT_TRUE(modelId.IsValid());

    DgnCategoryId catid = DgnDbTestUtils::InsertSpatialCategory(*m_db, TEST_NAME);

    StopWatch timer(true);

    Utf8String code;
    for (int i = 0; i < s_instanceCount; i++)
        {
        //Call GetPreparedECSqlStatement for each instance (instead of once before) to insert as this is closer to the real world scenario
        CachedECSqlStatementPtr insertStmt = m_db->GetPreparedECSqlStatement("INSERT INTO ts.Element4 (ECInstanceId,Model.Id,CodeSpec.Id,CodeValue,CodeScope, Category.Id) VALUES (?,?,?,?,'',?)");
        ASSERT_TRUE(insertStmt != nullptr);

        std::vector<CachedECSqlStatementPtr> updateStmts;
        CachedECSqlStatementPtr updateStmt = m_db->GetPreparedECSqlStatement("UPDATE ONLY ts.Element1 SET Prop1_1 = ?, Prop1_2 = ?, Prop1_3 = ? WHERE ECInstanceId=?");
        ASSERT_TRUE(updateStmt != nullptr);
        updateStmts.push_back(updateStmt);

        updateStmt = m_db->GetPreparedECSqlStatement("UPDATE ONLY ts.Element2 SET Prop2_1 = ?, Prop2_2 = ?, Prop2_3 = ? WHERE ECInstanceId=?");
        ASSERT_TRUE(updateStmt != nullptr);
        updateStmts.push_back(updateStmt);

        updateStmt = m_db->GetPreparedECSqlStatement("UPDATE ONLY ts.Element3 SET Prop3_1 = ?, Prop3_2 = ?, Prop3_3 = ? WHERE ECInstanceId=?");
        ASSERT_TRUE(updateStmt != nullptr);
        updateStmts.push_back(updateStmt);

        updateStmt = m_db->GetPreparedECSqlStatement("UPDATE ONLY ts.Element4 SET Prop4_1 = ?, Prop4_2 = ?, Prop4_3 = ? WHERE ECInstanceId=?");
        ASSERT_TRUE(updateStmt != nullptr);
        updateStmts.push_back(updateStmt);

        const ECInstanceId id((uint64_t) (i + 100));
        insertStmt->BindId(1, id);
        insertStmt->BindId(2, modelId);
        insertStmt->BindId(3, s_codeSpecId);
        code.Sprintf("Id-%d", i);
        insertStmt->BindText(4, code.c_str(), IECSqlBinder::MakeCopy::No);

        insertStmt->BindId(5, catid);

        ECInstanceKey newKey;
        ASSERT_EQ(BE_SQLITE_DONE, insertStmt->Step(newKey));
        insertStmt->Reset();
        insertStmt->ClearBindings();
        m_db->SaveChanges();//Save changes to Db otherwise Update operations will simply be performed In-Memory

        for (CachedECSqlStatementPtr& updateStmt : updateStmts)
            {
            updateStmt->BindText(1, s_textVal, IECSqlBinder::MakeCopy::No);
            updateStmt->BindInt64(2, s_int64Val);
            updateStmt->BindDouble(3, s_doubleVal);
            updateStmt->BindId(4, newKey.GetInstanceId());
            ASSERT_EQ(BE_SQLITE_DONE, updateStmt->Step());
            updateStmt->Reset();
            updateStmt->ClearBindings();
            }
        }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_instanceCount, "Insert and Update sub props");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementTestFixture, ElementInsertInDbWithSingleInsertApproachNamedParameters)
    {
    SetupSeedProject();
    ASSERT_EQ(SUCCESS, ImportTestSchema());

    PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*m_db, "Instances");
    DgnModelId modelId = model->GetModelId();
    ASSERT_TRUE(modelId.IsValid());

    DgnCategoryId catid = DgnDbTestUtils::InsertSpatialCategory(*m_db, TEST_NAME);

    StopWatch timer(true);
    Utf8String code;
    for (int i = 0; i < s_instanceCount; i++)
        {
        //Call GetPreparedECSqlStatement for each instance (instead of once before) to insert as this is closer to the real world scenario
        CachedECSqlStatementPtr insertStmt = m_db->GetPreparedECSqlStatement("INSERT INTO ts.Element4 (ECInstanceId,Model.Id,CodeSpec.Id,CodeValue,CodeScope,Category.Id,"
                                                                             "Prop1_1,Prop1_2,Prop1_3,"
                                                                             "Prop2_1,Prop2_2,Prop2_3,"
                                                                             "Prop3_1,Prop3_2,Prop3_3,"
                                                                             "Prop4_1,Prop4_2,Prop4_3) "
                                                                             "VALUES (:ecinstanceid,:modelid,:codespecid,:code,'',:categoryid,"
                                                                             ":p11,:p12,:p13,"
                                                                             ":p21,:p22,:p23,"
                                                                             ":p31,:p32,:p33,"
                                                                             ":p41,:p42,:p43)");
        ASSERT_TRUE(insertStmt != nullptr);

        CachedECSqlStatement& stmt = *insertStmt;

        const ECInstanceId id((uint64_t) (i + 100));
        stmt.BindId(stmt.GetParameterIndex("ecinstanceid"), id);
        stmt.BindId(stmt.GetParameterIndex("modelid"), modelId);
        stmt.BindId(stmt.GetParameterIndex("codespecid"), s_codeSpecId);
        code.Sprintf("Id-%d", i);
        stmt.BindText(stmt.GetParameterIndex("code"), code.c_str(), IECSqlBinder::MakeCopy::No);
        stmt.BindId(stmt.GetParameterIndex("categoryid"), catid);

        stmt.BindText(stmt.GetParameterIndex("p11"), s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(stmt.GetParameterIndex("p12"), s_int64Val);
        stmt.BindDouble(stmt.GetParameterIndex("p13"), s_doubleVal);

        stmt.BindText(stmt.GetParameterIndex("p21"), s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(stmt.GetParameterIndex("p22"), s_int64Val);
        stmt.BindDouble(stmt.GetParameterIndex("p23"), s_doubleVal);

        stmt.BindText(stmt.GetParameterIndex("p31"), s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(stmt.GetParameterIndex("p32"), s_int64Val);
        stmt.BindDouble(stmt.GetParameterIndex("p33"), s_doubleVal);

        stmt.BindText(stmt.GetParameterIndex("p41"), s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(stmt.GetParameterIndex("p42"), s_int64Val);
        stmt.BindDouble(stmt.GetParameterIndex("p43"), s_doubleVal);

        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

        stmt.Reset();
        stmt.ClearBindings();
        }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), s_instanceCount, "Single Insert (named parameters)");
    }

struct PerformanceElementsTests : PerformanceElementsCRUDTestFixture
    {
    enum class Op
        {
        Select,
        Insert,
        Update,
        Delete
        };

    static std::vector<Utf8String> GetClasses()
        {
        std::vector<Utf8String> classes;
        classes.push_back(PERF_TEST_PERFELEMENT_CLASS_NAME);
        classes.push_back(PERF_TEST_PERFELEMENTSUB1_CLASS_NAME);
        classes.push_back(PERF_TEST_PERFELEMENTSUB2_CLASS_NAME);
        classes.push_back(PERF_TEST_PERFELEMENTSUB3_CLASS_NAME);
        return classes;
        }
    static std::array<int, 3> GetInitalInstanceCount()
        {
        return  {10000, 100000, 1000000};
        }
    static std::array<int, 3> GetOpCount() { return  {1000, 2000, 3000}; }
    void Execute(Op op)
        {
        //WaitForUserInput();
        for (int initalInstanceCount : GetInitalInstanceCount())
            {
            if (initalInstanceCount <= 0)
                continue;

            for (int opCount : GetOpCount())
                {
                if (opCount <= 0)
                    continue;

                for (Utf8StringCR perfClass : GetClasses())
                    {
                    if (perfClass.empty())
                        continue;

                    if (op == Op::Select)
                        ApiSelectTime(perfClass.c_str(), initalInstanceCount, opCount);
                    else if (op == Op::Insert)
                        ApiInsertTime(perfClass.c_str(), initalInstanceCount, opCount);
                    else if (op == Op::Update)
                        ApiUpdateTime(perfClass.c_str(), initalInstanceCount, opCount);
                    else if (op == Op::Delete)
                        ApiDeleteTime(perfClass.c_str(), initalInstanceCount, opCount);
                    else
                        {
                        ASSERT_TRUE(false);
                        }
                    }
                }

            //printf("%s\n", GetDbSettings().c_str());
            }
        }
    };

#if 0
//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    10/2016
//---------------------------------------------------------------------------------------
TEST_F(PerformanceElementsTests, ElementsInsertStrategies)
    {
    ApiInsertTime(PERF_TEST_PERFELEMENTSUB3_CLASS_NAME, 0, 500 * 1000 + 0, false, 0); // null FederationGuid, sequential IDs
    ApiInsertTime(PERF_TEST_PERFELEMENTSUB3_CLASS_NAME, 0, 500 * 1000 + 1, true, 0); // random FederationGuid, sequential IDs
    ApiInsertTime(PERF_TEST_PERFELEMENTSUB3_CLASS_NAME, 0, 500 * 1000 + 2, false, 1); // null FederationGuid, time-based IDs
    ApiInsertTime(PERF_TEST_PERFELEMENTSUB3_CLASS_NAME, 0, 500 * 1000 + 3, true, 1); // random FederationGuid, time-based IDs
    ApiInsertTime(PERF_TEST_PERFELEMENTSUB3_CLASS_NAME, 0, 500 * 1000 + 4, false, 2); // null FederationGuid, alternating briefcase IDs
    ApiInsertTime(PERF_TEST_PERFELEMENTSUB3_CLASS_NAME, 0, 500 * 1000 + 5, true, 2); // random FederationGuid, alternating briefcase IDs
    }
#endif

//Pragma("cache_size=200000");
//Pragma("temp_store=MEMORY");

/*---------------------------------------------------------------------------------**//**
// @betest                                     Affan.Khan                       10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceElementsTests, ElementsInsert) { Execute(Op::Insert); }
/*---------------------------------------------------------------------------------**//**
// @betest                                     Affan.Khan                       10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceElementsTests, ElementsRead)   { Execute(Op::Select); }
/*---------------------------------------------------------------------------------**//**
// @betest                                     Affan.Khan                       10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceElementsTests, ElementsUpdate) { Execute(Op::Update); }
/*---------------------------------------------------------------------------------**//**
// @betest                                     Affan.Khan                       10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceElementsTests, ElementsDelete) { Execute(Op::Delete); }

#define CACHE_SPILL Pragma ("cache_spill = off")
#define CACHE_SIZE  Pragma (SqlPrintfString ("cache_size = %d", 300000));

/*---------------------------------------------------------------------------------**//**
// @betest                                     Affan.Khan                       10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceElementsTests, ServerElementsInsert) 
    { 
    CACHE_SPILL;
    CACHE_SIZE;
    Execute(Op::Insert); 
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                     Affan.Khan                       10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceElementsTests, ServerElementsRead)   
    { 
    CACHE_SPILL;
    CACHE_SIZE;
    Execute(Op::Select);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                     Affan.Khan                       10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceElementsTests, ServerElementsUpdate) 
    { 
    CACHE_SPILL;
    CACHE_SIZE;
    Execute(Op::Update);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                     Affan.Khan                       10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceElementsTests, ServerElementsDelete) 
    { 
    CACHE_SPILL;
    CACHE_SIZE;
    Execute(Op::Delete);
    }


/*---------------------------------------------------------------------------------**//**
// @beClass                                     Maha Nasir                       05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct PerformanceFixtureCRUD : PerformanceElementsCRUDTestFixture
    {
    enum class Op
        {
        Select,
        Insert,
        Update,
        Delete
        };

    static std::vector<Utf8String> GetClasses()
        {
        std::vector<Utf8String> classes;
        classes.push_back(PERF_TEST_PERFELEMENT_CLASS_NAME);
        classes.push_back(PERF_TEST_PERFELEMENTSUB1_CLASS_NAME);
        classes.push_back(PERF_TEST_PERFELEMENTSUB2_CLASS_NAME);
        classes.push_back(PERF_TEST_PERFELEMENTSUB3_CLASS_NAME);
        return classes;
        }

    /*---------------------------------------------------------------------------------**//**
    //                                    Maha Nasir                       05/2018
    +---------------+---------------+---------------+---------------+---------------+------*/

    void Execute(Op op, int percentageOfInitialCount, int initalInstanceCount)
        {

        ASSERT_NE(0, initalInstanceCount) << "InitialCount should be greater than zero for the test to execute.";

        ASSERT_NE(0, percentageOfInitialCount) << "Percentage of the initial count should not be zero.";

        //Calculating the %age of the initial count to be used as OperationCount
        int opCount = initalInstanceCount * percentageOfInitialCount / 100;

        for (Utf8StringCR perfClass : GetClasses())
            {
            if (perfClass.empty())
                continue;

            if (op == Op::Select)
                ApiSelectTime(perfClass.c_str(), initalInstanceCount, opCount);
            else if (op == Op::Insert)
                ApiInsertTime(perfClass.c_str(), initalInstanceCount, opCount);
            else if (op == Op::Update)
                ApiUpdateTime(perfClass.c_str(), initalInstanceCount, opCount);
            else if (op == Op::Delete)
                ApiDeleteTime(perfClass.c_str(), initalInstanceCount, opCount);
            else
                {
                ASSERT_TRUE(false);
                }
            }

        }

    /*---------------------------------------------------------------------------------**//**
    //                                     Maha Nasir                       05/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ParseJsonAndRunTest(BeFileName jsonFilePath, Op op)
        {
        BeFile file;
        if (BeFileStatus::Success != file.Open(jsonFilePath.c_str(), BeFileAccess::Read))
            return;

        ByteStream byteStream;
        if (BeFileStatus::Success != file.ReadEntireFile(byteStream))
            return;

        Utf8String json((Utf8CP)byteStream.GetData(), byteStream.GetSize());
        file.Close();

        Json::Reader reader;
        Json::Value value;

        if (reader.Parse(json, value))
            {
            auto requirements = value["CrudTestRequirement"];

            for (auto iter : requirements)
                {
                auto InitialCount = iter["InitialCount"].asInt();
                for (auto percentage : iter["InitialCountPercentage"])
                    {
                    switch (op) {

                        case Op::Insert:
                            Execute(Op::Insert, percentage.asInt(), InitialCount);
                            break;

                        case Op::Delete:
                            Execute(Op::Delete, percentage.asInt(), InitialCount);
                            break;

                        case Op::Select:
                            Execute(Op::Select, percentage.asInt(), InitialCount);
                            break;

                        case Op::Update:
                            Execute(Op::Update, percentage.asInt(), InitialCount);
                            break;

                        default:
                            printf("Invalid operation.");
                        }
                    }
                }
            }
        }

    };


/*---------------------------------------------------------------------------------**//**
// @betest                                     Maha Nasir                       05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceFixtureCRUD, ElementsInsert)
    {
    WString jsonFile;
    jsonFile.Sprintf(L"ElementCrud.json");

    BeFileName jsonFilePath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(jsonFilePath);
    jsonFilePath.AppendToPath(L"PerformanceTestData");
    jsonFilePath.AppendToPath(jsonFile.c_str());
    ASSERT_TRUE(jsonFilePath.DoesPathExist());

    ParseJsonAndRunTest(jsonFilePath, Op::Insert);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                     Maha Nasir                       05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceFixtureCRUD, ElementsDelete)
    {
    WString jsonFile;
    jsonFile.Sprintf(L"ElementCrud.json");

    BeFileName jsonFilePath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(jsonFilePath);
    jsonFilePath.AppendToPath(L"PerformanceTestData");
    jsonFilePath.AppendToPath(jsonFile.c_str());
    ASSERT_TRUE(jsonFilePath.DoesPathExist());

    ParseJsonAndRunTest(jsonFilePath, Op::Delete);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                     Maha Nasir                       05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceFixtureCRUD, ElementsUpdate)
    {
    WString jsonFile;
    jsonFile.Sprintf(L"ElementCrud.json");

    BeFileName jsonFilePath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(jsonFilePath);
    jsonFilePath.AppendToPath(L"PerformanceTestData");
    jsonFilePath.AppendToPath(jsonFile.c_str());
    ASSERT_TRUE(jsonFilePath.DoesPathExist());

    ParseJsonAndRunTest(jsonFilePath, Op::Update);
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                     Maha Nasir                       05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceFixtureCRUD, ElementsSelect)
    {
    WString jsonFile;
    jsonFile.Sprintf(L"ElementCrud.json");

    BeFileName jsonFilePath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(jsonFilePath);
    jsonFilePath.AppendToPath(L"PerformanceTestData");
    jsonFilePath.AppendToPath(jsonFile.c_str());
    ASSERT_TRUE(jsonFilePath.DoesPathExist());

    ParseJsonAndRunTest(jsonFilePath, Op::Select);
    }
