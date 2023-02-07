/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/NavNode.h>
#include <ECPresentation/LabelDefinition.h>
#include <ECPresentation/Update.h>
#include "../RulesEngineTypes.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NavNodesFactory
{
public:
    ECPRESENTATION_EXPORT NavNodePtr CreateECInstanceNode(IConnectionCR, Utf8StringCR, NavNodeKeyCP, bvector<ECClassInstanceKey> const&, LabelDefinitionCR label) const;
    ECPRESENTATION_EXPORT NavNodePtr CreateECInstanceNode(IConnectionCR, Utf8StringCR, NavNodeKeyCP, bvector<ECInstanceKey> const&, LabelDefinitionCR label) const;
    ECPRESENTATION_EXPORT NavNodePtr CreateECInstanceNode(IConnectionCR, Utf8StringCR, NavNodeKeyCP, ECClassId, ECInstanceId, LabelDefinitionCR label) const;
    ECPRESENTATION_EXPORT NavNodePtr CreateECInstanceNode(IConnectionCR, Utf8StringCR, NavNodeKeyCP, IECInstanceCR, LabelDefinitionCR label) const;
    ECPRESENTATION_EXPORT NavNodePtr CreateECClassGroupingNode(IConnectionCR, Utf8StringCR, NavNodeKeyCP, ECClassCR, bool, LabelDefinitionCR label, uint64_t, PresentationQueryCP) const;
    ECPRESENTATION_EXPORT NavNodePtr CreateECRelationshipGroupingNode(IConnectionCR, Utf8StringCR, NavNodeKeyCP, ECRelationshipClassCR, LabelDefinitionCR label, uint64_t, PresentationQueryCP) const;
    ECPRESENTATION_EXPORT NavNodePtr CreateECPropertyGroupingNode(IConnectionCR, Utf8StringCR, NavNodeKeyCP, ECClassCR, ECPropertyCR, LabelDefinitionCR label, Utf8CP imageId, RapidJsonValueCR groupingValue, bool isRangeGrouping, uint64_t, PresentationQueryCP) const;
    ECPRESENTATION_EXPORT NavNodePtr CreateDisplayLabelGroupingNode(IConnectionCR, Utf8StringCR, NavNodeKeyCP, LabelDefinitionCR label, uint64_t, PresentationQueryCP, std::unique_ptr<bvector<ECInstanceKey>> = nullptr) const;
    ECPRESENTATION_EXPORT NavNodePtr CreateCustomNode(IConnectionCR, Utf8StringCR, NavNodeKeyCP, LabelDefinitionCR label, Utf8CP description, Utf8CP imageId, Utf8CP type, PresentationQueryCP) const;
    ECPRESENTATION_EXPORT NavNodePtr CreateFromJson(IConnectionCR, RapidJsonValueCR, NavNodeKeyR) const;
};
typedef NavNodesFactory const& NavNodesFactoryCR;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NavNodesHelper
{
private:
    NavNodesHelper() {}
public:
    static void AddRelatedInstanceInfo(NavNodeR node, Utf8CP serializedJson);
    static bool IsGroupingNode(NavNodeCR);
    static bool IsCustomNode(NavNodeCR node);
    ECPRESENTATION_EXPORT static Utf8String NodeKeyHashPathToString(NavNodeKeyCR key);
    static bvector<Utf8String> NodeKeyHashPathFromString(Utf8CP str);

    static rapidjson::Document SerializeNodeToJson(NavNodeCR);
    static NavNodePtr DeserializeNodeFromJson(RapidJsonValueCR);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
