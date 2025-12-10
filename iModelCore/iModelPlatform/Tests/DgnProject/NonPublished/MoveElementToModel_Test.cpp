/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_DPTEST

class MoveElementToModelTests : public DgnDbTestFixture
    {
public:
    DgnModelPtr sourceModel;
    DgnModelId sourceModelId;
    DgnModelId targetModelId;
    DgnModelPtr targetModel;
    DgnCategoryId categoryId;
    DgnElementCPtr firstElement;
    DgnClassId parentRelClassId;

    void SetUp() override
        {
        DgnDbTestFixture::SetUp();
        SetupSeedProject(BeSQLite::Db::OpenMode::ReadWrite, true /*wantStandalone*/);

        sourceModel = DgnDbTestUtils::InsertPhysicalModel(*m_db, "SourceModel");
        ASSERT_TRUE(sourceModel.IsValid());
        sourceModelId = sourceModel->GetModelId();

        targetModel = DgnDbTestUtils::InsertPhysicalModel(*m_db, "TargetModel");
        ASSERT_TRUE(targetModel.IsValid());
        targetModelId = targetModel->GetModelId();

        categoryId = DgnDbTestUtils::GetFirstSpatialCategoryId(*m_db);
        ASSERT_TRUE(categoryId.IsValid());

        parentRelClassId = m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements);
        ASSERT_TRUE(parentRelClassId.IsValid());

        // Create a simple element in the source model
        TestElementPtr element = TestElement::Create(*m_db, sourceModelId, categoryId, DgnCode());
        element->SetUserLabel("FirstElement");

        firstElement = m_db->Elements().Insert(*element);
        ASSERT_TRUE(firstElement.IsValid());
        }

    DgnElementCPtr CreateChildElement(DgnElementId parentId, Utf8CP label = nullptr, DgnCodeCP codeValue = nullptr)
        {
        TestElementPtr child = TestElement::Create(*m_db, sourceModelId, categoryId, codeValue == nullptr ? DgnCode() : *codeValue);
        child->SetUserLabel(label);
        child->SetParentId(parentId, parentRelClassId);
        return m_db->Elements().Insert(*child);
        }

    DgnElementCPtr CreateElementWithCode(DgnModelId modelId, CodeSpecId codeSpecId, DgnElementId scopeElementId, Utf8CP label = nullptr, Utf8CP codeValue = nullptr)
        {
        const auto code = DgnCode::CreateWithDbContext(*m_db, codeSpecId, scopeElementId, codeValue);
        TestElementPtr element = TestElement::Create(*m_db, modelId, categoryId, code);
        if (label != nullptr)
            element->SetUserLabel(label);
        return m_db->Elements().Insert(*element);
        }

    CodeSpecPtr CreateModelScopedCodeSpec(Utf8CP name)
        {
        CodeSpecPtr codeSpec = CodeSpec::Create(*m_db, name, CodeScopeSpec::CreateModelScope());
        if (!codeSpec.IsValid())
            return nullptr;
        DgnDbStatus stat = codeSpec->Insert();
        if (stat != DgnDbStatus::Success)
            return nullptr;
        return codeSpec;
        }

    DgnDbStatus MoveToTargetModel(DgnElementCR element)
        {
        return m_db->Elements().MoveElementToModel(element, targetModelId);
        }

    void VerifyInSourceModel(std::vector<DgnElementId> elementIds, Utf8CP context = nullptr)
        {
        for (auto const& elementId : elementIds)
            {
            DgnElementCPtr element = m_db->Elements().GetElement(elementId);
            ASSERT_TRUE(element.IsValid()) << "Element " << elementId.ToString().c_str() << " does not exist. " << context;
            EXPECT_EQ(sourceModelId, element->GetModelId()) << "Element " << elementId.ToString().c_str() << " not in source model. " << context;
            }
        }

    void VerifyInTargetModel(std::vector<DgnElementId> elementIds, Utf8CP context = nullptr)
        {
        for (auto const& elementId : elementIds)
            {
            DgnElementCPtr element = m_db->Elements().GetElement(elementId);
            ASSERT_TRUE(element.IsValid()) << "Element " << elementId.ToString().c_str() << " does not exist. " << context;
            EXPECT_EQ(targetModelId, element->GetModelId()) << "Element " << elementId.ToString().c_str() << " not in target model. " << context;
            }
        }
    };

