/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/PerformanceElementsCRUDTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceElementsCRUDTests.h"

// Uncomment this if you want elapsed time of each test case logged to console in addition to the log file.
// #define PERF_ELEM_CRUD_LOG_TO_CONSOLE 1

HANDLER_DEFINE_MEMBERS (PerformanceElement1Handler)
HANDLER_DEFINE_MEMBERS (PerformanceElement2Handler)
HANDLER_DEFINE_MEMBERS (PerformanceElement3Handler)
HANDLER_DEFINE_MEMBERS (PerformanceElement4Handler)

DOMAIN_DEFINE_MEMBERS (PerformanceElementTestDomain)

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
PerformanceElementTestDomain::PerformanceElementTestDomain() : DgnDomain(ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, "Test Schema", 1)
    {
    RegisterHandler(PerformanceElement1Handler::GetHandler());
    RegisterHandler(PerformanceElement2Handler::GetHandler());
    RegisterHandler(PerformanceElement3Handler::GetHandler());
    RegisterHandler(PerformanceElement4Handler::GetHandler());
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::SetUpTestDgnDb(WCharCP destFileName, Utf8CP testClassName, int initialInstanceCount)
    {
    WString seedFileName;
    seedFileName.Sprintf(L"dgndb_ecsqlvssqlite_%d_%ls_seed%d.ibim", initialInstanceCount, WString(testClassName, BentleyCharEncoding::Utf8).c_str(), DateTime::GetCurrentTimeUtc().GetDayOfYear());

    BeFileName seedFilePath;
    BeTest::GetHost().GetOutputRoot(seedFilePath);
    seedFilePath.AppendToPath(seedFileName.c_str());

    if (!seedFilePath.DoesPathExist())
        {
        SetupSeedProject(seedFileName.c_str());
        ECN::ECSchemaReadContextPtr schemaContext = ECN::ECSchemaReadContext::CreateContext();
        BeFileName searchDir;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(searchDir);
        searchDir.AppendToPath(L"ECSchemas").AppendToPath(L"Dgn");
        schemaContext->AddSchemaPath(searchDir.GetName());

        ECN::ECSchemaPtr schema = nullptr;
        ASSERT_EQ (ECN::SchemaReadStatus::Success, ECN::ECSchema::ReadFromXmlString(schema, s_testSchemaXml, *schemaContext));

        schemaContext->AddSchema(*schema);
        ASSERT_EQ(DgnDbStatus::Success, DgnBaseDomain::GetDomain().ImportSchema(*m_db, schemaContext->GetCache()));
        ASSERT_TRUE (m_db->IsDbOpen());

        bvector<DgnElementPtr> testElements;
        CreateElements(initialInstanceCount, testClassName, testElements, "InitialInstances", true);
        DgnDbStatus stat = DgnDbStatus::Success;
        for (DgnElementPtr& element : testElements)
            {
            element->Insert(&stat);
            ASSERT_EQ (DgnDbStatus::Success, stat);
            }
        m_db->SaveChanges();
        m_db->CloseDb();
        }

    BeFileName dgndbFilePath;
    BeTest::GetHost().GetOutputRoot(dgndbFilePath);
    dgndbFilePath.AppendToPath(destFileName);

    ASSERT_EQ (BeFileNameStatus::Success, BeFileName::BeCopyFile(seedFilePath, dgndbFilePath, false));

    DbResult status;
    m_db = DgnDb::OpenDgnDb(&status, dgndbFilePath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    EXPECT_EQ (DbResult::BE_SQLITE_OK, status) << status;
    ASSERT_TRUE (m_db.IsValid());
    }

uint64_t PerformanceElementsCRUDTestFixture::s_elementId = UINT64_C(2000000);

Utf8CP const PerformanceElementsCRUDTestFixture::s_testSchemaXml =
    "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECSchemaReference name = 'dgn' version = '02.00' prefix = 'dgn' />"
        "  <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
        "  <ECClass typeName='Element1' >"
        "    <ECCustomAttributes>"
        "       <ClassHasHandler xmlns=\"dgn.02.00\" />"
        "    </ECCustomAttributes>"
        "    <BaseClass>dgn:PhysicalElement</BaseClass>"
        "    <ECProperty propertyName='Prop1_1' typeName='string' />"
        "    <ECProperty propertyName='Prop1_2' typeName='long' />"
        "    <ECProperty propertyName='Prop1_3' typeName='double' />"
        "  </ECClass>"
        "  <ECClass typeName='Element2' >"
        "    <ECCustomAttributes>"
        "       <ClassHasHandler xmlns=\"dgn.02.00\" />"
        "    </ECCustomAttributes>"
        "    <BaseClass>Element1</BaseClass>"
        "    <ECProperty propertyName='Prop2_1' typeName='string' />"
        "    <ECProperty propertyName='Prop2_2' typeName='long' />"
        "    <ECProperty propertyName='Prop2_3' typeName='double' />"
        "  </ECClass>"
        "  <ECClass typeName='Element3' >"
        "    <ECCustomAttributes>"
        "       <ClassHasHandler xmlns=\"dgn.02.00\" />"
        "    </ECCustomAttributes>"
        "    <BaseClass>Element2</BaseClass>"
        "    <ECProperty propertyName='Prop3_1' typeName='string' />"
        "    <ECProperty propertyName='Prop3_2' typeName='long' />"
        "    <ECProperty propertyName='Prop3_3' typeName='double' />"
        "  </ECClass>"
        "  <ECClass typeName='Element4' >"
        "    <ECCustomAttributes>"
        "       <ClassHasHandler xmlns=\"dgn.02.00\" />"
        "    </ECCustomAttributes>"
        "    <BaseClass>Element3</BaseClass>"
        "    <ECProperty propertyName='Prop4_1' typeName='string' />"
        "    <ECProperty propertyName='Prop4_2' typeName='long' />"
        "    <ECProperty propertyName='Prop4_3' typeName='double' />"
        "  </ECClass>"
        "  <ECClass typeName='Element4b' >"
        "    <ECCustomAttributes>"
        "       <ClassHasHandler xmlns=\"dgn.02.00\" />"
        "    </ECCustomAttributes>"
        "    <BaseClass>Element3</BaseClass>"
        "    <ECProperty propertyName='Prop4b_1' typeName='string' />"
        "    <ECProperty propertyName='Prop4b_2' typeName='long' />"
        "    <ECProperty propertyName='Prop4b_3' typeName='double' />"
        "    <ECProperty propertyName='Prop4b_4' typeName='point3d' />"
        "  </ECClass>"
        "  <ECClass typeName='TestMultiAspect' isDomainClass='True'>"
        "    <BaseClass>dgn:ElementMultiAspect</BaseClass>"
        "    <ECCustomAttributes>"
        "       <ClassMap xmlns = 'ECDbMap.01.00'>"
        "           <Indexes>"
        "               <DbIndex>"
        "                   <Name>IDX_TMAspect</Name>"
        "                   <IsUnique>False</IsUnique>"
        "                   <Properties>"
        "                       <string>ElementId</string>"
        "                   </Properties>"
        "               </DbIndex>"
        "           </Indexes>"
        "       </ClassMap>"
        "    </ECCustomAttributes>"
        "    <ECProperty propertyName='TestMultiAspectProperty' typeName='string' />"
        "  </ECClass>"
        "  <ECRelationshipClass typeName = 'ElementOwnsTestMultiAspect' strength = 'embedding'>"
        "    <ECCustomAttributes>"
        "      <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00'>"
        "        <OnDeleteAction>Cascade</OnDeleteAction>"
        "      </ForeignKeyRelationshipMap>"
        "    </ECCustomAttributes>"
        "    <Source cardinality = '(1,1)' polymorphic = 'true'>"
        "      <Class class = 'Element1' />"
        "    </Source>"
        "    <Target cardinality = '(0,N)' polymorphic = 'true'>"
        "      <Class class = 'TestMultiAspect'>"
        "        <Key>"
        "          <Property name = 'ElementId' />"
        "        </Key>"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>";

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement1::BindParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    if ((ECSqlStatus::Success != statement.BindText(statement.GetParameterIndex("Prop1_1"), m_prop1_1.c_str(), IECSqlBinder::MakeCopy::Yes)) ||
        (ECSqlStatus::Success != statement.BindInt64(statement.GetParameterIndex("Prop1_2"), m_prop1_2)) ||
        (ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex("Prop1_3"), m_prop1_3)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement1::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat = BindParams(statement);
    if (DgnDbStatus::Success != stat)
        return stat;

    return T_Super::_BindInsertParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus PerformanceElement1::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    EXPECT_EQ (DgnDbStatus::Success, T_Super::_ReadSelectParams(stmt, params));
    EXPECT_EQ (0, strcmp(stmt.GetValueText(params.GetSelectIndex("Prop1_1")), "Element1 - InitValue"));
    EXPECT_EQ (10000000, stmt.GetValueInt64(params.GetSelectIndex("Prop1_2")));
    EXPECT_EQ (-3.1415, stmt.GetValueDouble(params.GetSelectIndex("Prop1_3")));

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement1::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus status = T_Super::_BindUpdateParams(statement);
    if (DgnDbStatus::Success != status)
        return status;

    m_prop1_1 = "Element1 - UpdatedValue";
    m_prop1_2 = 20000000LL;
    m_prop1_3 = -6.283;

    return BindParams(statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement1Ptr PerformanceElement1::Create(Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, bool specifyPropertyValues)
    {
    if (specifyPropertyValues)
        return new PerformanceElement1 (PerformanceElement1::CreateParams (db, modelId, classId, category), "Element1 - InitValue", 10000000LL, -3.1415);

        return new PerformanceElement1 (PerformanceElement1::CreateParams (db, modelId, classId, category));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement1CPtr PerformanceElement1::Insert()
    {
    return GetDgnDb().Elements().Insert<PerformanceElement1> (*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement1CPtr PerformanceElement1::Update()
    {
    return GetDgnDb().Elements().Update<PerformanceElement1> (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
static bool appendEllipse3d(GeometryBuilder& builder, double cx, double cy, double cz)
    {
    DEllipse3d ellipseData = DEllipse3d::From(cx, cy, cz,
        0, 0, 2,
        0, 3, 0,
        0.0, Angle::TwoPi());
    ICurvePrimitivePtr ellipse = ICurvePrimitive::CreateArc(ellipseData);
    return builder.Append(*ellipse);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Majd.Uddin            01/2016
//---------------+---------------+---------------+---------------+---------------+-------
static bool appendSolidPrimitive(GeometryBuilder& builder, double dz, double radius)
{
    DgnConeDetail cylinderDetail(DPoint3d::From(0, 0, 0), DPoint3d::From(0, 0, dz), radius, radius, true);
    ISolidPrimitivePtr solidPrimitive = ISolidPrimitive::CreateDgnCone(cylinderDetail);
    GeometricPrimitivePtr elmGeom3 = GeometricPrimitive::Create(*solidPrimitive);
    EXPECT_TRUE(elmGeom3.IsValid());
    EXPECT_TRUE(GeometricPrimitive::GeometryType::SolidPrimitive == elmGeom3->GetGeometryType());
    ISolidPrimitivePtr getAsSolid = elmGeom3->GetAsISolidPrimitive();
    EXPECT_TRUE(getAsSolid.IsValid());
    
    return builder.Append(*getAsSolid);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestMultiAspect::_LoadProperties(DgnElementCR el)
{
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("SELECT TestMultiAspectProperty FROM %s WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()).c_str());
    stmt->BindId(1, GetAspectInstanceId());
    if (BE_SQLITE_ROW != stmt->Step())
        return DgnDbStatus::ReadError;
    m_testMultiAspectProperty = stmt->GetValueText(0);
    return DgnDbStatus::Success;
}

static bool useEllipse = true;
//---------------------------------------------------------------------------------------
// @bsimethod                                   Majd.Uddin            12/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElement1::AddGeomtry()
{
    GeometrySourceP geomElem = ToGeometrySourceP();
    GeometryBuilderPtr builder = GeometryBuilder::Create(*GetModel(), GetCategoryId(), DPoint3d::From(0.0, 0.0, 0.0));
    if (useEllipse)
        ASSERT_TRUE(appendEllipse3d(*builder, 1, 2, 3));
    else
        ASSERT_TRUE(appendSolidPrimitive(*builder, 3.0, 1.5));
    ASSERT_EQ(SUCCESS, builder->SetGeometryStreamAndPlacement(*geomElem));

    ASSERT_TRUE(HasGeometry());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceElement1::ExtendGeometry()
    {
    GeometrySourceP geomElem = ToGeometrySourceP();
    GeometryBuilderPtr builder = GeometryBuilder::Create(*GetModel(), GetCategoryId(), DPoint3d::From(0.0, 0.0, 0.0));
    if (useEllipse)
        ASSERT_TRUE(appendEllipse3d(*builder, 3, 2, 1));
    else
        ASSERT_TRUE(appendSolidPrimitive(*builder, 6.0, 3.0));

    ASSERT_EQ(SUCCESS, builder->SetGeometryStreamAndPlacement(*geomElem));
    ASSERT_TRUE(HasGeometry());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement2::BindParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    if ((ECSqlStatus::Success != statement.BindText(statement.GetParameterIndex("Prop2_1"), m_prop2_1.c_str(), IECSqlBinder::MakeCopy::Yes)) ||
        (ECSqlStatus::Success != statement.BindInt64(statement.GetParameterIndex("Prop2_2"), m_prop2_2)) ||
        (ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex("Prop2_3"), m_prop2_3)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement2::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat = BindParams(statement);
    if (DgnDbStatus::Success != stat)
        return stat;

    return T_Super::_BindInsertParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus PerformanceElement2::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    EXPECT_EQ (DgnDbStatus::Success, T_Super::_ReadSelectParams(stmt, params));
    EXPECT_EQ (0, strcmp(stmt.GetValueText(params.GetSelectIndex("Prop2_1")), "Element2 - InitValue"));
    EXPECT_EQ (20000000, stmt.GetValueInt64(params.GetSelectIndex("Prop2_2")));
    EXPECT_EQ (2.71828, stmt.GetValueDouble(params.GetSelectIndex("Prop2_3")));

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement2::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus status = T_Super::_BindUpdateParams(statement);
    if (DgnDbStatus::Success != status)
        return status;

    m_prop2_1 = "Element2 - UpdatedValue";
    m_prop2_2 = 40000000LL;
    m_prop2_3 = 5.43656;

    return BindParams(statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement2Ptr PerformanceElement2::Create(Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, bool specifyPropertyValues)
    {
    if (specifyPropertyValues)
        return new PerformanceElement2 (PerformanceElement2::CreateParams (db, modelId, classId, category), "Element1 - InitValue", 10000000LL, -3.1415, "Element2 - InitValue", 20000000LL, 2.71828);
    else
        return new PerformanceElement2 (PerformanceElement2::CreateParams (db, modelId, classId, category));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement2CPtr PerformanceElement2::Insert()
    {
    return GetDgnDb().Elements().Insert<PerformanceElement2> (*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement2CPtr PerformanceElement2::Update()
    {
    return GetDgnDb().Elements().Update<PerformanceElement2> (*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement3::BindParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    if ((ECSqlStatus::Success != statement.BindText(statement.GetParameterIndex("Prop3_1"), m_prop3_1.c_str(), IECSqlBinder::MakeCopy::Yes)) ||
        (ECSqlStatus::Success != statement.BindInt64(statement.GetParameterIndex("Prop3_2"), m_prop3_2)) ||
        (ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex("Prop3_3"), m_prop3_3)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement3::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat = BindParams(statement);
    if (DgnDbStatus::Success != stat)
        return stat;

    return T_Super::_BindInsertParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus PerformanceElement3::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    EXPECT_EQ (DgnDbStatus::Success, T_Super::_ReadSelectParams(stmt, params));
    EXPECT_EQ (0, strcmp(stmt.GetValueText(params.GetSelectIndex("Prop3_1")), "Element3 - InitValue"));
    EXPECT_EQ (30000000, stmt.GetValueInt64(params.GetSelectIndex("Prop3_2")));
    EXPECT_EQ (1.414121, stmt.GetValueDouble(params.GetSelectIndex("Prop3_3")));

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement3::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus status = T_Super::_BindUpdateParams(statement);
    if (DgnDbStatus::Success != status)
        return status;

    m_prop3_1 = "Element3 - UpdatedValue";
    m_prop3_2 = 60000000LL;
    m_prop3_3 = 2.828242;

    return BindParams(statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement3Ptr PerformanceElement3::Create(Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, bool specifyPropertyValues)
    {
    if (specifyPropertyValues)
        return new PerformanceElement3 (PerformanceElement3::CreateParams (db, modelId, classId, category), "Element1 - InitValue", 10000000LL, -3.1415, "Element2 - InitValue", 20000000LL, 2.71828, "Element3 - InitValue", 30000000LL, 1.414121);
    else
        return new PerformanceElement3 (PerformanceElement3::CreateParams (db, modelId, classId, category));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement3CPtr PerformanceElement3::Insert()
    {
    return GetDgnDb().Elements().Insert<PerformanceElement3> (*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement3CPtr PerformanceElement3::Update()
    {
    return GetDgnDb().Elements().Update<PerformanceElement3> (*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement4::BindParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    if ((ECSqlStatus::Success != statement.BindText(statement.GetParameterIndex("Prop4_1"), m_prop4_1.c_str(), IECSqlBinder::MakeCopy::Yes)) ||
        (ECSqlStatus::Success != statement.BindInt64(statement.GetParameterIndex("Prop4_2"), m_prop4_2)) ||
        (ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex("Prop4_3"), m_prop4_3)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement4::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat = BindParams(statement);
    if (DgnDbStatus::Success != stat)
        return stat;

    return T_Super::_BindInsertParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus PerformanceElement4::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    EXPECT_EQ (DgnDbStatus::Success, T_Super::_ReadSelectParams(stmt, params));
    EXPECT_EQ (0, strcmp(stmt.GetValueText(params.GetSelectIndex("Prop4_1")), "Element4 - InitValue"));
    EXPECT_EQ (40000000, stmt.GetValueInt64(params.GetSelectIndex("Prop4_2")));
    EXPECT_EQ (1.61803398874, stmt.GetValueDouble(params.GetSelectIndex("Prop4_3")));

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement4::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus status = T_Super::_BindUpdateParams(statement);
    if (DgnDbStatus::Success != status)
        return status;

    m_prop4_1 = "Element4 - UpdatedValue";
    m_prop4_2 = 80000000LL;
    m_prop4_3 = 3.23606797748;

    return BindParams(statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement4Ptr PerformanceElement4::Create(Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, bool specifyPropertyValues)
    {
    if (specifyPropertyValues)
        return new PerformanceElement4 (PerformanceElement4::CreateParams (db, modelId, classId, category), "Element1 - InitValue", 10000000LL, -3.1415, "Element2 - InitValue", 20000000LL, 2.71828, "Element3 - InitValue", 30000000LL, 1.414121, "Element4 - InitValue", 40000000LL, 1.61803398874);
    else
        return new PerformanceElement4 (PerformanceElement4::CreateParams (db, modelId, classId, category));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement4CPtr PerformanceElement4::Insert()
    {
    return GetDgnDb().Elements().Insert<PerformanceElement4> (*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement4CPtr PerformanceElement4::Update()
    {
    return GetDgnDb().Elements().Update<PerformanceElement4> (*this);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::CreateElements(int numInstances, Utf8CP className, bvector<DgnElementPtr>& elements, Utf8String modelCode, bool specifyPropertyValues) const
    {
    DgnClassId mclassId = DgnClassId(m_db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_SpatialModel));
    SpatialModelPtr targetModel = new SpatialModel(SpatialModel::CreateParams(*m_db, mclassId, DgnModel::CreateModelCode(modelCode)));
    EXPECT_EQ (DgnDbStatus::Success, targetModel->Insert());       /* Insert the new model into the DgnDb */
    DgnCategoryId catid = DgnCategory::QueryHighestCategoryId(*m_db);
    DgnClassId classId = DgnClassId(m_db->Schemas().GetECClassId(ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className));

    bool addMultiAspect = false;
    bool addExtKey = false;

    if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT1_CLASS))
        {
        for (int i = 0; i < numInstances; i++)
            {
            PerformanceElement1Ptr element = PerformanceElement1::Create(*m_db, targetModel->GetModelId(), classId, catid, specifyPropertyValues);
            element->AddGeomtry();
            if (addMultiAspect)
                DgnElement::MultiAspect::AddAspect(*element, *TestMultiAspect::Create("Initial Value"));
            if (addExtKey)
            {
                DgnElement::ExternalKeyAspectPtr extkeyAspect = DgnElement::ExternalKeyAspect::Create(DgnAuthorityId((uint64_t)1), "TestExtKey");
                ASSERT_TRUE(extkeyAspect.IsValid());
                element->AddAppData(DgnElement::ExternalKeyAspect::GetAppDataKey(), extkeyAspect.get());
            }
            ASSERT_TRUE (element != nullptr);
            elements.push_back(element);
            }
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT2_CLASS))
        {
        for (int i = 0; i < numInstances; i++)
            {
            PerformanceElement2Ptr element = PerformanceElement2::Create(*m_db, targetModel->GetModelId(), classId, catid, specifyPropertyValues);
            element->AddGeomtry();
            if (addMultiAspect)
                DgnElement::MultiAspect::AddAspect(*element, *TestMultiAspect::Create("Initial Value"));
            if (addExtKey)
            {
                DgnElement::ExternalKeyAspectPtr extkeyAspect = DgnElement::ExternalKeyAspect::Create(DgnAuthorityId((uint64_t)1), "TestExtKey");
                ASSERT_TRUE(extkeyAspect.IsValid());
                element->AddAppData(DgnElement::ExternalKeyAspect::GetAppDataKey(), extkeyAspect.get());
            }
            ASSERT_TRUE (element != nullptr);
            elements.push_back(element);
            }
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT3_CLASS))
        {
        for (int i = 0; i < numInstances; i++)
            {
            PerformanceElement3Ptr element = PerformanceElement3::Create(*m_db, targetModel->GetModelId(), classId, catid, specifyPropertyValues);
            element->AddGeomtry();
            if (addMultiAspect)
                DgnElement::MultiAspect::AddAspect(*element, *TestMultiAspect::Create("Initial Value"));
            if (addExtKey)
            {
                DgnElement::ExternalKeyAspectPtr extkeyAspect = DgnElement::ExternalKeyAspect::Create(DgnAuthorityId((uint64_t)1), "TestExtKey");
                ASSERT_TRUE(extkeyAspect.IsValid());
                element->AddAppData(DgnElement::ExternalKeyAspect::GetAppDataKey(), extkeyAspect.get());
            }
            ASSERT_TRUE (element != nullptr);
            elements.push_back(element);
            }
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT4_CLASS))
        {
        for (int i = 0; i < numInstances; i++)
            {
            PerformanceElement4Ptr element = PerformanceElement4::Create(*m_db, targetModel->GetModelId(), classId, catid, specifyPropertyValues);
            element->AddGeomtry();
            if (addMultiAspect)
                DgnElement::MultiAspect::AddAspect(*element, *TestMultiAspect::Create("Initial Value"));
            if (addExtKey)
            {
                DgnElement::ExternalKeyAspectPtr extkeyAspect = DgnElement::ExternalKeyAspect::Create(DgnAuthorityId((uint64_t)1), "TestExtKey");
                ASSERT_TRUE(extkeyAspect.IsValid());
                element->AddAppData(DgnElement::ExternalKeyAspect::GetAppDataKey(), extkeyAspect.get());
            }
            ASSERT_TRUE (element != nullptr);
            elements.push_back(element);
            }
        }
    ASSERT_EQ (numInstances, (int)elements.size());
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement1PropertyParams(BeSQLite::Statement& stmt, bool updateParams)
    {
    Utf8String stringVal = "Element1 - ";
    int64_t intVal = 10000000LL;
    double doubleVal = -3.1415;
    if (updateParams)
        {
        stringVal.append("UpdatedValue");
        intVal = intVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append("InitValue");
        }

    if ((DbResult::BE_SQLITE_OK != stmt.BindText(stmt.GetParameterIndex(":Prop1_1"), stringVal.c_str(), BeSQLite::Statement::MakeCopy::No)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindInt64(stmt.GetParameterIndex(":Prop1_2"), intVal)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindDouble(stmt.GetParameterIndex(":Prop1_3"), doubleVal)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement2PropertyParams(BeSQLite::Statement& stmt, bool updateParams)
    {
    Utf8String stringVal = "Element2 - ";
    int64_t intVal = 20000000LL;
    double doubleVal = 2.71828;
    if (updateParams)
        {
        stringVal.append("UpdatedValue");
        intVal = intVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append("InitValue");
        }

    if ((DgnDbStatus::Success != BindElement1PropertyParams(stmt, updateParams)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindText(stmt.GetParameterIndex(":Prop2_1"), stringVal.c_str(), BeSQLite::Statement::MakeCopy::No)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindInt64(stmt.GetParameterIndex(":Prop2_2"), intVal)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindDouble(stmt.GetParameterIndex(":Prop2_3"), doubleVal)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement3PropertyParams(BeSQLite::Statement& stmt, bool updateParams)
    {
    Utf8String stringVal = "Element3 - ";
    int64_t intVal = 30000000LL;
    double doubleVal = 1.414121;
    if (updateParams)
        {
        stringVal.append("UpdatedValue");
        intVal = intVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append("InitValue");
        }

    if ((DgnDbStatus::Success != BindElement2PropertyParams(stmt, updateParams)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindText(stmt.GetParameterIndex(":Prop3_1"), stringVal.c_str(), BeSQLite::Statement::MakeCopy::No)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindInt64(stmt.GetParameterIndex(":Prop3_2"), intVal)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindDouble(stmt.GetParameterIndex(":Prop3_3"), doubleVal)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement4PropertyParams(BeSQLite::Statement& stmt, bool updateParams)
    {
    Utf8String stringVal = "Element4 - ";
    int64_t intVal = 40000000LL;
    double doubleVal = 1.61803398874;
    if (updateParams)
        {
        stringVal.append("UpdatedValue");
        intVal = intVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append("InitValue");
        }

    if ((DgnDbStatus::Success != BindElement3PropertyParams(stmt, updateParams)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindText(stmt.GetParameterIndex(":Prop4_1"), stringVal.c_str(), BeSQLite::Statement::MakeCopy::No)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindInt64(stmt.GetParameterIndex(":Prop4_2"), intVal)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindDouble(stmt.GetParameterIndex(":Prop4_3"), doubleVal)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
void PerformanceElementsCRUDTestFixture::BindParams(DgnElementPtr& element, BeSQLite::Statement& stmt, Utf8CP className)
    {
    bool updateParams = false;
    const ECInstanceId id(s_elementId++);
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindId(stmt.GetParameterIndex(":Id"), id));
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindId(stmt.GetParameterIndex(":ModelId"), element->GetModelId()));

    DgnCode elementCode = DgnCode::CreateEmpty();
    if (elementCode.IsEmpty())
        {
        ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindNull(stmt.GetParameterIndex(":Code_Value")));
        }
    else
        {
        ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindText(stmt.GetParameterIndex(":Code_Value"), elementCode.GetValue().c_str(), BeSQLite::Statement::MakeCopy::No));
        }
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindId(stmt.GetParameterIndex(":Code_AuthorityId"), elementCode.GetAuthority()));
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindText(stmt.GetParameterIndex(":Code_Namespace"), elementCode.GetNamespace().c_str(), BeSQLite::Statement::MakeCopy::No));
    
    if (element->HasLabel())
        {
        ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.BindText(stmt.GetParameterIndex(":Label"), element->GetLabel(), BeSQLite::Statement::MakeCopy::No));
        }
    else
        {
        ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.BindNull(stmt.GetParameterIndex(":Label")));
        }

    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindId(stmt.GetParameterIndex(":ParentId"), element->GetParentId()));

    if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT1_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement1PropertyParams(stmt, updateParams));
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT2_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement2PropertyParams(stmt, updateParams));
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT3_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement3PropertyParams(stmt, updateParams));
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT4_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement4PropertyParams(stmt, updateParams));
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
void PerformanceElementsCRUDTestFixture::BindUpdateParams(BeSQLite::Statement& stmt, Utf8CP className)
    {
    bool updateParams = true;
    if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT1_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement1PropertyParams(stmt, updateParams));
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT2_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement2PropertyParams(stmt, updateParams));
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT3_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement3PropertyParams(stmt, updateParams));
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT4_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement4PropertyParams(stmt, updateParams));
        }
    }

//BindParams Overloads for ECSql
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement1PropertyParams(ECSqlStatement& statement, bool updateParams)
    {
    Utf8String stringVal = "Element1 - ";
    int64_t intVal = 10000000LL;
    double doubleVal = -3.1416;

    if (updateParams)
        {
        stringVal.append("UpdatedValue");
        intVal = intVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append("InitValue");
        }

    if ((ECSqlStatus::Success != statement.BindText(statement.GetParameterIndex("Prop1_1"), stringVal.c_str(), IECSqlBinder::MakeCopy::No)) ||
        (ECSqlStatus::Success != statement.BindInt64(statement.GetParameterIndex("Prop1_2"), intVal)) ||
        (ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex("Prop1_3"), doubleVal)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement2PropertyParams(ECSqlStatement& stmt, bool updateParams)
    {
    Utf8String stringVal = "Element2 - ";
    int64_t intVal = 20000000LL;
    double doubleVal = 2.71828;

    if (updateParams)
        {
        stringVal.append("UpdatedValue");
        intVal = intVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append("InitValue");
        }

    if ((DgnDbStatus::Success != BindElement1PropertyParams(stmt, updateParams)) ||
        (ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex("Prop2_1"), stringVal.c_str(), IECSqlBinder::MakeCopy::No)) ||
        (ECSqlStatus::Success != stmt.BindInt64(stmt.GetParameterIndex("Prop2_2"), intVal)) ||
        (ECSqlStatus::Success != stmt.BindDouble(stmt.GetParameterIndex("Prop2_3"), doubleVal)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement3PropertyParams(ECSqlStatement& stmt, bool updateParams)
    {
    Utf8String stringVal = "Element3 - ";
    int64_t intVal = 30000000LL;
    double doubleVal = 1.414121;

    if (updateParams)
        {
        stringVal.append("UpdatedValue");
        intVal = intVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append("InitValue");
        }

    if ((DgnDbStatus::Success != BindElement2PropertyParams(stmt, updateParams)) ||
        (ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex("Prop3_1"), stringVal.c_str(), IECSqlBinder::MakeCopy::No)) ||
        (ECSqlStatus::Success != stmt.BindInt64(stmt.GetParameterIndex("Prop3_2"), intVal)) ||
        (ECSqlStatus::Success != stmt.BindDouble(stmt.GetParameterIndex("Prop3_3"), doubleVal)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement4PropertyParams(ECSqlStatement& stmt, bool updateParams)
    {
    Utf8String stringVal = "Element4 - ";
    int64_t intVal = 40000000LL;
    double doubleVal = 1.61803398874;

    if (updateParams)
        {
        stringVal.append("UpdatedValue");
        intVal = intVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append("InitValue");
        }

    if ((DgnDbStatus::Success != BindElement3PropertyParams(stmt, updateParams)) ||
        (ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex("Prop4_1"), stringVal.c_str(), IECSqlBinder::MakeCopy::No)) ||
        (ECSqlStatus::Success != stmt.BindInt64(stmt.GetParameterIndex("Prop4_2"), intVal)) ||
        (ECSqlStatus::Success != stmt.BindDouble(stmt.GetParameterIndex("Prop4_3"), doubleVal)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
struct GeomBlobHeader
    {
    enum { Signature = 0x0600, };

    uint32_t m_signature;
    uint32_t m_size;
    GeomBlobHeader(GeometryStream const& geom) { m_signature = Signature; m_size = geom.GetSize(); }
    GeomBlobHeader(SnappyReader& in) { uint32_t actuallyRead; in._Read((Byte*) this, sizeof (*this), actuallyRead); }
    };

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
void PerformanceElementsCRUDTestFixture::BindParams(DgnElementPtr& element, ECSqlStatement& stmt, Utf8CP className)
    {
    bool updateParams = false;
    const ECInstanceId id(s_elementId++);
    ASSERT_EQ (ECSqlStatus::Success, stmt.BindId(stmt.GetParameterIndex("ECInstanceId"), id));
    ASSERT_EQ (ECSqlStatus::Success, stmt.BindId(stmt.GetParameterIndex("ModelId"), element->GetModelId()));

    // Bind Code
    {
        DgnCode elementCode = DgnCode::CreateEmpty();
    IECSqlStructBinder& codeBinder = stmt.BindStruct(stmt.GetParameterIndex("Code"));

    if (elementCode.IsEmpty())
        {
        ASSERT_EQ (ECSqlStatus::Success, codeBinder.GetMember("Value").BindNull());
        }
    else
        {
        ASSERT_EQ (ECSqlStatus::Success, codeBinder.GetMember("Value").BindText(elementCode.GetValue().c_str(), IECSqlBinder::MakeCopy::No));
        }

    ASSERT_EQ (ECSqlStatus::Success, codeBinder.GetMember("AuthorityId").BindId(elementCode.GetAuthority()));
    ASSERT_EQ (ECSqlStatus::Success, codeBinder.GetMember("Namespace").BindText(elementCode.GetNamespace().c_str(), IECSqlBinder::MakeCopy::No));
    }

    if (element->HasLabel())
        {
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(stmt.GetParameterIndex("Label"), element->GetLabel(), IECSqlBinder::MakeCopy::No));
        }
    else
        {
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindNull(stmt.GetParameterIndex("Label")));
        }

    ASSERT_EQ (ECSqlStatus::Success, stmt.BindId(stmt.GetParameterIndex("ParentId"), element->GetParentId()));
    ASSERT_EQ (ECSqlStatus::Success, stmt.BindId(stmt.GetParameterIndex("CategoryId"), element->ToGeometrySource()->GetCategoryId()));

    //bind Geometry
    {
    ASSERT_TRUE (element->ToGeometrySourceP ()->HasGeometry());

    // Compress the serialized GeomStream
    bool m_multiChunkGeomStream = false;
    SnappyToBlob snappyTo;
    snappyTo.Init();

    GeometryStream geom = element->ToGeometrySource()->GetGeometryStream();
    if (0 < geom.GetSize())
        {
        GeomBlobHeader header(geom);
        snappyTo.Write((Byte const*)&header, sizeof (header));
        snappyTo.Write(geom.GetData(), geom.GetSize());
        }

    auto geomIndex = stmt.GetParameterIndex ("GeometryStream");
    uint32_t zipSize = snappyTo.GetCompressedSize();
    if (0 < zipSize)
        {
        if (1 == snappyTo.GetCurrChunk())
            {
            ASSERT_EQ (ECSqlStatus::Success, stmt.BindBinary(geomIndex, snappyTo.GetChunkData(0), zipSize, IECSqlBinder::MakeCopy::No));
            }
        else
            {
            m_multiChunkGeomStream = true;
            ASSERT_EQ (ECSqlStatus::Success, stmt.BindNull(geomIndex));
            }
        }
    else
        {
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindNull(geomIndex));
        }
    }

    ASSERT_EQ (ECSqlStatus::Success, stmt.BindInt (stmt.GetParameterIndex ("InSpatialIndex"), CoordinateSpace::World == element->GetModel ()->ToGeometricModel ()->GetCoordinateSpace () ? 1 : 0));

    Placement3dCR placement = element->ToGeometrySource3d()->GetPlacement();
    if (!placement.IsValid())
        {
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindNull(stmt.GetParameterIndex("Origin")));
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindNull(stmt.GetParameterIndex("Yaw")));
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindNull(stmt.GetParameterIndex("Pitch")));
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindNull(stmt.GetParameterIndex("Roll")));
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindNull(stmt.GetParameterIndex("BBoxLow")));
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindNull(stmt.GetParameterIndex("BBoxHigh")));
        }
    else
        {
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindPoint3D (stmt.GetParameterIndex("Origin"), placement.GetOrigin()));
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindDouble(stmt.GetParameterIndex("Yaw"), placement.GetAngles().GetYaw().Degrees()));
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindDouble(stmt.GetParameterIndex("Pitch"), placement.GetAngles().GetPitch().Degrees()));
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindDouble(stmt.GetParameterIndex("Roll"), placement.GetAngles().GetRoll().Degrees()));
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindPoint3D (stmt.GetParameterIndex("BBoxLow"), placement.GetElementBox().low));
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindPoint3D (stmt.GetParameterIndex("BBoxHigh"), placement.GetElementBox().high));
        }

    if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT1_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement1PropertyParams(stmt, updateParams));
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT2_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement2PropertyParams(stmt, updateParams));
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT3_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement3PropertyParams(stmt, updateParams));
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT4_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement4PropertyParams(stmt, updateParams));
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
void PerformanceElementsCRUDTestFixture::BindUpdateParams(DgnElementPtr& element, ECSqlStatement& stmt, Utf8CP className)
    {
    bool updateParams = true;
    // Bind Code
    {
    DgnCode elementCode = element->GetCode();
    IECSqlStructBinder& codeBinder = stmt.BindStruct(stmt.GetParameterIndex("Code"));
    if (elementCode.IsEmpty())
        {
        ASSERT_EQ (ECSqlStatus::Success, codeBinder.GetMember("Value").BindNull());
        }
    else
        {
        ASSERT_EQ (ECSqlStatus::Success, codeBinder.GetMember("Value").BindText(elementCode.GetValue().c_str(), IECSqlBinder::MakeCopy::No));
        }

    ASSERT_EQ (ECSqlStatus::Success, codeBinder.GetMember("AuthorityId").BindId(elementCode.GetAuthority()));
    ASSERT_EQ (ECSqlStatus::Success, codeBinder.GetMember("Namespace").BindText(elementCode.GetNamespace().c_str(), IECSqlBinder::MakeCopy::No));
    }

    if (element->HasLabel())
        {
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindText(stmt.GetParameterIndex("Label"), element->GetLabel(), IECSqlBinder::MakeCopy::No));
        }
    else
        {
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindNull(stmt.GetParameterIndex("Label")));
        }
    ASSERT_EQ (ECSqlStatus::Success, stmt.BindId(stmt.GetParameterIndex("ParentId"), element->GetParentId()));
    ASSERT_EQ (ECSqlStatus::Success, stmt.BindId(stmt.GetParameterIndex("CategoryId"), element->ToGeometrySource()->GetCategoryId()));

    //bind Geometry
    {
    ASSERT_TRUE (element->ToGeometrySourceP ()->HasGeometry());

    // Compress the serialized GeomStream
    bool m_multiChunkGeomStream = false;
    SnappyToBlob snappyTo;
    snappyTo.Init();

    GeometryStream geom = element->ToGeometrySource()->GetGeometryStream();
    if (0 < geom.GetSize())
        {
        GeomBlobHeader header(geom);
        snappyTo.Write((Byte const*)&header, sizeof (header));
        snappyTo.Write(geom.GetData(), geom.GetSize());
        }

    auto geomIndex = stmt.GetParameterIndex ("GeometryStream");
    uint32_t zipSize = snappyTo.GetCompressedSize();
    if (0 < zipSize)
        {
        if (1 == snappyTo.GetCurrChunk())
            {
            // Common case - only one chunk in geom stream. Bind it directly.
            // NB: This requires that no other code uses DgnElements::SnappyToBlob() until our ECSqlStatement is executed...
            stmt.BindBinary(geomIndex, snappyTo.GetChunkData(0), zipSize, IECSqlBinder::MakeCopy::No);
            }
        else
            {
            // More than one chunk in geom stream. Avoid expensive alloc+copy by deferring writing geom stream until ECSqlStatement executes.
            m_multiChunkGeomStream = true;
            stmt.BindNull(geomIndex);
            }
        }
    else
        {
        // No geometry
        stmt.BindNull(geomIndex);
        }
    }
    ASSERT_EQ (ECSqlStatus::Success, stmt.BindInt (stmt.GetParameterIndex ("InSpatialIndex"), CoordinateSpace::World == element->GetModel()->ToGeometricModel()->GetCoordinateSpace() ? 1 : 0));

    Placement3dCR placement = element->ToGeometrySource3d()->GetPlacement();
    if (!placement.IsValid())
        {
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindNull(stmt.GetParameterIndex("Origin")));
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindNull(stmt.GetParameterIndex("Yaw")));
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindNull(stmt.GetParameterIndex("Pitch")));
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindNull(stmt.GetParameterIndex("Roll")));
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindNull(stmt.GetParameterIndex("BBoxLow")));
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindNull(stmt.GetParameterIndex("BBoxHigh")));
        }
    else
        {
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindPoint3D (stmt.GetParameterIndex("Origin"), placement.GetOrigin()));
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindDouble(stmt.GetParameterIndex("Yaw"), placement.GetAngles().GetYaw().Degrees()));
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindDouble(stmt.GetParameterIndex("Pitch"), placement.GetAngles().GetPitch().Degrees()));
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindDouble(stmt.GetParameterIndex("Roll"), placement.GetAngles().GetRoll().Degrees()));
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindPoint3D (stmt.GetParameterIndex("BBoxLow"), placement.GetElementBox().low));
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindPoint3D (stmt.GetParameterIndex("BBoxHigh"), placement.GetElementBox().high));
        }

    if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT1_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement1PropertyParams(stmt, updateParams));
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT2_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement2PropertyParams(stmt, updateParams));
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT3_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement3PropertyParams(stmt, updateParams));
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT4_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement4PropertyParams(stmt, updateParams));
        }
    }

