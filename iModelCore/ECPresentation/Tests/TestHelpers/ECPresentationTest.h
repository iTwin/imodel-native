/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/ECPresentationManager.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/Logging.h>

/**
 * Ignore for the scope of all ECPresentation tests (do not pop):
 *
 * - C6326: indicates a potential comparison of a constant with another constant, which is redundant code.
 *   This warning is caused gtest's `GTEST_AMBIGUOUS_ELSE_BLOCKER_` macro which is used everytime we assert.
 *
 * - C6011: indicates that your code dereferences a potentially null pointer.
 *   This is a pretty dumb check which doesn't understand that ASSERT returns on failure. This means asserting
 *   that a value is not null is not enough - we have to explicitly add an `if (ptr == nullptr) return;` call
 *   and we don't want to pollute our tests with that.
 */
PUSH_REVIEWED_STATIC_ANALYSIS_WARNING(6011 6326)

#include <Bentley/BeTest.h>

#define BEGIN_ECPRESENTATIONTESTS_NAMESPACE BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE namespace Tests {
#define END_ECPRESENTATIONTESTS_NAMESPACE   } END_BENTLEY_ECPRESENTATION_NAMESPACE
#define USING_NAMESPACE_ECPRESENTATIONTESTS using namespace BentleyApi::ECPresentation::Tests;

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ECPresentationTest : ::testing::Test
    {
    virtual void SetUp() override;
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PresentationManagerTestsHelper
    {
    struct HierarchyDepthLimiter
        {
        private:
            mutable BeMutex m_mutex;
            mutable int m_depthLimitReachCount;
            int m_depthLimit;

        public:
            HierarchyDepthLimiter(int limit) : m_depthLimit(limit), m_depthLimitReachCount(0) {}
            bool IsDepthLimitReached(int currentDepth) const;
            int GetDepthLimitReachCount() const;
            void ResetDepthLimitReachCount();
        };

    static PresentationRuleSetPtr GetItemsRuleset();
    static void WaitForAllFutures(bvector<folly::Future<folly::Unit>>& futures, bool checkHasException = true);
    static BentleyStatus ReadFileContent(BeFileNameCR fileName, Utf8StringR content);

    // PresentationManagerTestsHelper: Content
    static bvector<ECClassInstanceKey> GetGeometricElementKeys(ECDbCR project);
    static folly::Future<folly::Unit> GetContent(ECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId, SelectionInfo const*, KeySetCR inputKeys, Utf8CP type, int flags = 0, int expectedContentSize = -1);
    static folly::Future<folly::Unit> GetContentSetSize(ECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId, SelectionInfo const*, KeySetCR inputKeys, Utf8CP type, int flags = 0, int expectedContentSize = -1);
    static folly::Future<folly::Unit> GetContentForAllGeometricElements(ECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId, Utf8CP type, int flags = 0, int expectedContentSize = -1);
    static folly::Future<folly::Unit> GetContentClassesForGeometricElement(ECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId, Utf8CP type, int expectedContentClassesCount = -1);

    // PresentationManagerTestsHelper: Navigation
    static folly::Future<bvector<NavNodeCPtr>> GetNodesPath(ECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId, int numOfLevels);
    static folly::Future<folly::Unit> GetFullHierarchy(ECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId, int expectedNodesCount = -1);
    static folly::Future<folly::Unit> FilterNodes(ECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId, Utf8CP filterText, int expectedNodesPathsCount = -1);
    static folly::Future<folly::Unit> GetNodesCount(ECPresentationManager& manager, ECDbCR project, bvector<NavNodeCPtr> const& nodesPath, Utf8CP rulesetId, int expectedNodesCount = -1);
    static folly::Future<bvector<NavNodeCPtr>> GetNodesPaged(ECPresentationManager&, AsyncHierarchyRequestParams const&, size_t pageSize, std::function<void(int, double, double)>);
    static folly::Future<size_t> GatAllNodesPaged(ECPresentationManager&, AsyncHierarchyRequestParams const&, size_t pageSize, std::function<void(int, double, double)>, HierarchyDepthLimiter const* depthLimiter = nullptr, int depth = 1);
    static folly::Future<size_t> GetAllNodes(ECPresentationManager& manager, AsyncHierarchyRequestParams const&, std::function<void(double)>, HierarchyDepthLimiter const* depthLimiter = nullptr, int depth = 1);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TResponse>
static typename TResponse::ResultType GetValidatedResponse(folly::Future<TResponse> future)
    {
    TResponse response = future.get();
    return *response;
    }

END_ECPRESENTATIONTESTS_NAMESPACE
