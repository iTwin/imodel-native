/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <time.h>
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include <Bentley/DateTime.h>
#include <ECPresentation/DefaultECPresentationSerializer.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include <DgnPlatform/VersionCompareChangeSummary.h>
#include <DgnPlatform/ChangedElementsManager.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include <UnitTests/BackDoor/DgnPlatform/DgnPlatformTestDomain.h>
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/Connection.h>
#include <ECPresentation/ECPresentationManager.h>

USING_NAMESPACE_BENTLEY_DPTEST
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_ECPRESENTATION

//=======================================================================================
// @bsistruct
//=======================================================================================
struct VersionCompareTestFixture : public ::testing::Test
{
public:
    static ScopedDgnHost*       s_host;

    static ECPresentation::ECPresentationManager* m_manager;
    static DgnDbPtr            m_db;
    static PhysicalModelPtr    m_defaultModel;
    static DgnModelId          m_defaultModelId;
    static DgnModelId          m_defaultDrawingModelId;
    static DgnModelId          m_typeModelId;
    static DgnCategoryId       m_defaultCategoryId;
    static DgnCategoryId       m_defaultDrawingCategoryId;
    static Utf8String          m_rulesetDir;

    bool ModifyElementPlacement(DgnElementId elementId);
    bool ModifyElementGeometry(DgnElementId elementId, bool placementChange);
    bool ModifyElementParent(DgnElementId elementId, DgnElementId parentId);
    bool ModifyMultiple(DgnElementId elementId, bool wantGeometricChange);
    bool ModifyIntProp(DgnElementId testElementId, int prop, int value);
    bool ModifyStringProp(DgnElementId elementId, Utf8CP value);
    DgnElementPtr InsertPhysicalElement(Utf8String codeName, TestPhysicalTypePtr type);
    DgnElementPtr InsertPhysicalElement(Utf8String codeName, DPoint3dCR center, DPoint3dCR size, TestPhysicalTypePtr type);
    DgnElementCPtr InsertAnnotationElement();

    TestPhysicalTypePtr CreatePhysicalType(Utf8String name, Utf8String propertyValue);

    static DgnDbPtr CloneTemporaryDb(DgnDbPtr db);
    static DgnRevisionPtr CreateRevision();

    void SetUp() override;
    void TearDown() override;

    static void LoadSchema();

    void DumpRevision(DgnRevisionCR revision, Utf8CP summary);
    void SetUniqueAspectPropertyValue(DgnElementR element, ECN::ECClassCR aspectClass, Utf8CP propertyName, Utf8CP propertyValue);
    void SetUniqueAspectPropertyValueInt(DgnElementR element, ECN::ECClassCR aspectClass, Utf8CP propertyName, int value);
    void SetMultiAspectPropertyValue(DgnElementR element, ECN::ECClassCR aspectClass, ECInstanceId aspectInstanceId, Utf8CP propertyName, Utf8CP propertyValue);

    static void SetUpTestCase();
    static void TearDownTestCase();
};

ScopedDgnHost*                      VersionCompareTestFixture::s_host           = nullptr;
ECPresentationManager*              VersionCompareTestFixture::m_manager        = nullptr;
DgnDbPtr                            VersionCompareTestFixture::m_db             = nullptr;
PhysicalModelPtr                    VersionCompareTestFixture::m_defaultModel   = nullptr;
DgnModelId                          VersionCompareTestFixture::m_defaultModelId;
DgnModelId                          VersionCompareTestFixture::m_defaultDrawingModelId;
DgnModelId                          VersionCompareTestFixture::m_typeModelId;
DgnCategoryId                       VersionCompareTestFixture::m_defaultCategoryId;
DgnCategoryId                       VersionCompareTestFixture::m_defaultDrawingCategoryId;
Utf8String                          VersionCompareTestFixture::m_rulesetDir;

static BeFileName                   s_changesetDir;

