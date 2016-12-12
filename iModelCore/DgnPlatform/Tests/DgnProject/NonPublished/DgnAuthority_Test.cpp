/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnAuthority_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DPTEST

#define EXPECT_STR_EQ(X,Y) { if ((X).empty() || (Y).empty()) { EXPECT_EQ ((X).empty(), (Y).empty()); } else { EXPECT_EQ ((X), (Y)); } }
/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnAuthoritiesTest : public DgnDbTestFixture
    {
    void Compare(DgnAuthorityId id, Utf8CP name)
        {
        DgnAuthorityCPtr auth = GetDgnDb().Authorities().GetAuthority(id);
        ASSERT_TRUE(auth.IsValid());
        EXPECT_EQ(auth->GetName(), name);

        DgnAuthorityId authId = GetDgnDb().Authorities().QueryAuthorityId(name);
        EXPECT_TRUE(authId.IsValid());
        EXPECT_EQ(authId, id);
        }

    DatabaseScopeAuthorityPtr Create(Utf8CP name, bool insert = true)
        {
        auto auth = DatabaseScopeAuthority::Create(name, GetDgnDb());
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
        DgnCode::Iterator iter = DgnCode::MakeIterator(GetDgnDb(), options);
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
* @bsimethod                                    Ridha.Malik                      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbPtr initDb(WCharCP fileName, Db::OpenMode mode = Db::OpenMode::ReadWrite, bool needBriefCase = true)
    {
    BeFileName dbName;
    EXPECT_TRUE(DgnDbStatus::Success == DgnDbTestFixture::GetSeedDbCopy(dbName, fileName));
    DgnDbPtr db;
    DgnDbTestFixture::OpenDb(db, dbName, mode, needBriefCase);
    return db;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
//---------------------------------------------------------------------------------------
static DgnDbPtr openCopyOfDb(WCharCP destName, DgnDb::OpenMode mode = Db::OpenMode::ReadWrite, bool importDummySchemaFirst = true)
    {
    DgnDbPtr db2;
    db2 = initDb(destName,mode,true);
    if (!db2.IsValid())
        return nullptr;
    if (importDummySchemaFirst)
        DgnPlatformTestDomain::ImportDummySchema(*db2);
    DgnPlatformTestDomain::ImportSchema(*db2);
    return db2;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnAuthoritiesTest, Authorities)
    {
    SetupSeedProject();

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
    SetupSeedProject();
    DgnDbR db = GetDgnDb();

    EXPECT_TRUE(CodeExists(db.Elements().GetElement(db.Elements().GetDictionaryPartitionId())->GetCode()));

    AnnotationTextStyle style(db);
    style.SetName("MyStyle");
    DgnCode originalStyleCode = style.GetCode();
    EXPECT_TRUE(style.Insert().IsValid());

    EXPECT_TRUE(CodeExists(originalStyleCode));

    AnnotationTextStylePtr pStyle = AnnotationTextStyle::Get(db, "MyStyle")->CreateCopy();
    pStyle->SetName("RenamedStyle");
    EXPECT_TRUE(pStyle->Update().IsValid());

    EXPECT_TRUE(CodeExists(pStyle->GetCode()));
    EXPECT_FALSE(CodeExists(originalStyleCode));

    // Insert element with empty code
    EXPECT_TRUE(InsertElement().IsValid());

    // Test with various options
    DgnCode emptyCode = DgnCode::CreateEmpty();
    DgnCode elementCode = pStyle->GetCode();

    EXPECT_TRUE(CodeExists(emptyCode, IteratorOptions(true)));
    EXPECT_FALSE(CodeExists(emptyCode, IteratorOptions(false)));

    EXPECT_TRUE(CodeExists(elementCode, IteratorOptions(true)));
    EXPECT_TRUE(CodeExists(elementCode, IteratorOptions(false)));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnAuthoritiesTest, RegenerateCode)
    {
    SetupSeedProject();
    PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*m_db, "testcode");
    ASSERT_TRUE(model->IsPhysicalModel());
    DgnElementId eleid = model->GetModeledElementId();
    DgnElementCPtr ele = m_db->Elements().GetElement(eleid);
    DgnAuthorityCPtr elea = ele->GetCodeAuthority();
    DgnCode partitionCode1 = InformationPartitionElement::CreateCode(*m_db->Elements().GetRootSubject(), "testcode");
    EXPECT_TRUE(DgnDbStatus::Success==elea->RegenerateCode(partitionCode1, *ele));
    EXPECT_TRUE(partitionCode1.GetValue()== "testcode");
    EXPECT_TRUE(CodeExists(partitionCode1, IteratorOptions(true)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnAuthoritiesTest, ImportAuthority)
    {
    SetupSeedProject();
    // Create new authority
    DatabaseScopeAuthorityPtr auth1 = Create("Auth1");
    DgnAuthorityId auth1Id=auth1->GetAuthorityId();
    // Test persistent
    Compare(auth1Id, "Auth1");
    DgnDbPtr db2 = openCopyOfDb(L"AuthorityImported.bim");
    ASSERT_TRUE(db2.IsValid());
    DgnImportContext import3(*m_db, *db2);
    DgnDbStatus status;
    DgnAuthorityPtr autp = DgnAuthority::Import(&status,*auth1,import3);
    ASSERT_TRUE(status==DgnDbStatus::Success);
    ASSERT_TRUE(autp.IsValid());
    //Verify Imported Authority
    Compare(autp->GetAuthorityId(), "Auth1");
    // Try to import duplicade authority code
    autp = DgnAuthority::Import(&status, *auth1, import3);
    ASSERT_TRUE(status == DgnDbStatus::DuplicateName);
    ASSERT_FALSE(autp.IsValid());
    db2->SaveChanges();
    }