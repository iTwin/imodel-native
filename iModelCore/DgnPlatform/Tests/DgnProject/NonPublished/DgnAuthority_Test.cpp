/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnAuthority_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

USING_NAMESPACE_BENTLEY_SQLITE

#define EXPECT_STR_EQ(X,Y) { if ((X).empty() || (Y).empty()) { EXPECT_EQ ((X).empty(), (Y).empty()); } else { EXPECT_EQ ((X), (Y)); } }

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

    void Compare(DgnAuthorityId id, Utf8CP name, Utf8StringCR uri)
        {
        DgnAuthorityCPtr auth = GetDb().Authorities().GetAuthority(id);
        ASSERT_TRUE(auth.IsValid());
        EXPECT_EQ(auth->GetName(), name);
        EXPECT_STR_EQ(auth->GetUri(), uri);

        DgnAuthorityId authId = GetDb().Authorities().QueryAuthorityId(name);
        EXPECT_TRUE(authId.IsValid());
        EXPECT_EQ(authId, id);
        }

    DgnAuthorityPtr Create(Utf8CP name, Utf8CP uri = nullptr, bool insert = true)
        {
        DgnAuthorityPtr auth = NamespaceAuthority::CreateNamespaceAuthority(name, GetDb(), uri);
        if (insert)
            {
            EXPECT_EQ(DgnDbStatus::Success, auth->Insert());
            auto authId = auth->GetAuthorityId();
            EXPECT_TRUE(authId.IsValid());
            }

        return auth;
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
    DgnAuthorityCPtr localAuth = auths.GetAuthority(DgnAuthority::LocalId());
    ASSERT_TRUE (localAuth.IsValid());
    ASSERT_EQ (localAuth->GetAuthorityId(), DgnAuthority::LocalId());
    ASSERT_TRUE (localAuth->GetName().Equals ("Local")) << "Actual: " << localAuth->GetName().c_str();
    auto localAuthId = auths.QueryAuthorityId ("Local");
    ASSERT_EQ (localAuthId, DgnAuthority::LocalId()) << localAuthId.GetValueUnchecked();

    // Create some new authorities
    auto auth1Id = Create("Auth1")->GetAuthorityId();
    auto auth2Id = Create("Auth2", "auth2:uri")->GetAuthorityId();

    // Test persistent
    Compare(auth1Id, "Auth1", nullptr);
    Compare(auth2Id, "Auth2", "auth2:uri");
    Compare(DgnAuthority::LocalId(), "Local", nullptr);

    // Names must be unique
    auto badAuth = Create("Auth1", "This is a duplicate name", false);
    EXPECT_EQ(DgnDbStatus::DuplicateName, badAuth->Insert());
    EXPECT_FALSE(badAuth->GetAuthorityId().IsValid());
    }

