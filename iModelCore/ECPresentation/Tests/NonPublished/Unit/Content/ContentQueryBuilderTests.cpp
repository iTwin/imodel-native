/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ContentQueryBuilderTests.h"
#include "../../../../Source/Shared/RulesPreprocessor.h"
#include "../../../../Source/Content/ContentHelpers.h"

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ContentQueryBuilderTests::SetUp()
    {
    static const RulesetVariables s_emptyVariables;

    QueryBuilderTest::SetUp();
    m_ruleset = PresentationRuleSet::CreateInstance("");
    m_rulesPreprocessor = std::make_unique<RulesPreprocessor>(GetConnections(), GetConnection(), *m_ruleset,
        m_rulesetVariables, nullptr, GetSchemaHelper().GetECExpressionsCache());
    m_context = std::make_unique<ContentDescriptorBuilder::Context>(GetSchemaHelper(), GetConnections(), GetConnection(), &GetCancellationToken(),
        *m_rulesPreprocessor, *m_ruleset, ContentDisplayType::Undefined, s_emptyVariables, m_categorySupplier, nullptr, ECPresentation::UnitSystem::Undefined,
        *NavNodeKeyListContainer::Create(), nullptr);
    m_context->SetContentFlagsCalculator([](int defaultFlags){return defaultFlags | (int)ContentFlags::SkipInstancesCheck;});
    m_descriptorBuilder = std::make_unique<ContentDescriptorBuilder>(*m_context);
    m_queryBuilder = std::make_unique<ContentQueryBuilder>(ContentQueryBuilderParameters(GetSchemaHelper(), GetConnections(),
        m_nodesLocater, GetConnection(), &GetCancellationToken(), *m_rulesPreprocessor, *m_ruleset, m_rulesetVariables, GetSchemaHelper().GetECExpressionsCache(),
        nullptr, m_categorySupplier, true, false, nullptr, nullptr));
    m_queryBuilder->GetParameters().SetDisableOmittingFilteredOutQueries(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentQueryBuilderTests::ValidateContentQuerySet(QuerySet const& querySet)
    {
    for (size_t i = 0; i < querySet.GetQueries().size(); ++i)
        {
        Utf8String queryStr = querySet.GetQueries().at(i)->GetQuery()->GetQueryString();
        PrepareQuery(queryStr, Utf8PrintfString("%s[%d]", BeTest::GetNameOfCurrentTest(), i));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
QuerySet ContentQueryBuilderTests::PrepareContentQuerySet(std::function<QuerySet()> querySetFactory)
    {
    QuerySet querySet = querySetFactory();
    ValidateContentQuerySet(querySet);
    return querySet;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
QuerySet ContentQueryBuilderTests::PrepareContentQuerySet(std::function<PresentationQueryBuilderPtr()> queryFactory)
    {
    auto query = queryFactory();
    QuerySet querySet = QuerySet({ query });
    ValidateContentQuerySet(querySet);
    return querySet;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentQueryBuilderTests::ValidateQueries(QuerySet const& actual, QuerySet const& expected)
    {
    EXPECT_TRUE(expected.Equals(actual))
        << "Expected: " << expected.ToString() << "\r\n"
        << "Actual:   " << actual.ToString();
    EXPECT_TRUE(AreDescriptorsEqual(expected, actual))
        << "Expected: " << BeRapidJsonUtilities::ToPrettyString(expected.GetContract()->AsContentQueryContract()->GetDescriptor().AsJson()) << "\r\n"
        << "Actual:   " << BeRapidJsonUtilities::ToPrettyString(actual.GetContract()->AsContentQueryContract()->GetDescriptor().AsJson());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentQueryBuilderTests::ValidateQueries(QuerySet const& actual, std::function<PresentationQueryBuilderPtr()> expectedSetFactory)
    {
    auto expectedSet = PrepareContentQuerySet(expectedSetFactory);
    ValidateQueries(actual, expectedSet);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentQueryBuilderTests::ValidateQueries(QuerySet const& actual, std::function<QuerySet()> expectedSetFactory)
    {
    auto expectedSet = PrepareContentQuerySet(expectedSetFactory);
    ValidateQueries(actual, expectedSet);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentQueryBuilderTests::AreDescriptorsEqual(QuerySet const& lhs, QuerySet const& rhs)
    {
    auto const& lhsDescriptor = lhs.GetContract()->AsContentQueryContract()->GetDescriptor();
    auto const& rhsDescriptor = rhs.GetContract()->AsContentQueryContract()->GetDescriptor();
    return ContentHelpers::AreDescriptorsEqual(lhsDescriptor, rhsDescriptor, RulesetCompareOption::ById);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentQueryBuilderTests::RegisterCategoryInDescriptorIfNeeded(ContentDescriptorR descriptor, std::shared_ptr<ContentDescriptor::Category const> category)
    {
    if (!category)
        return;

    if (category->GetParentCategory())
        RegisterCategoryInDescriptorIfNeeded(descriptor, category->GetParentCategory()->shared_from_this());

    if (ContainerHelpers::Contains(descriptor.GetCategories(), [&](auto const& descriptorCategory) {return *descriptorCategory == *category; }))
        return;

    descriptor.GetCategories().push_back(category);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentQueryBuilderTests::RegisterCategoryIfNeeded(ContentDescriptorR descriptor, ContentDescriptor::Field const& field)
    {
    RegisterCategoryInDescriptorIfNeeded(descriptor, field.GetCategory());

    if (field.IsNestedContentField())
        {
        for (auto const& nestedField : field.AsNestedContentField()->GetFields())
            {
            if (!nestedField->GetCategory())
                nestedField->SetCategory(field.GetCategory());
            RegisterCategoryIfNeeded(descriptor, *nestedField);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MultiSchemaClass* ContentQueryBuilderTests::CreateMultiSchemaClass(bvector<ECClassCP> const& classes, bool polymorphic, Utf8CP schemaName)
    {
    return new MultiSchemaClass(schemaName, polymorphic, ContainerHelpers::TransformContainer<bvector<Utf8String>>(classes, [](ECClassCP ecClass) {return ecClass->GetName();}));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryContractPtr ContentQueryBuilderTests::CreateQueryContract(int id, ContentDescriptorCR descriptor, ECClassCP ecClass, IQueryInfoProvider const& queryInfo, PresentationQueryContractFieldPtr displayLabelField, bvector<RelatedClassPath> relatedInstancePaths, Utf8CP inputAlias)
    {
    auto contract = ContentQueryContract::Create(id, descriptor, ecClass, queryInfo, displayLabelField, relatedInstancePaths, false, false);
    if (inputAlias)
        contract->SetInputClassAlias(inputAlias);
    return contract;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::ECPropertiesField* ContentQueryBuilderTests::CreatePropertiesField(std::shared_ptr<ContentDescriptor::Category> category, ContentDescriptor::Property prop, Utf8String customName)
    {
    auto field = new ContentDescriptor::ECPropertiesField(category, prop);
    field->SetUniqueName(customName.empty() ? field->CreateName() : customName);
    return field;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::RelatedContentField* ContentQueryBuilderTests::CreateRelatedField(std::shared_ptr<ContentDescriptor::Category> category, Utf8CP uniqueName, ECClassCR relatedClass, RelatedClassPath pathFromSelectClass, bvector<ContentDescriptor::Field*> fields, RelationshipMeaning meaning)
    {
    auto field = new ContentDescriptor::RelatedContentField(category, uniqueName, relatedClass.GetDisplayLabel(), pathFromSelectClass, fields);
    field->SetRelationshipMeaning(meaning);
    return field;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field& ContentQueryBuilderTests::AddField(ContentDescriptorR descriptor, std::shared_ptr<ContentDescriptor::Category> category, ContentDescriptor::Property prop)
    {
    RegisterCategoryInDescriptorIfNeeded(descriptor, category);
    descriptor.AddRootField(*CreatePropertiesField(category, prop));
    return *descriptor.GetAllFields().back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field& ContentQueryBuilderTests::AddField(ContentDescriptorR descriptor, std::shared_ptr<ContentDescriptor::Category> category, bvector<ContentDescriptor::Property> props)
    {
    RegisterCategoryInDescriptorIfNeeded(descriptor, category);
    descriptor.AddRootField(*CreatePropertiesField(category, props.front()));
    ContentDescriptor::Field& field = *descriptor.GetAllFields().back();
    for (size_t i = 1; i < props.size(); ++i)
        field.AsPropertiesField()->AddProperty(props[i]);
    field.SetUniqueName(FIELD_NAME(ContainerHelpers::TransformContainer<bvector<ECClassCP>>(props, [](ContentDescriptor::Property const& p) {return &p.GetPropertyClass(); }), props.front().GetProperty().GetName().c_str()));
    return field;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field& ContentQueryBuilderTests::AddField(ContentDescriptorR descriptor, ContentDescriptor::Field& field)
    {
    RegisterCategoryIfNeeded(descriptor, field);

    if (field.GetUniqueName().empty())
        field.SetUniqueName(field.CreateName());
    descriptor.AddRootField(field);
    return *descriptor.GetAllFields().back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Property ContentQueryBuilderTests::CreateProperty(Utf8String prefix, ECClassCR propertyClass, ECPropertyCR ecProperty)
    {
    return ContentDescriptor::Property(prefix, propertyClass, ecProperty);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<ContentDescriptor::Category> ContentQueryBuilderTests::CreateCategory(ECClassCR ecClass, std::shared_ptr<ContentDescriptor::Category> parentCategory)
    {
    auto category = std::make_shared<ContentDescriptor::Category>(ecClass.GetName(), ecClass.GetDisplayLabel(), ecClass.GetDescription(), 1000);
    category->SetParentCategory(parentCategory);
    return category;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptorPtr ContentQueryBuilderTests::GetEmptyContentDescriptor(Utf8CP displayType)
    {
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(GetConnection(), *PresentationRuleSet::CreateInstance(""), RulesetVariables(), *NavNodeKeyListContainer::Create());
    descriptor->SetPreferredDisplayType(displayType);
    descriptor->AddContentFlag(ContentFlags::SkipInstancesCheck);
    return descriptor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bmap<ECClassCP, bvector<InstanceLabelOverride const*>> ContentQueryBuilderTests::CreateLabelOverrideSpecificationsMap(ECClassCR ecClass, InstanceLabelOverride const& spec)
    {
    bmap<ECClassCP, bvector<InstanceLabelOverride const*>> labelOverrideMap;
    labelOverrideMap.Insert(&ecClass, { &spec });
    return labelOverrideMap;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedClassPath ContentQueryBuilderTests::ReverseRelationshipPath(RelatedClassPath path, Utf8CP targetClassAlias, bool isTargetPolymorphic)
    {
    path.Reverse(targetClassAlias, isTargetPolymorphic);
    return path;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(FieldNamesDontCollideWhenSelectingInstanceAndRelatedPropertyOfTheSameClass, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (ContentQueryBuilderTests, FieldNamesDontCollideWhenSelectingInstanceAndRelatedPropertyOfTheSameClass)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(GetDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(GetDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(GetDb(), *relAB, *a, *b);

    // set up selection
    TestParsedInput info({
        bpair<ECClassCP, ECInstanceId>(classA, {(ECInstanceId)ECInstanceId::FromString(a->GetInstanceId().c_str())}),
        bpair<ECClassCP, ECInstanceId>(classB, {(ECInstanceId)ECInstanceId::FromString(b->GetInstanceId().c_str())}),
        });

    // create the spec
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification(*new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)),
        {new PropertySpecification("*")}, RelationshipMeaning::SameInstance));

    // get the query
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        std::shared_ptr<ContentDescriptor::Category> rootCategory = DEFAULT_CONTENT_FIELD_CATEGORY;
        std::shared_ptr<ContentDescriptor::Category> nestedPropertyCategory = CreateCategory(*classB, rootCategory);

        RelatedClassPath aTob({RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true))});

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false).SetRelatedPropertyPaths({aTob}), "");
        descriptor->AddSelectClass(SelectClassInfo(*classB, "this", false), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(rootCategory, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, *CreateRelatedField(rootCategory, NESTED_CONTENT_FIELD_NAME(classA, classB), *classB, { aTob },
            {
            CreatePropertiesField(nestedPropertyCategory, CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), *classB, *classB->GetPropertyP("Prop"))),
            }, RelationshipMeaning::SameInstance));
        AddField(*descriptor, *CreatePropertiesField(rootCategory, CreateProperty("this", *classB, *classB->GetPropertyP("Prop")), FIELD_NAME_C(classB, "Prop", 2)));

        ComplexQueryBuilderPtr q1 = ComplexQueryBuilder::Create();
        q1->SelectContract(*CreateQueryContract(1, *descriptor, classA, *q1), "this");
        q1->From(*classA, false, "this");
        q1->Join(aTob);
        q1->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId::FromString(a->GetInstanceId().c_str()))});

        ComplexQueryBuilderPtr q2 = ComplexQueryBuilder::Create();
        q2->SelectContract(*CreateQueryContract(2, *descriptor, classB, *q2), "this");
        q2->From(*classB, false, "this");
        q2->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId::FromString(b->GetInstanceId().c_str()))});

        UnionQueryBuilderPtr query = UnionQueryBuilder::Create({q1, q2});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(FieldNamesContainNamesOfAllRelatedClassesWhenSelectingMultipleClassesWithSameRelatedProperty, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (ContentQueryBuilderTests, FieldNamesContainNamesOfAllRelatedClassesWhenSelectingMultipleClassesWithSameRelatedProperty)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetECClass("B_To_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(GetDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(GetDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(GetDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(GetDb(), *relAB, *a, *b);
    RulesEngineTestHelpers::InsertRelationship(GetDb(), *relBC, *b, *c);

    // set up selection
    TestParsedInput info({
        bpair<ECClassCP, ECInstanceId>(classA, {(ECInstanceId)ECInstanceId::FromString(a->GetInstanceId().c_str())}),
        bpair<ECClassCP, ECInstanceId>(classC, {(ECInstanceId)ECInstanceId::FromString(c->GetInstanceId().c_str())}),
        });

    // create the spec
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification(*new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)),
        { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance));
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification(*new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Backward)),
        { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance));

    // get the query
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        RelatedClassPath aTob({RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true))});
        RelatedClassPath cTob({RelatedClass(*classC, SelectClass<ECRelationshipClass>(*relBC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relBC, 0)), false, SelectClass<ECClass>(*classB,RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true))});

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false)
            .SetRelatedPropertyPaths({aTob}), "");
        descriptor->AddSelectClass(SelectClassInfo(*classC, "this", false)
            .SetRelatedPropertyPaths({cTob}), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, *CreateRelatedField(CreateCategory(*classB), NESTED_CONTENT_FIELD_NAME(classA, classB), *classB, { aTob },
            {
            CreatePropertiesField(CreateCategory(*classB), CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), *classB, *classB->GetPropertyP("Prop"))),
            }));
        AddField(*descriptor, *CreateRelatedField(CreateCategory(*classB), NESTED_CONTENT_FIELD_NAME(classC, classB), *classB, { cTob },
            {
            CreatePropertiesField(CreateCategory(*classB), CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), *classB, *classB->GetPropertyP("Prop")), FIELD_NAME_C(classB, "Prop", 2)),
            }));

        ComplexQueryBuilderPtr q1 = ComplexQueryBuilder::Create();
        q1->SelectContract(*CreateQueryContract(1, *descriptor, classA, *q1), "this");
        q1->From(*classA, false, "this");
        q1->Join(aTob);
        q1->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId::FromString(a->GetInstanceId().c_str()))});

        ComplexQueryBuilderPtr q2 = ComplexQueryBuilder::Create();
        q2->SelectContract(*CreateQueryContract(2, *descriptor, classC, *q2), "this");
        q2->From(*classC, false, "this");
        q2->Join(cTob);
        q2->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId::FromString(c->GetInstanceId().c_str()))});

        UnionQueryBuilderPtr query = UnionQueryBuilder::Create({q1, q2});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif

        return query;
    });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AppliesRelatedPropertiesSpecificationFromContentModifier, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (ContentQueryBuilderTests, AppliesRelatedPropertiesSpecificationFromContentModifier)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(GetDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(GetDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(GetDb(), *relAB, *a, *b);

    // create the specs
    m_ruleset->AddPresentationRule(*new ContentModifier(classA->GetSchema().GetName(), classA->GetName()));
    m_ruleset->GetContentModifierRules().back()->AddRelatedProperty(*new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification(*new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)),
        {new PropertySpecification("*")}, RelationshipMeaning::RelatedInstance));
    ContentInstancesOfSpecificClassesSpecification spec(1, false, "", {CreateMultiSchemaClass({classA}, false)}, {}, false);

    // get the query
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor);
    ValidateQueries(querySet, [&]()
        {
        RelatedClassPath aTob({RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true))});

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false)
            .SetRelatedPropertyPaths({aTob}), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, *CreateRelatedField(CreateCategory(*classB), NESTED_CONTENT_FIELD_NAME(classA, classB), *classB, {aTob},
            {
            CreatePropertiesField(CreateCategory(*classB), CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), *classB, *classB->GetPropertyP("Prop"))),
            }));

        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query), "this");
        query->From(*classA, false, "this");
        query->Join(aTob);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesntApplyRelatedPropertiesSpecificationFromContentModifierToNestedRelatedClasses, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="PropC" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (ContentQueryBuilderTests, DoesntApplyRelatedPropertiesSpecificationFromContentModifierToNestedRelatedClasses)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetECClass("B_To_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(GetDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(GetDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(GetDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(GetDb(), *relAB, *a, *b);
    RulesEngineTestHelpers::InsertRelationship(GetDb(), *relBC, *b, *c);

    // create the specs
    m_ruleset->AddPresentationRule(*new ContentModifier(classB->GetSchema().GetName(), classB->GetName()));
    m_ruleset->GetContentModifierRules().back()->AddRelatedProperty(*new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification(*new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)),
        { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance));

    ContentInstancesOfSpecificClassesSpecification spec(1, false, "", { CreateMultiSchemaClass({classA}, false) }, {}, false);
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification(*new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)),
        { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance));

    // get the query
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor);
    ValidateQueries(querySet, [&]()
        {
        RelatedClassPath aTob({ RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS (* classB, 0), true)) });

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false)
            .SetRelatedPropertyPaths({ aTob }), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, *CreateRelatedField(CreateCategory(*classB), NESTED_CONTENT_FIELD_NAME(classA, classB), *classB, { aTob },
            {
            CreatePropertiesField(CreateCategory(*classB), CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), *classB, *classB->GetPropertyP("PropB"))),
            }));

        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query), "this");
        query->From(*classA, false, "this");
        query->Join(aTob);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CreatesContentFieldsForXToManyRelatedInstanceProperties, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (ContentQueryBuilderTests, CreatesContentFieldsForXToManyRelatedInstanceProperties)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(GetDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(GetDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(GetDb(), *relAB, *a, *b);

    // create the specs
    ContentInstancesOfSpecificClassesSpecification spec(1, false, "", {CreateMultiSchemaClass({classA}, false)}, {}, false);
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification(*new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)),
        {new PropertySpecification("*")}, RelationshipMeaning::RelatedInstance));

    // get the query
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor);
    ValidateQueries(querySet, [&]()
        {
        RelatedClassPath aTob{RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true))};

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false)
            .SetRelatedPropertyPaths({aTob}), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, *CreateRelatedField(CreateCategory(*classB), NESTED_CONTENT_FIELD_NAME(classA, classB), *classB, {aTob},
            {
            CreatePropertiesField(CreateCategory(*classB), CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), *classB, *classB->GetPropertyP("Prop")))
            }));

        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query), "this");
        query->From(*classA, false, "this");
        query->Join(aTob);
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CreatesNestedContentFieldsForXToManyRelatedInstanceProperties, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="PropC" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (ContentQueryBuilderTests, CreatesNestedContentFieldsForXToManyRelatedInstanceProperties)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetECClass("B_To_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(GetDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(GetDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(GetDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(GetDb(), *relAB, *a, *b);
    RulesEngineTestHelpers::InsertRelationship(GetDb(), *relBC, *b, *c);

    // create the specs
    ContentInstancesOfSpecificClassesSpecification spec(1, false, "", {CreateMultiSchemaClass({classA}, false)}, {}, false);
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification(*new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)),
        {new PropertySpecification("*")}, RelationshipMeaning::RelatedInstance));
    spec.GetRelatedProperties().back()->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification(*new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)),
        {new PropertySpecification("*")}, RelationshipMeaning::RelatedInstance));

    // get the query
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor);
    ValidateQueries(querySet, [&]()
        {
        auto bCategory = CreateCategory(*classB);
        auto cCategory = CreateCategory(*classC, bCategory);

        RelatedClass aTob(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true));
        RelatedClass bToc(*classB, SelectClass<ECRelationshipClass>(*relBC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relBC, 0)), true, SelectClass<ECClass>(*classC, RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0), true));

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false)
            .SetRelatedPropertyPaths(
                {
                {aTob},
                {aTob, bToc},
                }), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, *CreateRelatedField(bCategory, NESTED_CONTENT_FIELD_NAME(classA, classB), *classB, {aTob},
            {
            CreatePropertiesField(bCategory, CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), *classB, *classB->GetPropertyP("PropB"))),
            CreateRelatedField(cCategory, NESTED_CONTENT_FIELD_NAME(classB, classC), *classC, {bToc},
                {
                CreatePropertiesField(cCategory, CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0), *classC, *classC->GetPropertyP("PropC")))
                })
            }));

        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query), "this");
        query->From(*classA, false, "this");
        query->Join(aTob);
        query->Join({aTob, bToc});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(CreatesNestedContentFieldsForXToManySameInstanceProperties, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="PropC" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ContentQueryBuilderTests, CreatesNestedContentFieldsForXToManySameInstanceProperties)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetECClass("B_To_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(GetDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(GetDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(GetDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(GetDb(), *relAB, *a, *b);
    RulesEngineTestHelpers::InsertRelationship(GetDb(), *relBC, *b, *c);

    // create the specs
    ContentInstancesOfSpecificClassesSpecification spec(1, false, "", {CreateMultiSchemaClass({classA}, false)}, {}, false);
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification(*new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)),
        {new PropertySpecification("*")}, RelationshipMeaning::RelatedInstance));
    spec.GetRelatedProperties().back()->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification(*new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)),
        {new PropertySpecification("*")}, RelationshipMeaning::SameInstance));

    // get the query
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor);
    ValidateQueries(querySet, [&]()
        {
        auto bCategory = CreateCategory(*classB);
        auto cCategory = CreateCategory(*classC, bCategory);

        RelatedClass aTob(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true));
        RelatedClass bToc(*classB, SelectClass<ECRelationshipClass>(*relBC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relBC, 0)), true, SelectClass<ECClass>(*classC, RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0), true));

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false)
            .SetRelatedPropertyPaths(
                {
                {aTob},
                {aTob, bToc},
                }), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, *CreateRelatedField(bCategory, NESTED_CONTENT_FIELD_NAME(classA, classB), *classB, {aTob},
            {
            CreatePropertiesField(bCategory, CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), *classB, *classB->GetPropertyP("PropB"))),
            CreateRelatedField(bCategory, NESTED_CONTENT_FIELD_NAME(classB, classC), *classC, {bToc},
                {
                CreatePropertiesField(cCategory, CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0), *classC, *classC->GetPropertyP("PropC")))
                })
            }));

        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query), "this");
        query->From(*classA, false, "this");
        query->Join(aTob);
        query->Join({aTob, bToc});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SetsShowImagesFlag, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, SetsShowImagesFlag)
    {
    ECClassCP classA = GetECClass("A");

    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    spec.SetShowImages(true);

    TestParsedInput info(*classA, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("Prop")));
        descriptor->AddContentFlag(ContentFlags::ShowImages);

        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query), "this");
        query->From(*classA, false, "this");
        query->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123))});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SetsShowLabelsFlagForGridContentType, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, SetsShowLabelsFlagForGridContentType)
    {
    ECClassCP classA = GetECClass("A");

    m_descriptorBuilder->GetContext().SetPreferredDisplayType(ContentDisplayType::Grid);

    SelectedNodeInstancesSpecification spec(1, false, "", "", false);

    TestParsedInput info(*classA, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor(ContentDisplayType::Grid);
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("Prop")));
        descriptor->AddContentFlag(ContentFlags::ShowLabels);

        SelectClass<ECClass> selectClass(*classA, "this", false);
        ComplexQueryBuilderPtr nested = ComplexQueryBuilder::Create();
        nested->SelectContract(*CreateQueryContract(1, *descriptor, classA, *nested, CreateDisplayLabelField(selectClass)), "this");
        nested->From(selectClass);
        nested->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123))});

