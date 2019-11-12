/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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

//========================================================================================
// @bsiclass                                                    Shaun.Sewall    01/2017
//========================================================================================
struct CodeSpecTests : public DgnDbTestFixture
{
    void ValidateCodeFragmentSpec(CodeFragmentSpecCR, CodeFragmentSpec::Type, Utf8CP, bool, size_t minChars=0, size_t maxChars=CodeFragmentSpec::MAX_MaxChars);
};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2017
//---------------------------------------------------------------------------------------
void CodeSpecTests::ValidateCodeFragmentSpec(CodeFragmentSpecCR fragmentSpec, CodeFragmentSpec::Type type, Utf8CP prompt, bool inSequenceMask, size_t minChars, size_t maxChars)
    {
    BeAssert(fragmentSpec.IsValid());
    BeAssert(fragmentSpec.GetType() == type);
    BeAssert(fragmentSpec.IsInSequenceMask() == inSequenceMask);
    BeAssert(0 == strcmp(fragmentSpec.GetPrompt().c_str(), prompt));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2017
//---------------------------------------------------------------------------------------
TEST_F(CodeSpecTests, CRUD)
    {
    SetupSeedProject();

    // CodeFragmentSpec::Type::FixedString
        {
        Utf8CP fixedString = "-";
        Utf8CP prompt = "FixedStringPrompt";
        bool inSequenceMask = true;
        CodeFragmentSpec fragmentSpec = CodeFragmentSpec::FromFixedString(fixedString, prompt);
        ValidateCodeFragmentSpec(fragmentSpec, CodeFragmentSpec::Type::FixedString, prompt, inSequenceMask, strlen(fixedString), strlen(fixedString));
        ASSERT_TRUE(fragmentSpec.IsFixedString());
        ASSERT_STREQ(fragmentSpec.GetFixedString().c_str(), fixedString);
        }
    
    // CodeFragmentSpec::Type::ElementTypeCode
        {
        Utf8CP prompt = "ElementTypeCodePrompt";
        bool inSequenceMask = true;
        int minChars = 1;
        int maxChars = 1;
        CodeFragmentSpec fragmentSpec = CodeFragmentSpec::FromElementTypeCode(prompt);
        fragmentSpec.SetMaxChars(maxChars);
        ValidateCodeFragmentSpec(fragmentSpec, CodeFragmentSpec::Type::ElementTypeCode, prompt, inSequenceMask, minChars, maxChars);
        ASSERT_TRUE(fragmentSpec.IsElementTypeCode());
        }
    
    // CodeFragmentSpec::Type::Sequence
        {
        Utf8CP prompt = "SequencePrompt";
        bool inSequenceMask = false;
        int minChars = 1;
        int maxChars = 4;
        CodeFragmentSpec fragmentSpec = CodeFragmentSpec::FromSequence(prompt);
        fragmentSpec.SetMaxChars(maxChars);
        ValidateCodeFragmentSpec(fragmentSpec, CodeFragmentSpec::Type::Sequence, prompt, inSequenceMask, minChars, maxChars);
        ASSERT_TRUE(fragmentSpec.IsSequence());
        ASSERT_EQ(fragmentSpec.GetStartNumber(), 1) << "Unexpected default value";
        ASSERT_EQ(fragmentSpec.GetNumberGap(), 1) << "Unexpected default value";

        fragmentSpec.SetStartNumber(100);
        fragmentSpec.SetNumberGap(10);
        ASSERT_EQ(fragmentSpec.GetStartNumber(), 100);
        ASSERT_EQ(fragmentSpec.GetNumberGap(), 10);
        }

    // CodeFragmentSpec::Type::PropertyValue
        {
        Utf8CP propertyName = "PropertyName";
        Utf8CP prompt = "PropertyValuePrompt";
        bool inSequenceMask = false;
        int minChars = 0;
        int maxChars = CodeFragmentSpec::MAX_MaxChars;
        CodeFragmentSpec fragmentSpec = CodeFragmentSpec::FromPropertyValue(propertyName, prompt, inSequenceMask);
        ValidateCodeFragmentSpec(fragmentSpec, CodeFragmentSpec::Type::PropertyValue, prompt, inSequenceMask, minChars, maxChars);
        ASSERT_TRUE(fragmentSpec.IsPropertyValue());
        ASSERT_STREQ(fragmentSpec.GetPropertyName().c_str(), propertyName);
        }

    // Invalid min and max chars
        {
        int invalidMaxChars = CodeFragmentSpec::MAX_MaxChars + 1;
        CodeFragmentSpec fragmentSpec = CodeFragmentSpec::FromSequence();
        fragmentSpec.SetMinChars(invalidMaxChars);
        fragmentSpec.SetMaxChars(invalidMaxChars);
        ASSERT_EQ(fragmentSpec.GetMinChars(), 0);
        ASSERT_EQ(fragmentSpec.GetMaxChars(), CodeFragmentSpec::MAX_MaxChars);

        fragmentSpec.SetMaxChars(16);
        fragmentSpec.SetMinChars(17);
        ASSERT_EQ(fragmentSpec.GetMinChars(), 0);
        ASSERT_EQ(fragmentSpec.GetMaxChars(), 16);
        }

    // CodeScopeSpec static Create* methods
        {
        CodeScopeSpec repositoryScope = CodeScopeSpec::CreateRepositoryScope();
        CodeScopeSpec modelScope = CodeScopeSpec::CreateModelScope();
        CodeScopeSpec parentElementScope = CodeScopeSpec::CreateParentElementScope();
        Utf8CP relationship = BIS_SCHEMA("ShouldBeRelationshipName"); // in production this would be a valid ECRelationshipClass name
        CodeScopeSpec relatedElementScope = CodeScopeSpec::CreateRelatedElementScope(relationship);
        ASSERT_EQ(CodeScopeSpec::Type::Repository, repositoryScope.GetType());
        ASSERT_EQ(CodeScopeSpec::Type::Model, modelScope.GetType());
        ASSERT_EQ(CodeScopeSpec::Type::ParentElement, parentElementScope.GetType());
        ASSERT_EQ(CodeScopeSpec::Type::RelatedElement, relatedElementScope.GetType());
        ASSERT_STREQ(relationship, relatedElementScope.GetRelationship().c_str());
        }
    
    // CodeSpec for TestSpatialLocation elements
        {
        CodeSpecPtr codeSpec = CodeSpec::Create(*m_db, DPTEST_SCHEMA(DPTEST_CLASS_TestSpatialLocation));
        ASSERT_TRUE(codeSpec.IsValid());
        ASSERT_EQ(codeSpec->GetScope().GetType(), CodeScopeSpec::Type::Repository);
        ASSERT_TRUE(codeSpec->IsRepositoryScope());
        ASSERT_TRUE(codeSpec->GetRegistrySuffix().empty());
        ASSERT_TRUE(codeSpec->IsManagedWithDgnDb());
        codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromElementTypeCode("Enter class name"));
        codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromFixedString("-"));
        codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromPropertyValue("UserLabel", "Enter UserLabel value", false));
        codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromFixedString("-"));
        codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromSequence("Enter sequence number"));
        ASSERT_EQ(DgnDbStatus::Success, codeSpec->Insert());
        }

    // CodeSpec for TestElement elements
        {
        CodeSpecPtr codeSpec = CodeSpec::Create(*m_db, DPTEST_SCHEMA(DPTEST_TEST_ELEMENT_CLASS_NAME), CodeScopeSpec::CreateModelScope());
        ASSERT_TRUE(codeSpec.IsValid());
        codeSpec->SetRegistrySuffix("RegistrySuffix");
        codeSpec->SetIsManagedWithDgnDb(false);
        ASSERT_EQ(codeSpec->GetScope().GetType(), CodeScopeSpec::Type::Model);
        ASSERT_TRUE(codeSpec->IsModelScope());
        ASSERT_STREQ(codeSpec->GetRegistrySuffix().c_str(), "RegistrySuffix");
        ASSERT_FALSE(codeSpec->IsManagedWithDgnDb());
        codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromElementTypeCode("Enter class name"));
        codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromFixedString(":"));
        codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromPropertyValue("i", "Enter integer value"));
        codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromFixedString("_"));
        codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromPropertyValue("l", "Enter long value"));
        codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromFixedString("_"));
        codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromPropertyValue("s", "Enter string value"));
        ASSERT_EQ(DgnDbStatus::Success, codeSpec->Insert());
        }
    }
