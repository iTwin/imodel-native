/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_DPTEST

struct MoveElementToModelTests : public DgnDbTestFixture
    {
    DgnModelPtr sourceModel;
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

        targetModel = DgnDbTestUtils::InsertPhysicalModel(*m_db, "TargetModel");
        ASSERT_TRUE(targetModel.IsValid());

        categoryId = DgnDbTestUtils::GetFirstSpatialCategoryId(*m_db);
        ASSERT_TRUE(categoryId.IsValid());

        parentRelClassId = m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements);
        ASSERT_TRUE(parentRelClassId.IsValid());

        // Create a simple element in the source model
        TestElementPtr element = TestElement::Create(*m_db, sourceModel->GetModelId(), categoryId, DgnCode());
        element->SetUserLabel("FirstElement");

        firstElement = m_db->Elements().Insert(*element);
        ASSERT_TRUE(firstElement.IsValid());
        }

    DgnElementCPtr CreateChildElement(DgnElementId parentId, Utf8CP label = nullptr, DgnCodeCP codeValue = nullptr)
        {
        TestElementPtr child = TestElement::Create(*m_db, sourceModel->GetModelId(), categoryId, codeValue == nullptr ? DgnCode() : *codeValue);
        child->SetUserLabel(label);
        child->SetParentId(parentId, parentRelClassId);
        return m_db->Elements().Insert(*child);
        }

    void VerifyElementInModel(DgnElementId elementId, DgnModelId expectedModelId, Utf8CP context = nullptr)
        {
        DgnElementCPtr element = m_db->Elements().GetElement(elementId);
        ASSERT_TRUE(element.IsValid()) << "Element with id " << elementId.ToString().c_str() << " does not exist." << (context ? Utf8PrintfString(" Context: %s", context).c_str() : "");
        EXPECT_EQ(expectedModelId, element->GetModelId()) << "Incorrect model id for element with id " << elementId.ToString().c_str() << (context ? Utf8PrintfString(". Context: %s", context).c_str() : "");
        }

    void VerifyElementsInModel(bvector<DgnElementId> const& elementIds, DgnModelId expectedModelId, Utf8CP context = nullptr)
        {
        for (auto const& elementId : elementIds)
            VerifyElementInModel(elementId, expectedModelId, context);
        }

    DgnElementCPtr CreateElementWithCode(DgnModelId modelId, CodeSpecId codeSpecId, DgnElementId scopeElementId, Utf8CP label = nullptr, Utf8CP codeValue = nullptr)
        {
        const auto code = DgnCode::CreateWithDbContext(*m_db, codeSpecId, scopeElementId, codeValue);
        TestElementPtr element = TestElement::Create(*m_db, modelId, categoryId, code);
        if (label != nullptr)
            element->SetUserLabel(label);
        return m_db->Elements().Insert(*element);
        }
    };

TEST_F(MoveElementToModelTests, SimpleElementMoveAcrossModels)
    {
    const DgnElementId elementId = firstElement->GetElementId();

    // Verify element is in source model
    VerifyElementInModel(elementId, sourceModel->GetModelId(), "Before move");

    // Move element to target model
    DgnDbStatus stat;
    auto movedIds = m_db->Elements().MoveElementToModel(*firstElement, targetModel->GetModelId(), stat);
    EXPECT_EQ(DgnDbStatus::Success, stat);

    // Verify element is now in target model
    VerifyElementInModel(elementId, targetModel->GetModelId(), "After move");
    EXPECT_STREQ("FirstElement", m_db->Elements().GetElement(elementId)->GetUserLabel());
    }

TEST_F(MoveElementToModelTests, RejectMoveElementWithParent)
    {
    // Create child element with parent
    DgnElementCPtr childElement = CreateChildElement(firstElement->GetElementId(), "ChildElement");
    ASSERT_TRUE(childElement.IsValid());
    EXPECT_TRUE(childElement->GetParentId().IsValid());

    // Attempt to move child (which has a parent element)
    DgnDbStatus stat;
    m_db->Elements().MoveElementToModel(*childElement, targetModel->GetModelId(), stat);
    EXPECT_EQ(DgnDbStatus::ParentBlockedChange, stat);

    // Verify child is still in source model
    VerifyElementInModel(childElement->GetElementId(), sourceModel->GetModelId(), "After move");
    }

