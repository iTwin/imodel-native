/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
struct DgnCodeSpecsTest : public DgnDbTestFixture
    {
    void Compare(CodeSpecId id, Utf8CP name)
        {
        CodeSpecCPtr codeSpec = GetDgnDb().CodeSpecs().GetCodeSpec(id);
        ASSERT_TRUE(codeSpec.IsValid());
        EXPECT_EQ(codeSpec->GetName(), name);

        CodeSpecId codeSpecId = GetDgnDb().CodeSpecs().QueryCodeSpecId(name);
        EXPECT_TRUE(codeSpecId.IsValid());
        EXPECT_EQ(codeSpecId, id);
        }

    CodeSpecPtr Create(Utf8CP name, bool insert = true)
        {
        auto codeSpec = CodeSpec::Create(GetDgnDb(), name);
        if (insert)
            {
            EXPECT_EQ(DgnDbStatus::Success, codeSpec->Insert());
            auto codeSpecId = codeSpec->GetCodeSpecId();
            EXPECT_TRUE(codeSpecId.IsValid());
            }

        return codeSpec;
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
static DgnDbPtr openCopyOfDb(WCharCP destName, DgnDb::OpenMode mode = Db::OpenMode::ReadWrite)
    {
    DgnDbPtr db2;
    db2 = initDb(destName,mode,true);
    if (!db2.IsValid())
        return nullptr;
    DgnPlatformTestDomain::GetDomain().ImportSchema(*db2);
    return db2;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnCodeSpecsTest, CodeSpecs)
    {
    SetupSeedProject();

    // Create some new CodeSpecs
    auto codeSpec1Id = Create("CodeSpec1")->GetCodeSpecId();
    auto codeSpec2Id = Create("CodeSpec2")->GetCodeSpecId();

    // Test persistent
    Compare(codeSpec1Id, "CodeSpec1");
    Compare(codeSpec2Id, "CodeSpec2");

    // Names must be unique
    auto badAuth = Create("CodeSpec1", false);
    EXPECT_EQ(DgnDbStatus::DuplicateName, badAuth->Insert());
    EXPECT_FALSE(badAuth->GetCodeSpecId().IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnCodeSpecsTest, IterateCodes)
    {
    SetupSeedProject();
    DgnDbR db = GetDgnDb();

    EXPECT_TRUE(CodeExists(db.Elements().GetElement(db.Elements().GetDictionaryPartitionId())->GetCode()));

    AnnotationTextStyle style(db.GetDictionaryModel());
    style.SetName("MyStyle");
    DgnCode originalStyleCode = style.GetCode();
    EXPECT_TRUE(style.Insert().IsValid());

    EXPECT_TRUE(CodeExists(originalStyleCode));

    AnnotationTextStylePtr pStyle = AnnotationTextStyle::Get(db.GetDictionaryModel(), "MyStyle")->CreateCopy();
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
TEST_F(DgnCodeSpecsTest, ImportCodeSpec)
    {
    SetupSeedProject();
    // Create new CodeSpec
    CodeSpecPtr codeSpec1 = Create("CodeSpec1");
    CodeSpecId codeSpec1Id=codeSpec1->GetCodeSpecId();
    // Test persistent
    Compare(codeSpec1Id, "CodeSpec1");
    DgnDbPtr db2 = openCopyOfDb(L"CodeSpecImported.bim");
    ASSERT_TRUE(db2.IsValid());
    DgnImportContext import3(*m_db, *db2);
    DgnDbStatus status;
    CodeSpecPtr autp = CodeSpec::Import(&status,*codeSpec1,import3);
    ASSERT_TRUE(status==DgnDbStatus::Success);
    ASSERT_TRUE(autp.IsValid());
    //Verify Imported CodeSpec
    Compare(autp->GetCodeSpecId(), "CodeSpec1");
    // Try to import duplicate CodeSpec name
    autp = CodeSpec::Import(&status, *codeSpec1, import3);
    ASSERT_TRUE(status == DgnDbStatus::DuplicateName);
    ASSERT_FALSE(autp.IsValid());
    db2->SaveChanges();
    }
