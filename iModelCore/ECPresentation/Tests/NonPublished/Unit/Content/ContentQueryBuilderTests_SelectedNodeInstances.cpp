/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ContentQueryBuilderTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_ReturnsNullDescriptorWhenNoSelectedNodes)
    {
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, TestParsedInput());
    ASSERT_TRUE(descriptor.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_ReturnsInstanceQueryWhenSelectedOneInstanceNode, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_ReturnsInstanceQueryWhenSelectedOneInstanceNode)
    {
    ECClassCP classA = GetECClass("A");
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);

    TestParsedInput info(*classA, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("PropA")));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
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
DEFINE_SCHEMA(SelectedNodeInstances_ReturnsInstanceQueryWhenSelectedMultipleInstanceNodesOfTheSameClass, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_ReturnsInstanceQueryWhenSelectedMultipleInstanceNodesOfTheSameClass)
    {
    ECClassCP classA = GetECClass("A");
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);

    TestParsedInput info(*classA, {ECInstanceId((uint64_t)123), ECInstanceId((uint64_t)125)});
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("PropA")));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query), "this");
        query->From(*classA, false, "this");
        query->Where("[this].[ECInstanceId] IN (?,?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)), std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)125))});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_ReturnsInstanceQueryWhenSelectedMultipleInstanceNodesOfDifferentClasses, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_ReturnsInstanceQueryWhenSelectedMultipleInstanceNodesOfDifferentClasses)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);

    TestParsedInput info({
        bpair<ECClassCP, ECInstanceId>(classA, {ECInstanceId((uint64_t)123)}),
        bpair<ECClassCP, ECInstanceId>(classB, {ECInstanceId((uint64_t)123)})
        });
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false), "");
        descriptor->AddSelectClass(SelectClassInfo(*classB, "this", false), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("PropA")));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classB, *classB->GetPropertyP("PropB")));

        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query1), "this");
        query1->From(*classA, false, "this");
        query1->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123))});

        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*CreateQueryContract(2, *descriptor, classB, *query2), "this");
        query2->From(*classB, false, "this");
        query2->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123))});

        UnionContentQueryPtr query = UnionContentQuery::Create({ query1, query2 });
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_FiltersSelectedNodesBySchemaName, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_FiltersSelectedNodesBySchemaName)
    {
    ECClassCP classA = GetECClass("A");
    SelectedNodeInstancesSpecification spec(1, false, "OtherSchema", "", false);

    TestParsedInput info(*classA, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_FiltersSelectedNodesByClassName, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_FiltersSelectedNodesByClassName)
    {
    ECClassCP classA = GetECClass("A");
    SelectedNodeInstancesSpecification spec(1, false, "", "OtherClass", false);

    TestParsedInput info(*classA, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_FiltersSelectedNodesByClassNamePolymorphically, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="PropB" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
        <ECProperty propertyName="PropC" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_FiltersSelectedNodesByClassNamePolymorphically)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    SelectedNodeInstancesSpecification spec(1, false, "", "B", true);

    TestParsedInput info({
        bpair<ECClassCP, ECInstanceId>(classA, {ECInstanceId((uint64_t)123)}),
        bpair<ECClassCP, ECInstanceId>(classB, {ECInstanceId((uint64_t)123)}),
        bpair<ECClassCP, ECInstanceId>(classC, {ECInstanceId((uint64_t)123)})
        });
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classB, "this", false), "");
        descriptor->AddSelectClass(SelectClassInfo(*classC, "this", false), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));

        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classB, *classB->GetPropertyP("PropA")));
        descriptor->GetAllFields().back()->AsPropertiesField()->AddProperty(CreateProperty("this", *classC, *classC->GetPropertyP("PropA")));

        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classB, *classB->GetPropertyP("PropB")));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classC, *classC->GetPropertyP("PropC")));

        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*CreateQueryContract(1, *descriptor, classB, *query1), "this");
        query1->From(*classB, false, "this");
        query1->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123))});

        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*CreateQueryContract(2, *descriptor, classC, *query2), "this");
        query2->From(*classC, false, "this");
        query2->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123))});

        UnionContentQueryPtr query = UnionContentQuery::Create({query1, query2});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_RemovesHiddenProperty, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA1" typeName="string" />
        <ECProperty propertyName="PropA2" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_RemovesHiddenProperty)
    {
    ECClassCP classA = GetECClass("A");
    SelectedNodeInstancesSpecification spec(1, false, classA->GetSchema().GetName(), classA->GetName(), false);
    spec.AddPropertyOverride(*new PropertySpecification("PropA1", 1000, "", nullptr, false));

    TestParsedInput info(*classA, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("PropA2")));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
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
DEFINE_SCHEMA(SelectedNodeInstances_RemovesAllHiddenPropertiesWhenHidingAtSpecificationLevel, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="ElementProperty" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="PhysicalElement">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="PhysicalProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_RemovesAllHiddenPropertiesWhenHidingAtSpecificationLevel)
    {
    ECClassCP ecClass = GetECClass("PhysicalElement");

    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    spec.AddPropertyOverride(*new PropertySpecification("*", 1000, "", nullptr, false));

    TestParsedInput info(*ecClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*ecClass, "this", false), "");
        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, ecClass, *query), "this");
        query->From(*ecClass, false, "this");
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
DEFINE_SCHEMA(SelectedNodeInstances_RemovesAllHiddenBaseClassPropertiesWhenHidingAtContentModifierLevel, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="ElementProperty" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="PhysicalElement">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="PhysicalProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_RemovesAllHiddenBaseClassPropertiesWhenHidingAtContentModifierLevel)
    {
    ECClassCP baseClass = GetECClass("Element");
    ECClassCP derivedClass = GetECClass("PhysicalElement");

    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    ContentModifier* modifier = new ContentModifier(baseClass->GetSchema().GetName(), baseClass->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("*", 1000, "", nullptr, false));
    m_ruleset->AddPresentationRule(*modifier);

    TestParsedInput info(*derivedClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*derivedClass, "this", false), "");
        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, ContentDescriptor::Property("this", *derivedClass, *derivedClass->GetPropertyP("PhysicalProperty")));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, derivedClass, *query), "this");
        query->From(*derivedClass, false, "this");
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
DEFINE_SCHEMA(SelectedNodeInstances_RemovesAllHiddenDerivedClassPropertiesWhenHidingAtContentModifierLevel, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="ElementProperty" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="PhysicalElement">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="PhysicalProperty" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_RemovesAllHiddenDerivedClassPropertiesWhenHidingAtContentModifierLevel)
    {
    ECClassCP derivedClass = GetECClass("PhysicalElement");

    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    ContentModifier* modifier = new ContentModifier(derivedClass->GetSchema().GetName(), derivedClass->GetName());
    modifier->AddPropertyOverride(*new PropertySpecification("*", 1000, "", nullptr, false));
    m_ruleset->AddPresentationRule(*modifier);

    TestParsedInput info(*derivedClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*derivedClass, "this", false), "");
        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, derivedClass, *query), "this");
        query->From(*derivedClass, false, "this");
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
DEFINE_SCHEMA(SelectedNodeInstances_RemovesMultipleHiddenProperties, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA1" typeName="string" />
        <ECProperty propertyName="PropA2" typeName="string" />
        <ECProperty propertyName="PropA3" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_RemovesMultipleHiddenProperties)
    {
    ECClassCP classA = GetECClass("A");
    SelectedNodeInstancesSpecification spec(1, false, classA->GetSchema().GetName(), classA->GetName(), false);
    spec.AddPropertyOverride(*new PropertySpecification("PropA1", 1000, "", nullptr, false));
    spec.AddPropertyOverride(*new PropertySpecification("PropA3", 1000, "", nullptr, false));

    TestParsedInput info(*classA, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("PropA2")));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
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
DEFINE_SCHEMA(SelectedNodeInstances_RemovesMultipleHiddenPropertiesOfDifferentClasses, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA1" typeName="string" />
        <ECProperty propertyName="PropA2" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB1" typeName="string" />
        <ECProperty propertyName="PropB2" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_RemovesMultipleHiddenPropertiesOfDifferentClasses)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    SelectedNodeInstancesSpecification spec(1, false, classA->GetSchema().GetName(), "", false);
    spec.AddPropertyOverride(*new PropertySpecification("PropA2", 1000, "", nullptr, false));
    spec.AddPropertyOverride(*new PropertySpecification("PropB2", 1000, "", nullptr, false));

    TestParsedInput info({
        bpair<ECClassCP, ECInstanceId>(classA, {ECInstanceId((uint64_t)123)}),
        bpair<ECClassCP, ECInstanceId>(classB, {ECInstanceId((uint64_t)123)})
        });
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false), "");
        descriptor->AddSelectClass(SelectClassInfo(*classB, "this", false), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("PropA1")));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classB, *classB->GetPropertyP("PropB1")));

        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query1), "this");
        query1->From(*classA, false, "this");
        query1->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123))});

        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*CreateQueryContract(2, *descriptor, classB, *query2), "this");
        query2->From(*classB, false, "this");
        query2->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123))});

        UnionContentQueryPtr query = UnionContentQuery::Create({query1, query2});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_AddsSingleRelatedProperty, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB1" typeName="string" />
        <ECProperty propertyName="PropB2" typeName="string" />
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
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_AddsSingleRelatedProperty)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(GetDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(GetDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(GetDb(), *relAB, *a, *b);

    SelectedNodeInstancesSpecification spec(1, false, classA->GetSchema().GetName(), classA->GetName(), false);
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification(*new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)),
        {new PropertySpecification("PropB1")}, RelationshipMeaning::RelatedInstance));

    TestParsedInput info(*classA, ECInstanceId(ECInstanceId::FromString(a->GetInstanceId().c_str())));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        RelatedClassPath aTobPath({RelatedClass(*classA , SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true))});

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false)
            .SetRelatedPropertyPaths({aTobPath}), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, ContentDescriptor::Property("this", *classA, *classA->GetPropertyP("PropA")));
        AddField(*descriptor, *CreateRelatedField(CreateCategory(*classB), NESTED_CONTENT_FIELD_NAME(classA, classB), *classB, aTobPath,
            {
            CreatePropertiesField(CreateCategory(*classB), CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), *classB, *classB->GetPropertyP("PropB1"))),
            }));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query), "this");
        query->From(*classA, false, "this");
        query->Join(aTobPath);
        query->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId::FromString(a->GetInstanceId().c_str()))});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_AddsMultipleRelatedProperties, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB1" typeName="string" />
        <ECProperty propertyName="PropB2" typeName="string" />
        <ECProperty propertyName="PropB3" typeName="string" />
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
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_AddsMultipleRelatedProperties)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(GetDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(GetDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(GetDb(), *relAB, *a, *b);

    SelectedNodeInstancesSpecification spec(1, false, classA->GetSchema().GetName(), classA->GetName(), false);
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification(*new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)),
        {new PropertySpecification("PropB1"), new PropertySpecification("PropB2")}, RelationshipMeaning::RelatedInstance));

    TestParsedInput info(*classA, ECInstanceId(ECInstanceId::FromString(a->GetInstanceId().c_str())));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        RelatedClassPath aTobPath({RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true))});

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false)
            .SetRelatedPropertyPaths({aTobPath}), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("PropA")));
        AddField(*descriptor, *CreateRelatedField(CreateCategory(*classB), NESTED_CONTENT_FIELD_NAME(classA, classB), *classB, aTobPath,
            {
            CreatePropertiesField(CreateCategory(*classB), CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), *classB, *classB->GetPropertyP("PropB1"))),
            CreatePropertiesField(CreateCategory(*classB), CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), *classB, *classB->GetPropertyP("PropB2"))),
            }));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query), "this");
        query->From(*classA, false, "this");
        query->Join(aTobPath);
        query->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId::FromString(a->GetInstanceId().c_str()))});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_AddsAllRelatedProperties, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB1" typeName="string" />
        <ECProperty propertyName="PropB2" typeName="string" />
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
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_AddsAllRelatedProperties)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(GetDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(GetDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(GetDb(), *relAB, *a, *b);

    SelectedNodeInstancesSpecification spec(1, false, classA->GetSchema().GetName(), classA->GetName(), false);
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification(*new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)),
        {new PropertySpecification("*")}, RelationshipMeaning::RelatedInstance));

    TestParsedInput info(*classA, ECInstanceId(ECInstanceId::FromString(a->GetInstanceId().c_str())));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        RelatedClassPath aTobPath({RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true))});

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false)
            .SetRelatedPropertyPaths({aTobPath}), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("PropA")));
        AddField(*descriptor, *CreateRelatedField(CreateCategory(*classB), NESTED_CONTENT_FIELD_NAME(classA, classB), *classB, aTobPath,
            {
            CreatePropertiesField(CreateCategory(*classB), CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), *classB, *classB->GetPropertyP("PropB1"))),
            CreatePropertiesField(CreateCategory(*classB), CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), *classB, *classB->GetPropertyP("PropB2"))),
            }));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query), "this");
        query->From(*classA, false, "this");
        query->Join(aTobPath);
        query->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId::FromString(a->GetInstanceId().c_str()))});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_AddsBackwardRelatedProperties, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="string" />
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
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_AddsBackwardRelatedProperties)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(GetDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(GetDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(GetDb(), *relAB, *a, *b);

    SelectedNodeInstancesSpecification spec(1, false, classB->GetSchema().GetName(), classB->GetName(), false);
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification(*new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Backward)),
        {new PropertySpecification("*")}, RelationshipMeaning::RelatedInstance));

    TestParsedInput info(*classB, ECInstanceId(ECInstanceId::FromString(b->GetInstanceId().c_str())));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        RelatedClassPath bToaPath({RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, RULES_ENGINE_RELATED_CLASS_ALIAS(*classA, 0), true))});

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classB, "this", false)
            .SetRelatedPropertyPaths({bToaPath}), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, ContentDescriptor::Property("this", *classB, *classB->GetPropertyP("PropB")));
        AddField(*descriptor, *CreateRelatedField(CreateCategory(*classA), NESTED_CONTENT_FIELD_NAME(classB, classA), *classA, bToaPath,
            {
            CreatePropertiesField(CreateCategory(*classA), CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classA, 0), *classA, *classA->GetPropertyP("PropA"))),
            }));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classB, *query), "this");
        query->From(*classB, false, "this");
        query->Join(bToaPath);
        query->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId::FromString(b->GetInstanceId().c_str()))});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* note: this test not only tests relationships on both directions, but also:
* - multiple specified relationships between different classes;
* - empty related classes property (defaults to "any class");
* - a property that exists in different related classes.
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_AddsBothDirectionsRelatedProperties, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="PropC" typeName="string" />
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
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_AddsBothDirectionsRelatedProperties)
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

    SelectedNodeInstancesSpecification spec(1, false, classB->GetSchema().GetName(), classB->GetName(), false);
    Utf8PrintfString relationshipClasses("%s:%s,%s", relAB->GetSchema().GetName().c_str(), relAB->GetName().c_str(), relBC->GetName().c_str());
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Both, relationshipClasses, "", "PropA,PropC", RelationshipMeaning::RelatedInstance));

    TestParsedInput info(*classB, ECInstanceId(ECInstanceId::FromString(b->GetInstanceId().c_str())));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        RelatedClassPath bToaPath({RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, RULES_ENGINE_RELATED_CLASS_ALIAS(*classA, 0), true))});
        RelatedClassPath bTocPath({RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relBC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relBC, 0)), true, SelectClass<ECClass>(*classC, RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0), true))});

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classB, "this", false)
            .SetRelatedPropertyPaths({bToaPath, bTocPath}), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classB, *classB->GetPropertyP("PropB")));
        AddField(*descriptor, *CreateRelatedField(CreateCategory(*classA), NESTED_CONTENT_FIELD_NAME(classB, classA), *classA, bToaPath,
            {
            CreatePropertiesField(CreateCategory(*classA), CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classA, 0), *classA, *classA->GetPropertyP("PropA"))),
            }));
        AddField(*descriptor, *CreateRelatedField(CreateCategory(*classC), NESTED_CONTENT_FIELD_NAME(classB, classC), *classC, bTocPath,
            {
            CreatePropertiesField(CreateCategory(*classC), CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0), *classC, *classC->GetPropertyP("PropC"))),
            }));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classB, *query), "this");
        query->From(*classB, false, "this");
        query->Join(bToaPath);
        query->Join(bTocPath);
        query->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId::FromString(b->GetInstanceId().c_str()))});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_AddsPropertiesOfTheSameClassFoundByFollowingDifferentRelationships, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="A_Has_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_AddsPropertiesOfTheSameClassFoundByFollowingDifferentRelationships)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAToB = GetECClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relAHasB = GetECClass("A_Has_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(GetDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(GetDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(GetDb(), *relAToB, *a, *b);
    RulesEngineTestHelpers::InsertRelationship(GetDb(), *relAHasB, *a, *b);

    SelectedNodeInstancesSpecification spec(1, false, classA->GetSchema().GetName(), classA->GetName(), false);
    Utf8PrintfString relationshipClasses("%s:%s,%s", relAToB->GetSchema().GetName().c_str(), relAToB->GetName().c_str(), relAHasB->GetName().c_str());
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, relationshipClasses, "", "PropB", RelationshipMeaning::RelatedInstance));

    TestParsedInput info(*classA, ECInstanceId(ECInstanceId::FromString(a->GetInstanceId().c_str())));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        RelatedClassPath aHasbPath({RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAHasB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAHasB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true))});
        RelatedClassPath aTobPath({RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAToB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAToB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 1), true))});

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false)
            .SetRelatedPropertyPaths({aHasbPath, aTobPath}), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("PropA")));
        AddField(*descriptor, *CreateRelatedField(CreateCategory(*classB), NESTED_CONTENT_FIELD_NAME(classA, classB), *classB, aHasbPath,
            {
            CreatePropertiesField(CreateCategory(*classB), CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), *classB, *classB->GetPropertyP("PropB"))),
            }));
        AddField(*descriptor, *CreateRelatedField(CreateCategory(*classB), NESTED_CONTENT_FIELD_NAME_C(classA, classB, 2), *classB, aTobPath,
            {
            CreatePropertiesField(CreateCategory(*classB), CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 1), *classB, *classB->GetPropertyP("PropB")), FIELD_NAME_C(classB, "PropB", 2)),
            }));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query), "this");
        query->From(*classA, false, "this");
        query->Join(aHasbPath);
        query->Join(aTobPath);
        query->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId::FromString(a->GetInstanceId().c_str()))});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_AddsNestedPropertiesOfTheSameClassFoundByFollowingDifferentRelationships, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="PropC" typeName="string" />
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
    <ECRelationshipClass typeName="B_Has_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_AddsNestedPropertiesOfTheSameClassFoundByFollowingDifferentRelationships)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAToB = GetECClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBToC = GetECClass("B_To_C")->GetRelationshipClassCP();
    ECRelationshipClassCP relBHasC = GetECClass("B_Has_C")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(GetDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(GetDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(GetDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(GetDb(), *relAToB, *a, *b);
    RulesEngineTestHelpers::InsertRelationship(GetDb(), *relBToC, *b, *c);
    RulesEngineTestHelpers::InsertRelationship(GetDb(), *relBHasC, *b, *c);

    SelectedNodeInstancesSpecification spec(1, false, classA->GetSchema().GetName(),classA->GetName(), false);

    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification(*new RelationshipStepSpecification(relAToB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)),
        {}, RelationshipMeaning::RelatedInstance);

    Utf8PrintfString relationshipClasses("%s:%s,%s", relBToC->GetSchema().GetName().c_str(), relBToC->GetName().c_str(), relBHasC->GetName().c_str());
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, relationshipClasses, "", "PropC", RelationshipMeaning::RelatedInstance));
    spec.AddRelatedProperty(*relatedPropertiesSpec);

    TestParsedInput info(*classA, ECInstanceId(ECInstanceId::FromString(a->GetInstanceId().c_str())));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        auto bCategory = CreateCategory(*classB);
        auto cCategory = CreateCategory(*classC, bCategory);

        RelatedClass aTob(*classA, SelectClass<ECRelationshipClass>(*relAToB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAToB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true));
        RelatedClass bHasc(*classB, SelectClass<ECRelationshipClass>(*relBHasC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relBHasC, 0)), true, SelectClass<ECClass>(*classC, RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0), true));
        RelatedClass bToc(*classB, SelectClass<ECRelationshipClass>(*relBToC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relBToC, 0)), true, SelectClass<ECClass>(*classC, RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 1), true));

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false)
            .SetRelatedPropertyPaths(
                {
                {aTob, bHasc},
                {aTob, bToc},
                }), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("PropA")));
        AddField(*descriptor, *CreateRelatedField(bCategory, NESTED_CONTENT_FIELD_NAME(classA, classB), *classB, {aTob},
            {
            CreateRelatedField(cCategory, NESTED_CONTENT_FIELD_NAME(classB, classC), *classC, {bHasc},
                {
                CreatePropertiesField(cCategory, CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0), *classC, *classC->GetPropertyP("PropC"))),
                }),
            CreateRelatedField(cCategory, NESTED_CONTENT_FIELD_NAME_C(classB, classC, 2), *classC, {bToc},
                {
                CreatePropertiesField(cCategory, CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 1), *classC, *classC->GetPropertyP("PropC")), FIELD_NAME_C(classC, "PropC", 2))
                })
            }));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query), "this");
        query->From(*classA, false, "this");
        query->Join({aTob, bHasc});
        query->Join({aTob, bToc});
        query->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId::FromString(a->GetInstanceId().c_str()))});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_AddsNestedRelatedProperties, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="PropC" typeName="string" />
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
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_AddsNestedRelatedProperties)
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

    SelectedNodeInstancesSpecification spec(1, false, classA->GetSchema().GetName(), classA->GetName(), false);
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification(*new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)),
        {}, RelationshipMeaning::RelatedInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification(*new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)),
        {new PropertySpecification("PropC")}, RelationshipMeaning::RelatedInstance));
    spec.AddRelatedProperty(*relatedPropertiesSpec);

    TestParsedInput info(*classA, ECInstanceId(ECInstanceId::FromString(a->GetInstanceId().c_str())));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
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
                {aTob, bToc}
                }), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("PropA")));
        AddField(*descriptor, *CreateRelatedField(bCategory, NESTED_CONTENT_FIELD_NAME(classA, classB), *classB, {aTob},
            {
            CreateRelatedField(cCategory, NESTED_CONTENT_FIELD_NAME(classB, classC), *classC, {bToc},
                {
                CreatePropertiesField(cCategory, CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0), *classC, *classC->GetPropertyP("PropC"))),
                })
            }));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query), "this");
        query->From(*classA, false, "this");
        query->Join({aTob, bToc});
        query->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId::FromString(a->GetInstanceId().c_str()))});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_AddsNestedRelatedProperties2, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="PropC" typeName="string" />
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
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_AddsNestedRelatedProperties2)
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

    SelectedNodeInstancesSpecification spec(1, false, classA->GetSchema().GetName(), classA->GetName(), false);
    RelatedPropertiesSpecificationP relatedPropertiesSpec = new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification(*new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)),
        {new PropertySpecification("PropB")}, RelationshipMeaning::RelatedInstance);
    relatedPropertiesSpec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(
        *new RelationshipPathSpecification(*new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)),
        {new PropertySpecification("PropC")}, RelationshipMeaning::RelatedInstance));
    spec.AddRelatedProperty(*relatedPropertiesSpec);

    TestParsedInput info(*classA, ECInstanceId(ECInstanceId::FromString(a->GetInstanceId().c_str())));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
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
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("PropA")));
        AddField(*descriptor, *CreateRelatedField(bCategory, NESTED_CONTENT_FIELD_NAME(classA, classB), *classB, {aTob},
            {
            CreatePropertiesField(bCategory, CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), *classB, *classB->GetPropertyP("PropB"))),
            CreateRelatedField(cCategory, NESTED_CONTENT_FIELD_NAME(classB, classC), *classC, {bToc},
                {
                CreatePropertiesField(cCategory, CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0), *classC, *classC->GetPropertyP("PropC")))
                }),
            }));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query), "this");
        query->From(*classA, false, "this");
        query->Join({aTob});
        query->Join({aTob, bToc});
        query->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId::FromString(a->GetInstanceId().c_str()))});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        return query;
        });
    }