TEST_F(MoveElementToModelTests, MoveElementWithChildren)
    {
    const DgnElementId parentId = firstElement->GetElementId();

    // Create children
    DgnElementCPtr child1Element = CreateChildElement(parentId, "Child1");
    DgnElementCPtr child2Element = CreateChildElement(parentId, "Child2");
    DgnElementCPtr grandchildElement = CreateChildElement(child1Element->GetElementId(), "Grandchild1");

    ASSERT_TRUE(child1Element.IsValid());
    ASSERT_TRUE(child2Element.IsValid());
    ASSERT_TRUE(grandchildElement.IsValid());

    const DgnElementId child1Id = child1Element->GetElementId();
    const DgnElementId child2Id = child2Element->GetElementId();
    const DgnElementId grandchildId = grandchildElement->GetElementId();

    // Verify all elements are in source model
    VerifyElementsInModel({parentId, child1Id, child2Id, grandchildId}, sourceModel->GetModelId(), "Before move - elements should be in source model");

    // Move parent with descendants
    DgnDbStatus stat;
    auto movedIds = m_db->Elements().MoveElementToModel(*firstElement, targetModel->GetModelId(), stat);
    EXPECT_EQ(DgnDbStatus::Success, stat);

    // Verify all elements are now in target model
    VerifyElementsInModel({parentId, child1Id, child2Id, grandchildId}, targetModel->GetModelId(), "After move - elements should be in target model");

    // Verify parent-child relationships are maintained
    DgnElementCPtr movedChild1 = m_db->Elements().GetElement(child1Id);
    DgnElementCPtr movedChild2 = m_db->Elements().GetElement(child2Id);
    DgnElementCPtr movedGrandchild = m_db->Elements().GetElement(grandchildId);
    EXPECT_EQ(parentId, movedChild1->GetParentId());
    EXPECT_EQ(parentId, movedChild2->GetParentId());
    EXPECT_EQ(child1Id, movedGrandchild->GetParentId());
    }

TEST_F(MoveElementToModelTests, UniqueAspectPreserved)
    {
    const DgnElementId elementId = firstElement->GetElementId();

    ECN::ECClassCP aspectClass = TestUniqueAspect::GetECClass(*m_db);
    ASSERT_TRUE(aspectClass != nullptr);

    // Add unique aspect to element
    DgnElementPtr editElement = firstElement->MakeCopy<DgnElement>();
    TestUniqueAspectPtr aspect1 = TestUniqueAspect::Create("UniqueAspect");
    DgnElement::UniqueAspect::SetAspect(*editElement, *aspect1);
    EXPECT_EQ(DgnDbStatus::Success, m_db->Elements().Update(*editElement));

    // Move element
    DgnElementCPtr beforeMove = m_db->Elements().GetElement(elementId);
    DgnDbStatus stat;
    auto movedIds = m_db->Elements().MoveElementToModel(*beforeMove, targetModel->GetModelId(), stat);
    EXPECT_EQ(DgnDbStatus::Success, stat);

    // Verify aspect preserved
    DgnElementCPtr afterMove = m_db->Elements().GetElement(elementId);
    TestUniqueAspectCPtr aspectAfter = DgnElement::UniqueAspect::Get<TestUniqueAspect>(*afterMove, *aspectClass);
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

    // Move element
    DgnElementCPtr beforeMove = m_db->Elements().GetElement(elementId);
    DgnDbStatus stat;
    auto movedIds = m_db->Elements().MoveElementToModel(*beforeMove, targetModel->GetModelId(), stat);
    EXPECT_EQ(DgnDbStatus::Success, stat);

    // Verify aspect preserved
    DgnElementCPtr afterMove = m_db->Elements().GetElement(elementId);
    TestMultiAspectCPtr aspectAfter = DgnElement::MultiAspect::Get<TestMultiAspect>(*afterMove, *aspectClass, aspect->GetAspectInstanceId());
    ASSERT_TRUE(aspectAfter != nullptr);
    EXPECT_STREQ("MultiAspect", aspectAfter->GetTestMultiAspectProperty().c_str());
    }

