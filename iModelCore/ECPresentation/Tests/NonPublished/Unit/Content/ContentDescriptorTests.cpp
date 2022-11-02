/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentation/Content.h>
#include "../../Helpers/TestHelpers.h"

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentDescriptorTests : ::testing::Test
    {
    static std::unique_ptr<ECDbTestProject> s_project;
    IConnectionCPtr m_connection;
    std::shared_ptr<ContentDescriptor::Category> m_category;

    static void SetUpTestCase()
        {
        s_project = std::make_unique<ECDbTestProject>();
        s_project->Create("ContentDescriptorTests");
        }

    static void TearDownTestCase()
        {
        s_project = nullptr;
        }

    void SetUp() override
        {
        m_connection = new TestConnection(s_project->GetECDb());
        m_category = std::make_shared<ContentDescriptor::Category>("test", "Test", "", 0);
        }

    ContentDescriptorPtr CreateEmptyDescriptor() const
        {
        return ContentDescriptor::Create(*m_connection, *PresentationRuleSet::CreateInstance(""), RulesetVariables(), *NavNodeKeyListContainer::Create());
        }

    ContentDescriptor::CalculatedPropertyField* CreateCalculatedField(Utf8CP name) const
        {
        auto field = new ContentDescriptor::CalculatedPropertyField(m_category, name, name, "", nullptr);
        field->SetUniqueName(field->CreateName());
        return field;
        }

    static bvector<ContentDescriptor::Field const*> GetFields(ContentDescriptor::NestedContentField const& field)
        {
        return ContainerHelpers::TransformContainer<bvector<ContentDescriptor::Field const*>>(field.GetFields());
        }

    static bvector<ContentDescriptor::Field const*> GetFields(ContentDescriptorCR descriptor)
        {
        return ContainerHelpers::TransformContainer<bvector<ContentDescriptor::Field const*>>(descriptor.GetAllFields());
        }

    static Utf8String CreateFieldsStructureStr(bvector<ContentDescriptor::Field const*> const& fields)
        {
        Utf8String str;
        for (auto field : fields)
            {
            if (!str.empty())
                str.append(", ");
            str.append(field->GetLabel());
            if (field->IsNestedContentField())
                str.append("(").append(CreateFieldsStructureStr(GetFields(*field->AsNestedContentField()))).append(")");
            }
        return str;
        }
    static Utf8String CreateFieldsStructureStr(ContentDescriptorCR descriptor)
        {
        return CreateFieldsStructureStr(GetFields(descriptor));
        }
    };
