/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/RulesEngine/PresentationManagerTestsHelper.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "TestsHelper.h"
#include <ECPresentation/Content.h>
#include <ECPresentation/IECPresentationManager.h>
#include <folly/futures/Future-inl.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                       Mantas.Kontrimas                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSetPtr PresentationManagerTestsHelper::GetItemsRuleset()
    {
    // taken from Gist
    PresentationRuleSetPtr ruleset = PresentationRuleSet::ReadFromXmlString(R"ruleset(
        <PresentationRuleSet RuleSetId="Items" VersionMajor="1" VersionMinor="3" SupportedSchemas="Generic,BisCore"
                                xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">

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

            <!-- Content rules -->
            <!-- Grid / DGN view -->
            <ContentRule Condition='(ContentDisplayType="Grid" OR ContentDisplayType="Graphics") ANDALSO SelectedNode.ECInstance.IsOfClass("Model", "BisCore")' OnlyIfNotHandled='true'>
                <ContentRelatedInstances RelationshipClassNames='BisCore:ModelContainsElements' RequiredDirection='Forward' />
            </ContentRule>
            <ContentRule Condition='(ContentDisplayType="Grid" OR ContentDisplayType="Graphics") ANDALSO SelectedNode.ECInstance.IsOfClass("Category", "BisCore")' OnlyIfNotHandled='true'>
                <ContentRelatedInstances RelationshipClassNames='BisCore:GeometricElement2dIsInCategory,GeometricElement3dIsInCategory' RequiredDirection='Backward' />
            </ContentRule>
            <ContentRule Condition='(ContentDisplayType="Grid" OR ContentDisplayType="Graphics") ANDALSO SelectedNode.ECInstance.IsOfClass("Element", "BisCore")' OnlyIfNotHandled='true'>
                <ContentRelatedInstances RelationshipClassNames='BisCore:ElementOwnsChildElements' RelatedClassNames='BisCore:Element' RequiredDirection='Forward' IsRecursive='true' />
                <SelectedNodeInstances />
            </ContentRule>
            <!-- Any other (property pane, list, other) -->
            <ContentRule OnlyIfNotHandled='true'>
                <SelectedNodeInstances />
            </ContentRule>

            <!-- Content modifiers that apply to any content rule -->
            <ContentModifier ClassName="Element" SchemaName="BisCore">
                <RelatedProperties RelationshipClassNames='BisCore:ElementOwnsUniqueAspect' RelatedClassNames='BisCore:ElementUniqueAspect'
                                    RequiredDirection='Forward' IsPolymorphic='True' />
                <RelatedProperties RelationshipClassNames='BisCore:ElementOwnsMultiAspects' RelatedClassNames='BisCore:ElementMultiAspect'
                                    RequiredDirection='Forward' IsPolymorphic='True' />
            </ContentModifier>
            <ContentModifier ClassName="PhysicalElement" SchemaName="BisCore">
                <RelatedProperties RelationshipClassNames='BisCore:PhysicalElementIsOfType' RelatedClassNames='BisCore:PhysicalType'
                                    RequiredDirection='Forward' IsPolymorphic='True' />
            </ContentModifier>
            <ContentModifier ClassName="SpatialLocationElement" SchemaName="BisCore">
                <RelatedProperties RelationshipClassNames='BisCore:SpatialLocationIsOfType' RelatedClassNames='BisCore:SpatialLocationType'
                                    RequiredDirection='Forward' IsPolymorphic='True' />
            </ContentModifier>

        </PresentationRuleSet>
        )ruleset");
    return ruleset;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                       Mantas.Kontrimas                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECClassInstanceKey> PresentationManagerTestsHelper::GetGeometricElementKeys(ECDbCR project)
    {
    // getting content for all geometric elements in the dataset
    bvector<ECClassInstanceKey> keys;
    ECSqlStatement stmt;
    stmt.Prepare(project, "SELECT ECClassId, ECInstanceId FROM [BisCore].[GeometricElement]");
    while (BeSQLite::DbResult::BE_SQLITE_ROW == stmt.Step())
        keys.push_back(ECClassInstanceKey(project.Schemas().GetClass(stmt.GetValueId<ECClassId>(0)), stmt.GetValueId<ECInstanceId>(1)));

    return keys;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                       Mantas.Kontrimas                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static folly::Future<ContentDescriptorCPtr> GetDescriptor(RulesDrivenECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId, SelectionInfo const* selection,
    KeySetCR inputKeys, Utf8CP type, int flags)
    {
    RulesDrivenECPresentationManager::ContentOptions options = RulesDrivenECPresentationManager::ContentOptions(rulesetId);
    return manager.GetContentDescriptor(project, type, 0, inputKeys, selection, options.GetJson())
        .then([flags] (ContentDescriptorCPtr descriptor)
        {
        if (descriptor.IsValid() && descriptor->GetContentFlags() != (flags | descriptor->GetContentFlags()))
            {
            ContentDescriptorPtr modifiedDescriptor = ContentDescriptor::Create(*descriptor);
            modifiedDescriptor->SetContentFlags(flags | descriptor->GetContentFlags());
            descriptor = modifiedDescriptor;
            }

        return descriptor;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                       Mantas.Kontrimas                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> PresentationManagerTestsHelper::GetContent(RulesDrivenECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId, SelectionInfo const* selection,
    KeySetCR inputKeys, Utf8CP type, int flags, int expectedContentSize)
    {
    return GetDescriptor(manager, project, rulesetId, selection, inputKeys, type, flags)
        .then([&manager, expectedContentSize](ContentDescriptorCPtr descriptor) -> folly::Future<folly::Unit>
        {
        if (!descriptor.IsValid())
            return folly::makeFuture();
        return manager.GetContent(*descriptor, PageOptions())
            .then([expectedContentSize](ContentCPtr content) -> folly::Future<folly::Unit>
            {
            EXPECT_TRUE(content.IsValid());
            if (-1 != expectedContentSize)
                EXPECT_EQ(expectedContentSize, (int)content->GetContentSet().GetSize());
            for (ContentSetItemCPtr record : content->GetContentSet())
                EXPECT_TRUE(record.IsValid());

            return folly::makeFuture();
            });
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                       Mantas.Kontrimas                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> PresentationManagerTestsHelper::GetContentSetSize(RulesDrivenECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId, SelectionInfo const* selection,
    KeySetCR inputKeys, Utf8CP type, int flags, int expectedContentSize)
    {
    return GetDescriptor(manager, project, rulesetId, selection, inputKeys, type, flags)
        .then([&manager, expectedContentSize](ContentDescriptorCPtr descriptor) -> folly::Future<folly::Unit>
        {
        if (!descriptor.IsValid())
            return folly::makeFuture();
        return manager.GetContentSetSize(*descriptor)
            .then([expectedContentSize](size_t size) -> folly::Future<folly::Unit>
            {
            if (-1 != expectedContentSize)
                EXPECT_EQ(expectedContentSize, (int)size);
            return folly::makeFuture();
            });
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                       Mantas.Kontrimas                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> PresentationManagerTestsHelper::GetContentForAllGeometricElements(RulesDrivenECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId, Utf8CP type,
    int flags, int expectedContentSize)
    {
    // getting content for all geometric elements in the dataset
    return GetContent(manager, project, rulesetId, nullptr, *KeySet::Create(GetGeometricElementKeys(project)), type, expectedContentSize, flags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                       Mantas.Kontrimas                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> PresentationManagerTestsHelper::GetContentClassesForGeometricElement(RulesDrivenECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId,
    Utf8CP type, int expectedContentClassesCount)
    {
    ECClassCP elementClass = project.Schemas().GetClass("BisCore", "GeometricElement");
    RulesDrivenECPresentationManager::ContentOptions options = RulesDrivenECPresentationManager::ContentOptions(rulesetId);
    return manager.GetContentClasses(project, type, 0, {elementClass}, options.GetJson())
        .then([expectedContentClassesCount](bvector<SelectClassInfo> selectClassInfo)
        {
        if (-1 != expectedContentClassesCount)
            EXPECT_EQ(expectedContentClassesCount, (int)selectClassInfo.size());
        return folly::makeFuture();
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                       Mantas.Kontrimas                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static folly::Future<bvector<NavNodeCPtr>> GetNodesPathInternal(RulesDrivenECPresentationManager& manager, ECDbCR project, NavNodeCP parent, Json::Value options, int numOfLevels)
    {
    if (numOfLevels == 0)
        return folly::makeFuture(bvector<NavNodeCPtr>());

    return PresentationManagerTestsHelper::GetNodes(manager, project, PageOptions(), options, parent).then([&manager, &project, options, numOfLevels](NavNodesContainer container)
        {
        bool foundChild = false;
        for (size_t nodeIndex = 0; nodeIndex < container.GetSize(); ++nodeIndex)
            {
            if (container[nodeIndex]->HasChildren())
                {
                NavNodeCPtr node = container[nodeIndex];
                return GetNodesPathInternal(manager, project, node.get(), options, numOfLevels - 1).then([node](bvector<NavNodeCPtr> childrenPath)
                    {
                    childrenPath.insert(childrenPath.begin(), node);
                    return childrenPath;
                    });
                break;
                }
            }
        return folly::makeFuture(bvector<NavNodeCPtr>());
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                       Mantas.Kontrimas                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<bvector<NavNodeCPtr>> PresentationManagerTestsHelper::GetNodesPath(RulesDrivenECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId,
    int numOfLevels)
    {
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rulesetId).GetJson();
    return GetNodesPathInternal(manager, project, nullptr, options, numOfLevels);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                       Mantas.Kontrimas                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<NavNodesContainer> PresentationManagerTestsHelper::GetNodes(RulesDrivenECPresentationManager& manager, ECDbCR project, PageOptionsCR pageOptions,
    Json::Value const& options, NavNodeCP parentNode)
    {
    if (nullptr == parentNode)
        return manager.GetRootNodes(project, pageOptions, options);
    return manager.GetChildren(project, *parentNode, pageOptions, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                       Mantas.Kontrimas                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static folly::Future<size_t> IncrementallyGetNodes(RulesDrivenECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId, NavNodeCP parentNode)
    {
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rulesetId).GetJson();
    return PresentationManagerTestsHelper::GetNodes(manager, project, PageOptions(), options, parentNode).then([&, rulesetId = Utf8String(rulesetId)](NavNodesContainer container)
        {
        size_t count = container.GetSize();
        std::vector<folly::Future<size_t>> childrenFutures;
        for (NavNodeCPtr node : container)
            {
            if (node->HasChildren())
                childrenFutures.push_back(IncrementallyGetNodes(manager, project, rulesetId.c_str(), node.get()));
            }
        return folly::collect(childrenFutures).then([count](std::vector<size_t> childrenCounts)
            {
            size_t total = count;
            for (size_t childrenCount : childrenCounts)
                total += childrenCount;
            return total;
            });
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                       Mantas.Kontrimas                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> PresentationManagerTestsHelper::GetFullHierarchy(RulesDrivenECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId,
    int expectedNodesCount)
    {
    return IncrementallyGetNodes(manager, project, rulesetId, nullptr).then([expectedNodesCount](uint64_t count)
        {
        if (-1 != expectedNodesCount)
            EXPECT_EQ(expectedNodesCount, (int)count);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                       Mantas.Kontrimas                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> PresentationManagerTestsHelper::FilterNodes(RulesDrivenECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId, 
    Utf8CP filterText, int expectedNodesPathsCount)
    {
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rulesetId).GetJson();
    return manager.GetFilteredNodePaths(project, filterText, options)
        .then([expectedNodesPathsCount](bvector<NodesPathElement> paths)
        {
        if (-1 != expectedNodesPathsCount)
            EXPECT_EQ(expectedNodesPathsCount, (int)paths.size());
        return folly::makeFuture();
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                       Mantas.Kontrimas                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> PresentationManagerTestsHelper::GetNodesCount(RulesDrivenECPresentationManager& manager, ECDbCR project,
    bvector<NavNodeCPtr> const& nodesPath, Utf8CP rulesetId, int expectedNodesCount)
    {
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rulesetId).GetJson();
    std::vector<folly::Future<size_t>> futures;
    futures.push_back(manager.GetRootNodesCount(project, options));
    for (NavNodeCPtr node: nodesPath)
        futures.push_back(manager.GetChildrenCount(project, *node, options));
    auto f = folly::collect(futures);    
    if (-1 != expectedNodesCount)
        {
        return f.then([expectedNodesCount](std::vector<size_t> counts)
            {
            size_t totalCount = 0;
            for (size_t count : counts)
                totalCount += count;
            EXPECT_EQ(expectedNodesCount, totalCount);
            });
        }
    return f.then();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                       Mantas.Kontrimas                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationManagerTestsHelper::WaitForAllFutures(bvector<folly::Future<folly::Unit>>& futures, bool checkHasException)
    {
    auto result = folly::collect(futures).wait();
    if (checkHasException)
        {
        EXPECT_FALSE(result.hasException());
        EXPECT_TRUE(result.hasValue());
        }
    }