TEST_F(MoveElementToModelTests, SimpleElementMoveAcrossModels)
    {
    const DgnElementId elementId = firstElement->GetElementId();

    VerifyInSourceModel({elementId}, "Before move");

    EXPECT_EQ(DgnDbStatus::Success, MoveToTargetModel(*firstElement));

    VerifyInTargetModel({elementId}, "After move");
    EXPECT_STREQ("FirstElement", m_db->Elements().GetElement(elementId)->GetUserLabel());
    }

TEST_F(MoveElementToModelTests, MoveElementWithChildren)
    {
    const DgnElementId parentId = firstElement->GetElementId();

    // Create hierarchy: parent -> child1, child2 -> grandchild
    DgnElementCPtr child1 = CreateChildElement(parentId, "Child1");
    ASSERT_TRUE(child1.IsValid());
    const DgnElementId child1Id = child1->GetElementId();
    
    DgnElementCPtr child2 = CreateChildElement(parentId, "Child2");
    ASSERT_TRUE(child2.IsValid());
    const DgnElementId child2Id = child2->GetElementId();

    DgnElementCPtr grandchild = CreateChildElement(child1Id, "Grandchild1");
    ASSERT_TRUE(grandchild.IsValid());
    const DgnElementId grandchildId = grandchild->GetElementId();

    VerifyInSourceModel({parentId, child1Id, child2Id, grandchildId}, "Before move");

    EXPECT_EQ(DgnDbStatus::Success, MoveToTargetModel(*firstElement));

    // Only parent moves; children remain in source model
    VerifyInTargetModel({parentId}, "After move");
    VerifyInSourceModel({child1Id, child2Id, grandchildId}, "After move");

    // Verify parent-child relationships are maintained
    EXPECT_EQ(parentId, m_db->Elements().GetElement(child1Id)->GetParentId());
    EXPECT_EQ(parentId, m_db->Elements().GetElement(child2Id)->GetParentId());
    EXPECT_EQ(child1Id, m_db->Elements().GetElement(grandchildId)->GetParentId());
    }

TEST_F(MoveElementToModelTests, UniqueAspectPreserved)
    {
    const DgnElementId elementId = firstElement->GetElementId();
    ECN::ECClassCP aspectClass = TestUniqueAspect::GetECClass(*m_db);
    ASSERT_TRUE(aspectClass != nullptr);

    // Add unique aspect to element
    DgnElementPtr editElement = firstElement->MakeCopy<DgnElement>();
    DgnElement::UniqueAspect::SetAspect(*editElement, *TestUniqueAspect::Create("UniqueAspect"));
    EXPECT_EQ(DgnDbStatus::Success, m_db->Elements().Update(*editElement));

    EXPECT_EQ(DgnDbStatus::Success, MoveToTargetModel(*m_db->Elements().GetElement(elementId)));

    // Verify aspect preserved after move
    TestUniqueAspectCPtr aspectAfter = DgnElement::UniqueAspect::Get<TestUniqueAspect>(*m_db->Elements().GetElement(elementId), *aspectClass);
    ASSERT_TRUE(aspectAfter != nullptr);
    EXPECT_STREQ("UniqueAspect", aspectAfter->GetTestUniqueAspectProperty().c_str());
    }

