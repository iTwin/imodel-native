/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnAuthority_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

    typedef DgnCode::Iterator::Options IteratorOptions;

    bool CodeExists(DgnCodeCR toFind, IteratorOptions options=IteratorOptions())
        {
        DgnCode::Iterator iter = DgnCode::MakeIterator(GetDb(), options);
        for (auto const& entry : iter)
            {
            DgnCode code = entry.GetCode();
            if (code == toFind)
                return true;
            }

        return false;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnAuthoritiesTest, Authorities)
    {
    SetupProject(L"authorities.ibim");

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnAuthoritiesTest, IterateCodes)
    {
    SetupProject(L"IterateCodes.ibim");

    EXPECT_TRUE(CodeExists(GetDb().GetDictionaryModel().GetCode()));

    AnnotationTextStyle style(GetDb());
    style.SetName("MyStyle");
    DgnCode originalStyleCode = style.GetCode();
    EXPECT_TRUE(style.Insert().IsValid());

    EXPECT_TRUE(CodeExists(originalStyleCode));

    AnnotationTextStylePtr pStyle = AnnotationTextStyle::Get(GetDb(), "MyStyle")->CreateCopy();
    pStyle->SetName("RenamedStyle");
    EXPECT_TRUE(pStyle->Update().IsValid());

    EXPECT_TRUE(CodeExists(pStyle->GetCode()));
    EXPECT_FALSE(CodeExists(originalStyleCode));

    // Test with various options
#ifdef TEST_EMPTY_CODES
    DgnCode emptyCode = DgnCode::CreateEmpty();
#endif
    DgnCode modelCode = GetDb().GetDictionaryModel().GetCode();
    DgnCode elementCode = pStyle->GetCode();

    typedef DgnCode::Iterator::Include Include;

#ifdef TEST_EMPTY_CODES
    EXPECT_TRUE(CodeExists(emptyCode, IteratorOptions(Include::Both, true)));
    EXPECT_FALSE(CodeExists(emptyCode, IteratorOptions(Include::Both, false)));
    EXPECT_TRUE(CodeExists(emptyCode, IteratorOptions(Include::Models, true)));
    EXPECT_FALSE(CodeExists(emptyCode, IteratorOptions(Include::Models, false)));
    EXPECT_TRUE(CodeExists(emptyCode, IteratorOptions(Include::Elements, true)));
    EXPECT_FALSE(CodeExists(emptyCode, IteratorOptions(Include::Elements, false)));
#endif

    EXPECT_TRUE(CodeExists(modelCode, IteratorOptions(Include::Models, false)));
    EXPECT_TRUE(CodeExists(modelCode, IteratorOptions(Include::Models, true)));
    EXPECT_TRUE(CodeExists(modelCode, IteratorOptions(Include::Both, false)));
    EXPECT_TRUE(CodeExists(modelCode, IteratorOptions(Include::Both, true)));
    EXPECT_FALSE(CodeExists(modelCode, IteratorOptions(Include::Elements, false)));
    EXPECT_FALSE(CodeExists(modelCode, IteratorOptions(Include::Elements, true)));

    EXPECT_TRUE(CodeExists(elementCode, IteratorOptions(Include::Both, true)));
    EXPECT_TRUE(CodeExists(elementCode, IteratorOptions(Include::Both, false)));
    EXPECT_TRUE(CodeExists(elementCode, IteratorOptions(Include::Elements, true)));
    EXPECT_TRUE(CodeExists(elementCode, IteratorOptions(Include::Elements, false)));
    EXPECT_FALSE(CodeExists(elementCode, IteratorOptions(Include::Models, true)));
    EXPECT_FALSE(CodeExists(elementCode, IteratorOptions(Include::Models, false)));
    }

