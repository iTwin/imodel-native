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
};

//========================================================================================
// @bsiclass                                                    Shaun.Sewall    01/2017
//========================================================================================
struct TestCodeAdmin : DgnPlatformLib::Host::CodeAdmin
{
    DEFINE_T_SUPER(DgnPlatformLib::Host::CodeAdmin)
    bmap<Utf8CP, Utf8CP, ECN::less_str> m_classToAuthorityMap;

    TestCodeAdmin();
    DgnDbStatus _RegisterDefaultCodeSpec(Utf8CP className, Utf8CP codeSpecName) override;
    CodeSpecId _GetDefaultCodeSpecId(DgnDbR, ECN::ECClassCR) const override;
    DgnDbStatus _GetElementClassName(Utf8StringR, DgnElementCR, CodeFragmentSpecCR) const override;
    DgnDbStatus _GetNextSequenceNumber(Utf8StringR, DgnElementCR, CodeSpecCR, CodeFragmentSpecCR) const override;
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
    m_classToAuthorityMap[className] = codeSpecName;
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
    auto found = m_classToAuthorityMap.find(className.c_str());
    if (m_classToAuthorityMap.end() != found)
        return db.Authorities().QueryAuthorityId(found->second);

    if (inputClass.HasBaseClasses())
        return _GetDefaultCodeSpecId(db, *inputClass.GetBaseClasses()[0]);