TEST_F(MoveElementToModelTests, MultiAspectPreserved)
    {
    const DgnElementId elementId = firstElement->GetElementId();
    ECN::ECClassCP aspectClass = TestMultiAspect::GetECClass(*m_db);
    ASSERT_TRUE(aspectClass != nullptr);

    // Add multi aspect to element
    DgnElementPtr editElement = firstElement->MakeCopy<DgnElement>();
    TestMultiAspectPtr aspect = TestMultiAspect::Create("MultiAspect");
    DgnElement::MultiAspect::AddAspect(*editElement, *aspect);
    EXPECT_EQ(DgnDbStatus::Success, m_db->Elements().Update(*editElement));

    EXPECT_EQ(DgnDbStatus::Success, MoveToTargetModel(*m_db->Elements().GetElement(elementId)));

    // Verify aspect preserved after move
    TestMultiAspectCPtr aspectAfter = DgnElement::MultiAspect::Get<TestMultiAspect>(*m_db->Elements().GetElement(elementId), *aspectClass, aspect->GetAspectInstanceId());
    ASSERT_TRUE(aspectAfter != nullptr);
    EXPECT_STREQ("MultiAspect", aspectAfter->GetTestMultiAspectProperty().c_str());
    }

TEST_F(MoveElementToModelTests, DeepHierarchyMove)
    {
    /* Create a deep hierarchy: parent -> child1 -> grandchild1 -> greatGrandChild1
                                       -> child2 -> grandchild2
    */
    const DgnElementId parentId = firstElement->GetElementId();
    DgnElementCPtr child1 = CreateChildElement(parentId, "Child1");
    ASSERT_TRUE(child1.IsValid());
    const DgnElementId child1Id = child1->GetElementId();

    DgnElementCPtr child2 = CreateChildElement(parentId, "Child2");
    ASSERT_TRUE(child2.IsValid());
    const DgnElementId child2Id = child2->GetElementId();

    DgnElementCPtr grandchild1 = CreateChildElement(child1Id, "Grandchild1");
    ASSERT_TRUE(grandchild1.IsValid());
    const DgnElementId grandchild1Id = grandchild1->GetElementId();

    DgnElementCPtr grandchild2 = CreateChildElement(child2Id, "Grandchild2");
    ASSERT_TRUE(grandchild2.IsValid());
    const DgnElementId grandchild2Id = grandchild2->GetElementId();

    DgnElementCPtr greatGrandchild1 = CreateChildElement(grandchild1Id, "GreatGrandchild1");
    ASSERT_TRUE(greatGrandchild1.IsValid());
    const DgnElementId greatGrandchild1Id = greatGrandchild1->GetElementId();

    // Move grandchild1 to target
    EXPECT_EQ(DgnDbStatus::Success, MoveToTargetModel(*m_db->Elements().GetElement(grandchild1Id)));
    VerifyInSourceModel({parentId, child1Id, child2Id, grandchild2Id, greatGrandchild1Id}, "After first move");
    VerifyInTargetModel({grandchild1Id}, "After first move");

    // Move remaining elements one by one
    for (auto elementId : {greatGrandchild1Id, grandchild2Id, child1Id, child2Id})
        EXPECT_EQ(DgnDbStatus::Success, MoveToTargetModel(*m_db->Elements().GetElement(elementId)));

    VerifyInSourceModel({parentId}, "After all moves");
    VerifyInTargetModel({child1Id, child2Id, grandchild1Id, grandchild2Id, greatGrandchild1Id}, "After all moves");
    }

TEST_F(MoveElementToModelTests, MoveToSameModelIsNoop)
    {
    EXPECT_EQ(DgnDbStatus::Success, m_db->Elements().MoveElementToModel(*firstElement, sourceModelId));
    VerifyInSourceModel({firstElement->GetElementId()});
    }

TEST_F(MoveElementToModelTests, MoveToInvalidModelFails)
    {
    DgnModelId invalidModelId;
    EXPECT_EQ(DgnDbStatus::InvalidId, m_db->Elements().MoveElementToModel(*firstElement, invalidModelId));
    VerifyInSourceModel({firstElement->GetElementId()});
    }

