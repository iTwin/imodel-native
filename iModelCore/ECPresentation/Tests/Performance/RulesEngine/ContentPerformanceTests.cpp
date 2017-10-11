/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/RulesEngine/ContentPerformanceTests.cpp $
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
struct ContentPerformanceTests : RulesEnginePerformanceTests
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
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bset<ECClassCP> GetDerivedClasses(ECDbCR db, ECClassCR base)
    {
    bset<ECClassCP> allDerivedClasses;
    for (ECClassCP derivedClass : db.Schemas().GetDerivedClasses(base))
        {
        bset<ECClassCP> const& derivedClasses = GetDerivedClasses(db, *derivedClass);
        allDerivedClasses.insert(derivedClasses.begin(), derivedClasses.end());
        allDerivedClasses.insert(derivedClass);
        }
    return allDerivedClasses;
    }

/*---------------------------------------------------------------------------------**//**
* The test is based on products' property selector use case where a content descriptor
* is requested for all element subclasses classes.
* @betest                                       Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentPerformanceTests, GetDescriptorForAllElementSubclasses)
    {    
    // set up selection
    ECClassCP elementClass = m_project.Schemas().GetClass("BisCore", "Element");
    bset<ECClassCP> allElementClassesSet = GetDerivedClasses(m_project, *elementClass);
    bvector<ECClassCP> allElementClasses(allElementClassesSet.begin(), allElementClassesSet.end());
    SelectionInfo selection(allElementClasses);
    
    // get the descriptor
    Timer _timer;
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(m_project, ContentDisplayType::PropertyPane, selection, CreateContentOptions().GetJson());
    EXPECT_TRUE(descriptor.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* The test is based on Version Compare use case where the library requests content 
* classes for all derived classes of BisCore:Element to find property relationship
* paths.
* @betest                                       Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentPerformanceTests, GetContentClassesForBisElements)
    {    
    ECClassCP elementClass = m_project.Schemas().GetClass("BisCore", "Element");

    Timer _timer;
    bvector<SelectClassInfo> classes = m_manager->GetContentClasses(m_project, ContentDisplayType::PropertyPane, {elementClass}, CreateContentOptions().GetJson());
    EXPECT_TRUE(!classes.empty());
    }

/*---------------------------------------------------------------------------------**//**
* The test is based on a use case where application has a list of instance keys and wants
* to get their display labels.
* @betest                                       Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentPerformanceTests, GetDisplayLabels)
    {
    // getting labels of all geometric elements in the dataset
    NavNodeKeyList keys;
    ECSqlStatement stmt;
    stmt.Prepare(m_project, "SELECT ECClassId, ECInstanceId FROM [BisCore].[GeometricElement]");
    while (BeSQLite::DbResult::BE_SQLITE_ROW == stmt.Step())
        keys.push_back(ECInstanceNodeKey::Create(stmt.GetValueId<ECClassId>(0), stmt.GetValueId<ECInstanceId>(1)));
    SelectionInfo selection("", false, *NavNodeKeyListContainer::Create(keys));

    // start the timer
    Timer _timer;

    // get the descriptor
    RulesDrivenECPresentationManager::ContentOptions options = CreateContentOptions();
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(m_project, nullptr, selection, options.GetJson());

    // modify the descriptor to include display labels and exclude all other fields
    ContentDescriptorPtr labelsDescriptor = ContentDescriptor::Create(*descriptor);
    labelsDescriptor->SetContentFlags((int)ContentFlags::ShowLabels);
    bvector<ContentDescriptor::Field*> fields = labelsDescriptor->GetVisibleFields();
    for (ContentDescriptor::Field const* field : fields)
        {
        if (!field->IsDisplayLabelField())
            labelsDescriptor->RemoveField(*field);
        }

    // get the content
    ContentCPtr content = m_manager->GetContent(m_project, *labelsDescriptor, selection, PageOptions(), options.GetJson());
    ASSERT_TRUE(content.IsValid());
    EXPECT_EQ(keys.size(), content->GetContentSet().GetSize());

    for (ContentSetItemCPtr record : content->GetContentSet())
        EXPECT_TRUE(record.IsValid());
    }