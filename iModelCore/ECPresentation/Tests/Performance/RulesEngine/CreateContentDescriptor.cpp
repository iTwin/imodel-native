/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/RulesEngine/CreateContentDescriptor.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RulesEnginePerformanceTests.h"
#include <ECPresentation/Content.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+===============+===============+===============+===============+===============+======*/
struct CreateContentDescriptorPerformanceTests : RulesEnginePerformanceTests
    {
    BeFileName _SupplyProjectPath() const override
        {
        BeFileName path;
        BeTest::GetHost().GetDocumentsRoot(path);
        path.AppendToPath(L"Performance");
        path.AppendToPath(L"Oakland.ibim");
        return path;
        }

    PresentationRuleSetPtr _SupplyRuleset() const override
        {
        // taken from Gist
        PresentationRuleSetPtr ruleset = PresentationRuleSet::ReadFromXmlString(R"ruleset(
            <PresentationRuleSet RuleSetId="Items" VersionMajor="1" VersionMinor="3" SupportedSchemas="Generic,BisCore"
                                 xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="PresentationRuleSetSchema.xsd">

                <ContentRule Condition='ContentDisplayType &lt;&gt; "PropertyPane" ANDALSO SelectedNode.ECInstance.IsOfClass("Model", "BisCore")' OnlyIfNotHandled='true'>
                    <ContentRelatedInstances RelationshipClassNames='BisCore:ModelContainsElements' RequiredDirection='Forward' />
                </ContentRule>
    
                <ContentRule Condition='ContentDisplayType &lt;&gt; "PropertyPane" ANDALSO SelectedNode.ECInstance.IsOfClass("Category", "BisCore")' OnlyIfNotHandled='true'>
                    <ContentRelatedInstances RelationshipClassNames='BisCore:GeometricElement2dIsInCategory,GeometricElement3dIsInCategory' RequiredDirection='Backward' />
                </ContentRule>
    
                <ContentRule Condition='ContentDisplayType &lt;&gt; "PropertyPane" ANDALSO SelectedNode.ECInstance.IsOfClass("Element", "BisCore")' OnlyIfNotHandled='true'>
                    <ContentRelatedInstances RelationshipClassNames='BisCore:ElementOwnsChildElements' RelatedClassNames='BisCore:Element' RequiredDirection='Forward' IsRecursive='true' />
                    <SelectedNodeInstances />
                </ContentRule>
    
                <ContentRule Condition='SelectedNode.ECInstance.IsOfClass("PhysicalElement", "BisCore")' OnlyIfNotHandled='true'>
                    <SelectedNodeInstances>
                        <RelatedProperties RelationshipClassNames='BisCore:PhysicalElementIsOfType' RelatedClassNames='BisCore:PhysicalType' RequiredDirection='Forward' PropertyNames='CodeValue'/>
                    </SelectedNodeInstances>
                </ContentRule>
    
                <ContentRule Condition='SelectedNode.ECInstance.IsOfClass("SpatialLocationElement", "BisCore")' OnlyIfNotHandled='true'>
                    <SelectedNodeInstances>
                        <RelatedProperties RelationshipClassNames='BisCore:SpatialLocationIsOfType' RelatedClassNames='BisCore:SpatialLocationType' RequiredDirection='Forward' PropertyNames='CodeValue'/>
                    </SelectedNodeInstances>
                </ContentRule>
    
                <ContentRule OnlyIfNotHandled='true'>
                    <SelectedNodeInstances />
                </ContentRule>

            </PresentationRuleSet>
            )ruleset");
        return ruleset;
        }

    bset<ECClassCP> GetDerivedClasses(ECClassCR base);
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bset<ECClassCP> CreateContentDescriptorPerformanceTests::GetDerivedClasses(ECClassCR base)
    {
    bset<ECClassCP> allDerivedClasses;
    for (ECClassCP derivedClass : m_project.Schemas().GetDerivedClasses(base))
        {
        bset<ECClassCP> const& derivedClasses = GetDerivedClasses(*derivedClass);
        allDerivedClasses.insert(derivedClasses.begin(), derivedClasses.end());
        allDerivedClasses.insert(derivedClass);
        }
    return allDerivedClasses;
    }

/*---------------------------------------------------------------------------------**//**
* The test is based on Version Compare use case where the library requests a content 
* descriptor for all derived classes of BisCore:Element to find property relationship
* paths.
* @betest                                       Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CreateContentDescriptorPerformanceTests, GetDescriptorForAllElementSubclasses)
    {    
    // set up selection
    Timer t_selection("GetDescriptorForAllElementSubclasses: Set up selection");
    ECClassCP elementClass = m_project.Schemas().GetClass("BisCore", "Element");
    bset<ECClassCP> allElementClassesSet = GetDerivedClasses(*elementClass);
    bvector<ECClassCP> allElementClasses(allElementClassesSet.begin(), allElementClassesSet.end());
    SelectionInfo selection(allElementClasses);
    t_selection.Finish();
    EXPECT_EQ(1278, allElementClasses.size());

    // set up options
    RulesDrivenECPresentationManager::ContentDescriptorOptions options = CreateContentOptions();
    options.SetCreateFields(false);

    // get the descriptor
    Timer t_descriptor("GetDescriptorForAllElementSubclasses: Get descriptor");
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(m_project, ContentDisplayType::PropertyPane, selection, options.GetJson());
    t_descriptor.Finish();
    }