//#define DUMP_SUMMARY_OUTPUT
#define DUMP_REVISION
#define LOG (NativeLogging::CategoryLogger("DgnCore"))

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void VersionCompareTestFixture::LoadSchema()
    {
    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext();

    // Import schema for changed elements manager
    BeFileName ecSchemaFilePath(context->GetHostAssetsDirectory());
    ecSchemaFilePath.AppendToPath(L"ECSchemas");
    ecSchemaFilePath.AppendToPath(L"Dgn");
    ecSchemaFilePath.AppendToPath(L"ChangedElements.01.00.00.ecschema.xml");

    ASSERT_TRUE(ecSchemaFilePath.DoesPathExist());
    context->AddSchemaPath(ecSchemaFilePath.GetName());
    context->AddSchemaLocater(m_db->GetSchemaLocater());
    ECSchemaPtr schema = nullptr;
    SchemaReadStatus stat = ECSchema::ReadFromXmlFile(schema, ecSchemaFilePath, *context);
    ASSERT_EQ(SchemaReadStatus::Success, stat);
    ASSERT_TRUE(schema.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void VersionCompareTestFixture::SetUpTestCase()
    {
    s_host = new ScopedDgnHost();

    DgnDomains::RegisterDomain(DgnPlatformTestDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::No);

    BeFileName dgndbPath;
    BeTest::GetHost().GetOutputRoot(dgndbPath);
    dgndbPath.AppendToPath(L"VersionCompareTest.bim");
    if (dgndbPath.DoesPathExist())
        {
        dgndbPath.BeDeleteFile();
        EXPECT_FALSE(dgndbPath.DoesPathExist());
        }

    BeSQLite::DbResult result;
    CreateDgnDbParams createParams ("VersionCompareTest");
    m_db = DgnDb::CreateDgnDb(&result, dgndbPath, createParams);
    ASSERT_TRUE(m_db.IsValid());
    ASSERT_TRUE(result == BeSQLite::DbResult::BE_SQLITE_OK);

    m_db->ResetBriefcaseId(BeBriefcaseId(2));

    // Import DgnPlatformTest schema containing the test element and such
    SchemaStatus schemaStatus = DgnPlatformTestDomain::GetDomain().ImportSchema(*m_db);
    ASSERT_TRUE(schemaStatus == SchemaStatus::Success);

    m_defaultModel = DgnDbTestUtils::InsertPhysicalModel(*m_db, "VersionCompareTestModel");
    m_defaultModelId = m_defaultModel->GetModelId();
    DocumentListModelPtr drawingListModel = DgnDbTestUtils::InsertDocumentListModel(*m_db, "MyDrawingListModel");
    DrawingPtr drawing = DgnDbTestUtils::InsertDrawing(*drawingListModel, "MyDrawing");
    DrawingModelPtr drawingModel = DgnDbTestUtils::InsertDrawingModel(*drawing);
    m_defaultDrawingModelId = drawingModel->GetModelId();
    m_defaultCategoryId = DgnDbTestUtils::InsertSpatialCategory(*m_db, "VersionCompareTestCategory");
    m_defaultDrawingCategoryId = DgnDbTestUtils::InsertDrawingCategory(*m_db, "MyDrawingCategory");

    ASSERT_TRUE(m_defaultModelId.IsValid());
    ASSERT_TRUE(m_defaultModel.IsValid());
    ASSERT_TRUE(m_defaultCategoryId.IsValid());

    m_db->SaveChanges();

    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());

    BeTest::GetHost().GetTempDir(s_changesetDir);
    s_changesetDir.AppendToPath(L"DgnDbChangeSets");
    s_changesetDir.AppendSeparator();

    LoadSchema();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void VersionCompareTestFixture::DumpRevision(DgnRevisionCR revision, Utf8CP summary)
    {
#ifdef DUMP_REVISION
    LOG.infov("---------------------------------------------------------");
    if (summary != nullptr)
        LOG.infov(summary);
    revision.Dump(*m_db);
    LOG.infov("---------------------------------------------------------");
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void VersionCompareTestFixture::SetUp()
    {
    // Setup required BeSQLite initialization for ECPresentation
    BeFileName rulesetsDir;
    BeFileName tempDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(rulesetsDir);
    BeTest::GetHost().GetTempDir(tempDir);
    BeSQLiteLib::Initialize(tempDir);

    // Initialize ECPresentationManager and connection
    ECPresentationManager::SetSerializer(new ECPresentation::DefaultECPresentationSerializer());
    ECPresentationManager::Params params(ECPresentationManager::Paths(rulesetsDir, tempDir));
    m_manager = new ECPresentationManager(params);
    m_rulesetDir = rulesetsDir.GetNameUtf8().c_str();

    // Add presentation rules
    rulesetsDir.AppendToPath(L"PresentationRules");
    RuleSetLocaterPtr locater = DirectoryRuleSetLocater::Create(rulesetsDir.GetNameUtf8().c_str());
    m_manager->GetLocaters().RegisterLocater(*locater);

#define CREATE_DGNDBCHANGESETSDIR
#ifdef CREATE_DGNDBCHANGESETSDIR
    if (!s_changesetDir.DoesPathExist())
        {
        BeFileNameStatus status = BeFileName::CreateNewDirectory(s_changesetDir.GetName());
        ASSERT_TRUE(status == BeFileNameStatus::Success);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void VersionCompareTestFixture::TearDownTestCase()
    {
    m_db->AbandonChanges();
    m_db = nullptr;
    m_defaultModel = nullptr;
    delete s_host;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void VersionCompareTestFixture::TearDown()
    {
    delete m_manager;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool VersionCompareTestFixture::ModifyElementPlacement(DgnElementId elementId)
    {
    RefCountedPtr<PhysicalElement> testElement = m_db->Elements().GetForEdit<PhysicalElement>(elementId);
    if (!testElement.IsValid())
        return false;

    Placement3d newPlacement = testElement->GetPlacement();
    newPlacement.GetOriginR().x += 1.0;

    testElement->SetPlacement(newPlacement);

    DgnDbStatus dbStatus = testElement->Update();
    return (dbStatus == DgnDbStatus::Success);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool VersionCompareTestFixture::ModifyElementGeometry(DgnElementId elementId, bool placementChange)
    {
    RefCountedPtr<PhysicalElement> testElement = m_db->Elements().GetForEdit<PhysicalElement>(elementId);
    if (!testElement.IsValid())
        return false;

    // Either make a box that can be contained inside the current range or not based on the boolean
    DgnBoxDetail blockDetail = DgnBoxDetail::InitFromCenterAndSize(DPoint3d::FromZero(), !placementChange ? DPoint3d::FromXYZ(0.1, 0.1, 0.1) : DPoint3d::FromXYZ(5.0, 5.0, 5.0), true);
    ISolidPrimitivePtr geomPtr = ISolidPrimitive::CreateDgnBox(blockDetail);
    BeAssert(geomPtr.IsValid());

    auto gs = testElement->GetAsGeometrySource3dP()->GetGeometryStream();
    GeometryBuilderPtr builder = GeometryBuilder::Create(*testElement, gs);
    builder->Append(*geomPtr);
    BentleyStatus status = builder->Finish(*testElement);
    BeAssert(status == SUCCESS);

    DgnDbStatus dbStatus = testElement->Update();
    return (dbStatus == DgnDbStatus::Success);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool VersionCompareTestFixture::ModifyElementParent(DgnElementId elementId, DgnElementId parentId)
    {
    RefCountedPtr<TestElement> testElement = m_db->Elements().GetForEdit<TestElement>(elementId);
    if (!testElement.IsValid())
        return false;

    testElement->SetParentId(parentId, m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements));
    DgnDbStatus dbStatus = testElement->Update();
    BeAssert(dbStatus == DgnDbStatus::Success);
    return (dbStatus == DgnDbStatus::Success);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool VersionCompareTestFixture::ModifyMultiple(DgnElementId elementId, bool wantGeometricChange = true)
    {
    RefCountedPtr<TestElement> testElement = m_db->Elements().GetForEdit<TestElement>(elementId);
    if (!testElement.IsValid())
        return false;

    if (wantGeometricChange)
        {
        // Change geometry and placement
        DgnBoxDetail blockDetail = DgnBoxDetail::InitFromCenterAndSize(DPoint3d::FromZero(), DPoint3d::FromXYZ(5.0, 5.0, 5.0), true);
        ISolidPrimitivePtr geomPtr = ISolidPrimitive::CreateDgnBox(blockDetail);
        BeAssert(geomPtr.IsValid());

        auto gs = testElement->GetAsGeometrySource3dP()->GetGeometryStream();
        GeometryBuilderPtr builder = GeometryBuilder::Create(*testElement, gs);
        builder->Append(*geomPtr);
        BentleyStatus status = builder->Finish(*testElement);
        BeAssert(status == SUCCESS);
        }

    // Change integer property
    testElement->SetIntegerProperty(0, 5);
    // Change hidden property
    testElement->SetPropertyValue("JsonProperties", "{}");

    DgnDbStatus dbStatus = testElement->Update();
    return (dbStatus == DgnDbStatus::Success);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool VersionCompareTestFixture::ModifyIntProp(DgnElementId elementId, int prop, int value)
    {
    RefCountedPtr<TestElement> testElement = m_db->Elements().GetForEdit<TestElement>(elementId);
    if (!testElement.IsValid())
        return false;

    // Change integer property
    testElement->SetIntegerProperty(prop, value);

    DgnDbStatus dbStatus = testElement->Update();
    return (dbStatus == DgnDbStatus::Success);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool VersionCompareTestFixture::ModifyStringProp(DgnElementId elementId, Utf8CP value)
    {
    RefCountedPtr<TestElement> testElement = m_db->Elements().GetForEdit<TestElement>(elementId);
    if (!testElement.IsValid())
        return false;

    // Change integer property
    testElement->SetPropertyValue("TestElementProperty", value);

    DgnDbStatus dbStatus = testElement->Update();
    return (dbStatus == DgnDbStatus::Success);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr   VersionCompareTestFixture::InsertPhysicalElement(Utf8String codeName, DPoint3dCR center, DPoint3dCR size, TestPhysicalTypePtr type = nullptr)
    {
    PhysicalModelR model = *m_defaultModel->ToPhysicalModelP();
    GenericPhysicalObjectPtr physicalElementPtr = GenericPhysicalObject::Create(model, m_defaultCategoryId);
    auto codeSpec = CodeSpec::Create(*m_db, "MyCodeSpec");
    physicalElementPtr->SetCode(codeSpec->CreateCode(codeName));

    // Set physical type if provided
    if (type != nullptr)
        {
        physicalElementPtr->SetTypeDefinition(type->GetElementId(), m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_PhysicalElementIsOfType));
        BeAssert(physicalElementPtr->GetTypeDefinitionId().IsValid());
        BeAssert(physicalElementPtr->GetPhysicalType().IsValid());
        }

    DgnBoxDetail blockDetail = DgnBoxDetail::InitFromCenterAndSize(DPoint3d::FromZero(), size, true);
    ISolidPrimitivePtr geomPtr = ISolidPrimitive::CreateDgnBox(blockDetail);
    BeAssert(geomPtr.IsValid());

    GeometryBuilderPtr builder = GeometryBuilder::Create(*m_defaultModel, m_defaultCategoryId, center, YawPitchRollAngles());
    builder->Append(*geomPtr);
    BentleyStatus status = builder->Finish(*physicalElementPtr);
    BeAssert(status == SUCCESS);

    m_db->Elements().Insert(*physicalElementPtr);
    return physicalElementPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TestPhysicalTypePtr VersionCompareTestFixture::CreatePhysicalType(Utf8String name, Utf8String propertyValue)
    {
    DefinitionModelPtr typeModel;
    if (!m_typeModelId.IsValid())
        {
        typeModel = DgnDbTestUtils::InsertDefinitionModel(*m_db, "PhysicalTypes");
        m_typeModelId = typeModel->GetModelId();
        }
    else
        {
        typeModel = m_db->Models().Get<DefinitionModel>(m_typeModelId);
        }

    TestPhysicalTypePtr physicalType = TestPhysicalType::Create(*typeModel, name.c_str());
    physicalType->SetStringProperty(propertyValue.c_str());
    m_db->Elements().Insert(*physicalType);
    return physicalType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr   VersionCompareTestFixture::InsertAnnotationElement()
    {
    DgnDbR db = *m_db;
    DgnModelId modelId = m_defaultDrawingModelId;
    if(!modelId.IsValid())
        return nullptr;

    DgnModelPtr modelP = db.Models().GetModel(modelId);
    if(!modelP.IsValid())
        return nullptr;

    DgnCategoryId categoryId = DgnDbTestUtils::GetFirstDrawingCategoryId(db);
    if(!categoryId.IsValid())
        return nullptr;

    DgnClassId elementClassId = DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_AnnotationElement2d));
    if(!elementClassId.IsValid())
        return nullptr;
    AnnotationElement2dPtr elementPtr = AnnotationElement2d::Create(AnnotationElement2d::CreateParams(db, modelId, elementClassId, categoryId));
    if(!elementPtr.IsValid())
        return nullptr;
    return db.Elements().Insert(*elementPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr   VersionCompareTestFixture::InsertPhysicalElement(Utf8String codeName, TestPhysicalTypePtr type = nullptr)
    {
    DPoint3d blockSizeRange = DPoint3d::From(2.0, 2.0, 2.0);
    DPoint3d center = DPoint3d::FromZero();
    return VersionCompareTestFixture::InsertPhysicalElement(codeName, center, blockSizeRange, type);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr    VersionCompareTestFixture::CloneTemporaryDb(DgnDbPtr db)
    {
    WString name = WString(L"Temp_") + db->GetFileName().GetFileNameWithoutExtension();
    BeFileName tempFilename = db->GetFileName().GetDirectoryName();
    tempFilename.AppendToPath(name.c_str());
    tempFilename.AppendExtension(L"bim");

    // Try to create temporary file by copying base bim file
    BeFileNameStatus fileStatus = BeFileName::BeCopyFile(db->GetFileName(), tempFilename);

    if (BeFileNameStatus::Success != fileStatus)
        return nullptr;

    // If the file was copied before, act on it instead of re-copying each time we compare revisions
    // Open the target db using the temporary filename
    BeSQLite::DbResult result;
    DgnDb::OpenParams params (Db::OpenMode::ReadWrite);
    DgnDbPtr clonedDb = DgnDb::OpenDgnDb(&result, tempFilename, params);
    return clonedDb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnRevisionPtr VersionCompareTestFixture::CreateRevision()
    {
    m_db->SaveChanges();

    DgnRevisionPtr revision = m_db->Revisions().StartCreateRevision();
    EXPECT_TRUE(revision.IsValid());
    if (!revision.IsValid())
        return nullptr;

    RevisionStatus status = m_db->Revisions().FinishCreateRevision(-1);
    EXPECT_TRUE(status == RevisionStatus::Success);
    if (RevisionStatus::Success != status)
        return nullptr;

    return revision;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void VersionCompareTestFixture::SetUniqueAspectPropertyValue(DgnElementR element, ECN::ECClassCR aspectClass, Utf8CP propertyName, Utf8CP propertyValue)
    {
    DgnElement::UniqueAspect* aspect = DgnElement::UniqueAspect::GetAspectP(element, aspectClass);
    ASSERT_NE(aspect, nullptr);
    ASSERT_EQ(DgnDbStatus::Success, aspect->SetPropertyValue(propertyName, ECN::ECValue(propertyValue)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void VersionCompareTestFixture::SetUniqueAspectPropertyValueInt(DgnElementR element, ECN::ECClassCR aspectClass, Utf8CP propertyName, int propertyValue)
    {
    DgnElement::UniqueAspect* aspect = DgnElement::UniqueAspect::GetAspectP(element, aspectClass);
    ASSERT_NE(aspect, nullptr);
    ASSERT_EQ(DgnDbStatus::Success, aspect->SetPropertyValue(propertyName, ECN::ECValue(propertyValue)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void VersionCompareTestFixture::SetMultiAspectPropertyValue(DgnElementR element, ECN::ECClassCR aspectClass, ECInstanceId aspectInstanceId, Utf8CP propertyName, Utf8CP propertyValue)
    {
    DgnElement::MultiAspect* aspect = DgnElement::MultiAspect::GetAspectP(element, aspectClass, aspectInstanceId);
    ASSERT_NE(aspect, nullptr);
    ASSERT_EQ(DgnDbStatus::Success, aspect->SetPropertyValue(propertyName, ECN::ECValue(propertyValue)));
    }

//=======================================================================================
// Helper to check the output of version compare change summaries
// @bsistruct
//=======================================================================================
struct ElementData
    {
    DbOpcode                m_opcode;
    ECClassId               m_classId;
    DgnModelId              m_modelId;
    ECInstanceKey           m_parentKey;
    AxisAlignedBox3d        m_bbox;
    ElementChangesType      m_changes;

    ElementData(DgnElementPtr element, DbOpcode opcode) : m_opcode(opcode)
        {
        m_classId = element->GetElementClass()->GetId();
        m_modelId = element->GetModelId();
        GeometrySource3dCP source = element->ToGeometrySource3d();
        m_bbox = nullptr != source ? source->CalculateRange3d() : AxisAlignedBox3d();
        }

    ElementData(DgnElementPtr element, DbOpcode opcode, ECInstanceKey parentKey) : m_opcode(opcode), m_parentKey(parentKey)
        {
        m_classId = element->GetElementClass()->GetId();
        m_modelId = element->GetModelId();
        GeometrySource3dCP source = element->ToGeometrySource3d();
        m_bbox = nullptr != source ? source->CalculateRange3d() : AxisAlignedBox3d();
        }

    ElementData(DgnElementPtr element, DbOpcode opcode, int changes) : m_opcode(opcode)
        {
        m_classId = element->GetElementClass()->GetId();
        m_modelId = element->GetModelId();
        GeometrySource3dCP source = element->ToGeometrySource3d();
        m_bbox = nullptr != source ? source->CalculateRange3d() : AxisAlignedBox3d();
        m_changes = ElementChangesType();
        m_changes.m_flags = changes;
        }

    ElementData(DgnElementPtr element, DbOpcode opcode, ECInstanceKey parentKey, int changes) : m_opcode(opcode), m_parentKey(parentKey)
        {
        m_classId = element->GetElementClass()->GetId();
        m_modelId = element->GetModelId();
        GeometrySource3dCP source = element->ToGeometrySource3d();
        m_bbox = nullptr != source ? source->CalculateRange3d() : AxisAlignedBox3d();
        m_changes = ElementChangesType();
        m_changes.m_flags = changes;
        }

    ElementData(DgnElementPtr element, DbOpcode opcode, ECInstanceKey parentKey, int changes, bvector<Utf8String> properties) : m_opcode(opcode), m_parentKey(parentKey)
        {
        m_classId = element->GetElementClass()->GetId();
        m_modelId = element->GetModelId();
        GeometrySource3dCP source = element->ToGeometrySource3d();
        m_bbox = nullptr != source ? source->CalculateRange3d() : AxisAlignedBox3d();
        m_changes = ElementChangesType();
        m_changes.m_flags = changes;
        // Make copies
        for (auto prop : properties)
            m_changes.m_properties.insert(prop);
        }

    ElementData(DgnElementPtr element, DbOpcode opcode, int changes, bvector<Utf8String> properties) : m_opcode(opcode)
        {
        m_classId = element->GetElementClass()->GetId();
        m_modelId = element->GetModelId();
        GeometrySource3dCP source = element->ToGeometrySource3d();
        m_bbox = nullptr != source ? source->CalculateRange3d() : AxisAlignedBox3d();
        m_changes = ElementChangesType();
        m_changes.m_flags = changes;
        // Make copies
        for (auto prop : properties)
            m_changes.m_properties.insert(prop);
        }

    ElementData(DgnElementCPtr element, DbOpcode opcode, int changes, bvector<Utf8String> properties) : m_opcode(opcode)
        {
        m_classId = element->GetElementClass()->GetId();
        m_modelId = element->GetModelId();
        GeometrySource3dCP source = element->ToGeometrySource3d();
        m_bbox = nullptr != source ? source->CalculateRange3d() : AxisAlignedBox3d();
        m_changes = ElementChangesType();
        m_changes.m_flags = changes;
        // Make copies
        for (auto prop : properties)
            m_changes.m_properties.insert(prop);
        }

    ElementData(ECClassId classId, DbOpcode opcode, DgnModelId modelId, AxisAlignedBox3d bbox) : m_opcode(opcode), m_classId(classId), m_modelId(modelId), m_bbox(bbox) { }
    ElementData(ECClassId classId, DbOpcode opcode, DgnModelId modelId, AxisAlignedBox3d bbox, ElementChangesType changes) : m_opcode(opcode), m_classId(classId), m_modelId(modelId), m_bbox(bbox), m_changes(changes) { }
    ElementData() : m_opcode((DbOpcode)0) { m_classId.Invalidate(); }

    bool IsValid()
        {
        return m_classId.IsValid() && m_opcode != (DbOpcode)0;
        }
    }; // ElementData

typedef bmap<DgnElementId, ElementData> ElementMap;


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PrintFoundRecord(DgnDbR db, DgnElementId elementId, ECClassId ecclassId, DbOpcode opcode)
    {
#ifdef DUMP_SUMMARY_OUTPUT
    Utf8String opcodeStr = "";
    switch (opcode)
        {
        case DbOpcode::Insert:
            opcodeStr = "Insert";
            break;
        case DbOpcode::Delete:
            opcodeStr = "Delete";
            break;
        case DbOpcode::Update:
            opcodeStr = "Update";
            break;
        }

    Utf8String ecclassName = db.Schemas().GetClass(ecclassId)->GetFullName();
    printf("ID: %s \tECClass: %s \tOpcode: %s\n", elementId.ToHexStr().c_str(), ecclassName.c_str(), opcodeStr.c_str());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CheckOutput(DgnDbR db, ElementMap & map, bvector<ChangedElement> const& elements)
    {
    EXPECT_EQ(elements.size(), map.size());

    ECClassId bisElementClassId = db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_Element);

    // Checks the results of comparison against a test-made map
    for (ChangedElement const& element : elements)
        {
        DgnElementId currentId      = element.m_elementId;
        ECClassId currentECClassId  = element.m_classId;
        DbOpcode currentOpcode      = element.m_opcode;
        DgnModelId modelId          = element.m_modelId;
        AxisAlignedBox3d boundBox   = element.m_bbox;
        ElementChangesType change   = element.m_changes;

        PrintFoundRecord(db, currentId, currentECClassId, currentOpcode);

        EXPECT_TRUE(map.find(currentId) != map.end());
        if (map.find(currentId) == map.end())
            continue;

        // Ensure all elements have valid model Ids
        EXPECT_TRUE(modelId.IsValid());

        ElementData & currentElemData = map[currentId];
        EXPECT_EQ(currentElemData.IsValid(), true);
        if (!currentElemData.IsValid())
            continue;

        EXPECT_EQ(currentElemData.m_opcode, currentOpcode);
        EXPECT_EQ(currentElemData.m_classId, currentECClassId);

        // Parent key object should match
        EXPECT_EQ(currentElemData.m_parentKey.IsValid(), element.m_parentKey.IsValid());
        if (currentElemData.m_parentKey.IsValid() && element.m_parentKey.IsValid())
            {
            EXPECT_EQ(element.m_parentKey.GetInstanceId().GetValue(), currentElemData.m_parentKey.GetInstanceId().GetValue());
            // We should either have the proper class Id or default to Bis Element
            EXPECT_TRUE(element.m_parentKey.GetClassId().GetValue() == currentElemData.m_parentKey.GetClassId().GetValue() || element.m_parentKey.GetClassId().GetValue() == bisElementClassId.GetValue());
            }

        if (currentOpcode == DbOpcode::Update)
            {
            EXPECT_EQ(change.m_flags, currentElemData.m_changes.m_flags);
            // Check properties are contained if given
            EXPECT_EQ(change.m_properties.size(), currentElemData.m_changes.m_properties.size());
            if (change.m_properties.size() != 0)
                {
                for (auto const& prop : currentElemData.m_changes.m_properties)
                    {
                    EXPECT_TRUE(change.m_properties.find(prop) != change.m_properties.end());
                    }
                }
            }
        else
            {
            // Ensure no properties are maintained on Inserts or Deletes
            EXPECT_TRUE(change.m_properties.size() == 0);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CreateSummaryAndCheckOutput(DgnDbPtr db, ElementMap& map, bvector<DgnRevisionPtr>& changesets, ECPresentationManagerR presentationManager, Utf8String debugLabel = "", bool filterSpatial = false, bool filterLastMod = false, bool wantParents = false, bool wantTargetState = false, bool wantBriefcaseRoll = false, bool checkOutput = true)
    {
    bvector<ChangedElement> elements;
    StatusInt status = SUCCESS;

    BeFileName fileName = db->GetFileName();
    SummaryOptions options;
    options.presentationManager = &presentationManager;
    options.filterSpatial = filterSpatial;
    options.filterLastMod = filterLastMod;
    options.tempLocation = BeFileName();
    options.rulesetId = "Items";
    options.wantTargetState = wantTargetState;
    options.wantParents = wantParents;
    options.wantBriefcaseRoll = wantBriefcaseRoll;
    options.wantChunkTraversal = false;

    // If rolling briefcase, we must close the Db before processing
    if (wantBriefcaseRoll)
        db->CloseDb();

    clock_t t0 = clock();
    VersionCompareChangeSummaryPtr changeSummary = VersionCompareChangeSummary::Generate (fileName, changesets, options);
    clock_t t1 = clock();
    double elapsed = (t1-t0)/(double)CLOCKS_PER_SEC;
    printf("VC Relationship Caching - Generate: %lf seconds\n", elapsed);

    status = changeSummary->GetChangedElements(elements);
    EXPECT_EQ(SUCCESS, status);

    // Destruct change summary
    changeSummary = nullptr;

    // Compare desired output with actual output
    if (checkOutput)
        CheckOutput(*db, map, elements);

    // Test without relationship caching
    elements.clear();
    options.wantRelationshipCaching = false;
    t0 = clock();
    VersionCompareChangeSummaryPtr changeSummaryWithoutCaching = VersionCompareChangeSummary::Generate (fileName, changesets, options);
    t1 = clock();
    elapsed = (t1-t0)/(double)CLOCKS_PER_SEC;
    printf("VC No Relationship Caching - Generate: %lf seconds\n", elapsed);

    status = changeSummaryWithoutCaching->GetChangedElements(elements);
    EXPECT_EQ(SUCCESS, status);

    // Destruct change summary
    changeSummaryWithoutCaching = nullptr;

    // Compare desired output with actual output
    if (checkOutput)
        CheckOutput(*db, map, elements);

    // Test with chunk traversal
    elements.clear();
    options.wantChunkTraversal = true;
    t0 = clock();
    VersionCompareChangeSummaryPtr changeSummaryWithChunk = VersionCompareChangeSummary::Generate (fileName, changesets, options);
    t1 = clock();
    elapsed = (t1-t0)/(double)CLOCKS_PER_SEC;
    printf("VC Chunk Traversal - Generate: %lf seconds\n", elapsed);

    status = changeSummaryWithChunk->GetChangedElements(elements);
    EXPECT_EQ(SUCCESS, status);

    // Destruct change summary
    changeSummaryWithChunk = nullptr;

    // Compare desired output with actual output
    if (checkOutput)
        CheckOutput(*db, map, elements);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionCompareTestFixture, CompareOneRevisionOneInsertion)
    {
    // Test one insertion on one changeset
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    // Clone with the current revision
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // CHANGESET 1
    // Insert an element
    DgnElementPtr firstElement = InsertPhysicalElement("X");
    elementMap[firstElement->GetElementId()] = ElementData(firstElement, DbOpcode::Insert);
    changesets.push_back(CreateRevision());

    // Test that the output matches with the input rolling forward
    // Single changeset must be processed with up-to-date Db
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionCompareTestFixture, CompareOneRevisionOneDeletion)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    DgnElementPtr firstElement = InsertPhysicalElement("X");
    // Create first revision containing an element
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    // Clone with the current revision
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // CHANGESET 1
    // Delete the first element
    elementMap[firstElement->GetElementId()] = ElementData(firstElement, DbOpcode::Delete);
    m_db->Elements().Delete(firstElement->GetElementId());
    changesets.push_back(CreateRevision());

    // Test that the output matches with the input rolling forward
    // Single changeset must be processed with up-to-date Db
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionCompareTestFixture, CompareOneRevisionOneUpdate)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    DgnElementPtr firstElement = InsertPhysicalElement("X");
    // Create first revision containing an element
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    // Clone with the current revision
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // CHANGESET 1
    // Modify the first element
    elementMap[firstElement->GetElementId()] = ElementData(firstElement, DbOpcode::Update, ElementChangesType::Type::Mask_Placement);
    ModifyElementPlacement(firstElement->GetElementId());
    changesets.push_back(CreateRevision());

    // Test that the output matches with the input rolling forward
    // Single changeset must be processed with up-to-date Db
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionCompareTestFixture, CompareTenVersionsTwentyInserts)
    {
    // Test one insertion on one changeset
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // Create 10 changesets, each with 2 insertions
    for (int i = 0; i < 10; ++i)
        {
        Utf8PrintfString codeName1("%d", i * 2);
        Utf8PrintfString codeName2("%d", i * 2 + 1);
        DgnElementPtr element = InsertPhysicalElement(codeName1);
        DgnElementPtr element2 = InsertPhysicalElement(codeName2);
        elementMap[element->GetElementId()] = ElementData(element, DbOpcode::Insert);
        elementMap[element2->GetElementId()] = ElementData(element2, DbOpcode::Insert);
        changesets.push_back(CreateRevision());
        }

    // Test that the output matches with the input rolling forward
    CreateSummaryAndCheckOutput(targetDb, elementMap, changesets, *m_manager);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionCompareTestFixture, TestFilterSpatial)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // CHANGESET 1
    // Insert a 3D element
    TestElementPtr tempEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement");
    m_db->Elements().Insert(*tempEl);
    // Insert a 2D element
    TestElement2dPtr tempEl2d = TestElement2d::Create(*m_db, m_defaultDrawingModelId, m_defaultDrawingCategoryId, DgnCode::CreateEmpty(), 3.0);
    m_db->Elements().Insert(*tempEl2d);
    // Only insert the 3D element since the 2D element should be filtered out
    elementMap[tempEl->GetElementId()] = ElementData(tempEl, DbOpcode::Insert);
    changesets.push_back(CreateRevision());

    // Test filtering out the 2D element first
    // Test that the output matches with the input rolling forward
    // Single changeset must be processed with up-to-date Db
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager, "", true);

    // Test without filtering out the 2D element now
    elementMap[tempEl2d->GetElementId()] = ElementData(tempEl2d, DbOpcode::Insert);
    // Test that the output matches with the input rolling forward
    // Single changeset must be processed with up-to-date Db
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager, "", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionCompareTestFixture, CompareSchemaChangesAccumulation1)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    // Clone with the current revision
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // CHANGESET 1
    // Insert an element in the same changeset we will have schema changes
    DgnElementPtr firstElement = InsertPhysicalElement("X");
    elementMap[firstElement->GetElementId()] = ElementData(firstElement, DbOpcode::Insert);
    // Schema changes
    m_db->CreateTable("TestTable1", "Id INTEGER PRIMARY KEY, Column1 INTEGER");
    m_db->CreateTable("TestTable2", "Id INTEGER PRIMARY KEY, Column1 INTEGER");
    // Create a revision with the schema changes
    changesets.push_back(CreateRevision());

    // CHANGESET 2
    // Insert another element
    DgnElementPtr secondElement = InsertPhysicalElement("Y");
    elementMap[secondElement->GetElementId()] = ElementData(secondElement, DbOpcode::Insert);
    // Create a revision with a new element without schema changes
    changesets.push_back(CreateRevision());

    // Test that the output matches with the input rolling forward
    CreateSummaryAndCheckOutput(targetDb, elementMap, changesets, *m_manager);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionCompareTestFixture, CompareSchemaChangesAccumulation2)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    // Clone with the current revision
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // CHANGESET 1
    //  Insert an element in the same changeset we will have schema changes
    //  Don't add it to the element map because we will delete the same element
    //  in the next changeset to make sure we are accumulating changes correctly
    //  on schema changes
    DgnElementPtr firstElement = InsertPhysicalElement("X");
    DgnElementPtr secondElement = InsertPhysicalElement("Y");
    elementMap[secondElement->GetElementId()] = ElementData(secondElement, DbOpcode::Insert);
    // Schema changes
    m_db->CreateTable("TestTable3", "Id INTEGER PRIMARY KEY, Column1 INTEGER");
    m_db->CreateTable("TestTable4", "Id INTEGER PRIMARY KEY, Column1 INTEGER");
    // Create a revision with the schema changes
    changesets.push_back(CreateRevision());

    // CHANGESET 2
    // Delete the element
    m_db->Elements().Delete(firstElement->GetElementId());
    changesets.push_back(CreateRevision());

    // Test that the output matches with the input rolling forward
    CreateSummaryAndCheckOutput(targetDb, elementMap, changesets, *m_manager);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionCompareTestFixture, CompareSchemaChangesAccumulation3)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    DgnElementPtr zeroElement = InsertPhysicalElement("Zero");
    DgnElementPtr firstElement = InsertPhysicalElement("X");
    DgnElementPtr secondElement = InsertPhysicalElement("Y");
    DgnElementPtr thirdElement = InsertPhysicalElement("Z");

    // Create the starting changeset and clone the db
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // CHANGESET 1 (Schema Change)
    // Modify the first element's geometry and add schema change
    ModifyElementGeometry(firstElement->GetElementId(), false);
    elementMap[firstElement->GetElementId()] = ElementData(firstElement, DbOpcode::Update, ElementChangesType::Type::Mask_Geometry);
    m_db->CreateTable("TestTable5", "Id INTEGER PRIMARY KEY, Column1 INTEGER");
    changesets.push_back(CreateRevision());

    // CHANGESET 2
    // Delete second element
    elementMap[secondElement->GetElementId()] = ElementData(secondElement, DbOpcode::Delete);
    m_db->Elements().Delete(secondElement->GetElementId());
    changesets.push_back(CreateRevision());

    // CHANGESET 3
    // Insert a new element and modify third element's geometry and placement
    DgnElementPtr fourthElement = InsertPhysicalElement("A");
    elementMap[fourthElement->GetElementId()] = ElementData(fourthElement, DbOpcode::Insert);
    ModifyElementGeometry(thirdElement->GetElementId(), true);
    elementMap[thirdElement->GetElementId()] = ElementData(fourthElement, DbOpcode::Update, ElementChangesType::Type::Mask_Geometry | ElementChangesType::Type::Mask_Placement);
    changesets.push_back(CreateRevision());

    // CHANGESET 4
    // Modify fourth element (should still show up as an inserted element, so don't do anything to the element map)
    ModifyElementPlacement(fourthElement->GetElementId());
    changesets.push_back(CreateRevision());

    // CHANGESET 5 (Schema Change)
    // Insert two elements and add a schema change
    DgnElementPtr fifthElement = InsertPhysicalElement("B");
    DgnElementPtr sixthElement = InsertPhysicalElement("C");
    elementMap[fifthElement->GetElementId()] = ElementData(fifthElement, DbOpcode::Insert);
    elementMap[sixthElement->GetElementId()] = ElementData(sixthElement, DbOpcode::Insert);
    m_db->CreateTable("TestTable6", "Id INTEGER PRIMARY KEY, Column1 INTEGER");
    changesets.push_back(CreateRevision());

    // CHANGESET 6
    // Delete zeroElement
    elementMap[zeroElement->GetElementId()] = ElementData(zeroElement, DbOpcode::Delete);
    m_db->Elements().Delete(zeroElement->GetElementId());
    changesets.push_back(CreateRevision());

    // Test that the output matches with the input rolling forward
    CreateSummaryAndCheckOutput(targetDb, elementMap, changesets, *m_manager);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionCompareTestFixture, CompareAspectChange1)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // CHANGESET 1
    // Insert an element with an aspect
    TestElementPtr tempEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement");
    DgnElement::UniqueAspect::SetAspect(*tempEl, *TestUniqueAspect::Create("Initial Value"));
    m_db->Elements().Insert(*tempEl);
    // We should only end up with an insertion of the element, not the aspect
    elementMap[tempEl->GetElementId()] = ElementData(tempEl, DbOpcode::Insert);
    changesets.push_back(CreateRevision());

    // Test that the output matches with the input rolling forward
    // Single changeset must be processed with up-to-date Db
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionCompareTestFixture, CompareAspectChange2)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    // INITIAL CHANGESET
    // Create the starting changeset containing an element with an aspect
    TestElementPtr tempEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement");
    TestUniqueAspectPtr aspect = TestUniqueAspect::Create("Initial Value");
    DgnElement::UniqueAspect::SetAspect(*tempEl, *aspect);
    aspect->SetPropertyValue("TestUniqueAspectProperty", ECValue("Old Value for Property"));
    m_db->Elements().Insert(*tempEl);
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // CHANGESET 1
    // Modify the aspect, we should get a modification of the element
    ECClassCP aspectClassUnique = TestUniqueAspect::GetECClass(*m_db);
    ASSERT_NE(aspectClassUnique, nullptr);
    SetUniqueAspectPropertyValue(*tempEl, *aspectClassUnique, "TestUniqueAspectProperty", "New Value for Property");
    tempEl->Update();
    bvector<Utf8String> props;
    props.push_back("TestUniqueAspectProperty");
    elementMap[tempEl->GetElementId()] = ElementData(tempEl, DbOpcode::Update, ElementChangesType::Type::Mask_Indirect | ElementChangesType::Type::Mask_Property, props);
    DgnRevisionPtr changeset1 = CreateRevision();
    DumpRevision(*changeset1, "CompareAspectChange2: Aspect modification");
    changesets.push_back(changeset1);

    // Test that the output matches with the input rolling forward
    // Single changeset must be processed with up-to-date Db
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionCompareTestFixture, CompareMultiAspectChange1)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // CHANGESET 1
    // Insert an element with an aspect
    TestElementPtr tempEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement");
    TestMultiAspectPtr a1 = TestMultiAspect::Create("1");
    TestMultiAspectPtr a2 = TestMultiAspect::Create("2");
    DgnElement::MultiAspect::AddAspect(*tempEl, *a1);
    DgnElement::MultiAspect::AddAspect(*tempEl, *a2);
    m_db->Elements().Insert(*tempEl);
    // We should only end up with an insertion of the element, not the aspect
    elementMap[tempEl->GetElementId()] = ElementData(tempEl, DbOpcode::Insert);
    // Create changeset and dump
    DgnRevisionPtr changeset1 = CreateRevision();
    DumpRevision(*changeset1, "CompareMultiAspectChange1: Aspect insertion");
    changesets.push_back(changeset1);

    // Test that the output matches with the input rolling forward
    // Single changeset must be processed with up-to-date Db
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager);
    }

//--------------------------------------------------------------------------------------
// @bsistruct
//--------------------------------------------------------------------------------------
struct PropertyData
{
    Utf8String      m_accessor;
    Utf8String      m_accessorPrefix;
    ECValue         m_oldValue;
    ECValue         m_newValue;

    PropertyData(Utf8String accessor, Utf8String prefix, ECValue oldVal, ECValue newVal)
    : m_accessor(accessor), m_accessorPrefix(prefix), m_oldValue(oldVal), m_newValue(newVal) { }
}; // PropertyData

typedef bmap<DgnElementId, bvector<PropertyData>> ElementInputPropertyMap;
typedef bmap<DgnElementId, Json::Value> ElementOutputPropertyMap;

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AddEntry(ElementInputPropertyMap& map, DgnElementId elementId, Utf8String accessor, ECValue oldVal, ECValue newVal, Utf8String prefix = "")
    {
    if (map.find(elementId) == map.end())
        map[elementId] = bvector<PropertyData>();

    PropertyData data(accessor, prefix, oldVal, newVal);
    map[elementId].push_back(data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TestEqual(Json::Value & json, ECValue value)
    {
    switch(json.type())
        {
        case Json::ValueType::uintValue:
        case Json::ValueType::intValue:
            ASSERT_EQ(json.asInt(), value.GetInteger());
            return;
        case Json::ValueType::realValue:
            ASSERT_EQ(json.asDouble(), value.GetDouble());
            return;
        case Json::ValueType::stringValue:
            ASSERT_EQ(json.asString(), Utf8String(value.GetUtf8CP()));
            return;
        case Json::ValueType::booleanValue:
            ASSERT_EQ(json.asBool(), value.GetBoolean());
            return;
        }

    ASSERT_TRUE(false && "This shouldn't happen");
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
void CheckPropertyOutput(ElementInputPropertyMap& inputMap, ElementOutputPropertyMap& outputMap)
    {
    for (auto inputEntry : inputMap)
        {
        DgnElementId elementId = inputEntry.first;
        bvector<PropertyData> propertyDatas = inputEntry.second;
        ASSERT_TRUE(outputMap.find(elementId) != outputMap.end());
        Json::Value outputContent = outputMap[elementId];

        Utf8String jsonString = outputContent.ToString();
        if (jsonString.empty()) { }

        for (PropertyData const& data : propertyDatas)
            {
            Json::Value currentValue = outputContent[data.m_accessor]["currentValue"];
            Json::Value targetValue = outputContent[data.m_accessor]["targetValue"];
            ASSERT_TRUE(!currentValue.isNull() && !targetValue.isNull());
            TestEqual(currentValue, data.m_oldValue);
            TestEqual(targetValue, data.m_newValue);
            }
        }
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
TEST_F(VersionCompareTestFixture, ChangedElementsManagerTest1)
    {
    // Test generating the changed elements ECDb based on a change summary
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;
    DgnDbPtr initialDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(initialDb.IsValid());

    // CHANGESET 1
    // Insert elements
    DgnElementPtr firstElement = InsertPhysicalElement("X1");
    DgnElementPtr secondElement = InsertPhysicalElement("X2");
    elementMap[firstElement->GetElementId()] = ElementData(firstElement, DbOpcode::Insert);
    elementMap[secondElement->GetElementId()] = ElementData(secondElement, DbOpcode::Insert);
    changesets.push_back(CreateRevision());

    // CHANGESET 2
    // Create another element and update one
    DgnElementPtr thirdElement = InsertPhysicalElement("X3");
    elementMap[thirdElement->GetElementId()] = ElementData(thirdElement, DbOpcode::Insert);
    // Should still be mark as an insertion
    ModifyElementPlacement(secondElement->GetElementId());
    changesets.push_back(CreateRevision());

    // CHANGESET 3
    // Delete an element
    m_db->Elements().Delete(firstElement->GetElementId());
    changesets.push_back(CreateRevision());

    // Filename and cleanup if needed
    BeFileName cacheFilename;
    BeTest::GetHost().GetOutputRoot(cacheFilename);
    cacheFilename.AppendToPath(L"ChangedElements.chems");
    if (BeFileName::DoesPathExist(cacheFilename.GetName()))
        BeFileName::BeDeleteFile(cacheFilename.GetName());

	// Process forward from initial Db
    ChangedElementsManager ceMgr(initialDb);
    ceMgr.SetWantChunkTraversal(true);
    ECDb cacheDb;
    // Create the Db file
    // TODO: Good filename
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.CreateChangedElementsCache(cacheDb, cacheFilename));
    // Process the changeset
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.ProcessChangesets(cacheDb, "Items", changesets));
    // Should be able to retrieve the changed elements from the cache Db
    ChangedElementsMap map;
    // Check first changeset 1 and 2 above
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedElements(cacheDb, map, changesets[0]->GetChangesetId(), changesets[1]->GetChangesetId()));
    // Check size and data from retrieved map
    EXPECT_EQ(3, map.size());
    // Should contain all of those elements inserted in 1 and 2
    EXPECT_FALSE(map.find(firstElement->GetECInstanceKey()) == map.end());
    EXPECT_FALSE(map.find(secondElement->GetECInstanceKey()) == map.end());
    EXPECT_FALSE(map.find(thirdElement->GetECInstanceKey()) == map.end());
    EXPECT_EQ(map[firstElement->GetECInstanceKey()].m_opcode, DbOpcode::Insert);
    EXPECT_EQ(map[secondElement->GetECInstanceKey()].m_opcode, DbOpcode::Insert);
    EXPECT_EQ(map[thirdElement->GetECInstanceKey()].m_opcode, DbOpcode::Insert);
    // Test we have the correct model Ids
    EXPECT_EQ(elementMap[firstElement->GetElementId()].m_modelId, map[firstElement->GetECInstanceKey()].m_modelId);
    EXPECT_EQ(elementMap[secondElement->GetElementId()].m_modelId, map[secondElement->GetECInstanceKey()].m_modelId);
    EXPECT_EQ(elementMap[thirdElement->GetElementId()].m_modelId, map[thirdElement->GetECInstanceKey()].m_modelId);

    // Test we can get changed models
    bmap<DgnModelId, AxisAlignedBox3d> changedModels;
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedModels(cacheDb, changedModels, changesets[0]->GetChangesetId(), changesets[1]->GetChangesetId()));
    // Should have the default model in the changed models
    EXPECT_TRUE(changedModels.find(m_defaultModelId) != changedModels.end());

    // Now test getting changed elements for changeset 2 only
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedElements(cacheDb, map, changesets[1]->GetChangesetId(), changesets[1]->GetChangesetId()));
    EXPECT_EQ(2, map.size());
    // Should contain the third inserted element and the update to the second element
    EXPECT_TRUE(map.find(firstElement->GetECInstanceKey()) == map.end());
    EXPECT_FALSE(map.find(secondElement->GetECInstanceKey()) == map.end());
    EXPECT_FALSE(map.find(thirdElement->GetECInstanceKey()) == map.end());
    EXPECT_EQ(map[secondElement->GetECInstanceKey()].m_opcode, DbOpcode::Update);
    EXPECT_EQ(map[thirdElement->GetECInstanceKey()].m_opcode, DbOpcode::Insert);

    // Test we can get changed models for only one changeset
    changedModels.clear();
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedModels(cacheDb, changedModels, changesets[1]->GetChangesetId(), changesets[1]->GetChangesetId()));
    // Should have the default model in the changed models
    EXPECT_TRUE(changedModels.find(m_defaultModelId) != changedModels.end());

    // Now test getting full range of changed elements, first element should never appear due to accumulation
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedElements(cacheDb, map, changesets[0]->GetChangesetId(), changesets[2]->GetChangesetId()));
    EXPECT_EQ(2, map.size());
    // Should contain the elements that were inserted but not the one that got deleted
    EXPECT_FALSE(map.find(secondElement->GetECInstanceKey()) == map.end());
    EXPECT_FALSE(map.find(thirdElement->GetECInstanceKey()) == map.end());
    EXPECT_EQ(map[secondElement->GetECInstanceKey()].m_opcode, DbOpcode::Insert);
    EXPECT_EQ(map[thirdElement->GetECInstanceKey()].m_opcode, DbOpcode::Insert);

    // Test we can get changed models for all changesets
    changedModels.clear();
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedModels(cacheDb, changedModels, changesets[0]->GetChangesetId(), changesets[2]->GetChangesetId()));
    // Should have the default model in the changed models
    EXPECT_TRUE(changedModels.find(m_defaultModelId) != changedModels.end());

    // Close cache Db
    cacheDb.CloseDb();
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
TEST_F(VersionCompareTestFixture, ChangedElementsManagerTest_SingleChangeset)
    {
    // Test single changeset case that is handled without creating a temporary cloned db
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    // CHANGESET 1
    // Insert some elements
    DgnElementPtr firstElement = InsertPhysicalElement("X1");
    DgnElementPtr secondElement = InsertPhysicalElement("X2");
    elementMap[firstElement->GetElementId()] = ElementData(firstElement, DbOpcode::Insert);
    elementMap[secondElement->GetElementId()] = ElementData(secondElement, DbOpcode::Insert);
    changesets.push_back(CreateRevision());

    // Filename and cleanup if needed
    BeFileName cacheFilename;
    BeTest::GetHost().GetOutputRoot(cacheFilename);
    cacheFilename.AppendToPath(L"ChangedElements.chems");
    if (BeFileName::DoesPathExist(cacheFilename.GetName()))
        BeFileName::BeDeleteFile(cacheFilename.GetName());

    ChangedElementsManager ceMgr(m_db);
    ceMgr.SetWantChunkTraversal(true);
    ECDb cacheDb;
    // Create the Db file
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.CreateChangedElementsCache(cacheDb, cacheFilename));
    // Process the changeset
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.ProcessChangesets(cacheDb, "Items", changesets));
    // Should be able to retrieve the changed elements from the cache Db
    ChangedElementsMap map;
    // Get elements for the single changeset
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedElements(cacheDb, map, changesets[0]->GetChangesetId(), changesets[0]->GetChangesetId()));
    // Check size and data from retrieved map
    EXPECT_EQ(2, map.size());
    // Should contain all of those elements inserted in 1 and 2
    EXPECT_FALSE(map.find(firstElement->GetECInstanceKey()) == map.end());
    EXPECT_FALSE(map.find(secondElement->GetECInstanceKey()) == map.end());
    EXPECT_EQ(map[firstElement->GetECInstanceKey()].m_opcode, DbOpcode::Insert);
    EXPECT_EQ(map[secondElement->GetECInstanceKey()].m_opcode, DbOpcode::Insert);
    // Test we have the correct model Ids
    EXPECT_EQ(elementMap[firstElement->GetElementId()].m_modelId, map[firstElement->GetECInstanceKey()].m_modelId);
    EXPECT_EQ(elementMap[secondElement->GetElementId()].m_modelId, map[secondElement->GetECInstanceKey()].m_modelId);

    // Test we can get changed models
    bmap<DgnModelId, AxisAlignedBox3d> changedModels;
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedModels(cacheDb, changedModels, changesets[0]->GetChangesetId(), changesets[0]->GetChangesetId()));
    // Should have the default model in the changed models
    EXPECT_TRUE(changedModels.find(m_defaultModelId) != changedModels.end());

    // Close cache Db
    cacheDb.CloseDb();
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
TEST_F(VersionCompareTestFixture, ChangedElementsManagerTest2_ChangedModels)
    {
    // Test generating the changed elements ECDb based on a change summary
    bvector<DgnRevisionPtr> changesets;
    DgnDbPtr initialDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(initialDb.IsValid());

    // CHANGESET 1
    // Insert elements
    DgnElementPtr firstElement = InsertPhysicalElement("X1", DPoint3d::From(1.0, 1.0, 1.0), DPoint3d::From(1.0, 1.0, 1.0));
    DgnElementPtr secondElement = InsertPhysicalElement("X2", DPoint3d::From(3.0, 3.0, 3.0), DPoint3d::From(1.0, 1.0, 1.0));
    DgnRevisionPtr changeset1 = CreateRevision();
    changesets.push_back(changeset1);

    // CHANGESET 2
    // Create another element and update one
    DgnElementPtr thirdElement = InsertPhysicalElement("X3", DPoint3d::From(9.0, 9.0, 9.0), DPoint3d::From(1.0, 1.0, 1.0));
    // Should still be mark as an insertion
    ModifyElementPlacement(secondElement->GetElementId());
    DgnRevisionPtr changeset2 = CreateRevision();
    changesets.push_back(changeset2);

    // Filename and cleanup if needed
    BeFileName cacheFilename;
    BeTest::GetHost().GetOutputRoot(cacheFilename);
    cacheFilename.AppendToPath(L"ChangedElements.chems");
    if (BeFileName::DoesPathExist(cacheFilename.GetName()))
        BeFileName::BeDeleteFile(cacheFilename.GetName());

    ChangedElementsManager ceMgr(initialDb);
    ceMgr.SetWantChunkTraversal(true);
    ECDb cacheDb;
    // Create the Db file
    // TODO: Good filename
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.CreateChangedElementsCache(cacheDb, cacheFilename));
    // Process the changeset
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.ProcessChangesets(cacheDb, "Items", changesets));

    // Test we can get changed models for first changeset
    bmap<DgnModelId, AxisAlignedBox3d> changedModels;
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedModels(cacheDb, changedModels, changeset1->GetChangesetId(), changeset1->GetChangesetId()));
    // Should have the default model in the changed models
    EXPECT_TRUE(changedModels.find(m_defaultModelId) != changedModels.end());
    AxisAlignedBox3d bbox = changedModels[m_defaultModelId];
    // The high of the bounding box should be the high center second element (3.0, 3.0, 3.0) plus the size of the box halved (1.0, 1.0, 1.0)
    EXPECT_EQ(bbox.high.x, 3.5);
    EXPECT_EQ(bbox.high.y, 3.5);
    EXPECT_EQ(bbox.high.z, 3.5);
    // Same for the low because of first element inserted in (.0, .0, .0)
    EXPECT_EQ(bbox.low.x, 0.5);
    EXPECT_EQ(bbox.low.y, 0.5);
    EXPECT_EQ(bbox.low.z, 0.5);

    // Test we can get changed models for both changesets
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedModels(cacheDb, changedModels, changeset1->GetChangesetId(), changeset2->GetChangesetId()));
    // Should have the default model in the changed models
    EXPECT_TRUE(changedModels.find(m_defaultModelId) != changedModels.end());
    bbox = changedModels[m_defaultModelId];
    // The high of the bounding box should be the high center third element (9.0, 9.0, 9.0) plus the size of the box halved (1.0, 1.0, 1.0)
    EXPECT_EQ(bbox.high.x, 9.5);
    EXPECT_EQ(bbox.high.y, 9.5);
    EXPECT_EQ(bbox.high.z, 9.5);
    // Same for the low because of first element inserted in (.0, .0, .0)
    EXPECT_EQ(bbox.low.x, 0.5);
    EXPECT_EQ(bbox.low.y, 0.5);
    EXPECT_EQ(bbox.low.z, 0.5);

    // Test we can get changed models for last changeset only
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedModels(cacheDb, changedModels, changeset2->GetChangesetId(), changeset2->GetChangesetId()));
    // Should have the default model in the changed models
    EXPECT_TRUE(changedModels.find(m_defaultModelId) != changedModels.end());
    bbox = changedModels[m_defaultModelId];
    // The high of the bounding box should be the high center third element (9.0, 9.0, 9.0) plus the size of the box halved (1.0, 1.0, 1.0)
    EXPECT_EQ(bbox.high.x, 9.5);
    EXPECT_EQ(bbox.high.y, 9.5);
    EXPECT_EQ(bbox.high.z, 9.5);
    // Account for the modification of the second element done, should have (3.0, 3.0, 3.0) center, then moved in X one unit, minus 1.0 halved (size)
    EXPECT_EQ(bbox.low.x, 3.5);
    EXPECT_EQ(bbox.low.y, 2.5);
    EXPECT_EQ(bbox.low.z, 2.5);

    // Close cache Db
    cacheDb.CloseDb();
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
TEST_F(VersionCompareTestFixture, ChangedElementsManagerTest_PassRulesetDirectoryToManager)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // INITIAL CHANGESET
    // Create the starting changeset containing an element with an aspect
    TestElementPtr tempEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement");
    TestUniqueAspectPtr aspect = TestUniqueAspect::Create("Initial Value");
    DgnElement::UniqueAspect::SetAspect(*tempEl, *aspect);
    aspect->SetPropertyValue("TestUniqueAspectProperty", ECValue("Old Value for Property"));
    m_db->Elements().Insert(*tempEl);
    DgnRevisionPtr initialRevision = CreateRevision();
    changesets.push_back(initialRevision);
    ASSERT_TRUE(initialRevision.IsValid());

    // CHANGESET 1
    // Modify the aspect, we should get a modification of the element
    ECClassCP aspectClassUnique = TestUniqueAspect::GetECClass(*m_db);
    ASSERT_NE(aspectClassUnique, nullptr);
    SetUniqueAspectPropertyValue(*tempEl, *aspectClassUnique, "TestUniqueAspectProperty", "New Value for Property");
    tempEl->Update();
    elementMap[tempEl->GetElementId()] = ElementData(tempEl, DbOpcode::Update, ElementChangesType::Type::Mask_Indirect);

    DgnRevisionPtr changeset1 = CreateRevision();
    DumpRevision(*changeset1, "ChangedElementsManagerTest_Aspects: Aspect modification");
    changesets.push_back(changeset1);

    // Filename and cleanup if needed
    BeFileName cacheFilename;
    BeTest::GetHost().GetOutputRoot(cacheFilename);
    cacheFilename.AppendToPath(L"ChangedElements.chems");
    if (BeFileName::DoesPathExist(cacheFilename.GetName()))
        BeFileName::BeDeleteFile(cacheFilename.GetName());

    // Process backwards
    std::reverse(changesets.begin(), changesets.end());

    ChangedElementsManager ceMgr(targetDb);
    ceMgr.SetWantChunkTraversal(true);
    ECDb cacheDb;
    // Create the Db file
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.CreateChangedElementsCache(cacheDb, cacheFilename));
    // IMPORTANT: Test that we can provide a ruleset directory to the manager so that the related property paths are found properly in this test
    ceMgr.SetPresentationRulesetDirectory(VersionCompareTestFixture::m_rulesetDir);
    // Process the changeset
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.ProcessChangesets(cacheDb, "Items", changesets));
    // Should be able to retrieve the changed elements from the cache Db
    ChangedElementsMap map;
    // Check first changeset 1 and 2 above
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedElements(cacheDb, map, changesets[0]->GetChangesetId(), changesets[0]->GetChangesetId()));
    // Check size and data from retrieved map
    EXPECT_EQ(1, map.size());
    // Should contain the element whose aspect got modified
    EXPECT_FALSE(map.find(tempEl->GetECInstanceKey()) == map.end());
    EXPECT_EQ(map[tempEl->GetECInstanceKey()].m_opcode, DbOpcode::Update);
    // Test we have the correct model Ids
    EXPECT_EQ(elementMap[tempEl->GetElementId()].m_modelId, map[tempEl->GetECInstanceKey()].m_modelId);
    // Test we are getting type of change from the db
    EXPECT_EQ(elementMap[tempEl->GetElementId()].m_changes.m_flags, ElementChangesType::Type::Mask_Indirect);

    // Test we can get changed models
    bmap<DgnModelId, AxisAlignedBox3d> changedModels;
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedModels(cacheDb, changedModels, changesets[0]->GetChangesetId(), changesets[0]->GetChangesetId()));
    // Should have the default model in the changed models
    EXPECT_TRUE(changedModels.find(m_defaultModelId) != changedModels.end());

    // Close cache Db
    cacheDb.CloseDb();
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
TEST_F(VersionCompareTestFixture, ChangesTypeTest)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    // INITIAL CHANGESET
    // Create the starting changeset containing  some elements
    DgnElementPtr placementEl = InsertPhysicalElement("A");
    DgnElementPtr geometryEl = InsertPhysicalElement("B");
    TestElementPtr propertyEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "PropertyEl");
    TestElementPtr hiddenPropEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "HiddenPropertyEl");
    TestElementPtr indirectEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "IndirectEl");
    TestElementPtr multiEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "MultiEl");
    TestUniqueAspectPtr aspect = TestUniqueAspect::Create("Initial Value");
    DgnElement::UniqueAspect::SetAspect(*indirectEl, *aspect);
    aspect->SetPropertyValue("TestUniqueAspectProperty", ECValue("Old Value for Property"));
    m_db->Elements().Insert(*placementEl);
    m_db->Elements().Insert(*geometryEl);
    m_db->Elements().Insert(*propertyEl);
    m_db->Elements().Insert(*hiddenPropEl);
    m_db->Elements().Insert(*indirectEl);
    m_db->Elements().Insert(*multiEl);

    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // CHANGESET 1
    // Indirect change via aspect
    ECClassCP aspectClassUnique = TestUniqueAspect::GetECClass(*m_db);
    ASSERT_NE(aspectClassUnique, nullptr);
    SetUniqueAspectPropertyValue(*indirectEl, *aspectClassUnique, "TestUniqueAspectProperty", "New Value for Property");
    indirectEl->Update();
    bvector<Utf8String> props1;
    props1.push_back("TestUniqueAspectProperty");
    elementMap[indirectEl->GetElementId()] = ElementData(indirectEl, DbOpcode::Update, ElementChangesType::Type::Mask_Indirect | ElementChangesType::Type::Mask_Property, props1);
    // Property change
    propertyEl->SetIntegerProperty(0, 5);
    propertyEl->Update();
    bvector<Utf8String> props2;
    props2.push_back("TestIntegerProperty1");
    elementMap[propertyEl->GetElementId()] = ElementData(propertyEl, DbOpcode::Update, ElementChangesType::Type::Mask_Property, props2);
    // Hidden property change (changed properties should not show JsonProperties)
    hiddenPropEl->SetPropertyValue("JsonProperties", "{}");
    hiddenPropEl->Update();
    elementMap[hiddenPropEl->GetElementId()] = ElementData(hiddenPropEl, DbOpcode::Update, ElementChangesType::Type::Mask_Hidden);
    // Geometry change
    ModifyElementGeometry(geometryEl->GetElementId(), false);
    elementMap[geometryEl->GetElementId()] = ElementData(geometryEl, DbOpcode::Update, ElementChangesType::Type::Mask_Geometry);
    // Placement change
    ModifyElementPlacement(placementEl->GetElementId());
    elementMap[placementEl->GetElementId()] = ElementData(placementEl, DbOpcode::Update, ElementChangesType::Type::Mask_Placement);
    // Multiple type of changes (Property, hidden and placement changes)
    ModifyMultiple(multiEl->GetElementId());
    bvector<Utf8String> props4;
    // Only non-hidden property
    props4.push_back("TestIntegerProperty1");
    elementMap[multiEl->GetElementId()] = ElementData(multiEl, DbOpcode::Update, ElementChangesType::Type::Mask_Placement | ElementChangesType::Type::Mask_Geometry | ElementChangesType::Type::Mask_Property | ElementChangesType::Type::Mask_Hidden, props4);
    // Create changeset
    changesets.push_back(CreateRevision());

    // Test that the output matches with the input rolling forward
    // Single changeset must be processed with up-to-date Db
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
TEST_F(VersionCompareTestFixture, PropertyMergingTest)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    // INITIAL CHANGESET
    // Create the starting changeset containing  some elements
    TestElementPtr propertyEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "PropertyEl");
    m_db->Elements().Insert(*propertyEl);

    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // CHANGESET - Change TestIntegerProperty1 to 5 (inside ModifyMultiple) and JsonProperties (hidden property)
    ModifyMultiple(propertyEl->GetElementId(), false);
    changesets.push_back(CreateRevision());

    // CHANGESET - Change TestIntegerProperty2
    ModifyIntProp(propertyEl->GetElementId(), 1, 3);
    changesets.push_back(CreateRevision());

    // CHANGESET - Change TestIntegerProperty3 and TestIntegerProperty4
    ModifyIntProp(propertyEl->GetElementId(), 2, 3);
    ModifyIntProp(propertyEl->GetElementId(), 3, 3);
    changesets.push_back(CreateRevision());

    // CHANGESET - Change the same TestIntegerProperty1 again (should only show once)
    ModifyIntProp(propertyEl->GetElementId(), 0, 3);
    changesets.push_back(CreateRevision());

    // All properties that changed in all changesets
    // Hidden properties are not part of the result, so JsonProperties shouldn't show
    bvector<Utf8String> props;
    props.push_back("TestIntegerProperty1");
    props.push_back("TestIntegerProperty2");
    props.push_back("TestIntegerProperty3");
    props.push_back("TestIntegerProperty4");
    elementMap[propertyEl->GetElementId()] = ElementData(propertyEl, DbOpcode::Update, ElementChangesType::Type::Mask_Property | ElementChangesType::Type::Mask_Hidden, props);

    // Test that the output matches with the input rolling forward
    CreateSummaryAndCheckOutput(targetDb, elementMap, changesets, *m_manager);

    // Check manager can accumulate property names properly from Db
    // Filename and cleanup if needed
    BeFileName cacheFilename;
    BeTest::GetHost().GetOutputRoot(cacheFilename);
    cacheFilename.AppendToPath(L"ChangedElements.chems");
    if (BeFileName::DoesPathExist(cacheFilename.GetName()))
        BeFileName::BeDeleteFile(cacheFilename.GetName());

    ChangedElementsManager ceMgr(targetDb);
    ceMgr.SetWantChunkTraversal(true);
    ECDb cacheDb;
    // Create the Db file
    // TODO: Good filename
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.CreateChangedElementsCache(cacheDb, cacheFilename));
    // Process the changeset
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.ProcessChangesets(cacheDb, "Items", changesets));
    // Should be able to retrieve the changed elements from the cache Db
    ChangedElementsMap map;
    // Check accumulation of all changesets
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedElements(cacheDb, map, changesets[3]->GetChangesetId(), changesets[0]->GetChangesetId()));
    // Check size and data from retrieved map
    EXPECT_EQ(1, map.size());
    // Element should exist in cache
    EXPECT_FALSE(map.find(propertyEl->GetECInstanceKey()) == map.end());
    // Ensure we have all expected properties in the result
    ChangedElementRecord& info = map.find(propertyEl->GetECInstanceKey())->second;
    EXPECT_TRUE(info.m_changes.m_properties.size() == 4);
    EXPECT_TRUE(info.m_changes.m_properties.find("TestIntegerProperty1") != info.m_changes.m_properties.end());
    EXPECT_TRUE(info.m_changes.m_properties.find("TestIntegerProperty2") != info.m_changes.m_properties.end());
    EXPECT_TRUE(info.m_changes.m_properties.find("TestIntegerProperty3") != info.m_changes.m_properties.end());
    EXPECT_TRUE(info.m_changes.m_properties.find("TestIntegerProperty4") != info.m_changes.m_properties.end());
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
TEST_F(VersionCompareTestFixture, PropertyChecksumTest)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    // INITIAL CHANGESET
    // Create the starting changeset containing  some elements
    TestElementPtr propertyEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "PropertyEl");
    // Initialize it to 3 so that we test for same checksum at the end
    propertyEl->SetIntegerProperty(0, 3);
    propertyEl->SetPropertyValue("TestElementProperty", "Initial Value");
    m_db->Elements().Insert(*propertyEl);

    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // CHANGESET - Change TestIntegerProperty1 to 1 (inside ModifyMultiple) and JsonProperties (hidden property)
    // Change TestIntegerProperty2 to 1
    ModifyIntProp(propertyEl->GetElementId(), 0, 1);
    ModifyIntProp(propertyEl->GetElementId(), 1, 1);
    ModifyStringProp(propertyEl->GetElementId(), "New Value");
    changesets.push_back(CreateRevision());

    // CHANGESET - Change TestIntegerProperty1 to 5
    // Change TestIntegerProperty2 to 2
    ModifyIntProp(propertyEl->GetElementId(), 0, 5);
    ModifyIntProp(propertyEl->GetElementId(), 1, 2);
    changesets.push_back(CreateRevision());

    // CHANGESET - Change TestIntegerProperty1 back to 3
    // Change TestIntegerProperty2 to 3
    // Change string prop back to initial value to test checksums
    ModifyIntProp(propertyEl->GetElementId(), 0, 3);
    ModifyIntProp(propertyEl->GetElementId(), 1, 3);
    ModifyStringProp(propertyEl->GetElementId(), "Initial Value");
    changesets.push_back(CreateRevision());

    // We should have the following changed properties
    bvector<Utf8String> props;
    props.push_back("TestIntegerProperty1");
    props.push_back("TestIntegerProperty2");
    props.push_back("TestElementProperty");
    elementMap[propertyEl->GetElementId()] = ElementData(propertyEl, DbOpcode::Update, ElementChangesType::Type::Mask_Property, props);

    // Test that the output matches with the input rolling forward
    CreateSummaryAndCheckOutput(targetDb, elementMap, changesets, *m_manager);

    // Check manager can accumulate property names properly from Db
    // Filename and cleanup if needed
    BeFileName cacheFilename;
    BeTest::GetHost().GetOutputRoot(cacheFilename);
    cacheFilename.AppendToPath(L"ChangedElements.chems");
    if (BeFileName::DoesPathExist(cacheFilename.GetName()))
        BeFileName::BeDeleteFile(cacheFilename.GetName());

    ChangedElementsManager ceMgr(targetDb);
    ceMgr.SetWantChunkTraversal(true);
    ECDb cacheDb;
    // Create the Db file
    // TODO: Good filename
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.CreateChangedElementsCache(cacheDb, cacheFilename));
    // Process the changeset
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.ProcessChangesets(cacheDb, "Items", changesets));
    // Should be able to retrieve the changed elements from the cache Db
    ChangedElementsMap map;
    // Check accumulation of all changesets
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedElements(cacheDb, map, changesets[2]->GetChangesetId(), changesets[0]->GetChangesetId()));
    // Check size and data from retrieved map
    EXPECT_EQ(1, map.size());
    // Element should exist in cache
    EXPECT_FALSE(map.find(propertyEl->GetECInstanceKey()) == map.end());
    // Ensure we have all expected properties in the result
    ChangedElementRecord& info = map.find(propertyEl->GetECInstanceKey())->second;
    EXPECT_TRUE(info.m_changes.m_properties.size() == 3);
    EXPECT_TRUE(info.m_changes.m_properties.find("TestIntegerProperty1") != info.m_changes.m_properties.end());
    EXPECT_TRUE(info.m_changes.m_properties.find("TestIntegerProperty2") != info.m_changes.m_properties.end());
    EXPECT_TRUE(info.m_changes.m_properties.find("TestElementProperty") != info.m_changes.m_properties.end());

    // Checksum for the property should be available
    EXPECT_TRUE(info.m_changes.m_propertyChecksums.find("TestIntegerProperty1") != info.m_changes.m_propertyChecksums.end());
    auto const& checksumInfo = info.m_changes.m_propertyChecksums.find("TestIntegerProperty1")->second;
    EXPECT_TRUE(checksumInfo.m_oldValue != 0);
    EXPECT_TRUE(checksumInfo.m_newValue != 0);
    // Since the value of the property was the same in the first and in the last changeset, they should match
    EXPECT_TRUE(checksumInfo.m_oldValue == checksumInfo.m_newValue);

    // Checksum for the second property should be available
    EXPECT_TRUE(info.m_changes.m_propertyChecksums.find("TestIntegerProperty2") != info.m_changes.m_propertyChecksums.end());
    auto const& checksumInfo2 = info.m_changes.m_propertyChecksums.find("TestIntegerProperty2")->second;
    EXPECT_TRUE(checksumInfo2.m_oldValue != 0);
    EXPECT_TRUE(checksumInfo2.m_newValue != 0);
    // Since the value of the property is different in the end of the changesets than in the beginning, they should not match
    EXPECT_TRUE(checksumInfo2.m_oldValue != checksumInfo2.m_newValue);

    // Checksum for the third property should be available
    EXPECT_TRUE(info.m_changes.m_propertyChecksums.find("TestElementProperty") != info.m_changes.m_propertyChecksums.end());
    auto const& checksumInfo3 = info.m_changes.m_propertyChecksums.find("TestElementProperty")->second;
    EXPECT_TRUE(checksumInfo3.m_oldValue != 0);
    EXPECT_TRUE(checksumInfo3.m_newValue != 0);
    // Checksum for the string property should be the same "Initial Value" vs. "Initial Value"
    EXPECT_TRUE(checksumInfo3.m_oldValue == checksumInfo3.m_newValue);

    // Should be able to retrieve the changed elements from the cache Db for just changeset 0 and 1
    ChangedElementsMap map2;
    // Check accumulation of all changesets
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedElements(cacheDb, map2, changesets[1]->GetChangesetId(), changesets[0]->GetChangesetId()));
    // Get info
    ChangedElementRecord& info2 = map2.find(propertyEl->GetECInstanceKey())->second;

    // Checksum for the string property should be available
    EXPECT_TRUE(info2.m_changes.m_propertyChecksums.find("TestElementProperty") != info2.m_changes.m_propertyChecksums.end());
    auto const& checksumInfo4 = info2.m_changes.m_propertyChecksums.find("TestElementProperty")->second;
    EXPECT_TRUE(checksumInfo4.m_oldValue != 0);
    EXPECT_TRUE(checksumInfo4.m_newValue != 0);
    // Checksum for the string property should be different "Initial Value" vs. "New Value"
    EXPECT_TRUE(checksumInfo4.m_oldValue != checksumInfo4.m_newValue);
    }


