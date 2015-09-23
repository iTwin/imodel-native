/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/PerformanceElementsTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeTimeUtilities.h>
#include <ECDb/ECDbApi.h>
#include "PerformanceTestFixture.h"
#include "../TestFixture/DgnDbTestFixtures.h"
#include "PerformanceElements.h"

#include <Logging/bentleylogging.h>

#define LOG (*NativeLogging::LoggingManager::GetLogger (L"DgnDbPerformance"))

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_DGNDB_UNIT_TESTS_NAMESPACE
using namespace ElementInsertPerformanceTestNamespace;

HANDLER_DEFINE_MEMBERS(SimpleElementHandler)
HANDLER_DEFINE_MEMBERS(PerformanceElement1Handler)
HANDLER_DEFINE_MEMBERS(PerformanceElement2Handler)
HANDLER_DEFINE_MEMBERS(PerformanceElement3Handler)
HANDLER_DEFINE_MEMBERS(PerformanceElement4Handler)
HANDLER_DEFINE_MEMBERS(PerformanceElement4bHandler)
DOMAIN_DEFINE_MEMBERS(PerformanceElementsTestDomain)

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElementsTestDomain::PerformanceElementsTestDomain() : DgnDomain(ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, "Test Schema", 1)
    {
    RegisterHandler(SimpleElementHandler::GetHandler());
    RegisterHandler(PerformanceElement1Handler::GetHandler());
    RegisterHandler(PerformanceElement2Handler::GetHandler());
    RegisterHandler(PerformanceElement3Handler::GetHandler());
    RegisterHandler(PerformanceElement4Handler::GetHandler());
    RegisterHandler(PerformanceElement4bHandler::GetHandler());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElementsTestDomain::RegisterDomainAndImportSchema(DgnDbR db, ECN::ECSchemaR schema)
    {
    DgnDomains::RegisterDomain(PerformanceElementsTestDomain::GetDomain()); 

    ECN::ECSchemaReadContextPtr schemaContext = ECN::ECSchemaReadContext::CreateContext();
    schemaContext->AddSchema(schema);
    DgnBaseDomain::GetDomain().ImportSchema(db, schemaContext->GetCache());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElement1::GetParams(bvector<Utf8CP>& params)
    {
    params.push_back("Prop1_1");
    params.push_back("Prop1_2");
    params.push_back("Prop1_3");
    }

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
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElement1::_GetInsertParams(bvector<Utf8CP>& insertParams)
    {
    T_Super::_GetInsertParams(insertParams);
    GetParams(insertParams);
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElement1::_GetUpdateParams(bvector<Utf8CP>& updateParams)
    {
    GetParams(updateParams);
    T_Super::_GetUpdateParams(updateParams);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement1::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus status = T_Super::_BindUpdateParams(statement);
    if (DgnDbStatus::Success != status)
        return status;

    int random = rand();
    Utf8PrintfString newString("New Prop1_1 string %d", random);
    m_prop1_1 = newString;
    m_prop1_2 = rand();
    m_prop1_3 = 3.14 * rand();
    return BindParams(statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement1Ptr PerformanceElement1::Create(Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category)
    {
    PerformanceElement1Ptr ptr = new PerformanceElement1(PhysicalElement::CreateParams(db, modelId, classId, category));
    if (!ptr.IsValid())
        return nullptr;
    return ptr.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement1CPtr PerformanceElement1::Insert()
    {
    return GetDgnDb().Elements().Insert<PerformanceElement1>(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement1CPtr PerformanceElement1::Update()
    {
    return GetDgnDb().Elements().Update<PerformanceElement1>(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElement2::GetParams(bvector<Utf8CP>& params)
    {
    params.push_back("Prop2_1");
    params.push_back("Prop2_2");
    params.push_back("Prop2_3");
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
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElement2::_GetInsertParams(bvector<Utf8CP>& insertParams)
    {
    T_Super::_GetInsertParams(insertParams);
    GetParams(insertParams);
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElement2::_GetUpdateParams(bvector<Utf8CP>& updateParams)
    {
    GetParams(updateParams);
    T_Super::_GetUpdateParams(updateParams);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement2::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus status = T_Super::_BindUpdateParams(statement);
    if (DgnDbStatus::Success != status)
        return status;

    int random = rand();
    Utf8PrintfString newString("New Prop2_1 string %d", random);
    m_prop2_1 = newString;
    m_prop2_2 = rand();
    m_prop2_3 = 3.14 * rand();
    return BindParams(statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement2Ptr PerformanceElement2::Create(Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category)
    {
    PerformanceElement2Ptr ptr = new PerformanceElement2(PhysicalElement::CreateParams(db, modelId, classId, category));
    if (!ptr.IsValid())
        return nullptr;
    return ptr.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement2CPtr PerformanceElement2::Insert()
    {
    return GetDgnDb().Elements().Insert<PerformanceElement2>(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement2CPtr PerformanceElement2::Update()
    {
    return GetDgnDb().Elements().Update<PerformanceElement2>(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElement3::GetParams(bvector<Utf8CP>& params)
    {
    params.push_back("Prop3_1");
    params.push_back("Prop3_2");
    params.push_back("Prop3_3");
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
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElement3::_GetInsertParams(bvector<Utf8CP>& insertParams)
    {
    T_Super::_GetInsertParams(insertParams);
    GetParams(insertParams);
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElement3::_GetUpdateParams(bvector<Utf8CP>& updateParams)
    {
    GetParams(updateParams);
    T_Super::_GetUpdateParams(updateParams);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement3::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus status = T_Super::_BindUpdateParams(statement);
    if (DgnDbStatus::Success != status)
        return status;

    int random = rand();
    Utf8PrintfString newString("New Prop3_1 string %d", random);
    m_prop3_1 = newString;
    m_prop3_2 = rand();
    m_prop3_3 = 3.14 * rand();
    return BindParams(statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement3Ptr PerformanceElement3::Create(Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category)
    {
    PerformanceElement3Ptr ptr = new PerformanceElement3(PhysicalElement::CreateParams(db, modelId, classId, category));
    if (!ptr.IsValid())
        return nullptr;
    return ptr.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement3CPtr PerformanceElement3::Insert()
    {
    return GetDgnDb().Elements().Insert<PerformanceElement3>(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement3CPtr PerformanceElement3::Update()
    {
    return GetDgnDb().Elements().Update<PerformanceElement3>(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElement4::GetParams(bvector<Utf8CP>& params)
    {
    params.push_back("Prop4_1");
    params.push_back("Prop4_2");
    params.push_back("Prop4_3");
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
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElement4::_GetInsertParams(bvector<Utf8CP>& insertParams)
    {
    T_Super::_GetInsertParams(insertParams);
    GetParams(insertParams);
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElement4::_GetUpdateParams(bvector<Utf8CP>& updateParams)
    {
    GetParams(updateParams);
    T_Super::_GetUpdateParams(updateParams);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement4::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus status = T_Super::_BindUpdateParams(statement);
    if (DgnDbStatus::Success != status)
        return status;

    int random = rand();
    Utf8PrintfString newString("New Prop4_1 string %d", random);
    m_prop4_1 = newString;
    m_prop4_2 = rand();
    m_prop4_3 = 3.14 * rand();
    return BindParams(statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement4Ptr PerformanceElement4::Create(Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category)
    {
    PerformanceElement4Ptr ptr = new PerformanceElement4(PhysicalElement::CreateParams(db, modelId, classId, category));
    if (!ptr.IsValid())
        return nullptr;
    return ptr.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement4CPtr PerformanceElement4::Insert()
    {
    return GetDgnDb().Elements().Insert<PerformanceElement4>(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement4CPtr PerformanceElement4::Update()
    {
    return GetDgnDb().Elements().Update<PerformanceElement4>(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElement4b::GetParams(bvector<Utf8CP>& params)
    {
    params.push_back("Prop4b_1");
    params.push_back("Prop4b_2");
    params.push_back("Prop4b_3");
    params.push_back("Prop4b_4");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement4b::BindParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    if ((ECSqlStatus::Success != statement.BindText(statement.GetParameterIndex("Prop4b_1"), m_prop4b_1.c_str(), IECSqlBinder::MakeCopy::Yes)) ||
        (ECSqlStatus::Success != statement.BindInt64(statement.GetParameterIndex("Prop4b_2"), m_prop4b_2)) ||
        (ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex("Prop4b_3"), m_prop4b_3)) ||
        (ECSqlStatus::Success != statement.BindPoint3D(statement.GetParameterIndex("Prop4b_4"), m_prop4b_4)))
        return DgnDbStatus::BadArg;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElement4b::_GetInsertParams(bvector<Utf8CP>& insertParams)
    {
    T_Super::_GetInsertParams(insertParams);
    GetParams(insertParams);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement4b::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat = BindParams(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindInsertParams(statement);    
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElement4b::_GetUpdateParams(bvector<Utf8CP>& updateParams)
    {
    GetParams(updateParams);
    T_Super::_GetUpdateParams(updateParams);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement4b::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus status = T_Super::_BindUpdateParams(statement);
    if (DgnDbStatus::Success != status)
        return status;

    int random = rand();
    Utf8PrintfString newString("New Prop4b_1 string %d", random);
    m_prop4b_1 = newString;
    m_prop4b_2 = rand();
    m_prop4b_3 = 3.14 * rand();
    m_prop4b_4.x = 1.1 * rand();
    m_prop4b_4.y = 1.1 * rand();
    m_prop4b_4.z = 1.1 * rand();
    return BindParams(statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement4bPtr PerformanceElement4b::Create(Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category)
    {
    PerformanceElement4bPtr ptr = new PerformanceElement4b(PhysicalElement::CreateParams(db, modelId, classId, category));
    if (!ptr.IsValid())
        return nullptr;
    return ptr.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement4bCPtr PerformanceElement4b::Insert()
    {
    return GetDgnDb().Elements().Insert<PerformanceElement4b>(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement4bCPtr PerformanceElement4b::Update()
    {
    return GetDgnDb().Elements().Update<PerformanceElement4b>(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
SimpleElementPtr SimpleElement::Create(Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category)
    {
    SimpleElementPtr ptr = new SimpleElement(DgnElement::CreateParams(db, modelId, classId));
    if (!ptr.IsValid())
        return nullptr;
    return ptr.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
SimpleElementCPtr SimpleElement::Insert()
    {
    return GetDgnDb().Elements().Insert<SimpleElement>(*this).get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
SimpleElementCPtr SimpleElement::Update()
    {
    return GetDgnDb().Elements().Update<SimpleElement>(*this).get();
    }

//=======================================================================================
// @bsiclass                                                  Carole.MacDonald    09/15
//=======================================================================================
struct PerformanceElementsTestFixture : public PerformanceElementTestFixture
    {
    private:
        void CreateElements(int numInstances, Utf8CP schemaName, Utf8CP className, bvector<DgnElementPtr>& elements);
        void InitializeProject(WCharCP dbName);

    protected:
    //PerformanceTestingFrameWork     m_testObj;
    void TimeInsertion(int numInstances, Utf8CP schemaName, Utf8CP className, Utf8String testcaseName, Utf8String testName);
    void TimeUpdate(int numInstances, Utf8CP schemaName, Utf8CP className, Utf8String testcaseName, Utf8String testName);

    public:
        PerformanceElementsTestFixture()
            {
            srand ((uint32_t)BeTimeUtilities::QueryMillisecondsCounter ());
            }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElementsTestFixture::InitializeProject(WCharCP dbName)
    {
    SetupProject(L"3dMetricGeneral.idgndb", dbName, BeSQLite::Db::OpenMode::ReadWrite);
    ECN::ECSchemaReadContextPtr schemaContext = ECN::ECSchemaReadContext::CreateContext();
    BeFileName searchDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(searchDir);
    searchDir.AppendToPath(L"ECSchemas").AppendToPath(L"Dgn");
    schemaContext->AddSchemaPath(searchDir.GetName ());

    ECN::ECSchemaPtr schema = nullptr;
    if (ECN::SCHEMA_READ_STATUS_Success != ECN::ECSchema::ReadFromXmlString(schema, s_testSchemaXml, *schemaContext))
        return;

    PerformanceElementsTestDomain::RegisterDomainAndImportSchema(*m_db, *schema);

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElementsTestFixture::CreateElements(int numInstances, Utf8CP schemaName, Utf8CP className, bvector<DgnElementPtr>& elements)
    {
    DgnModelPtr model = CreatePhysicalModel();
    DgnCategoryId catid = m_db->Categories().QueryHighestId();
    DgnClassId mclassId = DgnClassId(m_db->Schemas().GetECClassId(schemaName, className));
    
    if (0 == strcmp(className, ELEMENT_PERFORMANCE_SIMPLEELEMENT_CLASS))
        {
        for (int i = 0; i < numInstances; i++)
            {
            SimpleElementPtr element = SimpleElement::Create(*m_db, model->GetModelId(), mclassId, catid);
            ASSERT_TRUE(element != nullptr);
            elements.push_back(element);
            }
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT1_CLASS))
        {
        for (int i = 0; i < numInstances; i++)
            {
            PerformanceElement1Ptr element = PerformanceElement1::Create(*m_db, model->GetModelId(), mclassId, catid);
            ASSERT_TRUE(element != nullptr);
            int random = rand();
            Utf8PrintfString str("Element1 - %d", random);
            element->SetString1(str.c_str());
            elements.push_back(element);
            }
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT2_CLASS))
        {
        for (int i = 0; i < numInstances; i++)
            {
            PerformanceElement2Ptr element = PerformanceElement2::Create(*m_db, model->GetModelId(), mclassId, catid);
            ASSERT_TRUE(element != nullptr);
            int random = rand();
            Utf8PrintfString str1("Element2 - %d", random);
            element->SetString1(str1.c_str());
            random = rand();
            Utf8PrintfString str2("Element2 - %d", random);
            element->SetString2(str2.c_str());
            elements.push_back(element);
            }
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT3_CLASS))
        {
        for (int i = 0; i < numInstances; i++)
            {
            PerformanceElement3Ptr element = PerformanceElement3::Create(*m_db, model->GetModelId(), mclassId, catid);
            ASSERT_TRUE(element != nullptr);
            int random = rand();
            Utf8PrintfString str1("Element2 - %d", random);
            element->SetString1(str1.c_str());
            random = rand();
            Utf8PrintfString str2("Element2 - %d", random);
            element->SetString2(str2.c_str());
            random = rand();
            Utf8PrintfString str3("Element3 - %d", random);
            element->SetString3(str3.c_str());
            elements.push_back(element);
            }
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT4_CLASS))
        {
        for (int i = 0; i < numInstances; i++)
            {
            PerformanceElement4Ptr element = PerformanceElement4::Create(*m_db, model->GetModelId(), mclassId, catid);
            ASSERT_TRUE(element != nullptr);
            int random = rand();
            Utf8PrintfString str1("Element2 - %d", random);
            element->SetString1(str1.c_str());
            random = rand();
            Utf8PrintfString str2("Element2 - %d", random);
            element->SetString2(str2.c_str());
            random = rand();
            Utf8PrintfString str3("Element3 - %d", random);
            element->SetString3(str3.c_str());
            random = rand();
            Utf8PrintfString str4("Element4 - %d", random);
            element->SetString4(str4.c_str());
            elements.push_back(element);
            }
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT4b_CLASS))
        {
        for (int i = 0; i < numInstances; i++)
            {
            PerformanceElement4bPtr element = PerformanceElement4b::Create(*m_db, model->GetModelId(), mclassId, catid);
            ASSERT_TRUE(element != nullptr);
            int random = rand();
            Utf8PrintfString str1("Element2 - %d", random);
            element->SetString1(str1.c_str());
            random = rand();
            Utf8PrintfString str2("Element2 - %d", random);
            element->SetString2(str2.c_str());
            random = rand();
            Utf8PrintfString str3("Element3 - %d", random);
            element->SetString3(str3.c_str());
            random = rand();
            Utf8PrintfString str4("Element4b - %d", random);
            element->SetString4b(str4.c_str());
            elements.push_back(element);
            }
        }
    else if (0 == strcmp(className, "PhysicalElement"))
        {
        for (int i = 0; i < numInstances; i++)
            {
            PhysicalElementPtr element = PhysicalElement::Create(PhysicalElement::CreateParams(*m_db, model->GetModelId(), mclassId, catid));
            ASSERT_TRUE(element != nullptr);
            elements.push_back(element);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElementsTestFixture::TimeInsertion(int numInstances, Utf8CP schemaName, Utf8CP className, Utf8String testcaseName, Utf8String testName)
    {
    WString wClassName;
    wClassName.AssignUtf8(className);
    WPrintfString dbName(L"PerformanceElement\\Insert%ls%d.dgndb", wClassName.c_str(), numInstances);
    InitializeProject(dbName.c_str());
    bvector<DgnElementPtr> testElements;
    CreateElements(numInstances, schemaName, className, testElements);

    DgnDbStatus stat = DgnDbStatus::Success;
    StopWatch timer(true);
    for (DgnElementPtr& element : testElements)
        {
        element->Insert(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        }
    timer.Stop();

    LOGTODB(testcaseName, testName, timer.GetElapsedSeconds(), Utf8PrintfString("Inserting %d %s elements", numInstances, className).c_str(), numInstances);
    //m_testObj.writeTodb(insertTime, "PerformanceElementTestFixture.TimeInsertion", Utf8PrintfString("Inserting %d %s elements", numInstances, className).c_str(), numInstances);
    m_db->SaveChanges();
    //m_db->CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElementsTestFixture::TimeUpdate(int numInstances, Utf8CP schemaName, Utf8CP className, Utf8String testcaseName, Utf8String testName)
    {
    WString wClassName;
    wClassName.AssignUtf8(className);
    WPrintfString dbName(L"PerformanceElement\\Update%ls%d.dgndb", wClassName.c_str(), numInstances);
    InitializeProject(dbName.c_str());
    bvector<DgnElementPtr> testElements;
    CreateElements(numInstances, schemaName, className, testElements);

    DgnDbStatus stat = DgnDbStatus::Success;
    for (DgnElementPtr& element : testElements)
        {
        element->Insert(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        }
    printf("Attach profiler"); getchar();
    StopWatch timer(true);
    for (DgnElementPtr& element : testElements)
        {
        element->Update(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        }
    timer.Stop();
    printf("Detach profiler"); getchar();
    LOGTODB(testcaseName, testName, timer.GetElapsedSeconds(), Utf8PrintfString("Updating %d %s elements", numInstances, className).c_str(), numInstances);

    }

TEST_F(PerformanceElementsTestFixture, ElementInsert)
    {
    TimeInsertion(1000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_SIMPLEELEMENT_CLASS, TEST_DETAILS);
    TimeInsertion(10000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_SIMPLEELEMENT_CLASS, TEST_DETAILS);
    TimeInsertion(100000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_SIMPLEELEMENT_CLASS, TEST_DETAILS);
    TimeInsertion(1000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_ELEMENT1_CLASS, TEST_DETAILS);
    TimeInsertion(10000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_ELEMENT1_CLASS, TEST_DETAILS);
    TimeInsertion(100000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_ELEMENT1_CLASS, TEST_DETAILS);
    TimeInsertion(1000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_ELEMENT4b_CLASS, TEST_DETAILS);
    TimeInsertion(10000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_ELEMENT4b_CLASS, TEST_DETAILS);
    TimeInsertion(100000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_ELEMENT4b_CLASS, TEST_DETAILS);
    TimeInsertion(1000, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement, TEST_DETAILS);
    TimeInsertion(10000, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement, TEST_DETAILS);
    TimeInsertion(100000, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement, TEST_DETAILS);
    }

TEST_F(PerformanceElementsTestFixture, ElementUpdate)
    {
    //TimeUpdate(1000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_SIMPLEELEMENT_CLASS, TEST_DETAILS);
    //TimeUpdate(10000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_SIMPLEELEMENT_CLASS, TEST_DETAILS);
    //TimeUpdate(100000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_SIMPLEELEMENT_CLASS, TEST_DETAILS);
    //TimeUpdate(1000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_ELEMENT1_CLASS, TEST_DETAILS);
    //TimeUpdate(10000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_ELEMENT1_CLASS, TEST_DETAILS);
    //TimeUpdate(100000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_ELEMENT1_CLASS, TEST_DETAILS);
    //TimeUpdate(1000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_ELEMENT4b_CLASS, TEST_DETAILS);
    //TimeUpdate(10000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_ELEMENT4b_CLASS, TEST_DETAILS);
    //TimeUpdate(100000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_ELEMENT4b_CLASS, TEST_DETAILS);
    //TimeUpdate(1000, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement, TEST_DETAILS);
    TimeUpdate(10000, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement, TEST_DETAILS);
    //TimeUpdate(100000, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement, TEST_DETAILS);
    }
