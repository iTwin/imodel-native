/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentation/ECPresentationManager.h>
#include <UnitTests/ECPresentation/TestECInstanceChangeEventsSource.h>
#include "../PerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

#define HIERARCHY_REQUEST_PAGE_SIZE 20

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct HierarchyPerformanceTests : RulesEngineSingleProjectTests
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

    void IncrementallyGetNodes(Utf8CP rulesetId, NavNodeCP parentNode, bool usePaging);
    bvector<ECClassInstanceKey> GetGeometricElementKeys();
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECClassInstanceKey> HierarchyPerformanceTests::GetGeometricElementKeys()
    {
    // getting content for all geometric elements in the dataset
    bvector<ECClassInstanceKey> keys;
    ECSqlStatement stmt;
    stmt.Prepare(m_project, "SELECT ECClassId, ECInstanceId FROM [BisCore].[GeometricElement]");
    while (BeSQLite::DbResult::BE_SQLITE_ROW == stmt.Step())
        keys.push_back(ECClassInstanceKey(m_project.Schemas().GetClass(stmt.GetValueId<ECClassId>(0)), stmt.GetValueId<ECInstanceId>(1)));
    return keys;
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
void HierarchyPerformanceTests::IncrementallyGetNodes(Utf8CP rulesetId, NavNodeCP parentNode, bool usePaging)
    {
    auto params = AsyncHierarchyRequestParams::Create(m_project, rulesetId, RulesetVariables(), parentNode);

    size_t pageCount = 1;
    if (usePaging)
        {
        size_t nodesCount = 0;
        NodesCountResponse nodesCountResponse = m_manager->GetNodesCount(params).get();
        pageCount = (size_t)ceil(1.0 * nodesCountResponse.GetResult() / HIERARCHY_REQUEST_PAGE_SIZE);
        }

    for (size_t pageIndex = 0; pageIndex < pageCount; ++pageIndex)
        {
        PageOptions pageOptions;
        if (usePaging)
            {
            pageOptions.SetPageStart(pageIndex * HIERARCHY_REQUEST_PAGE_SIZE);
            pageOptions.SetPageSize(HIERARCHY_REQUEST_PAGE_SIZE);
            }

        NodesResponse nodesResponse = m_manager->GetNodes(MakePaged(params, pageOptions)).get();
        for (size_t nodeIndex = 0; nodeIndex < nodesResponse.GetResult().GetSize(); ++nodeIndex)
            {
            NavNodeCPtr node = nodesResponse.GetResult()[nodeIndex];
            if (node->HasChildren())
                IncrementallyGetNodes(rulesetId, node.get(), usePaging);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyPerformanceTests, CreateFullHierarchyWithPaging)
    {
    Timer t_hierarchy;
    IncrementallyGetNodes("Items", nullptr, true);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyPerformanceTests, CreateFullHierarchyWithoutPaging)
    {
    Timer t_hierarchy;
    IncrementallyGetNodes("Items", nullptr, false);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyPerformanceTests, FilterNodesFromNotExpandedHierarchy)
    {
    Timer t_hierarchy;
    NodePathsResponse pathsResponse = m_manager->GetNodePaths(AsyncNodePathsFromFilterTextRequestParams::Create(m_project, "Items", RulesetVariables(), "ist")).get();
    EXPECT_EQ(1, pathsResponse.GetResult().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyPerformanceTests, FilterNodesFromExpandedHierarchy)
    {
    IncrementallyGetNodes("Items", nullptr, false);
    Timer t_hierarchy;
    NodePathsResponse pathsResponse = m_manager->GetNodePaths(AsyncNodePathsFromFilterTextRequestParams::Create(m_project, "Items", RulesetVariables(), "ist")).get();
    EXPECT_EQ(1, pathsResponse.GetResult().size());
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct HierarchyUpdatePerformanceTests : HierarchyPerformanceTests
    {
    std::shared_ptr<TestECInstanceChangeEventsSource> m_eventSource = std::make_shared<TestECInstanceChangeEventsSource>();

    virtual void _ConfigureManagerParams(ECPresentationManager::Params& params) override
        {
        HierarchyPerformanceTests::_ConfigureManagerParams(params);
        params.SetECInstanceChangeEventSources({ m_eventSource });
        }
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyUpdatePerformanceTests, UpdateGeometricElementInHierarchy)
    {
    IncrementallyGetNodes("Items", nullptr, false);

    bvector<ECClassInstanceKey> keys = GetGeometricElementKeys();

    ECInstanceId id = keys[0].GetId();
    ECClassCP ecClass = keys[0].GetClass();
    Timer _time;
    m_eventSource->NotifyECInstanceUpdated(m_project, ECClassInstanceKey(*ecClass, id));
    _time.Finish();
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NavigatorClassificationHierarchyPerformanceTests : HierarchyPerformanceTests
    {
    BeFileName _SupplyProjectPath() const override
        {
        BeFileName path;
        BeTest::GetHost().GetDocumentsRoot(path);
        path.AppendToPath(L"Performance");
        path.AppendToPath(L"RedstoneLAO.bim");
        return path;
        }

    PresentationRuleSetPtr _SupplyRuleset() const override
        {
        return PresentationRuleSet::ReadFromXmlString(R"ruleset(
            <PresentationRuleSet RuleSetId="Classification" VersionMajor="1" VersionMinor="3" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
                <RootNodeRule>
                    <InstancesOfSpecificClasses ClassNames='BisCore:SpatialElement'
                        InstanceFilter='this.ECClassId &lt;&gt; GetECClassId("PhysicalObject","Generic")'
                        GroupByClass='true' GroupByLabel='true' ArePolymorphic='true'
                        HasChildren='Never'
                    />
                </RootNodeRule>
            </PresentationRuleSet>
            )ruleset");
        }
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavigatorClassificationHierarchyPerformanceTests, GetChildNodes_VSTS38248)
    {
    NodesResponse rootNodesResponse = m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_project, "Classification", RulesetVariables(), nullptr)).get();
    ASSERT_EQ(2, rootNodesResponse.GetResult().GetSize());

    Timer t_hierarchy;
    NodesResponse childNodesResponse = m_manager->GetNodes(MakePaged(AsyncHierarchyRequestParams::Create(m_project, "Classification", RulesetVariables(), rootNodesResponse.GetResult()[0].get()), PageOptions(0, 20))).get();
    ASSERT_TRUE(childNodesResponse.GetResult().GetSize() > 0);
    ASSERT_TRUE(childNodesResponse.GetResult().Get(0).IsValid());
    }

/*=================================================================================**//**
* @bsiclass
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LabelOverrideHierarchyPerformanceTests, CreateFullHierarchy)
    {
    Timer t_hierarchy;
    IncrementallyGetNodes("Items", nullptr, false);
    }

/*=================================================================================**//**
* @betest
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceLabelOverrideHierarchyPerformanceTests, CreateFullHierarchy)
    {
    Timer t_hierarchy;
    IncrementallyGetNodes("Items", nullptr, false);
    }
