/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/RulesEngine/PresentationManagerTestsHelper.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "PerformanceTests.h"
#include <ECPresentation/Content.h>
#include <ECPresentation/IECPresentationManager.h>

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Mantas.Kontrimas                07/2018
+===============+===============+===============+===============+===============+======*/
struct PresentationManagerTestsHelper
{
    static PresentationRuleSetPtr GetItemsRuleset();
    static void WaitForAllFutures(bvector<folly::Future<folly::Unit>>& futures, bool checkHasException = true);

    // PresentationManagerTestsHelper: Content
    static bvector<ECClassInstanceKey> GetGeometricElementKeys(ECDbCR project);
    static folly::Future<folly::Unit> GetContent(RulesDrivenECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId, SelectionInfo const*, KeySetCR inputKeys, Utf8CP type, int flags = 0, int expectedContentSize = -1);
    static folly::Future<folly::Unit> GetContentSetSize(RulesDrivenECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId, SelectionInfo const*, KeySetCR inputKeys, Utf8CP type, int flags = 0, int expectedContentSize = -1);
    static folly::Future<folly::Unit> GetContentForAllGeometricElements(RulesDrivenECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId, Utf8CP type, int flags = 0, int expectedContentSize = -1);
    static folly::Future<folly::Unit> GetContentClassesForGeometricElement(RulesDrivenECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId, Utf8CP type, int expectedContentClassesCount = -1);

    // PresentationManagerTestsHelper: Navigation
    static folly::Future<bvector<NavNodeCPtr>> GetNodesPath(RulesDrivenECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId, int numOfLevels);
    static folly::Future<NavNodesContainer> GetNodes(RulesDrivenECPresentationManager& manager, ECDbCR project, PageOptionsCR pageOptions, Json::Value const& options, NavNodeCP parentNode);
    static folly::Future<folly::Unit> GetFullHierarchy(RulesDrivenECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId, int expectedNodesCount = -1);
    static folly::Future<folly::Unit> FilterNodes(RulesDrivenECPresentationManager& manager, ECDbCR project, Utf8CP rulesetId, Utf8CP filterText, int expectedNodesPathsCount = -1);
    static folly::Future<folly::Unit> GetNodesCount(RulesDrivenECPresentationManager& manager, ECDbCR project, bvector<NavNodeCPtr> const& nodesPath, Utf8CP rulesetId, int expectedNodesCount = -1);
};

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                10/2017
+===============+===============+===============+===============+===============+======*/
struct TestECInstanceChangeEventsSource : ECInstanceChangeEventSource
{
private:
    void NotifyECInstanceChanged(ECDbCR db, ECInstanceId const& id, ECClassCR ecClass, ChangeType change) const
        {
        ECInstanceChangeEventSource::NotifyECInstanceChanged(db, ECInstanceChangeEventSource::ChangedECInstance(ecClass, id, change));
        }
public:
    static RefCountedPtr<TestECInstanceChangeEventsSource> Create() {return new TestECInstanceChangeEventsSource();}
    void NotifyECInstanceUpdated(ECDbCR db, ECInstanceId const& id, ECClassCR ecClass) const {NotifyECInstanceChanged(db, id, ecClass, ChangeType::Update);}
};

END_ECPRESENTATIONTESTS_NAMESPACE
