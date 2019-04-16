/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_DPTEST

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      01/15
+===============+===============+===============+===============+===============+======*/
struct CategoryTests : public DgnDbTestFixture
    {
    void CompareCategories(DgnCategoryId catId, Utf8CP name, DgnCategory::Rank rank, Utf8CP descr)
        {
        DgnCategoryCPtr cat = DgnCategory::Get(*m_db, catId);
        EXPECT_TRUE(cat.IsValid());
        if (cat.IsValid())
            CompareCategories(*cat, name, rank, descr);
        }

    void CompareCategories(DgnCategoryId catId, DgnCategoryCR other)
        {
        DgnCategoryCPtr cat = DgnCategory::Get(*m_db, catId);
        EXPECT_TRUE(cat.IsValid());
        if (cat.IsValid())
            CompareCategories(*cat, other);
        }

    void CompareCategories(DgnCategoryCR cat, DgnCategoryCR other)
        {
        CompareCategories(cat, other.GetCategoryName().c_str(), other.GetRank(), other.GetDescription());
        }

    void CompareCategories(DgnCategoryCR cat, Utf8CP name, DgnCategory::Rank rank, Utf8CP descr)
        {
        EXPECT_STREQ(name, cat.GetCategoryName().c_str());
        EXPECT_EQ(rank, cat.GetRank());
        EXPECT_STREQ(descr, cat.GetDescription());
        }

    void CompareSubCategories(DgnSubCategoryId subcatId, DgnSubCategoryCR other)
        {
        DgnSubCategoryCPtr subcat = DgnSubCategory::Get(*m_db, subcatId);
        EXPECT_TRUE(subcat.IsValid());
        if (subcat.IsValid())
            {
            EXPECT_STREQ(subcat->GetSubCategoryName().c_str(), other.GetSubCategoryName().c_str());
            EXPECT_EQ(subcat->GetCategoryId(), other.GetCategoryId());
            EXPECT_EQ(subcat->GetCode().GetScopeElementId(*m_db), other.GetCode().GetScopeElementId(*m_db));
            EXPECT_EQ(subcat->GetDescription(), other.GetDescription());
            EXPECT_TRUE(subcat->GetAppearance().IsEqual(other.GetAppearance()));
            }
        }
    };

//=======================================================================================
//! Test for inserting categories and checking their properties
// @bsiclass                                                     Maha Nasir      07/15
//=======================================================================================
TEST_F (CategoryTests, InsertCategory)
    {
    SetupSeedProject();
    ASSERT_TRUE(m_db.IsValid());
        
    //Category properties.
    Utf8CP cat_name = "Test Category";
    Utf8CP cat_desc = "This is a test category.";

    DefinitionModelR dictionary = m_db->GetDictionaryModel();
    SpatialCategory category(dictionary, cat_name, DgnCategory::Rank::Domain, cat_desc);

    //Appearence properties.
    uint32_t weight = 10;
    double trans = 0.5;
    uint32_t dp = 1;

    DgnSubCategory::Appearance appearence;
    appearence.SetInvisible (false);
    appearence.SetColor (ColorDef::DarkRed ());
    appearence.SetWeight (weight);
    appearence.SetTransparency (trans);
    appearence.SetDisplayPriority (dp);

    DgnCategoryCPtr pCategory = category.Insert(appearence);
    ASSERT_TRUE(pCategory.IsValid());

    //Verifying category properties
    CompareCategories(category, cat_name, DgnCategory::Rank::Domain, cat_desc);
    EXPECT_TRUE (category.GetCategoryId ().IsValid ());
    EXPECT_FALSE (pCategory->IsSystemCategory ());
    EXPECT_FALSE (pCategory->IsUserCategory ());
    CompareCategories(*pCategory, category);

    DgnCategoryId id = SpatialCategory::QueryCategoryId(dictionary, cat_name);
    EXPECT_TRUE (id.IsValid ());
    EXPECT_EQ(id, category.GetCategoryId());
    EXPECT_EQ(id, pCategory->GetCategoryId());

    DgnCategoryCPtr query = DgnCategory::Get(*m_db, id);
    EXPECT_TRUE (query.IsValid ());

    //Inserts Category 2
    Utf8CP cat2_name = "Test Category 2";
    Utf8CP cat2_desc = "This is test category 2.";

    SpatialCategory category2(dictionary, cat2_name, DgnCategory::Rank::System, cat2_desc);
    SpatialCategoryCPtr pCategory2 = category2.Insert(appearence);
    ASSERT_TRUE(pCategory2.IsValid());

    //Inserts Category 3
    Utf8CP cat3_name = "Test Category 3";
    Utf8CP cat3_desc = "This is test category 3.";

    SpatialCategory category3(dictionary, cat3_name, DgnCategory::Rank::User, cat3_desc);
    SpatialCategoryCPtr pCategory3 = category3.Insert(appearence);
    ASSERT_TRUE(pCategory3.IsValid());

    //Inserts Category 4
    Utf8CP cat4_name = "Test Category 4";
    Utf8CP cat4_desc = "This is test category 4.";

    DrawingCategory category4(dictionary, cat4_name, DgnCategory::Rank::User, cat4_desc);
    DrawingCategoryCPtr pCategory4 = category4.Insert(appearence);
    ASSERT_TRUE(pCategory4.IsValid());

    //Iterator for categories.
    DgnCategoryIdSet spatialCategoryIds = SpatialCategory::MakeIterator(*m_db).BuildIdSet<DgnCategoryId>();
    EXPECT_EQ(4, spatialCategoryIds.size());
    DgnCategoryIdSet drawingCategoryIds = DrawingCategory::MakeIterator(*m_db).BuildIdSet<DgnCategoryId>();
    EXPECT_EQ(1, drawingCategoryIds.size());
    int nCompared = 0;
    int nNotCompared = 0;
    for (auto const& catId : spatialCategoryIds)
        {
        DgnCategory const* pCompareTo = nullptr;
        if (category.GetCategoryId() == catId)
            pCompareTo = &category;
        else if (category2.GetCategoryId() == catId)
            pCompareTo = &category2;
        else if (category3.GetCategoryId() == catId)
            pCompareTo = &category3;
        else if (category4.GetCategoryId() == catId)
            pCompareTo = &category4;

        if (nullptr != pCompareTo)
            {
            CompareCategories(catId, *pCompareTo);
            ++nCompared;
            }
        else
            {
            ++nNotCompared;
            }
        }

    EXPECT_EQ(1, nNotCompared);
    EXPECT_EQ(3, nCompared);
    
    // Ordered List verification
    int count = 0;
    DgnCategoryIdList orderedList = SpatialCategory::MakeIterator(*m_db, nullptr, "ORDER BY [CodeValue]").BuildIdList<DgnCategoryId>();
    
    DgnCategoryId lastId;
    for (DgnCategoryId id : orderedList)
        {
        if (lastId.IsValid())
            {
            SpatialCategoryCPtr current = SpatialCategory::Get(*m_db, id);
            SpatialCategoryCPtr lastCategory = SpatialCategory::Get(*m_db, lastId);
            EXPECT_TRUE(current->GetCode().GetValueUtf8().CompareTo( lastCategory->GetCode().GetValueUtf8().c_str()) > 0);
            ++count;
            }
        lastId = id;
        }
    EXPECT_EQ(4, orderedList.size());
    }