TEST_F(MoveElementToModelTests, DeepHierarchyMove)
    {
    /* Create a deep hierarchy: parent -> child1 -> grandchild1 -> greatGrandChild1
                                       -> child2 -> grandchild2
    */
    DgnElementCPtr child1Element = CreateChildElement(firstElement->GetElementId());
    DgnElementCPtr child2Element = CreateChildElement(firstElement->GetElementId());
    DgnElementCPtr grandchild1Element = CreateChildElement(child1Element->GetElementId());
    DgnElementCPtr grandchild2Element = CreateChildElement(child2Element->GetElementId());
    DgnElementCPtr greatGrandchild1Element = CreateChildElement(grandchild1Element->GetElementId());

    ASSERT_TRUE(child1Element.IsValid());
    ASSERT_TRUE(child2Element.IsValid());
    ASSERT_TRUE(grandchild1Element.IsValid());
    ASSERT_TRUE(grandchild2Element.IsValid());
    ASSERT_TRUE(greatGrandchild1Element.IsValid());

    // Move root element
    DgnDbStatus stat;
    auto movedIds = m_db->Elements().MoveElementToModel(*firstElement, targetModel->GetModelId(), stat);
    EXPECT_EQ(DgnDbStatus::Success, stat);

    // Verify all descendants moved
    VerifyElementsInModel({
        firstElement->GetElementId(),
        child1Element->GetElementId(),
        child2Element->GetElementId(),
        grandchild1Element->GetElementId(),
        grandchild2Element->GetElementId(),
        greatGrandchild1Element->GetElementId()
    }, targetModel->GetModelId(), "Deep hierarchy move - all elements should be in target model");
    }

TEST_F(MoveElementToModelTests, MoveToSameModelIsNoop)
    {
    // Try to move element to its current model
    DgnDbStatus stat;
    m_db->Elements().MoveElementToModel(*firstElement, sourceModel->GetModelId(), stat);
    EXPECT_EQ(DgnDbStatus::Success, stat);

    // Verify element is still in source model
    VerifyElementInModel(firstElement->GetElementId(), sourceModel->GetModelId(), "After move");
    }

TEST_F(MoveElementToModelTests, MoveToInvalidModelFails)
    {
    DgnModelId invalidModelId;
    DgnDbStatus stat;
    m_db->Elements().MoveElementToModel(*firstElement, invalidModelId, stat);
    EXPECT_EQ(DgnDbStatus::InvalidId, stat);

    // Verify element is still in source model
    VerifyElementInModel(firstElement->GetElementId(), sourceModel->GetModelId(), "After move");
    }

TEST_F(MoveElementToModelTests, CodePreservedAfterMove)
    {
    // Create element with a specific code
    CodeSpecPtr codeSpec = CodeSpec::Create(*m_db, "TestCodeSpec", CodeScopeSpec::CreateModelScope());
    ASSERT_TRUE(codeSpec.IsValid());
    ASSERT_EQ(DgnDbStatus::Success, codeSpec->Insert());

    DgnElementCPtr insertedElement = CreateElementWithCode(sourceModel->GetModelId(), codeSpec->GetCodeSpecId(), sourceModel->GetModeledElementId(), "UniqueCode123");
    ASSERT_TRUE(insertedElement.IsValid());

    DgnCode originalCode = insertedElement->GetCode();

    // Move element
    DgnDbStatus stat;
    auto movedIds = m_db->Elements().MoveElementToModel(*insertedElement, targetModel->GetModelId(), stat);
    EXPECT_EQ(DgnDbStatus::Success, stat);

    // Verify code is preserved
    DgnElementCPtr movedElement = m_db->Elements().GetElement(insertedElement->GetElementId());
    EXPECT_EQ(originalCode.GetCodeSpecId(), movedElement->GetCode().GetCodeSpecId());
    EXPECT_STREQ(originalCode.GetValue().GetUtf8CP(), movedElement->GetCode().GetValue().GetUtf8CP());
    }

TEST_F(MoveElementToModelTests, CacheInvalidatedAfterMove)
    {
    const DgnElementId elementId = firstElement->GetElementId();

    // Load element into cache
    VerifyElementInModel(elementId, sourceModel->GetModelId(), "Before move");

    // Move element
    DgnDbStatus stat;
    auto movedIds = m_db->Elements().MoveElementToModel(*firstElement, targetModel->GetModelId(), stat);
    EXPECT_EQ(DgnDbStatus::Success, stat);

    // Load element again - should get updated version from DB
    VerifyElementInModel(elementId, targetModel->GetModelId(), "After move");
    }

