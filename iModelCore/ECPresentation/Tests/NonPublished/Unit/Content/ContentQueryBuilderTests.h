/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <UnitTests/ECPresentation/ECPresentationTest.h>
#include "../../../../Source/Content/ContentQueryBuilder.h"
#include "../QueryBuilderTest.h"
#include "../../Helpers/TestHelpers.h"
#include "../../Helpers/TestNavNode.h"

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

#define DEFAULT_CONTENT_FIELD_CATEGORY DefaultCategorySupplier().CreateDefaultCategory()

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentQueryBuilderTests : QueryBuilderTest
    {
    PresentationRuleSetPtr m_ruleset;
    RulesetVariables m_rulesetVariables;
    TestNodeLocater m_nodesLocater;
    DefaultCategorySupplier m_categorySupplier;
    std::shared_ptr<ContentDescriptorBuilder::Context> m_context;
    std::shared_ptr<ContentDescriptorBuilder> m_descriptorBuilder;
    std::shared_ptr<ContentQueryBuilder> m_queryBuilder;
    std::unique_ptr<IRulesPreprocessor> m_rulesPreprocessor;

    void SetUp() override;

    ContentDescriptorBuilder& GetDescriptorBuilder() {return *m_descriptorBuilder;}
    ContentQueryBuilder& GetQueryBuilder() {return *m_queryBuilder;}

    void ValidateContentQuerySet(ContentQuerySet const& querySet);
    ContentQuerySet PrepareContentQuerySet(std::function<ContentQuerySet()> querySetFactory);
    ContentQuerySet PrepareContentQuerySet(std::function<ContentQueryPtr()> queryFactory);

    void ValidateQueries(ContentQuerySet const& actual, ContentQuerySet const& expected);
    void ValidateQueries(ContentQuerySet const& actual, std::function<ContentQueryPtr()> expectedSetFactory);
    void ValidateQueries(ContentQuerySet const& actual, std::function<ContentQuerySet()> expectedSetFactory);

    bool AreDescriptorsEqual(ContentQuerySet const& lhs, ContentQuerySet const& rhs);
    MultiSchemaClass* CreateMultiSchemaClass(bvector<ECClassCP> const& classes, bool polymorphic, Utf8CP schemaName = BeTest::GetNameOfCurrentTest());

    void RegisterCategoryInDescriptorIfNeeded(ContentDescriptorR descriptor, std::shared_ptr<ContentDescriptor::Category const> category);
    void RegisterCategoryIfNeeded(ContentDescriptorR descriptor, ContentDescriptor::Field const& field);
    ContentQueryContractPtr CreateQueryContract(int id, ContentDescriptorCR descriptor, ECClassCP ecClass, IQueryInfoProvider const& queryInfo, PresentationQueryContractFieldPtr displayLabelField = nullptr, bvector<RelatedClassPath> relatedInstancePaths = {}, Utf8CP inputAlias = nullptr);
    ContentDescriptor::ECPropertiesField* CreatePropertiesField(std::shared_ptr<ContentDescriptor::Category> category, ContentDescriptor::Property prop, Utf8String customName = "");
    ContentDescriptor::RelatedContentField* CreateRelatedField(std::shared_ptr<ContentDescriptor::Category> category, Utf8CP uniqueName, ECClassCR relatedClass, RelatedClassPath pathFromSelectClass, bvector<ContentDescriptor::Field*> fields, RelationshipMeaning meaning = RelationshipMeaning::RelatedInstance);
    ContentDescriptor::Field& AddField(ContentDescriptorR descriptor, std::shared_ptr<ContentDescriptor::Category> category, ContentDescriptor::Property prop);
    ContentDescriptor::Field& AddField(ContentDescriptorR descriptor, std::shared_ptr<ContentDescriptor::Category> category, bvector<ContentDescriptor::Property> props);
    ContentDescriptor::Field& AddField(ContentDescriptorR descriptor, ContentDescriptor::Field& field);
    ContentDescriptor::Property CreateProperty(Utf8String prefix, ECClassCR propertyClass, ECPropertyCR ecProperty);
    std::shared_ptr<ContentDescriptor::Category> CreateCategory(ECClassCR ecClass, std::shared_ptr<ContentDescriptor::Category> parentCategory = nullptr);
    ContentDescriptorPtr GetEmptyContentDescriptor(Utf8CP displayType = ContentDisplayType::Undefined);
    bmap<ECClassCP, bvector<InstanceLabelOverride const*>> CreateLabelOverrideSpecificationsMap(ECClassCR ecClass, InstanceLabelOverride const& spec);
    RelatedClassPath ReverseRelationshipPath(RelatedClassPath path, Utf8CP targetClassAlias, bool isTargetPolymorphic);
    };

END_ECPRESENTATIONTESTS_NAMESPACE
