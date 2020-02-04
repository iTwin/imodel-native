/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include "./RulesEngine/ECDbTestProject.h"
#include "./RulesEngine/TestHelpers.h"

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2020
+===============+===============+===============+===============+===============+======*/
struct RelatedClassPathTests : ECPresentationTest
    {
    DECLARE_SCHEMA_REGISTRY(RelatedClassPathTests)
    static ECDbTestProject* s_project;
    static void SetUpTestCase()
        {
        s_project = new ECDbTestProject();
        s_project->Create("RelatedClassPathTests");
        INIT_SCHEMA_REGISTRY(s_project->GetECDb());
        }
    static void TearDownTestCase()
        {
        DELETE_AND_CLEAR(s_project);
        }
    };
ECDbTestProject* RelatedClassPathTests::s_project = nullptr;
DEFINE_SCHEMA_REGISTRY(RelatedClassPathTests)

#define DEFINE_SCHEMA(name, schema_xml) DEFINE_REGISTRY_SCHEMA(RelatedClassPathTests, name, schema_xml)

//---------------------------------------------------------------------------------------
// @betest                                      Grigas.Petraitis                01/2020
//---------------------------------------------------------------------------------------
DEFINE_SCHEMA(Reverse_CorrectlyReversesSingleStepPath, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RelatedClassPathTests, Reverse_CorrectlyReversesSingleStepPath)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECRelationshipClassCP relAB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_To_B")->GetRelationshipClassCP();

    RelatedClassPath path{
        RelatedClass(*classA, *classB, *relAB, true, "b", "r_ab"),
        };
    path.Reverse("a", false);

    ASSERT_EQ(1, path.size());
    EXPECT_EQ(RelatedClass(*classB, SelectClass(*classA, false), *relAB, false, "a", "r_ab"), path[0]);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Grigas.Petraitis                01/2020
//---------------------------------------------------------------------------------------
DEFINE_SCHEMA(Reverse_CorrectlyReversesMultiStepPath, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RelatedClassPathTests, Reverse_CorrectlyReversesMultiStepPath)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    ECRelationshipClassCP relAB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B_To_C")->GetRelationshipClassCP();

    RelatedClassPath path{
        RelatedClass(*classA, *classB, *relAB, true, "b", "r_ab"),
        RelatedClass(*classB, *classC, *relBC, true, "c", "r_bc"),
        };
    path.Reverse("a", false);

    ASSERT_EQ(2, path.size());
    EXPECT_EQ(RelatedClass(*classC, *classB, *relBC, false, "b", "r_bc"), path[0]);
    EXPECT_EQ(RelatedClass(*classB, SelectClass(*classA, false), *relAB, false, "a", "r_ab"), path[1]);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Grigas.Petraitis                01/2020
//---------------------------------------------------------------------------------------
DEFINE_SCHEMA(Reverse_CorrectlyReversesSingleStepPathWithTargetIds, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RelatedClassPathTests, Reverse_CorrectlyReversesSingleStepPathWithTargetIds)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_To_B")->GetRelationshipClassCP();

    bset<ECInstanceId> targetIds = ContainerHelpers::Create<bset<ECInstanceId>>(ECInstanceId((uint64_t)1));
    RelatedClassPath path{
        RelatedClass(*classA, *rel, true, "rel", *classB, targetIds, "b"),
        };
    path.Reverse("a", false);

    ASSERT_EQ(0, path.size());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Grigas.Petraitis                01/2020
//---------------------------------------------------------------------------------------
DEFINE_SCHEMA(Reverse_CorrectlyReversesMultiStepPathWithTargetIds, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECEntityClass typeName="D" />
    <ECEntityClass typeName="E" />
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="C_To_D" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="C"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="D" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="D_To_E" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="D"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="E" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RelatedClassPathTests, Reverse_CorrectlyReversesMultiStepPathWithTargetIds)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    ECClassCP classD = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "D");
    ECClassCP classE = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "E");
    ECRelationshipClassCP relAB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_To_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B_To_C")->GetRelationshipClassCP();
    ECRelationshipClassCP relCD = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C_To_D")->GetRelationshipClassCP();
    ECRelationshipClassCP relDE = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "D_To_E")->GetRelationshipClassCP();

    bset<ECInstanceId> targetIds = ContainerHelpers::Create<bset<ECInstanceId>>(ECInstanceId((uint64_t)1));
    RelatedClassPath path{
        RelatedClass(*classA, *relAB, true, "r_ab", *classB, "b"),
        RelatedClass(*classB, *relBC, true, "r_bc", *classC, targetIds, "c"),
        RelatedClass(*classC, *relCD, true, "r_cd", *classD, "d"),
        RelatedClass(*classD, *relDE, true, "r_de", *classE, "e"),
        };
    path.Reverse("a", false);
    
    ASSERT_EQ(2, path.size());
    EXPECT_EQ(RelatedClass(*classE, *relDE, false, "r_de", *classD, "d"), path[0]);
    EXPECT_EQ(RelatedClass(*classD, *relCD, false, "r_cd", *classC, targetIds, "c"), path[1]);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Grigas.Petraitis                01/2020
//---------------------------------------------------------------------------------------
DEFINE_SCHEMA(Unify_CorrectlyUnifiesMultiStepPath, R"*(
    <ECEntityClass typeName="A">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="AA">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="Base_of_CD">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>Base_of_CD</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>Base_of_CD</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="E" />
    <ECRelationshipClass typeName="B_To_A" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="is contained by" polymorphic="true">
            <Class class="A" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="A_To_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_To_CD" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Base_of_CD" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="CD_To_E" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="contains" polymorphic="true">
            <Class class="Base_of_CD"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="E" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(RelatedClassPathTests, Unify_CorrectlyUnifiesMultiStepPath)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classAA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "AA");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECClassCP classCD = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Base_of_CD");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    ECClassCP classD = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "D");
    ECClassCP classE = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "E");
    ECRelationshipClassCP relBA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B_To_A")->GetRelationshipClassCP();
    ECRelationshipClassCP relBCD = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B_To_CD")->GetRelationshipClassCP();
    ECRelationshipClassCP relCDE = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "CD_To_E")->GetRelationshipClassCP();

    RelatedClassPath path1{
        RelatedClass(*classAA, *relBA, false, "r_ba", *classB, "b"),
        RelatedClass(*classB, *relBCD, true, "r_bcd", *classC, "c"),
        RelatedClass(*classC, *relCDE, true, "r_cde", *classE, "e"),
        };
    RelatedClassPath path2{
        RelatedClass(*classA, *relBA, false, "r_ba", *classB, "b"),
        RelatedClass(*classB, *relBCD, true, "r_bcd", *classD, "d"),
        RelatedClass(*classD, *relCDE, true, "r_cde", *classE, "e"),
        };
    RelatedClassPath result = RelatedClassPath::Unify(path1, path2);

    ASSERT_EQ(3, result.size());
    EXPECT_EQ(RelatedClass(*classA, *relBA, false, "r_ba", *classB, "b"), result[0]);
    EXPECT_EQ(RelatedClass(*classB, *relBCD, true, "r_bcd", *classCD, "c"), result[1]);
    EXPECT_EQ(RelatedClass(*classCD, *relCDE, true, "r_cde", *classE, "e"), result[2]);
    }