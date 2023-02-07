/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <UnitTests/ECPresentation/ECPresentationTest.h>
#include "../../../../Source/Hierarchies/NavigationQueryBuilder.h"
#include "../../../../Source/Shared/Queries/QueryBuilderHelpers.h"
#include "../../Helpers/TestNavNode.h"
#include "../../Helpers/TestHelpers.h"
#include "../../Helpers/TestNodesCache.h"
#include "../QueryBuilderTest.h"

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

#define SEARCH_NODE_QUERY(selectClass) \
    Utf8PrintfString("SELECT * FROM [%s].[%s] WHERE [%s].[ECInstanceId] > 0", (selectClass).GetSchema().GetName().c_str(), (selectClass).GetName().c_str(), (selectClass).GetName().c_str())
#define SEARCH_NODE_QUERY_PROCESSED(selectClass) \
    Utf8PrintfString("SELECT *, ECInstanceId AS [" SEARCH_QUERY_FIELD_ECInstanceId "] FROM [%s].[%s] WHERE [%s].[ECInstanceId] > 0", (selectClass).GetSchema().GetName().c_str(), (selectClass).GetName().c_str(), (selectClass).GetName().c_str())

#define CHILD_INSTANCE_KEYS_QUERY "SELECT 123 ECClassId, 456 ECInstanceId"
#define LABEL_GROUPING_NODE_LABEL "MyLabel"

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NavigationQueryBuilderTests : QueryBuilderTest
    {
    PresentationRuleSetPtr m_ruleset;
    RulesetVariables m_rulesetVariables;
    TestNodesCache m_nodesCache;
    std::unique_ptr<RootNodeRule> m_rootNodeRule;
    std::unique_ptr<ChildNodeRule> m_childNodeRule;
    std::unique_ptr<NavigationQueryBuilder> m_builder;
    std::unique_ptr<IRulesPreprocessor> m_rulesPreprocessor;

    void SetUp() override;

    NavigationQueryBuilder& GetBuilder() {return *m_builder;}

    PresentationQueryBuilderPtr PrepareNavigationQuery(std::function<PresentationQueryBuilderPtr()> queryFactory);
    void ValidateQuery(ChildNodeSpecificationCR spec, PresentationQueryBuilderPtr actualQuery, std::function<PresentationQueryBuilderPtr()> expectedQueryFactory);

    PresentationQueryContractFieldPtr CreateGroupingDisplayLabelField();
    RefCountedPtr<SimpleQueryContract> CreateRelatedInstancesQueryContract();
    RefCountedPtr<SimpleQueryContract> CreateSelect1QueryContract();
    ComplexQueryBuilderR SetLabelGroupingNodeChildrenWhereClause(ComplexQueryBuilderR query);

    Utf8String GetECInstanceNodesOrderByClause();
    Utf8String GetLabelGroupingNodesOrderByClause();
    Utf8String GetECPropertyGroupingNodesOrderByClause();
    Utf8String GetECClassGroupingNodesOrderByClause();
    };

END_ECPRESENTATIONTESTS_NAMESPACE