//=======================================================================================
//! Test for Deleting a category.
// @bsiclass                                                     Maha Nasir      07/15
//=======================================================================================
TEST_F (CategoryTests, DeleteCategory)
    {
    SetupSeedProject();

    Utf8CP name = "TestCategory";
    Utf8CP desc = "This is a test category.";

    DefinitionModelR dictionary = m_db->GetDictionaryModel();
    SpatialCategory category(dictionary, name, DgnCategory::Rank::Domain, desc);

    //Appearence properties.
    uint32_t weight = 10;
    double trans = 0.5;
    uint32_t dp = 1;

    DgnSubCategory::Appearance appearence;
    appearence.SetInvisible (false);
    appearence.SetColor (ColorDef::DarkRed ());
    appearence.SetWeight (weight);
    appearence.SetTransparency (trans);
    appearence.SetDisplayPriority (dp);

    //Inserts a category
    DgnCategoryCPtr pCat = category.Insert(appearence);
    ASSERT_TRUE(pCat.IsValid());
    DgnCategoryId id = SpatialCategory::QueryCategoryId(dictionary, name);
    EXPECT_TRUE(id.IsValid());

    // Deletion of a category is not supported.
    DgnDbStatus dlt = pCat->Delete();
    EXPECT_EQ(DgnDbStatus::DeletionProhibited, dlt);
    DgnCategoryId id1 = SpatialCategory::QueryCategoryId(dictionary, name);
    EXPECT_TRUE(id1.IsValid());
    }