TEST_F(MoveElementToModelTests, CodePreservedAfterMove)
    {
    CodeSpecPtr codeSpec = CreateModelScopedCodeSpec("TestCodeSpec");
    DgnElementCPtr element = CreateElementWithCode(sourceModelId, codeSpec->GetCodeSpecId(), sourceModel->GetModeledElementId(), "ElementLabel", "UniqueCode123");
    ASSERT_TRUE(element.IsValid());
    DgnCode originalCode = element->GetCode();

    EXPECT_EQ(DgnDbStatus::Success, MoveToTargetModel(*element));

    // Verify code is preserved
    DgnElementCPtr movedElement = m_db->Elements().GetElement(element->GetElementId());
    EXPECT_EQ(originalCode.GetCodeSpecId(), movedElement->GetCode().GetCodeSpecId());
    EXPECT_STREQ(originalCode.GetValue().GetUtf8CP(), movedElement->GetCode().GetValue().GetUtf8CP());
    }

TEST_F(MoveElementToModelTests, CacheInvalidatedAfterMove)
    {
    const DgnElementId elementId = firstElement->GetElementId();

    VerifyInSourceModel({elementId}, "Before move");
    EXPECT_EQ(DgnDbStatus::Success, MoveToTargetModel(*firstElement));
    VerifyInTargetModel({elementId}, "After move");
    }

TEST_F(MoveElementToModelTests, RollbackWhenTargetModelRejectsInsert)
    {
    // Create non-compatible target models
    const DgnModelPtr definitionModel = DgnDbTestUtils::InsertDefinitionModel(*m_db, "DefinitionModel");
    ASSERT_TRUE(definitionModel.IsValid());

    DgnDbTestUtils::InsertDrawingCategory(*m_db, "TestDrawingCategory");
    DocumentListModelPtr drawingListModel = DgnDbTestUtils::InsertDocumentListModel(*m_db, "DrawingListModel");
    DrawingPtr drawing = DgnDbTestUtils::InsertDrawing(*drawingListModel, "TestDrawing");
    const DrawingModelPtr drawingModel = DgnDbTestUtils::InsertDrawingModel(*drawing);
    drawingModel->FillRangeIndex();

    // Create element hierarchy
    const DgnElementId parentId = firstElement->GetElementId();
    DgnElementCPtr child1 = CreateChildElement(parentId, "Child1");
    ASSERT_TRUE(child1.IsValid());
    const DgnElementId child1Id = child1->GetElementId();

    DgnElementCPtr child2 = CreateChildElement(parentId, "Child2");
    ASSERT_TRUE(child2.IsValid());
    const DgnElementId child2Id = child2->GetElementId();

    DgnElementCPtr grandchild = CreateChildElement(child1Id, "Grandchild1");
    ASSERT_TRUE(grandchild.IsValid());
    const DgnElementId grandchildId = grandchild->GetElementId();

    m_db->SaveChanges();

    VerifyInSourceModel({parentId, child1Id, child2Id, grandchildId}, "Before failed move");

    // Attempt move to DefinitionModel - should fail (TestElement is Geometric3d)
    EXPECT_EQ(DgnDbStatus::WrongModel, m_db->Elements().MoveElementToModel(*firstElement, definitionModel->GetModelId()));
    VerifyInSourceModel({parentId, child1Id, child2Id, grandchildId}, "After failed move to DefinitionModel");

    // Attempt move to DrawingModel - should fail (TestElement is 3d, not 2d)
    EXPECT_EQ(DgnDbStatus::Mismatch2d3d, m_db->Elements().MoveElementToModel(*firstElement, drawingModel->GetModelId()));
    VerifyInSourceModel({parentId, child1Id, child2Id, grandchildId}, "After failed move to DrawingModel");
    }

