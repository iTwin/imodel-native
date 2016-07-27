/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnLight_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnLight.h>

USING_NAMESPACE_BENTLEY_SQLITE

#define EXPECT_STR_EQ(X,Y) { if ((X).empty() || (Y).empty()) { EXPECT_EQ ((X).empty(), (Y).empty()); } else { EXPECT_EQ ((X), (Y)); } }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnLightsTest : public GenericDgnModelTestFixture
{
    Utf8String MakeLightValue(Utf8CP dummyJsonValue)
        {
        Json::Value json(Json::objectValue);
        json["dummy"] = dummyJsonValue;
        return Json::FastWriter::ToString(json);
        }

    LightDefinition::CreateParams MakeParams(Utf8CP name, Utf8CP dummyJsonValue, Utf8CP descr=nullptr)
        {
            return LightDefinition::CreateParams(*m_dgnDb, name, MakeLightValue(dummyJsonValue).c_str(), descr);
        }

    template<typename T, typename U> void Compare(T const& lhs, U const& rhs)
        {
        EXPECT_EQ(lhs.GetLightId(), rhs.GetLightId());
        EXPECT_STR_EQ(lhs.GetName(), rhs.GetName());
        EXPECT_STR_EQ(lhs.GetDescr(), rhs.GetDescr());
        EXPECT_STR_EQ(lhs.GetValue(), rhs.GetValue());
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnLightsTest, CRUD)
    {
    DgnDbPtr db = GetDgnDb(L"CRUD.bim");
    auto params = MakeParams("Light1", "one");
    LightDefinition lt(params);
    LightDefinitionCPtr persistent = lt.Insert();
    ASSERT_TRUE(persistent.IsValid());
    DgnCode lightCode = persistent->GetCode();
    DgnLightId lightId = persistent->GetLightId();

    Compare(lt, *persistent);

    lt.SetDescr("new description");
    lt.SetValue("value:4321");

    // Update 
    LightDefinitionCPtr updatedLt = lt.Update();
    ASSERT_TRUE(updatedLt.IsValid());
    Compare(lt, *updatedLt);

    // Query 
    LightDefinitionCPtr toFind = LightDefinition::QueryLightDefinition(lightId, *db);
    Compare(*updatedLt, *toFind);

    DgnLightId idToFind = LightDefinition::QueryLightId("Light1", *db);
    EXPECT_TRUE(lightId == idToFind);

    idToFind = LightDefinition::QueryLightId(lightCode, *db);
    EXPECT_TRUE(lightId == idToFind);

    }