#ifdef WIP_SORTING_GRID_CONTENT
        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectAll();
        query->From(*nested);
        query->OrderBy(Utf8PrintfString(FUNCTION_NAME_GetSortingValue "(%s), %s", ContentQueryContract::DisplayLabelFieldName, ContentQueryContract::ECInstanceIdFieldName).c_str());
        return query;
#else
        return nested;
#endif
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SetsNoFieldsAndKeysOnlyFlagForGraphicsContentType, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, SetsNoFieldsAndKeysOnlyFlagForGraphicsContentType)
    {
    ECClassCP classA = GetECClass("A");

    m_descriptorBuilder->GetContext().SetPreferredDisplayType(ContentDisplayType::Graphics);

    SelectedNodeInstancesSpecification spec(1, false, "", "", false);

    TestParsedInput info(*classA, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor(ContentDisplayType::Graphics);
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false), "");
        descriptor->AddContentFlag(ContentFlags::KeysOnly);
        descriptor->AddContentFlag(ContentFlags::NoFields);
        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));

        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query), "this");
        query->From(*classA, false, "this");
        query->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123))});
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SetsNoFieldsAndShowLabelsFlagsForListContentType, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, SetsNoFieldsAndShowLabelsFlagsForListContentType)
    {
    ECClassCP classA = GetECClass("A");

    m_descriptorBuilder->GetContext().SetPreferredDisplayType(ContentDisplayType::List);

    SelectedNodeInstancesSpecification spec(1, false, "", "", false);

    TestParsedInput info(*classA, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor(ContentDisplayType::List);
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false), "");
        descriptor->AddContentFlag(ContentFlags::ShowLabels);
        descriptor->AddContentFlag(ContentFlags::NoFields);
        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));

        SelectClass<ECClass> selectClass(*classA, "this", false);
        ComplexQueryBuilderPtr nested = ComplexQueryBuilder::Create();
        nested->SelectContract(*CreateQueryContract(1, *descriptor, classA, *nested, CreateDisplayLabelField(selectClass)), "this");
        nested->From(selectClass);
        nested->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123))});