//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
TEST_F(VersionCompareTestFixture, AspectCreationTest)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    // INITIAL CHANGESET
    // Create an element
    TestElementPtr indirectEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "IndirectAspectEl");
    m_db->Elements().Insert(*indirectEl);

    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // CHANGESET 1
    // Create an aspect and add it to the indirect element
    RefCountedPtr<PhysicalElement> editElement = m_db->Elements().GetForEdit<PhysicalElement>(indirectEl->GetElementId());
    ASSERT_TRUE(editElement.IsValid());
    TestUniqueAspectPtr aspect = TestUniqueAspect::Create("Initial Value");
    DgnElement::UniqueAspect::SetAspect(*editElement, *aspect);
    aspect->SetPropertyValue("TestUniqueAspectProperty", ECValue("Old Value for Property"));
    // Update element
    DgnDbStatus dbStatus = editElement->Update();
    ASSERT_TRUE(dbStatus == DgnDbStatus::Success);
    // Add the element to our testing map, should show as an update with indirect props changes, including the aspect property
    bvector<Utf8String> props;
    props.push_back("TestUniqueAspectProperty");
    elementMap[indirectEl->GetElementId()] = ElementData(indirectEl, DbOpcode::Update, ElementChangesType::Type::Mask_Indirect | ElementChangesType::Type::Mask_Property, props);
    // Create changeset
    changesets.push_back(CreateRevision());

    // Test that the output matches with the input rolling forward
	// Single changeset must be processed with latest Db
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
TEST_F(VersionCompareTestFixture, AspectSetTest)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    // INITIAL CHANGESET
    // Create 2 elements, 1 aspect and set the aspect to one of them
    TestElementPtr indirectEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "IndirectAspectEl");
    TestElementPtr indirectEl2 = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "IndirectAspectEl2");
    TestUniqueAspectPtr aspect = TestUniqueAspect::Create("Initial Value");
    aspect->SetPropertyValue("TestUniqueAspectProperty", ECValue("Old Value for Property"));
    m_db->Elements().Insert(*indirectEl);
    m_db->Elements().Insert(*indirectEl2);
    // Set aspect to one element
    DgnElement::UniqueAspect::SetAspect(*indirectEl, *aspect);

    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // CHANGESET 1
    // "Re-parent" the aspect to the other element and try to find properties for it that aren't in the changeset
    RefCountedPtr<PhysicalElement> editElement = m_db->Elements().GetForEdit<PhysicalElement>(indirectEl2->GetElementId());
    DgnElement::UniqueAspect::SetAspect(*editElement, *aspect);
    ASSERT_TRUE(editElement.IsValid());
    // Update element
    DgnDbStatus dbStatus = editElement->Update();
    ASSERT_TRUE(dbStatus == DgnDbStatus::Success);

    // Add the element to our testing map, should show as an update with indirect props changes, including the aspect property
    bvector<Utf8String> props;
    // Since the aspect got fully added to the element, should find all of the aspect properties as changed properties of the element
    props.push_back("TestUniqueAspectProperty");
    elementMap[indirectEl2->GetElementId()] = ElementData(indirectEl2, DbOpcode::Update, ElementChangesType::Type::Mask_Indirect | ElementChangesType::Type::Mask_Property, props);
    // Create changeset
    changesets.push_back(CreateRevision());

    // Test that the output matches with the input rolling forward
	// Single changeset must be processed with latest Db
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
#ifdef WIP_TODO
TEST_F(VersionCompareTestFixture, RelatedPropertyTest)
    {
    // Test the case where only the relationship changes but not the actual elements
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    // INITIAL CHANGESET
    // Create elements and link elements to test related property paths finding Link properties
    DgnDbTestUtils::InsertDrawingCategory(*m_db, "TestDrawingCategory");

    DgnElementCPtr annotation = InsertAnnotationElement();
    ASSERT_TRUE(annotation.IsValid());

    LinkModelPtr linkModel = DgnDbTestUtils::InsertLinkModel(*m_db, "TestLinkModel");
    static const Utf8CP LINK1_DISPLAY_LABEL = "Url Link 1";
    static const Utf8CP LINK1_URL = "http://www.google.com";
    static const Utf8CP LINK1_DESCRIPTION = "This is Url Link Element";
    UrlLinkPtr link1 = UrlLink::Create(UrlLink::CreateParams(*linkModel));
    link1->SetUserLabel(LINK1_DISPLAY_LABEL);
    link1->SetUrl(LINK1_URL);
    link1->SetDescription(LINK1_DESCRIPTION);
    UrlLinkCPtr link1A = link1->Insert();
    ASSERT_TRUE(link1A.IsValid());

    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // CHANGESET 1
    // Create a changeset where we simply connect the link to the element Id, testing the case
    // in which the properties that are changed are NOT in the changeset
    BentleyStatus status = link1A->AddToSource(annotation->GetElementId());
    ASSERT_TRUE(status == SUCCESS);
    bvector<Utf8String> props;
    props.push_back("UserLabel");
    props.push_back("Url");
    props.push_back("Description");
    elementMap[annotation->GetElementId()] = ElementData(annotation, DbOpcode::Update, ElementChangesType::Type::Mask_Indirect | ElementChangesType::Type::Mask_Property, props);
    // Create changeset
    changesets.push_back(CreateRevision());

    // Test that the output matches with the input rolling forward
    // CreateSummaryAndCheckOutput(targetDb, elementMap, changesets, *m_manager, false);
    }
