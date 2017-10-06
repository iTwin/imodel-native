/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/TestNavNode.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <UnitTests/BackDoor/ECPresentation/ECPresentationTest.h>
#include "../../../Source/RulesDriven/RulesEngine/JsonNavNode.h"
#include "../../../Source/RulesDriven/RulesEngine/ExtendedData.h"
#include <functional>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_EC

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2015
+===============+===============+===============+===============+===============+======*/
struct TestNavNode : JsonNavNode
{
friend struct TestNodesHelper;

private:
    ECClassId m_classId;
private:
    TestNavNode(ECDb const* db) {InitNode(db);}
    void InitNode(ECDb const* db);
protected:
    NavNodeKeyCPtr _CreateKey() const override;
public:
    static RefCountedPtr<TestNavNode> Create(ECDb const* db = nullptr) {return new TestNavNode(db);}
    Utf8CP GetRulesetId() const {return NavNodeExtendedData(*this).GetRulesetId();}
    BeGuid GetConnectionId() const {return NavNodeExtendedData(*this).GetConnectionId();}
};
typedef RefCountedPtr<TestNavNode> TestNavNodePtr;

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2015
+===============+===============+===============+===============+===============+======*/
struct TestNodesHelper
    {
    static uint64_t CreateNodeId() { static uint64_t s_nodeIdentifiers = 1; return s_nodeIdentifiers++; }
    static TestNavNodePtr CreateInstanceNode(ECClassCR ecClass, ECInstanceId instanceId = ECInstanceId((uint64_t)123));
    static TestNavNodePtr CreateInstanceNode(IECInstanceR instance);
    static TestNavNodePtr CreateClassGroupingNode(ECClassCR ecClass, Utf8CP label);
    static TestNavNodePtr CreateRelationshipGroupingNode(ECRelationshipClassCR ecClass, Utf8CP label);
    static TestNavNodePtr CreatePropertyGroupingNode(ECClassCR ecClass, ECPropertyCR ecProperty, Utf8CP label, RapidJsonValueCR groupingValue, bool isRangeGrouping);
    static TestNavNodePtr CreateLabelGroupingNode(Utf8CP label);
    static TestNavNodePtr CreateCustomNode(Utf8CP type, Utf8CP label, Utf8CP description);
    };

END_ECPRESENTATIONTESTS_NAMESPACE
