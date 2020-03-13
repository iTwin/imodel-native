/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
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
#include <ECPresentation/IECPresentationManager.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>

USING_NAMESPACE_BENTLEY_DPTEST
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_ECPRESENTATION

//=======================================================================================
// @bsistruct                                                   Diego.Pinate    09/17
//=======================================================================================
struct VersionCompareTestFixture : public ::testing::Test
{
public:
    static ScopedDgnHost*       s_host;

    static ECPresentation::RulesDrivenECPresentationManager* m_manager;
    static DgnDbPtr            m_db;
    static PhysicalModelPtr    m_defaultModel;
    static DgnModelId          m_defaultModelId;
    static DgnModelId          m_defaultDrawingModelId;
    static DgnCategoryId       m_defaultCategoryId;
    static DgnCategoryId       m_defaultDrawingCategoryId;
    static Utf8String          m_rulesetDir;

    bool ModifyElement(DgnElementId elementId);
    DgnElementPtr InsertPhysicalElement(Utf8String codeName);
    DgnElementPtr InsertPhysicalElement(Utf8String codeName, DPoint3dCR center, DPoint3dCR size);

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
RulesDrivenECPresentationManager*   VersionCompareTestFixture::m_manager        = nullptr;
DgnDbPtr                            VersionCompareTestFixture::m_db             = nullptr;
PhysicalModelPtr                    VersionCompareTestFixture::m_defaultModel   = nullptr;
DgnModelId                          VersionCompareTestFixture::m_defaultModelId;
DgnModelId                          VersionCompareTestFixture::m_defaultDrawingModelId;
DgnCategoryId                       VersionCompareTestFixture::m_defaultCategoryId;
DgnCategoryId                       VersionCompareTestFixture::m_defaultDrawingCategoryId;
Utf8String                          VersionCompareTestFixture::m_rulesetDir;

static BeFileName                   s_changesetDir;

//#define DUMP_SUMMARY_OUTPUT
#define DUMP_REVISION
#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger (L"DgnCore"))

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    12/18
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
* @bsimethod                                                    Diego.Pinate    09/17
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
    createParams.SetDbType(BeSQLite::Db::CreateParams::DbType::Standalone);
    m_db = DgnDb::CreateDgnDb(&result, dgndbPath, createParams);
    ASSERT_TRUE(m_db.IsValid());
    ASSERT_TRUE(result == BeSQLite::DbResult::BE_SQLITE_OK);

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
* @bsimethod                                                    Diego.Pinate    09/17
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
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
void VersionCompareTestFixture::SetUp()
    {
    BeFileName sqlang;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(sqlang);
    sqlang.AppendToPath(L"PresentationRules");
    sqlang.AppendToPath(L"RulesEngineLocalizedStrings.sqlang.db3");

    L10N::Initialize(L10N::SqlangFiles(sqlang));

    // Setup required BeSQLite initialization for ECPresentation
    BeFileName rulesetsDir;
    BeFileName tempDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(rulesetsDir);
    BeTest::GetHost().GetTempDir(tempDir);
    BeSQLiteLib::Initialize(tempDir);

    // Initialize RulesDrivenECPresentationManager and connection
    IECPresentationManager::SetSerializer(new ECPresentation::DefaultECPresentationSerializer());
    RulesDrivenECPresentationManager::Params params(RulesDrivenECPresentationManager::Paths(rulesetsDir, tempDir));
    params.SetLocalizationProvider(new ECPresentation::SQLangLocalizationProvider());
    m_manager = new RulesDrivenECPresentationManager(params);
    m_rulesetDir = rulesetsDir.GetNameUtf8().c_str();

    // Add presentation rules
    rulesetsDir.AppendToPath(L"PresentationRules");
    RuleSetLocaterPtr locater = DirectoryRuleSetLocater::Create(rulesetsDir.GetNameUtf8().c_str());
    m_manager->GetLocaters().RegisterLocater(*locater);

    // Notify we have a connection to a db
    m_manager->GetConnections().CreateConnection(*m_db);

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
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
void VersionCompareTestFixture::TearDownTestCase()
    {
    m_db->AbandonChanges();
    m_db = nullptr;
    m_defaultModel = nullptr;
    delete s_host;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
void VersionCompareTestFixture::TearDown()
    {
    delete m_manager;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool VersionCompareTestFixture::ModifyElement(DgnElementId elementId)
    {
    RefCountedPtr<PhysicalElement> testElement = m_db->Elements().GetForEdit<PhysicalElement>(elementId);
    if (!testElement.IsValid())
        return false;

    Placement3d newPlacement = testElement->GetPlacement();
    newPlacement.GetOriginR().x += 1.0;

    testElement->SetPlacement(newPlacement);

    DgnDbStatus dbStatus;
    testElement->Update(&dbStatus);
    return (dbStatus == DgnDbStatus::Success);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    08/19
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr   VersionCompareTestFixture::InsertPhysicalElement(Utf8String codeName, DPoint3dCR center, DPoint3dCR size)
    {
    PhysicalModelR model = *m_defaultModel->ToPhysicalModelP();
    GenericPhysicalObjectPtr physicalElementPtr = GenericPhysicalObject::Create(model, m_defaultCategoryId);
    auto codeSpec = CodeSpec::Create(*m_db, "MyCodeSpec");
    physicalElementPtr->SetCode(codeSpec->CreateCode(codeName));

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
* @bsimethod                                                    Diego.Pinate    08/19
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr   VersionCompareTestFixture::InsertPhysicalElement(Utf8String codeName)
    {
    DPoint3d blockSizeRange = DPoint3d::From(2.0, 2.0, 2.0);
    DPoint3d center = DPoint3d::FromZero();
    return VersionCompareTestFixture::InsertPhysicalElement(codeName, center, blockSizeRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
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
    m_manager->GetConnections().CreateConnection(*clonedDb);
    return clonedDb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnRevisionPtr VersionCompareTestFixture::CreateRevision()
    {
    m_db->SaveChanges();

    DgnRevisionPtr revision = m_db->Revisions().StartCreateRevision();
    EXPECT_TRUE(revision.IsValid());
    if (!revision.IsValid())
        return nullptr;

    RevisionStatus status = m_db->Revisions().FinishCreateRevision();
    EXPECT_TRUE(status == RevisionStatus::Success);
    if (RevisionStatus::Success != status)
        return nullptr;

    return revision;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    07/17
+---------------+---------------+---------------+---------------+---------------+------*/
void VersionCompareTestFixture::SetUniqueAspectPropertyValue(DgnElementR element, ECN::ECClassCR aspectClass, Utf8CP propertyName, Utf8CP propertyValue)
    {
    DgnElement::UniqueAspect* aspect = DgnElement::UniqueAspect::GetAspectP(element, aspectClass);
    ASSERT_NE(aspect, nullptr);
    ASSERT_EQ(DgnDbStatus::Success, aspect->SetPropertyValue(propertyName, ECN::ECValue(propertyValue)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    07/17
+---------------+---------------+---------------+---------------+---------------+------*/
void VersionCompareTestFixture::SetUniqueAspectPropertyValueInt(DgnElementR element, ECN::ECClassCR aspectClass, Utf8CP propertyName, int propertyValue)
    {
    DgnElement::UniqueAspect* aspect = DgnElement::UniqueAspect::GetAspectP(element, aspectClass);
    ASSERT_NE(aspect, nullptr);
    ASSERT_EQ(DgnDbStatus::Success, aspect->SetPropertyValue(propertyName, ECN::ECValue(propertyValue)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    07/17
+---------------+---------------+---------------+---------------+---------------+------*/
void VersionCompareTestFixture::SetMultiAspectPropertyValue(DgnElementR element, ECN::ECClassCR aspectClass, ECInstanceId aspectInstanceId, Utf8CP propertyName, Utf8CP propertyValue)
    {
    DgnElement::MultiAspect* aspect = DgnElement::MultiAspect::GetAspectP(element, aspectClass, aspectInstanceId);
    ASSERT_NE(aspect, nullptr);
    ASSERT_EQ(DgnDbStatus::Success, aspect->SetPropertyValue(propertyName, ECN::ECValue(propertyValue)));
    }

//=======================================================================================
// Helper to check the output of version compare change summaries
// @bsistruct                                                   Diego.Pinate    09/17
//=======================================================================================
struct ElementData
    {
    DbOpcode        m_opcode;
    ECClassId       m_classId;
    DgnModelId      m_modelId;
    AxisAlignedBox3d m_bbox;

    ElementData(DgnElementPtr element, DbOpcode opcode) : m_opcode(opcode)
        {
        m_classId = element->GetElementClass()->GetId();
        m_modelId = element->GetModelId();
        GeometrySource3dCP source = element->ToGeometrySource3d();
        m_bbox = nullptr != source ? source->CalculateRange3d() : AxisAlignedBox3d();
        }

    ElementData(ECClassId classId, DbOpcode opcode, DgnModelId modelId, AxisAlignedBox3d bbox) : m_opcode(opcode), m_classId(classId), m_modelId(modelId), m_bbox(bbox) { }
    ElementData() : m_opcode((DbOpcode)0) { m_classId.Invalidate(); }

    bool IsValid()
        {
        return m_classId.IsValid() && m_opcode != (DbOpcode)0;
        }
    }; // ElementData

typedef bmap<DgnElementId, ElementData> ElementMap;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
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
    Utf8PrintfString dataEntry("ID: %d \t\t ECClass: %s \t\t Opcode: %s\n", elementId.GetValue(), ecclassName.c_str(), opcodeStr.c_str());
    printf("%s\n", dataEntry.c_str());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
void CheckOutput(DgnDbR db, ElementMap & map, bvector<DgnElementId> const& elementIds, bvector<ECClassId> const& ecclassIds, bvector<DbOpcode> const& opcodes, bvector<DgnModelId> const& modelIds, bvector<AxisAlignedBox3d> const& bboxes)
    {
    EXPECT_EQ(elementIds.size(), map.size());

    // Checks the results of comparison against a test-made map
    for (int i = 0; i < elementIds.size(); ++i)
        {
        DgnElementId currentId      = elementIds[i];
        ECClassId currentECClassId  = ecclassIds[i];
        DbOpcode currentOpcode      = opcodes[i];
        DgnModelId modelId          = modelIds[i];
        AxisAlignedBox3d boundBox   = bboxes[i];

        PrintFoundRecord(db, currentId, currentECClassId, currentOpcode);

        EXPECT_TRUE(map.find(currentId) != map.end());
        if (map.find(currentId) == map.end())
            continue;

        ElementData & currentElemData = map[currentId];
        EXPECT_EQ(currentElemData.IsValid(), true);
        if (!currentElemData.IsValid())
            continue;

        EXPECT_EQ(currentElemData.m_opcode, currentOpcode);
        EXPECT_EQ(currentElemData.m_classId, currentECClassId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
void CreateSummaryAndCheckOutput(DgnDbPtr db, ElementMap& map, bvector<DgnRevisionPtr>& changesets, IECPresentationManagerR presentationManager, bool backwards = true, Utf8String debugLabel = "", bool filterSpatial = false, bool filterLastMod = false)
    {
    bvector<DgnElementId> elementIds;
    bvector<ECClassId> ecclassIds;
    bvector<DbOpcode> opcodes;
    bvector<DgnModelId> modelIds;
    bvector<AxisAlignedBox3d> bboxes;
    StatusInt status = SUCCESS;

    clock_t t0 = clock();
    VersionCompareChangeSummaryPtr changeSummary = VersionCompareChangeSummary::Generate (*db, changesets, presentationManager, "Items", backwards, filterSpatial, filterLastMod, BeFileName());
    clock_t t1 = clock();
    double elapsed = (t1-t0)/(double)CLOCKS_PER_SEC;
    if (!Utf8String::IsNullOrEmpty(debugLabel.c_str()))
        printf("%s - Generate: %lf seconds\n", debugLabel.c_str(), elapsed);

    status = changeSummary->GetChangedElements(elementIds, ecclassIds, opcodes, modelIds, bboxes);
    EXPECT_EQ(SUCCESS, status);

    // Compare desired output with actual output
    CheckOutput(*db, map, elementIds, ecclassIds, opcodes, modelIds, bboxes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
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
    CreateSummaryAndCheckOutput(targetDb, elementMap, changesets, *m_manager, false);
    // Test that the output matches with the input rolling backwards
    std::reverse(changesets.begin(), changesets.end());
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
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
    CreateSummaryAndCheckOutput(targetDb, elementMap, changesets, *m_manager, false);
    // Test that the output matches with the input rolling backwards
    std::reverse(changesets.begin(), changesets.end());
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
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
    elementMap[firstElement->GetElementId()] = ElementData(firstElement, DbOpcode::Update);
    ModifyElement(firstElement->GetElementId());
    changesets.push_back(CreateRevision());

    // Test that the output matches with the input rolling forward
    CreateSummaryAndCheckOutput(targetDb, elementMap, changesets, *m_manager, false);
    // Test that the output matches with the input rolling backwards
    std::reverse(changesets.begin(), changesets.end());
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
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
    CreateSummaryAndCheckOutput(targetDb, elementMap, changesets, *m_manager, false);
    // Test that the output matches with the input rolling backwards
    std::reverse(changesets.begin(), changesets.end());
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    11/18
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
    CreateSummaryAndCheckOutput(targetDb, elementMap, changesets, *m_manager, false, "", true);
    // Test that the output matches with the input rolling backwards
    std::reverse(changesets.begin(), changesets.end());
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager, true, "", true);

    // Test without filtering out the 2D element now
    elementMap[tempEl2d->GetElementId()] = ElementData(tempEl2d, DbOpcode::Insert);
    // Test that the output matches with the input rolling forward
    CreateSummaryAndCheckOutput(targetDb, elementMap, changesets, *m_manager, false, "", false);
    // Test that the output matches with the input rolling backwards
    std::reverse(changesets.begin(), changesets.end());
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager, true, "", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
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
    CreateSummaryAndCheckOutput(targetDb, elementMap, changesets, *m_manager, false);
    // Test that the output matches with the input rolling backwards
    std::reverse(changesets.begin(), changesets.end());
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
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
    CreateSummaryAndCheckOutput(targetDb, elementMap, changesets, *m_manager, false);
    // Test that the output matches with the input rolling backwards
    std::reverse(changesets.begin(), changesets.end());
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
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
    // Modify the first element and add schema change
    ModifyElement(firstElement->GetElementId());
    elementMap[firstElement->GetElementId()] = ElementData(firstElement, DbOpcode::Update);
    m_db->CreateTable("TestTable5", "Id INTEGER PRIMARY KEY, Column1 INTEGER");
    changesets.push_back(CreateRevision());

    // CHANGESET 2
    // Delete second element
    elementMap[secondElement->GetElementId()] = ElementData(secondElement, DbOpcode::Delete);
    m_db->Elements().Delete(secondElement->GetElementId());
    changesets.push_back(CreateRevision());

    // CHANGESET 3
    // Insert a new element and modify first again
    DgnElementPtr fourthElement = InsertPhysicalElement("A");
    elementMap[fourthElement->GetElementId()] = ElementData(fourthElement, DbOpcode::Insert);
    ModifyElement(firstElement->GetElementId());
    changesets.push_back(CreateRevision());

    // CHANGESET 4
    // Modify fourth element (should still show up as an inserted element, so don't do anything to the element map)
    ModifyElement(fourthElement->GetElementId());
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
    CreateSummaryAndCheckOutput(targetDb, elementMap, changesets, *m_manager, false);
    // Test that the output matches with the input rolling backwards
    std::reverse(changesets.begin(), changesets.end());
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
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
    CreateSummaryAndCheckOutput(targetDb, elementMap, changesets, *m_manager, false);
    // Test that the output matches with the input rolling backwards
    std::reverse(changesets.begin(), changesets.end());
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
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
    elementMap[tempEl->GetElementId()] = ElementData(tempEl, DbOpcode::Update);
    DgnRevisionPtr changeset1 = CreateRevision();
    DumpRevision(*changeset1, "CompareAspectChange2: Aspect modification");
    changesets.push_back(changeset1);

    // Test that the output matches with the input rolling forward
    CreateSummaryAndCheckOutput(targetDb, elementMap, changesets, *m_manager, false);
    // Test that the output matches with the input rolling backwards
    std::reverse(changesets.begin(), changesets.end());
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
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
    CreateSummaryAndCheckOutput(targetDb, elementMap, changesets, *m_manager, false);
    // Test that the output matches with the input rolling backwards
    std::reverse(changesets.begin(), changesets.end());
    CreateSummaryAndCheckOutput(m_db, elementMap, changesets, *m_manager, true);
    }

//--------------------------------------------------------------------------------------
// @bsistruct                                                   Diego.Pinate    10/17
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
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
void AddEntry(ElementInputPropertyMap& map, DgnElementId elementId, Utf8String accessor, ECValue oldVal, ECValue newVal, Utf8String prefix = "")
    {
    if (map.find(elementId) == map.end())
        map[elementId] = bvector<PropertyData>();

    PropertyData data(accessor, prefix, oldVal, newVal);
    map[elementId].push_back(data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
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
// @bsimethod                                                 Diego.Pinate     10/17
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

#ifdef OLD_WAY__
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
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
            // Got to add TestElement_ prefix as pres rules append the class name like so in content
            Utf8String fixedAccessor = data.m_accessorPrefix + data.m_accessor;
            Json::Value currentValue = outputContent["ContentSet"][0]["Values"][fixedAccessor]["Current"];
            Json::Value targetValue = outputContent["ContentSet"][0]["Values"][fixedAccessor]["Target"];
            ASSERT_TRUE(!currentValue.isNull() && !targetValue.isNull());
            TestEqual(currentValue, data.m_oldValue);
            TestEqual(targetValue, data.m_newValue);
            }
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
void CreateSummaryAndCheckPropertyOutput(DgnDbPtr db, ElementInputPropertyMap& inputMap, bvector<DgnRevisionPtr>& changesets, IECPresentationManagerR presentationManager, bool backwards = true, bool filterSpatial = false, bool filterLastMod = false)
    {
    bvector<DgnElementId> elementIds;
    bvector<ECClassId> ecclassIds;
    bvector<DbOpcode> opcodes;
    bvector<DgnModelId> modelIds;
    bvector<AxisAlignedBox3d> bboxes;
    StatusInt status = SUCCESS;
    VersionCompareChangeSummaryPtr changeSummary = VersionCompareChangeSummary::Generate (*db, changesets, presentationManager, "Items", backwards, filterSpatial, filterLastMod, BeFileName());

    status = changeSummary->GetChangedElements(elementIds, ecclassIds, opcodes, modelIds, bboxes);
    EXPECT_EQ(SUCCESS, status);

    // Swap input map's new/old values
    if (backwards)
        {
        for (auto& mapping : inputMap)
            {
            for (auto& inputEntry : mapping.second)
                {
                ECValue temp = inputEntry.m_newValue;
                inputEntry.m_newValue = inputEntry.m_oldValue;
                inputEntry.m_oldValue = temp;
                }
            }
        }

    ElementOutputPropertyMap outputMap;
    for (int i = 0; i < elementIds.size(); ++i)
        {
        Json::Value content;
#ifdef OLD_WAY__
        status = changeSummary->GetPropertyContentComparison(elementIds[i], ecclassIds[i], false, content);
#endif
        status = changeSummary->GetPropertyComparison(elementIds[i], ecclassIds[i], true, content);
        EXPECT_EQ(SUCCESS, status);
        outputMap[elementIds[i]] = content;
        }

    // Compare desired output with actual output
    CheckPropertyOutput(inputMap, outputMap);
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionCompareTestFixture, PropertyTest1)
    {
    bvector<DgnRevisionPtr> changesets;
    ElementInputPropertyMap inputMap;

    // INITIAL CHANGESET
    // Create the starting changeset containing an element
    TestElementPtr tempEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "PropertyTest1_Element1");
    TestElementPtr tempEl2 = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "PropertyTest1_Element2");
    ASSERT_EQ(tempEl->SetPropertyValue(DPTEST_TEST_ELEMENT_IntegerProperty1, ECValue(0)), DgnDbStatus::Success); 
    ASSERT_EQ(tempEl2->SetPropertyValue(DPTEST_TEST_ELEMENT_DoubleProperty1, ECValue(0.3)), DgnDbStatus::Success);
    m_db->Elements().Insert(*tempEl);
    m_db->Elements().Insert(*tempEl2);
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // CHANGESET 1
    // Modify the integer property
    ASSERT_EQ(tempEl->SetPropertyValue(DPTEST_TEST_ELEMENT_IntegerProperty1, ECValue(2)), DgnDbStatus::Success);
    tempEl->Update();
    AddEntry(inputMap, tempEl->GetElementId(), DPTEST_TEST_ELEMENT_IntegerProperty1, ECValue(0), ECValue(2));
    DgnRevisionPtr changeset1 = CreateRevision();
    DumpRevision(*changeset1, "PropertyTest1: Element property modification");
    changesets.push_back(changeset1);

    // CHANGESET 2
    // Modify another element with a double property
    ASSERT_EQ(tempEl2->SetPropertyValue(DPTEST_TEST_ELEMENT_DoubleProperty1, ECValue(23.9)), DgnDbStatus::Success);
    tempEl2->Update();
    AddEntry(inputMap, tempEl2->GetElementId(), DPTEST_TEST_ELEMENT_DoubleProperty1, ECValue(0.3), ECValue(23.9));
    DgnRevisionPtr changeset2 = CreateRevision();
    DumpRevision(*changeset2, "PropertyTest1: Element 2 property modification");
    changesets.push_back(changeset2);

    // Test property comparison going forwards
    CreateSummaryAndCheckPropertyOutput(targetDb, inputMap, changesets, *m_manager, false);
    // Test property comparison going backwards
    std::reverse(changesets.begin(), changesets.end());
    CreateSummaryAndCheckPropertyOutput(m_db, inputMap, changesets, *m_manager, true);
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                                    Diego.Pinate    09/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionCompareTestFixture, PropertyTest2)
    {
    ElementInputPropertyMap inputMap;
    bvector<DgnRevisionPtr> changesets;

    // INITIAL CHANGESET
    // Create the starting changeset containing an element with an aspect
    TestElementPtr tempEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "PropertyTest2_Element1");
    TestUniqueAspectPtr aspect = TestUniqueAspect::Create("Aspect");
    DgnElement::UniqueAspect::SetAspect(*tempEl, *aspect);
    aspect->SetPropertyValue("TestUniqueAspectProperty", ECValue("This is the old value"));
    m_db->Elements().Insert(*tempEl);
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // CHANGESET 1
    // Modify the aspect, we should get a modification of the element
    ECClassCP aspectClassUnique = TestUniqueAspect::GetECClass(*m_db);
    ASSERT_NE(aspectClassUnique, nullptr);
    SetUniqueAspectPropertyValue(*tempEl, *aspectClassUnique, "TestUniqueAspectProperty", "This is the new value");
    tempEl->Update();
    AddEntry(inputMap, tempEl->GetElementId(), "TestUniqueAspect -> TestUniqueAspectProperty", ECValue("This is the old value"), ECValue("This is the new value"));
    DgnRevisionPtr changeset1 = CreateRevision();
    DumpRevision(*changeset1, "CompareAspectChange2: Aspect modification");
    changesets.push_back(changeset1);

    // Test property comparison going forwards
    CreateSummaryAndCheckPropertyOutput(targetDb, inputMap, changesets, *m_manager, false);
    // Test property comparison going backwards
    std::reverse(changesets.begin(), changesets.end());
    CreateSummaryAndCheckPropertyOutput(m_db, inputMap, changesets, *m_manager, true);
    }

#ifdef WIP_
/*---------------------------------------------------------------------------------**//**
* TODO: Need a way to set LastMod property since it is read-only from the handler
* @bsimethod                                                    Diego.Pinate    11/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionCompareTestFixture, TestFilterLastMod)
    {
    bvector<DgnRevisionPtr> changesets;
    ElementInputPropertyMap inputMap;

    ECValue time1;
    // INITIAL CHANGESET
    // Create the starting changeset containing an element
    TestElementPtr tempEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "PropertyTest1_Element1");
    m_db->Elements().Insert(*tempEl);
    tempEl->GetPropertyValue(time1, "LastMod");
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());


    DateTime time2 = DateTime::GetCurrentTime();
    // CHANGESET 1
    // Modify the last mod property
    ASSERT_EQ(tempEl->SetPropertyValue("LastMod", ECValue(time2)), DgnDbStatus::Success);
    DgnRevisionPtr changeset1 = CreateRevision();
    DumpRevision(*changeset1, "TestFilterLastMod: Element property modification");
    changesets.push_back(changeset1);

    // Test property comparison going forwards and filtering last modified
    CreateSummaryAndCheckPropertyOutput(targetDb, inputMap, changesets, *m_manager, false, false, true);
    // Test property comparison going backwards and filtering last modified
    std::reverse(changesets.begin(), changesets.end());
    CreateSummaryAndCheckPropertyOutput(m_db, inputMap, changesets, *m_manager, true, false, true);

    // Now test without filtering
    AddEntry(inputMap, tempEl->GetElementId(), "LastMod", time1, ECValue(time2));
    // Test property comparison going forwards and filtering last modified
    CreateSummaryAndCheckPropertyOutput(targetDb, inputMap, changesets, *m_manager, false, false, false);
    // Test property comparison going backwards and filtering last modified
    std::reverse(changesets.begin(), changesets.end());
    CreateSummaryAndCheckPropertyOutput(m_db, inputMap, changesets, *m_manager, true, false, false);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     10/17
//-------------------------------------------------------------------------------------------
void CreateSummaryAndCheckModels(DgnDbPtr db, bmap<DgnModelId,DbOpcode>& map, bvector<DgnRevisionPtr>& changesets, bool backwards = true)
    {
    bvector<DgnModelId> modelIds;
    bvector<DbOpcode> opcodes;
    StatusInt status = SUCCESS;
    VersionCompareChangeSummaryPtr changeSummary = VersionCompareChangeSummary::Generate (*db, changesets, "Items", backwards);
    
    status = changeSummary->GetChangedModels(modelIds, opcodes);
    EXPECT_EQ(SUCCESS, status);

    // We must have the same number of changed models as the input
    EXPECT_EQ(map.size(), modelIds.size());

    // Check IDs and opcodes
    for (int i = 0; i < modelIds.size(); ++i)
        {
        DgnModelId currentId = modelIds[i];
        EXPECT_TRUE(map.find(currentId) != map.end());
        EXPECT_EQ(map[currentId], opcodes[i]);
        }
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     10/17
//-------------------------------------------------------------------------------------------
TEST_F(VersionCompareTestFixture, ChangedModelsTest)
    {
    bvector<DgnRevisionPtr> changesets;
    bmap<DgnModelId,DbOpcode> modelIdMap;

    // SETUP
    // Insert a physical model to delete later
    PhysicalModelCPtr modelToDelete = DgnDbTestUtils::InsertPhysicalModel(*m_db, "ChangedModelsTest_ModelToDelete");
    DgnModelId modelToDelId = modelToDelete->GetModelId();
    DgnRevisionPtr initialRevision = CreateRevision();
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);

    // CHANGESET 1
    // Insert a model
    PhysicalModelCPtr newModel = DgnDbTestUtils::InsertPhysicalModel(*m_db, "ChangedModelsTest_Model1");
    DgnModelId modelId = newModel->GetModelId();
    modelIdMap[modelId] = DbOpcode::Insert;
    changesets.push_back(CreateRevision());

    // CHANGESET 2
    // Insert another model
    PhysicalModelCPtr newModel2 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "ChangedModelsTest_Model2");
    DgnModelId modelId2 = newModel2->GetModelId();
    modelIdMap[modelId] = DbOpcode::Insert;
    changesets.push_back(CreateRevision());

    // CHANGESET 3
    // Delete the model in initial revision
    // TODO

    // Test the model changes
    CreateSummaryAndCheckModels(targetDb, modelIdMap, changesets, false);
    // Test in reverse direction
    std::reverse(changesets.begin(), changesets.end());
    CreateSummaryAndCheckModels(m_db, modelIdMap, changesets, true);
    }
#endif

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     12/18
//-------------------------------------------------------------------------------------------
TEST_F(VersionCompareTestFixture, ChangedElementsManagerTest1)
    {
    // Test generating the changed elements ECDb based on a change summary
    ElementMap elementMap;
    bvector<DgnRevisionPtr> changesets;

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
    ModifyElement(secondElement->GetElementId());
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

    // Process backwards
    std::reverse(changesets.begin(), changesets.end());

    ChangedElementsManager ceMgr(m_db);
    ECDb cacheDb;
    // Create the Db file
    // TODO: Good filename
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.CreateChangedElementsCache(cacheDb, cacheFilename));
    // Process the changeset
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.ProcessChangesets(cacheDb, "Items", changesets));
    // Should be able to retrieve the changed elements from the cache Db
    ChangedElementsMap map;
    // Check first changeset 1 and 2 above
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedElements(cacheDb, map, changesets[2]->GetId(), changesets[1]->GetId()));
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
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedModels(cacheDb, changedModels, changesets[2]->GetId(), changesets[1]->GetId()));
    // Should have the default model in the changed models
    EXPECT_TRUE(changedModels.find(m_defaultModelId) != changedModels.end());

    // Now test getting changed elements for changeset 2 only
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedElements(cacheDb, map, changesets[1]->GetId(), changesets[1]->GetId()));
    EXPECT_EQ(2, map.size());
    // Should contain the third inserted element and the update to the second element
    EXPECT_TRUE(map.find(firstElement->GetECInstanceKey()) == map.end());
    EXPECT_FALSE(map.find(secondElement->GetECInstanceKey()) == map.end());
    EXPECT_FALSE(map.find(thirdElement->GetECInstanceKey()) == map.end());
    EXPECT_EQ(map[secondElement->GetECInstanceKey()].m_opcode, DbOpcode::Update);
    EXPECT_EQ(map[thirdElement->GetECInstanceKey()].m_opcode, DbOpcode::Insert);

    // Test we can get changed models for only one changeset
    changedModels.clear();
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedModels(cacheDb, changedModels, changesets[1]->GetId(), changesets[1]->GetId()));
    // Should have the default model in the changed models
    EXPECT_TRUE(changedModels.find(m_defaultModelId) != changedModels.end());

    // Now test getting full range of changed elements, first element should never appear due to accumulation
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedElements(cacheDb, map, changesets[2]->GetId(), changesets[0]->GetId()));
    EXPECT_EQ(2, map.size());
    // Should contain the elements that were inserted but not the one that got deleted
    EXPECT_FALSE(map.find(secondElement->GetECInstanceKey()) == map.end());
    EXPECT_FALSE(map.find(thirdElement->GetECInstanceKey()) == map.end());
    EXPECT_EQ(map[secondElement->GetECInstanceKey()].m_opcode, DbOpcode::Insert);
    EXPECT_EQ(map[thirdElement->GetECInstanceKey()].m_opcode, DbOpcode::Insert);

    // Test we can get changed models for all changesets
    changedModels.clear();
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedModels(cacheDb, changedModels, changesets[2]->GetId(), changesets[0]->GetId()));
    // Should have the default model in the changed models
    EXPECT_TRUE(changedModels.find(m_defaultModelId) != changedModels.end());

    // Close cache Db
    cacheDb.CloseDb();
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     12/18
//-------------------------------------------------------------------------------------------
TEST_F(VersionCompareTestFixture, ChangedElementsManagerTest2_ChangedModels)
    {
    // Test generating the changed elements ECDb based on a change summary
    bvector<DgnRevisionPtr> changesets;

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
    ModifyElement(secondElement->GetElementId());
    DgnRevisionPtr changeset2 = CreateRevision();
    changesets.push_back(changeset2);

    // Filename and cleanup if needed
    BeFileName cacheFilename;
    BeTest::GetHost().GetOutputRoot(cacheFilename);
    cacheFilename.AppendToPath(L"ChangedElements.chems");
    if (BeFileName::DoesPathExist(cacheFilename.GetName()))
        BeFileName::BeDeleteFile(cacheFilename.GetName());

    // Process backwards
    std::reverse(changesets.begin(), changesets.end());

    ChangedElementsManager ceMgr(m_db);
    ECDb cacheDb;
    // Create the Db file
    // TODO: Good filename
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.CreateChangedElementsCache(cacheDb, cacheFilename));
    // Process the changeset
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.ProcessChangesets(cacheDb, "Items", changesets));

    // Test we can get changed models for first changeset
    bmap<DgnModelId, AxisAlignedBox3d> changedModels;
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedModels(cacheDb, changedModels, changeset1->GetId(), changeset1->GetId()));
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
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedModels(cacheDb, changedModels, changeset1->GetId(), changeset2->GetId()));
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
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedModels(cacheDb, changedModels, changeset2->GetId(), changeset2->GetId()));
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
// @bsimethod                                                 Diego.Pinate     12/18
//-------------------------------------------------------------------------------------------
TEST_F(VersionCompareTestFixture, ChangedElementsManagerTest_PassRulesetDirectoryToManager)
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
    changesets.push_back(initialRevision);
    ASSERT_TRUE(initialRevision.IsValid());
    DgnDbPtr targetDb = CloneTemporaryDb(m_db);
    ASSERT_TRUE(targetDb.IsValid());

    // CHANGESET 1
    // Modify the aspect, we should get a modification of the element
    ECClassCP aspectClassUnique = TestUniqueAspect::GetECClass(*m_db);
    ASSERT_NE(aspectClassUnique, nullptr);
    SetUniqueAspectPropertyValue(*tempEl, *aspectClassUnique, "TestUniqueAspectProperty", "New Value for Property");
    tempEl->Update();
    elementMap[tempEl->GetElementId()] = ElementData(tempEl, DbOpcode::Update);
    
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

    ChangedElementsManager ceMgr(m_db);
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
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedElements(cacheDb, map, changesets[0]->GetId(), changesets[0]->GetId()));
    // Check size and data from retrieved map
    EXPECT_EQ(1, map.size());
    // Should contain the element whose aspect got modified
    EXPECT_FALSE(map.find(tempEl->GetECInstanceKey()) == map.end());
    EXPECT_EQ(map[tempEl->GetECInstanceKey()].m_opcode, DbOpcode::Update);
    // Test we have the correct model Ids
    EXPECT_EQ(elementMap[tempEl->GetElementId()].m_modelId, map[tempEl->GetECInstanceKey()].m_modelId);

    // Test we can get changed models
    bmap<DgnModelId, AxisAlignedBox3d> changedModels;
    EXPECT_EQ(BE_SQLITE_OK, ceMgr.GetChangedModels(cacheDb, changedModels, changesets[0]->GetId(), changesets[0]->GetId()));
    // Should have the default model in the changed models
    EXPECT_TRUE(changedModels.find(m_defaultModelId) != changedModels.end());

    // Close cache Db
    cacheDb.CloseDb();
    }