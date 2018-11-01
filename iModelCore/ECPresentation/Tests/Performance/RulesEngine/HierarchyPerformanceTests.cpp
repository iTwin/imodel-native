/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/RulesEngine/HierarchyPerformanceTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RulesEnginePerformanceTests.h"
#include <ECPresentation/IECPresentationManager.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

#define HIERARCHY_REQUEST_PAGE_SIZE 20

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                10/2017
+===============+===============+===============+===============+===============+======*/
struct TestECInstanceChangeEventsSource : ECInstanceChangeEventSource
    {
    void NotifyECInstanceChanged(ECDbCR db, ECInstanceId& id, ECClassCR ecClass, ChangeType change) const
        {
        ECInstanceChangeEventSource::NotifyECInstanceChanged(db, ECInstanceChangeEventSource::ChangedECInstance(ecClass, id, change));
        }

    static RefCountedPtr<TestECInstanceChangeEventsSource> Create() {return new TestECInstanceChangeEventsSource();}
    };

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                10/2017
+===============+===============+===============+===============+===============+======*/
struct HierarchyPerformanceTests : RulesEnginePerformanceTests
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

                <ImageIdOverride ImageId='"ECLiteralImage://CATEGORY"' Condition='ThisNode.IsInstanceNode ANDALSO this.IsOfClass("Category", "BisCore")'  Priority='100'/>
                <ImageIdOverride ImageId='"ECLiteralImage://MODEL"' Condition='ThisNode.IsInstanceNode ANDALSO this.IsOfClass("Model", "BisCore")' Priority='100'/>

                <RootNodeRule>
                    <InstancesOfSpecificClasses 
                        ClassNames='BisCore:Subject' ArePolymorphic='false' InstanceFilter='this.Parent = NULL' 
                        GroupByClass='false' GroupByLabel='false'/>
                </RootNodeRule>

                <ChildNodeRule Condition='ParentNode.ECInstance.IsOfClass("Subject", "BisCore")'>
                    <RelatedInstances 
                        RelationshipClassNames='BisCore:SubjectOwnsPartitionElements' RequiredDirection='Forward' 
                        RelatedClassNames='BisCore:InformationPartitionElement'
                        GroupByClass='false' GroupByLabel='false'/>
                    <RelatedInstances 
                        RelationshipClassNames='BisCore:SubjectOwnsSubjects' RequiredDirection='Forward' 
                        RelatedClassNames='BisCore:Subject'
                        GroupByClass='false' GroupByLabel='false'/>
                </ChildNodeRule>

                <ChildNodeRule Condition='ParentNode.IsInstanceNode ANDALSO ParentNode.ECInstance.IsOfClass("InformationPartitionElement", "BisCore")'>
                    <RelatedInstances 
                        RelationshipClassNames='BisCore:ModelModelsElement' RequiredDirection='Backward' 
                        RelatedClassNames='BisCore:Model'
                        GroupByClass='false' GroupByLabel='false' HideNodesInHierarchy='true' />
                </ChildNodeRule>

                <ChildNodeRule Condition='ParentNode.ECInstance.IsOfClass("GeometricModel2d", "BisCore")'>
                    <RelatedInstances
                        RelationshipClassNames='BisCore:ModelContainsElements' RequiredDirection='Forward'
                        RelatedClassNames='BisCore:GeometricElement2d' InstanceFilter='this.Parent = NULL'
                        GroupByClass='false' GroupByLabel='false'/>
                </ChildNodeRule>
                <GroupingRule ClassName='GeometricElement2d' SchemaName='BisCore' Condition='ParentNode.IsInstanceNode ANDALSO ParentNode.ECInstance.IsOfClass("GeometricModel2d", "BisCore")'>
                    <PropertyGroup PropertyName='Category' CreateGroupForSingleItem='true'/>
                </GroupingRule>
                <ChildNodeRule Condition='ParentNode.ECInstance.IsOfClass("GeometricElement2d", "BisCore")'>
                    <RelatedInstances
                        RelationshipClassNames='BisCore:ElementOwnsChildElements' RequiredDirection='Forward'
                        RelatedClassNames='BisCore:GeometricElement2d'
                        GroupByClass='false' GroupByLabel='false'/>
                </ChildNodeRule>

                <ChildNodeRule Condition='ParentNode.ECInstance.IsOfClass("GeometricModel3d", "BisCore")'>
                    <RelatedInstances
                        RelationshipClassNames='BisCore:ModelContainsElements' RequiredDirection='Forward'
                        RelatedClassNames='BisCore:GeometricElement3d' InstanceFilter='this.Parent = NULL'
                        GroupByClass='false' GroupByLabel='false'/>
                </ChildNodeRule>
                <GroupingRule ClassName='GeometricElement3d' SchemaName='BisCore' Condition='ParentNode.IsInstanceNode ANDALSO ParentNode.ECInstance.IsOfClass("GeometricModel3d", "BisCore")'>
                    <PropertyGroup PropertyName='Category' CreateGroupForSingleItem='true'/>
                </GroupingRule>
                <ChildNodeRule Condition='ParentNode.ECInstance.IsOfClass("GeometricElement3d", "BisCore")'>
                    <RelatedInstances
                        RelationshipClassNames='BisCore:ElementOwnsChildElements' RequiredDirection='Forward'
                        RelatedClassNames='BisCore:GeometricElement3d'
                        GroupByClass='false' GroupByLabel='false'/>
                </ChildNodeRule>

                <ChildNodeRule Condition='ParentNode.IsInstanceNode ANDALSO ParentNode.ECInstance.IsOfClass("DefinitionModel", "BisCore")'>
                    <RelatedInstances
                        RelationshipClassNames='BisCore:ModelContainsElements' RequiredDirection='Forward'
                        RelatedClassNames='BisCore:DefinitionElement' InstanceFilter='this.Parent = NULL'
                        GroupByClass='true' GroupByLabel='false'>
                        <ChildNodeRule Condition='ParentNode.ECInstance.IsOfClass("RecipeDefinitionElement", "BisCore")'>
                            <RelatedInstances 
                                RelationshipClassNames='BisCore:ModelModelsElement' RequiredDirection='Backward' 
                                RelatedClassNames='BisCore:Model'
                                GroupByClass='false' GroupByLabel='false' HideNodesInHierarchy='true' />
                        </ChildNodeRule>
                    </RelatedInstances>
                </ChildNodeRule>
                <ChildNodeRule Condition='ParentNode.IsInstanceNode ANDALSO ParentNode.ECInstance.IsOfClass("Category", "BisCore")'>
                    <RelatedInstances
                        RelationshipClassNames='BisCore:CategoryOwnsSubCategories' RequiredDirection='Forward'
                        RelatedClassNames='BisCore:SubCategory'
                        GroupByClass='false' GroupByLabel='false'/>
                </ChildNodeRule>
                <ChildNodeRule Condition='ParentNode.IsInstanceNode ANDALSO ParentNode.ECInstance.IsOfClass("RenderMaterial", "BisCore")'>
                    <RelatedInstances
                        RelationshipClassNames='BisCore:RenderMaterialOwnsRenderMaterials' RequiredDirection='Forward'
                        RelatedClassNames='BisCore:RenderMaterial'
                        GroupByClass='false' GroupByLabel='false'/>
                </ChildNodeRule>

                <ChildNodeRule Condition='ParentNode.ECInstance.IsOfClass("DocumentListModel", "BisCore")'>
                    <RelatedInstances
                        RelationshipClassNames='BisCore:ModelContainsElements' RequiredDirection='Forward'
                        RelatedClassNames='BisCore:Document'
                        GroupByClass='true' GroupByLabel='false'>
                        <ChildNodeRule Condition='ParentNode.ECInstance.IsOfClass("Document", "BisCore")'>
                            <RelatedInstances 
                                RelationshipClassNames='BisCore:ModelModelsElement' RequiredDirection='Backward' 
                                RelatedClassNames='BisCore:Model'
                                GroupByClass='false' GroupByLabel='false' HideNodesInHierarchy='true' />
                        </ChildNodeRule>
                    </RelatedInstances>
                </ChildNodeRule>

                <ChildNodeRule Condition='ParentNode.IsInstanceNode ANDALSO ParentNode.ECInstance.IsOfClass("GroupInformationModel", "BisCore")'>
                    <RelatedInstances
                        RelationshipClassNames='BisCore:ModelContainsElements' RequiredDirection='Forward'
                        RelatedClassNames='BisCore:GroupInformationElement'
                        GroupByClass='false' GroupByLabel='false'>
                        <ChildNodeRule Condition='ParentNode.IsInstanceNode ANDALSO ParentNode.ECInstance.IsOfClass("GroupInformationElement", "BisCore")'>
                            <RelatedInstances
                                RelationshipClassNames='BisCore:ElementGroupsMembers' RequiredDirection='Forward'
                                RelatedClassNames='BisCore:Element'
                                GroupByClass='false' GroupByLabel='false'/>
                        </ChildNodeRule>
                    </RelatedInstances>
                </ChildNodeRule>

                <ChildNodeRule Condition='ParentNode.ECInstance.IsOfClass("InformationRecordModel", "BisCore")'>
                    <RelatedInstances
                        RelationshipClassNames='BisCore:ModelContainsElements' RequiredDirection='Forward'
                        RelatedClassNames='BisCore:InformationRecordElement'
                        GroupByClass='false' GroupByLabel='false'/>
                </ChildNodeRule>

                <ChildNodeRule Condition='ParentNode.ECInstance.IsOfClass("LinkModel", "BisCore")'>
                    <RelatedInstances
                        RelationshipClassNames='BisCore:ModelContainsElements' RequiredDirection='Forward'
                        RelatedClassNames='BisCore:LinkElement'
                        GroupByClass='false' GroupByLabel='false'/>
                </ChildNodeRule>

                <ChildNodeRule Condition='ParentNode.ECInstance.IsOfClass("PlanningModel", "Planning")'>
                    <RelatedInstances
                        RelationshipClassNames='BisCore:ModelContainsElements' RequiredDirection='Forward'
                        RelatedClassNames='Planning:WorkBreakdown' InstanceFilter='this.Parent = NULL'
                        GroupByClass='false' GroupByLabel='false'/>
                </ChildNodeRule>
                <ChildNodeRule Condition='ParentNode.IsInstanceNode ANDALSO ParentNode.ECInstance.IsOfClass("WorkBreakdown", "Planning")'>
                    <RelatedInstances
                        RelationshipClassNames='Planning:WorkBreakdownOwnsWorkBreakdowns' RequiredDirection='Forward'
                        RelatedClassNames='Planning:WorkBreakdown'
                        GroupByClass='false' GroupByLabel='false'/>
                    <RelatedInstances
                        RelationshipClassNames='Planning:WorkBreakdownOwnsActivities' RequiredDirection='Forward'
                        RelatedClassNames='Planning:Activity'
                        GroupByClass='false' GroupByLabel='false'/>
                </ChildNodeRule>
            </PresentationRuleSet>
            )ruleset");
        return ruleset;
        }

    DataContainer<NavNodeCPtr> GetNodes(PageOptions& pageOptions, Json::Value const& options, NavNodeCP parentNode);
    void IncrementallyGetNodes(Utf8CP rulesetId, NavNodeCP parentNode, bool usePaging);
    NavNodeKeyList GetGeometricElementKeys();
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyList HierarchyPerformanceTests::GetGeometricElementKeys()
    {
    // getting content for all geometric elements in the dataset
    NavNodeKeyList keys;
    ECSqlStatement stmt;
    stmt.Prepare(m_project, "SELECT ECClassId, ECInstanceId FROM [BisCore].[GeometricElement]");
    while (BeSQLite::DbResult::BE_SQLITE_ROW == stmt.Step())
        keys.push_back(ECInstanceNodeKey::Create(stmt.GetValueId<ECClassId>(0), stmt.GetValueId<ECInstanceId>(1)));
    return keys;
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesContainer HierarchyPerformanceTests::GetNodes(PageOptions& pageOptions, Json::Value const& options, NavNodeCP parentNode)
    {
    if (nullptr == parentNode)
        return m_manager->GetRootNodes(m_project, pageOptions, options).get();

    return m_manager->GetChildren(m_project, *parentNode, pageOptions, options).get();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void HierarchyPerformanceTests::IncrementallyGetNodes(Utf8CP rulesetId, NavNodeCP parentNode, bool usePaging)
    {
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rulesetId, TargetTree_MainTree).GetJson();

    size_t pageCount = 1;
    if (usePaging)
        {
        size_t nodesCount = 0;
        if (nullptr == parentNode)
            nodesCount = m_manager->GetRootNodesCount(m_project, options).get();
        else
            nodesCount = m_manager->GetChildrenCount(m_project, *parentNode, options).get();
        pageCount = (size_t)ceil(1.0 * nodesCount / HIERARCHY_REQUEST_PAGE_SIZE);
        }

    for (size_t pageIndex = 0; pageIndex < pageCount; ++pageIndex)
        {
        PageOptions pageOptions;
        if (usePaging)
            {
            pageOptions.SetPageStart(pageIndex * HIERARCHY_REQUEST_PAGE_SIZE);
            pageOptions.SetPageSize(HIERARCHY_REQUEST_PAGE_SIZE);
            }

        DataContainer<NavNodeCPtr> nodes = GetNodes(pageOptions, options, parentNode);

        for (size_t nodeIndex = 0; nodeIndex < nodes.GetSize(); ++nodeIndex)
            {
            NavNodeCPtr node = nodes[nodeIndex];
            if (node->HasChildren())
                IncrementallyGetNodes(rulesetId, node.get(), usePaging);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyPerformanceTests, CreateFullHierarchyWithPaging)
    {
    Timer t_hierarchy;
    IncrementallyGetNodes("Items", nullptr, true);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyPerformanceTests, CreateFullHierarchyWithoutPaging)
    {
    Timer t_hierarchy;
    IncrementallyGetNodes("Items", nullptr, false);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyPerformanceTests, FilterNodesFromNotExpandedHierarchy)
    {
    Timer t_hierarchy;
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Items", TargetTree_MainTree).GetJson();
    m_manager->GetFilteredNodesPaths(m_project, "ist", options).wait();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyPerformanceTests, FilterNodesFromExpandedHierarchy)
    {
    IncrementallyGetNodes("Items", nullptr, false);
    Timer t_hierarchy;
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions("Items", TargetTree_MainTree).GetJson();
    m_manager->GetFilteredNodesPaths(m_project, "ist", options).wait();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyPerformanceTests, UpdateGeometricElementInHierarchy)
    {
    IncrementallyGetNodes("Items", nullptr, false);

    NavNodeKeyList keys = GetGeometricElementKeys();
    RefCountedPtr<TestECInstanceChangeEventsSource> source = TestECInstanceChangeEventsSource::Create();
    m_manager->RegisterECInstanceChangeEventSource(*source);

    ECInstanceId id = keys[0]->AsECInstanceNodeKey()->GetInstanceId();
    ECClassCP ecClass = m_project.Schemas().GetClass(keys[0]->AsECInstanceNodeKey()->GetECClassId());
    Timer _time;
    source->NotifyECInstanceChanged(m_project, id, *ecClass, ChangeType::Update);
    _time.Finish();
    }

/*=================================================================================**//**
* @bsiclass                                     Aidas.Vaiksnoras                01/2018
+===============+===============+===============+===============+===============+======*/
struct LabelOverrideHierarchyPerformanceTests : HierarchyPerformanceTests 
    {
    PresentationRuleSetPtr _SupplyRuleset() const override
        {
        PresentationRuleSetPtr ruleset = HierarchyPerformanceTests::_SupplyRuleset();
        ruleset->AddPresentationRule(*new LabelOverride(R"(ThisNode.IsInstanceNode ANDALSO this.IsOfClass("Element", "BisCore"))", 100, R"(IIF(IsNull(this.UserLabel) OR this.UserLabel="", this.CodeValue, this.UserLabel))", ""));
        return ruleset;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LabelOverrideHierarchyPerformanceTests, CreateFullHierarchy)
    {
    Timer t_hierarchy;
    IncrementallyGetNodes("Items", nullptr, false);
    }

/*=================================================================================**//**
* @betest                                       Aidas.Vaiksnoras                01/2018
+===============+===============+===============+===============+===============+======*/
struct InstanceLabelOverrideHierarchyPerformanceTests : HierarchyPerformanceTests 
    {
    PresentationRuleSetPtr _SupplyRuleset() const override
        {
        PresentationRuleSetPtr ruleset = HierarchyPerformanceTests::_SupplyRuleset();
        ruleset->AddPresentationRule(*new InstanceLabelOverride(100, true, "BisCore:Element", "CodeValue,UserLabel"));
        return ruleset;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceLabelOverrideHierarchyPerformanceTests, CreateFullHierarchy)
    {
    Timer t_hierarchy;
    IncrementallyGetNodes("Items", nullptr, false);
    }
