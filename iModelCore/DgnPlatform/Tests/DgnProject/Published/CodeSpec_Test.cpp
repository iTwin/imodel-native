/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/CodeSpec_Test.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>

USING_NAMESPACE_BENTLEY_DPTEST

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