#endif

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
TEST_F(VersionCompareTestFixture, AspectDeleteTest)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    // INITIAL CHANGESET
    // Create an element
    TestElementPtr indirectEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "IndirectAspectEl");
    TestUniqueAspectPtr aspect = TestUniqueAspect::Create("Initial Value");
    TestElementPtr indirectEl2 = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "IndirectAspectEl2");
    TestUniqueAspectPtr aspect2 = TestUniqueAspect::Create("Initial Value");
    aspect2->SetPropertyValue("TestUniqueAspectProperty", ECValue("Old Value for Property"));
    // Set aspects
    DgnElement::UniqueAspect::SetAspect(*indirectEl, *aspect);
    DgnElement::UniqueAspect::SetAspect(*indirectEl2, *aspect2);
    aspect->SetPropertyValue("TestUniqueAspectProperty", ECValue("Old Value for Property"));
    m_db->Elements().Insert(*indirectEl);
    // The only way to get the proper relationship is for an element with the aspect and relationship to exist in the Db
    m_db->Elements().Insert(*indirectEl2);

    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // CHANGESET 1
    // Delete aspect
    ECN::ECClassCR aclass = *TestUniqueAspect::GetECClass(*m_db);
    TestUniqueAspectP aspectP = DgnElement::UniqueAspect::GetP<TestUniqueAspect>(*indirectEl, aclass);
    aspectP->Delete();
    m_db->Elements().Update(*indirectEl);
    // Properties should have changed for the element
    bvector<Utf8String> props;
    props.push_back("TestUniqueAspectProperty");
    elementMap[indirectEl->GetElementId()] = ElementData(indirectEl, DbOpcode::Update, ElementChangesType::Type::Mask_Indirect | ElementChangesType::Type::Mask_Property, props);
    // Create changeset
    changesets.push_back(CreateRevision());

    // Test that the output matches with the input rolling forward
	// Single changeset must be processed with latest Db
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
TEST_F(VersionCompareTestFixture, ParentTestDelete)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    // INITIAL CHANGESET
    // Create an element
    TestElementPtr parent = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "ParentEl");
    TestElementPtr child1 = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "ChildEl1");
    m_db->Elements().Insert(*parent);

    child1->SetParentId(parent->GetElementId(), m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements));
    m_db->Elements().Insert(*child1);

    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    ECInstanceKey parentKey(ECClassId(parent->GetElementClassId().GetValue()), ECInstanceId(parent->GetElementId().GetValue()));

    // CHANGESET 1
    // Delete child1
    m_db->Elements().Delete(child1->GetElementId());
    // Parent Key should be found for deleted element
    elementMap[child1->GetElementId()] = ElementData(child1, DbOpcode::Delete, parentKey);
    // Create changeset
    changesets.push_back(CreateRevision());

    // Test that the output matches with the input rolling forward
    // Single changeset must be processed with up-to-date Db
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager, "", false, false, true);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
TEST_F(VersionCompareTestFixture, ParentTestModify)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    // INITIAL CHANGESET
    // Create an element
    TestElementPtr parent = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "ParentEl");
    m_db->Elements().Insert(*parent);

    TestElementPtr child2 = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "ChildEl2");

    child2->SetParentId(parent->GetElementId(), m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements));
    m_db->Elements().Insert(*child2);

    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    ECInstanceKey parentKey(ECClassId(parent->GetElementClassId().GetValue()), ECInstanceId(parent->GetElementId().GetValue()));

    // CHANGESET 1
    // Change element's geometry and ensure parent key is added to the modification
    ModifyElementGeometry(child2->GetElementId(), true);
    elementMap[child2->GetElementId()] = ElementData(child2, DbOpcode::Update, parentKey, ElementChangesType::Type::Mask_Geometry | ElementChangesType::Type::Mask_Placement);
    // TODO: Indirect change to the parent is marked when the child changes
    // elementMap[parent->GetElementId()] = ElementData(parent, DbOpcode::Update, ElementChangesType::Type::Mask_Indirect);
    // Create changeset
    changesets.push_back(CreateRevision());

    // Test that the output matches with the input rolling forward
    // Single changeset must be processed with up-to-date Db
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager, "", false, false, true);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
TEST_F(VersionCompareTestFixture, ParentTestModifyParent)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    // INITIAL CHANGESET
    // Create an element
    TestElementPtr parent = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "ParentEl");
    TestElementPtr extraParent = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "ExtraParentEl");
    m_db->Elements().Insert(*parent);
    m_db->Elements().Insert(*extraParent);

    TestElementPtr child3 = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "ChildEl3");
    child3->SetParentId(parent->GetElementId(), m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements));
    m_db->Elements().Insert(*child3);

    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    ECInstanceKey parentKey(ECClassId(parent->GetElementClassId().GetValue()), ECInstanceId(parent->GetElementId().GetValue()));
    ECInstanceKey extraParentKey(ECClassId(extraParent->GetElementClassId().GetValue()), ECInstanceId(extraParent->GetElementId().GetValue()));

    // CHANGESET 1
    // Change element's parent
    ModifyElementParent(child3->GetElementId(), extraParent->GetElementId());
    elementMap[child3->GetElementId()] = ElementData(child3, DbOpcode::Update, extraParentKey, ElementChangesType::Type::Mask_Parent);
    // Create changeset
    changesets.push_back(CreateRevision());

    // Test that the output matches with the input rolling forward
    // Single changeset must be processed with up-to-date db
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager, "", false, false, true);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
TEST_F(VersionCompareTestFixture, ParentTestInsert)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    // DgnRevisionPtr initialRevision = CreateRevision();
    // ASSERT_TRUE(initialRevision.IsValid());
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // CHANGESET 1
    // Insert new parent and child elements
    TestElementPtr parent2 = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "ParentEl2");
    TestElementPtr child4 = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "ChildEl4");
    m_db->Elements().Insert(*parent2);
    child4->SetParentId(parent2->GetElementId(), m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements));
    m_db->Elements().Insert(*child4);
    ECInstanceKey parent2Key(ECClassId(parent2->GetElementClassId().GetValue()), ECInstanceId(parent2->GetElementId().GetValue()));
    elementMap[child4->GetElementId()] = ElementData(child4, DbOpcode::Insert, parent2Key);
    elementMap[parent2->GetElementId()] = ElementData(parent2, DbOpcode::Insert);
    // Create changeset
    changesets.push_back(CreateRevision());

    // Test that the output matches with the input rolling forward
    // Single changeset must be processed with up-to-date db
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager, "", false, false, true);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
TEST_F(VersionCompareTestFixture, TopParentTestInsert)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    // DgnRevisionPtr initialRevision = CreateRevision();
    // ASSERT_TRUE(initialRevision.IsValid());
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // CHANGESET 1
    // Insert a top parent, a intermediate child/parent and a child
    TestElementPtr topParent = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "TopParentEl");
    TestElementPtr directChild = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "DirectChildEl");
    TestElementPtr childOfChild = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "LeafChild");
    m_db->Elements().Insert(*topParent);
    directChild->SetParentId(topParent->GetElementId(), m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements));
    m_db->Elements().Insert(*directChild);
    childOfChild->SetParentId(directChild->GetElementId(), m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements));
    m_db->Elements().Insert(*childOfChild);
    ECInstanceKey topParentKey(ECClassId(topParent->GetElementClassId().GetValue()), ECInstanceId(topParent->GetElementId().GetValue()));
    ECInstanceKey directChildKey(ECClassId(directChild->GetElementClassId().GetValue()), ECInstanceId(directChild->GetElementId().GetValue()));
    elementMap[topParent->GetElementId()] = ElementData(topParent, DbOpcode::Insert);
    // Ensure we get the top parent in both scenarios
    elementMap[directChild->GetElementId()] = ElementData(directChild, DbOpcode::Insert, topParentKey);
    elementMap[childOfChild->GetElementId()] = ElementData(childOfChild, DbOpcode::Insert, topParentKey);
    // Create changeset
    changesets.push_back(CreateRevision());

    // Test that the output matches with the input rolling forward
    // Single changeset ust be processed with up-to-date db
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager, "", false, false, true);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
TEST_F(VersionCompareTestFixture, FindTopParentOfModifiedElementThroughAspectChange)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    // SETUP
    // Insert a top parent, a intermediate child/parent and a child
    // Create a physical type
    TestPhysicalTypePtr testType = CreatePhysicalType("TestTypeElementForParent", "Value");
    ASSERT_TRUE(testType.IsValid());
    // Set aspect to the type
    TestUniqueAspectPtr aspect = TestUniqueAspect::Create("Initial Value");
    DgnElement::UniqueAspect::SetAspect(*testType, *aspect);
    aspect->SetPropertyValue("TestUniqueAspectProperty", ECValue("Old Value for Property"));
    testType->Update();
    // Create elements
    TestElementPtr topParent = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "TopParentEl");
    TestElementPtr directChild = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "DirectChildEl");
    m_db->Elements().Insert(*topParent);
    directChild->SetParentId(topParent->GetElementId(), m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements));
    m_db->Elements().Insert(*directChild);
    DgnElementPtr childOfChild = InsertPhysicalElement("LeafChild", testType);
    childOfChild->SetParentId(directChild->GetElementId(), m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements));
    childOfChild->Update();
    ECInstanceKey topParentKey(ECClassId(topParent->GetElementClassId().GetValue()), ECInstanceId(topParent->GetElementId().GetValue()));
    ECInstanceKey directChildKey(ECClassId(directChild->GetElementClassId().GetValue()), ECInstanceId(directChild->GetElementId().GetValue()));

    // BASELINE
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // CHANGESET 1 - Change property of the aspect to ensure that the resulting element's parent is found in the result
    ECClassCP aspectClassUnique = TestUniqueAspect::GetECClass(*m_db);
    ASSERT_NE(aspectClassUnique, nullptr);
    TestPhysicalTypePtr typeForEdit = m_db->Elements().GetForEdit<TestPhysicalType>(testType->GetElementId());
    SetUniqueAspectPropertyValue(*typeForEdit, *aspectClassUnique, "TestUniqueAspectProperty", "New Value for Property");
    typeForEdit->Update();
    bvector<Utf8String> props;
    props.push_back("TestUniqueAspectProperty");
    elementMap[typeForEdit->GetElementId()] = ElementData(typeForEdit, DbOpcode::Update, ElementChangesType::Type::Mask_Property | ElementChangesType::Type::Mask_Indirect, props);
    // Element should show the changed property with property change and indirect change
    elementMap[childOfChild->GetElementId()] = ElementData(childOfChild, DbOpcode::Update, topParentKey, ElementChangesType::Type::Mask_Property | ElementChangesType::Type::Mask_Indirect, props);
    // Create changeset
    changesets.push_back(CreateRevision());

    // Test that the output matches with the input rolling forward
    // Single changeset ust be processed with up-to-date db
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager, "", false, false, true);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
TEST_F(VersionCompareTestFixture, TopParentTestDelete)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    TestElementPtr topParent = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "TopParentEl");
    TestElementPtr directChild = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "DirectChildEl");
    TestElementPtr childOfChild = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "LeafChild");
    m_db->Elements().Insert(*topParent);
    directChild->SetParentId(topParent->GetElementId(), m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements));
    m_db->Elements().Insert(*directChild);
    childOfChild->SetParentId(directChild->GetElementId(), m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements));
    m_db->Elements().Insert(*childOfChild);

    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    ECInstanceKey topParentKey(ECClassId(topParent->GetElementClassId().GetValue()), ECInstanceId(topParent->GetElementId().GetValue()));
    ECInstanceKey directChildKey(ECClassId(directChild->GetElementClassId().GetValue()), ECInstanceId(directChild->GetElementId().GetValue()));

    // CHANGESET 1
    // Delete elements and ensure we can find the proper parents
    m_db->Elements().Delete(topParent->GetElementId());
    m_db->Elements().Delete(directChild->GetElementId());
    m_db->Elements().Delete(childOfChild->GetElementId());
    elementMap[topParent->GetElementId()] = ElementData(topParent, DbOpcode::Delete);
    // Ensure we get the top parent in both scenarios
    elementMap[directChild->GetElementId()] = ElementData(directChild, DbOpcode::Delete, topParentKey);
    elementMap[childOfChild->GetElementId()] = ElementData(childOfChild, DbOpcode::Delete, topParentKey);
    // Create changeset
    changesets.push_back(CreateRevision());

    // Test that the output matches with the input rolling forward
    // Single changeset must be processed with up-to-date Db
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager, "", false, false, true);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
TEST_F(VersionCompareTestFixture, NestedPropertyPaths)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    // Test a nested property path as follows
    // TestElement -> PhysicalElementIsOfType -> TestPhysicalType -> ElementOwnsUniqueAspect -> TestUniqueAspect

    // Create a physical type
    TestPhysicalTypePtr testType = CreatePhysicalType("TestTypeElement", "Value");
    ASSERT_TRUE(testType.IsValid());
    // Set aspect to the type
    TestUniqueAspectPtr aspect = TestUniqueAspect::Create("Initial Value");
    DgnElement::UniqueAspect::SetAspect(*testType, *aspect);
    aspect->SetPropertyValue("TestUniqueAspectProperty", ECValue("Old Value for Property"));
    testType->Update();
    // Create physical elements with given type
    DgnElementPtr element = InsertPhysicalElement("ElementWithType", testType);
    DgnElementPtr element2 = InsertPhysicalElement("ElementWithType2", testType);

    // Create baseline revision
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // Changeset 1 - Change property of the aspect
    ECClassCP aspectClassUnique = TestUniqueAspect::GetECClass(*m_db);
    ASSERT_NE(aspectClassUnique, nullptr);
    TestPhysicalTypePtr typeForEdit = m_db->Elements().GetForEdit<TestPhysicalType>(testType->GetElementId());
    SetUniqueAspectPropertyValue(*typeForEdit, *aspectClassUnique, "TestUniqueAspectProperty", "New Value for Property");
    typeForEdit->Update();
    bvector<Utf8String> props;
    props.push_back("TestUniqueAspectProperty");
    elementMap[typeForEdit->GetElementId()] = ElementData(typeForEdit, DbOpcode::Update, ElementChangesType::Type::Mask_Property | ElementChangesType::Type::Mask_Indirect, props);
    // Element should show the changed property with property change and indirect change
    elementMap[element->GetElementId()] = ElementData(element, DbOpcode::Update, ElementChangesType::Type::Mask_Property | ElementChangesType::Type::Mask_Indirect, props);
    elementMap[element2->GetElementId()] = ElementData(element2, DbOpcode::Update, ElementChangesType::Type::Mask_Property | ElementChangesType::Type::Mask_Indirect, props);
    // Create changeset
    changesets.push_back(CreateRevision());

    // Test that the output matches with the input rolling forward
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager, "", false, false, true);
    }