//=======================================================================================
//! Test for Updating a category.
// @bsiclass                                                     Maha Nasir      07/15
//=======================================================================================
TEST_F (CategoryTests, UpdateCategory)
    {
    SetupSeedProject();

    //Category properties.
    Utf8CP name = "TestCategory";
    Utf8CP desc = "This is a test category.";

    DefinitionModelR dictionary = m_db->GetDictionaryModel();
    SpatialCategory category(dictionary, name, DgnCategory::Rank::Domain, desc);

    //Appearence properties.
    uint32_t weight = 10;
    double trans = 0.5;
    uint32_t dp = 1;

    DgnSubCategory::Appearance appearence;
    appearence.SetInvisible (false);
    appearence.SetColor (ColorDef::DarkRed ());
    appearence.SetWeight (weight);
    appearence.SetTransparency (trans);
    appearence.SetDisplayPriority (dp);

    //Inserts a category
    EXPECT_TRUE(category.Insert(appearence).IsValid());
    DgnCategoryId id = category.GetCategoryId();
    EXPECT_TRUE(id.IsValid ());

    //Utf8CP u_name = "UpdatedTestCategory";
    Utf8CP u_desc = "This is the updated test category.";

    //Updates category.
    DgnCategoryPtr toFind = m_db->Elements().GetForEdit<DgnCategory>(id);
    EXPECT_TRUE(toFind.IsValid());
    toFind->SetDescription(Utf8String(u_desc));
    DgnDbStatus updateStatus;
    toFind->Update(&updateStatus);
    ASSERT_EQ(DgnDbStatus::Success, updateStatus);
    
    //Verification of category properties
    SpatialCategoryCPtr updatedCat = SpatialCategory::Get(*m_db, id);
    EXPECT_TRUE(updatedCat.IsValid());
    EXPECT_STREQ(u_desc, updatedCat->GetDescription());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CategoryTests, IterateCategories)
    {
    SetupSeedProject();
    int numCategories = SpatialCategory::MakeIterator(*m_db).BuildIdSet<DgnCategoryId>().size();
    DgnDbTestUtils::InsertSpatialCategory(*m_db, "TestCategory1");
    DgnDbTestUtils::InsertSpatialCategory(*m_db, "TestCategory2");
    DgnDbTestUtils::InsertSpatialCategory(*m_db, "TestCategory3");
    numCategories += 3;
    ASSERT_EQ(numCategories, SpatialCategory::MakeIterator(*m_db).BuildIdList<DgnCategoryId>().size());

    bool foundCategory1=false;
    bool foundCategory2=false;
    bool foundCategory3=false;

    for (ElementIteratorEntryCR entry : SpatialCategory::MakeIterator(*m_db))
        {
        DgnCategoryId categoryId = entry.GetId<DgnCategoryId>();
        DgnCategoryCPtr category = SpatialCategory::Get(*m_db, categoryId);
        ASSERT_TRUE(category.IsValid());
        ASSERT_EQ(entry.GetClassId(), category->GetElementClassId());
        ASSERT_EQ(entry.GetModelId(), category->GetModelId());
        ASSERT_STREQ(entry.GetCodeValue(), category->GetCode().GetValueUtf8CP());
        ASSERT_FALSE(entry.GetParentId().IsValid());
        ASSERT_FALSE(category->GetParentId().IsValid());

        if (0 == strcmp(entry.GetCodeValue(), "TestCategory1"))
            {
            foundCategory1 = true;
            ASSERT_EQ(1, DgnSubCategory::MakeIterator(*m_db, categoryId).BuildIdSet<DgnSubCategoryId>().size());
            }
        else if (0 == strcmp(entry.GetCodeValue(), "TestCategory2")) 
            {
            foundCategory2 = true;
            ASSERT_EQ(1, DgnSubCategory::MakeIterator(*m_db, categoryId).BuildIdSet<DgnSubCategoryId>().size());
            }
        else if (0 == strcmp(entry.GetCodeValue(), "TestCategory3")) 
            {
            foundCategory3 = true;
            ASSERT_EQ(1, DgnSubCategory::MakeIterator(*m_db, categoryId).BuildIdSet<DgnSubCategoryId>().size());
            }
        }

    ASSERT_TRUE(foundCategory1);
    ASSERT_TRUE(foundCategory2);
    ASSERT_TRUE(foundCategory3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    07/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CategoryTests, ChangeElementCategory)
    {
    SetupSeedProject();
    DgnCategoryId spatialCategoryId1 = DgnDbTestUtils::InsertSpatialCategory(*m_db, "MySpatialCategory1");
    DgnCategoryId spatialCategoryId2 = DgnDbTestUtils::InsertSpatialCategory(*m_db, "MySpatialCategory2");
    DgnCategoryId drawingCategoryId = DgnDbTestUtils::InsertDrawingCategory(*m_db, "MyDrawingCategory");
    PhysicalModelPtr physicalModel = DgnDbTestUtils::InsertPhysicalModel(*m_db, "MyPhysicalModel");

    GenericPhysicalObjectPtr element = GenericPhysicalObject::Create(*physicalModel, spatialCategoryId1);
    ASSERT_TRUE(element.IsValid());
    ASSERT_TRUE(element->Insert().IsValid());
    ASSERT_EQ(spatialCategoryId1.GetValue(), element->GetCategoryId().GetValue());

    ASSERT_EQ(DgnDbStatus::Success, element->SetCategoryId(spatialCategoryId2));
    ASSERT_EQ(spatialCategoryId2.GetValue(), element->GetCategoryId().GetValue());

    ASSERT_NE(DgnDbStatus::Success, element->SetCategoryId(drawingCategoryId));
    ASSERT_EQ(spatialCategoryId2.GetValue(), element->GetCategoryId().GetValue());
    }
	
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    06/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CategoryTests, ValidateCategoryClass)
    {
    PhysicalModelPtr physicalModel = DgnDbTestUtils::InsertPhysicalModel(*m_db, "MyPhysicalModel");
    DgnCategoryId spatialCategoryId = DgnDbTestUtils::InsertSpatialCategory(*m_db, "MySpatialCategory");
    DocumentListModelPtr drawingListModel = DgnDbTestUtils::InsertDocumentListModel(GetDgnDb(), "MyDrawingListModel");
    DrawingPtr drawing = DgnDbTestUtils::InsertDrawing(*drawingListModel, "MyDrawing");
    DrawingModelPtr drawingModel = DgnDbTestUtils::InsertDrawingModel(*drawing);
    DgnCategoryId drawingCategoryId = DgnDbTestUtils::InsertDrawingCategory(*m_db, "MyDrawingCategory");

    GenericPhysicalObjectPtr physicalElement = GenericPhysicalObject::Create(*physicalModel, drawingCategoryId);
    EXPECT_TRUE(physicalElement.IsValid());
    DgnDbStatus insertStatus;
    EXPECT_FALSE(physicalElement->Insert(&insertStatus).IsValid()) << "Should not be able to insert a PhysicalElement with a DrawingCategory";
    EXPECT_EQ(DgnDbStatus::InvalidCategory, insertStatus);
    EXPECT_EQ(DgnDbStatus::Success, physicalElement->SetCategoryId(spatialCategoryId));
    EXPECT_TRUE(physicalElement->Insert().IsValid());

    DrawingGraphicPtr drawingGraphic = DrawingGraphic::Create(*drawingModel, spatialCategoryId);
    EXPECT_TRUE(drawingGraphic.IsValid());
    EXPECT_FALSE(drawingGraphic->Insert(&insertStatus).IsValid()) << "Should not be able to insert a DrawingGraphic with a SpatialCategory";
    EXPECT_EQ(DgnDbStatus::InvalidCategory, insertStatus);
    EXPECT_EQ(DgnDbStatus::Success, drawingGraphic->SetCategoryId(drawingCategoryId));
    EXPECT_TRUE(drawingGraphic->Insert().IsValid());
    }

