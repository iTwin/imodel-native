/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../PresentationManagerIntegrationTests.h"
#include "../../Helpers/ECDbTestProject.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

//=======================================================================================
// @bsiclass
//=======================================================================================
struct LocatingClasses : PresentationManagerIntegrationTests
    {};

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsEmptyListWhenRulesetIsEmpty, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(LocatingClasses, ReturnsEmptyListWhenRulesetIsEmpty)
    {
    // set up lookup classes
    bvector<ECClassCP> lookup({GetClass("A")});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);

    // validate descriptor
    bvector<SelectClassInfo> result = GetValidatedResponse(m_manager->GetContentClasses(AsyncContentClassesRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, lookup)));
    EXPECT_EQ(0, result.size());
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsLookupListWhenRulesetHasSelectedNodeInstancesRule, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
)*");
TEST_F(LocatingClasses, ReturnsLookupListWhenRulesetHasSelectedNodeInstancesRule)
    {
    // set up lookup classes
    bvector<ECClassCP> lookup({GetClass("A"), GetClass("B"), GetClass("C")});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new ContentRule());
    rules->GetContentRules().back()->AddSpecification(*new SelectedNodeInstancesSpecification());

    // validate descriptor
    bvector<SelectClassInfo> result = GetValidatedResponse(m_manager->GetContentClasses(AsyncContentClassesRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, lookup)));
    ASSERT_EQ(lookup.size(), result.size());
    for (size_t i = 0; i < lookup.size(); ++i)
        {
        EXPECT_EQ(lookup[i], &result[i].GetSelectClass().GetClass());
        EXPECT_TRUE(result[i].GetSelectClass().IsSelectPolymorphic());
        }
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsFilteredLookupListWhenRulesetHasSelectedNodeInstancesRuleWithAcceptableClasses, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
)*");
TEST_F(LocatingClasses, ReturnsFilteredLookupListWhenRulesetHasSelectedNodeInstancesRuleWithAcceptableClasses)
    {
    // set up lookup classes
    bvector<ECClassCP> lookup({GetClass("A"), GetClass("B"), GetClass("C")});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new ContentRule());
    rules->GetContentRules().back()->AddSpecification(*new SelectedNodeInstancesSpecification(1, false,
        "", "A,C", true));

    // validate descriptor
    bvector<SelectClassInfo> result = GetValidatedResponse(m_manager->GetContentClasses(AsyncContentClassesRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, lookup)));
    ASSERT_EQ(2, result.size());

    EXPECT_EQ(GetClass("A"), &result[0].GetSelectClass().GetClass());
    EXPECT_TRUE(result[0].GetSelectClass().IsSelectPolymorphic());

    EXPECT_EQ(GetClass("C"), &result[1].GetSelectClass().GetClass());
    EXPECT_TRUE(result[1].GetSelectClass().IsSelectPolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsLookupListWhenRulesetHasInstanceNodesOfSpecificClassesRule, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
)*");
TEST_F(LocatingClasses, ReturnsLookupListWhenRulesetHasInstanceNodesOfSpecificClassesRule)
    {
    // set up lookup classes
    bvector<ECClassCP> lookup({GetClass("A"), GetClass("B"), GetClass("C")});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new ContentRule());
    rules->GetContentRules().back()->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", RulesEngineTestHelpers::CreateClassNamesList(lookup), false, false));

    // validate descriptor
    bvector<SelectClassInfo> result = GetValidatedResponse(m_manager->GetContentClasses(AsyncContentClassesRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, lookup)));
    ASSERT_EQ(lookup.size(), result.size());
    for (size_t i = 0; i < lookup.size(); ++i)
        {
        EXPECT_EQ(lookup[i], &result[i].GetSelectClass().GetClass());
        EXPECT_TRUE(result[i].GetSelectClass().IsSelectPolymorphic());
        }
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsLookupListWhenAllRulesetRuleSpecificationsContainsOnlyIfNotHandledSpecifications, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
)*");
TEST_F(LocatingClasses, ReturnsLookupListWhenAllRulesetRuleSpecificationsContainsOnlyIfNotHandledSpecifications)
    {
    // set up lookup classes
    ECClassCP classA = GetClass("A");
    ECClassCP classB = GetClass("B");
    ECClassCP classC = GetClass("C");
    bvector<ECClassCP> lookup({ classA, classB, classC });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new ContentRule());

    auto specA = new ContentInstancesOfSpecificClassesSpecification(2, true, "",
        bvector<MultiSchemaClass*> { new MultiSchemaClass(BeTest::GetNameOfCurrentTest(), false, bvector<Utf8String> { "A" })},
        bvector<MultiSchemaClass*>(), false);
    rules->GetContentRules().back()->AddSpecification(*specA);

    auto specB = new SelectedNodeInstancesSpecification(1, true, BeTest::GetNameOfCurrentTest(), "B", false);
    rules->GetContentRules().back()->AddSpecification(*specB);

    auto specC = new ContentInstancesOfSpecificClassesSpecification(1, true, "",
        bvector<MultiSchemaClass*> { new MultiSchemaClass(BeTest::GetNameOfCurrentTest(), false, bvector<Utf8String> { "C" })},
        bvector<MultiSchemaClass*>(), false);
    rules->GetContentRules().back()->AddSpecification(*specC);

    bvector<SelectClassInfo> result = GetValidatedResponse(m_manager->GetContentClasses(AsyncContentClassesRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, lookup)));

    // validate specA classes
    ASSERT_EQ(1, result.size());
    EXPECT_EQ(classA, &result[0].GetSelectClass().GetClass());
    EXPECT_TRUE(result[0].GetSelectClass().IsSelectPolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsFilteredListWhenRulesetHasInstanceNodesOfSpecificClassesRuleWithSupportedClasses, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
)*");
TEST_F(LocatingClasses, ReturnsFilteredListWhenRulesetHasInstanceNodesOfSpecificClassesRuleWithSupportedClasses)
    {
    // set up lookup classes
    bvector<ECClassCP> lookup({GetClass("A"), GetClass("B"), GetClass("C")});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new ContentRule());
    rules->GetContentRules().back()->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", Utf8PrintfString("%s:A,C", GetSchema()->GetName().c_str()), false, false));

    // validate descriptor
    bvector<SelectClassInfo> result = GetValidatedResponse(m_manager->GetContentClasses(AsyncContentClassesRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, lookup)));
    ASSERT_EQ(2, result.size());

    EXPECT_EQ(GetClass("A"), &result[0].GetSelectClass().GetClass());
    EXPECT_TRUE(result[0].GetSelectClass().IsSelectPolymorphic());

    EXPECT_EQ(GetClass("C"), &result[1].GetSelectClass().GetClass());
    EXPECT_TRUE(result[1].GetSelectClass().IsSelectPolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
// note: we don't care about ContentRelatedInstances rule for GetContentClasses request.
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsEmptyListWhenRulesetHasContentRelatedInstancesRule, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="R" strength="referencing" strengthDirection="forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="R" polymorphic="False">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="R" polymorphic="False">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(LocatingClasses, ReturnsEmptyListWhenRulesetHasContentRelatedInstancesRule)
    {
    // set up lookup classes
    bvector<ECClassCP> lookup({GetClass("A")});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new ContentRule());
    rules->GetContentRules().back()->AddSpecification(*new ContentRelatedInstancesSpecification(1, false, false,
        "", RequiredRelationDirection_Forward, GetRelationshipClass("R")->GetFullName(), GetClass("B")->GetFullName()));

    // validate descriptor
    bvector<SelectClassInfo> result = GetValidatedResponse(m_manager->GetContentClasses(AsyncContentClassesRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, lookup)));
    EXPECT_EQ(0, result.size());
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SplitsLookupListClassesByClassesUsedInRuleConditionIsOfClassExpression, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C" />
    <ECEntityClass typeName="D">
        <BaseClass>C</BaseClass>
    </ECEntityClass>
)*");
TEST_F(LocatingClasses, SplitsLookupListClassesByClassesUsedInRuleConditionIsOfClassExpression)
    {
    // set up lookup classes
    bvector<ECClassCP> lookup({GetClass("A"), GetClass("C")});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new ContentRule(Utf8PrintfString("SelectedNode.IsOfClass(\"D\", \"%s\")", GetSchema()->GetName().c_str()), 1, false));
    rules->GetContentRules().back()->AddSpecification(*new SelectedNodeInstancesSpecification());

    // validate descriptor
    bvector<SelectClassInfo> result = GetValidatedResponse(m_manager->GetContentClasses(AsyncContentClassesRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, lookup)));
    ASSERT_EQ(1, result.size());
    EXPECT_EQ(GetClass("D"), &result[0].GetSelectClass().GetClass());
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SplitsLookupListClassesByClassesUsedInRuleConditionClassNameExpression, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C" />
    <ECEntityClass typeName="D">
        <BaseClass>C</BaseClass>
    </ECEntityClass>
)*");
TEST_F(LocatingClasses, SplitsLookupListClassesByClassesUsedInRuleConditionClassNameExpression)
    {
    // set up lookup classes
    bvector<ECClassCP> lookup({GetClass("A"), GetClass("C")});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new ContentRule("SelectedNode.ClassName = \"D\"", 1, false));
    rules->GetContentRules().back()->AddSpecification(*new SelectedNodeInstancesSpecification());

    // validate descriptor
    bvector<SelectClassInfo> result = GetValidatedResponse(m_manager->GetContentClasses(AsyncContentClassesRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, lookup)));
    ASSERT_EQ(1, result.size());
    EXPECT_EQ(GetClass("D"), &result[0].GetSelectClass().GetClass());
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DetectsRelatedPropertyPathsWhenPropertiesAreOnDerivedClasses, R"*(
    <ECEntityClass typeName="X" />
    <ECEntityClass typeName="A">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>C</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="X_A" strength="referencing" strengthDirection="Forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="xa" polymorphic="true">
            <Class class="X" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ax" polymorphic="true">
            <Class class="A"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(LocatingClasses, DetectsRelatedPropertyPathsWhenPropertiesAreOnDerivedClasses)
    {
    ECClassCP classX = GetClass("X");
    ECClassCP classA = GetClass("A");
    ECClassCP relXA = GetClass("X_A");

    // set up lookup classes
    bvector<ECClassCP> lookup({ classX });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new ContentRule());
    rules->GetContentRules().back()->AddSpecification(*new SelectedNodeInstancesSpecification());
    rules->AddPresentationRule(*new ContentModifier(GetSchema()->GetName(), classX->GetName()));
    rules->GetContentModifierRules().back()->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relXA->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true));

    // validate descriptor
    bvector<SelectClassInfo> result = GetValidatedResponse(m_manager->GetContentClasses(AsyncContentClassesRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, lookup)));
    ASSERT_EQ(1, result.size());
    EXPECT_EQ(classX, &result[0].GetSelectClass().GetClass());
    EXPECT_EQ(1, result[0].GetRelatedPropertyPaths().size());
    EXPECT_EQ(1, result[0].GetRelatedPropertyPaths()[0].size());
    EXPECT_EQ(relXA, &result[0].GetRelatedPropertyPaths()[0][0].GetRelationship().GetClass());
    EXPECT_EQ(classA, &result[0].GetRelatedPropertyPaths()[0][0].GetTargetClass().GetClass());
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DetectsRelatedPropertyPathsOnDerivedClasses, R"*(
    <ECEntityClass typeName="A">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="B_C" strength="referencing" strengthDirection="Forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="bc" polymorphic="true">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="cb" polymorphic="false">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(LocatingClasses, DetectsRelatedPropertyPathsOnDerivedClasses)
    {
    auto classA = GetClass("A");
    auto classB = GetClass("B");
    auto classC = GetClass("C");
    auto relBC = GetClass("B_C")->GetRelationshipClassCP();

    // set up lookup classes
    bvector<ECClassCP> lookup({ classA });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new ContentRule());
    rules->GetContentRules().back()->AddSpecification(*new SelectedNodeInstancesSpecification());
    rules->AddPresentationRule(*new ContentModifier(GetSchema()->GetName(), classB->GetName()));
    rules->GetContentModifierRules().back()->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection_Forward),
        }), { new PropertySpecification("*") }, RelationshipMeaning::RelatedInstance, true));

    // validate descriptor
    bvector<SelectClassInfo> result = GetValidatedResponse(m_manager->GetContentClasses(AsyncContentClassesRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, lookup)));
    ASSERT_EQ(1, result.size());
    EXPECT_EQ(classA, &result[0].GetSelectClass().GetClass());
    EXPECT_EQ(1, result[0].GetRelatedPropertyPaths().size());
    EXPECT_EQ(1, result[0].GetRelatedPropertyPaths()[0].size());
    EXPECT_EQ(relBC, &result[0].GetRelatedPropertyPaths()[0][0].GetRelationship().GetClass());
    EXPECT_EQ(classC, &result[0].GetRelatedPropertyPaths()[0][0].GetTargetClass().GetClass());
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DetectsNavigationPropertyPathsWhenPropertiesAreOnDerivedClasses, R"*(
    <ECEntityClass typeName="A">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
        <ECNavigationProperty propertyName="NavProp" relationshipName="NavPropRel" direction="Forward" />
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>C</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="E" />
    <ECRelationshipClass typeName="NavPropRel" strength="referencing" strengthDirection="Backward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="has" polymorphic="true">
            <Class class="C" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="is referenced by" polymorphic="false">
            <Class class="E"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(LocatingClasses, DetectsNavigationPropertyPathsWhenPropertiesAreOnDerivedClasses)
    {
    // set up lookup classes
    bvector<ECClassCP> lookup({GetClass("A")});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new ContentRule());
    rules->GetContentRules().back()->AddSpecification(*new SelectedNodeInstancesSpecification());

    // validate descriptor
    bvector<SelectClassInfo> result = GetValidatedResponse(m_manager->GetContentClasses(AsyncContentClassesRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, lookup)));
    ASSERT_EQ(1, result.size());
    EXPECT_EQ(GetClass("A"), &result[0].GetSelectClass().GetClass());
    EXPECT_EQ(1, result[0].GetNavigationPropertyClasses().size());
    EXPECT_EQ(GetClass("NavPropRel"), &result[0].GetNavigationPropertyClasses()[0].GetRelationship().GetClass());
    EXPECT_EQ(GetClass("E"), &result[0].GetNavigationPropertyClasses()[0].GetTargetClass().GetClass());
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DetectsOnlyDisplayedNavigationProperties, R"*(
    <ECEntityClass typeName="A">
        <ECNavigationProperty propertyName="DisplayedProp" relationshipName="A_B" direction="Forward" />
        <ECNavigationProperty propertyName="HiddenProp" relationshipName="A_C" direction="Forward" />
    </ECEntityClass>
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_B" strength="referencing" strengthDirection="Backward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="A_C" strength="referencing" strengthDirection="Backward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="ac" polymorphic="true">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ca" polymorphic="true">
            <Class class="C"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(LocatingClasses, DetectsOnlyDisplayedNavigationProperties)
    {
    // set up lookup classes
    bvector<ECClassCP> lookup({ GetClass("A") });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new ContentRule());
    rules->GetContentRules().back()->AddSpecification(*new SelectedNodeInstancesSpecification());
    rules->AddPresentationRule(*new ContentModifier(GetSchema()->GetName(), GetClass("A")->GetName()));
    rules->GetContentModifierRules().back()->AddPropertyOverride(*new PropertySpecification("HiddenProp", 1000, "", nullptr, false));

    // validate descriptor
    bvector<SelectClassInfo> result = GetValidatedResponse(m_manager->GetContentClasses(AsyncContentClassesRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, lookup)));
    ASSERT_EQ(1, result.size());
    EXPECT_EQ(GetClass("A"), &result[0].GetSelectClass().GetClass());
    EXPECT_EQ(1, result[0].GetNavigationPropertyClasses().size());
    EXPECT_EQ(GetClass("A_B"), &result[0].GetNavigationPropertyClasses()[0].GetRelationship().GetClass());
    EXPECT_EQ(GetClass("B"), &result[0].GetNavigationPropertyClasses()[0].GetTargetClass().GetClass());
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DetectsOnlyDisplayedRelatedNavigationProperties, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
        <ECNavigationProperty propertyName="DisplayedProp" relationshipName="C_E" direction="Forward" />
        <ECNavigationProperty propertyName="HiddenProp" relationshipName="C_F" direction="Forward" />
    </ECEntityClass>
    <ECEntityClass typeName="E" />
    <ECEntityClass typeName="F" />
    <ECRelationshipClass typeName="A_B" strength="referencing" strengthDirection="Forward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ba" polymorphic="true">
            <Class class="B"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="C_E" strength="referencing" strengthDirection="Backward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="ce" polymorphic="true">
            <Class class="C" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ec" polymorphic="true">
            <Class class="E"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="C_F" strength="referencing" strengthDirection="Backward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="cf" polymorphic="true">
            <Class class="C" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="fc" polymorphic="true">
            <Class class="F"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(LocatingClasses, DetectsOnlyDisplayedRelatedNavigationProperties)
    {
    // set up lookup classes
    bvector<ECClassCP> lookup({ GetClass("A") });

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new ContentRule());
    rules->GetContentRules().back()->AddSpecification(*new SelectedNodeInstancesSpecification());
    rules->AddPresentationRule(*new ContentModifier(GetSchema()->GetName(), GetClass("A")->GetName()));
    rules->GetContentModifierRules().back()->AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(
        {
        new RelationshipStepSpecification(GetClass("A_B")->GetFullName(), RequiredRelationDirection_Forward, GetClass("C")->GetFullName()),
        }), { new PropertySpecification("DisplayedProp") }, RelationshipMeaning::RelatedInstance, true));

    // validate descriptor
    bvector<SelectClassInfo> result = GetValidatedResponse(m_manager->GetContentClasses(AsyncContentClassesRequestParams::Create(s_project->GetECDb(), rules->GetRuleSetId(), RulesetVariables(), "", 0, lookup)));
    ASSERT_EQ(1, result.size());
    EXPECT_EQ(GetClass("A"), &result[0].GetSelectClass().GetClass());
    EXPECT_EQ(2, result[0].GetRelatedPropertyPaths().size());
    EXPECT_EQ(2, result[0].GetRelatedPropertyPaths()[0].size());
    EXPECT_EQ(GetClass("A_B"), &result[0].GetRelatedPropertyPaths()[0][0].GetRelationship().GetClass());
    EXPECT_EQ(GetClass("C"), &result[0].GetRelatedPropertyPaths()[0][0].GetTargetClass().GetClass());
    EXPECT_EQ(GetClass("C_E"), &result[0].GetRelatedPropertyPaths()[0][1].GetRelationship().GetClass());
    EXPECT_EQ(GetClass("E"), &result[0].GetRelatedPropertyPaths()[0][1].GetTargetClass().GetClass());
    EXPECT_EQ(1, result[0].GetRelatedPropertyPaths()[1].size());
    EXPECT_EQ(GetClass("A_B"), &result[0].GetRelatedPropertyPaths()[1][0].GetRelationship().GetClass());
    EXPECT_EQ(GetClass("C"), &result[0].GetRelatedPropertyPaths()[1][0].GetTargetClass().GetClass());
    }