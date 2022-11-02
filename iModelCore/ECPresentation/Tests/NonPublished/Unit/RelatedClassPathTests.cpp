/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../Helpers/ECDbTestProject.h"
#include "../Helpers/TestHelpers.h"

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass
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
// @bsimethod
//---------------------------------------------------------------------------------------
static RelatedClass WithTargetInstanceFilter(RelatedClass rc, Utf8String filter)
    {
    rc.SetTargetInstanceFilter(filter);
    return rc;
    }

//---------------------------------------------------------------------------------------
// @betest
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
        RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, "r_ab"), true, SelectClass<ECClass>(*classB, "b")),
        };
    ASSERT_EQ(SUCCESS, path.Reverse("a", false));
    ASSERT_EQ(1, path.size());
    EXPECT_EQ(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, "r_ab"), false, SelectClass<ECClass>(*classA, "a", false)), path[0]);
    }

//---------------------------------------------------------------------------------------
// @betest
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
        RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, "r_ab"), true, SelectClass<ECClass>(*classB, "b")),
        RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relBC, "r_bc"), true, SelectClass<ECClass>(*classC, "c")),
        };
    ASSERT_EQ(SUCCESS, path.Reverse("a", false));
    ASSERT_EQ(2, path.size());
    EXPECT_EQ(RelatedClass(*classC, SelectClass<ECRelationshipClass>(*relBC, "r_bc"), false, SelectClass<ECClass>(*classB, "b")), path[0]);
    EXPECT_EQ(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, "r_ab"), false, SelectClass<ECClass>(*classA, "a", false)), path[1]);
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
DEFINE_SCHEMA(Reverse_ReturnsErrorWhenReversingSingleStepPathWithTargetIds, R"*(
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
TEST_F(RelatedClassPathTests, Reverse_ReturnsErrorWhenReversingSingleStepPathWithTargetIds)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_To_B")->GetRelationshipClassCP();

    RelatedClassPath path{
        RelatedClass(*classA, SelectClass<ECRelationshipClass>(*rel, "rel"), true, SelectClass<ECClass>(*classB, "b"), { ECInstanceId((uint64_t)1) }),
        };
    ASSERT_EQ(ERROR, path.Reverse("a", false));
    ASSERT_EQ(0, path.size());
    }

//---------------------------------------------------------------------------------------
// @betest
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

    bvector<ECInstanceId> targetIds = { ECInstanceId((uint64_t)1) };
    RelatedClassPath path{
        RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, "r_ab"), true, SelectClass<ECClass>(*classB, "b")),
        RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relBC, "r_bc"), true, SelectClass<ECClass>(*classC, "c"), targetIds),
        RelatedClass(*classC, SelectClass<ECRelationshipClass>(*relCD, "r_cd"), true, SelectClass<ECClass>(*classD, "d")),
        RelatedClass(*classD, SelectClass<ECRelationshipClass>(*relDE, "r_de"), true, SelectClass<ECClass>(*classE, "e")),
        };
    ASSERT_EQ(SUCCESS, path.Reverse("a", false));
    ASSERT_EQ(2, path.size());
    EXPECT_EQ(RelatedClass(*classE, SelectClass<ECRelationshipClass>(*relDE, "r_de"), false, SelectClass<ECClass>(*classD, "d")), path[0]);
    EXPECT_EQ(RelatedClass(*classD, SelectClass<ECRelationshipClass>(*relCD, "r_cd"), false, SelectClass<ECClass>(*classC, "c"), targetIds), path[1]);
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
DEFINE_SCHEMA(Reverse_CorrectlyReversesSingleStepPathWithTargetInstanceFilter, R"*(
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
TEST_F(RelatedClassPathTests, Reverse_CorrectlyReversesSingleStepPathWithTargetInstanceFilter)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_To_B")->GetRelationshipClassCP();

    RelatedClassPath path{
        WithTargetInstanceFilter(RelatedClass(*classA, SelectClass<ECRelationshipClass>(*rel, "rel"), true, SelectClass<ECClass>(*classB, "b")), "xxx")
        };
    ASSERT_EQ(SUCCESS, path.Reverse("a", true));
    EXPECT_EQ(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*rel, "rel"), false, SelectClass<ECClass>(*classA, "a")), path[0]);
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
DEFINE_SCHEMA(Reverse_CorrectlyReversesMultiStepPathWithTargetInstanceFilter, R"*(
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
TEST_F(RelatedClassPathTests, Reverse_CorrectlyReversesMultiStepPathWithTargetInstanceFilter)
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

    RelatedClassPath path{
        RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, "r_ab"), true, SelectClass<ECClass>(*classB, "b")),
        WithTargetInstanceFilter(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relBC, "r_bc"), true, SelectClass<ECClass>(*classC, "c")), "bcbc"),
        WithTargetInstanceFilter(RelatedClass(*classC, SelectClass<ECRelationshipClass>(*relCD, "r_cd"), true, SelectClass<ECClass>(*classD, "d")), "cdcd"),
        RelatedClass(*classD, SelectClass<ECRelationshipClass>(*relDE, "r_de"), true, SelectClass<ECClass>(*classE, "e")),
        };
    ASSERT_EQ(SUCCESS, path.Reverse("a", true));
    ASSERT_EQ(4, path.size());
    EXPECT_EQ(WithTargetInstanceFilter(RelatedClass(*classE, SelectClass<ECRelationshipClass>(*relDE, "r_de"), false, SelectClass<ECClass>(*classD, "d")), "cdcd"), path[0]);
    EXPECT_EQ(WithTargetInstanceFilter(RelatedClass(*classD, SelectClass<ECRelationshipClass>(*relCD, "r_cd"), false, SelectClass<ECClass>(*classC, "c")), "bcbc"), path[1]);
    EXPECT_EQ(RelatedClass(*classC, SelectClass<ECRelationshipClass>(*relBC, "r_bc"), false, SelectClass<ECClass>(*classB, "b")), path[2]);
    EXPECT_EQ(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, "r_ab"), false, SelectClass<ECClass>(*classA, "a")), path[3]);
    }

//---------------------------------------------------------------------------------------
// @betest
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
        RelatedClass(*classAA, SelectClass<ECRelationshipClass>(*relBA, "r_ba"), false, SelectClass<ECClass>(*classB, "b")),
        RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relBCD, "r_bcd"), true, SelectClass<ECClass>(*classC, "c")),
        RelatedClass(*classC, SelectClass<ECRelationshipClass>(*relCDE, "r_cde"), true, SelectClass<ECClass>(*classE, "e")),
        };
    RelatedClassPath path2{
        RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relBA, "r_ba"), false, SelectClass<ECClass>(*classB, "b")),
        RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relBCD, "r_bcd"), true, SelectClass<ECClass>(*classD, "d")),
        RelatedClass(*classD, SelectClass<ECRelationshipClass>(*relCDE, "r_cde"), true, SelectClass<ECClass>(*classE, "e")),
        };
    RelatedClassPath result;
    ASSERT_EQ(SUCCESS, RelatedClassPath::Unify(result, path1, path2));
    ASSERT_EQ(3, result.size());
    EXPECT_EQ(RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relBA, "r_ba"), false, SelectClass<ECClass>(*classB, "b")), result[0]);
    EXPECT_EQ(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relBCD, "r_bcd"), true, SelectClass<ECClass>(*classCD, "c")), result[1]);
    EXPECT_EQ(RelatedClass(*classCD, SelectClass<ECRelationshipClass>(*relCDE, "r_cde"), true, SelectClass<ECClass>(*classE, "e")), result[2]);
    }
