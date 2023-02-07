/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ContentQueryBuilderTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedContentField_WithSingleStepRelationshipPath, R"*(
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
TEST_F (ContentQueryBuilderTests, RelatedContentField_WithSingleStepRelationshipPath)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();

    ContentDescriptor::RelatedContentField field(std::make_unique<ContentDescriptor::Category>("name", "label", "", 1), "field_name", "field_label",
        {RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0)))},
        {
        new ContentDescriptor::ECPropertiesField(DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), *classB, *classB->GetPropertyP("PropB")))
        });

    auto querySet = GetQueryBuilder().CreateQuerySet(field);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->RemoveContentFlag(ContentFlags::SkipInstancesCheck);
        descriptor->AddSelectClass(SelectClassInfo(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true), "");
        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), *classB, *classB->GetPropertyP("PropB")));

        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classB, *query), RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0).c_str());
        query->From(*classB, true, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0).c_str());
        query->Join(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, "related", true), false));
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedContentField_WithMultiStepRelationshipPath, R"*(
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
TEST_F (ContentQueryBuilderTests, RelatedContentField_WithMultiStepRelationshipPath)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetECClass("B_To_C")->GetRelationshipClassCP();

    ContentDescriptor::RelatedContentField field(std::make_unique<ContentDescriptor::Category>("name", "label", "", 1), "field_name", "field_label",
        {
        RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0))),
        RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relBC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relBC, 0)), true, SelectClass<ECClass>(*classC, RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0))),
        },
        {
        new ContentDescriptor::ECPropertiesField(DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0), *classC, *classC->GetPropertyP("PropC"))),
        });

    auto querySet = GetQueryBuilder().CreateQuerySet(field);
    ValidateQueries(querySet, [&]()
        {
        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->RemoveContentFlag(ContentFlags::SkipInstancesCheck);
        descriptor->AddSelectClass(SelectClassInfo(*classC, RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0), true), "");
        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0), *classC, *classC->GetPropertyP("PropC")));

        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classC, *query), RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0).c_str());
        query->From(*classC, true, RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0).c_str());

        RelatedClassPath path = {
            RelatedClass(*classC, SelectClass<ECRelationshipClass>(*relBC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relBC, 0)), false, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true), false),
            RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, "related", true), false)
            };
        query->Join(path);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(RelatedContentField_WithNestedContentFields, R"*(
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
TEST_F (ContentQueryBuilderTests, RelatedContentField_WithNestedContentFields)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAB = GetECClass("A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = GetECClass("B_To_C")->GetRelationshipClassCP();

    auto category = std::make_shared<ContentDescriptor::Category>("name", "label", "", 1);

    ContentDescriptor::RelatedContentField field(category, "classB_field_name", "classB_field_label",
        {RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), false))},
        {
        new ContentDescriptor::ECPropertiesField(DEFAULT_CONTENT_FIELD_CATEGORY, CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), *classB, *classB->GetPropertyP("PropB"))),
        new ContentDescriptor::RelatedContentField(category, "classC_field_name", "classC_field_label",
            {RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relBC,  RULES_ENGINE_RELATED_CLASS_ALIAS(*relBC, 0)), true, SelectClass<ECClass>(*classC, RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0), false))},
            {
            new ContentDescriptor::ECPropertiesField(category, ContentDescriptor::Property(RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0), *classC, *classC->GetPropertyP("PropC"))),
            })
        });

    auto querySet = GetQueryBuilder().CreateQuerySet(field);
    ValidateQueries(querySet, [&]()
        {
        RelatedClassPath bTocPath{ RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relBC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relBC, 0)), true, SelectClass<ECClass>(*classC, RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0), false)) };

        ContentDescriptorPtr descriptor = GetEmptyContentDescriptor();
        descriptor->RemoveContentFlag(ContentFlags::SkipInstancesCheck);
        descriptor->AddSelectClass(SelectClassInfo(*classB, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), true)
            .SetRelatedPropertyPaths({bTocPath}), "");

        auto customCategory = std::make_shared<ContentDescriptor::Category>("name", "label", "", 1);

        AddField(*descriptor, *new ContentDescriptor::DisplayLabelField(DEFAULT_CONTENT_FIELD_CATEGORY, CommonStrings::ECPRESENTATION_DISPLAYLABEL, 0));
        AddField(*descriptor, DEFAULT_CONTENT_FIELD_CATEGORY, ContentDescriptor::Property(RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0), *classB, *classB->GetPropertyP("PropB")));
        AddField(*descriptor, *new ContentDescriptor::RelatedContentField(customCategory, "classC_field_name", "classC_field_label", bTocPath,
            {
            CreatePropertiesField(customCategory, CreateProperty(RULES_ENGINE_RELATED_CLASS_ALIAS(*classC, 0), *classC, *classC->GetPropertyP("PropC")))
            }));

        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->SelectContract(*CreateQueryContract(1, *descriptor, classB, *query), RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0).c_str());
        query->From(*classB, true, RULES_ENGINE_RELATED_CLASS_ALIAS(*classB, 0).c_str());
        query->Join(bTocPath);
        query->Join(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, "related", true), false));
        return query;
        });
    }
