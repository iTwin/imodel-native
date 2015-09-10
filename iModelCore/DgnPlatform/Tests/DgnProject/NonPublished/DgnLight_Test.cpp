/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnLight_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

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
    typedef DgnLights LT;

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

    LT::Light MakeLight(Utf8CP name, Utf8CP dummyJsonValue, Utf8CP descr=nullptr)
        {
        return LT::Light(name, MakeLightValue(dummyJsonValue).c_str(), descr);
        }

    bool Insert(LT::Light& light) { return m_db->Lights().Insert(light).IsValid(); }

    struct LightEntry : LT::Light
    {
        DgnLightId      m_lightId;

        LightEntry(LT::Iterator::Entry const& entry) : LT::Light(entry.GetName(), entry.GetValue(), entry.GetDescr()), m_lightId(entry.GetId()) { }

        DgnLightId GetId() const { return m_lightId; }
    };

    template<typename T, typename U> void Compare(T const& lhs, U const& rhs)
        {
        EXPECT_EQ(lhs.GetId(), rhs.GetId());
        EXPECT_STR_EQ(lhs.GetName(), rhs.GetName());
        EXPECT_STR_EQ(lhs.GetDescr(), rhs.GetDescr());
        EXPECT_STR_EQ(lhs.GetValue(), rhs.GetValue());
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnLightsTest, InsertUpdateIterate)
    {
    SetupProject();
    DgnDbR db = GetDb();

    DgnLights& lts = db.Lights();
    bvector<LT::Light> expectedLts;

    auto lt = MakeLight("Light1", "one");
    EXPECT_TRUE(Insert(lt));
    expectedLts.push_back(lt);

    // name must be unique
    lt = MakeLight("Light1", "duplicate");
    EXPECT_FALSE(Insert(lt));

    lt = MakeLight("Light2", "two", "second light");
    EXPECT_TRUE(Insert(lt));
    expectedLts.push_back(lt);

    // Test iteration
    auto iter = lts.MakeIterator();
    EXPECT_EQ(iter.QueryCount(), expectedLts.size());

    size_t nFound = 0;
    for (auto& entry : iter)
        {
        ++nFound;
        auto match = std::find_if(expectedLts.begin(), expectedLts.end(), [&](LT::Light const& arg) { return arg.GetId() == entry.GetId(); });
        ASSERT_FALSE(match == expectedLts.end());
        Compare (LightEntry(entry), *match);
        }

    EXPECT_EQ(nFound, expectedLts.size());

    // query by name and ID
    for (auto const& expectedLt : expectedLts)
        {
        auto ltId = expectedLt.GetId();
        auto roundTrippedLtId = lts.QueryLightId (expectedLt.GetName());
        EXPECT_EQ(roundTrippedLtId, ltId);

        lt = lts.Query(ltId);
        EXPECT_TRUE(lt.IsValid());
        Compare(expectedLt, lt);
        }

    // Test modification
    lt = expectedLts.back();
    lt.SetDescr("updated descr");
    lt.SetValue(MakeLightValue("updated value"));
    EXPECT_EQ(DgnDbStatus::Success, lts.Update(lt));
    LT::Light updatedLt = lts.Query(lt.GetId());
    EXPECT_TRUE(updatedLt.IsValid());
    Compare(updatedLt, lt);
    }

