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
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("Items");
    ruleset->SetSupportedSchemas("Generic,BisCore");

    auto rootNodeRule = new RootNodeRule();
    rootNodeRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(
        *new InstanceNodesOfSpecificClassesSpecification(1000, ChildrenHint::Unknown, false, true, false, false, "",
            { new MultiSchemaClass("BisCore", true, bvector<Utf8String>{"InformationPartitionElement"}) }, {})
    ));
    ruleset->AddPresentationRule(*rootNodeRule);

    auto subjectChildrenRule = new ChildNodeRule("ParentNode.IsOfClass('Subject', 'BisCore')", 1000, false);
    subjectChildrenRule->AddSpecification(
        *new RelatedInstanceNodesSpecification(1000, ChildrenHint::Unknown, false, true, false, false, "",
            { new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification("BisCore:SubjectOwnsPartitionElements", RequiredRelationDirection_Forward) }) })
    );
    subjectChildrenRule->AddSpecification(
        *new RelatedInstanceNodesSpecification(1000, ChildrenHint::Unknown, false, true, false, false, "",
            { new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification("BisCore:SubjectOwnsSubjects", RequiredRelationDirection_Forward) }) })
    );
    ruleset->AddPresentationRule(*subjectChildrenRule);

    auto informationPartitionElementChildrenRule = new ChildNodeRule("ParentNode.IsOfClass('InformationPartitionElement', 'BisCore')", 1000, false);
    informationPartitionElementChildrenRule->AddSpecification(
        *new RelatedInstanceNodesSpecification(1000, ChildrenHint::Unknown, true, true, false, false, "",
            { new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification("BisCore:ModelModelsElement", RequiredRelationDirection_Backward) }) })
    );
    ruleset->AddPresentationRule(*informationPartitionElementChildrenRule);

    auto geometricModel2dChildrenRule = new ChildNodeRule("ParentNode.IsOfClass('GeometricModel2d', 'BisCore')", 1000, false);
    geometricModel2dChildrenRule->AddSpecification(
        *new RelatedInstanceNodesSpecification(1000, ChildrenHint::Unknown, false, false, false, false, "this.Parent = NULL",
            { new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification("BisCore:ModelContainsElements", RequiredRelationDirection_Forward) }) })
    );
    ruleset->AddPresentationRule(*geometricModel2dChildrenRule);

    auto geometricElement2dChildrenRule = new ChildNodeRule("ParentNode.IsOfClass('GeometricElement2d', 'BisCore')", 1000, false);
    geometricElement2dChildrenRule->AddSpecification(
        *new RelatedInstanceNodesSpecification(1000, ChildrenHint::Unknown, false, false, false, false, "",
            { new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification("BisCore:ElementOwnsChildElements", RequiredRelationDirection_Forward) }) })
    );
    ruleset->AddPresentationRule(*geometricElement2dChildrenRule);

    auto geometricModel3dChildrenRule = new ChildNodeRule("ParentNode.IsOfClass('GeometricModel3d', 'BisCore')", 1000, false);
    geometricModel3dChildrenRule->AddSpecification(
        *new RelatedInstanceNodesSpecification(1000, ChildrenHint::Unknown, false, false, false, false, "this.Parent = NULL",
            { new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification("BisCore:ModelContainsElements", RequiredRelationDirection_Forward) }) })
    );
    ruleset->AddPresentationRule(*geometricModel3dChildrenRule);

    auto geometricElement3dChildrenRule = new ChildNodeRule("ParentNode.IsOfClass('GeometricElement3d', 'BisCore')", 1000, false);
    geometricElement3dChildrenRule->AddSpecification(
        *new RelatedInstanceNodesSpecification(1000, ChildrenHint::Unknown, false, false, false, false, "",
            { new RepeatableRelationshipPathSpecification({ new RepeatableRelationshipStepSpecification("BisCore:ElementOwnsChildElements", RequiredRelationDirection_Forward) }) })
    );
    ruleset->AddPresentationRule(*geometricElement3dChildrenRule);

    auto contentRule = new ContentRule();
    contentRule->AddSpecification(*new SelectedNodeInstancesSpecification());
    ruleset->AddPresentationRule(*contentRule);

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
    return manager.GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(project, rulesetId, RulesetVariables(), type, flags, inputKeys, selection))
        .then([](ContentDescriptorResponse descriptorResponse)
        {
        return *descriptorResponse;
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
    futures.push_back(manager.GetNodesCount(AsyncHierarchyRequestParams::Create(project, rulesetId, RulesetVariables())));
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