TEST_F(MoveElementToModelTests, ModelScopedCodeConflictWithRootElement)
    {
    CodeSpecPtr codeSpec = CreateModelScopedCodeSpec("ModelScopedCodeSpec");

    // Create elements in both models with same code value
    DgnElementCPtr element1 = CreateElementWithCode(sourceModelId, codeSpec->GetCodeSpecId(), sourceModel->GetModeledElementId(), "Element1", "DuplicateCode");
    ASSERT_TRUE(element1.IsValid());
    DgnElementCPtr element2 = CreateElementWithCode(targetModelId, codeSpec->GetCodeSpecId(), targetModel->GetModeledElementId(), "Element2", "DuplicateCode");
    ASSERT_TRUE(element2.IsValid());

    // Attempt move - should fail due to code conflict
    EXPECT_EQ(DgnDbStatus::DuplicateCode, m_db->Elements().MoveElementToModel(*element1, targetModelId));

    // Verify elements remain in their original models
    VerifyInSourceModel({element1->GetElementId()});
    VerifyInTargetModel({element2->GetElementId()});
    }

TEST_F(MoveElementToModelTests, ModelScopedCodeConflictWithChildElement)
    {
    CodeSpecPtr codeSpec = CreateModelScopedCodeSpec("ModelScopedCodeSpec");

    // Create parent with child that has code "DuplicateCode"
    DgnElementCPtr parent = CreateElementWithCode(sourceModelId, codeSpec->GetCodeSpecId(), sourceModel->GetModeledElementId(), "Parent", "ParentCode");
    ASSERT_TRUE(parent.IsValid());
    const auto childCode = DgnCode::CreateWithDbContext(*m_db, codeSpec->GetCodeSpecId(), parent->GetElementId(), "DuplicateCode");
    
    DgnElementCPtr child = CreateChildElement(parent->GetElementId(), "Child", &childCode);
    ASSERT_TRUE(child.IsValid());
    const DgnElementId childId = child->GetElementId();

    // Create element in target model with conflicting code
    DgnElementCPtr targetElement = CreateElementWithCode(targetModelId, codeSpec->GetCodeSpecId(), targetModel->GetModeledElementId(), "TargetElement", "DuplicateCode");
    ASSERT_TRUE(targetElement.IsValid());

    // Move parent (child stays in source) - should succeed since parent itself has no conflict
    EXPECT_EQ(DgnDbStatus::Success, m_db->Elements().MoveElementToModel(*parent, targetModelId));

    // Verify: parent moved, child and targetElement remain
    VerifyInSourceModel({childId});
    VerifyInTargetModel({parent->GetElementId(), targetElement->GetElementId()});
    }

TEST_F(MoveElementToModelTests, UndoRedoMoveElement)
    {
    const DgnElementId parentId = firstElement->GetElementId();
    DgnElementCPtr child1 = CreateChildElement(parentId, "Child1");
    ASSERT_TRUE(child1.IsValid());
    const DgnElementId child1Id = child1->GetElementId();

    DgnElementCPtr child2 = CreateChildElement(parentId, "Child2");
    ASSERT_TRUE(child2.IsValid());
    const DgnElementId child2Id = child2->GetElementId();

    DgnElementCPtr grandchild = CreateChildElement(child1->GetElementId(), "Grandchild1");
    ASSERT_TRUE(grandchild.IsValid());
    const DgnElementId grandchildId = grandchild->GetElementId();

    VerifyInSourceModel({parentId, child1Id, child2Id, grandchildId}, "Before move");
    m_db->SaveChanges("Insert test elements");

    EXPECT_EQ(DgnDbStatus::Success, MoveToTargetModel(*firstElement));
    m_db->SaveChanges("Move parent to target model");

    VerifyInTargetModel({parentId}, "After move");
    VerifyInSourceModel({child1Id, child2Id, grandchildId}, "After move");

    EXPECT_EQ(DgnDbStatus::Success, m_db->Txns().ReverseSingleTxn());
    VerifyInSourceModel({parentId, child1Id, child2Id, grandchildId}, "After undo");

    EXPECT_EQ(DgnDbStatus::Success, m_db->Txns().ReinstateTxn());
    VerifyInTargetModel({parentId}, "After redo");
    VerifyInSourceModel({child1Id, child2Id, grandchildId}, "After redo");
    }