TEST_F(MoveElementToModelTests, RollbackWhenTargetModelRejectsInsert)
    {
    // Create non-compatible target models (DefinitionModel, Geometric2d DrawingModel)
    const DgnModelPtr definitionModel = DgnDbTestUtils::InsertDefinitionModel(*m_db, "DefinitionModel");
    ASSERT_TRUE(definitionModel.IsValid());

    DgnDbTestUtils::InsertDrawingCategory(*m_db, "TestDrawingCategory");
    DocumentListModelPtr drawingListModel = DgnDbTestUtils::InsertDocumentListModel(*m_db, "DrawingListModel");
    DrawingPtr drawing = DgnDbTestUtils::InsertDrawing(*drawingListModel, "TestDrawing");
    
    const DrawingModelPtr drawingModel = DgnDbTestUtils::InsertDrawingModel(*drawing);
    drawingModel->FillRangeIndex();
    
    // Create element hierarchy in the physical model
    DgnElementCPtr child1Element = CreateChildElement(firstElement->GetElementId());
    DgnElementCPtr child2Element = CreateChildElement(firstElement->GetElementId());
    DgnElementCPtr grandchildElement = CreateChildElement(child1Element->GetElementId());
    
    m_db->SaveChanges();
    
    // Get element IDs
    const DgnElementId parentId = firstElement->GetElementId();
    const DgnElementId child1Id = child1Element->GetElementId();
    const DgnElementId child2Id = child2Element->GetElementId();
    const DgnElementId grandchildId = grandchildElement->GetElementId();
    bvector<DgnElementId> allElements = {parentId, child1Id, child2Id, grandchildId};

    // Verify all in source model before attempting move
    VerifyElementsInModel(allElements, sourceModel->GetModelId(), "Before failed move - elements should be in source model");

    // Attempt move to definition model - should fail because TestElement is Geometric3d
    // DefinitionModel's _OnInsertElement will return DgnDbStatus::WrongModel
    DgnDbStatus stat;
    m_db->Elements().MoveElementToModel(*firstElement, definitionModel->GetModelId(), stat);
    EXPECT_EQ(DgnDbStatus::WrongModel, stat);
    
    // Verify ALL elements remain in source model
    VerifyElementsInModel(allElements, sourceModel->GetModelId(), "After failed move to DefinitionModel - elements should remain in source model");

    // Attempt move to drawing model - should fail because TestElement is Geometric3d and not Geometric2d
    // DrawingModel's _OnInsertElement will return DgnDbStatus::Mismatch2d3d
    m_db->Elements().MoveElementToModel(*firstElement, drawingModel->GetModelId(), stat);
    EXPECT_EQ(DgnDbStatus::Mismatch2d3d, stat);
    
    // Verify ALL elements remain in source model
    VerifyElementsInModel(allElements, sourceModel->GetModelId(), "After failed move to DrawingModel - elements should remain in source model");
    }

TEST_F(MoveElementToModelTests, ModelScopedCodeConflictWithRootElement)
    {
    // Create a model-scoped CodeSpec
    CodeSpecPtr modelScopedCodeSpec = CodeSpec::Create(*m_db, "ModelScopedCodeSpec", CodeScopeSpec::CreateModelScope());
    ASSERT_TRUE(modelScopedCodeSpec.IsValid());
    ASSERT_EQ(DgnDbStatus::Success, modelScopedCodeSpec->Insert());

    // Create element1 in source model with code "DuplicateCode"
    DgnElementCPtr insertedElement1 = CreateElementWithCode(sourceModel->GetModelId(), modelScopedCodeSpec->GetCodeSpecId(), sourceModel->GetModeledElementId(), "Element1", "DuplicateCode");
    ASSERT_TRUE(insertedElement1.IsValid());

    // Create an element in the target model with a conflicting code value
    DgnElementCPtr insertedElement2 = CreateElementWithCode(targetModel->GetModelId(), modelScopedCodeSpec->GetCodeSpecId(), targetModel->GetModeledElementId(), "Element2", "DuplicateCode");
    ASSERT_TRUE(insertedElement2.IsValid());

    // Attempt move - should fail with DuplicateCode
    DgnDbStatus stat;
    m_db->Elements().MoveElementToModel(*insertedElement1, targetModel->GetModelId(), stat);
    EXPECT_EQ(DgnDbStatus::DuplicateCode, stat);

    // Verify elements remain in their original models
    VerifyElementInModel(insertedElement1->GetElementId(), sourceModel->GetModelId(), "After move");
    VerifyElementInModel(insertedElement2->GetElementId(), targetModel->GetModelId(), "After move");
    }

