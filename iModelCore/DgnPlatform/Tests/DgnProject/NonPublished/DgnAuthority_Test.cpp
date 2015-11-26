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

//=======================================================================================
// Test Authority
// @bsistruct                                                    Umar.Hayat   10/15
//=======================================================================================
//struct TestAuthority : DgnAuthority
//{
//    DEFINE_T_SUPER(DgnAuthority)
//
//    TestAuthority(CreateParams const& params) : T_SUPER(params) { }
//
//    DgnAuthority::Code CreateCode(Utf8StringCR value, Utf8StringCR nameSpace = "testnamespace::") const { return T_Super::CreateCode(value, nameSpace); }
//
//    DGNPLATFORM_EXPORT static RefCountedPtr<NamespaceAuthority> CreateNamespaceAuthority(Utf8CP name, DgnDbR dgndb, Utf8CP uri = nullptr);
//    DGNPLATFORM_EXPORT static DgnAuthority::Code CreateCode(Utf8CP authorityName, Utf8StringCR value, DgnDbR dgndb, Utf8StringCR nameSpace = "");
//};
//
//namespace dgn_AuthorityHandler
//{
//    struct Test : Authority
//    {
//        AUTHORITYHANDLER_DECLARE_MEMBERS("TestAuthority", TestAuthority, Namespace, Authority, DGNPLATFORM_EXPORT)
//    };
//};
/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnAuthoritiesTest : public BlankDgnDbTestFixture
    {

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
    SetupProject(L"authorities.idgndb");

    // Create some new authorities
    auto auth1Id = Create("Auth1")->GetAuthorityId();
    auto auth2Id = Create("Auth2", "auth2:uri")->GetAuthorityId();
    //auto auth3Id = Create("Auth3")->GetAuthorityId();
    //auto auth3Id = Create("Auth4", "auth2:uri")->GetAuthorityId();

    // Test persistent
    Compare(auth1Id, "Auth1", nullptr);
    Compare(auth2Id, "Auth2", "auth2:uri");

    // Names must be unique
    auto badAuth = Create("Auth1", "This is a duplicate name", false);
    EXPECT_EQ(DgnDbStatus::DuplicateName, badAuth->Insert());
    EXPECT_FALSE(badAuth->GetAuthorityId().IsValid());
    }