TEST_F(VersionCompareTestFixture, PerfNestedProps)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    // Test a nested property path as follows
    // TestElement -> PhysicalElementIsOfType -> TestPhysicalType -> ElementOwnsUniqueAspect -> TestUniqueAspect

    bvector<DgnElementId> typeIds;
    int testAmount = 1000;
    bvector<Utf8String> props;
    props.push_back("TestUniqueAspectProperty");
    for (int i = 0; i < testAmount; ++i)
        {
        // Create a physical type
        Utf8PrintfString name("PerfTestTypeElement%d",i);
        Utf8PrintfString nameElem1("Element1%d",i);
        Utf8PrintfString nameElem2("Element2%d",i);
        TestPhysicalTypePtr testType = CreatePhysicalType(name.c_str(), "Value");
        ASSERT_TRUE(testType.IsValid());
        // Set aspect to the type
        TestUniqueAspectPtr aspect = TestUniqueAspect::Create("Initial Value");
        DgnElement::UniqueAspect::SetAspect(*testType, *aspect);
        aspect->SetPropertyValue("TestUniqueAspectProperty", ECValue("Old Value for Property"));
        testType->Update();
        // Create physical elements with given type
        DgnElementPtr element = InsertPhysicalElement(nameElem1.c_str(), testType);
        DgnElementPtr element2 = InsertPhysicalElement(nameElem2.c_str(), testType);
        // Store type ids to change
        typeIds.push_back(testType->GetElementId());

        // Set element map to test results against
        elementMap[testType->GetElementId()] = ElementData(testType, DbOpcode::Update, ElementChangesType::Type::Mask_Property | ElementChangesType::Type::Mask_Indirect, props);
        elementMap[element->GetElementId()] = ElementData(element, DbOpcode::Update, ElementChangesType::Type::Mask_Property | ElementChangesType::Type::Mask_Indirect, props);
        elementMap[element2->GetElementId()] = ElementData(element2, DbOpcode::Update, ElementChangesType::Type::Mask_Property | ElementChangesType::Type::Mask_Indirect, props);
        }

    // Create baseline revision
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    ECClassCP aspectClassUnique = TestUniqueAspect::GetECClass(*m_db);
    ASSERT_NE(aspectClassUnique, nullptr);
    // Changeset 1 - Change property of the aspect
    for (int i = 0; i < typeIds.size(); ++i)
        {
        TestPhysicalTypePtr typeForEdit = m_db->Elements().GetForEdit<TestPhysicalType>(typeIds[i]);
        SetUniqueAspectPropertyValue(*typeForEdit, *aspectClassUnique, "TestUniqueAspectProperty", "New Value for Property");
        typeForEdit->Update();
        }
    changesets.push_back(CreateRevision());

    // Test that the output matches with the input rolling forward
    CreateSummaryAndCheckOutput(targetDb, elementMap, changesets, *m_manager, "", false, false, true, false, true);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
