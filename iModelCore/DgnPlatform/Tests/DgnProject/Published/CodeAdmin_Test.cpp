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
    void RoundtripCodeFragmentSpec(CodeFragmentSpecCR, CodeFragmentSpec::Type, Utf8CP, bool, int, int, Utf8CP fixedString=nullptr, Utf8CP propertyName=nullptr);
};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2017
//---------------------------------------------------------------------------------------
void CodeAdminTests::RoundtripCodeFragmentSpec(CodeFragmentSpecCR specIn, CodeFragmentSpec::Type type, Utf8CP prompt, bool inSequenceMask, int minChars, int maxChars, Utf8CP fixedString, Utf8CP propertyName)
    {
    CodeFragmentSpec specOut = CodeFragmentSpec::FromJson(specIn.ToJson());
    BeAssert(specIn.IsValid());
    BeAssert(specOut.IsValid());
    BeAssert(specIn.GetType() == specOut.GetType());
    BeAssert(specIn.GetType() == type);
    BeAssert(specIn.IsInSequenceMask() == specOut.IsInSequenceMask());
    BeAssert(specIn.IsInSequenceMask() == inSequenceMask);
    BeAssert(0 == strcmp(specIn.GetPrompt().c_str(), prompt));
    BeAssert(0 == strcmp(specOut.GetPrompt().c_str(), prompt));

    if (nullptr != fixedString)
        {
        BeAssert(0 == strcmp(specIn.GetFixedString().c_str(), fixedString));
        BeAssert(0 == strcmp(specOut.GetFixedString().c_str(), fixedString));
        }

    if (nullptr != propertyName)
        {
        BeAssert(0 == strcmp(specIn.GetPropertyName().c_str(), propertyName));
        BeAssert(0 == strcmp(specOut.GetPropertyName().c_str(), propertyName));
        }
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
    DgnDbStatus _GetNextSequenceNumber(Utf8StringR, DgnElementCR, CodeFragmentSpecCR, CodeScopeSpecCR, Utf8StringCR) const override;
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
// @bsimethod                                   Shaun.Sewall                    01/2017
//---------------------------------------------------------------------------------------
DgnDbStatus TestCodeAdmin::_GetNextSequenceNumber(Utf8StringR fragmentString, DgnElementCR element, CodeFragmentSpecCR fragmentSpec, CodeScopeSpecCR scopeSpec, Utf8StringCR sequenceMask) const
    {
    static uint32_t sequenceNumber=0;
    ++sequenceNumber;
    Utf8PrintfString sequenceNumberString("%" PRIu32, sequenceNumber);
    fragmentString = sequenceNumberString;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2017
//---------------------------------------------------------------------------------------
TEST_F(CodeAdminTests, CodeAdmin)
    {
    m_host.SetCodeAdmin(new TestCodeAdmin());
    SetupSeedProject();

    // CodeFragmentSpec::To/FromJson for FixedString
        {
        Utf8CP fixedString = "-";
        Utf8CP prompt = "FixedStringPrompt";
        bool inSequenceMask = true;
        CodeFragmentSpec fragmentSpec = CodeFragmentSpec::FromFixedString(fixedString, prompt);
        RoundtripCodeFragmentSpec(fragmentSpec, CodeFragmentSpec::Type::FixedString, prompt, inSequenceMask, strlen(fixedString), strlen(fixedString), fixedString);
        }
    
    // CodeFragmentSpec::To/FromJson for ElementClass
        {
        Utf8CP prompt = "ElementClassPrompt";
        bool inSequenceMask = true;
        int minChars = CodeFragmentSpec::MIN_MinChars;
        int maxChars = 1;
        CodeFragmentSpec fragmentSpec = CodeFragmentSpec::FromElementTypeCode(prompt);
        fragmentSpec.SetMaxChars(maxChars);
        RoundtripCodeFragmentSpec(fragmentSpec, CodeFragmentSpec::Type::ElementTypeCode, prompt, inSequenceMask, minChars, maxChars);
        }
    
    // CodeFragmentSpec::To/FromJson for SequenceNumber
        {
        Utf8CP prompt = "SequenceNumberPrompt";
        bool inSequenceMask = false;
        int minChars = CodeFragmentSpec::MIN_MinChars;
        int maxChars = 4;
        CodeFragmentSpec fragmentSpec = CodeFragmentSpec::FromSequenceNumber(prompt);
        fragmentSpec.SetMaxChars(maxChars);
        RoundtripCodeFragmentSpec(fragmentSpec, CodeFragmentSpec::Type::SequenceNumber, prompt, inSequenceMask, minChars, maxChars);
        }

    // CodeFragmentSpec::To/FromJson for PropertyValue
        {
        Utf8CP propertyName = "PropertyName";
        Utf8CP prompt = "PropertyValuePrompt";
        bool inSequenceMask = false;
        int minChars = CodeFragmentSpec::MIN_MinChars;
        int maxChars = CodeFragmentSpec::MAX_MaxChars;
        CodeFragmentSpec fragmentSpec = CodeFragmentSpec::FromPropertyValue(propertyName, prompt, false);
        RoundtripCodeFragmentSpec(fragmentSpec, CodeFragmentSpec::Type::PropertyValue, prompt, inSequenceMask, minChars, maxChars, nullptr, propertyName);
        }

    // CodeScopeSpec static Create* methods
        {
        CodeScopeSpec repositoryScope = CodeScopeSpec::CreateRepositoryScope();
        CodeScopeSpec modelScope = CodeScopeSpec::CreateModelScope();
        CodeScopeSpec parentElementScope = CodeScopeSpec::CreateParentElementScope();
        ASSERT_EQ(CodeScopeSpec::Type::Repository, repositoryScope.GetType());
        ASSERT_EQ(CodeScopeSpec::Type::Model, modelScope.GetType());
        ASSERT_EQ(CodeScopeSpec::Type::ParentElement, parentElementScope.GetType());
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
        codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromSequenceNumber("Enter sequence number"));
        ASSERT_EQ(DgnDbStatus::Success, codeSpec->Insert());
        }

    // CodeSpec for TestElement elements
        {
        CodeSpecPtr codeSpec = CodeSpec::Create(*m_db, DPTEST_SCHEMA(DPTEST_TEST_ELEMENT_CLASS_NAME), CodeScopeSpec::CreateModelScope(), "RegistrySuffix");
        ASSERT_TRUE(codeSpec.IsValid());
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
        element->SetPropertyValue("l", i*11LL);
        element->SetPropertyValue("s", Utf8PrintfString("String%" PRIi32, i).c_str());
        ASSERT_FALSE(element->GetCode().IsValid());
        ASSERT_EQ(DgnDbStatus::Success, element->GenerateCode());
        ASSERT_TRUE(element->GetCode().IsValid());
        ASSERT_TRUE(element->Insert().IsValid());
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
        Utf8String className;
        ASSERT_EQ(DgnDbStatus::Success, T_HOST.GetCodeAdmin()._GetElementTypeCode(className, *element, CodeFragmentSpec::FromElementTypeCode()));
        ASSERT_STREQ("T", className.c_str());
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
        Utf8String className;
        ASSERT_EQ(DgnDbStatus::Success, T_HOST.GetCodeAdmin()._GetElementTypeCode(className, *element, CodeFragmentSpec::FromElementTypeCode()));
        ASSERT_STREQ("P", className.c_str());
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Ridha.Malik                    01/2017
//---------------------------------------------------------------------------------------
TEST_F(CodeAdminTests, CodeFragmentSpecMinMaxChars)
    {
    m_host.SetCodeAdmin(new TestCodeAdmin());
    SetupSeedProject();
    // CodeFragmentSpec::Invalid min and max chars
    Utf8CP prompt = "SequenceNumberPrompt";
    bool inSequenceMask = false;
    int minChars = 0;
    int maxChars = 257;
    CodeFragmentSpec fragmentSpec = CodeFragmentSpec::FromSequenceNumber(prompt);
    fragmentSpec.SetMaxChars(maxChars);
    fragmentSpec.SetMinChars(minChars);
    ASSERT_NE(0, fragmentSpec.GetMinChars());
    ASSERT_EQ(1, fragmentSpec.GetMinChars());
    ASSERT_NE(257, fragmentSpec.GetMaxChars());
    ASSERT_EQ(256, fragmentSpec.GetMaxChars());
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
    Utf8String className;
    ASSERT_EQ(DgnDbStatus::Success, T_HOST.GetCodeAdmin()._GetElementTypeCode(className, *element, CodeFragmentSpec::FromElementTypeCode()));
    ASSERT_STREQ("T", className.c_str());
    }