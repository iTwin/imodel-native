/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/PerformanceElementsCRUDTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceElementsCRUDTests.h"

HANDLER_DEFINE_MEMBERS (PerformanceElement1Handler)
HANDLER_DEFINE_MEMBERS (PerformanceElement2Handler)
HANDLER_DEFINE_MEMBERS (PerformanceElement3Handler)
HANDLER_DEFINE_MEMBERS (PerformanceElement4Handler)

DOMAIN_DEFINE_MEMBERS (PerformanceElementTestDomain)

void PerformanceElement1Handler::_GetClassParams (ECSqlClassParams& params) { T_Super::_GetClassParams (params); PerformanceElement1::GetParams (params); }
void PerformanceElement2Handler::_GetClassParams (ECSqlClassParams& params) { T_Super::_GetClassParams (params); PerformanceElement2::GetParams (params); }
void PerformanceElement3Handler::_GetClassParams (ECSqlClassParams& params) { T_Super::_GetClassParams (params); PerformanceElement3::GetParams (params); }
void PerformanceElement4Handler::_GetClassParams (ECSqlClassParams& params) { T_Super::_GetClassParams (params); PerformanceElement4::GetParams (params); }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
PerformanceElementTestDomain::PerformanceElementTestDomain () : DgnDomain (ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, "Test Schema", 1)
    {
    RegisterHandler (PerformanceElement1Handler::GetHandler ());
    RegisterHandler (PerformanceElement2Handler::GetHandler ());
    RegisterHandler (PerformanceElement3Handler::GetHandler ());
    RegisterHandler (PerformanceElement4Handler::GetHandler ());
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::SetUpTestDgnDb (WCharCP destFileName, Utf8CP testClassName, int initialInstanceCount)
    {
    WString seedFileName;
    seedFileName.Sprintf (L"dgndb_ecsqlvssqlite_%d_%ls_seed%d.idgndb", initialInstanceCount, WString (testClassName, BentleyCharEncoding::Utf8).c_str (), DateTime::GetCurrentTimeUtc ().GetDayOfYear ());

    BeFileName seedFilePath;
    BeTest::GetHost ().GetOutputRoot (seedFilePath);
    seedFilePath.AppendToPath (seedFileName.c_str ());

    if (!seedFilePath.DoesPathExist ())
        {
        SetupProject (L"3dMetricGeneral.idgndb", seedFileName.c_str (), BeSQLite::Db::OpenMode::ReadWrite);
        ECN::ECSchemaReadContextPtr schemaContext = ECN::ECSchemaReadContext::CreateContext ();
        BeFileName searchDir;
        BeTest::GetHost ().GetDgnPlatformAssetsDirectory (searchDir);
        searchDir.AppendToPath (L"ECSchemas").AppendToPath (L"Dgn");
        schemaContext->AddSchemaPath (searchDir.GetName ());

        ECN::ECSchemaPtr schema = nullptr;
        ASSERT_EQ (ECN::SCHEMA_READ_STATUS_Success, ECN::ECSchema::ReadFromXmlString (schema, s_testSchemaXml, *schemaContext));

        schemaContext->AddSchema (*schema);
        DgnBaseDomain::GetDomain ().ImportSchema (*m_db, schemaContext->GetCache ());
        ASSERT_TRUE (m_db->IsDbOpen ());

        bvector<DgnElementPtr> testElements;
        CreateElements (initialInstanceCount, testClassName, testElements, "InitialInstances", true);
        DgnDbStatus stat = DgnDbStatus::Success;
        for (DgnElementPtr& element : testElements)
            {
            element->Insert (&stat);
            ASSERT_EQ (DgnDbStatus::Success, stat);
            }

        m_db->SaveChanges ();
        m_db->CloseDb ();
        }

    BeFileName dgndbFilePath;
    BeTest::GetHost ().GetOutputRoot (dgndbFilePath);
    dgndbFilePath.AppendToPath (destFileName);

    ASSERT_EQ (BeFileNameStatus::Success, BeFileName::BeCopyFile (seedFilePath, dgndbFilePath, false));

    DbResult status;
    m_db = DgnDb::OpenDgnDb (&status, dgndbFilePath, DgnDb::OpenParams (Db::OpenMode::ReadWrite));
    EXPECT_EQ (DbResult::BE_SQLITE_OK, status) << status;
    ASSERT_TRUE (m_db.IsValid ());
    }

Utf8CP const PerformanceElementsCRUDTestFixture::s_testSchemaXml =
    "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECSchemaReference name = 'dgn' version = '02.00' prefix = 'dgn' />"
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
        "  <ECClass typeName='SimpleElement'>"
        "    <ECCustomAttributes>"
        "       <ClassHasHandler xmlns=\"dgn.02.00\" />"
        "    </ECCustomAttributes>"
        "    <BaseClass>dgn:Element</BaseClass>"
        "  </ECClass>"
        "</ECSchema>";

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElement1::GetParams (ECSqlClassParams& params)
    {
    params.Add ("Prop1_1");
    params.Add ("Prop1_2");
    params.Add ("Prop1_3");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement1::BindParams (BeSQLite::EC::ECSqlStatement& statement)
    {
    if ((ECSqlStatus::Success != statement.BindText (statement.GetParameterIndex ("Prop1_1"), m_prop1_1.c_str (), IECSqlBinder::MakeCopy::Yes)) ||
        (ECSqlStatus::Success != statement.BindInt64 (statement.GetParameterIndex ("Prop1_2"), m_prop1_2)) ||
        (ECSqlStatus::Success != statement.BindDouble (statement.GetParameterIndex ("Prop1_3"), m_prop1_3)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement1::_BindInsertParams (BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat = BindParams (statement);
    if (DgnDbStatus::Success != stat)
        return stat;

    return T_Super::_BindInsertParams (statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus PerformanceElement1::_ExtractSelectParams (ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    EXPECT_EQ (0, strcmp (stmt.GetValueText (params.GetSelectIndex ("Prop1_1")), "Element1 - InitValue"));
    EXPECT_EQ (10000000, stmt.GetValueInt64 (params.GetSelectIndex ("Prop1_2")));
    EXPECT_EQ (-3.1415, stmt.GetValueDouble (params.GetSelectIndex ("Prop1_3")));

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement1::_BindUpdateParams (BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus status = T_Super::_BindUpdateParams (statement);
    if (DgnDbStatus::Success != status)
        return status;

    m_prop1_1 = "Element1 - UpdatedValue";
    m_prop1_2 = 20000000LL;
    m_prop1_3 = -6.283;

    return BindParams (statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement1Ptr PerformanceElement1::Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id, bool specifyProperyValues)
    {
    if (specifyProperyValues)
        return new PerformanceElement1 (PhysicalElement::CreateParams (db, modelId, classId, category), "Element1 - InitValue", 10000000LL, -3.1415);
    else
        return new PerformanceElement1 (PhysicalElement::CreateParams (db, modelId, classId, category, Dgn::Placement3d (), Dgn::DgnElement::Code (), id, Dgn::DgnElementId ()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement1CPtr PerformanceElement1::Insert ()
    {
    return GetDgnDb ().Elements ().Insert<PerformanceElement1> (*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement1CPtr PerformanceElement1::Update ()
    {
    return GetDgnDb ().Elements ().Update<PerformanceElement1> (*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElement2::GetParams (ECSqlClassParams& params)
    {
    params.Add ("Prop2_1");
    params.Add ("Prop2_2");
    params.Add ("Prop2_3");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement2::BindParams (BeSQLite::EC::ECSqlStatement& statement)
    {
    if ((ECSqlStatus::Success != statement.BindText (statement.GetParameterIndex ("Prop2_1"), m_prop2_1.c_str (), IECSqlBinder::MakeCopy::Yes)) ||
        (ECSqlStatus::Success != statement.BindInt64 (statement.GetParameterIndex ("Prop2_2"), m_prop2_2)) ||
        (ECSqlStatus::Success != statement.BindDouble (statement.GetParameterIndex ("Prop2_3"), m_prop2_3)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement2::_BindInsertParams (BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat = BindParams (statement);
    if (DgnDbStatus::Success != stat)
        return stat;

    return T_Super::_BindInsertParams (statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus PerformanceElement2::_ExtractSelectParams (ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    EXPECT_EQ (DgnDbStatus::Success, T_Super::_ExtractSelectParams (stmt, params));
    EXPECT_EQ (0, strcmp (stmt.GetValueText (params.GetSelectIndex ("Prop2_1")), "Element2 - InitValue"));
    EXPECT_EQ (20000000, stmt.GetValueInt64 (params.GetSelectIndex ("Prop2_2")));
    EXPECT_EQ (2.71828, stmt.GetValueDouble (params.GetSelectIndex ("Prop2_3")));

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement2::_BindUpdateParams (BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus status = T_Super::_BindUpdateParams (statement);
    if (DgnDbStatus::Success != status)
        return status;

    m_prop2_1 = "Element2 - UpdatedValue";
    m_prop2_2 = 40000000LL;
    m_prop2_3 = 5.43656;

    return BindParams (statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement2Ptr PerformanceElement2::Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id, bool specifyProperyValues)
    {
    if (specifyProperyValues)
        return new PerformanceElement2 (PhysicalElement::CreateParams (db, modelId, classId, category), "Element1 - InitValue", 10000000LL, -3.1415, "Element2 - InitValue", 20000000LL, 2.71828);
    else
        return new PerformanceElement2 (PhysicalElement::CreateParams (db, modelId, classId, category, Dgn::Placement3d (), Dgn::DgnElement::Code (), id, Dgn::DgnElementId ()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement2CPtr PerformanceElement2::Insert ()
    {
    return GetDgnDb ().Elements ().Insert<PerformanceElement2> (*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement2CPtr PerformanceElement2::Update ()
    {
    return GetDgnDb ().Elements ().Update<PerformanceElement2> (*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElement3::GetParams (ECSqlClassParams& params)
    {
    params.Add ("Prop3_1");
    params.Add ("Prop3_2");
    params.Add ("Prop3_3");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement3::BindParams (BeSQLite::EC::ECSqlStatement& statement)
    {
    if ((ECSqlStatus::Success != statement.BindText (statement.GetParameterIndex ("Prop3_1"), m_prop3_1.c_str (), IECSqlBinder::MakeCopy::Yes)) ||
        (ECSqlStatus::Success != statement.BindInt64 (statement.GetParameterIndex ("Prop3_2"), m_prop3_2)) ||
        (ECSqlStatus::Success != statement.BindDouble (statement.GetParameterIndex ("Prop3_3"), m_prop3_3)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement3::_BindInsertParams (BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat = BindParams (statement);
    if (DgnDbStatus::Success != stat)
        return stat;

    return T_Super::_BindInsertParams (statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus PerformanceElement3::_ExtractSelectParams (ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    EXPECT_EQ (DgnDbStatus::Success, T_Super::_ExtractSelectParams (stmt, params));
    EXPECT_EQ (0, strcmp (stmt.GetValueText (params.GetSelectIndex ("Prop3_1")), "Element3 - InitValue"));
    EXPECT_EQ (30000000, stmt.GetValueInt64 (params.GetSelectIndex ("Prop3_2")));
    EXPECT_EQ (1.414121, stmt.GetValueDouble (params.GetSelectIndex ("Prop3_3")));

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement3::_BindUpdateParams (BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus status = T_Super::_BindUpdateParams (statement);
    if (DgnDbStatus::Success != status)
        return status;

    m_prop3_1 = "Element3 - UpdatedValue";
    m_prop3_2 = 60000000LL;
    m_prop3_3 = 2.828242;

    return BindParams (statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement3Ptr PerformanceElement3::Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id, bool specifyProperyValues)
    {
    if (specifyProperyValues)
        return new PerformanceElement3 (PhysicalElement::CreateParams (db, modelId, classId, category), "Element1 - InitValue", 10000000LL, -3.1415, "Element2 - InitValue", 20000000LL, 2.71828, "Element3 - InitValue", 30000000LL, 1.414121);
    else
        return new PerformanceElement3 (PhysicalElement::CreateParams (db, modelId, classId, category, Dgn::Placement3d (), Dgn::DgnElement::Code (), id, Dgn::DgnElementId ()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement3CPtr PerformanceElement3::Insert ()
    {
    return GetDgnDb ().Elements ().Insert<PerformanceElement3> (*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement3CPtr PerformanceElement3::Update ()
    {
    return GetDgnDb ().Elements ().Update<PerformanceElement3> (*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElement4::GetParams (ECSqlClassParams& params)
    {
    params.Add ("Prop4_1");
    params.Add ("Prop4_2");
    params.Add ("Prop4_3");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement4::BindParams (BeSQLite::EC::ECSqlStatement& statement)
    {
    if ((ECSqlStatus::Success != statement.BindText (statement.GetParameterIndex ("Prop4_1"), m_prop4_1.c_str (), IECSqlBinder::MakeCopy::Yes)) ||
        (ECSqlStatus::Success != statement.BindInt64 (statement.GetParameterIndex ("Prop4_2"), m_prop4_2)) ||
        (ECSqlStatus::Success != statement.BindDouble (statement.GetParameterIndex ("Prop4_3"), m_prop4_3)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement4::_BindInsertParams (BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat = BindParams (statement);
    if (DgnDbStatus::Success != stat)
        return stat;

    return T_Super::_BindInsertParams (statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus PerformanceElement4::_ExtractSelectParams (ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    EXPECT_EQ (DgnDbStatus::Success, T_Super::_ExtractSelectParams (stmt, params));
    EXPECT_EQ (0, strcmp (stmt.GetValueText (params.GetSelectIndex ("Prop4_1")), "Element4 - InitValue"));
    EXPECT_EQ (40000000, stmt.GetValueInt64 (params.GetSelectIndex ("Prop4_2")));
    EXPECT_EQ (1.61803398874, stmt.GetValueDouble (params.GetSelectIndex ("Prop4_3")));

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement4::_BindUpdateParams (BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus status = T_Super::_BindUpdateParams (statement);
    if (DgnDbStatus::Success != status)
        return status;

    m_prop4_1 = "Element4 - UpdatedValue";
    m_prop4_2 = 80000000LL;
    m_prop4_3 = 3.23606797748;

    return BindParams (statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement4Ptr PerformanceElement4::Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id, bool specifyProperyValues)
    {
    if (specifyProperyValues)
        return new PerformanceElement4 (PhysicalElement::CreateParams (db, modelId, classId, category), "Element1 - InitValue", 10000000LL, -3.1415, "Element2 - InitValue", 20000000LL, 2.71828, "Element3 - InitValue", 30000000LL, 1.414121, "Element4 - InitValue", 40000000LL, 1.61803398874);
    else
        return new PerformanceElement4 (PhysicalElement::CreateParams (db, modelId, classId, category, Dgn::Placement3d (), Dgn::DgnElement::Code (), id, Dgn::DgnElementId ()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement4CPtr PerformanceElement4::Insert ()
    {
    return GetDgnDb ().Elements ().Insert<PerformanceElement4> (*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement4CPtr PerformanceElement4::Update ()
    {
    return GetDgnDb ().Elements ().Update<PerformanceElement4> (*this);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::CreateElements (int numInstances, Utf8CP className, bvector<DgnElementPtr>& elements, Utf8String modelCode, bool specifyProperyValues) const
    {
    DgnClassId mclassId = DgnClassId (m_db->Schemas ().GetECClassId (DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalModel));
    PhysicalModelPtr targetModel = new PhysicalModel (PhysicalModel::CreateParams (*m_db, mclassId, DgnModel::CreateModelCode (modelCode)));
    EXPECT_EQ (DgnDbStatus::Success, targetModel->Insert ());       /* Insert the new model into the DgnDb */
    DgnCategoryId catid = DgnCategory::QueryHighestCategoryId (*m_db);
    DgnClassId classId = DgnClassId (m_db->Schemas ().GetECClassId (ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className));

    if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT1_CLASS))
        {
        for (int i = 0; i < numInstances; i++)
            {
            DgnElementId id = DgnElementId ((uint64_t)(2000000 + i));
            PerformanceElement1Ptr element = PerformanceElement1::Create (*m_db, targetModel->GetModelId (), classId, catid, id, specifyProperyValues);
            ASSERT_TRUE (element != nullptr);
            elements.push_back (element);
            }
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT2_CLASS))
        {
        for (int i = 0; i < numInstances; i++)
            {
            DgnElementId id = DgnElementId ((uint64_t)(2000000 + i));
            PerformanceElement2Ptr element = PerformanceElement2::Create (*m_db, targetModel->GetModelId (), classId, catid, id, specifyProperyValues);
            ASSERT_TRUE (element != nullptr);
            elements.push_back (element);
            }
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT3_CLASS))
        {
        for (int i = 0; i < numInstances; i++)
            {
            DgnElementId id = DgnElementId ((uint64_t)(2000000 + i));
            PerformanceElement3Ptr element = PerformanceElement3::Create (*m_db, targetModel->GetModelId (), classId, catid, id, specifyProperyValues);
            ASSERT_TRUE (element != nullptr);
            elements.push_back (element);
            }
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT4_CLASS))
        {
        for (int i = 0; i < numInstances; i++)
            {
            DgnElementId id = DgnElementId ((uint64_t)(2000000 + i));
            PerformanceElement4Ptr element = PerformanceElement4::Create (*m_db, targetModel->GetModelId (), classId, catid, id, specifyProperyValues);
            ASSERT_TRUE (element != nullptr);
            elements.push_back (element);
            }
        }
    ASSERT_EQ (numInstances, (int)elements.size ());
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement1PropertyParams (BeSQLite::Statement& stmt, bool updateParams)
    {
    Utf8String stringVal = "Element1 - ";
    int64_t intVal = 10000000LL;
    double doubleVal = -3.1415;
    if (updateParams)
        {
        stringVal.append ("UpdatedValue");
        intVal = intVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append ("InitValue");
        }

    if ((DbResult::BE_SQLITE_OK != stmt.BindText (stmt.GetParameterIndex (":Prop1_1"), stringVal.c_str (), BeSQLite::Statement::MakeCopy::No)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindInt64 (stmt.GetParameterIndex (":Prop1_2"), intVal)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindDouble (stmt.GetParameterIndex (":Prop1_3"), doubleVal)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement2PropertyParams (BeSQLite::Statement& stmt, bool updateParams)
    {
    Utf8String stringVal = "Element2 - ";
    int64_t intVal = 20000000LL;
    double doubleVal = 2.71828;
    if (updateParams)
        {
        stringVal.append ("UpdatedValue");
        intVal = intVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append ("InitValue");
        }

    if ((DgnDbStatus::Success != BindElement1PropertyParams (stmt, updateParams)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindText (stmt.GetParameterIndex (":Prop2_1"), stringVal.c_str (), BeSQLite::Statement::MakeCopy::No)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindInt64 (stmt.GetParameterIndex (":Prop2_2"), intVal)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindDouble (stmt.GetParameterIndex (":Prop2_3"), doubleVal)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement3PropertyParams (BeSQLite::Statement& stmt, bool updateParams)
    {
    Utf8String stringVal = "Element3 - ";
    int64_t intVal = 30000000LL;
    double doubleVal = 1.414121;
    if (updateParams)
        {
        stringVal.append ("UpdatedValue");
        intVal = intVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append ("InitValue");
        }

    if ((DgnDbStatus::Success != BindElement2PropertyParams (stmt, updateParams)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindText (stmt.GetParameterIndex (":Prop3_1"), stringVal.c_str (), BeSQLite::Statement::MakeCopy::No)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindInt64 (stmt.GetParameterIndex (":Prop3_2"), intVal)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindDouble (stmt.GetParameterIndex (":Prop3_3"), doubleVal)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement4PropertyParams (BeSQLite::Statement& stmt, bool updateParams)
    {
    Utf8String stringVal = "Element4 - ";
    int64_t intVal = 40000000LL;
    double doubleVal = 1.61803398874;
    if (updateParams)
        {
        stringVal.append ("UpdatedValue");
        intVal = intVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append ("InitValue");
        }

    if ((DgnDbStatus::Success != BindElement3PropertyParams (stmt, updateParams)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindText (stmt.GetParameterIndex (":Prop4_1"), stringVal.c_str (), BeSQLite::Statement::MakeCopy::No)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindInt64 (stmt.GetParameterIndex (":Prop4_2"), intVal)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindDouble (stmt.GetParameterIndex (":Prop4_3"), doubleVal)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
void PerformanceElementsCRUDTestFixture::BindParams (DgnElementPtr& element, BeSQLite::Statement& stmt, Utf8CP className)
    {
    bool updateParams = false;
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindId (stmt.GetParameterIndex (":Id"), element->GetElementId ()));
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindId (stmt.GetParameterIndex (":ModelId"), element->GetModelId ()));
    DgnAuthority::Code elementCode = DgnAuthority::CreateDefaultCode ();
    if (elementCode.IsEmpty ())
        {
        ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindNull (stmt.GetParameterIndex (":Code")));
        }
    else
        {
        ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindText (stmt.GetParameterIndex (":Code"), elementCode.GetValue ().c_str (), BeSQLite::Statement::MakeCopy::No));
        }
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindId (stmt.GetParameterIndex (":CodeAuthorityId"), elementCode.GetAuthority ()));
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindText (stmt.GetParameterIndex (":CodeNameSpace"), elementCode.GetNameSpace ().c_str (), BeSQLite::Statement::MakeCopy::No));
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindId (stmt.GetParameterIndex (":ParentId"), element->GetParentId ()));

    if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT1_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement1PropertyParams (stmt, updateParams));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT2_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement2PropertyParams (stmt, updateParams));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT3_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement3PropertyParams (stmt, updateParams));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT4_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement4PropertyParams (stmt, updateParams));
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
void PerformanceElementsCRUDTestFixture::BindUpdateParams (BeSQLite::Statement& stmt, Utf8CP className)
    {
    bool updateParams = true;
    if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT1_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement1PropertyParams (stmt, updateParams));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT2_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement2PropertyParams (stmt, updateParams));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT3_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement3PropertyParams (stmt, updateParams));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT4_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement4PropertyParams (stmt, updateParams));
        }
    }

//BindParams Overloads for ECSql
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement1PropertyParams (ECSqlStatement& statement, bool updateParams)
    {
    Utf8String stringVal = "Element1 - ";
    int64_t intVal = 10000000LL;
    double doubleVal = -3.1416;

    if (updateParams)
        {
        stringVal.append ("UpdatedValue");
        intVal = intVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append ("InitValue");
        }

    if ((ECSqlStatus::Success != statement.BindText (statement.GetParameterIndex ("Prop1_1"), stringVal.c_str (), IECSqlBinder::MakeCopy::No)) ||
        (ECSqlStatus::Success != statement.BindInt64 (statement.GetParameterIndex ("Prop1_2"), intVal)) ||
        (ECSqlStatus::Success != statement.BindDouble (statement.GetParameterIndex ("Prop1_3"), doubleVal)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement2PropertyParams (ECSqlStatement& stmt, bool updateParams)
    {
    Utf8String stringVal = "Element2 - ";
    int64_t intVal = 20000000LL;
    double doubleVal = 2.71828;

    if (updateParams)
        {
        stringVal.append ("UpdatedValue");
        intVal = intVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append ("InitValue");
        }

    if ((DgnDbStatus::Success != BindElement1PropertyParams (stmt, updateParams)) ||
        (ECSqlStatus::Success != stmt.BindText (stmt.GetParameterIndex ("Prop2_1"), stringVal.c_str (), IECSqlBinder::MakeCopy::No)) ||
        (ECSqlStatus::Success != stmt.BindInt64 (stmt.GetParameterIndex ("Prop2_2"), intVal)) ||
        (ECSqlStatus::Success != stmt.BindDouble (stmt.GetParameterIndex ("Prop2_3"), doubleVal)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement3PropertyParams (ECSqlStatement& stmt, bool updateParams)
    {
    Utf8String stringVal = "Element3 - ";
    int64_t intVal = 30000000LL;
    double doubleVal = 1.414121;

    if (updateParams)
        {
        stringVal.append ("UpdatedValue");
        intVal = intVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append ("InitValue");
        }

    if ((DgnDbStatus::Success != BindElement2PropertyParams (stmt, updateParams)) ||
        (ECSqlStatus::Success != stmt.BindText (stmt.GetParameterIndex ("Prop3_1"), stringVal.c_str (), IECSqlBinder::MakeCopy::No)) ||
        (ECSqlStatus::Success != stmt.BindInt64 (stmt.GetParameterIndex ("Prop3_2"), intVal)) ||
        (ECSqlStatus::Success != stmt.BindDouble (stmt.GetParameterIndex ("Prop3_3"), doubleVal)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement4PropertyParams (ECSqlStatement& stmt, bool updateParams)
    {
    Utf8String stringVal = "Element4 - ";
    int64_t intVal = 40000000LL;
    double doubleVal = 1.61803398874;

    if (updateParams)
        {
        stringVal.append ("UpdatedValue");
        intVal = intVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append ("InitValue");
        }

    if ((DgnDbStatus::Success != BindElement3PropertyParams (stmt, updateParams)) ||
        (ECSqlStatus::Success != stmt.BindText (stmt.GetParameterIndex ("Prop4_1"), stringVal.c_str (), IECSqlBinder::MakeCopy::No)) ||
        (ECSqlStatus::Success != stmt.BindInt64 (stmt.GetParameterIndex ("Prop4_2"), intVal)) ||
        (ECSqlStatus::Success != stmt.BindDouble (stmt.GetParameterIndex ("Prop4_3"), doubleVal)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
void PerformanceElementsCRUDTestFixture::BindParams (DgnElementPtr& element, ECSqlStatement& stmt, Utf8CP className)
    {
    bool updateParams = false;
    ASSERT_EQ (ECSqlStatus::Success, stmt.BindId (stmt.GetParameterIndex ("ECInstanceId"), element->GetElementId ()));
    ASSERT_EQ (ECSqlStatus::Success, stmt.BindId (stmt.GetParameterIndex ("ModelId"), element->GetModelId ()));
    DgnAuthority::Code elementCode = DgnAuthority::CreateDefaultCode ();
    if (elementCode.IsEmpty ())
        {
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindNull (stmt.GetParameterIndex ("Code")));
        }
    else
        {
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindText (stmt.GetParameterIndex ("Code"), elementCode.GetValue ().c_str (), IECSqlBinder::MakeCopy::No));
        }
    ASSERT_EQ (ECSqlStatus::Success, stmt.BindId (stmt.GetParameterIndex ("CodeAuthorityId"), elementCode.GetAuthority ()));
    ASSERT_EQ (ECSqlStatus::Success, stmt.BindText (stmt.GetParameterIndex ("CodeNameSpace"), elementCode.GetNameSpace ().c_str (), IECSqlBinder::MakeCopy::No));
    ASSERT_EQ (ECSqlStatus::Success, stmt.BindId (stmt.GetParameterIndex ("ParentId"), element->GetParentId ()));

    if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT1_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement1PropertyParams (stmt, updateParams));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT2_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement2PropertyParams (stmt, updateParams));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT3_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement3PropertyParams (stmt, updateParams));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT4_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement4PropertyParams (stmt, updateParams));
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
void PerformanceElementsCRUDTestFixture::BindUpdateParams (ECSqlStatement& stmt, Utf8CP className)
    {
    bool updateParams = true;
    if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT1_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement1PropertyParams (stmt, updateParams));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT2_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement2PropertyParams (stmt, updateParams));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT3_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement3PropertyParams (stmt, updateParams));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT4_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement4PropertyParams (stmt, updateParams));
        }
    }

//Methods to verify Business Property Values returned by Sql Statements. 
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::ExtractElement1SelectParams (BeSQLite::Statement& stmt)
    {
    if ((0 != strcmp ("Element1 - InitValue", stmt.GetValueText (6))) ||
        (stmt.GetValueInt64 (7) != 10000000) ||
        (stmt.GetValueDouble (8) != -3.1415))
        return DgnDbStatus::ReadError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::ExtractElement2SelectParams (BeSQLite::Statement& stmt)
    {
    if ((DgnDbStatus::Success != ExtractElement1SelectParams (stmt)) ||
        (0 != strcmp ("Element2 - InitValue", stmt.GetValueText (9))) ||
        (stmt.GetValueInt64 (10) != 20000000) ||
        (stmt.GetValueDouble (11) != 2.71828))
        return DgnDbStatus::ReadError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::ExtractElement3SelectParams (BeSQLite::Statement& stmt)
    {
    if ((DgnDbStatus::Success != ExtractElement2SelectParams (stmt)) ||
        (0 != strcmp ("Element3 - InitValue", stmt.GetValueText (12))) ||
        (stmt.GetValueInt64 (13) != 30000000) ||
        (stmt.GetValueDouble (14) != 1.414121))
        return DgnDbStatus::ReadError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::ExtractElement4SelectParams (BeSQLite::Statement& stmt)
    {
    if ((DgnDbStatus::Success != ExtractElement3SelectParams (stmt)) ||
        (0 != strcmp ("Element4 - InitValue", stmt.GetValueText (15))) ||
        (stmt.GetValueInt64 (16) != 40000000) ||
        (stmt.GetValueDouble (17) != 1.61803398874))
        return DgnDbStatus::ReadError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
void PerformanceElementsCRUDTestFixture::ExtractSelectParams (BeSQLite::Statement& stmt, Utf8CP className)
    {
    if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT1_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, ExtractElement1SelectParams (stmt));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT2_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, ExtractElement2SelectParams (stmt));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT3_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, ExtractElement3SelectParams (stmt));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT4_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, ExtractElement4SelectParams (stmt));
        }
    }

//OverLoaded Methods to Verify Business property Values returned by ECSql Statements. 
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::ExtractElement1SelectParams (ECSqlStatement& stmt)
    {
    if ((0 != strcmp ("Element1 - InitValue", stmt.GetValueText (6))) ||
        (stmt.GetValueInt64 (7) != 10000000) ||
        (stmt.GetValueDouble (8) != -3.1415))
        return DgnDbStatus::ReadError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::ExtractElement2SelectParams (ECSqlStatement& stmt)
    {
    if ((DgnDbStatus::Success != ExtractElement1SelectParams (stmt)) ||
        (0 != strcmp ("Element2 - InitValue", stmt.GetValueText (9))) ||
        (stmt.GetValueInt64 (10) != 20000000) ||
        (stmt.GetValueDouble (11) != 2.71828))
        return DgnDbStatus::ReadError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::ExtractElement3SelectParams (ECSqlStatement& stmt)
    {
    if ((DgnDbStatus::Success != ExtractElement2SelectParams (stmt)) ||
        (0 != strcmp ("Element3 - InitValue", stmt.GetValueText (12))) ||
        (stmt.GetValueInt64 (13) != 30000000) ||
        (stmt.GetValueDouble (14) != 1.414121))
        return DgnDbStatus::ReadError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
DgnDbStatus PerformanceElementsCRUDTestFixture::ExtractElement4SelectParams (ECSqlStatement& stmt)
    {
    if ((DgnDbStatus::Success != ExtractElement3SelectParams (stmt)) ||
        (0 != strcmp ("Element4 - InitValue", stmt.GetValueText (15))) ||
        (stmt.GetValueInt64 (16) != 40000000) ||
        (stmt.GetValueDouble (17) != 1.61803398874))
        return DgnDbStatus::ReadError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
void PerformanceElementsCRUDTestFixture::ExtractSelectParams (ECSqlStatement& stmt, Utf8CP className)
    {
    if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT1_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, ExtractElement1SelectParams (stmt));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT2_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, ExtractElement2SelectParams (stmt));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT3_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, ExtractElement3SelectParams (stmt));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT4_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, ExtractElement4SelectParams (stmt));
        }
    }

//Methods to Generate Sql CRUD statements. 
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetInsertSql (Utf8CP className, Utf8StringR insertSql, DgnClassId classId) const
    {
    ECN::ECClassCP ecClass = m_db->Schemas ().GetECClass (ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className);

    insertSql = Utf8String ("INSERT INTO dgn_Element ([Id], ");
    Utf8String insertValuesSql (") VALUES (:Id, ");
    bool isFirstItem = true;
    for (auto prop : ecClass->GetProperties (true))
        {
        if (0 == strcmp ("LastMod", prop->GetName ().c_str ()))
            continue;
        if (!isFirstItem)
            {
            insertSql.append (", ");
            insertValuesSql.append (", ");
            }

        insertSql.append ("[").append (prop->GetName ()).append ("]");
        insertValuesSql.append (":").append (prop->GetName ());

        isFirstItem = false;
        }

    insertSql.append (", ECClassId");
    insertValuesSql.append (", ");
    Utf8String insertValues;
    insertValues.Sprintf ("%s%d", insertValuesSql.c_str (), (int)classId.GetValue ());
    insertSql.append (insertValues).append (")");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetSelectSql (Utf8CP className, Utf8StringR selectSql) const
    {
    ECN::ECClassCP ecClass = m_db->Schemas ().GetECClass (ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className);
    selectSql = "SELECT ";
    bool isFirstItem = true;
    for (auto prop : ecClass->GetProperties (true))
        {
        if (!isFirstItem)
            {
            selectSql.append (", ");
            }
        selectSql.append (prop->GetName ());
        isFirstItem = false;
        }
    selectSql.append (" FROM dgn_Element WHERE Id = ?");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetUpdateSql (Utf8CP className, Utf8StringR updateSql) const
    {
    ECN::ECClassCP ecClass = m_db->Schemas ().GetECClass (ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className);
    updateSql = "UPDATE dgn_Element SET ";
    bool isFirstItem = true;
    for (auto prop : ecClass->GetProperties (true))
        {
        if (0 == strcmp ("ModelId", prop->GetName ().c_str ()) || 0 == strcmp ("Code", prop->GetName ().c_str ()) || 0 == strcmp ("CodeAuthorityId", prop->GetName ().c_str ()) || 0 == strcmp ("CodeNameSpace", prop->GetName ().c_str ()) || 0 == strcmp ("ParentId", prop->GetName ().c_str ()) || 0 == strcmp ("LastMod", prop->GetName ().c_str ()))
            continue;
        if (!isFirstItem)
            {
            updateSql.append (", ");
            }
        updateSql.append (prop->GetName ()).append (" = :").append (prop->GetName ());
        isFirstItem = false;
        }
    updateSql.append (" WHERE Id = :Id");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetDeleteSql (Utf8StringR deleteSql) const
    {
    deleteSql = "DELETE FROM dgn_Element WHERE Id = ?";
    }

//Overloads to Generate ECSql statements. 
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetInsertECSql (Utf8CP className, Utf8StringR insertECSql) const
    {
    ECN::ECClassCP ecClass = m_db->Schemas ().GetECClass (ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className);
    Utf8String ecClassName = ECSqlBuilder::ToECSqlSnippet (*ecClass);

    insertECSql = Utf8String ("INSERT INTO ");
    insertECSql.append (ecClassName).append (" ([ECInstanceId], ");
    Utf8String insertValuesSql (") VALUES (:[ECInstanceId], ");
    bool isFirstItem = true;
    for (auto prop : ecClass->GetProperties (true))
        {
        if (0 == strcmp ("LastMod", prop->GetName ().c_str ()))
            continue;
        if (!isFirstItem)
            {
            insertECSql.append (", ");
            insertValuesSql.append (", ");
            }

        insertECSql.append ("[").append (prop->GetName ()).append ("]");
        insertValuesSql.append (":[").append (prop->GetName ()).append ("]");

        isFirstItem = false;
        }

    insertECSql.append (insertValuesSql).append (")");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetSelectECSql (Utf8CP className, Utf8StringR selectECSql) const
    {
    ECN::ECClassCP ecClass = m_db->Schemas ().GetECClass (ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className);

    Utf8String ecClassName = ECSqlBuilder::ToECSqlSnippet (*ecClass);
    selectECSql = "SELECT ";
    bool isFirstItem = true;
    for (auto prop : ecClass->GetProperties (true))
        {
        if (!isFirstItem)
            {
            selectECSql.append (", ");
            }
        selectECSql.append (prop->GetName ());
        isFirstItem = false;
        }

    selectECSql.append (" FROM ").append (ecClassName).append (" WHERE ECInstanceId = ?");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetUpdateECSql (Utf8CP className, Utf8StringR updateECSql) const
    {
    ECN::ECClassCP ecClass = m_db->Schemas ().GetECClass (ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className);
    Utf8String ecClassName = ECSqlBuilder::ToECSqlSnippet (*ecClass);
    updateECSql = "UPDATE ";
    updateECSql.append (ecClassName).append (" SET ");
    bool isFirstItem = true;
    for (auto prop : ecClass->GetProperties (true))
        {
        if (0 == strcmp ("ModelId", prop->GetName ().c_str ()) || 0 == strcmp ("Code", prop->GetName ().c_str ()) || 0 == strcmp ("CodeAuthorityId", prop->GetName ().c_str ()) || 0 == strcmp ("CodeNameSpace", prop->GetName ().c_str ()) || 0 == strcmp ("ParentId", prop->GetName ().c_str ()) || 0 == strcmp ("LastMod", prop->GetName ().c_str ()))
            continue;
        if (!isFirstItem)
            {
            updateECSql.append (", ");
            }
        updateECSql.append (prop->GetName ()).append (" = :").append (prop->GetName ());
        isFirstItem = false;
        }
    updateECSql.append (" WHERE ECInstanceId = :ecInstanceId");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetDeleteECSql (Utf8CP className, Utf8StringR deleteECSql) const
    {
    ECN::ECClassCP ecClass = m_db->Schemas ().GetECClass (ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className);
    Utf8String ecClassName = ECSqlBuilder::ToECSqlSnippet (*ecClass);
    deleteECSql = "DELETE FROM ONLY ";
    deleteECSql.append (ecClassName).append (" WHERE ").append ("ECInstanceId = ?");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ElementApiInsertTime(Utf8CP className, int initialInstanceCount, int opCount)
    {
    WString wClassName;
    wClassName.AssignUtf8 (className);
    WPrintfString dbName (L"ElementApiInsert%ls_%d.idgndb", wClassName.c_str (), opCount);
    SetUpTestDgnDb (dbName, className, initialInstanceCount);

    bvector<DgnElementPtr> testElements;
    CreateElements (opCount, className, testElements, "ElementApiInstances", true);
    ASSERT_EQ(opCount, (int) testElements.size());

    StopWatch timer (true);
    for (DgnElementPtr& element : testElements)
        {
        DgnDbStatus stat = DgnDbStatus::Success;
        element->Insert (&stat);
        ASSERT_EQ (DgnDbStatus::Success, stat);
        }
    timer.Stop ();
    LOGTODB (TEST_DETAILS, timer.GetElapsedSeconds (), Utf8PrintfString ("Element API Insert '%s' [Initial count: %d]", className, initialInstanceCount).c_str (), (int)testElements.size ());
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ElementApiSelectTime(Utf8CP className, int initialInstanceCount, int opCount)
    {
    WString wClassName;
    wClassName.AssignUtf8 (className);
    WPrintfString dbName (L"ElementApiSelect%ls_%d.idgndb", wClassName.c_str (), opCount);
    SetUpTestDgnDb (dbName, className, initialInstanceCount);

    const int elementIdIncrement = DetermineElementIdIncrement();
    StopWatch timer (true);
    for (uint64_t i = 0; i < opCount; i++)
        {
        const DgnElementId id(s_firstElementId + i*elementIdIncrement);
        DgnElementCPtr element = m_db->Elements().GetElement(id);
        ASSERT_TRUE(element != nullptr);
        }
    timer.Stop ();
    LOGTODB (TEST_DETAILS, timer.GetElapsedSeconds (), Utf8PrintfString ("Element API Read '%s' [Initial count: %d]", className, initialInstanceCount).c_str (), opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ElementApiUpdateTime(Utf8CP className, int initialInstanceCount, int opCount)
    {
    WPrintfString dbName (L"ElementApiUpdate%ls_%d.idgndb", WString (className, BentleyCharEncoding::Utf8).c_str (), opCount);
    SetUpTestDgnDb (dbName, className, initialInstanceCount);

    const int elementIdIncrement = DetermineElementIdIncrement();

    StopWatch timer (true);
    for (uint64_t i = 0; i < opCount; i++)
        {
        const DgnElementId id(s_firstElementId + i*elementIdIncrement);
        DgnElementPtr element = m_db->Elements().GetForEdit<DgnElement>(id);
        ASSERT_TRUE(element != nullptr);

        DgnDbStatus stat = DgnDbStatus::Success;
        element->Update (&stat);
        ASSERT_EQ (DgnDbStatus::Success, stat);
        }

    timer.Stop ();
    LOGTODB (TEST_DETAILS, timer.GetElapsedSeconds (), Utf8PrintfString ("Element API Update '%s' [Initial count: %d]", className, initialInstanceCount).c_str (), opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ElementApiDeleteTime(Utf8CP className, int initialInstanceCount, int opCount)
    {
    WPrintfString dbName(L"ElementApiDelete%ls_%d.idgndb", WString(className, BentleyCharEncoding::Utf8).c_str(), opCount);
    SetUpTestDgnDb(dbName, className, initialInstanceCount);

    const int elementIdIncrement = DetermineElementIdIncrement();

    StopWatch timer(true);
    for (uint64_t i = 0; i < opCount; i++)
        {
        const DgnElementId id(s_firstElementId + i*elementIdIncrement);
        const DgnDbStatus stat = m_db->Elements().Delete(id);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        }
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), Utf8PrintfString("Element API Delete '%s' [Initial count: %d]", className, initialInstanceCount).c_str(), opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ECSqlInsertTime(Utf8CP className, int initialInstanceCount, int opCount)
    {
    WPrintfString dbName (L"ECSqlInsert%ls_%d.idgndb", WString (className, BentleyCharEncoding::Utf8).c_str (), opCount);
    SetUpTestDgnDb (dbName, className, initialInstanceCount);

    bvector<DgnElementPtr> testElements;
    CreateElements (opCount, className, testElements, "ECSqlInstances", false);
    ASSERT_EQ(opCount, (int) testElements.size());

    ECSqlStatement stmt;
    Utf8String insertECSql;
    GetInsertECSql (className, insertECSql);
    //printf ("\n Insert ECSql %s : %s \n", className, insertECSql.c_str ());

    StopWatch timer (true);
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, insertECSql.c_str ()));
    for (DgnElementPtr& element : testElements)
        {
        BindParams (element, stmt, className);
        if (DbResult::BE_SQLITE_DONE != stmt.Step () || m_db->GetModifiedRowCount () == 0)
            ASSERT_TRUE (false);
        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    LOGTODB (TEST_DETAILS, timer.GetElapsedSeconds (), Utf8PrintfString ("ECSQL INSERT '%s' [Initial count: %d]", className, initialInstanceCount).c_str (), opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ECSqlSelectTime(Utf8CP className, int initialInstanceCount, int opCount)
    {
    WPrintfString dbName (L"ECSqlSelect%ls_%d.idgndb", WString (className, BentleyCharEncoding::Utf8).c_str (), initialInstanceCount);
    SetUpTestDgnDb (dbName, className, initialInstanceCount);

    ECSqlStatement stmt;
    Utf8String selectECSql;
    GetSelectECSql (className, selectECSql);
    //printf ("\n Select ECSql %s : %s \n", className, selectECSql.c_str ());

    const int elementIdIncrement = DetermineElementIdIncrement();

    StopWatch timer (true);
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, selectECSql.c_str ()));
    //printf ("\n Native Sql %s : %s \n", className, stmt.GetNativeSql());
    for (int i = 0; i < opCount; i++)
        {
        const ECInstanceId id(s_firstElementId + i*elementIdIncrement);
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindId (1, id));
        ASSERT_EQ (DbResult::BE_SQLITE_ROW, stmt.Step ());
        ExtractSelectParams (stmt, className);
        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    LOGTODB (TEST_DETAILS, timer.GetElapsedSeconds (), Utf8PrintfString ("ECSQL SELECT '%s' [Initial count: %d]", className, initialInstanceCount).c_str (), opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ECSqlUpdateTime(Utf8CP className, int initialInstanceCount, int opCount)
    {
    WPrintfString dbName (L"ECSqlUpdate%ls_%d.idgndb", WString (className, BentleyCharEncoding::Utf8).c_str (), opCount);
    SetUpTestDgnDb (dbName, className, initialInstanceCount);

    ECSqlStatement stmt;
    Utf8String updateECSql;
    GetUpdateECSql (className, updateECSql);
    //printf ("\n Update ECSql %s : %s \n", className, updateECSql.c_str ());

    const int elementIdIncrement = DetermineElementIdIncrement();

    StopWatch timer (true);
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, updateECSql.c_str ()));
    for (int i = 0; i < opCount; i++)
        {
        const ECInstanceId id(s_firstElementId + i*elementIdIncrement);
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindId (stmt.GetParameterIndex ("ecInstanceId"), id));
        BindUpdateParams (stmt, className);
        if (DbResult::BE_SQLITE_DONE != stmt.Step () || m_db->GetModifiedRowCount () == 0)
            ASSERT_TRUE (false);
        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    LOGTODB (TEST_DETAILS, timer.GetElapsedSeconds (), Utf8PrintfString ("ECSQL UPDATE '%s' [Initial count: %d]", className, initialInstanceCount).c_str (), opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ECSqlDeleteTime(Utf8CP className, int initialInstanceCount, int opCount)
    {
    WPrintfString dbName (L"ECSqlDelete%ls_%d.idgndb", WString (className, BentleyCharEncoding::Utf8).c_str (), opCount);
    SetUpTestDgnDb (dbName, className, initialInstanceCount);

    ECSqlStatement stmt;
    Utf8String deleteECSql;
    GetDeleteECSql (className, deleteECSql);
    //printf ("\n Delete ECSql %s : %s \n", className, deleteECSql.c_str ());

    const int elementIdIncrement = DetermineElementIdIncrement();

    StopWatch timer (true);
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, deleteECSql.c_str ()));
    for (int i = 0; i < opCount; i++)
        {
        const ECInstanceId id(s_firstElementId + i*elementIdIncrement);
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindId (1, id));
        if (DbResult::BE_SQLITE_DONE != stmt.Step () || m_db->GetModifiedRowCount () == 0)
            ASSERT_TRUE (false);
        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    LOGTODB (TEST_DETAILS, timer.GetElapsedSeconds (), Utf8PrintfString ("ECSQL DELETE '%s' [Initial count: %d]", className, initialInstanceCount).c_str (), opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::SqlInsertTime(Utf8CP className, int initialInstanceCount, int opCount)
    {
    WPrintfString dbName (L"SqlInsert%ls_%d.idgndb", WString (className, BentleyCharEncoding::Utf8).c_str (), opCount);
    SetUpTestDgnDb (dbName, className, initialInstanceCount);

    bvector<DgnElementPtr> testElements;
    CreateElements (opCount, className, testElements, "SqlInstances", false);
    ASSERT_EQ(opCount, (int) testElements.size());

    BeSQLite::Statement stmt;
    Utf8String insertSql;
    DgnElementPtr firstElement = testElements.front ();
    GetInsertSql (className, insertSql, firstElement->GetElementClassId ());
    //printf ("\n Insert Sql %s : %s \n", className, insertSql.c_str ());

    StopWatch timer (true);
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.Prepare (*m_db, insertSql.c_str ()));
    for (DgnElementPtr& element : testElements)
        {
        BindParams (element, stmt, className);
        if (DbResult::BE_SQLITE_DONE != stmt.Step () || m_db->GetModifiedRowCount () == 0)
            ASSERT_TRUE (false);
        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    LOGTODB (TEST_DETAILS, timer.GetElapsedSeconds (), Utf8PrintfString ("SQLite INSERT '%s' [Initial count: %d]", className, initialInstanceCount).c_str (), opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::SqlSelectTime(Utf8CP className, int initialInstanceCount, int opCount)
    {
    WPrintfString dbName (L"SqlSelect%ls_%d.idgndb", WString (className, BentleyCharEncoding::Utf8).c_str (), opCount);
    SetUpTestDgnDb (dbName, className, initialInstanceCount);

    BeSQLite::Statement stmt;
    Utf8String selectSql;
    GetSelectSql (className, selectSql);
    //printf ("\n Select Sql %s : %s \n", className, selectSql.c_str ());

    const int elementIdIncrement = DetermineElementIdIncrement();

    StopWatch timer (true);
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.Prepare (*m_db, selectSql.c_str ()));
    for (int i = 0; i < opCount; i++)
        {
        const ECInstanceId id(s_firstElementId + i*elementIdIncrement);
        ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindId (1, id));
        ASSERT_EQ (DbResult::BE_SQLITE_ROW, stmt.Step ());
        ExtractSelectParams (stmt, className);
        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    LOGTODB (TEST_DETAILS, timer.GetElapsedSeconds (), Utf8PrintfString ("SQLite SELECT '%s' [Initial count: %d]", className, initialInstanceCount).c_str (), opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::SqlUpdateTime(Utf8CP className, int initialInstanceCount, int opCount)
    {
    WPrintfString dbName (L"SqlUpdate%ls_%d.idgndb", WString (className, BentleyCharEncoding::Utf8).c_str (), opCount);
    SetUpTestDgnDb (dbName, className, initialInstanceCount);

    BeSQLite::Statement stmt;
    Utf8String updateSql;
    GetUpdateSql (className, updateSql);
    //printf ("\n Update Sql %s : %s \n", className, updateSql.c_str ());

    const int elementIdIncrement = DetermineElementIdIncrement();

    StopWatch timer (true);
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.Prepare (*m_db, updateSql.c_str ()));
    for (int i = 0; i < opCount; i++)
        {
        const ECInstanceId id(s_firstElementId + i*elementIdIncrement);
        ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindId (stmt.GetParameterIndex (":Id"), id));
        BindUpdateParams (stmt, className);
        if (DbResult::BE_SQLITE_DONE != stmt.Step () || m_db->GetModifiedRowCount () == 0)
            ASSERT_TRUE (false);
        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    LOGTODB (TEST_DETAILS, timer.GetElapsedSeconds (), Utf8PrintfString ("SQLite UPDATE '%s' [Initial count: %d]", className, initialInstanceCount).c_str (), opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::SqlDeleteTime(Utf8CP className, int initialInstanceCount, int opCount)
    {
    WPrintfString dbName (L"SqlDelete%ls_%d.idgndb", WString (className, BentleyCharEncoding::Utf8).c_str (), opCount);
    SetUpTestDgnDb (dbName, className, initialInstanceCount);

    BeSQLite::Statement stmt;
    Utf8String deleteSql;
    GetDeleteSql (deleteSql);
    //printf ("\n Delete Sql %s : %s \n", className, deleteSql.c_str ());

    const int elementIdIncrement = DetermineElementIdIncrement();

    StopWatch timer (true);
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.Prepare (*m_db, deleteSql.c_str ()));
    for (int i = 0; i < opCount; i++)
        {
        const ECInstanceId id(s_firstElementId + i*elementIdIncrement);
        ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindId (1, id));
        if (DbResult::BE_SQLITE_DONE != stmt.Step () || m_db->GetModifiedRowCount () == 0)
            ASSERT_TRUE (false);
        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    LOGTODB (TEST_DETAILS, timer.GetElapsedSeconds (), Utf8PrintfString ("SQLite DELETE '%s' [Initial count: %d]", className, initialInstanceCount).c_str (), opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, ElementsInsertSql)
    {
    SqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS);
    SqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS);
    SqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS);
    SqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, ElementsInsertECSql)
    {
    ECSqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS);
    ECSqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS);
    ECSqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS);
    ECSqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, ElementsInsertDgnApi)
    {
    ElementApiInsertTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS);
    ElementApiInsertTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS);
    ElementApiInsertTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS);
    ElementApiInsertTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, ElementsReadSql)
    {
    SqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS);
    SqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS);
    SqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS);
    SqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, ElementsReadECSql)
    {
    ECSqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS);
    ECSqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS);
    ECSqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS);
    ECSqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, ElementsReadDgnApi)
    {
    ElementApiSelectTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS);
    ElementApiSelectTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS);
    ElementApiSelectTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS);
    ElementApiSelectTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, ElementsUpdateSql)
    {
    SqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS);
    SqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS);
    SqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS);
    SqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, ElementsUpdateECSql)
    {
    ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS);
    ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS);
    ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS);
    ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, ElementsUpdateDgnApi)
    {
    ElementApiUpdateTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS);
    ElementApiUpdateTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS);
    ElementApiUpdateTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS);
    ElementApiUpdateTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, ElementsDeleteSql)
    {
    SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS);
    SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS);
    SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS);
    SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, ElementsDeleteECSql)
    {
    ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS);
    ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS);
    ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS);
    ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, ElementsDeleteDgnApi)
    {
    ElementApiDeleteTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS);
    ElementApiDeleteTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS);
    ElementApiDeleteTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS);
    ElementApiDeleteTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS);
    }