#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_SelectsRawValueAndGroupsByDisplayValue, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PointProp" typeName="point3d" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_SelectsRawValueAndGroupsByDisplayValue)
    {
    ECClassCP classA = GetECClass("A");
    TestParsedInput info(*classA, ECInstanceId((uint64_t)123));

    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    spec.AddPropertyOverride(*new PropertySpecification("PointProp", 1000, "", nullptr, true));

    ContentDescriptorPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());
    descriptor->AddContentFlag(ContentFlags::DistinctValues);

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddContentFlag(ContentFlags::DistinctValues);
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("PointProp")));

        ComplexContentQueryPtr nestedQuery = ComplexContentQuery::Create();
        ContentQueryContractPtr contract = CreateQueryContract(1, *descriptor, classA, *nestedQuery);
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(*classA, false, "this");
        nestedQuery->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123))});

        ComplexContentQueryPtr groupedQuery = ComplexContentQuery::Create();
        groupedQuery->SelectAll();
        groupedQuery->From(*nestedQuery);
        groupedQuery->GroupByContract(*contract);

        return groupedQuery;
        });
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_InstanceLabelOverride_AppliedByPriority, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop1" typeName="string" />
        <ECProperty propertyName="Prop2" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_InstanceLabelOverride_AppliedByPriority)
    {
    m_descriptorBuilder->GetContext().SetPreferredDisplayType(ContentDisplayType::Grid);

    ECClassCP classA = GetECClass("A");
    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), "Prop1"));
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(2, false, classA->GetFullName(), "Prop2"));

    TestParsedInput info(*classA, ECInstanceId((uint64_t)123));
    ContentDescriptorPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());
    descriptor->AddContentFlag(ContentFlags::ShowLabels);

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        auto& labelOverride = RegisterForDelete(*new InstanceLabelOverride(1, false, classA->GetFullName(), bvector<InstanceLabelOverrideValueSpecification*>{
            new InstanceLabelOverridePropertyValueSpecification("Prop2"),
            new InstanceLabelOverridePropertyValueSpecification("Prop1")
            }));

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor(ContentDisplayType::Grid);
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false), "");

        ContentDescriptor::DisplayLabelField* displayLabelField = new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0);
        displayLabelField->SetLabelOverrideSpecs(CreateLabelOverrideSpecificationsMap(*classA, labelOverride));
        AddField(*descriptor, *displayLabelField);
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("Prop1")));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("Prop2")));

        descriptor->AddContentFlag(ContentFlags::ShowLabels);

        SelectClass<ECClass> selectClass(*classA, "this", false);
        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query, CreateDisplayLabelField(selectClass, {}, labelOverride.GetValueSpecifications())), "this");
        query->From(selectClass);
        query->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123))});
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_InstanceLabelOverride_OverrideSpecifiedClassInstancesLabelsWhenMultipleClassesSelected, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_InstanceLabelOverride_OverrideSpecifiedClassInstancesLabelsWhenMultipleClassesSelected)
    {
    m_descriptorBuilder->GetContext().SetPreferredDisplayType(ContentDisplayType::Grid);

    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");

    // set up selection
    TestParsedInput info({
        bpair<ECClassCP, ECInstanceId>(classA, {ECInstanceId((uint64_t)1)}),
        bpair<ECClassCP, ECInstanceId>(classB, {ECInstanceId((uint64_t)2)}),
        });

    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "Prop"));

    ContentDescriptorPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());
    descriptor->AddContentFlag(ContentFlags::ShowLabels);

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        auto& labelOverride = RegisterForDelete(*new InstanceLabelOverride(1, false, classB->GetFullName(), bvector<InstanceLabelOverrideValueSpecification*>{
            new InstanceLabelOverridePropertyValueSpecification("Prop")
            }));

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor(ContentDisplayType::Grid);
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false), "");
        descriptor->AddSelectClass(SelectClassInfo(*classB, "this", false), "");

        ContentDescriptor::DisplayLabelField* displayLabelField = new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0);
        displayLabelField->SetLabelOverrideSpecs(CreateLabelOverrideSpecificationsMap(*classB, labelOverride));
        AddField(*descriptor, *displayLabelField);

        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, ContentDescriptor::Property("this", *classA, *classA->GetPropertyP("Prop")));
        descriptor->GetAllFields().back()->AsPropertiesField()->AddProperty(ContentDescriptor::Property("this", *classB, *classB->GetPropertyP("Prop")));
        descriptor->GetAllFields().back()->SetUniqueName(FIELD_NAME((bvector<ECClassCP>{classA, classB}), "Prop"));

        descriptor->AddContentFlag(ContentFlags::ShowLabels);

        SelectClass<ECClass> selectClass1(*classA, "this", false);
        ComplexContentQueryPtr q1 = ComplexContentQuery::Create();
        q1->SelectContract(*CreateQueryContract(1, *descriptor, classA, *q1, CreateDisplayLabelField(selectClass1)), "this");
        q1->From(selectClass1);
        q1->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)1))});

        SelectClass<ECClass> selectClass2(*classB, "this", false);
        ComplexContentQueryPtr q2 = ComplexContentQuery::Create();
        q2->SelectContract(*CreateQueryContract(2, *descriptor, classB, *q2, CreateDisplayLabelField(selectClass2, {}, labelOverride.GetValueSpecifications())), "this");
        q2->From(selectClass2);
        q2->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)2))});

        return UnionContentQuery::Create({ q1, q2 });
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_InstanceLabelOverride_OverrideNavigationProperty, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="string" />
        <ECNavigationProperty propertyName="NavA" relationshipName="A_To_B" direction="Backward" />
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
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_InstanceLabelOverride_OverrideNavigationProperty)
    {
    m_descriptorBuilder->GetContext().SetPreferredDisplayType(ContentDisplayType::Grid);

    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();

    // set up selection
    TestParsedInput info({
        bpair<ECClassCP, ECInstanceId>(classA, {ECInstanceId((uint64_t)1)}),
        bpair<ECClassCP, ECInstanceId>(classB, {ECInstanceId((uint64_t)2)}),
        });

    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "Prop"));

    ContentDescriptorPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());
    descriptor->AddContentFlag(ContentFlags::ShowLabels);

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        auto& labelOverride = RegisterForDelete(*new InstanceLabelOverride(1, false, classA->GetFullName(), bvector<InstanceLabelOverrideValueSpecification*>{
            new InstanceLabelOverridePropertyValueSpecification("Prop")
            }));

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor(ContentDisplayType::Grid);
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false), "");
        descriptor->AddSelectClass(SelectClassInfo(*classB, "this", false)
            .SetNavigationPropertyClasses(
                {
                RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_NAV_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, RULES_ENGINE_NAV_CLASS_ALIAS(*classA, 0), true))
                }), "");

        ContentDescriptor::DisplayLabelField* displayLabelField = new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0);
        displayLabelField->SetLabelOverrideSpecs(CreateLabelOverrideSpecificationsMap(*classA, labelOverride));
        AddField(*descriptor, *displayLabelField);

        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("Prop")));
        descriptor->GetAllFields().back()->AsPropertiesField()->AddProperty(CreateProperty("this", *classB, *classB->GetPropertyP("Prop")));
        descriptor->GetAllFields().back()->SetUniqueName(FIELD_NAME((bvector<ECClassCP>{classA, classB}), "Prop"));

        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty(RULES_ENGINE_NAV_CLASS_ALIAS(*classA, 0), *classB, *classB->GetPropertyP("NavA")));

        descriptor->AddContentFlag(ContentFlags::ShowLabels);

        SelectClass<ECClass> selectClass1(*classA, "this", false);
        ComplexContentQueryPtr q1 = ComplexContentQuery::Create();
        q1->SelectContract(*CreateQueryContract(1, *descriptor, classA, *q1, CreateDisplayLabelField(selectClass1, {}, labelOverride.GetValueSpecifications())), "this");
        q1->From(*classA, false, "this");
        q1->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)1))});

        SelectClass<ECClass> selectClass2(*classB, "this", false);
        ComplexContentQueryPtr q2 = ComplexContentQuery::Create();
        q2->SelectContract(*CreateQueryContract(2, *descriptor, classB, *q2, CreateDisplayLabelField(selectClass2)), "this");
        q2->From(*classB, false, "this");
        q2->Join(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_NAV_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, RULES_ENGINE_NAV_CLASS_ALIAS(*classA, 0), true)));
        q2->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)2))});

        return UnionContentQuery::Create({ q1, q2 });
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_JoinsRelatedInstanceWithInnerJoin, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="string" />
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
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_JoinsRelatedInstanceWithInnerJoin)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();
    TestParsedInput info(*classA, ECInstanceId((uint64_t) 1));

    SelectedNodeInstancesSpecification spec(1, false, "", "", false);
    spec.AddRelatedInstance(*new RelatedInstanceSpecification(RelationshipPathSpecification(*new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)), "bClass", true));

    ContentDescriptorPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        RelatedClass aTob(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, "bClass", true), false);

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false)
            .SetRelatedInstancePaths({ {aTob} }), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("PropA")));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query, nullptr, { {aTob} }), "this");
        query->From(*classA, false, "this");
        query->Join(aTob);
        query->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)1))});
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SelectedNodeInstances_DoesNotJoinTargetClassWhenOnlyRelationshipPropertiesSelected, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_B" modifier="None" strength="embedding">
        <ECProperty propertyName="PropAB" typeName="int" />
        <Source multiplicity="(1..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (ContentQueryBuilderTests, SelectedNodeInstances_DoesNotJoinTargetClassWhenOnlyRelationshipPropertiesSelected)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(GetDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(GetDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(GetDb(), *relAB, *a, *b);

    SelectedNodeInstancesSpecification spec(1, false, "", "A", false);
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward)
        }), {new PropertySpecification("*")}, RelationshipMeaning::RelatedInstance, false, false, false, {new PropertySpecification("*")}));

    TestParsedInput info(*classA, ECInstanceId(ECInstanceId::FromString(a->GetInstanceId().c_str())));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        auto categoryAB = CreateCategory(*relAB);

        RelatedClassPath relatedPropertyPathAB{RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true))};

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false)
            .SetRelatedPropertyPaths({relatedPropertyPathAB}), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, *new ContentDescriptor::RelatedContentField(categoryAB, NESTED_CONTENT_FIELD_NAME(classA, relAB),
            relAB->GetDisplayLabel(), relatedPropertyPathAB,
            {
            CreatePropertiesField(categoryAB, CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0), *relAB, *relAB->GetPropertyP("PropAB"))),
            }, false, ContentDescriptor::Property::DEFAULT_PRIORITY, true));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query), "this");
        query->From(*classA, false, "this");
        query->Join(relatedPropertyPathAB, false);
        query->Where("[this].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId::FromString(a->GetInstanceId().c_str()))});
        return query;
        });
    }