#ifdef WIP_SORTING_GRID_CONTENT
        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectAll();
        query->From(*nested);
        query->OrderBy(Utf8PrintfString(FUNCTION_NAME_GetSortingValue "(%s), %s", ContentQueryContract::DisplayLabelFieldName, ContentQueryContract::ECInstanceIdFieldName).c_str());
        return query;
#else
        return nested;
#endif
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(AppliesRelatedInstanceSpecificationForTheSameXToManyRelationshipAndClassTwoTimes, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (ContentQueryBuilderTests, AppliesRelatedInstanceSpecificationForTheSameXToManyRelationshipAndClassTwoTimes)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();

    SelectedNodeInstancesSpecification spec(1, false, classA->GetSchema().GetName(), classA->GetName(), false);
    spec.AddRelatedInstance(*new RelatedInstanceSpecification(RelationshipPathSpecification(
        *new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)), "aTob"));
    spec.AddRelatedInstance(*new RelatedInstanceSpecification(RelationshipPathSpecification(
        *new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)), "aTob2"));

    TestParsedInput info(*classA, ECInstanceId((uint64_t) 123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        RelatedClass aTob1(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, "aTob", true));
        RelatedClass aTob2(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, "aTob2", true));

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false)
            .SetRelatedInstancePaths({ {aTob1}, {aTob2} }), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));

        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query, nullptr, { {aTob1}, {aTob2} }), "this");
        query->From(*classA, false, "this");
        query->Join(aTob1);
        query->Join(aTob2);
        query->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123))});

        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(JoinsRelationshipsThatHaveOnlyOneTarget, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="PropC" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="A_To_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (ContentQueryBuilderTests, JoinsRelationshipsThatHaveOnlyOneTarget)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relAC = GetECClass("A_To_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(GetDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(GetDb(), *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(GetDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(GetDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(GetDb(), *relAB, *a, *b1);
    RulesEngineTestHelpers::InsertRelationship(GetDb(), *relAB, *a, *b2);
    RulesEngineTestHelpers::InsertRelationship(GetDb(), *relAC, *a, *c);

    SelectedNodeInstancesSpecification spec;
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(*new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward)),
        { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance));
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(*new RelationshipStepSpecification(relAC->GetFullName(), RequiredRelationDirection_Forward)),
        { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance));

    TestParsedInput info(*classA, ECInstanceId(ECInstanceId::FromString(a->GetInstanceId().c_str())));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        RelatedClassPath aTobPath = {RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true))};
        aTobPath.SetTargetsCount(2);
        RelatedClass aToc(*classA, SelectClass<ECRelationshipClass>(*relAC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAC, 0)), true, SelectClass<ECClass>(*classC, RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0), true));

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false)
            .SetRelatedPropertyPaths(
                {
                aTobPath,
                {aToc}
                }), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, ContentDescriptor::Property("this", *classA, *classA->GetPropertyP("PropA")));
        AddField(*descriptor, *CreateRelatedField(CreateCategory(*classB), NESTED_CONTENT_FIELD_NAME(classA, classB), *classB, aTobPath,
            {
            CreatePropertiesField(CreateCategory(*classB), CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), *classB, *classB->GetPropertyP("PropB"))),
            }));
        AddField(*descriptor, *CreateRelatedField(CreateCategory(*classC), NESTED_CONTENT_FIELD_NAME(classA, classC), *classC, {aToc},
            {
            CreatePropertiesField(CreateCategory(*classC), CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0), *classC, *classC->GetPropertyP("PropC"))),
            }));

        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query), "this");
        query->From(*classA, false, "this");
        query->Join(aToc);
        query->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId::FromString(a->GetInstanceId().c_str()))});
        return query;
        });
    }