//Methods to verify Business Property Values returned by Sql Statements. 
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::ExtractElement1SelectParams(BeSQLite::Statement& stmt)
    {
    if ((0 != strcmp("Element1 - InitValue", stmt.GetValueText(7))) ||
        (stmt.GetValueInt64(8) != 10000000) ||
        (stmt.GetValueDouble(9) != -3.1415))
        return DgnDbStatus::ReadError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::ExtractElement2SelectParams(BeSQLite::Statement& stmt)
    {
    if ((DgnDbStatus::Success != ExtractElement1SelectParams(stmt)) ||
        (0 != strcmp("Element2 - InitValue", stmt.GetValueText(10))) ||
        (stmt.GetValueInt64(11) != 20000000) ||
        (stmt.GetValueDouble(12) != 2.71828))
        return DgnDbStatus::ReadError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::ExtractElement3SelectParams(BeSQLite::Statement& stmt)
    {
    if ((DgnDbStatus::Success != ExtractElement2SelectParams(stmt)) ||
        (0 != strcmp("Element3 - InitValue", stmt.GetValueText(13))) ||
        (stmt.GetValueInt64(14) != 30000000) ||
        (stmt.GetValueDouble(15) != 1.414121))
        return DgnDbStatus::ReadError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::ExtractElement4SelectParams(BeSQLite::Statement& stmt)
    {
    if ((DgnDbStatus::Success != ExtractElement3SelectParams(stmt)) ||
        (0 != strcmp("Element4 - InitValue", stmt.GetValueText(16))) ||
        (stmt.GetValueInt64(17) != 40000000) ||
        (stmt.GetValueDouble(18) != 1.61803398874))
        return DgnDbStatus::ReadError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
void PerformanceElementsCRUDTestFixture::ExtractSelectParams(BeSQLite::Statement& stmt, Utf8CP className)
    {
    if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT1_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, ExtractElement1SelectParams(stmt));
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT2_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, ExtractElement2SelectParams(stmt));
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT3_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, ExtractElement3SelectParams(stmt));
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT4_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, ExtractElement4SelectParams(stmt));
        }
    }