TEST_F(VersionCompareTestFixture, DeleteLeafNestedPropertyPaths)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    // Test a nested property path as follows
    // TestElement -> PhysicalElementIsOfType -> TestPhysicalType -> ElementOwnsUniqueAspect -> TestUniqueAspect

    // Create a physical type
    TestPhysicalTypePtr testType = CreatePhysicalType("TestTypeElementDelete2", "Value");
    ASSERT_TRUE(testType.IsValid());
    // Set aspect to the type
    TestUniqueAspectPtr aspect = TestUniqueAspect::Create("Initial Value");
    DgnElement::UniqueAspect::SetAspect(*testType, *aspect);
    aspect->SetPropertyValue("TestUniqueAspectProperty", ECValue("Old Value for Property"));
    testType->Update();
    // Create physical elements with given type
    DgnElementPtr element = InsertPhysicalElement("ElementWithType", testType);
    DgnElementPtr element2 = InsertPhysicalElement("ElementWithType2", testType);

    // Create baseline revision
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // Changeset 1 - Delete aspect related to type
    ECClassCP aspectClassUnique = TestUniqueAspect::GetECClass(*m_db);
    ASSERT_NE(aspectClassUnique, nullptr);
    TestPhysicalTypePtr typeForEdit = m_db->Elements().GetForEdit<TestPhysicalType>(testType->GetElementId());
    ECN::ECClassCR aclass = *TestUniqueAspect::GetECClass(*m_db);
    TestUniqueAspectP aspectP = DgnElement::UniqueAspect::GetP<TestUniqueAspect>(*typeForEdit, aclass);
    aspectP->Delete();
    m_db->Elements().Update(*typeForEdit);
    bvector<Utf8String> props;
    props.push_back("TestUniqueAspectProperty");
    elementMap[typeForEdit->GetElementId()] = ElementData(typeForEdit, DbOpcode::Update, ElementChangesType::Type::Mask_Property | ElementChangesType::Type::Mask_Indirect, props);
    // Elements should show the changed property with property change and indirect change from the deleted aspect
    elementMap[element->GetElementId()] = ElementData(element, DbOpcode::Update, ElementChangesType::Type::Mask_Property | ElementChangesType::Type::Mask_Indirect, props);
    elementMap[element2->GetElementId()] = ElementData(element2, DbOpcode::Update, ElementChangesType::Type::Mask_Property | ElementChangesType::Type::Mask_Indirect, props);
    // Create changeset
    changesets.push_back(CreateRevision());

    // Test that the output matches with the input rolling forward from old state
    CreateSummaryAndCheckOutput(targetDb, elementMap, changesets, *m_manager, "", false, false, true, true);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
