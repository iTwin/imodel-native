/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/CreateFromInstance_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>

USING_NAMESPACE_BENTLEY_DPTEST

//----------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct CreateFromInstanceTests : public DgnDbTestFixture 
    {
    protected:
        void CreateAndInsertElement(DgnElementPtr& element, Utf8CP json, Utf8CP schemaName, Utf8CP className);
    };

void CreateFromInstanceTests::CreateAndInsertElement(DgnElementPtr& element, Utf8CP json, Utf8CP schemaName, Utf8CP className)
    {
    ECN::ECClassCP ecClass = m_db->Schemas().GetECClass(schemaName, className);
    ASSERT_TRUE(nullptr != ecClass);
    ECN::IECInstancePtr ecInstance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    ASSERT_TRUE(ecInstance.IsValid());

    Json::Value jsonInput;
    ASSERT_TRUE(Json::Reader::Parse(json, jsonInput));
    ASSERT_EQ(SUCCESS, ECN::ECJsonUtilities::ECInstanceFromJson(*ecInstance, jsonInput));

    element = m_db->Elements().CreateElement(*ecInstance);
    DgnElementCPtr inserted = element->Insert();
    ASSERT_TRUE(inserted != nullptr);
    m_db->Schemas().CreateECClassViewsInDb();
    m_db->SaveChanges();
    }

TEST_F(CreateFromInstanceTests, Category)
    {
    SetupSeedProject();

    Utf8CP json =
        "{"
        "\"CodeAuthorityId\" : \"3\","
        "\"CodeNamespace\" : \"\","
        "\"CodeValue\" : \"Hub\","
        "\"Descr\" : \"\","
        "\"ModelId\" : \"16\","
        "\"ParentId\" : null,"
        "\"Rank\" : 1,"
        "\"Scope\" : 1,"
        "\"UserLabel\" : null,"
        "\"UserProperties\" : null"
        "}";

    DgnElementPtr inserted = nullptr;
    CreateAndInsertElement(inserted, json, "BisCore", "Category");
    ASSERT_TRUE(inserted.IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CreateFromInstanceTests, ViewDefinition)
    {
    SetupSeedProject();

    Utf8CP json =
        "{"
        "\"$ECClassId\" : \"198\","
        "\"$ECClassKey\" : \"BisCore.CameraViewDefinition\","
        "\"$ECClassLabel\" : \"CameraViewDefinition\","
        "\"$ECInstanceId\" : \"502\","
        "\"$ECInstanceLabel\" : \"CameraViewDefinition\","
        "\"CodeAuthorityId\" : \"4\","
        "\"CodeNamespace\" : \"ViewDefinition\","
        "\"CodeValue\" : \"Default - View 1\","
        "\"Descr\" : \"\","
        "\"ModelId\" : \"16\","
        "\"Extents\" : {"
            "\"x\" : 85.413445258737553,"
            "\"y\" : 76.125601109667528,"
            "\"z\" : 112.79558108349732"
        "},"
        "\"EyePoint\" : {"
            "\"x\" : 293.99476935528162,"
            "\"y\" : 69.335060236322079,"
            "\"z\" : 68.339134990346963"
        "},"
        "\"FocusDistance\" : 100.61073354297713,"
        "\"LensAngle\" : 0.80285145591739238,"
        "\"Origin\" : {"
            "\"x\" : 338.90639657040640,"
            "\"y\" : 174.64311379840612,"
            "\"z\" : -53.387925168591018"
        "},"
        "\"ParentId\" : null,"
        "\"Pitch\" : -35.264389682754654,"
        "\"Roll\" : -45.000000000000007,"
        "\"Source\" : 2,"
        "\"UserLabel\" : null,"
        "\"UserProperties\" : null,"
        "\"Yaw\" : 29.999999999999986"
        "}";


    ECN::ECClassCP viewDefClass = m_db->Schemas().GetECClass("BisCore", "CameraViewDefinition");
    ASSERT_TRUE(nullptr != viewDefClass);
    ECN::IECInstancePtr ecInstance = viewDefClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    ASSERT_TRUE(ecInstance.IsValid());
    
    Json::Value jsonInput;
    ASSERT_TRUE(Json::Reader::Parse(json, jsonInput));
    ASSERT_EQ(SUCCESS, ECN::ECJsonUtilities::ECInstanceFromJson(*ecInstance, jsonInput));

    auto viewElement = m_db->Elements().Create<CameraViewDefinition>(*ecInstance);
    ASSERT_TRUE(viewElement.IsValid());

    viewElement->SetDisplayStyle3d(*new DisplayStyle3d(*m_db, ""));
    viewElement->SetCategorySelector(*new CategorySelector(*m_db, ""));
    viewElement->SetModelSelector(*new ModelSelector(*m_db, ""));

    {
    auto inserted = viewElement->Insert();
    ASSERT_TRUE(inserted.IsValid());
    } // make sure we release pointer to inserted

    m_db->Memory().PurgeUntil(0); // to makes ure we re-read the element from the db

    auto camera = m_db->Elements().Get<CameraViewDefinition>(viewElement->GetElementId());
    ASSERT_TRUE(camera.IsValid());
    DPoint3d eyepoint = camera->GetEyePoint();
    ASSERT_TRUE(293.99476935528162 == eyepoint.x);
    ASSERT_TRUE(69.335060236322079 == eyepoint.y);
    ASSERT_TRUE(68.339134990346963 == eyepoint.z);
    ASSERT_TRUE(100.61073354297713 == camera->GetFocusDistance());
    ASSERT_TRUE(0.80285145591739238 == camera->GetLensAngle());

    DPoint3d origin = camera->GetOrigin();
    ASSERT_TRUE(338.90639657040640 == origin.x);
    ASSERT_TRUE(174.64311379840612 == origin.y);
    ASSERT_TRUE(-53.387925168591018 == origin.z);

    DPoint3d extents = camera->GetExtents();
    ASSERT_TRUE(85.413445258737553 == extents.x);
    ASSERT_TRUE(76.125601109667528 == extents.y);
    ASSERT_TRUE(112.79558108349732 == extents.z);

    YawPitchRollAngles angles;
    YawPitchRollAngles::TryFromRotMatrix(angles, camera->GetRotation());
    ASSERT_TRUE(29.999999999999986 == angles.GetYaw().Degrees());
    ASSERT_TRUE(-35.264389682754654 == angles.GetPitch().Degrees());
    ASSERT_TRUE(-45.000000000000007 == angles.GetRoll().Degrees());
    }