//OverLoaded Methods to Verify Business property Values returned by ECSql Statements. 
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::ExtractElement1SelectParams(ECSqlStatement& stmt)
    {
    //printf ("\n String Prop : %s", stmt.GetValueText (15));
    //printf ("\n int Prop : %lld", stmt.GetValueInt64 (16));
    //printf ("\n double Prop : %f", stmt.GetValueDouble (17));

    if ((0 != strcmp ("Element1 - InitValue", stmt.GetValueText (15))) ||
        (stmt.GetValueInt64 (16) != 10000000) ||
        (stmt.GetValueDouble (17) != -3.1415))
        return DgnDbStatus::ReadError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::ExtractElement2SelectParams(ECSqlStatement& stmt)
    {
    if ((DgnDbStatus::Success != ExtractElement1SelectParams(stmt)) ||
        (0 != strcmp ("Element2 - InitValue", stmt.GetValueText (18))) ||
        (stmt.GetValueInt64 (19) != 20000000) ||
        (stmt.GetValueDouble (20) != 2.71828))
        return DgnDbStatus::ReadError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::ExtractElement3SelectParams(ECSqlStatement& stmt)
    {
    if ((DgnDbStatus::Success != ExtractElement2SelectParams(stmt)) ||
        (0 != strcmp ("Element3 - InitValue", stmt.GetValueText (21))) ||
        (stmt.GetValueInt64 (22) != 30000000) ||
        (stmt.GetValueDouble (23) != 1.414121))
        return DgnDbStatus::ReadError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::ExtractElement4SelectParams(ECSqlStatement& stmt)
    {
    if ((DgnDbStatus::Success != ExtractElement3SelectParams(stmt)) ||
        (0 != strcmp ("Element4 - InitValue", stmt.GetValueText (24))) ||
        (stmt.GetValueInt64 (25) != 40000000) ||
        (stmt.GetValueDouble (26) != 1.61803398874))
        return DgnDbStatus::ReadError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
void PerformanceElementsCRUDTestFixture::ExtractSelectParams(ECSqlStatement& stmt, Utf8CP className)
    {
    if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT1_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, ExtractElement1SelectParams(stmt));
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT2_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, ExtractElement2SelectParams(stmt));
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT3_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, ExtractElement3SelectParams(stmt));
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT4_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, ExtractElement4SelectParams(stmt));
        }
    }