TEST_F(VersionCompareTestFixture, DeleteIntermediaryNestedPropertyPaths)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    // Test a nested property path as follows
    // TestElement -> PhysicalElementIsOfType -> TestPhysicalType -> ElementOwnsUniqueAspect -> TestUniqueAspect

    // Create a physical type
    TestPhysicalTypePtr testType = CreatePhysicalType("TestTypeElement3", "Value");
    ASSERT_TRUE(testType.IsValid());
    // Set aspect to the type
    TestUniqueAspectPtr aspect = TestUniqueAspect::Create("Initial Value");
    DgnElement::UniqueAspect::SetAspect(*testType, *aspect);
    aspect->SetPropertyValue("TestUniqueAspectProperty", ECValue("Old Value for Property"));
    testType->Update();
    // Create physical elements with given type
    DgnElementPtr element = InsertPhysicalElement("ElementWithType", testType);
    DgnElementPtr element2 = InsertPhysicalElement("ElementWithType2", testType);

    // Create baseline revision
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // Changeset 1 - Delete type that has the link to the aspect
    TestPhysicalTypePtr typeForEdit = m_db->Elements().GetForEdit<TestPhysicalType>(testType->GetElementId());
    m_db->Elements().Delete(*typeForEdit);
    bvector<Utf8String> props;
    // Aspect property that will no longer be shown in the element
    props.push_back("TestUniqueAspectProperty");
    // Also add properties that are in the type that will be found in the delete by other property paths
    props.push_back("StringProperty");
    props.push_back("CodeValue");
    elementMap[typeForEdit->GetElementId()] = ElementData(typeForEdit, DbOpcode::Delete);
    // Elements should show the changed property with property change and indirect change from the deleted type that contained the aspect
    elementMap[element->GetElementId()] = ElementData(element, DbOpcode::Update, ElementChangesType::Type::Mask_Property | ElementChangesType::Type::Mask_Indirect | ElementChangesType::Type::Mask_Hidden, props);
    elementMap[element2->GetElementId()] = ElementData(element2, DbOpcode::Update, ElementChangesType::Type::Mask_Property | ElementChangesType::Type::Mask_Indirect | ElementChangesType::Type::Mask_Hidden, props);
    // Create changeset
    changesets.push_back(CreateRevision());

    // Test that the output matches with the input rolling forward from old state
    CreateSummaryAndCheckOutput(targetDb, elementMap, changesets, *m_manager, "", false, false, true, true);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
