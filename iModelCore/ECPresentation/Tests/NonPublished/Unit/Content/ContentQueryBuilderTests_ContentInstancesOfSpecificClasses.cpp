/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ContentQueryBuilderTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnSingleClass, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnSingleClass)
    {
    ECClassCP classA = GetECClass("A");
    ContentInstancesOfSpecificClassesSpecification spec(1, false, "", {CreateMultiSchemaClass({classA}, false)}, {}, false);

    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("Prop")));

        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query), "this");
        query->From(*classA, false, "this");
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnSingleClassPolymorphically, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnSingleClassPolymorphically)
    {
    ECClassCP classA = GetECClass("A");
    ContentInstancesOfSpecificClassesSpecification spec(1, false, "", {CreateMultiSchemaClass({classA}, true)}, {}, false);

    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", true), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("Prop")));

        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query), "this");
        query->From(*classA, true, "this");
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstacesOfSpecificClasses_ReturnsQueryWithNotIncludedHiddenProperties, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
        <ECProperty propertyName="HiddenProp" typeName="string">
            <ECCustomAttributes>
                <HiddenProperty xmlns="CoreCustomAttributes.01.00"/>
            </ECCustomAttributes>
        </ECProperty>
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, ContentInstacesOfSpecificClasses_ReturnsQueryWithNotIncludedHiddenProperties)
    {
    ECClassCP classA = GetECClass("A");
    ContentInstancesOfSpecificClassesSpecification spec(1, false, "", {CreateMultiSchemaClass({classA}, false)}, {}, false);

    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("Prop")));

        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query), "this");
        query->From(*classA, false, "this");
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstacesOfSpecificClasses_ReturnsQueryWithCalculatedProperties, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(ContentQueryBuilderTests, ContentInstacesOfSpecificClasses_ReturnsQueryWithCalculatedProperties)
    {
    ECClassCP classA = GetECClass("A");
    ContentInstancesOfSpecificClassesSpecification spec(1, false, "", {CreateMultiSchemaClass({classA}, false)}, {}, false);
    spec.AddCalculatedProperty(*new CalculatedPropertiesSpecification("Label_1", 1200, "\"Value\" & 1"));
    spec.AddCalculatedProperty(*new CalculatedPropertiesSpecification("Label_2", 1500, "this.Prop & \"Test\""));

    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("Prop")));
        AddField(*descriptor, *new ContentDescriptor::CalculatedPropertyField(DEFAULT_CONTENT_FIELD_CATEGORY, "Label_1", "CalculatedProperty_0", "\"Value\" & 1", nullptr, 1200));
        AddField(*descriptor, *new ContentDescriptor::CalculatedPropertyField(DEFAULT_CONTENT_FIELD_CATEGORY, "Label_2", "CalculatedProperty_1", "this.Prop & \"Test\"", nullptr, 1500));

        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query), "this");
        query->From(*classA, false, "this");
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstacesOfSpecificClasses_ReturnsQueryWithPropertyPriority, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" priority="1200" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, ContentInstacesOfSpecificClasses_ReturnsQueryWithPropertyPriority)
    {
    ECClassCP classA = GetECClass("A");
    ContentInstancesOfSpecificClassesSpecification spec(1, false, "", {CreateMultiSchemaClass({classA}, false)}, {}, false);

    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor);

    int priority = querySet.GetContract()->AsContentQueryContract()->GetDescriptor().GetVisibleFields()[0]->GetPriority();
    EXPECT_EQ(1200, priority);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstacesOfSpecificClasses_ReturnsQueryWithDefaultPropertyPriority, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(ContentQueryBuilderTests, ContentInstacesOfSpecificClasses_ReturnsQueryWithDefaultPropertyPriority)
    {
    ECClassCP classA = GetECClass("A");
    ContentInstancesOfSpecificClassesSpecification spec(1, false, "", {CreateMultiSchemaClass({classA}, false)}, {}, false);

    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor);

    int priority = querySet.GetContract()->AsContentQueryContract()->GetDescriptor().GetVisibleFields()[0]->GetPriority();
    EXPECT_EQ(ContentDescriptor::Property::DEFAULT_PRIORITY, priority);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnMultipleClasses, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnMultipleClasses)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ContentInstancesOfSpecificClassesSpecification spec(1, false, "", {CreateMultiSchemaClass({classA, classB}, false)}, {}, false);

    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false), "");
        descriptor->AddSelectClass(SelectClassInfo(*classB, "this", false), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("PropA")));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classB, *classB->GetPropertyP("PropB")));

        ComplexQueryBuilderPtr query1 = ComplexQueryBuilder::Create();
        query1->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query1), "this");
        query1->From(*classA, false, "this");

        ComplexQueryBuilderPtr query2 = ComplexQueryBuilder::Create();
        query2->SelectContract(*CreateQueryContract(2, *descriptor, classB, *query2), "this");
        query2->From(*classB, false, "this");

        UnionQueryBuilderPtr query = UnionQueryBuilder::Create({ query1, query2 });
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnMultipleSchemaClasses1, R"*(
    <ECEntityClass typeName="A1">
        <ECProperty propertyName="PropA1" typeName="int" />
    </ECEntityClass>
)*");
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnMultipleSchemaClasses2, R"*(
    <ECEntityClass typeName="A2">
        <ECProperty propertyName="PropA2" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, ContentInstancesOfSpecificClasses_ReturnsQueryBasedOnMultipleSchemaClasses)
    {
    ECClassCP classA1 = GetECClass(Utf8String(BeTest::GetNameOfCurrentTest()).append("1").c_str(), "A1");
    ECClassCP classA2 = GetECClass(Utf8String(BeTest::GetNameOfCurrentTest()).append("2").c_str(), "A2");
    ContentInstancesOfSpecificClassesSpecification spec(1, false, "",
        {
        CreateMultiSchemaClass({classA1}, false, classA1->GetSchema().GetName().c_str()),
        CreateMultiSchemaClass({classA2}, false, classA2->GetSchema().GetName().c_str()),
        }, {}, false);

    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA1, "this", false), "");
        descriptor->AddSelectClass(SelectClassInfo(*classA2, "this", false), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA1, *classA1->GetPropertyP("PropA1")));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA2, *classA2->GetPropertyP("PropA2")));

        ComplexQueryBuilderPtr query1 = ComplexQueryBuilder::Create();
        query1->SelectContract(*CreateQueryContract(1, *descriptor, classA1, *query1), "this");
        query1->From(*classA1, false, "this");

        ComplexQueryBuilderPtr query2 = ComplexQueryBuilder::Create();
        query2->SelectContract(*CreateQueryContract(2, *descriptor, classA2, *query2), "this");
        query2->From(*classA2, false, "this");

        UnionQueryBuilderPtr query = UnionQueryBuilder::Create({ query1, query2 });
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_AppliesInstanceFilter, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, ContentInstancesOfSpecificClasses_AppliesInstanceFilter)
    {
    ECClassCP classA = GetECClass("A");
    ContentInstancesOfSpecificClassesSpecification spec(1, false, "this.Prop = 10", {CreateMultiSchemaClass({classA}, false)}, {}, false);

    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("Prop")));

        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query), "this");
        query->From(*classA, false, "this");
        query->Where("[this].[Prop] = 10", BoundQueryValuesList());
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_AppliesInstanceFilterUsingRelatedInstanceSpecification, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="int" />
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
TEST_F (ContentQueryBuilderTests, ContentInstancesOfSpecificClasses_AppliesInstanceFilterUsingRelatedInstanceSpecification)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();

    ContentInstancesOfSpecificClassesSpecification spec(1, false, "classB.PropB = \"testValue\"", {CreateMultiSchemaClass({classA}, false)}, {}, false);
    spec.AddRelatedInstance(*new RelatedInstanceSpecification(
        RelationshipPathSpecification(*new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)), "classB"));

    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor);
    ValidateQueries(querySet, [&]()
        {
        RelatedClass aTob(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, "classB", true));

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false)
            .SetRelatedInstancePaths({ {aTob} }), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("PropA")));

        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query, nullptr, { {aTob} }), "this");
        query->From(*classA, false, "this");
        query->Join(aTob);
        query->Where("[classB].[PropB] = 'testValue'", BoundQueryValuesList());
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_CategorizesFields, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
        <ECProperty propertyName="CategorizedProp" typeName="int" category="CategoryName" />
    </ECEntityClass>
    <PropertyCategory typeName="CategoryName" displayLabel="Category Label" description="Category description" priority="1" />
)*");
TEST_F (ContentQueryBuilderTests, ContentInstancesOfSpecificClasses_CategorizesFields)
    {
    ECClassCP classA = GetECClass("A");
    ContentInstancesOfSpecificClassesSpecification spec(1, false, "", {CreateMultiSchemaClass({classA}, false)}, {}, false);

    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor);

    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(2, fields.size());

    EXPECT_STREQ(FIELD_NAME(classA, "Prop"), fields[0]->GetUniqueName().c_str());
    EXPECT_STREQ(DEFAULT_CONTENT_FIELD_CATEGORY->GetName().c_str(), fields[0]->GetCategory()->GetName().c_str());

    EXPECT_STREQ(FIELD_NAME(classA, "CategorizedProp"), fields[1]->GetUniqueName().c_str());
    EXPECT_STREQ("CategoryName", fields[1]->GetCategory()->GetName().c_str());
    EXPECT_STREQ("Category Label", fields[1]->GetCategory()->GetLabel().c_str());
    EXPECT_STREQ("Category description", fields[1]->GetCategory()->GetDescription().c_str());

    EXPECT_EQ(fields[0]->GetCategory().get(), fields[1]->GetCategory()->GetParentCategory());
    }

