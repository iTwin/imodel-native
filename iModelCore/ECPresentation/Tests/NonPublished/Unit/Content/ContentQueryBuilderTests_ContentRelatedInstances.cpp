/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ContentQueryBuilderTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsNullDescriptorWhenNoSelectedNodes, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_ReturnsNullDescriptorWhenNoSelectedNodes)
    {
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();
    ContentRelatedInstancesSpecification spec(1, false, "",
        {new RepeatableRelationshipPathSpecification(*new RepeatableRelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward))});
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, TestParsedInput());
    ASSERT_TRUE(descriptor.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsForwardRelatedInstanceQueryWhenSelectedOneInstanceNode, R"*(
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
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_ReturnsForwardRelatedInstanceQueryWhenSelectedOneInstanceNode)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relBC = GetECClass("B_To_C")->GetRelationshipClassCP();

    Utf8PrintfString classNames("%s:%s,%s", classA->GetSchema().GetName().c_str(), classA->GetName().c_str(), classC->GetName().c_str());
    ContentRelatedInstancesSpecification spec(1, 0, false, "", RequiredRelationDirection_Forward, "", classNames);

    TestParsedInput info(*classB, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        RelatedClass bToc(*classB, SelectClass<ECRelationshipClass>(*relBC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relBC, 0)), true, SelectClass<ECClass>(*classC, RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0), true), false);

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classC, "this", true)
            .SetPathFromInputToSelectClass({bToc}), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classC, *classC->GetPropertyP("PropC")));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classC, *query, nullptr, {}, "related"), "this");
        query->From(*classC, true, "this");
        query->Join(ReverseRelationshipPath({bToc}, "related", false));
        query->Where("[related].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123))});

#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsBackwardRelatedInstanceQueryWhenSelectedOneInstanceNode, R"*(
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
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_ReturnsBackwardRelatedInstanceQueryWhenSelectedOneInstanceNode)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();

    Utf8PrintfString classNames("%s:%s,%s", classA->GetSchema().GetName().c_str(), classA->GetName().c_str(), classC->GetName().c_str());
    ContentRelatedInstancesSpecification spec(1, 0, false, "", RequiredRelationDirection_Backward, "", classNames);

    TestParsedInput info(*classB, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        RelatedClass bToa(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, RULES_ENGINE_RELATED_CLASS_ALIAS(*classA, 0), true), false);

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", true)
            .SetPathFromInputToSelectClass({bToa}), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("PropA")));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query, nullptr, {}, "related"), "this");
        query->From(*classA, true, "this");
        query->Join(ReverseRelationshipPath({bToa},"related", false));
        query->Where("[related].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123))});

#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsBothDirectionsRelatedInstanceQueryWhenSelectedOneInstanceNode, R"*(
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
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_ReturnsBothDirectionsRelatedInstanceQueryWhenSelectedOneInstanceNode)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetECClass("B_To_C")->GetRelationshipClassCP();

    Utf8PrintfString classNames("%s:%s,%s", classA->GetSchema().GetName().c_str(), classA->GetName().c_str(), classC->GetName().c_str());
    ContentRelatedInstancesSpecification spec(1, 0, false, "", RequiredRelationDirection_Both, "", classNames);

    TestParsedInput info(*classB, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        RelatedClass bToa(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, RULES_ENGINE_RELATED_CLASS_ALIAS(*classA, 0), true), false);
        RelatedClass bToc(*classB, SelectClass<ECRelationshipClass>(*relBC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relBC, 0)), true, SelectClass<ECClass>(*classC, RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0), true), false);

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", true)
            .SetPathFromInputToSelectClass({bToa}), "");
        descriptor->AddSelectClass(SelectClassInfo(*classC, "this", true)
            .SetPathFromInputToSelectClass({bToc}), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("PropA")));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classC, *classC->GetPropertyP("PropC")));

        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query1, nullptr, {}, "related"), "this");
        query1->From(*classA, true, "this");
        query1->Join(ReverseRelationshipPath({bToa}, "related", false));
        query1->Where("[related].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123))});

        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*CreateQueryContract(2, *descriptor, classC, *query2, nullptr, {}, "related"), "this");
        query2->From(*classC, true, "this");
        query2->Join(ReverseRelationshipPath({bToc}, "related", false));
        query2->Where("[related].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123))});

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
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsRelatedInstanceQueryWhenSelectedMultipleInstanceNodesOfTheSameClass, R"*(
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
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_ReturnsRelatedInstanceQueryWhenSelectedMultipleInstanceNodesOfTheSameClass)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();
    ContentRelatedInstancesSpecification spec(1, 0, false, "", RequiredRelationDirection_Forward, "", classB->GetFullName());

    TestParsedInput info(*classA, {ECInstanceId((uint64_t)123), ECInstanceId((uint64_t)125)});
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        RelatedClass aTob(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true), false);

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classB, "this", true)
            .SetPathFromInputToSelectClass({aTob}), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classB, *classB->GetPropertyP("PropB")));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classB, *query, nullptr, {}, "related"), "this");
        query->From(*classB, true, "this");
        query->Join(ReverseRelationshipPath({aTob}, "related", false));
        query->Where("[related].[ECInstanceId] IN (?,?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)), std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)125))});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_ReturnsRelatedInstanceQueryWhenSelectedMultipleInstanceNodesOfDifferentClasses, R"*(
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
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_ReturnsRelatedInstanceQueryWhenSelectedMultipleInstanceNodesOfDifferentClasses)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetECClass("B_To_C")->GetRelationshipClassCP();

    Utf8PrintfString relationshipClassNames("%s:%s,%s", classA->GetSchema().GetName().c_str(), relAB->GetName().c_str(), relBC->GetName().c_str());
    ContentRelatedInstancesSpecification spec(1, 0, false, "", RequiredRelationDirection_Both, relationshipClassNames, classB->GetFullName());

    TestParsedInput info({
        bpair<ECClassCP, ECInstanceId>(classA, {ECInstanceId((uint64_t)123)}),
        bpair<ECClassCP, ECInstanceId>(classC, {ECInstanceId((uint64_t)125)}),
        });
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        RelatedClass aTob(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true), false);
        RelatedClass cTob(*classC, SelectClass<ECRelationshipClass>(*relBC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relBC, 0)), false, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 1), true), false);

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classB, "this", true)
            .SetPathFromInputToSelectClass({aTob}), "");
        descriptor->AddSelectClass(SelectClassInfo(*classB, "this", true)
            .SetPathFromInputToSelectClass({cTob}), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classB, *classB->GetPropertyP("PropB")));

        ComplexContentQueryPtr query1 = ComplexContentQuery::Create();
        query1->SelectContract(*CreateQueryContract(1, *descriptor, classB, *query1, nullptr, {}, "related"), "this");
        query1->From(*classB, true, "this");
        query1->Join(ReverseRelationshipPath({aTob}, "related", false));
        query1->Where("[related].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123))});

        ComplexContentQueryPtr query2 = ComplexContentQuery::Create();
        query2->SelectContract(*CreateQueryContract(2, *descriptor, classB, *query2, nullptr, {}, "related"), "this");
        query2->From(*classB, true, "this");
        query2->Join(ReverseRelationshipPath({cTob}, "related", false));
        query2->Where("[related].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)125))});

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
DEFINE_SCHEMA(ContentRelatedInstances_AppliesInstanceFilter, R"*(
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
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_AppliesInstanceFilter)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();
    ContentRelatedInstancesSpecification spec(1, 0, false, "this.PropB = \"TestValue\"", RequiredRelationDirection_Forward, "", classB->GetFullName());

    TestParsedInput info(*classA, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        RelatedClass aTob(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true), false);

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classB, "this", true)
            .SetPathFromInputToSelectClass({aTob}), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classB, *classB->GetPropertyP("PropB")));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classB, *query, nullptr, {}, "related"), "this");
        query->From(*classB, true, "this");
        query->Join(ReverseRelationshipPath({aTob}, "related", false));
        query->Where("[related].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123))});
        query->Where("[this].[PropB] = 'TestValue'", BoundQueryValuesList());
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_AppliesInstanceFilterUsingRelatedInstanceSpecification, R"*(
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
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_AppliesInstanceFilterUsingRelatedInstanceSpecification)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetECClass("B_To_C")->GetRelationshipClassCP();

    ContentRelatedInstancesSpecification spec(1, 0, false, "relatedC.PropC = \"TestValue\"", RequiredRelationDirection_Forward, relAB->GetFullName(), classB->GetFullName());
    spec.AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Forward, relBC->GetFullName(), classC->GetFullName(), "relatedC"));

    TestParsedInput info(*classA, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        RelatedClass aTob(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true), false);
        RelatedClass bToc(*classB, SelectClass<ECRelationshipClass>(*relBC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relBC, 0)), true, SelectClass<ECClass>(*classC, "relatedC", true));

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classB, "this", true)
            .SetPathFromInputToSelectClass({aTob})
            .SetRelatedInstancePaths({ {bToc} }), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, ContentDescriptor::Property("this", *classB, *classB->GetPropertyP("PropB")));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        auto contract = CreateQueryContract(1, *descriptor, classB, *query, nullptr, { {bToc} });
        contract->SetInputClassAlias("related");
        query->SelectContract(*contract, "this");
        query->From(*classB, true, "this");
        query->Join(ReverseRelationshipPath({aTob}, "related", false));
        query->Join(bToc);
        query->Where("[related].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123))});
        query->Where("[relatedC].[PropC] = 'TestValue'", BoundQueryValuesList());
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_SkipsRelatedLevel, R"*(
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
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_SkipsRelatedLevel)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetECClass("B_To_C")->GetRelationshipClassCP();
    ContentRelatedInstancesSpecification spec(1, 1, false, "", RequiredRelationDirection_Forward, "", classC->GetFullName());

    TestParsedInput info(*classA, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        RelatedClassPath aTocPath = {
            RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true), false),
            RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relBC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relBC, 0)), true, SelectClass<ECClass>(*classC, RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0), true), false),
            };

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classC, "this", true)
            .SetPathFromInputToSelectClass(aTocPath), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classC, *classC->GetPropertyP("PropC")));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classC, *query, nullptr, {}, "related"), "this");
        query->From(*classC, true, "this");
        query->Join(ReverseRelationshipPath(aTocPath, "related", false));
        query->Where("[related].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123))});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(ContentQueryContract::ECInstanceIdFieldName);
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_SkipsRelatedLevelWithSpecifiedRelationship, R"*(
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
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_SkipsRelatedLevelWithSpecifiedRelationship)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetECClass("B_To_C")->GetRelationshipClassCP();
    ContentRelatedInstancesSpecification spec(1, 1, false, "", RequiredRelationDirection_Backward, relAB->GetFullName(), classA->GetFullName());

    TestParsedInput info(*classC, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        RelatedClassPath cToaPath = {
            RelatedClass(*classC, SelectClass<ECRelationshipClass>(*relBC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relBC, 0)), false, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true), false),
            RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, RULES_ENGINE_RELATED_CLASS_ALIAS(*classA, 0), true), false)
            };

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classA, "this", true)
            .SetPathFromInputToSelectClass(cToaPath), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classA, *classA->GetPropertyP("PropA")));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classA, *query, nullptr, {}, "related"), "this");
        query->From(*classA, true, "this");
        query->Join(ReverseRelationshipPath(cToaPath, "related", false));
        query->Where("[related].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123))});
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_CreatesRecursiveQuery, R"*(
    <ECEntityClass typeName="Element">
        <ECProperty propertyName="ElementProperty" typeName="int" />
        <ECNavigationProperty propertyName="Parent" relationshipName="ElementOwnsChildElements" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsChildElements" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="Element"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_CreatesRecursiveQuery)
    {
    ECRelationshipClassCP rel = GetECClass("ElementOwnsChildElements")->GetRelationshipClassCP();
    ECClassCP ecClass = GetECClass("Element");

    ContentRelatedInstancesSpecification spec(1, 0, true, "", RequiredRelationDirection_Forward, rel->GetFullName(), ecClass->GetFullName());

    TestParsedInput info(*ecClass, {ECInstanceId((uint64_t)123), ECInstanceId((uint64_t)456)});
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*ecClass, "this", true)
            .SetPathFromInputToSelectClass(
                {
                RelatedClass(*ecClass, SelectClass<ECRelationshipClass>(*rel, RULES_ENGINE_RELATED_CLASS_ALIAS(*rel, 0)), true, SelectClass<ECClass>(*ecClass, RULES_ENGINE_RELATED_CLASS_ALIAS(*ecClass, 0), true), false)
                })
            .SetNavigationPropertyClasses(
                {
                RelatedClass(*ecClass, SelectClass<ECRelationshipClass>(*rel, RULES_ENGINE_NAV_CLASS_ALIAS(*rel, 0)), false, SelectClass<ECClass>(*ecClass, RULES_ENGINE_NAV_CLASS_ALIAS(*ecClass, 0), true))
                }), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, ContentDescriptor::Property("this", *ecClass, *ecClass->GetPropertyP("ElementProperty")));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, ContentDescriptor::Property(RULES_ENGINE_NAV_CLASS_ALIAS(*ecClass, 0), *ecClass, *ecClass->GetPropertyP("Parent")));

        bvector<ECInstanceId> selectedIds;
        selectedIds.push_back(ECInstanceId((uint64_t)123));
        selectedIds.push_back(ECInstanceId((uint64_t)456));

        bset<ECRelationshipClassCP> relationships;
        relationships.insert(rel);

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, ecClass, *query), "this");
        query->From(*ecClass, true, "this");
        query->Join(RelatedClass(*ecClass, SelectClass<ECRelationshipClass>(*rel, RULES_ENGINE_NAV_CLASS_ALIAS(*rel, 0)), false, SelectClass<ECClass>(*ecClass, RULES_ENGINE_NAV_CLASS_ALIAS(*ecClass, 0), true)));
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_CreatesRecursiveQueryWhenRelationshipIsOnBaseClass, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementProperty" typeName="int" />
        <ECNavigationProperty propertyName="Parent" relationshipName="ElementOwnsChildElements" direction="Backward" />
    </ECEntityClass>
    <ECEntityClass typeName="Sheet">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="SheetProperty" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsChildElements" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="Element"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_CreatesRecursiveQueryWhenRelationshipIsOnBaseClass)
    {
    ECClassCP baseClass = GetECClass("Element");
    ECClassCP derivedClass = GetECClass("Sheet");
    ECRelationshipClassCP rel = GetECClass("ElementOwnsChildElements")->GetRelationshipClassCP();

    ContentRelatedInstancesSpecification spec(1, 0, true, "", RequiredRelationDirection_Forward, rel->GetFullName(), baseClass->GetFullName());

    TestParsedInput info(*derivedClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*baseClass, "this", true)
            .SetPathFromInputToSelectClass(
                {
                RelatedClass(*derivedClass, SelectClass<ECRelationshipClass>(*rel, RULES_ENGINE_RELATED_CLASS_ALIAS(*rel, 0)), true, SelectClass<ECClass>(*baseClass, RULES_ENGINE_RELATED_CLASS_ALIAS(*baseClass, 0), true), false)
                })
            .SetNavigationPropertyClasses(
                {
                RelatedClass(*baseClass, SelectClass<ECRelationshipClass>(*rel, RULES_ENGINE_NAV_CLASS_ALIAS(*rel, 0)), false, SelectClass<ECClass>(*baseClass, RULES_ENGINE_NAV_CLASS_ALIAS(*baseClass, 0), true), false),
                }), "");
        descriptor->AddSelectClass(SelectClassInfo(*baseClass, "this", true)
            .SetPathFromInputToSelectClass(
                {
                RelatedClass(*derivedClass, SelectClass<ECRelationshipClass>(*rel, RULES_ENGINE_RELATED_CLASS_ALIAS(*rel, 0)), true, SelectClass<ECClass>(*baseClass, RULES_ENGINE_RELATED_CLASS_ALIAS(*baseClass, 0), true)),
                RelatedClass(*baseClass, SelectClass<ECRelationshipClass>(*rel, RULES_ENGINE_RELATED_CLASS_ALIAS(*rel, 1)), true, SelectClass<ECClass>(*baseClass, RULES_ENGINE_RELATED_CLASS_ALIAS(*baseClass, 1), true)),
                })
            .SetNavigationPropertyClasses(
                {
                RelatedClass(*baseClass, SelectClass<ECRelationshipClass>(*rel, RULES_ENGINE_NAV_CLASS_ALIAS(*rel, 0)), false, SelectClass<ECClass>(*baseClass, RULES_ENGINE_NAV_CLASS_ALIAS(*baseClass, 0), true))
                }), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, ContentDescriptor::Property("this", *baseClass, *baseClass->GetPropertyP("ElementProperty")));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, ContentDescriptor::Property(RULES_ENGINE_NAV_CLASS_ALIAS(*baseClass, 0), *baseClass, *baseClass->GetPropertyP("Parent")));

        bvector<ECInstanceId> selectedIds = {ECInstanceId((uint64_t)123)};

        bset<ECRelationshipClassCP> relationships;
        relationships.insert(rel);

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, baseClass, *query), "this");
        query->From(*baseClass, true, "this");
        query->Join(RelatedClass(*baseClass, SelectClass<ECRelationshipClass>(*rel, RULES_ENGINE_NAV_CLASS_ALIAS(*rel, 0)), false, SelectClass<ECClass>(*baseClass, RULES_ENGINE_NAV_CLASS_ALIAS(*baseClass, 0), true)));
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_DoesntSplitRecursiveQueryClassesIntoDerivedClasses, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="ElementProperty" typeName="int" />
        <ECNavigationProperty propertyName="Parent" relationshipName="ElementOwnsChildElements" direction="Backward" />
    </ECEntityClass>
    <ECEntityClass typeName="Sheet">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="SheetProperty" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsChildElements" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="Element"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_DoesntSplitRecursiveQueryClassesIntoDerivedClasses)
    {
    ECClassCP baseClass = GetECClass("Element");
    ECClassCP derivedClass = GetECClass("Sheet");
    ECRelationshipClassCP rel = GetECClass("ElementOwnsChildElements")->GetRelationshipClassCP();

    ContentRelatedInstancesSpecification spec(1, 0, true, "", RequiredRelationDirection_Forward, rel->GetFullName(), baseClass->GetFullName());
    m_ruleset->AddPresentationRule(*new ContentModifier(GetECSchema()->GetName(), derivedClass->GetName()));

    TestParsedInput info(*derivedClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*baseClass, "this", true)
            .SetPathFromInputToSelectClass(
                {
                RelatedClass(*derivedClass, SelectClass<ECRelationshipClass>(*rel, RULES_ENGINE_RELATED_CLASS_ALIAS(*rel, 0)), true, SelectClass<ECClass>(*baseClass, RULES_ENGINE_RELATED_CLASS_ALIAS(*baseClass, 0), true), false)
                })
            .SetNavigationPropertyClasses(
                {
                RelatedClass(*baseClass, SelectClass<ECRelationshipClass>(*rel, RULES_ENGINE_NAV_CLASS_ALIAS(*rel, 0)), false, SelectClass<ECClass>(*baseClass, RULES_ENGINE_NAV_CLASS_ALIAS(*baseClass, 0), true))
                }), "");
        descriptor->AddSelectClass(SelectClassInfo(*baseClass, "this", true)
            .SetPathFromInputToSelectClass(
                {
                RelatedClass(*derivedClass, SelectClass<ECRelationshipClass>(*rel, RULES_ENGINE_RELATED_CLASS_ALIAS(*rel, 0)), true, SelectClass<ECClass>(*baseClass, RULES_ENGINE_RELATED_CLASS_ALIAS(*baseClass, 0), true), false),
                RelatedClass(*baseClass, SelectClass<ECRelationshipClass>(*rel, RULES_ENGINE_RELATED_CLASS_ALIAS(*rel, 1)), true, SelectClass<ECClass>(*baseClass, RULES_ENGINE_RELATED_CLASS_ALIAS(*baseClass, 1), true), false),
                })
            .SetNavigationPropertyClasses(
                {
                RelatedClass(*baseClass, SelectClass<ECRelationshipClass>(*rel, RULES_ENGINE_NAV_CLASS_ALIAS(*rel, 0)), false, SelectClass<ECClass>(*baseClass, RULES_ENGINE_NAV_CLASS_ALIAS(*baseClass, 0), true))
                }), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, ContentDescriptor::Property("this", *baseClass, *baseClass->GetPropertyP("ElementProperty")));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, ContentDescriptor::Property(RULES_ENGINE_NAV_CLASS_ALIAS(*baseClass, 0), *baseClass, *baseClass->GetPropertyP("Parent")));

        bvector<ECInstanceId> selectedIds = {ECInstanceId((uint64_t)123)};

        bset<ECRelationshipClassCP> relationships;
        relationships.insert(rel);

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, baseClass, *query), "this");
        query->From(*baseClass, true, "this");
        query->Join(RelatedClass(*baseClass, SelectClass<ECRelationshipClass>(*rel, RULES_ENGINE_NAV_CLASS_ALIAS(*rel, 0)), false, SelectClass<ECClass>(*baseClass, RULES_ENGINE_NAV_CLASS_ALIAS(*baseClass, 0), true)));