//=======================================================================================
//! Test for inserting SubCategories and checking their properties.
// @bsiclass                                                     Maha Nasir      07/15
//=======================================================================================
TEST_F (CategoryTests, InsertSubCategory)
    {
    SetupSeedProject();

    Utf8CP name = "TestCategory";
    Utf8CP desc = "This is a test category.";

    DefinitionModelR dictionary = m_db->GetDictionaryModel();
    SpatialCategory category(dictionary, name, DgnCategory::Rank::Domain, desc);

    //Appearence properties.
    uint32_t weight = 10;
    double trans = 0.5;
    uint32_t dp = 1;

    DgnSubCategory::Appearance appearence;
    appearence.SetInvisible (false);
    appearence.SetColor (ColorDef::DarkRed ());
    appearence.SetWeight (weight);
    appearence.SetTransparency (trans);
    appearence.SetDisplayPriority (dp);
    appearence.SetDontLocate(true);
    appearence.SetDontPlot(true);
    appearence.SetDontSnap(true);
    appearence.SetDisplayPriority(1);
    // TODO: Set line style 
    
    //Inserts a category
    EXPECT_TRUE(category.Insert(appearence).IsValid());
    DgnCategoryId categoryId = category.GetCategoryId();

    Utf8CP sub_name = "Test SubCategory";
    Utf8CP sub_desc = "This is a test subcategory";
    DgnSubCategory subcategory(DgnSubCategory::CreateParams(*m_db, categoryId, sub_name, appearence, sub_desc));
    
    //Inserts a subcategory
    EXPECT_TRUE(subcategory.Insert().IsValid());
    DgnCode code = subcategory.GetCode();

    //Verifying appearence properties
    DgnSubCategory::Appearance app = subcategory.GetAppearance ();
    EXPECT_EQ (ColorDef::DarkRed (), app.GetColor ());
    EXPECT_EQ (dp, app.GetDisplayPriority ());
    EXPECT_EQ (trans, app.GetTransparency ());
    EXPECT_EQ (weight, app.GetWeight ());
    EXPECT_FALSE (app.IsInvisible ());
    EXPECT_TRUE(app.IsEqual(appearence));

    //Verifying subcategory properties
    DgnSubCategoryId subcat_id = DgnSubCategory::QuerySubCategoryId(*m_db, code);
    EXPECT_TRUE(subcat_id.IsValid());

    DgnSubCategoryCPtr query_sub = DgnSubCategory::Get(*m_db, subcat_id);
    EXPECT_TRUE (query_sub.IsValid ());

    DgnSubCategoryId default_subId = DgnCategory::GetDefaultSubCategoryId(categoryId);
    EXPECT_EQ(categoryId.GetValue()+1, default_subId.GetValue());

    //Inserts sub category 2
    Utf8CP sub2_name = "Test SubCategory 2";
    Utf8CP sub2_desc = "This is a test subcategory 2";

    DgnSubCategory subcategory2(DgnSubCategory::CreateParams(*m_db, categoryId, sub2_name, appearence, sub2_desc));
    EXPECT_TRUE(subcategory2.Insert().IsValid());

    //Inserts sub category 3
    Utf8CP sub3_name = "Test SubCategory 3";
    Utf8CP sub3_desc = "This is a test subcategory 3";

    DgnSubCategory subcategory3(DgnSubCategory::CreateParams(*m_db, categoryId, sub3_name, appearence, sub3_desc));
    EXPECT_TRUE(subcategory3.Insert().IsValid());

    EXPECT_EQ(4, (int)category.QuerySubCategoryCount());

    //Iterator for subcategories.
    ElementIterator iterator = category.MakeSubCategoryIterator();
    EXPECT_EQ(4, iterator.BuildIdSet<DgnElementId>().size());
    EXPECT_EQ(5, DgnSubCategory::QueryCount(*m_db)); // + default sub-category of category created by v8 converter
    EXPECT_EQ(4, DgnSubCategory::QueryCount(*m_db, categoryId));

    int nCompared = 0;
    int nNotCompared = 0;
    for (ElementIteratorEntryCR subCategoryEntry : iterator)
        {
        DgnSubCategoryId subCategoryId = subCategoryEntry.GetId<DgnSubCategoryId>();
        DgnSubCategoryCP pCompareTo = nullptr;

        if (subcategory.GetSubCategoryId() == subCategoryId)
            pCompareTo = &subcategory;
        else if (subcategory2.GetSubCategoryId() == subCategoryId)
            pCompareTo = &subcategory2;
        else if (subcategory3.GetSubCategoryId() == subCategoryId)
            pCompareTo = &subcategory3;

        if (nullptr != pCompareTo)
            {
            ++nCompared;
            CompareSubCategories(subCategoryId, *pCompareTo);
            }
        else
            {
            ++nNotCompared;
            }
        }

    EXPECT_EQ(3, nCompared);
    EXPECT_EQ(1, nNotCompared); // default sub-category
    SaveDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CategoryTests, SubCategoryInvariants)
    {
    SetupSeedProject();
    DgnDbR db = *m_db;

    DefinitionModelR dictionary = db.GetDictionaryModel();
    SpatialCategory cat1(dictionary, "Cat1", DgnCategory::Rank::Domain);
    DgnSubCategory::Appearance app;
    ASSERT_TRUE(cat1.Insert(app).IsValid());
    SpatialCategory cat2(dictionary, "Cat2", DgnCategory::Rank::Domain);
    ASSERT_TRUE(cat2.Insert(app).IsValid());
    DgnCategoryId cat1Id = cat1.GetCategoryId(),
                  cat2Id = cat2.GetCategoryId();

    // default sub-category exists with expected Code + ID
    DgnSubCategoryCPtr defaultSubCat1 = DgnSubCategory::Get(db, DgnCategory::GetDefaultSubCategoryId(cat1Id));
    ASSERT_TRUE(defaultSubCat1.IsValid());
    EXPECT_EQ(defaultSubCat1->GetCode().GetValueUtf8(), "Cat1");
    EXPECT_EQ(defaultSubCat1->GetSubCategoryId(), DgnCategory::GetDefaultSubCategoryId(cat1Id));
    db.SaveChanges();

    // Code validation
    DgnSubCategoryPtr defaultSubCat1Edit = defaultSubCat1->MakeCopy<DgnSubCategory>();

    DgnCode code = DgnSubCategory::CreateCode(db, cat2Id, "Cat2");
    EXPECT_EQ(DgnDbStatus::Success, defaultSubCat1Edit->SetCode(code));
    DgnDbStatus status;
    defaultSubCat1Edit->Update(&status);
    ASSERT_EQ(DgnDbStatus::DuplicateCode, status);
    db.SaveChanges();

    code = DgnSubCategory::CreateCode(db, cat2Id, "Cat1"); // Same category Code doens't effect anything.
    EXPECT_EQ(DgnDbStatus::Success, defaultSubCat1Edit->SetCode(code));
    defaultSubCat1Edit->Update(&status);
    ASSERT_EQ(DgnDbStatus::Success, status);
    db.SaveChanges();

    code = DgnSubCategory::CreateCode(db, cat1Id, "NewName"); // sub-category name must equal category name
    EXPECT_EQ(DgnDbStatus::Success, defaultSubCat1Edit->SetCode(code));
    defaultSubCat1Edit->Update(&status);
    ASSERT_EQ(DgnDbStatus::Success, status);
    db.SaveChanges();

    // Cannot delete default sub-category
    EXPECT_EQ(DgnDbStatus::ParentBlockedChange, defaultSubCat1->Delete());

    // Cannot change parent category
    EXPECT_EQ(DgnDbStatus::InvalidParent, defaultSubCat1Edit->SetParentId(cat2Id, DgnClassId()));

    // require valid parent category
    DgnSubCategory noParent(DgnSubCategory::CreateParams(db, DgnCategoryId(), "NoParent", app, "Sub-category requires valid parent category"));
    EXPECT_TRUE(noParent.Insert(&status).IsNull());
    EXPECT_NE(status, DgnDbStatus::Success);

    DgnSubCategory subcat2A(DgnSubCategory::CreateParams(db, cat2Id, "2A", app));
    DgnSubCategoryCPtr cpSubcat2A = subcat2A.Insert();
    EXPECT_TRUE(cpSubcat2A.IsValid());

    // name collisions
    DgnSubCategory subcat2A_2(DgnSubCategory::CreateParams(db, cat2Id, "2A", app));
    EXPECT_FALSE(subcat2A_2.Insert().IsValid());

    DgnSubCategory subcat2B(DgnSubCategory::CreateParams(db, cat2Id, "2B", app));
    DgnSubCategoryCPtr cpSubcat2B = subcat2B.Insert();
    ASSERT_TRUE(cpSubcat2B.IsValid());

    db.SaveChanges();
    //printf("\n%s, %s\n", pSubcat2B->GetCode().GetValue().c_str(), DgnSubCategory::CreateCode(db, cat2Id, "2A").GetValue().c_str());
    DgnSubCategoryPtr pSubcat2B = cpSubcat2B->MakeCopy<DgnSubCategory>();
    pSubcat2B->SetCode(DgnSubCategory::CreateCode(db, cat2Id, "2A"));
    EXPECT_TRUE(pSubcat2B->Update(&status).IsNull());
    EXPECT_EQ(DgnDbStatus::DuplicateCode, status);

    // Cannot change parent category
    EXPECT_EQ(DgnDbStatus::InvalidParent, pSubcat2B->SetParentId(cat1Id, DgnClassId()));

    // Code validation
    code = DgnSubCategory::CreateCode(db, cat1Id, "2B"); // wrong category
    EXPECT_EQ(DgnDbStatus::Success, pSubcat2B->SetCode(code));
    code = DgnSubCategory::CreateCode(db, cat2Id, "2BNewName");
    EXPECT_EQ(DgnDbStatus::Success, pSubcat2B->SetCode(code));

    // Can rename non-default sub-category if no name collisions
    cpSubcat2B = pSubcat2B->Update(&status);
    EXPECT_EQ(DgnDbStatus::Success, status);
    EXPECT_EQ(0, strcmp(cpSubcat2B->GetCode().GetValueUtf8().c_str(), "2BNewName"));

    // Illegal characters in names
    pSubcat2B = cpSubcat2B->MakeCopy<DgnSubCategory>();
    Utf8String invalidChars = DgnCategory::GetIllegalCharacters();
    for (auto const& invalidChar : invalidChars)
        {
        Utf8String newName("SubCat");
        newName.append(1, invalidChar);
        code = DgnSubCategory::CreateCode(db, cat2Id, newName);
        EXPECT_EQ(DgnDbStatus::Success, pSubcat2B->SetCode(code));

        pSubcat2B->Update(&status);
        ASSERT_EQ(DgnDbStatus::InvalidName, status);
        }

    // create and insert new subCategory with invalid Code, should return InvalidName.
    DgnSubCategory subcatWithInvalidName(DgnSubCategory::CreateParams(db, cat2Id, invalidChars, app));
    EXPECT_TRUE(subcatWithInvalidName.Insert(&status).IsNull());
    EXPECT_EQ(DgnDbStatus::InvalidName, status);
    }