/*---------------------------------------------------------------------------------**//**
* Reproduces the case when the spec results in 1 query and we can set the flag immediately
  on that querySet.
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_SetsMergeResultsFlagForPropertyPaneContentType, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, ContentInstancesOfSpecificClasses_SetsMergeResultsFlagForPropertyPaneContentType)
    {
    ECClassCP classA = GetECClass("A");
    ContentInstancesOfSpecificClassesSpecification spec(1, false, "", {CreateMultiSchemaClass({classA}, false)}, {}, false);

    m_descriptorBuilder->GetContext().SetPreferredDisplayType(ContentDisplayType::PropertyPane);

    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor(ContentDisplayType::PropertyPane);
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false), "");

        descriptor->AddContentFlag(ContentFlags::MergeResults);

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("Prop")));

        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query), "this");
        query->From(*classA, false, "this");
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_MergesSimilarPropertiesIntoOneField, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, ContentInstancesOfSpecificClasses_MergesSimilarPropertiesIntoOneField)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ContentInstancesOfSpecificClassesSpecification spec(1, false, "", {CreateMultiSchemaClass({classA, classB}, false)}, {}, false);

    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor);

    bvector<ContentDescriptor::Field*> fields = descriptor->GetVisibleFields();
    ASSERT_EQ(1, fields.size());

    EXPECT_STREQ(FIELD_NAME((bvector<ECClassCP>{classA, classB}), "Prop"), fields[0]->GetUniqueName().c_str());
    EXPECT_EQ(2, fields[0]->AsPropertiesField()->GetProperties().size());
    }

#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_SelectPointPropertyRawDataGroupedByDisplayValue, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PointProp" typeName="point3d" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, ContentInstancesOfSpecificClasses_SelectPointPropertyRawDataGroupedByDisplayValue)
    {
    ECClassCP classA = GetECClass("A");
    ContentInstancesOfSpecificClassesSpecification spec(1, false, "", {CreateMultiSchemaClass({classA}, false)}, {}, false);

    ContentDescriptorPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    descriptor->AddContentFlag(ContentFlags::DistinctValues);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddContentFlag(ContentFlags::DistinctValues);
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("PointProp")));

        ComplexQueryBuilderPtr nestedQuery = ComplexQueryBuilder::Create();
        ContentQueryContractPtr contract = CreateQueryContract(1, *descriptor, classA, *nestedQuery);
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(*classA, false, "this");

        ComplexQueryBuilderPtr groupedQuery = ComplexQueryBuilder::Create();
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
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_InstanceLabelOverride_AppliedByPriority, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop1" typeName="string" />
        <ECProperty propertyName="Prop2" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, ContentInstancesOfSpecificClasses_InstanceLabelOverride_AppliedByPriority)
    {
    ECClassCP classA = GetECClass("A");
    ContentInstancesOfSpecificClassesSpecification spec(1, false, "", {CreateMultiSchemaClass({classA}, false)}, {}, false);
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, false, classA->GetFullName(), "Prop1"));
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(2, false, classA->GetFullName(), "Prop2"));

    ContentDescriptorPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());
    descriptor->AddContentFlag(ContentFlags::ShowLabels);

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor);
    ValidateQueries(querySet, [&]()
        {
        auto& labelOverride = RegisterForDelete(*new InstanceLabelOverride(1, false, classA->GetFullName(), bvector<InstanceLabelOverrideValueSpecification*>{
            new InstanceLabelOverridePropertyValueSpecification("Prop2"),
            new InstanceLabelOverridePropertyValueSpecification("Prop1")
            }));

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false), "");

        ContentDescriptor::DisplayLabelField* displayLabelField = new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0);
        displayLabelField->SetLabelOverrideSpecs(CreateLabelOverrideSpecificationsMap(*classA, labelOverride));
        AddField(*descriptor, *displayLabelField);
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("Prop1")));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("Prop2")));
        descriptor->AddContentFlag(ContentFlags::ShowLabels);

        SelectClass<ECClass> selectClass(*classA, "this", false);
        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query, CreateDisplayLabelField(selectClass, {}, labelOverride.GetValueSpecifications())), "this");
        query->From(selectClass);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="string" />
    </ECEntityClass>
)*");
TEST_F (ContentQueryBuilderTests, ContentInstancesOfSpecificClasses_InstanceLabelOverride_OverrideOnlySpecifiedClassInstancesLabels)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ContentInstancesOfSpecificClassesSpecification spec(1, false, "", {CreateMultiSchemaClass({classA, classB}, false)}, {}, false);
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "PropA"));

    ContentDescriptorPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());
    descriptor->AddContentFlag(ContentFlags::ShowLabels);

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor);
    ValidateQueries(querySet, [&]()
        {
        auto& labelOverride = RegisterForDelete(*new InstanceLabelOverride(1, false, classA->GetFullName(), bvector<InstanceLabelOverrideValueSpecification*>{
            new InstanceLabelOverridePropertyValueSpecification("PropA")
            }));

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false), "");
        descriptor->AddSelectClass(SelectClassInfo(*classB, "this", false), "");

        ContentDescriptor::DisplayLabelField* displayLabelField = new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0);
        displayLabelField->SetLabelOverrideSpecs(CreateLabelOverrideSpecificationsMap(*classA, labelOverride));
        AddField(*descriptor, *displayLabelField);
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("PropA")));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classB, *classB->GetPropertyP("PropB")));
        descriptor->AddContentFlag(ContentFlags::ShowLabels);

        SelectClass<ECClass> selectClass1(*classA, "this", false);
        ComplexQueryBuilderPtr q1 = ComplexQueryBuilder::Create();
        q1->SelectContract(*CreateQueryContract(1, *descriptor, classA, *q1, CreateDisplayLabelField(selectClass1, {}, labelOverride.GetValueSpecifications())), "this");
        q1->From(selectClass1);

        SelectClass<ECClass> selectClass2(*classB, "this", false);
        ComplexQueryBuilderPtr q2 = ComplexQueryBuilder::Create();
        q2->SelectContract(*CreateQueryContract(2, *descriptor, classB, *q2, CreateDisplayLabelField(selectClass2)), "this");
        q2->From(selectClass2);

        UnionQueryBuilderPtr query = UnionQueryBuilder::Create({q1, q2});
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_InstanceLabelOverride_OverrideNavigationProperty, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="string" />
        <ECNavigationProperty propertyName="NavB" relationshipName="B_To_A" direction="Backward" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="B_To_A" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="A"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (ContentQueryBuilderTests, ContentInstancesOfSpecificClasses_InstanceLabelOverride_OverrideNavigationProperty)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relBA = GetECClass("B_To_A")->GetRelationshipClassCP();
    ContentInstancesOfSpecificClassesSpecification spec(1, false, "", {CreateMultiSchemaClass({classA}, false)}, {}, false);
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, classB->GetFullName(), "PropB"));

    ContentDescriptorPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());
    descriptor->AddContentFlag(ContentFlags::ShowLabels);

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor);
    ValidateQueries(querySet, [&]()
        {
        RelatedClass aTob(*classA, SelectClass<ECRelationshipClass>(*relBA, RULES_ENGINE_NAV_CLASS_ALIAS(*relBA, 0)), false, SelectClass<ECClass>(*classB, RULES_ENGINE_NAV_CLASS_ALIAS(*classB, 0), true));

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false)
            .SetNavigationPropertyClasses({aTob}), "");

        ContentDescriptor::DisplayLabelField* displayLabelField = new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0);
        AddField(*descriptor, *displayLabelField);
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("PropA")));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty(RULES_ENGINE_NAV_CLASS_ALIAS(*classB, 0), *classA, *classA->GetPropertyP("NavB")));
        descriptor->AddContentFlag(ContentFlags::ShowLabels);

        SelectClass<ECClass> selectClass(*classA, "this", false);
        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query, CreateDisplayLabelField(selectClass)), "this");
        query->From(selectClass);
        query->Join(aTob);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentInstancesOfSpecificClasses_DoesNotJoinClassesForExcludedDescriptorFields, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="int" />
        <ECNavigationProperty propertyName="NavD" relationshipName="D_To_A" direction="Backward" />
        <ECNavigationProperty propertyName="NavE" relationshipName="E_To_A" direction="Backward" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="PropC" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="D" />
    <ECEntityClass typeName="E" />
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="A_To_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="D_To_A" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="D"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="A"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="E_To_A" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="E"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="A"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ContentQueryBuilderTests, ContentInstancesOfSpecificClasses_DoesNotJoinClassesForExcludedDescriptorFields)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECClassCP classD = GetECClass("D");
    ECClassCP classE = GetECClass("E");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relAC = GetECClass("A_To_C")->GetRelationshipClassCP();
    ECRelationshipClassCP relDA = GetECClass("D_To_A")->GetRelationshipClassCP();
    ECRelationshipClassCP relEA = GetECClass("E_To_A")->GetRelationshipClassCP();

    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(GetDb(), *classA);
    IECInstancePtr instanceB = RulesEngineTestHelpers::InsertInstance(GetDb(), *classB);
    IECInstancePtr instanceC = RulesEngineTestHelpers::InsertInstance(GetDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(GetDb(), *relAB, *instanceA, *instanceB);
    RulesEngineTestHelpers::InsertRelationship(GetDb(), *relAC, *instanceA, *instanceC);

    ContentInstancesOfSpecificClassesSpecification spec(1, false, "", {CreateMultiSchemaClass({classA}, false)}, {}, false);
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification({new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward)}),
        {new PropertySpecification("PropB")}, RelationshipMeaning::RelatedInstance));
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification({new RelationshipStepSpecification(relAC->GetFullName(), RequiredRelationDirection_Forward)}),
        {new PropertySpecification("PropC")}, RelationshipMeaning::RelatedInstance));

    ContentDescriptorPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());

    descriptor->ExcludeFields({
        descriptor->FindField(PropertiesContentFieldMatcher(*classB->GetPropertyP("PropB"), {RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, ""), true, SelectClass<ECClass>(*classB, ""))})),
        descriptor->FindField(PropertiesContentFieldMatcher(*classA->GetPropertyP("NavD"), {})),
        });
    ASSERT_EQ(3, descriptor->GetVisibleFields().size()); // PropA, C and NavE

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor);
    ValidateQueries(querySet, [&]()
        {
        RelatedClassPath relatedPropertyPathAB{RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true))};
        RelatedClassPath relatedPropertyPathAC{RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAC, 0)), true, SelectClass<ECClass>(*classC, RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0), true))};
        RelatedClass navigationPropertyPathAD(*classA, SelectClass<ECRelationshipClass>(*relDA, RULES_ENGINE_NAV_CLASS_ALIAS(*relDA, 0)), false, SelectClass<ECClass>(*classD, RULES_ENGINE_NAV_CLASS_ALIAS(*classD, 0), true));
        RelatedClass navigationPropertyPathAE(*classA, SelectClass<ECRelationshipClass>(*relEA, RULES_ENGINE_NAV_CLASS_ALIAS(*relEA, 0)), false, SelectClass<ECClass>(*classE, RULES_ENGINE_NAV_CLASS_ALIAS(*classE, 0), true));

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", false)
            .SetRelatedPropertyPaths(
                {
                relatedPropertyPathAB,
                relatedPropertyPathAC,
                })
                .SetNavigationPropertyClasses(
                    {
                    navigationPropertyPathAD,
                    navigationPropertyPathAE,
                    }), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("PropA")));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty(RULES_ENGINE_NAV_CLASS_ALIAS(*classE, 0), *classA, *classA->GetPropertyP("NavE")));
        descriptor->GetCategories().push_back(CreateCategory(*classB)); // not needed, but included by the library (order is also important)
        AddField(*descriptor, *CreateRelatedField(CreateCategory(*classC), NESTED_CONTENT_FIELD_NAME(classA, classC), *classC, relatedPropertyPathAC,
            {
            CreatePropertiesField(CreateCategory(*classC), CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0), *classC, *classC->GetPropertyP("PropC"))),
            }));

        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query), "this");
        query->From(*classA, false, "this");
        query->Join(navigationPropertyPathAE);
        query->Join(relatedPropertyPathAC);
        return query;
        });
    }
