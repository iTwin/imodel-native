/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/CodeAdmin_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>

USING_NAMESPACE_BENTLEY_DPTEST

//========================================================================================
// @bsiclass                                                    Shaun.Sewall    01/2017
//========================================================================================
struct CodeAdminTests : public DgnDbTestFixture
{
    void ValidateCodeFragmentSpec(CodeFragmentSpecCR, CodeFragmentSpec::Type, Utf8CP, bool, int minChars=0, int maxChars=CodeFragmentSpec::MAX_MaxChars);
};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2017
//---------------------------------------------------------------------------------------
void CodeAdminTests::ValidateCodeFragmentSpec(CodeFragmentSpecCR fragmentSpec, CodeFragmentSpec::Type type, Utf8CP prompt, bool inSequenceMask, int minChars, int maxChars)
    {
    BeAssert(fragmentSpec.IsValid());
    BeAssert(fragmentSpec.GetType() == type);
    BeAssert(fragmentSpec.IsInSequenceMask() == inSequenceMask);
    BeAssert(0 == strcmp(fragmentSpec.GetPrompt().c_str(), prompt));
    }

//========================================================================================
// @bsiclass                                                    Shaun.Sewall    01/2017
//========================================================================================
struct TestCodeAdmin : DgnPlatformLib::Host::CodeAdmin
{
    DEFINE_T_SUPER(DgnPlatformLib::Host::CodeAdmin)
    bmap<Utf8CP, Utf8CP, ECN::less_str> m_classToCodeSpecMap;

