/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnAuthority_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/BlankDgnDbTestFixture.h"

USING_NAMESPACE_BENTLEY_SQLITE

#define EXPECT_STR_EQ(X,Y) { if ((X).empty() || (Y).empty()) { EXPECT_EQ ((X).empty(), (Y).empty()); } else { EXPECT_EQ ((X), (Y)); } }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnAuthoritiesTest : public BlankDgnDbTestFixture
    {

    void Compare(DgnAuthorityId id, Utf8CP name)
        {
        DgnAuthorityCPtr auth = GetDb().Authorities().GetAuthority(id);
        ASSERT_TRUE(auth.IsValid());
        EXPECT_EQ(auth->GetName(), name);

        DgnAuthorityId authId = GetDb().Authorities().QueryAuthorityId(name);
        EXPECT_TRUE(authId.IsValid());
        EXPECT_EQ(authId, id);
        }

    DgnAuthorityPtr Create(Utf8CP name, bool insert = true)
        {
        DgnAuthorityPtr auth = NamespaceAuthority::CreateNamespaceAuthority(name, GetDb());
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
    SetupProject(L"authorities.idgndb");

    // Create some new authorities
    auto auth1Id = Create("Auth1")->GetAuthorityId();
    auto auth2Id = Create("Auth2")->GetAuthorityId();

    // Test persistent
    Compare(auth1Id, "Auth1");
    Compare(auth2Id, "Auth2");

    // Names must be unique
    auto badAuth = Create("Auth1", false);
    EXPECT_EQ(DgnDbStatus::DuplicateName, badAuth->Insert());
    EXPECT_FALSE(badAuth->GetAuthorityId().IsValid());
    }