std::unique_ptr<ECDbTestProject> ContentDescriptorTests::s_project;

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ContentDescriptorTests, ExclusivelyIncludeFields_WithRootFields)
    {
    auto descriptor = CreateEmptyDescriptor();
    descriptor->AddRootField(*CreateCalculatedField("a"));
    descriptor->AddRootField(*CreateCalculatedField("b"));
    descriptor->AddRootField(*CreateCalculatedField("c"));
    descriptor->AddRootField(*CreateCalculatedField("d"));

    descriptor->ExclusivelyIncludeFields({
        descriptor->FindField(NamedContentFieldMatcher("a")),
        descriptor->FindField(NamedContentFieldMatcher("c"))
        });
    EXPECT_STREQ("a, c", CreateFieldsStructureStr(*descriptor).c_str());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ContentDescriptorTests, ExclusivelyIncludeFields_WithNestedField)
    {
    ECClassCP testClass1 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ECClassDef");
    ECClassCP testClass2 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ECSchemaDef");
    ECRelationshipClassCP testRelationship = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "SchemaOwnsClasses")->GetRelationshipClassCP();

    auto descriptor = CreateEmptyDescriptor();
    auto category = std::make_shared<ContentDescriptor::Category>("test", "Test", "", 0);
    descriptor->AddRootField(*new ContentDescriptor::RelatedContentField(category, "AB",
        { RelatedClass(*testClass1, SelectClass<ECRelationshipClass>(*testRelationship, ""), false, SelectClass<ECClass>(*testClass2, ""))},
        {
        CreateCalculatedField("a"),
        CreateCalculatedField("b")
        }));
    descriptor->AddRootField(*CreateCalculatedField("c"));

    descriptor->ExclusivelyIncludeFields({
        descriptor->FindField(NamedContentFieldMatcher("a"))
        });
    EXPECT_STREQ("AB(a)", CreateFieldsStructureStr(*descriptor).c_str());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ContentDescriptorTests, ExclusivelyIncludeFields_WithSiblingNestedFields)
    {
    ECClassCP testClass1 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ECClassDef");
    ECClassCP testClass2 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ECSchemaDef");
    ECRelationshipClassCP testRelationship = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "SchemaOwnsClasses")->GetRelationshipClassCP();

    auto descriptor = CreateEmptyDescriptor();
    auto category = std::make_shared<ContentDescriptor::Category>("test", "Test", "", 0);
    descriptor->AddRootField(*new ContentDescriptor::RelatedContentField(category, "ABC",
        { RelatedClass(*testClass1, SelectClass<ECRelationshipClass>(*testRelationship, ""), false, SelectClass<ECClass>(*testClass2, "")) },
        {
        CreateCalculatedField("a"),
        CreateCalculatedField("b"),
        CreateCalculatedField("c")
        }));
    descriptor->AddRootField(*CreateCalculatedField("d"));

    descriptor->ExclusivelyIncludeFields({
        descriptor->FindField(NamedContentFieldMatcher("a")),
        descriptor->FindField(NamedContentFieldMatcher("c"))
        });
    EXPECT_STREQ("ABC(a, c)", CreateFieldsStructureStr(*descriptor).c_str());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ContentDescriptorTests, ExclusivelyIncludeFields_WithDeeplyNestedField)
    {
    ECClassCP testClass1 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ECPropertyDef");
    ECClassCP testClass2 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ECClassDef");
    ECClassCP testClass3 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ECSchemaDef");
    ECRelationshipClassCP testRelationship1 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ClassOwnsLocalProperties")->GetRelationshipClassCP();
    ECRelationshipClassCP testRelationship2 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "SchemaOwnsClasses")->GetRelationshipClassCP();

    auto descriptor = CreateEmptyDescriptor();
    auto category = std::make_shared<ContentDescriptor::Category>("test", "Test", "", 0);
    descriptor->AddRootField(*new ContentDescriptor::RelatedContentField(category, "ABC",
        { RelatedClass(*testClass1, SelectClass<ECRelationshipClass>(*testRelationship1, ""), false, SelectClass<ECClass>(*testClass2, "")) },
        {
        new ContentDescriptor::RelatedContentField(category, "AB",
            { RelatedClass(*testClass2, SelectClass<ECRelationshipClass>(*testRelationship2, ""), false, SelectClass<ECClass>(*testClass3, "")) },
            {
            CreateCalculatedField("a"),
            CreateCalculatedField("b")
            }),
        CreateCalculatedField("c")
        }));
    descriptor->AddRootField(*CreateCalculatedField("d"));

    descriptor->ExclusivelyIncludeFields({
        descriptor->FindField(NamedContentFieldMatcher("a"))
        });
    EXPECT_STREQ("ABC(AB(a))", CreateFieldsStructureStr(*descriptor).c_str());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ContentDescriptorTests, ExcludeFields_RootField_WithoutSiblingsLeft)
    {
    auto descriptor = CreateEmptyDescriptor();
    descriptor->AddRootField(*CreateCalculatedField("a"));

    descriptor->ExcludeFields({
        descriptor->FindField(NamedContentFieldMatcher("a"))
        });
    EXPECT_STREQ("", CreateFieldsStructureStr(*descriptor).c_str());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ContentDescriptorTests, ExcludeFields_RootField_WithSiblingsLeft)
    {
    auto descriptor = CreateEmptyDescriptor();
    descriptor->AddRootField(*CreateCalculatedField("a"));
    descriptor->AddRootField(*CreateCalculatedField("b"));

    descriptor->ExcludeFields({
        descriptor->FindField(NamedContentFieldMatcher("a"))
        });
    EXPECT_STREQ("b", CreateFieldsStructureStr(*descriptor).c_str());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ContentDescriptorTests, ExclusivelyIncludeFields_NestedField_WithoutSiblingsLeft)
    {
    ECClassCP testClass1 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ECClassDef");
    ECClassCP testClass2 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ECSchemaDef");
    ECRelationshipClassCP testRelationship = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "SchemaOwnsClasses")->GetRelationshipClassCP();

    auto descriptor = CreateEmptyDescriptor();
    auto category = std::make_shared<ContentDescriptor::Category>("test", "Test", "", 0);
    descriptor->AddRootField(*new ContentDescriptor::RelatedContentField(category, "ABC",
        { RelatedClass(*testClass1, SelectClass<ECRelationshipClass>(*testRelationship, ""), false, SelectClass<ECClass>(*testClass2, "")) },
        {
        CreateCalculatedField("a"),
        CreateCalculatedField("b"),
        CreateCalculatedField("c")
        }));
    descriptor->AddRootField(*CreateCalculatedField("d"));

    descriptor->ExcludeFields({
        descriptor->FindField(NamedContentFieldMatcher("a")),
        descriptor->FindField(NamedContentFieldMatcher("b")),
        descriptor->FindField(NamedContentFieldMatcher("c"))
        });
    EXPECT_STREQ("d", CreateFieldsStructureStr(*descriptor).c_str());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ContentDescriptorTests, ExclusivelyIncludeFields_NestedField_WithSiblingsLeft)
    {
    ECClassCP testClass1 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ECClassDef");
    ECClassCP testClass2 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ECSchemaDef");
    ECRelationshipClassCP testRelationship = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "SchemaOwnsClasses")->GetRelationshipClassCP();

    auto descriptor = CreateEmptyDescriptor();
    auto category = std::make_shared<ContentDescriptor::Category>("test", "Test", "", 0);
    descriptor->AddRootField(*new ContentDescriptor::RelatedContentField(category, "ABC",
        { RelatedClass(*testClass1, SelectClass<ECRelationshipClass>(*testRelationship, ""), false, SelectClass<ECClass>(*testClass2, "")) },
        {
        CreateCalculatedField("a"),
        CreateCalculatedField("b"),
        CreateCalculatedField("c")
        }));
    descriptor->AddRootField(*CreateCalculatedField("d"));

    descriptor->ExcludeFields({
        descriptor->FindField(NamedContentFieldMatcher("a")),
        descriptor->FindField(NamedContentFieldMatcher("c"))
        });
    EXPECT_STREQ("ABC(b), d", CreateFieldsStructureStr(*descriptor).c_str());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ContentDescriptorTests, ExclusivelyIncludeFields_DeeplyNestedField_WithoutSiblingsLeft_WithoutParentSibling)
    {
    ECClassCP testClass1 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ECPropertyDef");
    ECClassCP testClass2 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ECClassDef");
    ECClassCP testClass3 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ECSchemaDef");
    ECRelationshipClassCP testRelationship1 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ClassOwnsLocalProperties")->GetRelationshipClassCP();
    ECRelationshipClassCP testRelationship2 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "SchemaOwnsClasses")->GetRelationshipClassCP();

    auto descriptor = CreateEmptyDescriptor();
    auto category = std::make_shared<ContentDescriptor::Category>("test", "Test", "", 0);
    descriptor->AddRootField(*new ContentDescriptor::RelatedContentField(category, "ABC",
        { RelatedClass(*testClass1, SelectClass<ECRelationshipClass>(*testRelationship1, ""), false, SelectClass<ECClass>(*testClass2, "")) },
        {
        new ContentDescriptor::RelatedContentField(category, "AB",
            { RelatedClass(*testClass2, SelectClass<ECRelationshipClass>(*testRelationship2, ""), false, SelectClass<ECClass>(*testClass3, "")) },
            {
            CreateCalculatedField("a"),
            CreateCalculatedField("b")
            }),
        CreateCalculatedField("c")
        }));
    descriptor->AddRootField(*CreateCalculatedField("d"));

    descriptor->ExcludeFields({
        descriptor->FindField(NamedContentFieldMatcher("a")),
        descriptor->FindField(NamedContentFieldMatcher("b")),
        descriptor->FindField(NamedContentFieldMatcher("c"))
        });
    EXPECT_STREQ("d", CreateFieldsStructureStr(*descriptor).c_str());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ContentDescriptorTests, ExclusivelyIncludeFields_DeeplyNestedField_WithoutSiblingsLeft_WithParentSibling)
    {
    ECClassCP testClass1 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ECPropertyDef");
    ECClassCP testClass2 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ECClassDef");
    ECClassCP testClass3 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ECSchemaDef");
    ECRelationshipClassCP testRelationship1 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ClassOwnsLocalProperties")->GetRelationshipClassCP();
    ECRelationshipClassCP testRelationship2 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "SchemaOwnsClasses")->GetRelationshipClassCP();

    auto descriptor = CreateEmptyDescriptor();
    auto category = std::make_shared<ContentDescriptor::Category>("test", "Test", "", 0);
    descriptor->AddRootField(*new ContentDescriptor::RelatedContentField(category, "ABC",
        { RelatedClass(*testClass1, SelectClass<ECRelationshipClass>(*testRelationship1, ""), false, SelectClass<ECClass>(*testClass2, "")) },
        {
        new ContentDescriptor::RelatedContentField(category, "AB",
            { RelatedClass(*testClass2, SelectClass<ECRelationshipClass>(*testRelationship2, ""), false, SelectClass<ECClass>(*testClass3, "")) },
            {
            CreateCalculatedField("a"),
            CreateCalculatedField("b")
            }),
        CreateCalculatedField("c")
        }));
    descriptor->AddRootField(*CreateCalculatedField("d"));

    descriptor->ExcludeFields({
        descriptor->FindField(NamedContentFieldMatcher("a")),
        descriptor->FindField(NamedContentFieldMatcher("b"))
        });
    EXPECT_STREQ("ABC(c), d", CreateFieldsStructureStr(*descriptor).c_str());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ContentDescriptorTests, ExclusivelyIncludeFields_DeeplyNestedField_WithSiblingsLeft)
    {
    ECClassCP testClass1 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ECPropertyDef");
    ECClassCP testClass2 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ECClassDef");
    ECClassCP testClass3 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ECSchemaDef");
    ECRelationshipClassCP testRelationship1 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ClassOwnsLocalProperties")->GetRelationshipClassCP();
    ECRelationshipClassCP testRelationship2 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "SchemaOwnsClasses")->GetRelationshipClassCP();

    auto descriptor = CreateEmptyDescriptor();
    auto category = std::make_shared<ContentDescriptor::Category>("test", "Test", "", 0);
    descriptor->AddRootField(*new ContentDescriptor::RelatedContentField(category, "ABC",
        { RelatedClass(*testClass1, SelectClass<ECRelationshipClass>(*testRelationship1, ""), false, SelectClass<ECClass>(*testClass2, "")) },
        {
        new ContentDescriptor::RelatedContentField(category, "AB",
            { RelatedClass(*testClass2, SelectClass<ECRelationshipClass>(*testRelationship2, ""), false, SelectClass<ECClass>(*testClass3, "")) },
            {
            CreateCalculatedField("a"),
            CreateCalculatedField("b")
            }),
        CreateCalculatedField("c")
        }));
    descriptor->AddRootField(*CreateCalculatedField("d"));

    descriptor->ExcludeFields({
        descriptor->FindField(NamedContentFieldMatcher("a"))
        });
    EXPECT_STREQ("ABC(AB(b), c), d", CreateFieldsStructureStr(*descriptor).c_str());
    }
