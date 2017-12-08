/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/CreateFromInstance_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include <DgnPlatform/GenericDomain.h>

USING_NAMESPACE_BENTLEY_DPTEST

//=======================================================================================
// @bsiclass                                   Carole.MacDonald            10/2016
//=======================================================================================
struct CreateFromInstanceTests : public DgnDbTestFixture 
    {
    protected:
        void CreateAndInsertElement(DgnElementPtr& element, Utf8CP json, Utf8CP schemaName, Utf8CP className);
        ECN::IECInstancePtr CreateInstanceFromElement(DgnElementCP element);
        ECN::IECInstancePtr CreateInstanceFromModel(DgnModelCP element);

        void CreateSpatialViewDefinition(ECN::IECInstancePtr& catSelectorInstance, ECN::IECInstancePtr& displayStyleInstance, ECN::IECInstancePtr& modelSelectorInstance, ECN::IECInstancePtr& cameraViewInstance, DgnModelId modelId, DgnCategoryId categoryId);
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
void CreateFromInstanceTests::CreateAndInsertElement(DgnElementPtr& element, Utf8CP json, Utf8CP schemaName, Utf8CP className)
    {
    ECN::ECClassCP ecClass = m_db->Schemas().GetClass(schemaName, className);
    ASSERT_TRUE(nullptr != ecClass);
    ECN::IECInstancePtr ecInstance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    ASSERT_TRUE(ecInstance.IsValid());

    Json::Value jsonInput;
    ASSERT_TRUE(Json::Reader::Parse(json, jsonInput));
    ASSERT_EQ(SUCCESS, ECN::JsonECInstanceConverter::JsonToECInstance(*ecInstance, jsonInput, m_db->GetClassLocater()));

    element = m_db->Elements().CreateElement(*ecInstance);
    DgnElementCPtr inserted = element->Insert();
    ASSERT_TRUE(inserted != nullptr);
    m_db->Schemas().CreateClassViewsInDb();
    m_db->SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2017
//---------------+---------------+---------------+---------------+---------------+-------
ECN::IECInstancePtr CreateFromInstanceTests::CreateInstanceFromElement(DgnElementCP element)
    {
    Utf8PrintfString ecSql("SELECT * FROM ONLY %s.%s WHERE ECInstanceId=?", element->GetElementClass()->GetSchema().GetAlias().c_str(), element->GetElementClass()->GetName().c_str());
    CachedECSqlStatementPtr statement = m_db->GetPreparedECSqlStatement(ecSql.c_str());
    statement->BindId(1, element->GetElementId());
    ECInstanceECSqlSelectAdapter adapter(*statement);
    while (BE_SQLITE_ROW == statement->Step())
        {
        ECN::IECInstancePtr tmp = adapter.GetInstance();
        // Need to remove the instance id so that when inserted, the db will generate a new id
        tmp->SetInstanceId(nullptr);
        return tmp;
        }
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2017
//---------------+---------------+---------------+---------------+---------------+-------
ECN::IECInstancePtr CreateFromInstanceTests::CreateInstanceFromModel(DgnModelCP model)
    {
    ECN::ECClassCP ecClass = m_db->Schemas().GetClass(model->GetClassId());
    Utf8PrintfString ecSql("SELECT * FROM ONLY %s.%s WHERE ECInstanceId=?", ecClass->GetSchema().GetAlias().c_str(), ecClass->GetName().c_str());
    CachedECSqlStatementPtr statement = m_db->GetPreparedECSqlStatement(ecSql.c_str());
    statement->BindId(1, model->GetModelId());
    ECInstanceECSqlSelectAdapter adapter(*statement);
    while (BE_SQLITE_ROW == statement->Step())
        {
        ECN::IECInstancePtr tmp = adapter.GetInstance();
        // Need to remove the instance id so that when inserted, the db will generate a new id
        tmp->SetInstanceId(nullptr);
        return tmp;
        }
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2017
//---------------+---------------+---------------+---------------+---------------+-------
void CreateFromInstanceTests::CreateSpatialViewDefinition(ECN::IECInstancePtr& catSelectorInstance, ECN::IECInstancePtr& displayStyleInstance, ECN::IECInstancePtr& modelSelectorInstance, ECN::IECInstancePtr& cameraViewInstance, DgnModelId modelId, DgnCategoryId categoryId)
    {
    // Create CategorySelector
    DefinitionModelR dictionary = m_db->GetDictionaryModel();
    CategorySelectorPtr catSelector = new CategorySelector(dictionary, "Test Category Selector");
    catSelector->AddCategory(categoryId);
    DgnDbStatus stat;
    catSelector->Insert(&stat);
    ASSERT_TRUE(DgnDbStatus::Success == stat);
    catSelectorInstance = CreateInstanceFromElement(catSelector.get());

    // Create DisplayStyle
    DisplayStyle3dPtr displayStyle = new DisplayStyle3d(dictionary, "Test Display Style");
    displayStyle->SetBackgroundColor(ColorDef::DarkCyan());
    Render::ViewFlags viewFlags = displayStyle->GetViewFlags();
//    viewFlags.SetIgnoreLighting(false);
    viewFlags.SetRenderMode(Render::RenderMode::SmoothShade);
    viewFlags.SetShowTextures(true);
    displayStyle->SetViewFlags(viewFlags);
    displayStyle->Insert(&stat);
    ASSERT_TRUE(DgnDbStatus::Success == stat);
    displayStyleInstance = CreateInstanceFromElement(displayStyle.get());

    // Create ModelSelector
    ModelSelectorPtr modelSelector = new ModelSelector(dictionary, "Test Model Selector");
    modelSelector->AddModel(modelId);
    modelSelector->Insert(&stat);
    ASSERT_TRUE(DgnDbStatus::Success == stat);
    modelSelectorInstance = CreateInstanceFromElement(modelSelector.get());

    // Create CameraViewDefinition
    SpatialViewDefinition spatialViewDefinition(dictionary, "Test Camera View Definition", *catSelector, *displayStyle, *modelSelector);
    spatialViewDefinition.Insert(&stat);
    ASSERT_TRUE(DgnDbStatus::Success == stat);
    cameraViewInstance = CreateInstanceFromElement(&spatialViewDefinition);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CreateFromInstanceTests, SpatialCategory)
    {
    SetupSeedProject();

    Utf8PrintfString json(
        "{"
        "\"codeSpec\" : {\"id\" : \"%s\"},"
        "\"codeScope\" : {\"id\" : \"0X10\"},"
        "\"codeValue\" : \"Hub\","
        "\"model\" : {\"id\" : \"0X10\"},"
        "\"rank\" : 1"
        "}", 
        m_db->CodeSpecs().QueryCodeSpecId(BIS_CODESPEC_SpatialCategory).ToHexStr().c_str()); // value for CodeSpec.Id

    DgnElementPtr inserted = nullptr;
    CreateAndInsertElement(inserted, json.c_str(), BIS_ECSCHEMA_NAME, BIS_CLASS_SpatialCategory);
    ASSERT_TRUE(inserted.IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CreateFromInstanceTests, ViewDefinition)
    {
    SetupSeedProject();

    Utf8PrintfString json(
        "{"
        "\"className\" : \"BisCore.SpatialViewDefinition\","
        "\"id\" : \"0X1F6\","
        "\"codeSpec\" : {\"id\" : \"%s\"},"
        "\"codeScope\" : {\"id\" : \"0X10\"},"
        "\"codeValue\" : \"Default - View 1\","
        "\"model\" : {\"id\" : \"0X10\"},"
        "\"extents\" : {"
            "\"x\" : 85.413445258737553,"
            "\"y\" : 76.125601109667528,"
            "\"z\" : 112.79558108349732"
        "},"
        "\"eyePoint\" : {"
            "\"x\" : 293.99476935528162,"
            "\"y\" : 69.335060236322079,"
            "\"z\" : 68.339134990346963"
        "},"
        "\"focusDistance\" : 100.610733542977,"
        "\"lensAngle\" : 0.802851455917392,"
        "\"origin\" : {"
            "\"x\" : 338.90639657040640,"
            "\"y\" : 174.64311379840612,"
            "\"z\" : -53.387925168591018"
        "},"
        "\"pitch\" : -35.264389682754654,"
        "\"roll\" : -45.000000000000007,"
        "\"isPrivate\" : false,"
        "\"yaw\" : 29.999999999999986"
        "}",
        m_db->CodeSpecs().QueryCodeSpecId(BIS_CODESPEC_ViewDefinition).ToHexStr().c_str()); // value for CodeSpec.Id

    ECN::ECClassCP viewDefClass = m_db->Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_SpatialViewDefinition);
    ASSERT_TRUE(nullptr != viewDefClass);
    ECN::IECInstancePtr ecInstance = viewDefClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    ASSERT_TRUE(ecInstance.IsValid());
    
    Json::Value jsonInput;
    ASSERT_TRUE(Json::Reader::Parse(json, jsonInput));
    ASSERT_EQ(SUCCESS, ECN::JsonECInstanceConverter::JsonToECInstance(*ecInstance, jsonInput, m_db->GetClassLocater()));

    auto viewElement = m_db->Elements().Create<SpatialViewDefinition>(*ecInstance);
    ASSERT_TRUE(viewElement.IsValid());

    DefinitionModelR dictionary = m_db->GetDictionaryModel();
    viewElement->SetDisplayStyle3d(*new DisplayStyle3d(dictionary, ""));
    viewElement->SetCategorySelector(*new CategorySelector(dictionary, ""));
    viewElement->SetModelSelector(*new ModelSelector(dictionary, ""));

    {
    auto inserted = viewElement->Insert();
    ASSERT_TRUE(inserted.IsValid());
    } // make sure we release pointer to inserted

    m_db->Elements().ClearCache(); // to make sure we re-read the element from the db

    auto camera = m_db->Elements().Get<SpatialViewDefinition>(viewElement->GetElementId());
    ASSERT_TRUE(camera.IsValid());
    DPoint3d eyepoint = camera->GetEyePoint();
    ASSERT_EQ(293.99476935528162, eyepoint.x);
    ASSERT_EQ(69.335060236322079, eyepoint.y);
    ASSERT_EQ(68.339134990346963, eyepoint.z);
    ASSERT_EQ(100.610733542977, camera->GetFocusDistance());
    ASSERT_EQ(0.802851455917392, camera->GetLensAngle().Radians());

    DPoint3d origin = camera->GetOrigin();
    ASSERT_EQ(338.90639657040640, origin.x);
    ASSERT_EQ(174.64311379840612, origin.y);
    ASSERT_EQ(-53.387925168591018, origin.z);

    DPoint3d extents = camera->GetExtents();
    ASSERT_EQ(85.413445258737553, extents.x);
    ASSERT_EQ(76.125601109667528, extents.y);
    ASSERT_EQ(112.79558108349732, extents.z);

    YawPitchRollAngles angles;
    YawPitchRollAngles::TryFromRotMatrix(angles, camera->GetRotation());
    ASSERT_EQ(29.999999999999986, angles.GetYaw().Degrees());
    ASSERT_EQ(-35.264389682754654, angles.GetPitch().Degrees());
    ASSERT_EQ(-45.000000000000007, angles.GetRoll().Degrees());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CreateFromInstanceTests, FullyCreateBim)
    {
    SetupSeedProject();

    // First create entities using the API.  Get each entity as an IECInstance.  And then use those to create new elements
    // Create a new Subject
    SubjectCPtr subjectElement = Subject::CreateAndInsert(*m_db->Elements().GetRootSubject(), "TestSubject", "Subject used for testing");
    ASSERT_TRUE(subjectElement.IsValid());
    ECN::IECInstancePtr subjectInstance = CreateInstanceFromElement(subjectElement.get());
    ASSERT_TRUE(subjectInstance.IsValid());

    // Create a new Physical  Partition
    PhysicalPartitionCPtr physicalPartition = PhysicalPartition::CreateAndInsert(*subjectElement, "TestPhysicalPartition", "Physical partition for test");
    ASSERT_TRUE(physicalPartition.IsValid());
    ECN::IECInstancePtr partitionInstance = CreateInstanceFromElement(physicalPartition.get());
    ASSERT_TRUE(partitionInstance.IsValid());

    // Create a new Physical Model
    PhysicalModelPtr physicalModel = PhysicalModel::Create(*physicalPartition);
    ASSERT_TRUE(physicalModel.IsValid());
    physicalModel->Insert();
    ECN::IECInstancePtr modelInstance = CreateInstanceFromModel(physicalModel.get());
    
    // Create a new Category
    SpatialCategory spatialCategory(m_db->GetDictionaryModel(), "Test Spatial Category", DgnCategory::Rank::Application, "This is a test category for spatial elements");
    ////Appearance properties.
    uint32_t weight = 10;
    double trans = 0.5;
    uint32_t dp = 1;

    DgnSubCategory::Appearance appearance;
    appearance.SetInvisible(false);
    appearance.SetColor(ColorDef::DarkRed());
    appearance.SetWeight(weight);
    appearance.SetTransparency(trans);
    appearance.SetDisplayPriority(dp);
    appearance.SetDontLocate(true);
    appearance.SetDontPlot(true);
    appearance.SetDontSnap(true);
    appearance.SetDisplayPriority(1);

    DgnCategoryCPtr categoryElement = spatialCategory.Insert(appearance);
    ASSERT_TRUE(categoryElement.IsValid());
    ECN::IECInstancePtr categoryInstance = CreateInstanceFromElement(categoryElement.get());
    DgnCategoryId categoryId = spatialCategory.GetCategoryId();
    
    // Create new subcategory
    Utf8CP sub_name = "Test SubCategory";
    Utf8CP sub_desc = "This is a test subcategory";
    DgnSubCategory subcategory(DgnSubCategory::CreateParams(*m_db, categoryId, sub_name, appearance, sub_desc));
    //Inserts a subcategory
    DgnSubCategoryCPtr subCategoryElement = subcategory.Insert();
    ASSERT_TRUE(subCategoryElement.IsValid());
    ECN::IECInstancePtr subCategoryInstance = CreateInstanceFromElement(subCategoryElement.get());

    ECN::IECInstancePtr catSelectorInstance, displayStyleInstance, modelSelectorInstance, cameraViewInstance;
    CreateSpatialViewDefinition(catSelectorInstance, displayStyleInstance, modelSelectorInstance, cameraViewInstance, physicalModel->GetModelId(), categoryId);

    // Create Geometric Element
    GenericPhysicalObjectPtr physicalElementPtr = GenericPhysicalObject::Create(*physicalModel, categoryId);

    DgnBoxDetail blockDetail = DgnBoxDetail::InitFromCenterAndSize(DPoint3d::FromZero(), DPoint3d::From(25.0, 25.0, 25.0), true);
    ISolidPrimitivePtr geomPtr = ISolidPrimitive::CreateDgnBox(blockDetail);
    BeAssert(geomPtr.IsValid());

    GeometryBuilderPtr builder = GeometryBuilder::Create(*physicalModel, categoryId, DPoint3d::From(37.5, 75.0, 100), YawPitchRollAngles());
    builder->Append(*geomPtr);
    BentleyStatus status = builder->Finish(*physicalElementPtr);
    BeAssert(status == SUCCESS);

    GenericPhysicalObjectCPtr insertedElement = m_db->Elements().Insert<GenericPhysicalObject>(*physicalElementPtr);
    BeAssert(insertedElement.IsValid());
    ECN::IECInstancePtr physicalObjectInstance = CreateInstanceFromElement(insertedElement.get());

    DgnDbPtr newDb = DgnPlatformSeedManager::OpenSeedDbCopy(DgnDbTestFixture::s_seedFileInfo.fileName, L"FullyCreateBimFromInstance.bim");
    DgnDbStatus stat;
    SubjectPtr subjectElement2 = newDb->Elements().Create<Subject>(*subjectInstance, &stat);
    ASSERT_TRUE(subjectElement2.IsValid());
    subjectElement2->Insert();

    PhysicalPartitionPtr physicalPartition2 = newDb->Elements().Create<PhysicalPartition>(*partitionInstance, &stat);
    ASSERT_TRUE(physicalPartition2.IsValid());
    physicalPartition2->Insert();

    m_db->Schemas().CreateClassViewsInDb();
    newDb->Schemas().CreateClassViewsInDb();
    newDb->SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    06/2017
//---------------------------------------------------------------------------------------
TEST_F(CreateFromInstanceTests, SampleForHopen)
    {
    USING_NAMESPACE_BENTLEY_EC;
    SetupSeedProject();
    m_db->Schemas().CreateClassViewsInDb();
    PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*m_db, "MyModel");
    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(*m_db, "MyCategory");
    DgnCode emptyCode = DgnCode::CreateEmpty();

    Utf8CP str_Zero = "Zero";
    Utf8CP str_One = "One";
    Utf8CP str_Two = "Two";
    Utf8CP str_Three = "Three";
    Utf8CP str_StocktonDrive = "Stockton Drive";
    Utf8CP str_PottstownPike = "Pottstown Pike";
    Utf8CP str_Exton = "Exton";
    Utf8CP str_Pennsylvania = "Pennsylvania";
    Utf8CP str_USA = "USA";
    const int32_t zipCode = 19341;

    ECClassCP testElementClass = m_db->Schemas().GetClass(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_CLASS_NAME);
    ECClassCP locationStruct = m_db->Schemas().GetClass(DPTEST_SCHEMA_NAME, DPTEST_TEST_LOCATION_STRUCT_CLASS_NAME);
    ASSERT_NE(testElementClass, nullptr);
    ASSERT_NE(locationStruct, nullptr);

    StandaloneECEnablerPtr locationEnabler = locationStruct->GetDefaultStandaloneEnabler();
    ASSERT_TRUE(locationEnabler.IsValid());

    StandaloneECInstancePtr locationInstance0 = locationEnabler->CreateInstance();
    locationInstance0->SetValue("Street", ECValue(str_StocktonDrive));
    locationInstance0->SetValue("City.Name", ECN::ECValue(str_Exton));
    locationInstance0->SetValue("City.State", ECN::ECValue(str_Pennsylvania));
    locationInstance0->SetValue("City.Country", ECN::ECValue(str_USA));
    locationInstance0->SetValue("City.Zip", ECN::ECValue(zipCode));
    ECValue locationValue0;
    locationValue0.SetStruct(locationInstance0.get());

    StandaloneECInstancePtr locationInstance1 = locationEnabler->CreateInstance();
    locationInstance1->SetValue("Street", ECValue(str_PottstownPike));
    locationInstance1->SetValue("City.Name", ECN::ECValue(str_Exton));
    locationInstance1->SetValue("City.State", ECN::ECValue(str_Pennsylvania));
    locationInstance1->SetValue("City.Country", ECN::ECValue(str_USA));
    locationInstance1->SetValue("City.Zip", ECN::ECValue(zipCode));
    ECValue locationValue1;
    locationValue1.SetStruct(locationInstance1.get());

    ECPropertyP locationProperty = testElementClass->GetPropertyP("Location");
    ASSERT_NE(locationProperty, nullptr);
    ASSERT_EQ(locationProperty->GetAsPrimitiveProperty(), nullptr);
    ASSERT_NE(locationProperty->GetAsStructProperty(), nullptr);

    ECPropertyP arrayOfStringProperty = testElementClass->GetPropertyP("ArrayOfString");
    ASSERT_NE(arrayOfStringProperty, nullptr);
    ASSERT_EQ(arrayOfStringProperty->GetAsPrimitiveProperty(), nullptr);
    ASSERT_NE(arrayOfStringProperty->GetAsArrayProperty(), nullptr);

    ECPropertyP arrayOfStructsProperty = testElementClass->GetPropertyP("ArrayOfStructs");
    ASSERT_NE(arrayOfStructsProperty, nullptr);
    ASSERT_EQ(arrayOfStructsProperty->GetAsPrimitiveProperty(), nullptr);
    ASSERT_NE(arrayOfStructsProperty->GetAsArrayProperty(), nullptr);

    StandaloneECEnablerPtr testElementEnabler = testElementClass->GetDefaultStandaloneEnabler();
    ASSERT_TRUE(testElementEnabler.IsValid());

    StandaloneECInstancePtr testElementInstance = testElementEnabler->CreateInstance();
    ASSERT_TRUE(testElementInstance.IsValid());

    ASSERT_EQ(ECObjectsStatus::Success, testElementInstance->SetValue("Model", ECValue(model->GetModelId())));
    ASSERT_EQ(ECObjectsStatus::Success, testElementInstance->SetValue("Category", ECValue(categoryId)));
    ASSERT_EQ(ECObjectsStatus::Success, testElementInstance->SetValue("CodeSpec", ECValue(emptyCode.GetCodeSpecId())));
    ASSERT_EQ(ECObjectsStatus::Success, testElementInstance->SetValue("CodeScope", ECValue(emptyCode.GetScopeElementId(*m_db))));
    ASSERT_EQ(ECObjectsStatus::Success, testElementInstance->SetValue("Location.Street", ECValue(str_StocktonDrive)));
    ASSERT_EQ(ECObjectsStatus::Success, testElementInstance->SetValue("Location.City.Name", ECValue(str_Exton)));
    ASSERT_EQ(ECObjectsStatus::Success, testElementInstance->SetValue("Location.City.State", ECValue(str_Pennsylvania)));
    ASSERT_EQ(ECObjectsStatus::Success, testElementInstance->SetValue("Location.City.Country", ECValue(str_USA)));
    ASSERT_EQ(ECObjectsStatus::Success, testElementInstance->SetValue("Location.City.Zip", ECValue(zipCode)));
    ASSERT_EQ(ECObjectsStatus::Success, testElementInstance->AddArrayElements("ArrayOfString", 4));
    ASSERT_EQ(ECObjectsStatus::Success, testElementInstance->SetValue("ArrayOfString", ECValue(str_Zero), 0));
    ASSERT_EQ(ECObjectsStatus::Success, testElementInstance->SetValue("ArrayOfString", ECValue(str_One), 1));
    ASSERT_EQ(ECObjectsStatus::Success, testElementInstance->SetValue("ArrayOfString", ECValue(str_Two), 2));
    ASSERT_EQ(ECObjectsStatus::Success, testElementInstance->SetValue("ArrayOfString", ECValue(str_Three), 3));
    ASSERT_EQ(ECObjectsStatus::Success, testElementInstance->AddArrayElements("ArrayOfStructs", 2));
    ASSERT_EQ(ECObjectsStatus::Success, testElementInstance->SetValue("ArrayOfStructs", locationValue0, 0));
    ASSERT_EQ(ECObjectsStatus::Success, testElementInstance->SetValue("ArrayOfStructs", locationValue1, 1));

    DgnElementId testElementId;

    // Create and Insert element from ECInstance
        {
        DgnElementPtr testElement = m_db->Elements().CreateElement(*testElementInstance);
        ASSERT_TRUE(testElement.IsValid());
        ASSERT_TRUE(testElement->Insert().IsValid());
        testElementId = testElement->GetElementId();
        }

    // Re-load element to make sure it was inserted properly
        {
        m_db->Elements().ClearCache();
        PhysicalElementCPtr testElement = m_db->Elements().Get<PhysicalElement>(testElementId);
        ASSERT_TRUE(testElement.IsValid());
        ASSERT_EQ(testElement->GetModelId(), model->GetModelId());
        ASSERT_EQ(testElement->GetCategoryId(), categoryId);
        ASSERT_EQ(testElement->GetCode().GetCodeSpecId(), emptyCode.GetCodeSpecId());
        ASSERT_EQ(testElement->GetCode().GetScopeElementId(*m_db), emptyCode.GetScopeElementId(*m_db));
        ASSERT_TRUE(testElement->GetCode().GetValue().empty());
        ASSERT_STREQ(testElement->GetPropertyValueString("Location.Street").c_str(), str_StocktonDrive);
        ASSERT_STREQ(testElement->GetPropertyValueString("Location.City.Name").c_str(), str_Exton);
        ASSERT_STREQ(testElement->GetPropertyValueString("Location.City.State").c_str(), str_Pennsylvania);
        ASSERT_STREQ(testElement->GetPropertyValueString("Location.City.Country").c_str(), str_USA);
        ASSERT_EQ(testElement->GetPropertyValueInt32("Location.City.Zip"), zipCode);
        ASSERT_STREQ(testElement->GetPropertyValueString("ArrayOfString", 0).c_str(), str_Zero);
        ASSERT_STREQ(testElement->GetPropertyValueString("ArrayOfString", 1).c_str(), str_One);
        ASSERT_STREQ(testElement->GetPropertyValueString("ArrayOfString", 2).c_str(), str_Two);
        ASSERT_STREQ(testElement->GetPropertyValueString("ArrayOfString", 3).c_str(), str_Three);
        ECValue structValue;
        ASSERT_EQ(DgnDbStatus::Success, testElement->GetPropertyValue(structValue, "ArrayOfStructs", 1));
        ASSERT_FALSE(structValue.IsNull());
        ASSERT_TRUE(structValue.IsStruct());
        IECInstancePtr structInstance = structValue.GetStruct();
        ASSERT_TRUE(structInstance.IsValid());
        ECValue structMemberValue;
        ASSERT_EQ(ECObjectsStatus::Success, structInstance->GetValue(structMemberValue, "Street"));
        ASSERT_STREQ(structMemberValue.GetUtf8CP(), str_PottstownPike);
        ASSERT_EQ(ECObjectsStatus::Success, structInstance->GetValue(structMemberValue, "City.Name"));
        ASSERT_STREQ(structMemberValue.GetUtf8CP(), str_Exton);
        ASSERT_EQ(ECObjectsStatus::Success, structInstance->GetValue(structMemberValue, "City.State"));
        ASSERT_STREQ(structMemberValue.GetUtf8CP(), str_Pennsylvania);
        ASSERT_EQ(ECObjectsStatus::Success, structInstance->GetValue(structMemberValue, "City.Country"));
        ASSERT_STREQ(structMemberValue.GetUtf8CP(), str_USA);
        ASSERT_EQ(ECObjectsStatus::Success, structInstance->GetValue(structMemberValue, "City.Zip"));
        ASSERT_EQ(structMemberValue.GetInteger(), zipCode);
        }
    }