#ifdef WIP_SORTING_GRID_CONTENT
        query->OrderBy(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName).c_str());
#endif
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* Element is related to Element through 2 relationships. When creating a recursive query,
* make sure we don't query from the Element class more than once.
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_DoesntDuplicateRecursiveQueryClasses, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsChildElements" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="Element"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ElementRefersToElements" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="Element"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_DoesntDuplicateRecursiveQueryClasses)
    {
    ECClassCP entityClass = GetECClass("Element");
    ECRelationshipClassCP rel1 = GetECClass("ElementOwnsChildElements")->GetRelationshipClassCP();
    ECRelationshipClassCP rel2 = GetECClass("ElementRefersToElements")->GetRelationshipClassCP();

    ContentRelatedInstancesSpecification spec(1, 0, true, "", RequiredRelationDirection_Forward,
        Utf8PrintfString("%s:%s,%s", rel1->GetSchema().GetName().c_str(), rel1->GetName().c_str(), rel2->GetName().c_str()),
        entityClass->GetFullName());

    TestParsedInput info(*entityClass, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*entityClass, "this", true)
            .SetPathFromInputToSelectClass(
                {
                RelatedClass(*entityClass, SelectClass<ECRelationshipClass>(*rel1, RULES_ENGINE_RELATED_CLASS_ALIAS(*rel1, 0)), true, SelectClass<ECClass>(*entityClass, RULES_ENGINE_RELATED_CLASS_ALIAS(*entityClass, 0), true), false)
                }), "");
        descriptor->AddSelectClass(SelectClassInfo(*entityClass, "this", true)
            .SetPathFromInputToSelectClass(
                {
                RelatedClass(*entityClass, SelectClass<ECRelationshipClass>(*rel2, RULES_ENGINE_RELATED_CLASS_ALIAS(*rel2, 0)), true, SelectClass<ECClass>(*entityClass, RULES_ENGINE_RELATED_CLASS_ALIAS(*entityClass, 1), true), false)
                }), "");

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));

        bvector<ECInstanceId> selectedIds = {ECInstanceId((uint64_t)123)};

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, entityClass, *query), "this");
        query->From(*entityClass, true, "this");
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
DEFINE_SCHEMA(ContentRelatedInstances_SelectPointPropertyRawDataGroupedByDisplayValue, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PointPropB" typeName="point3d" />
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
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_SelectPointPropertyRawDataGroupedByDisplayValue)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();
    ContentRelatedInstancesSpecification spec(1, 0, false, "", RequiredRelationDirection_Forward, relAB->GetFullName(), classB->GetFullName());

    TestParsedInput info(*classA, ECInstanceId((uint64_t)123));
    ContentDescriptorPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());
    descriptor->AddContentFlag(ContentFlags::DistinctValues);

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        RelatedClass aTob(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true), false);
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddContentFlag(ContentFlags::DistinctValues);
        descriptor->AddSelectClass(SelectClassInfo(*classB, "this", true)
            .SetPathFromInputToSelectClass({aTob}), "");
        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classB, *classB->GetPropertyP("PointPropB")));

        ComplexContentQueryPtr nestedQuery = ComplexContentQuery::Create();
        ContentQueryContractPtr contract = CreateQueryContract(1, *descriptor, classB, *nestedQuery, nullptr, {}, "related");
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(*classB, true, "this");
        nestedQuery->Join(ReverseRelationshipPath({aTob}, "related", false));
        nestedQuery->Where("[related].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123))});

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
DEFINE_SCHEMA(ContentRelatedInstances_InstanceLabelOverride_AppliedByPriorityForSpecifiedClass, R"*(
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
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_InstanceLabelOverride_AppliedByPriorityForSpecifiedClass)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();
    ContentRelatedInstancesSpecification spec(1, 0, false, "", RequiredRelationDirection_Forward, "", classB->GetFullName());
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, false, classB->GetFullName(), "PropB1"));
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(2, false, classB->GetFullName(), "PropB2"));

    TestParsedInput info(*classA, ECInstanceId((uint64_t)123));
    ContentDescriptorPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());
    descriptor->AddContentFlag(ContentFlags::ShowLabels);

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        auto& labelOverride = RegisterForDelete(*new InstanceLabelOverride(1, false, classB->GetFullName(), bvector<InstanceLabelOverrideValueSpecification*>{
            new InstanceLabelOverridePropertyValueSpecification("PropB2"),
            new InstanceLabelOverridePropertyValueSpecification("PropB1")
            }));

        RelatedClass aTob(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true), false);

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classB, "this", true)
            .SetPathFromInputToSelectClass({aTob}), "");

        ContentDescriptor::DisplayLabelField* displayLabelField = new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0);
        displayLabelField->SetLabelOverrideSpecs(CreateLabelOverrideSpecificationsMap(*classB, labelOverride));
        AddField(*descriptor, *displayLabelField);
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, ContentDescriptor::Property("this", *classB, *classB->GetPropertyP("PropB1")));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, ContentDescriptor::Property("this", *classB, *classB->GetPropertyP("PropB2")));
        descriptor->AddContentFlag(ContentFlags::ShowLabels);

        SelectClass<ECClass> selectClass(*classB, "this", true);

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        auto contract = CreateQueryContract(1, *descriptor, classB, *query, CreateDisplayLabelField(selectClass, {}, labelOverride.GetValueSpecifications()));
        contract->SetInputClassAlias("related");
        query->SelectContract(*contract, "this");
        query->From(selectClass);
        query->Join(ReverseRelationshipPath({aTob}, "related", false));
        query->Where("[related].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123))});
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ContentRelatedInstances_InstanceLabelOverride_OverrideNavigationProperty, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="PropA" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="string" />
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
TEST_F (ContentQueryBuilderTests, ContentRelatedInstances_InstanceLabelOverride_OverrideNavigationProperty)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();
    ContentRelatedInstancesSpecification spec(1, 0, false, "", RequiredRelationDirection_Forward, "", classB->GetFullName());
    m_ruleset->AddPresentationRule(*new InstanceLabelOverride(1, true, classA->GetFullName(), "PropA"));

    TestParsedInput info(*classA, ECInstanceId((uint64_t)123));
    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec, info);
    ASSERT_TRUE(descriptor.IsValid());

    auto querySet = GetQueryBuilder().CreateQuerySet(spec, *descriptor, info);
    ValidateQueries(querySet, [&]()
        {
        auto& labelOverride = RegisterForDelete(*new InstanceLabelOverride(1, false, classA->GetFullName(), bvector<InstanceLabelOverrideValueSpecification*>{
            new InstanceLabelOverridePropertyValueSpecification("PropA")
            }));

        RelatedClass aTob(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true), false);
        RelatedClass navbToa(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_NAV_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, RULES_ENGINE_NAV_CLASS_ALIAS(*classA, 0), true), true);

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->AddSelectClass(SelectClassInfo(*classB, "this", true)
            .SetPathFromInputToSelectClass({aTob})
            .SetNavigationPropertyClasses({navbToa}), "");

        ContentDescriptor::DisplayLabelField* displayLabelField = new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0);
        displayLabelField->SetLabelOverrideSpecs(CreateLabelOverrideSpecificationsMap(*classA, labelOverride));
        AddField(*descriptor, *displayLabelField);
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty("this", *classB, *classB->GetPropertyP("PropB")));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty(RULES_ENGINE_NAV_CLASS_ALIAS(*classA, 0), *classB, *classB->GetPropertyP("NavA")));

        ComplexContentQueryPtr query = ComplexContentQuery::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classB, *query, nullptr, {}, "related"), "this");
        query->From(*classB, true, "this");
        query->Join(navbToa);
        query->Join(ReverseRelationshipPath({aTob}, "related", false));
        query->Where("[related].[ECInstanceId] IN (?)", {std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123))});
        return query;
        });
    }