//=======================================================================================
//! Test for Deleting a subcategory.
// @bsiclass                                                     Maha Nasir      07/15
//=======================================================================================
TEST_F (CategoryTests, DeleteSubCategory)
    {
    SetupSeedProject();

    Utf8CP name = "TestCategory";
    Utf8CP desc = "This is a test category.";

    SpatialCategory category(m_db->GetDictionaryModel(), name, DgnCategory::Rank::Domain, desc);

    //Inserts a category.
    DgnSubCategory::Appearance appearence;
    EXPECT_TRUE(category.Insert(appearence).IsValid());
    DgnCategoryId id = category.GetCategoryId();
    EXPECT_TRUE(id.IsValid());

    Utf8CP sub_name = "TestSubCategory";
    Utf8CP sub_desc = "This is a test subcategory";
    DgnSubCategory subcategory(DgnSubCategory::CreateParams(*m_db, id, sub_name, appearence, sub_desc));

    //Inserts subcategory.
    DgnSubCategoryCPtr pSubCat = subcategory.Insert();
    ASSERT_TRUE(pSubCat.IsValid());

    //Verifying subcategory properties
    EXPECT_STREQ ("TestSubCategory", subcategory.GetSubCategoryName().c_str());
    EXPECT_STREQ ("This is a test subcategory", subcategory.GetDescription ());

    EXPECT_EQ(DgnDbStatus::Success, pSubCat->Delete());
    DgnSubCategoryId sub_id = DgnSubCategory::QuerySubCategoryId(*m_db, subcategory.GetCode());
    EXPECT_FALSE (sub_id.IsValid ());
    }

