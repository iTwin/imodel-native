/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/ElementInsertPerformance.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTestFixture.h"
#include "PerformanceElementsCRUDTests.h"

//=======================================================================================
// @bsiclass                                                     Krischan.Eberle      06/15
//=======================================================================================
struct PerformanceElementTestFixture : public DgnDbTestFixture
    {
    protected:
        static const DgnAuthorityId s_codeAuthorityId;
        static const int s_instanceCount = 500000;
        static Utf8CP const s_textVal;
        static const int64_t s_int64Val = 20000000LL;
        static const double s_doubleVal;
        Utf8CP const s_testSchemaXml =
            "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
            "  <ECSchemaReference name = 'dgn' version = '02.00' prefix = 'dgn' />"
            "  <ECClass typeName='Element1' >"
            "    <ECCustomAttributes>"
            "       <ClassHasHandler xmlns=\"BisCore.01.00\" />"
            "    </ECCustomAttributes>"
            "    <BaseClass>dgn:PhysicalElement</BaseClass>"
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
            "    <BaseClass>dgn:Element</BaseClass>"
            "  </ECClass>"
            "</ECSchema>";

        BentleyStatus ImportTestSchema() const;
        SpatialModelPtr CreateSpatialModel() const;
    };

//static
const DgnAuthorityId PerformanceElementTestFixture::s_codeAuthorityId = DgnAuthorityId((uint64_t) 1);
Utf8CP const PerformanceElementTestFixture::s_textVal = "bla bla";
const double PerformanceElementTestFixture::s_doubleVal = -3.1415;

