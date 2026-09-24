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
    PresentationRuleSetPtr m_ruleset;
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
        m_ruleset = PresentationRuleSet::CreateInstance("");
        m_category = std::make_shared<ContentDescriptor::Category>("test", "Test", "", 0);
        }

    ContentDescriptorPtr CreateEmptyDescriptor() const
        {
        return ContentDescriptor::Create(*m_connection, *m_ruleset, RulesetVariables(), *NavNodeKeyListContainer::Create());
        }

    ContentDescriptor::CalculatedPropertyField* CreateCalculatedField(Utf8CP name, Utf8CP label = nullptr) const
        {
        auto field = new ContentDescriptor::CalculatedPropertyField(m_category, label ? label : name, name, "", PRIMITIVETYPE_String, nullptr);
        field->SetUniqueName(field->CreateName());
        return field;
        }

    ContentDescriptor::RelatedContentField* CreateRelatedContentField(bvector<ContentDescriptor::Field*> fields) const
        {
        ECClassCP sourceClass = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ECClassDef");
        ECClassCP targetClass = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ECSchemaDef");
        ECRelationshipClassCP relationship = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "SchemaOwnsClasses")->GetRelationshipClassCP();
        auto field = new ContentDescriptor::RelatedContentField(m_category, "Related",
            { RelatedClass(*sourceClass, SelectClass<ECRelationshipClass>(*relationship, ""), false, SelectClass<ECClass>(*targetClass, "")) },
            fields);
        field->SetUniqueName(field->CreateName());
        return field;
        }

    static void CollectFieldNames(bvector<Utf8String>& names, bvector<ContentDescriptor::Field*> const& fields)
        {
        for (auto field : fields)
            {
            names.push_back(field->GetUniqueName());
            if (field->IsNestedContentField())
                CollectFieldNames(names, field->AsNestedContentField()->GetFields());
            }
        }

    static bool HasUniqueFieldNames(ContentDescriptorCR descriptor)
        {
        bvector<Utf8String> names;
        CollectFieldNames(names, descriptor.GetAllFields());
        return names.size() == std::set<Utf8String>(names.begin(), names.end()).size();
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

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ContentDescriptorTests, FieldClonesAreValid)
    {
    ECClassCP testClass1 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ECPropertyDef");
    ECClassCP testClass2 = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ECClassDef");
    ECRelationshipClassCP testRelationship = m_connection->GetECDb().Schemas().GetClass("ECDbMeta", "ClassOwnsLocalProperties")->GetRelationshipClassCP();

    auto category = std::make_shared<ContentDescriptor::Category>("test", "Test", "", 0);

    auto nestedField = CreateCalculatedField("a");
    nestedField->SetRenderer(new ContentFieldRenderer("a"));
    nestedField->SetEditor(new ContentFieldEditor("a"));

    auto parentField = new ContentDescriptor::RelatedContentField(category, "AB",
        { RelatedClass(*testClass1, SelectClass<ECRelationshipClass>(*testRelationship, ""), false, SelectClass<ECClass>(*testClass2, "")) },
        { nestedField });
    parentField->SetRenderer(new ContentFieldRenderer("b"));
    parentField->SetEditor(new ContentFieldEditor("b"));

    auto clonedParent = parentField->Clone();
    DELETE_AND_CLEAR(parentField);

    // ensure we can convert clone to json
    clonedParent->AsJson();

    // ensure pointers are valid
    EXPECT_STREQ("b", clonedParent->GetRenderer()->GetName().c_str());
    EXPECT_STREQ("b", clonedParent->GetEditor()->GetName().c_str());

    auto clonedChild = clonedParent->AsNestedContentField()->GetFields().at(0);
    EXPECT_EQ(clonedParent, clonedChild->GetParent());
    EXPECT_STREQ("a", clonedChild->GetRenderer()->GetName().c_str());
    EXPECT_STREQ("a", clonedChild->GetEditor()->GetName().c_str());

    DELETE_AND_CLEAR(clonedParent);
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ContentDescriptorTests, MergeWith_RenamesIncomingNestedFieldWhenNameCollidesWithRootField)
    {
    auto descriptor = CreateEmptyDescriptor();
    descriptor->AddRootField(*CreateCalculatedField("x", "Root"));

    auto other = CreateEmptyDescriptor();
    other->AddRootField(*CreateRelatedContentField({ CreateCalculatedField("x", "Nested") }));
    Utf8String relatedFieldName = other->GetAllFields()[0]->GetUniqueName();

    descriptor->MergeWith(*other);

    EXPECT_TRUE(HasUniqueFieldNames(*descriptor));
    ASSERT_EQ(2, descriptor->GetAllFields().size());
    EXPECT_STREQ("x", descriptor->GetAllFields()[0]->GetUniqueName().c_str());
    auto mergedRelatedField = descriptor->GetAllFields()[1]->AsNestedContentField();
    EXPECT_STREQ(relatedFieldName.c_str(), mergedRelatedField->GetUniqueName().c_str());
    EXPECT_STREQ("x/2", mergedRelatedField->GetFields()[0]->GetUniqueName().c_str());
    EXPECT_EQ(mergedRelatedField, mergedRelatedField->GetFields()[0]->GetParent());

    // source descriptor is not modified
    EXPECT_STREQ("x", other->GetAllFields()[0]->AsNestedContentField()->GetFields()[0]->GetUniqueName().c_str());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ContentDescriptorTests, MergeWith_RenamesCollidingNestedContentFieldAndItsChildren)
    {
    auto descriptor = CreateEmptyDescriptor();
    descriptor->AddRootField(*CreateRelatedContentField({ CreateCalculatedField("y", "A") }));
    Utf8String relatedFieldName = descriptor->GetAllFields()[0]->GetUniqueName();

    auto other = CreateEmptyDescriptor();
    other->AddRootField(*CreateRelatedContentField({ CreateCalculatedField("y", "B") }));

    descriptor->MergeWith(*other);

    EXPECT_TRUE(HasUniqueFieldNames(*descriptor));
    ASSERT_EQ(2, descriptor->GetAllFields().size());
    EXPECT_STREQ(relatedFieldName.c_str(), descriptor->GetAllFields()[0]->GetUniqueName().c_str());
    EXPECT_STREQ("y", descriptor->GetAllFields()[0]->AsNestedContentField()->GetFields()[0]->GetUniqueName().c_str());
    EXPECT_STREQ(Utf8PrintfString("%s/2", relatedFieldName.c_str()).c_str(), descriptor->GetAllFields()[1]->GetUniqueName().c_str());
    EXPECT_STREQ("y/2", descriptor->GetAllFields()[1]->AsNestedContentField()->GetFields()[0]->GetUniqueName().c_str());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ContentDescriptorTests, MergeWith_DoesntAssignNameUsedByAnotherIncomingField)
    {
    auto descriptor = CreateEmptyDescriptor();
    descriptor->AddRootField(*CreateCalculatedField("x", "A"));

    auto other = CreateEmptyDescriptor();
    other->AddRootField(*CreateCalculatedField("x", "B"));
    auto suffixedField = CreateCalculatedField("x", "C");
    suffixedField->SetUniqueName("x/2");
    other->AddRootField(*suffixedField);

    descriptor->MergeWith(*other);

    EXPECT_TRUE(HasUniqueFieldNames(*descriptor));
    ASSERT_EQ(3, descriptor->GetAllFields().size());
    EXPECT_STREQ("x", descriptor->GetAllFields()[0]->GetUniqueName().c_str());
    EXPECT_STREQ("x/3", descriptor->GetAllFields()[1]->GetUniqueName().c_str());
    EXPECT_STREQ("x/2", descriptor->GetAllFields()[2]->GetUniqueName().c_str());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ContentDescriptorTests, MergeWith_DoesntDuplicateFieldRenamedByPreviousMerge)
    {
    auto descriptor = CreateEmptyDescriptor();
    descriptor->AddRootField(*CreateCalculatedField("x", "Root"));

    auto other1 = CreateEmptyDescriptor();
    other1->AddRootField(*CreateRelatedContentField({ CreateCalculatedField("x", "Nested") }));
    descriptor->MergeWith(*other1);

    auto other2 = CreateEmptyDescriptor();
    other2->AddRootField(*CreateRelatedContentField({ CreateCalculatedField("x", "Nested") }));
    descriptor->MergeWith(*other2);

    EXPECT_TRUE(HasUniqueFieldNames(*descriptor));
    ASSERT_EQ(2, descriptor->GetAllFields().size());
    EXPECT_STREQ("x/2", descriptor->GetAllFields()[1]->AsNestedContentField()->GetFields()[0]->GetUniqueName().c_str());
    }