//=======================================================================================
//! Test for Quering a category.using elementID
// @betest                                                     Umar.Hayat      09/15
//=======================================================================================
TEST_F (CategoryTests, QueryByElementId)
    {
    SetupSeedProject();

    //Category properties.
    Utf8CP name = "TestCategory";
    Utf8CP desc = "This is a test category.";

    SpatialCategory category(m_db->GetDictionaryModel(), name, DgnCategory::Rank::Domain, desc);
    DgnSubCategory::Appearance appearence;

    //Inserts a category
    EXPECT_TRUE(category.Insert(appearence).IsValid());
    DgnCategoryId categoryId = category.GetCategoryId();
    EXPECT_TRUE(categoryId.IsValid());

    PhysicalModelPtr model = GetDefaultPhysicalModel();
    DgnElementPtr el = TestElement::Create(*m_db, m_defaultModelId, categoryId, DgnCode());
    GeometrySourceP geomElem = el->ToGeometrySourceP();
    GeometryBuilderPtr builder = GeometryBuilder::Create(*model, categoryId, DPoint3d::From(0.0, 0.0, 0.0));
    DEllipse3d ellipseData = DEllipse3d::From(1, 2, 3,
        0, 0, 2,
        0, 3, 0,
        0.0, Angle::TwoPi());
    ICurvePrimitivePtr ellipse = ICurvePrimitive::CreateArc(ellipseData);
    EXPECT_TRUE(builder->Append(*ellipse));

    EXPECT_EQ(SUCCESS, builder->Finish(*geomElem));
    auto elem = m_db->Elements().Insert(*el);
    EXPECT_TRUE(elem.IsValid());
    }