//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformanceElementTestFixture::ImportTestSchema() const
    {
    ECN::ECSchemaReadContextPtr schemaContext = ECN::ECSchemaReadContext::CreateContext();
    BeFileName searchDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(searchDir);
    searchDir.AppendToPath(L"ECSchemas").AppendToPath(L"Dgn");
    schemaContext->AddSchemaPath(searchDir.GetName());

    ECN::ECSchemaPtr schema = nullptr;
    if (ECN::SchemaReadStatus::Success != ECN::ECSchema::ReadFromXmlString(schema, s_testSchemaXml, *schemaContext))
        return ERROR;

    if (SUCCESS != m_db->Schemas().ImportECSchemas(schemaContext->GetCache()))
        return ERROR;

    m_db->ClearECDbCache();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
SpatialModelPtr PerformanceElementTestFixture::CreateSpatialModel() const
    {
    DgnClassId mclassId = DgnClassId(m_db->Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_SpatialModel));
    SpatialModelPtr targetModel = new SpatialModel(SpatialModel::CreateParams(*m_db, mclassId, DgnModel::CreateModelCode("Instances")));
    EXPECT_EQ(DgnDbStatus::Success, targetModel->Insert());       /* Insert the new model into the DgnDb */
    return targetModel;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementTestFixture, ElementInsertInDbWithSingleInsertApproach)
    {
    SetupSeedProject();
    ASSERT_EQ(SUCCESS, ImportTestSchema());

    SpatialModelPtr model = CreateSpatialModel();
    ASSERT_TRUE(model != nullptr);
    DgnModelId modelId = model->GetModelId();
    ASSERT_TRUE(modelId.IsValid());

    DgnCategoryId catid = DgnCategory::QueryHighestCategoryId(*m_db);

    StopWatch timer(true);

    Utf8String code;
    for (int i = 0; i < s_instanceCount; i++)
        {
        //Call GetPreparedECSqlStatement for each instance (instead of once before) to insert as this is closer to the real world scenario
        CachedECSqlStatementPtr insertStmt = m_db->GetPreparedECSqlStatement("INSERT INTO ts.Element4 (ECInstanceId,ModelId,[CodeAuthorityId],[CodeValue],[CodeNamespace],CategoryId,"
                                                                             "Prop1_1,Prop1_2,Prop1_3,"
                                                                             "Prop2_1,Prop2_2,Prop2_3,"
                                                                             "Prop3_1,Prop3_2,Prop3_3,"
                                                                             "Prop4_1,Prop4_2,Prop4_3) "
                                                                             "VALUES (?,?,?,?,'',?,?,?,?,?,?,?,?,?,?,?,?,?)");
        ASSERT_TRUE(insertStmt != nullptr);
        CachedECSqlStatement& stmt = *insertStmt;

        const ECInstanceId id((uint64_t) (i + 10));
        stmt.BindId(1, id);
        stmt.BindId(2, modelId);
        stmt.BindId(3, s_codeAuthorityId);
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
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "Single Insert (numeric parameters)", s_instanceCount);
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementTestFixture, ElementInsertInDbWithInsertUpdateApproach)
    {
    SetupSeedProject();
    ASSERT_EQ(SUCCESS, ImportTestSchema());

    SpatialModelPtr model = CreateSpatialModel();
    ASSERT_TRUE(model != nullptr);
    DgnModelId modelId = model->GetModelId();
    ASSERT_TRUE(modelId.IsValid());

    DgnCategoryId catid = DgnCategory::QueryHighestCategoryId(*m_db);

    StopWatch timer(true);

    Utf8String code;
    for (int i = 0; i < s_instanceCount; i++)
        {
        //Call GetPreparedECSqlStatement for each instance (instead of once before) to insert as this is closer to the real world scenario
        CachedECSqlStatementPtr insertStmt = m_db->GetPreparedECSqlStatement("INSERT INTO ts.Element4 (ECInstanceId,ModelId,[CodeAuthorityId],[CodeValue],[CodeNamespace], CategoryId) VALUES (?,?,?,?,'',?)");
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

        const ECInstanceId id((uint64_t) (i + 10));
        insertStmt->BindId(1, id);
        insertStmt->BindId(2, modelId);
        insertStmt->BindId(3, s_codeAuthorityId);
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
            updateStmt->BindId(4, newKey.GetECInstanceId());
            ASSERT_EQ(BE_SQLITE_DONE, updateStmt->Step());
            updateStmt->Reset();
            updateStmt->ClearBindings();
            }
        }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "Insert & Update sub props", s_instanceCount);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementTestFixture, ElementInsertInDbWithSingleInsertApproachNamedParameters)
    {
    SetupSeedProject();
    ASSERT_EQ(SUCCESS, ImportTestSchema());

    SpatialModelPtr model = CreateSpatialModel();
    ASSERT_TRUE(model != nullptr);
    DgnModelId modelId = model->GetModelId();
    ASSERT_TRUE(modelId.IsValid());

    DgnCategoryId catid = DgnCategory::QueryHighestCategoryId(*m_db);

    StopWatch timer(true);
    Utf8String code;
    for (int i = 0; i < s_instanceCount; i++)
        {
        //Call GetPreparedECSqlStatement for each instance (instead of once before) to insert as this is closer to the real world scenario
        CachedECSqlStatementPtr insertStmt = m_db->GetPreparedECSqlStatement("INSERT INTO ts.Element4 (ECInstanceId,ModelId,[CodeAuthorityId],[CodeValue],[CodeNamespace], CategoryId,"
                                                                             "Prop1_1,Prop1_2,Prop1_3,"
                                                                             "Prop2_1,Prop2_2,Prop2_3,"
                                                                             "Prop3_1,Prop3_2,Prop3_3,"
                                                                             "Prop4_1,Prop4_2,Prop4_3) "
                                                                             "VALUES (:ecinstanceid,:modelid,:authorityid,:code,'',:categoryid,"
                                                                             ":p11,:p12,:p13,"
                                                                             ":p21,:p22,:p23,"
                                                                             ":p31,:p32,:p33,"
                                                                             ":p41,:p42,:p43)");
        ASSERT_TRUE(insertStmt != nullptr);

        CachedECSqlStatement& stmt = *insertStmt;

        const ECInstanceId id((uint64_t) (i + 10));
        stmt.BindId(stmt.GetParameterIndex("ecinstanceid"), id);
        stmt.BindId(stmt.GetParameterIndex("modelid"), modelId);
        stmt.BindId(stmt.GetParameterIndex("authorityid"), s_codeAuthorityId);
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
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "Single Insert (named parameters)", s_instanceCount);
    }

struct PerformanceElementsTests : PerformanceElementsCRUDTestFixture
    {};

