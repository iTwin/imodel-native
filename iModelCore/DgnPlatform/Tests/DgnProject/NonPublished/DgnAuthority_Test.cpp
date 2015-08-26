/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnAuthority_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

USING_NAMESPACE_BENTLEY_SQLITE

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnAuthoritiesTest : public ::testing::Test
    {
private:
    ScopedDgnHost       m_host;
    DgnDbPtr            m_db;
protected:
    void SetupProject()
        {
        BeFileName filename = DgnDbTestDgnManager::GetOutputFilePath (L"authorities.idgndb");
        BeFileName::BeDeleteFile (filename);

        CreateDgnDbParams params;
        params.SetOverwriteExisting (false);
        DbResult status;
        m_db = DgnDb::CreateDgnDb (&status, filename, params);
        ASSERT_TRUE (m_db != nullptr);
        ASSERT_EQ (BE_SQLITE_OK, status) << status;
        }

    DgnDbR      GetDb()
        {
        return *m_db;
        }

    template<typename LHS, typename RHS>
    void        Compare (LHS const& lhs, RHS const& rhs)
        {
        EXPECT_EQ (lhs.GetId(), rhs.GetId());
        EXPECT_EQ (lhs.GetName(), rhs.GetName());
        Utf8String lhsUri = lhs.GetUri(), rhsUri = rhs.GetUri();
        if (lhsUri.empty() || rhsUri.empty())
            EXPECT_EQ (lhsUri.empty(), rhsUri.empty());
        else
            EXPECT_EQ (lhsUri, rhsUri);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnAuthoritiesTest, Authorities)
    {
    SetupProject();
    DgnDbR db = GetDb();

    // Every newly-created DgnDb contains exactly one "local" authority
    DgnAuthorities& auths = db.Authorities();
    DgnAuthorities::Authority localAuth = auths.Query (DgnAuthorities::Local());
    ASSERT_TRUE (localAuth.IsValid());
    ASSERT_EQ (localAuth.GetId(), DgnAuthorities::Local());
    ASSERT_TRUE (localAuth.GetName().Equals ("Local")) << "Actual: " << localAuth.GetName().c_str();
    auto localAuthId = auths.QueryAuthorityId ("Local");
    ASSERT_EQ (localAuthId, DgnAuthorities::Local()) << localAuthId.GetValueUnchecked();

    // Create some new authorities
    bvector<DgnAuthorities::Authority> expected;
    expected.push_back (localAuth);

    DgnAuthorities::Authority auth ("Auth1");
    auto authId = auths.Insert (auth);
    EXPECT_TRUE (authId.IsValid());
    EXPECT_TRUE (auth.IsValid());
    EXPECT_EQ (auth.GetId(), authId);

    expected.push_back (auth);

    auth = DgnAuthorities::Authority ("Auth2", "auth2:uri");
    authId = auths.Insert (auth);
    EXPECT_TRUE (authId.IsValid());

    expected.push_back (auth);

    size_t nAuths = 0;
    for (auto& entry : auths.MakeIterator())
        {
        nAuths++;
        auto match = std::find_if (expected.begin(), expected.end(), [&](DgnAuthorities::Authority const& arg) { return arg.GetId() == entry.GetId(); });
        ASSERT_FALSE (match == expected.end());
        Compare (*match, entry);
        }

    for (auto const& expectedAuth : expected)
        {
        authId = auths.QueryAuthorityId (expectedAuth.GetName());
        EXPECT_TRUE (authId.IsValid());
        auth = auths.Query (authId);
        EXPECT_TRUE (auth.IsValid());
        Compare (expectedAuth, auth);
        }

    EXPECT_EQ (nAuths, expected.size());

    // Names must be unique
    DgnDbStatus status;
    auth = DgnAuthorities::Authority ("Auth1", "This is a duplicate name");
    authId = auths.Insert (auth, &status);
    EXPECT_FALSE (authId.IsValid());
    EXPECT_EQ (status, DgnDbStatus::DuplicateName);

    // Update the URI of an existing item
    auth = expected.back();
    auth.SetUri ("updated URI");
    EXPECT_EQ (DgnDbStatus::Success, auths.Update (auth));
    auto updatedAuth = auths.Query (auth.GetId());
    Compare (auth, updatedAuth);
    }