TEST_F(MoveElementToModelTests, ModelScopedCodeConflictWithChildElement)
    {
    // Create a model-scoped CodeSpec
    CodeSpecPtr modelScopedCodeSpec = CodeSpec::Create(*m_db, "ModelScopedCodeSpec", CodeScopeSpec::CreateModelScope());
    ASSERT_TRUE(modelScopedCodeSpec.IsValid());
    ASSERT_EQ(DgnDbStatus::Success, modelScopedCodeSpec->Insert());

    // Create parent element with code in source model
    DgnElementCPtr insertedElement1 = CreateElementWithCode(sourceModel->GetModelId(), modelScopedCodeSpec->GetCodeSpecId(), sourceModel->GetModeledElementId(), "Element1", "ParentCode");
    ASSERT_TRUE(insertedElement1.IsValid());

    // Create child with code "DuplicateCode"
    const auto childCode = DgnCode::CreateWithDbContext(*m_db, modelScopedCodeSpec->GetCodeSpecId(), insertedElement1->GetElementId(), "DuplicateCode");
    DgnElementCPtr insertedChild1 = CreateChildElement(insertedElement1->GetElementId(), "DuplicateCode", &childCode);
    ASSERT_TRUE(insertedChild1.IsValid());

    // Create an element in the target model with a conflicting code value
    DgnElementCPtr insertedElement2 = CreateElementWithCode(targetModel->GetModelId(), modelScopedCodeSpec->GetCodeSpecId(), targetModel->GetModeledElementId(), "Element2", "DuplicateCode");
    ASSERT_TRUE(insertedElement2.IsValid());

    // Attempt move - should fail with DuplicateCode
    DgnDbStatus stat;
    m_db->Elements().MoveElementToModel(*insertedElement1, targetModel->GetModelId(), stat);
    EXPECT_EQ(DgnDbStatus::DuplicateCode, stat);

    // Verify all elements remain in their original models
    VerifyElementsInModel({ insertedElement1->GetElementId(), insertedChild1->GetElementId() }, sourceModel->GetModelId(), "After code conflict - elements should remain in source model");
    VerifyElementInModel(insertedElement2->GetElementId(), targetModel->GetModelId(), "After code conflict - conflicting element should remain in target model");
    }

TEST_F(MoveElementToModelTests, UndoRedoMoveElement)
    {
    const DgnElementId elementId = firstElement->GetElementId();
    DgnElementCPtr child1Element = CreateChildElement(elementId, "Child1");
    DgnElementCPtr child2Element = CreateChildElement(elementId, "Child2");
    DgnElementCPtr grandchildElement = CreateChildElement(child1Element->GetElementId(), "Grandchild1");

    // Verify element is in source model
    VerifyElementsInModel({elementId, child1Element->GetElementId(), child2Element->GetElementId(), grandchildElement->GetElementId()}, sourceModel->GetModelId(), "Before move");
    m_db->SaveChanges("Insert test elements");

    // Move element to target model
    DgnDbStatus stat;
    auto movedIds = m_db->Elements().MoveElementToModel(*firstElement, targetModel->GetModelId(), stat);
    EXPECT_EQ(DgnDbStatus::Success, stat);
    m_db->SaveChanges("Move elements to target model");

    // Verify element is now in target model
    VerifyElementsInModel({elementId, child1Element->GetElementId(), child2Element->GetElementId(), grandchildElement->GetElementId()}, targetModel->GetModelId(), "After move");

    // Undo the move
    EXPECT_EQ(m_db->Txns().ReverseSingleTxn(), DgnDbStatus::Success);

    // Verify element is back in source model
    VerifyElementsInModel({elementId, child1Element->GetElementId(), child2Element->GetElementId(), grandchildElement->GetElementId()}, sourceModel->GetModelId(), "After undo");

    // Redo the move
    EXPECT_EQ(m_db->Txns().ReinstateTxn(), DgnDbStatus::Success);

    // Verify element is again in target model
    VerifyElementsInModel({elementId, child1Element->GetElementId(), child2Element->GetElementId(), grandchildElement->GetElementId()}, targetModel->GetModelId(), "After redo");
    }