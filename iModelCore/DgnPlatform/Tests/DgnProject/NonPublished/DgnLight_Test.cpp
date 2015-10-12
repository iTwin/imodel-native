/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnLight_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnCore/DgnLight.h>

USING_NAMESPACE_BENTLEY_SQLITE

#define EXPECT_STR_EQ(X,Y) { if ((X).empty() || (Y).empty()) { EXPECT_EQ ((X).empty(), (Y).empty()); } else { EXPECT_EQ ((X), (Y)); } }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnLightsTest : public ::testing::Test
{
private:
    ScopedDgnHost   m_host;
    DgnDbPtr        m_db;
protected:
    void SetupProject()
        {
        BeFileName filename = DgnDbTestDgnManager::GetOutputFilePath (L"lights.idgndb");
        BeFileName::BeDeleteFile (filename);

        CreateDgnDbParams params;
        params.SetOverwriteExisting (false);
        DbResult status;
        m_db = DgnDb::CreateDgnDb (&status, filename, params);
        ASSERT_TRUE (m_db != nullptr);
        ASSERT_EQ (BE_SQLITE_OK, status) << status;
        }

    DgnDbR GetDb() { return *m_db; }

    Utf8String MakeLightValue(Utf8CP dummyJsonValue)
        {
        Json::Value json(Json::objectValue);
        json["dummy"] = dummyJsonValue;
        return Json::FastWriter::ToString(json);
        }

    LightDefinition::CreateParams MakeParams(Utf8CP name, Utf8CP dummyJsonValue, Utf8CP descr=nullptr)
        {
        return LightDefinition::CreateParams(*m_db, name, MakeLightValue(dummyJsonValue).c_str(), descr);
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
    SetupProject();
    auto params = MakeParams("Light1", "one");
    LightDefinition lt(params);
    LightDefinitionCPtr persistent = lt.Insert();
    ASSERT_TRUE(persistent.IsValid());

    Compare(lt, *persistent);

    lt.SetDescr("new description");
    lt.SetValue("value:4321");

    LightDefinitionCPtr updatedLt = lt.Update();
    ASSERT_TRUE(updatedLt.IsValid());
    Compare(lt, *updatedLt);
    }