TEST_F(VersionCompareTestFixture, NestedPropertyPathsDeleteElement)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    // Test a nested property path as follows
    // TestElement -> PhysicalElementIsOfType -> TestPhysicalType -> ElementOwnsUniqueAspect -> TestUniqueAspect

    // Create a physical type
    TestPhysicalTypePtr testType = CreatePhysicalType("TestTypeElement4", "Value");
    ASSERT_TRUE(testType.IsValid());
    // Set aspect to the type
    TestUniqueAspectPtr aspect = TestUniqueAspect::Create("Initial Value");
    DgnElement::UniqueAspect::SetAspect(*testType, *aspect);
    aspect->SetPropertyValue("TestUniqueAspectProperty", ECValue("Old Value for Property"));
    testType->Update();
    // Create physical elements with given type
    DgnElementPtr element = InsertPhysicalElement("ElementWithType", testType);
    DgnElementPtr element2 = InsertPhysicalElement("ElementWithType2", testType);

    // Create baseline revision
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // Changeset 1 - Change property of the aspect and delete one of the elements
    ECClassCP aspectClassUnique = TestUniqueAspect::GetECClass(*m_db);
    ASSERT_NE(aspectClassUnique, nullptr);
    TestPhysicalTypePtr typeForEdit = m_db->Elements().GetForEdit<TestPhysicalType>(testType->GetElementId());
    SetUniqueAspectPropertyValue(*typeForEdit, *aspectClassUnique, "TestUniqueAspectProperty", "New Value for Property");
    typeForEdit->Update();
    bvector<Utf8String> props;
    props.push_back("TestUniqueAspectProperty");
    elementMap[typeForEdit->GetElementId()] = ElementData(typeForEdit, DbOpcode::Update, ElementChangesType::Type::Mask_Property | ElementChangesType::Type::Mask_Indirect, props);
    // Element should show the changed property with property change and indirect change
    elementMap[element->GetElementId()] = ElementData(element, DbOpcode::Update, ElementChangesType::Type::Mask_Property | ElementChangesType::Type::Mask_Indirect, props);
    // Ensure deletes with changed related properties result in a delete regardless
    elementMap[element2->GetElementId()] = ElementData(element2, DbOpcode::Delete);
    m_db->Elements().Delete(element2->GetElementId());
    // Create changeset
    changesets.push_back(CreateRevision());

    // Test that the output matches with the input rolling forward
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager, "", false, false, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionCompareTestFixture, TestBriefcaseRolling)
    {
    // Test that we can roll the passed briefcase
    // This will be used for the agent to avoid doing roll + cloning of the database
    // So that the agent may roll and process in a single operation without the processing code
    // needing to do a temporary cloning to process the change without affecting the passed Db
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    // Clone with the current revision
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // CHANGESET 1
    // Insert an element
    DgnElementPtr firstElement = InsertPhysicalElement("X");
    elementMap[firstElement->GetElementId()] = ElementData(firstElement, DbOpcode::Insert);
    DgnRevisionPtr changeset = CreateRevision();
    changesets.push_back(changeset);

    BeFileName dbFilename = targetDb->GetFileName();

    // Should be in older state
    ASSERT_TRUE(targetDb->Revisions().GetParentRevisionId().Equals(changeset->GetParentId()));
    // Test that the output matches with the input rolling forward (need target state to do cloning as an option)
    CreateSummaryAndCheckOutput(targetDb, elementMap, changesets, *m_manager, "", false, false, false, true, false);
    // Should be in older state since briefcase roll option was false
    ASSERT_TRUE(targetDb->Revisions().GetParentRevisionId().Equals(changeset->GetParentId()));
    // Now try with briefcase rolling
    CreateSummaryAndCheckOutput(targetDb, elementMap, changesets, *m_manager, "", false, false, false, false, true);
    // Ensure target Db file got rolled properly
    BeSQLite::DbResult result;
    DgnDb::OpenParams params (Db::OpenMode::ReadWrite);
    DgnDbPtr updatedTargetDb = DgnDb::OpenDgnDb(&result, dbFilename, params);
    ASSERT_TRUE(updatedTargetDb->Revisions().GetParentRevisionId().Equals(changeset->GetChangesetId()));
    }

// Only useful for manual testing, takes too long for regular unit tests
// #define TEST_MEM_USAGE
#ifdef TEST_MEM_USAGE
//-------------------------------------------------------------------------------------------
// @bsimethod
//-------------------------------------------------------------------------------------------
TEST_F(VersionCompareTestFixture, MemoryUsageTest)
    {
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

    // INITIAL CHANGESET
    // Create elements and aspects
    size_t numMaxElements = 2000000;
    size_t numElements = numMaxElements / 2;
    bvector<DgnElementId> elementIds;
    for (size_t i = 0; i < numElements; ++i)
        {
        TestElementPtr indirectEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "IndirectAspectEl");
        TestUniqueAspectPtr aspect = TestUniqueAspect::Create("Initial Value");
        aspect->SetPropertyValue("TestUniqueAspectProperty", ECValue("Old Value for Property"));
        DgnElement::UniqueAspect::SetAspect(*indirectEl, *aspect);
        m_db->Elements().Insert(*indirectEl);
        // Set aspect to one element
        elementIds.push_back(indirectEl->GetElementId());
        }

    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // CHANGESET 1
    // Modify all aspects
    bvector<Utf8String> props;
    props.push_back("TestUniqueAspectProperty");
    ECClassCP aspectClassUnique = TestUniqueAspect::GetECClass(*m_db);
    ASSERT_NE(aspectClassUnique, nullptr);
    for (auto const& elementId : elementIds)
        {
        TestElementPtr element = m_db->Elements().GetForEdit<TestElement>(elementId);
        SetUniqueAspectPropertyValue(*element, *aspectClassUnique, "TestUniqueAspectProperty", "New Value for Property");
        element->Update();
        elementMap[element->GetElementId()] = ElementData(element, DbOpcode::Update, ElementChangesType::Type::Mask_Indirect | ElementChangesType::Type::Mask_Property, props);
        }
    // Create changeset
    changesets.push_back(CreateRevision());

    // Test output
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager);
    }
#endif