    TestCodeAdmin();
    DgnDbStatus _RegisterDefaultCodeSpec(Utf8CP className, Utf8CP codeSpecName) override;
    CodeSpecId _GetDefaultCodeSpecId(DgnDbR, ECN::ECClassCR) const override;
    DgnDbStatus _GetElementTypeCode(Utf8StringR, DgnElementCR, CodeFragmentSpecCR) const override;
    DgnDbStatus _ReserveCode(DgnElementCR, DgnCodeCR) const override;
    DgnCode _ReserveNextCodeInSequence(DgnElementCR, CodeSpecCR, Utf8StringCR) const override;
};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2017
//---------------------------------------------------------------------------------------
TestCodeAdmin::TestCodeAdmin()
    {
    _RegisterDefaultCodeSpec(DPTEST_SCHEMA(DPTEST_CLASS_TestSpatialLocation), DPTEST_SCHEMA(DPTEST_CLASS_TestSpatialLocation));
    _RegisterDefaultCodeSpec(DPTEST_SCHEMA(DPTEST_TEST_ELEMENT_CLASS_NAME), DPTEST_SCHEMA(DPTEST_TEST_ELEMENT_CLASS_NAME));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2017
//---------------------------------------------------------------------------------------
DgnDbStatus TestCodeAdmin::_RegisterDefaultCodeSpec(Utf8CP className, Utf8CP codeSpecName)
    {
    m_classToCodeSpecMap[className] = codeSpecName;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2017
//---------------------------------------------------------------------------------------
CodeSpecId TestCodeAdmin::_GetDefaultCodeSpecId(DgnDbR db, ECN::ECClassCR inputClass) const
    {
    if (!inputClass.Is(BIS_ECSCHEMA_NAME, BIS_CLASS_Element))
        return CodeSpecId();

    Utf8PrintfString className("%s.%s", inputClass.GetSchema().GetName().c_str(), inputClass.GetName().c_str());
    auto found = m_classToCodeSpecMap.find(className.c_str());
    if (m_classToCodeSpecMap.end() != found)
        return db.CodeSpecs().QueryCodeSpecId(found->second);

    if (inputClass.HasBaseClasses())
        return _GetDefaultCodeSpecId(db, *inputClass.GetBaseClasses()[0]);

    return T_Super::_GetDefaultCodeSpecId(db, inputClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2017
//---------------------------------------------------------------------------------------
DgnDbStatus TestCodeAdmin::_GetElementTypeCode(Utf8StringR classShortName, DgnElementCR element, CodeFragmentSpecCR fragmentSpec) const
    {
    WString className(element.GetElementClass()->GetName().c_str(), BentleyCharEncoding::Utf8);
    classShortName = Utf8String(className.substr(0,1).ToUpper());
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    03/2017
//---------------------------------------------------------------------------------------
DgnCode TestCodeAdmin::_ReserveNextCodeInSequence(DgnElementCR element, CodeSpecCR codeSpec, Utf8StringCR sequenceMask) const
    {
    if (!sequenceMask.Contains("*"))
        return DgnCode();

    static uint32_t sequenceNumber=0;
    ++sequenceNumber;
    Utf8PrintfString sequenceNumberString("%" PRIu32, sequenceNumber);

    Utf8String codeValue(sequenceMask);
    codeValue.ReplaceAll("*", sequenceNumberString.c_str());
    return DgnCode(codeSpec.GetCodeSpecId(), codeSpec.GetScopeElementId(element), codeValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    03/2017
//---------------------------------------------------------------------------------------
DgnDbStatus TestCodeAdmin::_ReserveCode(DgnElementCR element, DgnCodeCR code) const
    {
    return element.GetDgnDb().Elements().QueryElementIdByCode(code).IsValid() ? DgnDbStatus::DuplicateCode : DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2017
//---------------------------------------------------------------------------------------
TEST_F(CodeAdminTests, CodeAdmin)
    {
    m_host.SetCodeAdmin(new TestCodeAdmin());
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
    
    ECN::ECClassCP informationPartitionElementClass = m_db->Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_InformationPartitionElement);
    ECN::ECClassCP physicalPartitionClass = m_db->Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_PhysicalPartition);
    ECN::ECClassCP drawingClass = m_db->Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_Drawing);
    ASSERT_NE(nullptr, informationPartitionElementClass);
    ASSERT_NE(nullptr, physicalPartitionClass);
    ASSERT_NE(nullptr, drawingClass);

    ASSERT_TRUE(T_HOST.GetCodeAdmin()._GetDefaultCodeSpecId(*m_db, *informationPartitionElementClass).IsValid());
    ASSERT_TRUE(T_HOST.GetCodeAdmin()._GetDefaultCodeSpecId(*m_db, *physicalPartitionClass).IsValid());
    ASSERT_TRUE(T_HOST.GetCodeAdmin()._GetDefaultCodeSpecId(*m_db, *drawingClass).IsValid());

    // CodeSpec for TestSpatialLocation elements
        {
        CodeSpecPtr codeSpec = CodeSpec::Create(*m_db, DPTEST_SCHEMA(DPTEST_CLASS_TestSpatialLocation));
        ASSERT_TRUE(codeSpec.IsValid());
        ASSERT_EQ(codeSpec->GetScope().GetType(), CodeScopeSpec::Type::Repository);
        ASSERT_TRUE(codeSpec->IsRepositoryScope());
        ASSERT_TRUE(codeSpec->GetRegistrySuffix().empty());
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
        ASSERT_EQ(codeSpec->GetScope().GetType(), CodeScopeSpec::Type::Model);
        ASSERT_TRUE(codeSpec->IsModelScope());
        ASSERT_STREQ(codeSpec->GetRegistrySuffix().c_str(), "RegistrySuffix");
        codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromElementTypeCode("Enter class name"));
        codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromFixedString(":"));
        codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromPropertyValue("i", "Enter integer value"));
        codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromFixedString("_"));
        codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromPropertyValue("l", "Enter long value"));
        codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromFixedString("_"));
        codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromPropertyValue("s", "Enter string value"));
        ASSERT_EQ(DgnDbStatus::Success, codeSpec->Insert());
        }

    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(*m_db, "TestCategory");
    PhysicalModelPtr physicalModel = DgnDbTestUtils::InsertPhysicalModel(*m_db, "TestPhysicalModel");
    SpatialLocationModelPtr spatialLocationModel = DgnDbTestUtils::InsertSpatialLocationModel(*m_db, "TestSpatialLocationModel");

    for (int i=0; i<3; i++)
        {
        TestElementPtr element = TestElement::Create(*m_db, physicalModel->GetModelId(), categoryId);
        ASSERT_TRUE(element.IsValid());
        element->SetPropertyValue("i", i);
        element->SetPropertyValue("l", i*(int64_t)11LL);
        element->SetPropertyValue("s", Utf8PrintfString("String%" PRIi32, i).c_str());
        ASSERT_FALSE(element->GetCode().IsValid());
        ASSERT_EQ(DgnDbStatus::Success, element->GenerateCode());
        ASSERT_TRUE(element->GetCode().IsValid());
        ASSERT_TRUE(element->Insert().IsValid());
        ASSERT_NE(DgnDbStatus::Success, T_HOST.GetCodeAdmin()._ReserveCode(*element, element->GetCode()));
        Utf8String className;
        ASSERT_EQ(DgnDbStatus::Success, T_HOST.GetCodeAdmin()._GetElementTypeCode(className, *element, CodeFragmentSpec::FromElementTypeCode()));
        ASSERT_STREQ("T", className.c_str());
        }

    for (int i=0; i<5; i++)
        {
        Utf8PrintfString userLabel("Location%" PRIi32, i);
        TestSpatialLocationPtr element = TestSpatialLocation::Create(*spatialLocationModel, categoryId);
        ASSERT_TRUE(element.IsValid());
        element->SetUserLabel(userLabel.c_str());
        ASSERT_FALSE(element->GetCode().IsValid());
        ASSERT_EQ(DgnDbStatus::Success, element->GenerateCode());
        ASSERT_TRUE(element->GetCode().IsValid());
        ASSERT_TRUE(element->Insert().IsValid());
        Utf8String elementTypeCode;
        ASSERT_EQ(DgnDbStatus::Success, T_HOST.GetCodeAdmin()._GetElementTypeCode(elementTypeCode, *element, CodeFragmentSpec::FromElementTypeCode()));
        ASSERT_STREQ("T", elementTypeCode.c_str());
        }

    for (int i=0; i<4; i++)
        {
        Utf8PrintfString userLabel("PhysicalObject%" PRIi32, i);
        GenericPhysicalObjectPtr element = GenericPhysicalObject::Create(*physicalModel, categoryId);
        ASSERT_TRUE(element.IsValid());
        element->SetUserLabel(userLabel.c_str());
        ASSERT_FALSE(element->GetCode().IsValid());
        ASSERT_NE(DgnDbStatus::Success, element->GenerateCode());
        ASSERT_TRUE(element->Insert().IsValid());
        ASSERT_TRUE(element->GetCode().IsValid());
        Utf8String elementTypeCode;
        ASSERT_EQ(DgnDbStatus::Success, T_HOST.GetCodeAdmin()._GetElementTypeCode(elementTypeCode, *element, CodeFragmentSpec::FromElementTypeCode()));
        ASSERT_STREQ("P", elementTypeCode.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Ridha.Malik                    01/2017
//---------------------------------------------------------------------------------------
TEST_F(CodeAdminTests, GenerateCode)
    {
    m_host.SetCodeAdmin(new TestCodeAdmin());
    SetupSeedProject();

    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(*m_db, "TestCategory");
    PhysicalModelPtr physicalModel = DgnDbTestUtils::InsertPhysicalModel(*m_db, "TestPhysicalModel");

    TestElementPtr element = TestElement::Create(*m_db, physicalModel->GetModelId(), categoryId);

    ASSERT_TRUE(element.IsValid());
    element->SetPropertyValue("i", 1);
    ASSERT_FALSE(element->GetCode().IsValid());
    //GenerateCode with out inserting codespec it should return invalid
    ASSERT_EQ(DgnDbStatus::InvalidCodeSpec, element->GenerateCode());

    CodeSpecPtr codeSpec = CodeSpec::Create(*m_db, DPTEST_SCHEMA(DPTEST_TEST_ELEMENT_CLASS_NAME), CodeScopeSpec::CreateModelScope());
    ASSERT_EQ(codeSpec->GetScope().GetType(), CodeScopeSpec::Type::Model);
    ASSERT_TRUE(codeSpec->IsModelScope());
    //CanGenerateCode should return false before inserting any CodeFragmentSpec
    ASSERT_FALSE(codeSpec->CanGenerateCode());
    codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromElementTypeCode("Enter class name"));
    codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromFixedString(":"));
    //CanGenerateCode should return true after inserting any CodeFragmentSpec
    ASSERT_TRUE(codeSpec->CanGenerateCode());
    ASSERT_EQ(DgnDbStatus::Success, codeSpec->Insert()); 
    ASSERT_EQ(DgnDbStatus::Success, element->GenerateCode());
    ASSERT_TRUE(element->GetCode().IsValid());
    ASSERT_TRUE(element->Insert().IsValid());
    Utf8String elementTypeCode;
    ASSERT_EQ(DgnDbStatus::Success, T_HOST.GetCodeAdmin()._GetElementTypeCode(elementTypeCode, *element, CodeFragmentSpec::FromElementTypeCode()));
    ASSERT_STREQ("T", elementTypeCode.c_str());
    }
