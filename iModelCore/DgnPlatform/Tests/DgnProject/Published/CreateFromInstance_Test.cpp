/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/CreateFromInstance_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>

USING_NAMESPACE_BENTLEY_DPTEST

//=======================================================================================
// @bsiclass                                   Carole.MacDonald            10/2016
//=======================================================================================
struct CreateFromInstanceTests : public DgnDbTestFixture 
    {
    protected:
        void CreateAndInsertElement(DgnElementPtr& element, Utf8CP json, Utf8CP schemaName, Utf8CP className);
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CreateFromInstanceTests, SpatialCategory)
    {
    SetupSeedProject();

    Utf8PrintfString json(
        "{"
        "\"CodeSpec\" : {\"id\" : \"%d\"},"
        "\"CodeScope\" : \"\","
        "\"CodeValue\" : \"Hub\","
        "\"Description\" : \"\","
        "\"Model\" : {\"id\" : \"16\"},"
        "\"Parent\" : null,"
        "\"Rank\" : 1,"
        "\"UserLabel\" : null,"
        "\"UserProperties\" : null"
        "}", 
        static_cast<int>(m_db->CodeSpecs().QueryCodeSpecId(BIS_CODESPEC_SpatialCategory).GetValue())); // value for CodeSpec.Id

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
        "\"$ECClassId\" : \"198\","
        "\"$ECClassKey\" : \"BisCore.SpatialViewDefinition\","
        "\"$ECClassLabel\" : \"SpatialViewDefinition\","
        "\"$ECInstanceId\" : \"502\","
        "\"$ECInstanceLabel\" : \"SpatialViewDefinition\","
        "\"CodeSpec\" : {\"id\" : \"%d\"},"
        "\"CodeScope\" : \"ViewDefinition\","
        "\"CodeValue\" : \"Default - View 1\","
        "\"Description\" : \"\","
        "\"Model\" : {\"id\" : \"16\"},"
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
        "\"FocusDistance\" : 100.610733542977,"
        "\"LensAngle\" : 0.802851455917392,"
        "\"Origin\" : {"
            "\"x\" : 338.90639657040640,"
            "\"y\" : 174.64311379840612,"
            "\"z\" : -53.387925168591018"
        "},"
        "\"Parent\" : null,"
        "\"Pitch\" : -35.264389682754654,"
        "\"Roll\" : -45.000000000000007,"
        "\"IsPrivate\" : false,"
        "\"UserLabel\" : null,"
        "\"UserProperties\" : null,"
        "\"Yaw\" : 29.999999999999986"
        "}",
        static_cast<int>(m_db->CodeSpecs().QueryCodeSpecId(BIS_CODESPEC_ViewDefinition).GetValue())); // value for CodeSpec.Id

    ECN::ECClassCP viewDefClass = m_db->Schemas().GetECClass(BIS_ECSCHEMA_NAME, BIS_CLASS_SpatialViewDefinition);
    ASSERT_TRUE(nullptr != viewDefClass);
    ECN::IECInstancePtr ecInstance = viewDefClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    ASSERT_TRUE(ecInstance.IsValid());
    
    Json::Value jsonInput;
    ASSERT_TRUE(Json::Reader::Parse(json, jsonInput));
    ASSERT_EQ(SUCCESS, ECN::ECJsonUtilities::ECInstanceFromJson(*ecInstance, jsonInput));

    auto viewElement = m_db->Elements().Create<SpatialViewDefinition>(*ecInstance);
    ASSERT_TRUE(viewElement.IsValid());

    viewElement->SetDisplayStyle3d(*new DisplayStyle3d(*m_db, ""));
    viewElement->SetCategorySelector(*new CategorySelector(*m_db, ""));
    viewElement->SetModelSelector(*new ModelSelector(*m_db, ""));

    {
    auto inserted = viewElement->Insert();
    ASSERT_TRUE(inserted.IsValid());
    } // make sure we release pointer to inserted

    m_db->Memory().PurgeUntil(0); // to make sure we re-read the element from the db

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