//Methods to Generate Sql CRUD statements. 
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetInsertSql(Utf8CP className, Utf8StringR insertSql, DgnClassId classId) const
    {
    ECN::ECClassCP ecClass = m_db->Schemas().GetECClass(ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className);
    ASSERT_TRUE(ecClass != nullptr);

    insertSql = Utf8String("INSERT INTO dgn_Element ([Id], ");
    Utf8String insertValuesSql(") VALUES (:Id, ");
    bool isFirstItem = true;
    for (auto prop : ecClass->GetProperties(true))
        {
        if (0 == strcmp("LastMod", prop->GetName().c_str()))
            continue;
        if (!prop->GetIsStruct())
            {
            if (!isFirstItem)
                {
                insertSql.append(", ");
                insertValuesSql.append(", ");
                }

            insertSql.append("[").append(prop->GetName()).append("]");
            insertValuesSql.append(":").append(prop->GetName());

            isFirstItem = false;
            }
        else
            {
            for (auto structProp : prop->GetAsStructProperty()->GetType().GetProperties())
                {
                if (!isFirstItem)
                    {
                    insertSql.append(", ");
                    insertValuesSql.append(", ");
                    }

                insertSql.append("[").append(prop->GetName()).append("_").append(structProp->GetName()).append("]");
                insertValuesSql.append(":").append(prop->GetName()).append("_").append(structProp->GetName());

                isFirstItem = false;
                }
            }
        }

    insertSql.append(", ECClassId");
    insertValuesSql.append(", ");
    Utf8String insertValues;
    insertValues.Sprintf("%s%d", insertValuesSql.c_str(), (int)classId.GetValue());
    insertSql.append(insertValues).append(")");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetSelectSql(Utf8CP className, Utf8StringR selectSql, bool asTranslatedFromECSql, bool omitClassIdFilter) const
    {
    ECN::ECClassCP ecClass = m_db->Schemas().GetECClass(ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className);
    ASSERT_TRUE(ecClass != nullptr);

    if (!asTranslatedFromECSql)
        {
        selectSql = "SELECT ";
        bool isFirstItem = true;
        for (auto prop : ecClass->GetProperties(true))
            {
            Utf8CP alias = prop->GetName().StartsWithI("Prop") ? "p." : "e.";

            if (!prop->GetIsStruct())
                {
                if (!isFirstItem)
                    selectSql.append(",");

                selectSql.append(alias).append(prop->GetName());
                isFirstItem = false;
                }
            else
                {
                for (auto structProp : prop->GetAsStructProperty()->GetType().GetProperties())
                    {
                    if (!isFirstItem)
                        selectSql.append(",");

                    selectSql.append(alias).append(prop->GetName()).append("_").append(structProp->GetName());
                    isFirstItem = false;
                    }
                }
            }

        selectSql.append(" FROM dgn_Element e, dgn_SpatialElement p WHERE e.Id=p.ElementId AND e.ECClassId=p.ECClassId AND e.Id=?");
        if (!omitClassIdFilter)
            {
            Utf8String classIdFilter;
            classIdFilter.Sprintf(" AND e.ECClassId=%llu", ecClass->GetId().GetValue());
            selectSql.append(classIdFilter);
            }

        return;
        }
    else
        {
        Utf8String selectECSql;
        GetSelectECSql(className, selectECSql, omitClassIdFilter);
        ECSqlStatement stmt;
        ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare(*m_db, selectECSql.c_str()));
        selectSql = stmt.GetNativeSql();
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetUpdateSql(Utf8CP className, Utf8StringR updateSql, bool omitClassIdFilter) const
    {
    ECN::ECClassCP ecClass = m_db->Schemas().GetECClass(ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className);
    ASSERT_TRUE(ecClass != nullptr);

    updateSql = "UPDATE dgn_Element SET ";
    bool isFirstItem = true;
    for (auto prop : ecClass->GetProperties(true))
        {
        if (0 == strcmp("ModelId", prop->GetName().c_str()) || 0 == strcmp("Code", prop->GetName().c_str()) || 0 == strcmp("Label", prop->GetName().c_str()) || 0 == strcmp("ParentId", prop->GetName().c_str()) || 0 == strcmp("LastMod", prop->GetName().c_str()))
            continue;
        if (!isFirstItem)
            {
            updateSql.append(", ");
            }
        updateSql.append(prop->GetName()).append(" = :").append(prop->GetName());
        isFirstItem = false;
        }
    updateSql.append(" WHERE Id = :Id");

    if (!omitClassIdFilter)
        {
        Utf8String classIdFilter;
        classIdFilter.Sprintf(" AND ECClassId=%llu", ecClass->GetId().GetValue());
        updateSql.append(classIdFilter);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetDeleteSql(Utf8CP className, Utf8StringR deleteSql, bool omitClassIdFilter) const
    {
    deleteSql = "DELETE FROM dgn_Element WHERE Id = ?";

    if (!omitClassIdFilter)
        {
        ECN::ECClassCP ecClass = m_db->Schemas().GetECClass(ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className);
        ASSERT_TRUE(ecClass != nullptr);
        Utf8String classIdFilter;
        classIdFilter.Sprintf(" AND ECClassId=%llu", ecClass->GetId().GetValue());
        deleteSql.append(classIdFilter);
        }

    }

//Overloads to Generate ECSql statements. 
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetInsertECSql(Utf8CP className, Utf8StringR insertECSql) const
    {
    ECN::ECClassCP ecClass = m_db->Schemas().GetECClass(ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className);
    ASSERT_TRUE(ecClass != nullptr);

    insertECSql = Utf8String("INSERT INTO ");
    insertECSql.append(ecClass->GetECSqlName()).append(" ([ECInstanceId], ");
    Utf8String insertValuesSql(") VALUES (:[ECInstanceId], ");
    bool isFirstItem = true;
    for (auto prop : ecClass->GetProperties(true))
        {
        if (0 == strcmp("LastMod", prop->GetName().c_str()))
            continue;
        if (!isFirstItem)
            {
            insertECSql.append(", ");
            insertValuesSql.append(", ");
            }

        insertECSql.append("[").append(prop->GetName()).append("]");
        insertValuesSql.append(":[").append(prop->GetName()).append("]");

        isFirstItem = false;
        }

    insertECSql.append(insertValuesSql).append(")");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetSelectECSql(Utf8CP className, Utf8StringR selectECSql, bool omitClassIdFilter) const
    {
    ECN::ECClassCP ecClass = m_db->Schemas().GetECClass(ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className);
    ASSERT_TRUE(ecClass != nullptr);

    selectECSql = "SELECT ";
    bool isFirstItem = true;
    for (auto prop : ecClass->GetProperties(true))
        {
        if (!isFirstItem)
            {
            selectECSql.append(", ");
            }
        selectECSql.append(prop->GetName());
        isFirstItem = false;
        }

    selectECSql.append(" FROM ").append(ecClass->GetECSqlName()).append(" WHERE ECInstanceId = ?");
    if (omitClassIdFilter)
        selectECSql.append(" ECSQLOPTIONS NoECClassIdFilter");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetUpdateECSql(Utf8CP className, Utf8StringR updateECSql, bool omitClassIdFilter) const
    {
    ECN::ECClassCP ecClass = m_db->Schemas().GetECClass(ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className);
    ASSERT_TRUE(ecClass != nullptr);

    updateECSql = "UPDATE ONLY ";
    updateECSql.append(ecClass->GetECSqlName()).append(" SET ");
    bool isFirstItem = true;
    for (auto prop : ecClass->GetProperties(true))
        {
        if (0 == strcmp("ModelId", prop->GetName().c_str()) || 0 == strcmp("LastMod", prop->GetName().c_str()))
            continue;
        if (!isFirstItem)
            {
            updateECSql.append(", ");
            }
        updateECSql.append(prop->GetName()).append(" = :").append(prop->GetName());
        isFirstItem = false;
        }
    updateECSql.append(" WHERE ECInstanceId = :ecInstanceId");
    
    if (omitClassIdFilter)
        updateECSql.append(" ECSQLOPTIONS NoECClassIdFilter");

    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetDeleteECSql(Utf8CP className, Utf8StringR deleteECSql, bool omitClassIdFilter) const
    {
    ECN::ECClassCP ecClass = m_db->Schemas().GetECClass(ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className);
    ASSERT_TRUE(ecClass != nullptr);

    deleteECSql = "DELETE FROM ONLY ";
    deleteECSql.append(ecClass->GetECSqlName()).append(" WHERE ").append("ECInstanceId = ?");

    if (omitClassIdFilter)
        deleteECSql.append(" ECSQLOPTIONS NoECClassIdFilter");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ApiInsertTime(Utf8CP className, int initialInstanceCount, int opCount)
    {
    WString wClassName;
    wClassName.AssignUtf8(className);
    WPrintfString dbName(L"ElementApiInsert%ls_%d.ibim", wClassName.c_str(), opCount);
    SetUpTestDgnDb(dbName.c_str(), className, initialInstanceCount);

    bvector<DgnElementPtr> testElements;
    CreateElements(opCount, className, testElements, "ElementApiInstances", true);
    ASSERT_EQ(opCount, (int) testElements.size());

    StopWatch timer(true);
    for (DgnElementPtr& element : testElements)
        {
        DgnDbStatus stat = DgnDbStatus::Success;
        element->Insert(&stat);
        ASSERT_EQ (DgnDbStatus::Success, stat);
        }
    timer.Stop();
    LogTiming(timer, "Element API Insert", className, false, initialInstanceCount, opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ApiSelectTime(Utf8CP className, int initialInstanceCount, int opCount)
    {
    WString wClassName;
    wClassName.AssignUtf8(className);
    WPrintfString dbName(L"ElementApiSelect%ls_%d.ibim", wClassName.c_str(), opCount);
    SetUpTestDgnDb(dbName.c_str(), className, initialInstanceCount);

    const int elementIdIncrement = DetermineElementIdIncrement(initialInstanceCount, opCount);
    StopWatch timer(true);
    for (uint64_t i = 0; i < opCount; i++)
        {
        const DgnElementId id(s_firstElementId + i*elementIdIncrement);
        DgnElementCPtr element = m_db->Elements().GetElement(id);
        ASSERT_TRUE(element != nullptr);
        }
    timer.Stop();

    LogTiming(timer, "Element API Read", className, false, initialInstanceCount, opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ApiUpdateTime(Utf8CP className, int initialInstanceCount, int opCount)
    {
    WPrintfString dbName(L"ElementApiUpdate%ls_%d.ibim", WString(className, BentleyCharEncoding::Utf8).c_str(), opCount);
    SetUpTestDgnDb(dbName.c_str(), className, initialInstanceCount);

    const int elementIdIncrement = DetermineElementIdIncrement(initialInstanceCount, opCount);

    //First build dgnelements with modified Geomtry
    bvector<DgnElementPtr> elements;
    for (uint64_t i = 0; i < opCount; i++)
        {
        const DgnElementId id(s_firstElementId + i*elementIdIncrement);
        PerformanceElement1Ptr element = m_db->Elements().GetForEdit<PerformanceElement1>(id);
        ASSERT_TRUE(element != nullptr);

        element->ExtendGeometry();
        elements.push_back(element);
        }
    
    //Now update and record time
    StopWatch timer(true);
    for (DgnElementPtr& element : elements)
        {
        DgnDbStatus stat = DgnDbStatus::Success;
        element->DgnElement::Update(&stat);
        ASSERT_EQ (DgnDbStatus::Success, stat);
        }

    timer.Stop();
    LogTiming(timer, "Element API Update", className, false, initialInstanceCount, opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ApiDeleteTime(Utf8CP className, int initialInstanceCount, int opCount)
    {
    WPrintfString dbName(L"ElementApiDelete%ls_%d.ibim", WString(className, BentleyCharEncoding::Utf8).c_str(), opCount);
    SetUpTestDgnDb(dbName.c_str(), className, initialInstanceCount);

    const int elementIdIncrement = DetermineElementIdIncrement(initialInstanceCount, opCount);

    StopWatch timer(true);
    for (uint64_t i = 0; i < opCount; i++)
        {
        const DgnElementId id(s_firstElementId + i*elementIdIncrement);
        STATEMENT_DIAGNOSTICS_LOGCOMMENT("Elements::Delete - START");
        const DgnDbStatus stat = m_db->Elements().Delete(id);
        STATEMENT_DIAGNOSTICS_LOGCOMMENT("Elements::Delete - END");
        ASSERT_EQ(DgnDbStatus::Success, stat);
        }
    timer.Stop();
    LogTiming(timer, "Element API Delete", className, false, initialInstanceCount, opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ECSqlInsertTime(Utf8CP className, int initialInstanceCount, int opCount)
    {
    WPrintfString dbName(L"ECSqlInsert%ls_%d.ibim", WString(className, BentleyCharEncoding::Utf8).c_str(), opCount);
    SetUpTestDgnDb(dbName.c_str(), className, initialInstanceCount);

    bvector<DgnElementPtr> testElements;
    CreateElements(opCount, className, testElements, "ECSqlInstances", false);
    ASSERT_EQ(opCount, (int) testElements.size());

    ECSqlStatement stmt;
    Utf8String insertECSql;
    GetInsertECSql(className, insertECSql);
    //printf ("\n Insert ECSql %s : %s \n", className, insertECSql.c_str ());

    StopWatch timer(true);
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare(*m_db, insertECSql.c_str()));
    //printf ("\n Native Sql: %s", stmt.GetNativeSql ());
    for (DgnElementPtr& element : testElements)
        {
        BindParams(element, stmt, className);
        if (DbResult::BE_SQLITE_DONE != stmt.Step() || m_db->GetModifiedRowCount() == 0)
            ASSERT_TRUE (false);
        stmt.Reset();
        stmt.ClearBindings();
        }
    timer.Stop();
    LogTiming(timer, "ECSQL INSERT", className, false, initialInstanceCount, opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ECSqlSelectTime(Utf8CP className, bool omitClassIdFilter, int initialInstanceCount, int opCount)
    {
    WPrintfString dbName(L"ECSqlSelect%ls_%d.ibim", WString(className, BentleyCharEncoding::Utf8).c_str(), initialInstanceCount);
    SetUpTestDgnDb(dbName.c_str(), className, initialInstanceCount);

    Utf8String selectECSql;
    GetSelectECSql(className, selectECSql, omitClassIdFilter);
    //printf ("\n Select ECSql %s : %s \n", className, selectECSql.c_str ());

    const int elementIdIncrement = DetermineElementIdIncrement(initialInstanceCount, opCount);

    ECSqlStatement stmt;

    StopWatch timer(true);
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare(*m_db, selectECSql.c_str()));
    //printf ("\n Native Sql %s : %s \n", className, stmt.GetNativeSql());
    //printf("Attach to profiler...\r\n");getchar();
    for (int i = 0; i < opCount; i++)
        {
        const ECInstanceId id(s_firstElementId + i*elementIdIncrement);
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindId(1, id));
        ASSERT_EQ (DbResult::BE_SQLITE_ROW, stmt.Step());
        ExtractSelectParams(stmt, className);
        stmt.Reset();
        stmt.ClearBindings();
        }
    //printf("Detach from profiler...\r\n"); getchar();
    timer.Stop();
    LogTiming(timer, "ECSQL SELECT", className, omitClassIdFilter, initialInstanceCount, opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ECSqlUpdateTime(Utf8CP className, bool omitClassIdFilter, int initialInstanceCount, int opCount)
    {
    WPrintfString dbName(L"ECSqlUpdate%ls_%d.ibim", WString(className, BentleyCharEncoding::Utf8).c_str(), opCount);
    SetUpTestDgnDb(dbName.c_str(), className, initialInstanceCount);

    ECSqlStatement stmt;
    Utf8String updateECSql;
    GetUpdateECSql(className, updateECSql, omitClassIdFilter);
    //printf ("\n Update ECSql %s : %s \n", className, updateECSql.c_str ());

    bvector<DgnElementPtr> elements;
    const int elementIdIncrement = DetermineElementIdIncrement(initialInstanceCount, opCount);
    for (uint64_t i = 0; i < opCount; i++)
        {
        DgnElementId id(s_firstElementId + i*elementIdIncrement);
        PerformanceElement1Ptr element = m_db->Elements().GetForEdit<PerformanceElement1> (id);
        ASSERT_TRUE (element != nullptr);

        element->ExtendGeometry();
        elements.push_back(element);
        }

    StopWatch timer(true);
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare(*m_db, updateECSql.c_str()));
    //printf ("\n Native Sql: %s", stmt.GetNativeSql ());
    for (DgnElementPtr& element : elements)
        {
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindId(stmt.GetParameterIndex("ecInstanceId"), element->GetElementId()));
        BindUpdateParams(element, stmt, className);
        if (DbResult::BE_SQLITE_DONE != stmt.Step() || m_db->GetModifiedRowCount() == 0)
            ASSERT_TRUE (false);
        stmt.Reset();
        stmt.ClearBindings();
        }
    timer.Stop();

    LogTiming(timer, "ECSQL UPDATE", className, omitClassIdFilter, initialInstanceCount, opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ECSqlDeleteTime(Utf8CP className, bool omitClassIdFilter, int initialInstanceCount, int opCount)
    {
    WPrintfString dbName(L"ECSqlDelete%ls_%d.ibim", WString(className, BentleyCharEncoding::Utf8).c_str(), opCount);
    SetUpTestDgnDb(dbName.c_str(), className, initialInstanceCount);

    ECSqlStatement stmt;
    Utf8String deleteECSql;
    GetDeleteECSql(className, deleteECSql, omitClassIdFilter);
    //printf ("\n Delete ECSql %s : %s \n", className, deleteECSql.c_str ());

    const int elementIdIncrement = DetermineElementIdIncrement(initialInstanceCount, opCount);

    STATEMENT_DIAGNOSTICS_LOGCOMMENT("ECSQL DELETE - START");

    StopWatch timer(true);
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare(*m_db, deleteECSql.c_str()));
    for (int i = 0; i < opCount; i++)
        {
        const ECInstanceId id(s_firstElementId + i*elementIdIncrement);
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindId(1, id));
        if (DbResult::BE_SQLITE_DONE != stmt.Step() || m_db->GetModifiedRowCount() == 0)
            ASSERT_TRUE (false);
        stmt.Reset();
        stmt.ClearBindings();
        }
    timer.Stop();

    STATEMENT_DIAGNOSTICS_LOGCOMMENT("ECSQL DELETE - END");

    LogTiming(timer, "ECSQL DELETE", className, omitClassIdFilter, initialInstanceCount, opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::SqlInsertTime(Utf8CP className, int initialInstanceCount, int opCount)
    {
    WPrintfString dbName(L"SqlInsert%ls_%d.ibim", WString(className, BentleyCharEncoding::Utf8).c_str(), opCount);
    SetUpTestDgnDb(dbName.c_str(), className, initialInstanceCount);

    bvector<DgnElementPtr> testElements;
    CreateElements(opCount, className, testElements, "SqlInstances", false);
    ASSERT_EQ(opCount, (int) testElements.size());

    BeSQLite::Statement stmt;
    Utf8String insertSql;
    DgnElementPtr firstElement = testElements.front();
    GetInsertSql(className, insertSql, firstElement->GetElementClassId());
    //printf ("\n Insert Sql %s : %s \n", className, insertSql.c_str ());

    StopWatch timer(true);
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.Prepare(*m_db, insertSql.c_str()));
    for (DgnElementPtr& element : testElements)
        {
        BindParams(element, stmt, className);
        if (DbResult::BE_SQLITE_DONE != stmt.Step() || m_db->GetModifiedRowCount() == 0)
            ASSERT_TRUE (false);
        stmt.Reset();
        stmt.ClearBindings();
        }
    timer.Stop();

    LogTiming(timer, "SQLite INSERT", className, false, initialInstanceCount, opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::SqlSelectTime(Utf8CP className, bool asTranslatedByECSql, bool omitClassIdFilter, int initialInstanceCount, int opCount)
    {
    WPrintfString dbName(L"SqlSelect%ls_%d.ibim", WString(className, BentleyCharEncoding::Utf8).c_str(), opCount);
    SetUpTestDgnDb(dbName.c_str(), className, initialInstanceCount);

    BeSQLite::Statement stmt;
    Utf8String selectSql;
    GetSelectSql(className, selectSql, asTranslatedByECSql, omitClassIdFilter);
    //printf ("\n Select Sql %s : %s \n", className, selectSql.c_str ());

    const int elementIdIncrement = DetermineElementIdIncrement(initialInstanceCount, opCount);

    StopWatch timer(true);
    ASSERT_EQ (BE_SQLITE_OK, stmt.Prepare(*m_db, selectSql.c_str())) << className << " As translated by ECSQL: " << asTranslatedByECSql << " Omit ECClassIdFilter: " << omitClassIdFilter << " Error: " << m_db->GetLastError().c_str();
    for (int i = 0; i < opCount; i++)
        {
        const ECInstanceId id(s_firstElementId + i*elementIdIncrement);
        ASSERT_EQ (BE_SQLITE_OK, stmt.BindId(1, id)) << className << " As translated by ECSQL: " << asTranslatedByECSql << " Omit ECClassIdFilter: " << omitClassIdFilter;
        ASSERT_EQ (BE_SQLITE_ROW, stmt.Step()) << className << " As translated by ECSQL: " << asTranslatedByECSql << " Omit ECClassIdFilter: " << omitClassIdFilter;
        ExtractSelectParams(stmt, className);
        stmt.Reset();
        stmt.ClearBindings();
        }
    timer.Stop();
    Utf8CP description = asTranslatedByECSql ? "SQLite SELECT (ECSQL translation)" : "SQLite SELECT";
    LogTiming(timer, description, className, omitClassIdFilter, initialInstanceCount, opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::SqlUpdateTime(Utf8CP className, bool omitClassIdFilter, int initialInstanceCount, int opCount)
    {
    WPrintfString dbName(L"SqlUpdate%ls_%d.ibim", WString(className, BentleyCharEncoding::Utf8).c_str(), opCount);
    SetUpTestDgnDb(dbName.c_str(), className, initialInstanceCount);

    BeSQLite::Statement stmt;
    Utf8String updateSql;
    GetUpdateSql(className, updateSql, omitClassIdFilter);
    //printf ("\n Update Sql %s : %s \n", className, updateSql.c_str ());

    const int elementIdIncrement = DetermineElementIdIncrement(initialInstanceCount, opCount);

    StopWatch timer(true);
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.Prepare(*m_db, updateSql.c_str()));
    for (int i = 0; i < opCount; i++)
        {
        const ECInstanceId id(s_firstElementId + i*elementIdIncrement);
        ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindId(stmt.GetParameterIndex(":Id"), id));
        BindUpdateParams(stmt, className);
        if (DbResult::BE_SQLITE_DONE != stmt.Step() || m_db->GetModifiedRowCount() == 0)
            ASSERT_TRUE (false);
        stmt.Reset();
        stmt.ClearBindings();
        }
    timer.Stop();
    LogTiming(timer, "SQLite UPDATE", className, omitClassIdFilter, initialInstanceCount, opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::SqlDeleteTime(Utf8CP className, bool omitClassIdFilter, int initialInstanceCount, int opCount)
    {
    WPrintfString dbName(L"SqlDelete%ls_%d.ibim", WString(className, BentleyCharEncoding::Utf8).c_str(), opCount);
    SetUpTestDgnDb(dbName.c_str(), className, initialInstanceCount);

    BeSQLite::Statement stmt;
    Utf8String deleteSql;
    GetDeleteSql(className, deleteSql, omitClassIdFilter);
    //printf ("\n Delete Sql %s : %s \n", className, deleteSql.c_str ());

    const int elementIdIncrement = DetermineElementIdIncrement(initialInstanceCount, opCount);

    STATEMENT_DIAGNOSTICS_LOGCOMMENT("SQLite DELETE - START");

    StopWatch timer(true);
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.Prepare(*m_db, deleteSql.c_str()));
    for (int i = 0; i < opCount; i++)
        {
        const ECInstanceId id(s_firstElementId + i*elementIdIncrement);
        ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindId(1, id));
        if (DbResult::BE_SQLITE_DONE != stmt.Step() || m_db->GetModifiedRowCount() == 0)
            ASSERT_TRUE (false);
        stmt.Reset();
        stmt.ClearBindings();
        }
    timer.Stop();
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("SQLite DELETE - END");

    LogTiming(timer, "SQLite DELETE", className, omitClassIdFilter, initialInstanceCount, opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Krischan.Eberle                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::LogTiming(StopWatch& timer, Utf8CP description, Utf8CP testClassName, bool omitClassIdFilter, int initialInstanceCount, int opCount) const
    {
    Utf8CP noClassIdFilterStr = omitClassIdFilter ? "w/o ECClassId filter " : " ";

    Utf8String totalDescription;
    totalDescription.Sprintf("%s %s '%s' [Initial count: %d]", description, noClassIdFilterStr, testClassName, initialInstanceCount);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), totalDescription.c_str(), opCount);
#ifdef PERF_ELEM_CRUD_LOG_TO_CONSOLE
    printf("%.8f %s\n", timer.GetElapsedSeconds(), totalDescription.c_str());
#endif
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceElementsCRUDTestFixture, InsertSQLite)
    {
    //SqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS);
    //SqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS);
    //SqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS);
    //SqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceElementsCRUDTestFixture, InsertECSql)
    {
    ECSqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS);
    ECSqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS);
    ECSqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS);
    ECSqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceElementsCRUDTestFixture, InsertApi)
    {
    ApiInsertTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS);
    ApiInsertTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS);
    ApiInsertTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS);
    ApiInsertTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceElementsCRUDTestFixture, SelectSQLite)
    {
    //SqlSelectTime (ELEMENT_PERFORMANCE_ELEMENT1_CLASS, false /* optimal SQL */, false);
    //SqlSelectTime (ELEMENT_PERFORMANCE_ELEMENT2_CLASS, false /* optimal SQL */, false);
    //SqlSelectTime (ELEMENT_PERFORMANCE_ELEMENT3_CLASS, false /* optimal SQL */, false);
    //SqlSelectTime (ELEMENT_PERFORMANCE_ELEMENT4_CLASS, false /* optimal SQL */, false);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceElementsCRUDTestFixture, SelectSQLite_NoECClassIdFilter)
    {
    //SqlSelectTime (ELEMENT_PERFORMANCE_ELEMENT1_CLASS, false /* optimal SQL */, true);
    //SqlSelectTime (ELEMENT_PERFORMANCE_ELEMENT2_CLASS, false /* optimal SQL */, true);
    //SqlSelectTime (ELEMENT_PERFORMANCE_ELEMENT3_CLASS, false /* optimal SQL */, true);
    //SqlSelectTime (ELEMENT_PERFORMANCE_ELEMENT4_CLASS, false /* optimal SQL */, true);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceElementsCRUDTestFixture, SelectSQLiteAsGeneratedByECSql)
    {
    //SqlSelectTime (ELEMENT_PERFORMANCE_ELEMENT1_CLASS, true /* SQL as generated by ECSQL */, false);
    //SqlSelectTime (ELEMENT_PERFORMANCE_ELEMENT2_CLASS, true /* SQL as generated by ECSQL */, false);
    //SqlSelectTime (ELEMENT_PERFORMANCE_ELEMENT3_CLASS, true /* SQL as generated by ECSQL */, false);
    //SqlSelectTime (ELEMENT_PERFORMANCE_ELEMENT4_CLASS, true /* SQL as generated by ECSQL */, false);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceElementsCRUDTestFixture, SelectSQLiteAsGeneratedByECSql_NoECClassIdFilter)
    {
    //SqlSelectTime (ELEMENT_PERFORMANCE_ELEMENT1_CLASS, true /* SQL as generated by ECSQL */, true);
    //SqlSelectTime (ELEMENT_PERFORMANCE_ELEMENT2_CLASS, true /* SQL as generated by ECSQL */, true);
    //SqlSelectTime (ELEMENT_PERFORMANCE_ELEMENT3_CLASS, true /* SQL as generated by ECSQL */, true);
    //SqlSelectTime (ELEMENT_PERFORMANCE_ELEMENT4_CLASS, true /* SQL as generated by ECSQL */, true);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceElementsCRUDTestFixture, SelectECSql)
    {
    ECSqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, false);
    ECSqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, false);
    ECSqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, false);
    ECSqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, false);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceElementsCRUDTestFixture, SelectECSql_NoECClassIdFilter)
    {
    ECSqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, true);
    ECSqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, true);
    ECSqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, true);
    ECSqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, true);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, SelectApi)
    {
    ApiSelectTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS);
    ApiSelectTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS);
    ApiSelectTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS);
    ApiSelectTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceElementsCRUDTestFixture, UpdateSQLite)
    {
    //SqlUpdateTime (ELEMENT_PERFORMANCE_ELEMENT1_CLASS, false);
    //SqlUpdateTime (ELEMENT_PERFORMANCE_ELEMENT2_CLASS, false);
    //SqlUpdateTime (ELEMENT_PERFORMANCE_ELEMENT3_CLASS, false);
    //SqlUpdateTime (ELEMENT_PERFORMANCE_ELEMENT4_CLASS, false);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceElementsCRUDTestFixture, UpdateSQLite_NoECClassIdFilter)
    {
    //SqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, true);
    //SqlUpdateTime (ELEMENT_PERFORMANCE_ELEMENT2_CLASS, true);
    //SqlUpdateTime (ELEMENT_PERFORMANCE_ELEMENT3_CLASS, true);
    //SqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, true);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceElementsCRUDTestFixture, UpdateECSql)
    {
    ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, false);
    ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, false);
    ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, false);
    ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, false);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceElementsCRUDTestFixture, UpdateECSql_NoECClassIdFilter)
    {
    ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, true);
    ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, true);
    ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, true);
    ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, true);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, UpdateApi)
    {
    ApiUpdateTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS);
    ApiUpdateTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS);
    ApiUpdateTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS);
    ApiUpdateTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceElementsCRUDTestFixture, DeleteSQLite)
    {
    SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, false);
    SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, false);
    SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, false);
    SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, false);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceElementsCRUDTestFixture, DeleteSQLite_NoECClassIdFilter)
    {
    SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, true);
    SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, true);
    SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, true);
    SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, true);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceElementsCRUDTestFixture, DeleteECSql)
    {
    ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, false);
    ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, false);
    ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, false);
    ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, false);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceElementsCRUDTestFixture, DeleteECSql_NoECClassIdFilter)
    {
    ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, true);
    ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, true);
    ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, true);
    ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, true);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, DeleteApi)
    {
    ApiDeleteTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS);
    ApiDeleteTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS);
    ApiDeleteTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS);
    ApiDeleteTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS);
    }