////=======================================================================================
//// @betest                                                     Umar.Hayat      02/16
////=======================================================================================
//TEST_F (CategoryTests, SubCateOverridesJsonRoundTrip)
//    {
//    DgnSubCategory::Override overrides;
//    overrides.SetColor(ColorDef::Red());
//    overrides.SetDisplayPriority(2);
//    overrides.SetInvisible(false);
//    //overrides.SetMaterial((DgnMaterialId(6));
//    overrides.SetStyle(DgnStyleId());
//    overrides.SetTransparency(0.9);
//    overrides.SetWeight(3);
//
//    Json::Value jsonVal;
//    overrides.ToJson(jsonVal);
//
//    DgnSubCategory::Override overridesFromJson;
//    overridesFromJson.FromJson(jsonVal);
//
//    //EXPECT_TRUE(overrides == overridesFromJson);
//
//    }

//=======================================================================================
//
// @betest                                                     Ridha.Malik      11/16
//=======================================================================================
TEST_F(CategoryTests, UpdateSubCategory_VerifyPresistence)
    {
    SetupSeedProject();
    BeFileName outFileName = (BeFileName)m_db->GetDbFileName();
    BeSQLite::Db::OpenMode mode = BeSQLite::Db::OpenMode::ReadWrite;
    Utf8CP name = "TestCategory";
    Utf8CP desc = "This is a test category.";
    DgnCategoryId categoryId;
    DgnCode  sub2code;
    {
    SpatialCategory category(m_db->GetDictionaryModel(), name, DgnCategory::Rank::Domain, desc);

    //Appearence properties.
    uint32_t weight = 10;
    double trans = 0.5;
    uint32_t dp = 1;

    DgnSubCategory::Appearance appearence;
    appearence.SetInvisible(false);
    appearence.SetWeight(weight);
    appearence.SetColor(ColorDef::White());
    appearence.SetTransparency(trans);
    appearence.SetDisplayPriority(dp);

    //Insert category
    EXPECT_TRUE(category.Insert(appearence).IsValid());
    categoryId = category.GetCategoryId();
    EXPECT_TRUE(categoryId.IsValid());

    PhysicalModelPtr model = GetDefaultPhysicalModel();
    DgnElementPtr el = TestElement::Create(*m_db, m_defaultModelId, categoryId, DgnCode());
    GeometrySourceP geomElem = el->ToGeometrySourceP();
    GeometryBuilderPtr builder = GeometryBuilder::Create(*model, categoryId, DPoint3d::From(0.0, 0.0, 0.0));
    DEllipse3d ellipseData = DEllipse3d::From(1, 2, 3,
    0, 0, 2,
    0, 3, 0,
    0.0, Angle::TwoPi());
    ICurvePrimitivePtr ellipse = ICurvePrimitive::CreateArc(ellipseData);
    EXPECT_TRUE(builder->Append(*ellipse));

    // Insert child subcategory
    DgnSubCategory subcategory2(DgnSubCategory::CreateParams(*m_db, categoryId, "subcatecogory2", appearence, "subcatecogory2 of TestCategoty"));
    DgnSubCategoryCPtr sub2 = subcategory2.Insert();
    EXPECT_TRUE(sub2.IsValid());
    sub2code = sub2->GetCode();
    DgnSubCategoryId subCategoryId = DgnSubCategory::QuerySubCategoryId(*m_db, sub2code);
    EXPECT_TRUE(builder->Append(subCategoryId));
    EXPECT_EQ(SUCCESS, builder->Finish(*geomElem));
    auto elem = m_db->Elements().Insert(*el);
    EXPECT_TRUE(elem.IsValid());

    m_db->SaveChanges();
    }
    m_db->CloseDb();

    OpenDb(m_db, outFileName, mode);
    {
    //Updates default Subcategory appearance
    DgnSubCategory::Appearance appearance2;
    appearance2.SetColor(ColorDef::Red());
    appearance2.SetWeight(5);
    appearance2.SetTransparency(1);
    appearance2.SetDisplayPriority(2);
    DgnCategoryPtr subCatd = m_db->Elements().GetForEdit<DgnCategory>(categoryId);
    subCatd->SetDefaultAppearance(appearance2);
    DgnDbStatus updateStatus;
    subCatd->Update(&updateStatus);
    EXPECT_TRUE(DgnDbStatus::Success == updateStatus);

    //Verification of default subcategory properties
    EXPECT_TRUE(ColorDef::Red() == appearance2.GetColor());
    EXPECT_TRUE(5 == appearance2.GetWeight());
    EXPECT_TRUE(1 == appearance2.GetTransparency());
    EXPECT_TRUE(2 == appearance2.GetDisplayPriority());

    //Updates Child Subcategory appearance
    DgnSubCategoryId subCategoryId = DgnSubCategory::QuerySubCategoryId(*m_db, sub2code);
    DgnSubCategoryPtr subCat = m_db->Elements().GetForEdit<DgnSubCategory>(subCategoryId);
    appearance2 = subCat->GetAppearance();
    appearance2.SetColor(ColorDef::Green());
    appearance2.SetWeight(5);
    appearance2.SetTransparency(1);
    appearance2.SetDisplayPriority(2);
    subCat->GetAppearanceR() = appearance2;
    subCat->Update(&updateStatus);
    EXPECT_TRUE(DgnDbStatus::Success == updateStatus);

    //Verification of child subcategory properties
    EXPECT_TRUE(ColorDef::Green() == appearance2.GetColor());
    EXPECT_TRUE(5 == appearance2.GetWeight());
    EXPECT_TRUE(1 == appearance2.GetTransparency());
    EXPECT_TRUE(2 == appearance2.GetDisplayPriority());

    m_db->SaveChanges();
    }
    m_db->CloseDb();
    OpenDb(m_db, outFileName, mode);
    // Verify default subcategory updated apprearance values that stored in Db
    DgnSubCategoryId DsubCatid = DgnCategory::GetDefaultSubCategoryId(categoryId);
    DgnSubCategoryCPtr subCat = m_db->Elements().Get<DgnSubCategory>(DsubCatid);
    DgnSubCategory::Appearance appearance2 = subCat->GetAppearance();
    ASSERT_TRUE(ColorDef::Red() == appearance2.GetColor());
    ASSERT_TRUE(5 == appearance2.GetWeight());
    ASSERT_TRUE(1 == appearance2.GetTransparency());
    ASSERT_TRUE(2 == appearance2.GetDisplayPriority());

    // Verify child subcategory updated apprearance values that stored in Db
    DgnSubCategoryId C_subcatid = DgnSubCategory::QuerySubCategoryId(*m_db, sub2code);
    DgnSubCategoryCPtr C_subCat = m_db->Elements().Get<DgnSubCategory>(C_subcatid);
    appearance2 = C_subCat->GetAppearance();
    ASSERT_TRUE(ColorDef::Green() == appearance2.GetColor());
    ASSERT_TRUE(5 == appearance2.GetWeight());
    ASSERT_TRUE(1 == appearance2.GetTransparency());
    ASSERT_TRUE(2 == appearance2.GetDisplayPriority());
    }
