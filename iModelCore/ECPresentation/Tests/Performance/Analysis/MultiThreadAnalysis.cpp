/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentation/Content.h>
#include "../PerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MultiThreadingAnalysisTests : RulesEngineSingleProjectTests
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
        return PresentationManagerTestsHelper::GetItemsRuleset();
        }

    virtual void _ConfigureManagerParams(ECPresentationManager::Params& params) override
        {
        RulesEngineSingleProjectTests::_ConfigureManagerParams(params);
        params.SetMultiThreadingParams(ECPresentationManager::Params::MultiThreadingParams(bpair<int, unsigned>(INT_MAX, _GetBackgroundThreadsCount())));
        }

    virtual uint64_t _GetBackgroundThreadsCount() = 0;

    void ExecuteMultipleRequests()
        {
        bvector<folly::Future<folly::Unit>> futures;
        futures.push_back(PresentationManagerTestsHelper::GetContentForAllGeometricElements(*m_manager, m_project, "Items", ContentDisplayType::List));
        futures.push_back(PresentationManagerTestsHelper::GetContentForAllGeometricElements(*m_manager, m_project, "Items", ContentDisplayType::PropertyPane));
        futures.push_back(PresentationManagerTestsHelper::GetContentForAllGeometricElements(*m_manager, m_project, "Items", ContentDisplayType::Grid));
        futures.push_back(PresentationManagerTestsHelper::GetContentForAllGeometricElements(*m_manager, m_project, "Items", ContentDisplayType::Graphics));
        futures.push_back(PresentationManagerTestsHelper::GetContentForAllGeometricElements(*m_manager, m_project, "Items", ContentDisplayType::PropertyPane, (int)ContentFlags::ShowLabels));
        futures.push_back(PresentationManagerTestsHelper::GetFullHierarchy(*m_manager, m_project, "Items", 13426));
        PresentationManagerTestsHelper::WaitForAllFutures(futures);
        }

    void GetFullHierarchy()
        {
        bvector<folly::Future<folly::Unit>> futures;
        futures.push_back(PresentationManagerTestsHelper::GetFullHierarchy(*m_manager, m_project, "Items", 13426));
        PresentationManagerTestsHelper::WaitForAllFutures(futures);
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PresentationManagerSingleThreadPerformanceTests : MultiThreadingAnalysisTests
{
    virtual uint64_t _GetBackgroundThreadsCount() {return 1;}
};

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationManagerSingleThreadPerformanceTests, GetFullHierarchy)
    {
    Timer t_multipleRequests;
    GetFullHierarchy();
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationManagerSingleThreadPerformanceTests, ExecuteMultipleRequests)
    {
    Timer t_multipleRequests;
    ExecuteMultipleRequests();
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PresentationManagerFourThreadsPerformanceTests : MultiThreadingAnalysisTests
{
    virtual uint64_t _GetBackgroundThreadsCount() {return 4;}
};

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationManagerFourThreadsPerformanceTests, GetFullHierarchy)
    {
    Timer t_multipleRequests;
    GetFullHierarchy();
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationManagerFourThreadsPerformanceTests, ExecuteMultipleRequests)
    {
    Timer t_multipleRequests;
    ExecuteMultipleRequests();
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PresentationManagerEightThreadsPerformanceTests : MultiThreadingAnalysisTests
{
    virtual uint64_t _GetBackgroundThreadsCount() {return 8;}
};

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationManagerEightThreadsPerformanceTests, GetFullHierarchy)
    {
    Timer t_multipleRequests;
    GetFullHierarchy();
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationManagerEightThreadsPerformanceTests, ExecuteMultipleRequests)
    {
    Timer t_multipleRequests;
    ExecuteMultipleRequests();
    }
