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

    DgnDbStatus stat;
    element = m_db->Elements().CreateElement(&stat, *ecInstance);
    DgnElementCPtr inserted = element->Insert(&stat);
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
        "\"CodeAuthorityId\" : \"4\","
        "\"CodeNamespace\" : \"ViewDefinition\","
        "\"CodeValue\" : \"Default - View 1\","
        "\"Descr\" : \"\","
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
        "\"ModelId\" : \"16\","
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

    DgnElementPtr inserted = nullptr;
    CreateAndInsertElement(inserted, json, "BisCore", "CameraViewDefinition");
    CameraViewDefinitionCPtr camera = m_db->Elements().Get<CameraViewDefinition>(inserted->GetElementId());
    ASSERT_TRUE(camera.IsValid());
    DPoint3d eyepoint = camera->GetEyePoint();
    //ASSERT_EQ(293.99476935528162, eyepoint.x);
    }
