/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PresentationManagerIntegrationTests.h"
#include "../RulesEngine/ECDbTestProject.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

//=======================================================================================
// @bsiclass                                     Grigas.Petraitis                10/2017
//=======================================================================================
struct LocatingClasses : PresentationManagerIntegrationTests
    {};

/*---------------------------------------------------------------------------------**//**
// @betest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(ReturnsEmptyListWhenRulesetIsEmpty, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(LocatingClasses, ReturnsEmptyListWhenRulesetIsEmpty)
    {
    // set up lookup classes
    bvector<ECClassCP> lookup({GetClass("A")});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("ReturnsEmptyListWhenRulesetIsEmpty", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);

    // validate descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    bvector<SelectClassInfo> result = m_manager->GetContentClasses(s_project->GetECDb(), nullptr, 0, lookup, options.GetJson()).get();
    EXPECT_EQ(0, result.size());
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                      Grigas.Petraitis                10/2017
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("ReturnsLookupListWhenRulesetHasSelectedNodeInstancesRule", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new ContentRule());
    rules->GetContentRules().back()->AddSpecification(*new SelectedNodeInstancesSpecification());

    // validate descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    bvector<SelectClassInfo> result = m_manager->GetContentClasses(s_project->GetECDb(), nullptr, 0, lookup, options.GetJson()).get();
    ASSERT_EQ(lookup.size(), result.size());
    for (size_t i = 0; i < lookup.size(); ++i)
        {
        EXPECT_EQ(lookup[i], &result[i].GetSelectClass());
        EXPECT_TRUE(result[i].IsSelectPolymorphic());
        }
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                      Grigas.Petraitis                10/2017
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("ReturnsFilteredLookupListWhenRulesetHasSelectedNodeInstancesRuleWithAcceptableClasses", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new ContentRule());
    rules->GetContentRules().back()->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, 
        "", "A,C", true));

    // validate descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    bvector<SelectClassInfo> result = m_manager->GetContentClasses(s_project->GetECDb(), nullptr, 0, lookup, options.GetJson()).get();
    ASSERT_EQ(2, result.size());

    EXPECT_EQ(GetClass("A"), &result[0].GetSelectClass());
    EXPECT_TRUE(result[0].IsSelectPolymorphic());
    
    EXPECT_EQ(GetClass("C"), &result[1].GetSelectClass());
    EXPECT_TRUE(result[1].IsSelectPolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                      Grigas.Petraitis                10/2017
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("ReturnsLookupListWhenRulesetHasInstanceNodesOfSpecificClassesRule", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new ContentRule());
    rules->GetContentRules().back()->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "",
        GetClassNamesList(lookup), false));

    // validate descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    bvector<SelectClassInfo> result = m_manager->GetContentClasses(s_project->GetECDb(), nullptr, 0, lookup, options.GetJson()).get();
    ASSERT_EQ(lookup.size(), result.size());
    for (size_t i = 0; i < lookup.size(); ++i)
        {
        EXPECT_EQ(lookup[i], &result[i].GetSelectClass());
        EXPECT_TRUE(result[i].IsSelectPolymorphic());
        }
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                      Grigas.Petraitis                10/2017
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("ReturnsFilteredListWhenRulesetHasInstanceNodesOfSpecificClassesRuleWithSupportedClasses", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new ContentRule());
    rules->GetContentRules().back()->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "",
        Utf8PrintfString("%s:A,C", GetSchema()->GetName().c_str()), false));

    // validate descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    bvector<SelectClassInfo> result = m_manager->GetContentClasses(s_project->GetECDb(), nullptr, 0, lookup, options.GetJson()).get();
    ASSERT_EQ(2, result.size());

    EXPECT_EQ(GetClass("A"), &result[0].GetSelectClass());
    EXPECT_TRUE(result[0].IsSelectPolymorphic());
    
    EXPECT_EQ(GetClass("C"), &result[1].GetSelectClass());
    EXPECT_TRUE(result[1].IsSelectPolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
// note: we don't care about ContentRelatedInstances rule for GetContentClasses request.
// @betest                                      Grigas.Petraitis                10/2017
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("ReturnsEmptyListWhenRulesetHasContentRelatedInstancesRule", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new ContentRule());
    rules->GetContentRules().back()->AddSpecification(*new ContentRelatedInstancesSpecification(1, false, false,
        "", RequiredRelationDirection_Forward, GetRelationshipClass("R")->GetFullName(), GetClass("B")->GetFullName()));

    // validate descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    bvector<SelectClassInfo> result = m_manager->GetContentClasses(s_project->GetECDb(), nullptr, 0, lookup, options.GetJson()).get();
    EXPECT_EQ(0, result.size());
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                      Grigas.Petraitis                10/2017
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("SplitsLookupListClassesByClassesUsedInRuleConditionIsOfClassExpression", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new ContentRule(Utf8PrintfString("SelectedNode.IsOfClass(\"D\", \"%s\")", GetSchema()->GetName().c_str()), 1, false));
    rules->GetContentRules().back()->AddSpecification(*new SelectedNodeInstancesSpecification());

    // validate descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    bvector<SelectClassInfo> result = m_manager->GetContentClasses(s_project->GetECDb(), nullptr, 0, lookup, options.GetJson()).get();
    ASSERT_EQ(1, result.size());
    EXPECT_EQ(GetClass("D"), &result[0].GetSelectClass());
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                      Grigas.Petraitis                10/2017
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("SplitsLookupListClassesByClassesUsedInRuleConditionClassNameExpression", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new ContentRule("SelectedNode.ClassName = \"D\"", 1, false));
    rules->GetContentRules().back()->AddSpecification(*new SelectedNodeInstancesSpecification());

    // validate descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    bvector<SelectClassInfo> result = m_manager->GetContentClasses(s_project->GetECDb(), nullptr, 0, lookup, options.GetJson()).get();
    ASSERT_EQ(1, result.size());
    EXPECT_EQ(GetClass("D"), &result[0].GetSelectClass());
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SplitsLookupListClassesByClassesUsedInContentModifiers, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C" />
    <ECEntityClass typeName="D">
        <BaseClass>C</BaseClass>
    </ECEntityClass>
)*");
TEST_F(LocatingClasses, SplitsLookupListClassesByClassesUsedInContentModifiers)
    {    
    // set up lookup classes
    bvector<ECClassCP> lookup({GetClass("A"), GetClass("C")});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("SplitsLookupListClassesByClassesUsedInContentModifiers", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new ContentRule());
    rules->GetContentRules().back()->AddSpecification(*new SelectedNodeInstancesSpecification());
    rules->AddPresentationRule(*new ContentModifier(GetSchema()->GetName(), "D"));

    // validate descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    bvector<SelectClassInfo> result = m_manager->GetContentClasses(s_project->GetECDb(), nullptr, 0, lookup, options.GetJson()).get();
    ASSERT_EQ(3, result.size());
    EXPECT_EQ(GetClass("A"), &result[0].GetSelectClass());
    EXPECT_EQ(GetClass("C"), &result[1].GetSelectClass());
    EXPECT_EQ(GetClass("D"), &result[2].GetSelectClass());
    }

/*---------------------------------------------------------------------------------**//**
// @betest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(SplitsLookupListClassesByClassesWhichHaveNavigationProperties, R"*(
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
    <ECEntityClass typeName="E">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="NavPropRel" strength="referencing" strengthDirection="Backward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="has" polymorphic="true">
            <Class class="C" />
        </Source>
        <Target multiplicity="(0..1)" roleLabel="is referenced by" polymorphic="false">
            <Class class="E"/>
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(LocatingClasses, SplitsLookupListClassesByClassesWhichHaveNavigationProperties)
    {    
    // set up lookup classes
    bvector<ECClassCP> lookup({GetClass("A")});

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance("SplitsLookupListClassesByClassesWhichHaveNavigationProperties", 1, 0, false, "", "", "", false);
    m_locater->AddRuleSet(*rules);
    rules->AddPresentationRule(*new ContentRule());
    rules->GetContentRules().back()->AddSpecification(*new SelectedNodeInstancesSpecification());

    // validate descriptor
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    bvector<SelectClassInfo> result = m_manager->GetContentClasses(s_project->GetECDb(), nullptr, 0, lookup, options.GetJson()).get();
    ASSERT_EQ(2, result.size());
    EXPECT_EQ(GetClass("A"), &result[0].GetSelectClass());
    EXPECT_EQ(GetClass("C"), &result[1].GetSelectClass());
    }