// Uncomment this to profile ElementLocksPerformanceTest
// #define PROFILE_ELEMENT_LOCKS_TEST 1
// Uncomment this to output timings of ElementLocksPerformanceTest runs
// #define PRINT_ELEMENT_LOCKS_TEST 1

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   04/16
//=======================================================================================
struct ElementLocksPerformanceTest : PerformanceElementsCRUDTestFixture
{
    void TestInsert(bool asBriefcase, Utf8CP className, int numElems)
        {
        auto dbName = asBriefcase ? L"LocksBriefcase.ibim" : L"LocksRepository.ibim";
        SetUpTestDgnDb(dbName, className, 0);
        if (asBriefcase)
            TestDataManager::MustBeBriefcase(m_db, Db::OpenMode::ReadWrite);

        bvector<DgnElementPtr> elems;
        elems.reserve(numElems);
        CreateElements(numElems, className, elems, "MyModel", true);

#ifdef PROFILE_ELEMENT_LOCKS_TEST
        printf("Attach profiler...\n");
        getchar();
#endif
        StopWatch timer(true);
        for (auto& elem : elems)
            {
            DgnDbStatus stat;
            elem->Insert(&stat);
            EXPECT_EQ(DgnDbStatus::Success, stat);
            }

        timer.Stop();
#ifdef PRINT_ELEMENT_LOCKS_TEST
        printf("%ls (%d): %f\n", dbName, m_db->GetBriefcaseId().GetValue(), timer.GetElapsedSeconds());
#endif

        m_db->SaveChanges();
        }

    void TestInsert(bool asBriefcase, int nElems) { TestInsert(asBriefcase, ELEMENT_PERFORMANCE_ELEMENT4_CLASS, nElems); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementLocksPerformanceTest, Master_Insert100) { TestInsert(false, 100); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementLocksPerformanceTest, Master_Insert1000) { TestInsert(false, 1000); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementLocksPerformanceTest, Master_Insert10000) { TestInsert(false, 10000); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementLocksPerformanceTest, Briefcase_Insert100) { TestInsert(true, 100); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementLocksPerformanceTest, Briefcase_Insert1000) { TestInsert(true, 1000); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementLocksPerformanceTest, Briefcase_Insert10000) { TestInsert(true, 10000); }