    return T_Super::_GetDefaultCodeSpecId(db, inputClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2017
//---------------------------------------------------------------------------------------
DgnDbStatus TestCodeAdmin::_GetElementClassName(Utf8StringR classShortName, DgnElementCR element, CodeFragmentSpecCR fragmentSpec) const
    {
    WString className(element.GetElementClass()->GetName().c_str(), BentleyCharEncoding::Utf8);
    classShortName = Utf8String(className.substr(0,1).ToUpper());
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2017
//---------------------------------------------------------------------------------------
DgnDbStatus TestCodeAdmin::_GetNextSequenceNumber(Utf8StringR fragmentString, DgnElementCR element, CodeSpecCR codeSpec, CodeFragmentSpecCR fragmentSpec) const
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
        CodeFragmentSpec fragmentSpec1 = CodeFragmentSpec::FromFixedString(fixedString, prompt);
        CodeFragmentSpec fragmentSpec2 = CodeFragmentSpec::FromJson(fragmentSpec1.ToJson());
        ASSERT_TRUE(fragmentSpec1.IsValid());
        ASSERT_TRUE(fragmentSpec2.IsValid());
        ASSERT_EQ(fragmentSpec1.GetType(), fragmentSpec2.GetType());
        ASSERT_EQ(fragmentSpec1.GetType(), CodeFragmentSpec::Type::FixedString);
        ASSERT_EQ(fragmentSpec1.GetInSequenceMask(), fragmentSpec2.GetInSequenceMask());
        ASSERT_EQ(fragmentSpec1.GetInSequenceMask(), true);
        ASSERT_STREQ(fragmentSpec1.GetPrompt().c_str(), prompt);
        ASSERT_STREQ(fragmentSpec2.GetPrompt().c_str(), prompt);
        ASSERT_STREQ(fragmentSpec1.GetFixedString().c_str(), fixedString);
        ASSERT_STREQ(fragmentSpec2.GetFixedString().c_str(), fixedString);
        }
    
    // CodeFragmentSpec::To/FromJson for ElementClass
        {
        Utf8CP prompt = "ElementClassPrompt";
        CodeFragmentSpec fragmentSpec1 = CodeFragmentSpec::FromElementClass(prompt);
        CodeFragmentSpec fragmentSpec2 = CodeFragmentSpec::FromJson(fragmentSpec1.ToJson());
        ASSERT_TRUE(fragmentSpec1.IsValid());
        ASSERT_TRUE(fragmentSpec2.IsValid());
        ASSERT_EQ(fragmentSpec1.GetType(), fragmentSpec2.GetType());
        ASSERT_EQ(fragmentSpec1.GetType(), CodeFragmentSpec::Type::ElementClass);
        ASSERT_EQ(fragmentSpec1.GetInSequenceMask(), fragmentSpec2.GetInSequenceMask());
        ASSERT_EQ(fragmentSpec1.GetInSequenceMask(), true);
        ASSERT_STREQ(fragmentSpec1.GetPrompt().c_str(), prompt);
        ASSERT_STREQ(fragmentSpec2.GetPrompt().c_str(), prompt);
        }
    
    // CodeFragmentSpec::To/FromJson for SequenceNumber
        {
        Utf8CP prompt = "SequenceNumberPrompt";
        CodeFragmentSpec fragmentSpec1 = CodeFragmentSpec::FromSequenceNumber(prompt);
        CodeFragmentSpec fragmentSpec2 = CodeFragmentSpec::FromJson(fragmentSpec1.ToJson());
        ASSERT_TRUE(fragmentSpec1.IsValid());
        ASSERT_TRUE(fragmentSpec2.IsValid());
        ASSERT_EQ(fragmentSpec1.GetType(), fragmentSpec2.GetType());
        ASSERT_EQ(fragmentSpec1.GetType(), CodeFragmentSpec::Type::SequenceNumber);
        ASSERT_EQ(fragmentSpec1.GetInSequenceMask(), fragmentSpec2.GetInSequenceMask());
        ASSERT_EQ(fragmentSpec1.GetInSequenceMask(), false);
        ASSERT_STREQ(fragmentSpec1.GetPrompt().c_str(), prompt);
        ASSERT_STREQ(fragmentSpec2.GetPrompt().c_str(), prompt);
        }
    
    // CodeFragmentSpec::To/FromJson for PropertyValue
        {
        Utf8CP propertyName = "PropertyName";
        Utf8CP prompt = "PropertyValuePrompt";
        CodeFragmentSpec fragmentSpec1 = CodeFragmentSpec::FromPropertyValue(propertyName, prompt, false);
        CodeFragmentSpec fragmentSpec2 = CodeFragmentSpec::FromJson(fragmentSpec1.ToJson());
        ASSERT_TRUE(fragmentSpec1.IsValid());
        ASSERT_TRUE(fragmentSpec2.IsValid());
        ASSERT_EQ(fragmentSpec1.GetType(), fragmentSpec2.GetType());
        ASSERT_EQ(fragmentSpec1.GetType(), CodeFragmentSpec::Type::PropertyValue);
        ASSERT_EQ(fragmentSpec1.GetInSequenceMask(), fragmentSpec2.GetInSequenceMask());
        ASSERT_EQ(fragmentSpec1.GetInSequenceMask(), false);
        ASSERT_STREQ(fragmentSpec1.GetPrompt().c_str(), prompt);
        ASSERT_STREQ(fragmentSpec2.GetPrompt().c_str(), prompt);
        ASSERT_STREQ(fragmentSpec1.GetPropertyName().c_str(), propertyName);
        ASSERT_STREQ(fragmentSpec2.GetPropertyName().c_str(), propertyName);
        }

    // CodeScopeSpec static Create* methods
        {
        CodeScopeSpec dgnDbScope = CodeScopeSpec::CreateDgnDbScope();
        CodeScopeSpec modelScope = CodeScopeSpec::CreateModelScope();
        CodeScopeSpec parentElementScope = CodeScopeSpec::CreateParentElementScope();
        ASSERT_EQ(CodeScopeSpec::Type::DgnDb, dgnDbScope.GetType());
        ASSERT_EQ(CodeScopeSpec::Type::Model, modelScope.GetType());
        ASSERT_EQ(CodeScopeSpec::Type::ParentElement, parentElementScope.GetType());
        }
    
    ECN::ECClassCP informationPartitionElementClass = m_db->Schemas().GetECClass(BIS_ECSCHEMA_NAME, BIS_CLASS_InformationPartitionElement);
    ECN::ECClassCP physicalPartitionClass = m_db->Schemas().GetECClass(BIS_ECSCHEMA_NAME, BIS_CLASS_PhysicalPartition);
    ECN::ECClassCP drawingClass = m_db->Schemas().GetECClass(BIS_ECSCHEMA_NAME, BIS_CLASS_Drawing);
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
        ASSERT_EQ(codeSpec->GetScope().GetType(), CodeScopeSpec::Type::DgnDb);
        codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromElementClass("Enter class name"));
        codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromFixedString("-"));
        codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromPropertyValue("UserLabel", "Enter UserLabel value", false));
        codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromFixedString("-"));
        codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromSequenceNumber("Enter sequence number"));
        ASSERT_EQ(DgnDbStatus::Success, codeSpec->Insert());
        }

    // CodeSpec for TestElement elements
        {
        CodeSpecPtr codeSpec = CodeSpec::Create(*m_db, DPTEST_SCHEMA(DPTEST_TEST_ELEMENT_CLASS_NAME), CodeScopeSpec::CreateModelScope());
        ASSERT_TRUE(codeSpec.IsValid());
        ASSERT_EQ(codeSpec->GetScope().GetType(), CodeScopeSpec::Type::Model);
        codeSpec->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromElementClass("Enter class name"));
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
        }
    }
