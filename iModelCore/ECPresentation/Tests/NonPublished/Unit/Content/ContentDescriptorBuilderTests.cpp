/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ContentDescriptorBuilderTests.h"
#include "../../../../Source/Shared/RulesPreprocessor.h"

std::unique_ptr<ECDbTestProject> ContentDescriptorBuilderTests::s_project(nullptr);
DEFINE_SCHEMA_REGISTRY(ContentDescriptorBuilderTests);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptorBuilderTests::SetUp()
    {
    static const RulesetVariables s_emptyVariables;

    ECPresentationTest::SetUp();

    m_connection = m_connections.NotifyConnectionOpened(s_project->GetECDb());
    m_cancellationToken = SimpleCancelationToken::Create();
    m_customFunctions = std::make_unique<CustomFunctionsInjector>(m_connections, *m_connection);
    m_ruleset = PresentationRuleSet::CreateInstance("");
    m_schemaHelper = std::make_unique<ECSchemaHelper>(*m_connection, nullptr, nullptr);
    m_rulesPreprocessor = std::make_unique<RulesPreprocessor>(m_connections, *m_connection, *m_ruleset,
        m_rulesetVariables, nullptr, m_schemaHelper->GetECExpressionsCache());
    m_context = std::make_unique<ContentDescriptorBuilder::Context>(*m_schemaHelper, m_connections, *m_connection, m_cancellationToken.get(), *m_rulesPreprocessor, *m_ruleset,
        ContentDisplayType::Undefined, s_emptyVariables, m_categorySupplier, nullptr, ECPresentation::UnitSystem::Undefined,
        *NavNodeKeyListContainer::Create(), nullptr);
    m_context->SetContentFlagsCalculator([](int defaultFlags) {return defaultFlags | (int)ContentFlags::SkipInstancesCheck;});
    m_descriptorBuilder = std::make_unique<ContentDescriptorBuilder>(*m_context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptorBuilderTests::TearDown()
    {
    s_project->GetECDb().AbandonChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptorBuilderTests::SetUpTestCase()
    {
    s_project = std::make_unique<ECDbTestProject>();
    s_project->Create("ContentDescriptorBuilderTests");
    INIT_SCHEMA_REGISTRY(s_project->GetECDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptorBuilderTests::TearDownTestCase()
    {
    s_project.reset(nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ContentDescriptorBuilderTests::GetSchema()
    {
    return s_project->GetECDb().Schemas().GetSchema(BeTest::GetNameOfCurrentTest());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ContentDescriptorBuilderTests::GetClass(Utf8CP schemaName, Utf8CP className)
    {
    return s_project->GetECDb().Schemas().GetClass(schemaName, className);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ContentDescriptorBuilderTests::GetClass(Utf8CP name)
    {
    return GetClass(BeTest::GetNameOfCurrentTest(), name);
    }

/*---------------------------------------------------------------------------------**//**
* Categories supplier context stores categories in bmap which uses B-tree with 31 value
* in one node. To test functionality when B-tree has more nodes, at least 32 categories
* should be used. Therefore, a default category and 31 category of classB derived
* classes are created.
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentDescriptorBuilderTests, DoesNotIncludeCategoryOfNonExistentField)
    {
    RulesEngineTestHelpers::ImportSchema(s_project->GetECDb(), [&](ECSchemaR schema)
        {
        ECEntityClassP classA = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateEntityClass(classA, "A"));
        ASSERT_TRUE(nullptr != classA);

        ECEntityClassP classB = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateEntityClass(classB, "B"));
        ASSERT_TRUE(nullptr != classB);
        IECInstancePtr classMapCustomAttribute = GetClass("ECDbMap", "ClassMap")->GetDefaultStandaloneEnabler()->CreateInstance();
        classMapCustomAttribute->SetValue("MapStrategy", ECValue("TablePerHierarchy"));
        ASSERT_EQ(ECObjectsStatus::Success, classB->SetCustomAttribute(*classMapCustomAttribute));

        ECRelationshipClassP relAB = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, schema.CreateRelationshipClass(relAB, "A_B", *classA, "Source", *classB, "Target"));

        bvector<ECEntityClassP> derivedClasses = RulesEngineTestHelpers::CreateNDerivedClasses(schema, *classB, 31);
        for (int i = 0; i < 31; i++)
            {
            if (i == 21) // Class22 category will be stored in the root node of the B-tree
                continue;

            PrimitiveECPropertyP prop = nullptr;
            ASSERT_EQ(ECObjectsStatus::Success, derivedClasses[i]->CreatePrimitiveProperty(prop, "Label", PRIMITIVETYPE_String));
            }
        });

    ECClassCP classA = GetClass("A");
    ECRelationshipClassCP relAB = GetClass("A_B")->GetRelationshipClassCP();

    for (int i = 0; i < 31; i++)
        {
        ECClassCP classDerivedFromB = GetClass(Utf8PrintfString("Class%d", i + 1).c_str());
        IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
        IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classDerivedFromB);
        RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b);
        }

    ContentInstancesOfSpecificClassesSpecification spec(1, "", classA->GetFullName(), false, false);
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(*new RelationshipPathSpecification(*new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection_Forward)),
        {new PropertySpecification("*")}, RelationshipMeaning::RelatedInstance, true));

    ContentDescriptorCPtr descriptor = GetDescriptorBuilder().CreateDescriptor(spec);
    ASSERT_TRUE(descriptor.IsValid());
    EXPECT_EQ(31, descriptor->GetCategories().size()); // default category, 30 categories of classes derived from classB (all except Class22)
    auto categories = descriptor->GetCategories();
    EXPECT_EQ(categories.end(), std::find_if(categories.begin(), categories.end(), [](std::shared_ptr<ContentDescriptor::Category const> const category){return category->GetName() == "Class22";}));
    }
