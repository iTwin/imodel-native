/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DPTEST

#define EXPECT_STR_EQ(X,Y) { if ((X).empty() || (Y).empty()) { EXPECT_EQ ((X).empty(), (Y).empty()); } else { EXPECT_EQ ((X), (Y)); } }
/*---------------------------------------------------------------------------------**//**
* @bsistruct
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
        auto codeSpec = CodeSpec::CreateRepositorySpec(GetDgnDb(), name);
        if (insert)
            {
            EXPECT_EQ(DgnDbStatus::Success, codeSpec->Insert());
            auto codeSpecId = codeSpec->GetCodeSpecId();
            EXPECT_TRUE(codeSpecId.IsValid());
            }

        return codeSpec;
        }

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnCodeSpecsTest, WhitespaceHandling)
    {
    Utf8CP value = "Value";
    ASSERT_STREQ(value, DgnCodeValue("Value", DgnCodeValue::Behavior::TrimUnicodeWhitespace).GetUtf8CP());
    ASSERT_STREQ(value, DgnCodeValue(" Value", DgnCodeValue::Behavior::TrimUnicodeWhitespace).GetUtf8CP());
    ASSERT_STREQ(value, DgnCodeValue("Value ", DgnCodeValue::Behavior::TrimUnicodeWhitespace).GetUtf8CP());
    ASSERT_STREQ(value, DgnCodeValue(" Value ", DgnCodeValue::Behavior::TrimUnicodeWhitespace).GetUtf8CP());
    ASSERT_STREQ(value, DgnCodeValue("\tValue", DgnCodeValue::Behavior::TrimUnicodeWhitespace).GetUtf8CP());
    ASSERT_STREQ(value, DgnCodeValue("Value\t", DgnCodeValue::Behavior::TrimUnicodeWhitespace).GetUtf8CP());
    ASSERT_STREQ(value, DgnCodeValue("\tValue\t ", DgnCodeValue::Behavior::TrimUnicodeWhitespace).GetUtf8CP());
    ASSERT_STREQ(value, DgnCodeValue("\nValue", DgnCodeValue::Behavior::TrimUnicodeWhitespace).GetUtf8CP());
    ASSERT_STREQ(value, DgnCodeValue("Value\n", DgnCodeValue::Behavior::TrimUnicodeWhitespace).GetUtf8CP());
    ASSERT_STREQ(value, DgnCodeValue("\nValue\n", DgnCodeValue::Behavior::TrimUnicodeWhitespace).GetUtf8CP());
    ASSERT_STREQ(value, DgnCodeValue("  \t\nValue \t \n  ", DgnCodeValue::Behavior::TrimUnicodeWhitespace).GetUtf8CP());
    ASSERT_TRUE(DgnCodeValue().empty());
    ASSERT_TRUE(DgnCodeValue(" ", DgnCodeValue::Behavior::TrimUnicodeWhitespace).empty());
    ASSERT_TRUE(DgnCodeValue(" \t\n ", DgnCodeValue::Behavior::TrimUnicodeWhitespace).empty());

    #define ASSERT_EXACT(v) ASSERT_STREQ((v), DgnCodeValue((v), DgnCodeValue::Behavior::Exact).GetUtf8CP())
    ASSERT_EXACT(value);
    ASSERT_EXACT(value);
    ASSERT_EXACT(value);
    ASSERT_EXACT(value);
    ASSERT_EXACT(value);
    ASSERT_EXACT(value);
    ASSERT_EXACT(value);
    ASSERT_EXACT(value);
    ASSERT_EXACT(value);
    ASSERT_EXACT(value);
    ASSERT_EXACT(value);
    ASSERT_TRUE(DgnCodeValue().empty());
    ASSERT_EXACT(" ");
    ASSERT_TRUE(" \t\n ");
    }

//========================================================================================
// @bsiclass
//========================================================================================
struct CodeSpecTests : public DgnDbTestFixture
{
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(CodeSpecTests, CRUD)
    {
    SetupSeedProject();

    // CodeScopeSpec static Create* methods
        {
        CodeScopeSpec repositoryScope = CodeScopeSpec::CreateRepositoryScope();
        CodeScopeSpec modelScope = CodeScopeSpec::CreateModelScope();
        CodeScopeSpec parentElementScope = CodeScopeSpec::CreateParentElementScope();
        ASSERT_EQ(CodeScopeSpec::Type::Repository, repositoryScope.GetType());
        ASSERT_EQ(CodeScopeSpec::Type::Model, modelScope.GetType());
        ASSERT_EQ(CodeScopeSpec::Type::ParentElement, parentElementScope.GetType());
        }

    // CodeSpec for TestSpatialLocation elements
        {
        CodeSpecPtr codeSpec = CodeSpec::CreateRepositorySpec(*m_db, DPTEST_SCHEMA(DPTEST_CLASS_TestSpatialLocation));
        ASSERT_TRUE(codeSpec.IsValid());
        ASSERT_EQ(codeSpec->GetScope().GetType(), CodeScopeSpec::Type::Repository);
        ASSERT_TRUE(codeSpec->IsRepositoryScope());
        ASSERT_EQ(DgnDbStatus::Success, codeSpec->Insert());
        }

    // CodeSpec for TestElement elements
        {
        CodeSpecPtr codeSpec = CodeSpec::Create(*m_db, DPTEST_SCHEMA(DPTEST_TEST_ELEMENT_CLASS_NAME), CodeScopeSpec::CreateModelScope());
        ASSERT_TRUE(codeSpec.IsValid());
        ASSERT_EQ(codeSpec->GetScope().GetType(), CodeScopeSpec::Type::Model);
        ASSERT_TRUE(codeSpec->IsModelScope());
        ASSERT_EQ(DgnDbStatus::Success, codeSpec->Insert());
        }
    }