static bool needSqlECSql = true;
//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsTests, ElementsInsert)
    {
    int initInstanceCount = 10000;
    int insertCount = 1000;
    int opCount;

    for (int i = 1; i <= 3; i++)
        {
        opCount = insertCount*i;
        if (needSqlECSql)
            {
            //SqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, initInstanceCount, opCount);
            //SqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, initInstanceCount, opCount);
            //SqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, initInstanceCount, opCount);
            //SqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, initInstanceCount, opCount);

            ECSqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, initInstanceCount, opCount);
            ECSqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, initInstanceCount, opCount);
            ECSqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, initInstanceCount, opCount);
            ECSqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, initInstanceCount, opCount);
            }

        ApiInsertTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, initInstanceCount, opCount);
        ApiInsertTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, initInstanceCount, opCount);
        ApiInsertTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, initInstanceCount, opCount);
        ApiInsertTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, initInstanceCount, opCount);
        }

    initInstanceCount = 100000;
    for (int i = 1; i <= 3; i++)
        {
        opCount = insertCount*i;
        if (needSqlECSql)
            {
            //SqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, initInstanceCount, opCount);
            //SqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, initInstanceCount, opCount);
            //SqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, initInstanceCount, opCount);
            //SqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, initInstanceCount, opCount);

            ECSqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, initInstanceCount, opCount);
            ECSqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, initInstanceCount, opCount);
            ECSqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, initInstanceCount, opCount);
            ECSqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, initInstanceCount, opCount);
            }

        ApiInsertTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, initInstanceCount, opCount);
        ApiInsertTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, initInstanceCount, opCount);
        ApiInsertTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, initInstanceCount, opCount);
        ApiInsertTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, initInstanceCount, opCount);
        }

    initInstanceCount = 1000000;
    for (int i = 1; i <= 3; i++)
        {
        opCount = insertCount*i;
        if (needSqlECSql)
            {
            //SqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, initInstanceCount, opCount);
            //SqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, initInstanceCount, opCount);
            //SqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, initInstanceCount, opCount);
            //SqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, initInstanceCount, opCount);

            ECSqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, initInstanceCount, opCount);
            ECSqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, initInstanceCount, opCount);
            ECSqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, initInstanceCount, opCount);
            ECSqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, initInstanceCount, opCount);
            }

        ApiInsertTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, initInstanceCount, opCount);
        ApiInsertTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, initInstanceCount, opCount);
        ApiInsertTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, initInstanceCount, opCount);
        ApiInsertTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, initInstanceCount, opCount);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsTests, ElementsRead)
    {
    int initInstanceCount = 10000;
    int insertCount = 1000;
    int opCount;

    for (int i = 1; i <= 3; i++)
        {
        opCount = insertCount*i;
        if (needSqlECSql)
            {
            //SqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, false /* optimal SQL */, false, initInstanceCount, opCount);
            //SqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, false /* optimal SQL */, false, initInstanceCount, opCount);
            //SqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, false /* optimal SQL*/, false, initInstanceCount, opCount);
            //SqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, false /* optimal SQL */, false, initInstanceCount, opCount);

            ECSqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, false, initInstanceCount, opCount);
            ECSqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, false, initInstanceCount, opCount);
            ECSqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, false, initInstanceCount, opCount);
            ECSqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, false, initInstanceCount, opCount);
            }

        ApiSelectTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, initInstanceCount, opCount);
        ApiSelectTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, initInstanceCount, opCount);
        ApiSelectTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, initInstanceCount, opCount);
        ApiSelectTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, initInstanceCount, opCount);
        }

    initInstanceCount = 100000;
    for (int i = 1; i <= 3; i++)
        {
        opCount = insertCount*i;
        if (needSqlECSql)
            {
            //SqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, false /* optimal SQL */, false, initInstanceCount, opCount);
            //SqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, false /* optimal SQL */, false, initInstanceCount, opCount);
            //SqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, false /* optimal SQL */, false, initInstanceCount, opCount);
            //SqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, false /* optimal SQL */, false, initInstanceCount, opCount);

            ECSqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, false, initInstanceCount, opCount);
            ECSqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, false, initInstanceCount, opCount);
            ECSqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, false, initInstanceCount, opCount);
            ECSqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, false, initInstanceCount, opCount);
            }

        ApiSelectTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, initInstanceCount, opCount);
        ApiSelectTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, initInstanceCount, opCount);
        ApiSelectTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, initInstanceCount, opCount);
        ApiSelectTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, initInstanceCount, opCount);
        }

    initInstanceCount = 1000000;
    for (int i = 1; i <= 3; i++)
        {
        opCount = insertCount*i;
        if (needSqlECSql)
            {
            //SqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, false /* optimal SQL */, false, initInstanceCount, opCount);
            //SqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, false /* optimal SQL */, false, initInstanceCount, opCount);
            //SqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, false /* optimal SQL */, false, initInstanceCount, opCount);
            //SqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, false /* optimal SQL */, false, initInstanceCount, opCount);

            ECSqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, false, initInstanceCount, opCount);
            ECSqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, false, initInstanceCount, opCount);
            ECSqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, false, initInstanceCount, opCount);
            ECSqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, false, initInstanceCount, opCount);
            }

        ApiSelectTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, initInstanceCount, opCount);
        ApiSelectTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, initInstanceCount, opCount);
        ApiSelectTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, initInstanceCount, opCount);
        ApiSelectTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, initInstanceCount, opCount);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsTests, ElementsUpdate)
    {
    int initInstanceCount = 10000;
    int insertCount = 1000;
    int opCount;

    for (int i = 1; i <= 3; i++)
        {
        opCount = insertCount*i;
        if (needSqlECSql)
            {
            //SqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, false, initInstanceCount, opCount);
            //SqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, false, initInstanceCount, opCount);
            //SqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, false, initInstanceCount, opCount);
            //SqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, false, initInstanceCount, opCount);

            ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, false, initInstanceCount, opCount);
            ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, false, initInstanceCount, opCount);
            ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, false, initInstanceCount, opCount);
            ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, false, initInstanceCount, opCount);
            }

        ApiUpdateTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, initInstanceCount, opCount);
        ApiUpdateTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, initInstanceCount, opCount);
        ApiUpdateTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, initInstanceCount, opCount);
        ApiUpdateTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, initInstanceCount, opCount);
        }

    initInstanceCount = 100000;
    for (int i = 1; i <= 3; i++)
        {
        opCount = insertCount*i;
        if (needSqlECSql)
            {
            //SqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, false, initInstanceCount, opCount);
            //SqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, false, initInstanceCount, opCount);
            //SqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, false, initInstanceCount, opCount);
            //SqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, false, initInstanceCount, opCount);

            ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, false, initInstanceCount, opCount);
            ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, false, initInstanceCount, opCount);
            ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, false, initInstanceCount, opCount);
            ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, false, initInstanceCount, opCount);
            }

        ApiUpdateTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, initInstanceCount, opCount);
        ApiUpdateTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, initInstanceCount, opCount);
        ApiUpdateTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, initInstanceCount, opCount);
        ApiUpdateTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, initInstanceCount, opCount);
        }

    initInstanceCount = 1000000;
    for (int i = 1; i <= 3; i++)
        {
        opCount = insertCount*i;
        if (needSqlECSql)
            {
            //SqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, false, initInstanceCount, opCount);
            //SqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, false, initInstanceCount, opCount);
            //SqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, false, initInstanceCount, opCount);
            //SqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, false, initInstanceCount, opCount);

            ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, false, initInstanceCount, opCount);
            ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, false, initInstanceCount, opCount);
            ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, false, initInstanceCount, opCount);
            ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, false, initInstanceCount, opCount);
            }

        ApiUpdateTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, initInstanceCount, opCount);
        ApiUpdateTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, initInstanceCount, opCount);
        ApiUpdateTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, initInstanceCount, opCount);
        ApiUpdateTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, initInstanceCount, opCount);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsTests, ElementsDelete)
    {
    int initInstanceCount = 10000;
    int insertCount = 1000;
    int opCount;

    for (int i = 1; i <= 3; i++)
        {
        opCount = insertCount*i;
        if (needSqlECSql)
            {
            SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, false, initInstanceCount, opCount);
            SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, false, initInstanceCount, opCount);
            SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, false, initInstanceCount, opCount);
            SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, false, initInstanceCount, opCount);

            ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, false, initInstanceCount, opCount);
            ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, false, initInstanceCount, opCount);
            ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, false, initInstanceCount, opCount);
            ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, false, initInstanceCount, opCount);
            }

        ApiDeleteTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, initInstanceCount, opCount);
        ApiDeleteTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, initInstanceCount, opCount);
        ApiDeleteTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, initInstanceCount, opCount);
        ApiDeleteTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, initInstanceCount, opCount);
        }

    initInstanceCount = 100000;
    for (int i = 1; i <= 3; i++)
        {
        opCount = insertCount*i;
        if (needSqlECSql)
            {
            SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, false, initInstanceCount, opCount);
            SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, false, initInstanceCount, opCount);
            SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, false, initInstanceCount, opCount);
            SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, false, initInstanceCount, opCount);

            ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, false, initInstanceCount, opCount);
            ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, false, initInstanceCount, opCount);
            ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, false, initInstanceCount, opCount);
            ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, false, initInstanceCount, opCount);
            }

        ApiDeleteTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, initInstanceCount, opCount);
        ApiDeleteTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, initInstanceCount, opCount);
        ApiDeleteTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, initInstanceCount, opCount);
        ApiDeleteTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, initInstanceCount, opCount);
        }

    initInstanceCount = 1000000;
    for (int i = 1; i <= 3; i++)
        {
        opCount = insertCount*i;
        if (needSqlECSql)
            {
            SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, false, initInstanceCount, opCount);
            SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, false, initInstanceCount, opCount);
            SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, false, initInstanceCount, opCount);
            SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, false, initInstanceCount, opCount);

            ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, false, initInstanceCount, opCount);
            ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, false, initInstanceCount, opCount);
            ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, false, initInstanceCount, opCount);
            ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, false, initInstanceCount, opCount);
            }

        ApiDeleteTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, initInstanceCount, opCount);
        ApiDeleteTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, initInstanceCount, opCount);
        ApiDeleteTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, initInstanceCount, opCount);
        ApiDeleteTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, initInstanceCount, opCount);
        }
    }