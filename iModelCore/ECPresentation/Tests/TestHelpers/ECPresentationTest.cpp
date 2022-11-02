/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECPresentationTest.h"
#include <ECPresentation/ECPresentationManager.h>
#include <ECPresentation/DefaultECPresentationSerializer.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationTest::SetUp()
    {
    ECPresentationManager::SetSerializer(new DefaultECPresentationSerializer());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int PresentationManagerTestsHelper::HierarchyDepthLimiter::GetDepthLimitReachCount() const
    {
    BeMutexHolder lock(m_mutex);
    return m_depthLimitReachCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationManagerTestsHelper::HierarchyDepthLimiter::IsDepthLimitReached(int currentDepth) const
    {
    if (0 > m_depthLimit || currentDepth < m_depthLimit)
        return false;

    BeMutexHolder lock(m_mutex);
    m_depthLimitReachCount++;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationManagerTestsHelper::HierarchyDepthLimiter::ResetDepthLimitReachCount()
    {
    BeMutexHolder lock(m_mutex);
    m_depthLimitReachCount = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static folly::Future<ContentDescriptorCPtr> GetDescriptor(ECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId, SelectionInfo const* selection,
    KeySetCR inputKeys, Utf8CP type, int flags)
    {
    return manager.GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(project, rulesetId, RulesetVariables(), type, 0, inputKeys, selection))
        .then([flags](ContentDescriptorResponse descriptorResponse)
        {
        ContentDescriptorCPtr descriptor = *descriptorResponse;
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> PresentationManagerTestsHelper::GetContent(ECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId, SelectionInfo const* selection,
    KeySetCR inputKeys, Utf8CP type, int flags, int expectedContentSize)
    {
    return GetDescriptor(manager, project, rulesetId, selection, inputKeys, type, flags)
        .then([&manager, &project, expectedContentSize](ContentDescriptorCPtr descriptor) -> folly::Future<folly::Unit>
        {
        if (!descriptor.IsValid())
            return folly::makeFuture();

        return manager.GetContent(AsyncContentRequestParams::Create(project, *descriptor))
            .then([expectedContentSize](ContentResponse contentResponse)
            {
            EXPECT_TRUE(contentResponse.GetResult().IsValid());
            if (-1 != expectedContentSize)
                EXPECT_EQ(expectedContentSize, (int)contentResponse.GetResult()->GetContentSet().GetSize());
            for (ContentSetItemCPtr record : contentResponse.GetResult()->GetContentSet())
                EXPECT_TRUE(record.IsValid());
            });
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> PresentationManagerTestsHelper::GetContentSetSize(ECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId, SelectionInfo const* selection,
    KeySetCR inputKeys, Utf8CP type, int flags, int expectedContentSize)
    {
    return GetDescriptor(manager, project, rulesetId, selection, inputKeys, type, flags)
        .then([&manager, &project, expectedContentSize](ContentDescriptorCPtr descriptor) -> folly::Future<folly::Unit>
        {
        if (!descriptor.IsValid())
            return folly::makeFuture();

        return manager.GetContentSetSize(AsyncContentRequestParams::Create(project, *descriptor))
            .then([expectedContentSize](ContentSetSizeResponse sizeResponse)
            {
            if (-1 != expectedContentSize)
                EXPECT_EQ(expectedContentSize, (int)*sizeResponse);
            });
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> PresentationManagerTestsHelper::GetContentForAllGeometricElements(ECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId, Utf8CP type,
    int flags, int expectedContentSize)
    {
    // getting content for all geometric elements in the dataset
    return GetContent(manager, project, rulesetId, nullptr, *KeySet::Create(GetGeometricElementKeys(project)), type, expectedContentSize, flags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> PresentationManagerTestsHelper::GetContentClassesForGeometricElement(ECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId,
    Utf8CP type, int expectedContentClassesCount)
    {
    ECClassCP elementClass = project.Schemas().GetClass("BisCore", "GeometricElement");
    return manager.GetContentClasses(AsyncContentClassesRequestParams::Create(project, rulesetId, RulesetVariables(), type, 0, bvector<ECClassCP>{ elementClass }))
        .then([expectedContentClassesCount](ContentClassesResponse response)
        {
        if (-1 != expectedContentClassesCount)
            EXPECT_EQ(expectedContentClassesCount, (int)response.GetResult().size());
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static folly::Future<bvector<NavNodeCPtr>> GetNodesPathInternal(ECPresentationManager& manager, ECDbCR project, NavNodeCP parent, Utf8StringCR rulesetId, int numOfLevels)
    {
    if (numOfLevels == 0)
        return folly::makeFuture(bvector<NavNodeCPtr>());

    return manager.GetNodes(AsyncHierarchyRequestParams::Create(project, rulesetId, RulesetVariables(), parent))
        .then([&manager, &project, rulesetId, numOfLevels](NodesResponse response)
            {
            NavNodesContainer const& container = *response;
            for (size_t nodeIndex = 0; nodeIndex < container.GetSize(); ++nodeIndex)
                {
                if (container[nodeIndex]->HasChildren())
                    {
                    NavNodeCPtr node = container[nodeIndex];
                    return GetNodesPathInternal(manager, project, node.get(), rulesetId, numOfLevels - 1).then([node](bvector<NavNodeCPtr> childrenPath)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<bvector<NavNodeCPtr>> PresentationManagerTestsHelper::GetNodesPath(ECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId,
    int numOfLevels)
    {
    return GetNodesPathInternal(manager, project, nullptr, rulesetId, numOfLevels);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static folly::Future<size_t> IncrementallyGetNodes(ECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId, NavNodeCP parentNode)
    {
    return manager.GetNodes(AsyncHierarchyRequestParams::Create(project, rulesetId, RulesetVariables(), parentNode))
        .then([&, rulesetId = Utf8String(rulesetId)](NodesResponse response) -> folly::Future<size_t>
            {
            NavNodesContainer const& container = *response;
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> PresentationManagerTestsHelper::GetFullHierarchy(ECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId,
    int expectedNodesCount)
    {
    return IncrementallyGetNodes(manager, project, rulesetId, nullptr).then([expectedNodesCount](uint64_t count)
        {
        if (-1 != expectedNodesCount)
            EXPECT_EQ(expectedNodesCount, (int)count);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> PresentationManagerTestsHelper::FilterNodes(ECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId,
    Utf8CP filterText, int expectedNodesPathsCount)
    {
    return manager.GetNodePaths(AsyncNodePathsFromFilterTextRequestParams::Create(project, rulesetId, RulesetVariables(), filterText))
        .then([expectedNodesPathsCount](NodePathsResponse paths)
        {
        if (-1 != expectedNodesPathsCount)
            EXPECT_EQ(expectedNodesPathsCount, (int)paths.GetResult().size());
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> PresentationManagerTestsHelper::GetNodesCount(ECPresentationManager& manager, ECDbCR project,
    bvector<NavNodeCPtr> const& nodesPath, Utf8CP rulesetId, int expectedNodesCount)
    {
    std::vector<folly::Future<NodesCountResponse>> futures;
    futures.push_back(manager.GetNodesCount(AsyncHierarchyRequestParams::Create(project, rulesetId, RulesetVariables(), nullptr)));
    for (NavNodeCPtr node : nodesPath)
        futures.push_back(manager.GetNodesCount(AsyncHierarchyRequestParams::Create(project, rulesetId, RulesetVariables(), node.get())));

    auto f = folly::collect(futures);
    if (-1 != expectedNodesCount)
        {
        return f.then([expectedNodesCount](std::vector<NodesCountResponse> counts)
            {
            size_t totalCount = 0;
            for (NodesCountResponse const& count : counts)
                totalCount += *count;
            EXPECT_EQ(expectedNodesCount, totalCount);
            });
        }
    return f.then();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PresentationManagerTestsHelper::ReadFileContent(BeFileNameCR fileName, Utf8StringR content)
    {
    BeFile file;
    if (!fileName.DoesPathExist())
        return ERROR;

    if (BeFileStatus::Success != file.Open(fileName, BeFileAccess::Read))
        return ERROR;

    bvector<Byte> fileContents;
    if (BeFileStatus::Success != file.ReadEntireFile(fileContents))
        return ERROR;

    file.Close();

    content = Utf8String((Utf8CP) & *fileContents.begin(), (Utf8CP) & *fileContents.end());
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<bvector<NavNodeCPtr>> PresentationManagerTestsHelper::GetNodesPaged(ECPresentationManager& manager, AsyncHierarchyRequestParams const& params,
    size_t pageSize, std::function<void(int, double, double)> onPageLoaded)
    {
    auto countTimer = std::make_shared<StopWatch>("", false);

    auto countParams = AsyncHierarchyRequestParams::Create(params);
    countParams.SetTaskStartCallback([countTimer](){countTimer->Start(); });

    return manager.GetNodesCount(countParams).then([&manager, params, countTimer, pageSize, onPageLoaded](NodesCountResponse nodesCount)
        {
        double countTime = countTimer->GetCurrentSeconds();

        size_t pageCount = (size_t)ceil(1.0 * nodesCount.GetResult() / pageSize);
        std::vector<folly::Future<NodesResponse>> pagesFutures;
        for (size_t pageIndex = 0; pageIndex < pageCount; ++pageIndex)
            {
            auto pageTimer = std::make_shared<StopWatch>("", false);

            auto nodesPageParams = MakePaged(params, PageOptions(pageIndex * pageSize, pageSize));
            nodesPageParams.SetTaskStartCallback([pageTimer](){pageTimer->Start(); });

            auto pageFuture = manager.GetNodes(nodesPageParams).then([pageIndex, pageTimer, countTime, onPageLoaded](NodesResponse nodes)
                {
                onPageLoaded(pageIndex, pageTimer->GetCurrentSeconds(), countTime);
                return nodes;
                });
            pagesFutures.push_back(std::move(pageFuture));
            }

        return folly::collect(pagesFutures).then([](std::vector<NodesResponse> nodeResponses)
            {
            bvector<NavNodeCPtr> nodes;
            for (NodesResponse const& response : nodeResponses)
                {
                for (NavNodeCPtr node : *response)
                    nodes.push_back(node);
                }
            return nodes;
            });
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<size_t> PresentationManagerTestsHelper::GatAllNodesPaged(ECPresentationManager& manager, AsyncHierarchyRequestParams const& params,
    size_t pageSize, std::function<void(int, double, double)> onPageLoaded, HierarchyDepthLimiter const* depthLimiter, int depth)
    {
    return GetNodesPaged(manager, params, pageSize, onPageLoaded).then([&manager, params, pageSize, onPageLoaded, depthLimiter, depth](bvector<NavNodeCPtr> nodes)
        {
        if (nullptr != depthLimiter && depthLimiter->IsDepthLimitReached(depth))
            return folly::makeFuture(nodes.size());

        std::vector<folly::Future<size_t>> childrenFutures;
        for (NavNodeCPtr node : nodes)
            {
            if (node->HasChildren())
                {
                auto childrenParams = AsyncHierarchyRequestParams::Create(params);
                childrenParams.SetParentNode(node.get());
                childrenFutures.push_back(GatAllNodesPaged(manager, childrenParams, pageSize, onPageLoaded, depthLimiter, depth + 1));
                }
            }

        size_t nodesCount = nodes.size();
        return folly::collect(childrenFutures).then([nodesCount](std::vector<size_t> counts) mutable
            {
            for (size_t count : counts)
                nodesCount += count;
            return nodesCount;
            });
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<size_t> PresentationManagerTestsHelper::GetAllNodes(ECPresentationManager& manager, AsyncHierarchyRequestParams const& params,
    std::function<void(double)> onHierarchyLevelLoaded, HierarchyDepthLimiter const* depthLimiter, int depth)
    {
    auto timer = std::make_shared<StopWatch>("", false);

    auto nodesParams = AsyncHierarchyRequestParams::Create(params);
    nodesParams.SetTaskStartCallback([timer](){timer->Start(); });

    return manager.GetNodes(nodesParams).then([&manager, params, timer, onHierarchyLevelLoaded, depthLimiter, depth](NodesResponse response) -> folly::Future<size_t>
        {
        onHierarchyLevelLoaded(timer->GetCurrentSeconds());

        NavNodesContainer const& nodes = *response;

        if (nullptr != depthLimiter && depthLimiter->IsDepthLimitReached(depth))
            return nodes.GetSize();

        std::vector<folly::Future<size_t>> childrenFutures;
        for (NavNodeCPtr node : nodes)
            {
            if (node->HasChildren())
                {
                auto childrenParams = AsyncHierarchyRequestParams::Create(params);
                childrenParams.SetParentNode(node.get());
                childrenFutures.push_back(GetAllNodes(manager, childrenParams, onHierarchyLevelLoaded, depthLimiter, depth + 1));
                }
            }
        size_t nodesCount = nodes.GetSize();
        return folly::collect(childrenFutures).then([nodesCount](std::vector<size_t> counts) mutable
            {
            for (size_t count : counts)
                nodesCount += count;
            return nodesCount;
            });
        });
    }
