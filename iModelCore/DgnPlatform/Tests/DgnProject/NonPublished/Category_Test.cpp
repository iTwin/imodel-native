/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/Category_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../TestFixture/DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_DPTEST

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      01/15
+===============+===============+===============+===============+===============+======*/
struct CategoryTests : public DgnDbTestFixture
    {

    void CompareCategories(DgnCategoryId catId, Utf8CP name, DgnCategory::Scope scope, DgnCategory::Rank rank, Utf8CP descr)
        {
        DgnCategoryCPtr cat = DgnCategory::QueryCategory(catId, *m_db);
        EXPECT_TRUE(cat.IsValid());
        if (cat.IsValid())
            CompareCategories(*cat, name, scope, rank, descr);
        }

    void CompareCategories(DgnCategoryId catId, DgnCategoryCR other)
        {
        DgnCategoryCPtr cat = DgnCategory::QueryCategory(catId, *m_db);
        EXPECT_TRUE(cat.IsValid());
        if (cat.IsValid())
            CompareCategories(*cat, other);
        }

    void CompareCategories(DgnCategoryCR cat, DgnCategoryCR other)
        {
        CompareCategories(cat, other.GetCategoryName().c_str(), other.GetScope(), other.GetRank(), other.GetDescription());
        }

    void CompareCategories(DgnCategoryCR cat, Utf8CP name, DgnCategory::Scope scope, DgnCategory::Rank rank, Utf8CP descr)
        {
        EXPECT_STREQ(name, cat.GetCategoryName().c_str());
        EXPECT_EQ(scope, cat.GetScope());
        EXPECT_EQ(rank, cat.GetRank());
        EXPECT_STREQ(descr, cat.GetDescription());
        }

    void CompareSubCategories(DgnSubCategoryId subcatId, DgnSubCategoryCR other)
        {
        DgnSubCategoryCPtr subcat = DgnSubCategory::QuerySubCategory(subcatId, *m_db);
        EXPECT_TRUE(subcat.IsValid());
        if (subcat.IsValid())
            {
            EXPECT_STREQ(subcat->GetSubCategoryName().c_str(), other.GetSubCategoryName().c_str());
            EXPECT_EQ(subcat->GetCategoryId(), other.GetCategoryId());
            EXPECT_EQ(subcat->GetCode().GetNamespace(), other.GetCode().GetNamespace());
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

    DgnCategory category(DgnCategory::CreateParams(*m_db, cat_name, DgnCategory::Scope::Physical, DgnCategory::Rank::Domain, cat_desc));

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
    CompareCategories(category, cat_name, DgnCategory::Scope::Physical, DgnCategory::Rank::Domain, cat_desc);
    EXPECT_TRUE (category.GetCategoryId ().IsValid ());
    EXPECT_FALSE (pCategory->IsSystemCategory ());
    EXPECT_FALSE (pCategory->IsUserCategory ());
    CompareCategories(*pCategory, category);

    DgnCategoryId id = DgnCategory::QueryCategoryId(cat_name, *m_db);
    EXPECT_TRUE (id.IsValid ());
    EXPECT_EQ(id, category.GetCategoryId());
    EXPECT_EQ(id, pCategory->GetCategoryId());

    DgnCategoryCPtr query = DgnCategory::QueryCategory(id, *m_db);
    EXPECT_TRUE (query.IsValid ());

    DgnCategoryCPtr query_byname = DgnCategory::QueryCategory(cat_name, *m_db);
    EXPECT_TRUE (query_byname.IsValid ());

    //Inserts Category 2
    Utf8CP cat2_name = "Test Category 2";
    Utf8CP cat2_desc = "This is test category 2.";

    DgnCategory category2(DgnCategory::CreateParams(*m_db, cat2_name, DgnCategory::Scope::Any, DgnCategory::Rank::System, cat2_desc));
    DgnCategoryCPtr pCategory2 = category2.Insert(appearence);
    ASSERT_TRUE(pCategory2.IsValid());

    //Inserts Category 3
    Utf8CP cat3_name = "Test Category 3";
    Utf8CP cat3_desc = "This is test category 3.";

    DgnCategory category3(DgnCategory::CreateParams(*m_db, cat3_name, DgnCategory::Scope::Analytical, DgnCategory::Rank::User, cat3_desc));
    DgnCategoryCPtr pCategory3 = category3.Insert(appearence);
    ASSERT_TRUE(pCategory3.IsValid());

    //Inserts Category 4
    Utf8CP cat4_name = "Test Category 4";
    Utf8CP cat4_desc = "This is test category 4.";

    DgnCategory category4(DgnCategory::CreateParams(*m_db, cat4_name, DgnCategory::Scope::Annotation, DgnCategory::Rank::User, cat4_desc));
    DgnCategoryCPtr pCategory4 = category4.Insert(appearence);
    ASSERT_TRUE(pCategory4.IsValid());

    DgnCategoryId highest_id = DgnCategory::QueryHighestCategoryId(*m_db);
    EXPECT_EQ (category4.GetCategoryId().GetValue(), highest_id.GetValue ());

    //Iterator for categories.
    EXPECT_EQ(5, DgnCategory::QueryCount(*m_db));
    DgnCategoryIdSet catIds = DgnCategory::QueryCategories(*m_db);
    EXPECT_EQ(5, catIds.size());
    int nCompared = 0;
    int nNotCompared = 0;
    for (auto const& catId : catIds)
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
    EXPECT_EQ(4, nCompared);
    
    // Ordered List verification
    int count = 0;
    DgnCategoryIdList orderedList = DgnCategory::QueryOrderedCategories(*m_db);
    
    DgnCategoryId lastId;
    for (DgnCategoryId id : orderedList)
        {
        if (lastId.IsValid())
        {
        DgnCategoryCPtr current = DgnCategory::QueryCategory(id, *m_db);
        DgnCategoryCPtr lastCategory = DgnCategory::QueryCategory(lastId, *m_db);
        EXPECT_TRUE(current->GetCode().GetValue().CompareTo( lastCategory->GetCode().GetValue().c_str()) > 0);
        ++count;
        }
        lastId = id;
        }
    EXPECT_EQ(5, orderedList.size());
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

    DgnCategory category(DgnCategory::CreateParams(*m_db, name, DgnCategory::Scope::Physical, DgnCategory::Rank::Domain, desc));

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
    DgnCategoryId id = DgnCategory::QueryCategoryId(name, *m_db);
    EXPECT_TRUE (id.IsValid ());

    // Deletion of a category is not supported.
    DgnDbStatus dlt = pCat->Delete();
    EXPECT_EQ(DgnDbStatus::DeletionProhibited, dlt);
    DgnCategoryId id1 = DgnCategory::QueryCategoryId(name, *m_db);
    EXPECT_TRUE (id1.IsValid ());
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

    DgnCategory category(DgnCategory::CreateParams(*m_db, name, DgnCategory::Scope::Physical, DgnCategory::Rank::Domain, desc));

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
    DgnCategoryId id = DgnCategory::QueryCategoryId (name, *m_db);
    EXPECT_TRUE (id.IsValid ());

    //Utf8CP u_name = "UpdatedTestCategory";
    Utf8CP u_desc = "This is the updated test category.";

    //Updates category.
    DgnCategoryPtr toFind = m_db->Elements().GetForEdit<DgnCategory>(id);
    EXPECT_TRUE(toFind.IsValid());
    toFind->SetScope(DgnCategory::Scope::Any);
    toFind->SetDescription(Utf8String(u_desc));
    DgnDbStatus updateStatus;
    toFind->Update(&updateStatus);
    ASSERT_EQ(DgnDbStatus::Success, updateStatus);
    
    //Verification of category properties
    DgnCategoryCPtr updatedCat = DgnCategory::QueryCategory(id, *m_db);
    EXPECT_TRUE(updatedCat.IsValid());
    EXPECT_STREQ(u_desc, updatedCat->GetDescription());
    EXPECT_TRUE(DgnCategory::Scope::Any == updatedCat->GetScope());
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

    DgnCategory category(DgnCategory::CreateParams(*m_db, name, DgnCategory::Scope::Physical, DgnCategory::Rank::Domain, desc));


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
    DgnCategoryId id = DgnCategory::QueryCategoryId(name, *m_db);
    EXPECT_TRUE (id.IsValid ());

    Utf8CP sub_name = "Test SubCategory";
    Utf8CP sub_desc = "This is a test subcategory";
    DgnSubCategory subcategory(DgnSubCategory::CreateParams(*m_db, id, sub_name, appearence, sub_desc));
    

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
    DgnSubCategoryId subcat_id = DgnSubCategory::QuerySubCategoryId(id, sub_name, *m_db);
    EXPECT_TRUE (subcat_id.IsValid ());

    DgnSubCategoryId subcat_id_byCode = DgnSubCategory::QuerySubCategoryId(code, *m_db);
    EXPECT_TRUE(subcat_id_byCode.IsValid());
    EXPECT_TRUE(subcat_id == subcat_id_byCode);

    DgnSubCategoryCPtr query_sub = DgnSubCategory::QuerySubCategory(subcat_id, *m_db);
    EXPECT_TRUE (query_sub.IsValid ());

    DgnSubCategoryId default_subId = DgnCategory::GetDefaultSubCategoryId(id);
    EXPECT_EQ(id.GetValue()+1, default_subId.GetValue());

    //Inserts sub category 2
    Utf8CP sub2_name = "Test SubCategory 2";
    Utf8CP sub2_desc = "This is a test subcategory 2";

    DgnSubCategory subcategory2(DgnSubCategory::CreateParams(*m_db, id, sub2_name, appearence, sub2_desc));
    EXPECT_TRUE(subcategory2.Insert().IsValid());

    //Inserts sub category 3
    Utf8CP sub3_name = "Test SubCategory 3";
    Utf8CP sub3_desc = "This is a test subcategory 3";

    DgnSubCategory subcategory3(DgnSubCategory::CreateParams(*m_db, id, sub3_name, appearence, sub3_desc));
    EXPECT_TRUE(subcategory3.Insert().IsValid());

    EXPECT_EQ(4, (int)category.QuerySubCategoryCount());

    //Iterator for subcategories.
    DgnSubCategoryIdSet subcatIds = DgnSubCategory::QuerySubCategories(*m_db, id);
    EXPECT_EQ(4, subcatIds.size());
    EXPECT_EQ(subcatIds.size()+1, DgnSubCategory::QueryCount(*m_db)); // + default sub-category of category created by v8 converter
    EXPECT_EQ(subcatIds.size()+1, DgnSubCategory::QuerySubCategories(*m_db).size()); // + default sub-category of category created by v8 converter
    EXPECT_EQ(subcatIds.size(), DgnSubCategory::QueryCount(*m_db, id));

    int nCompared = 0;
    int nNotCompared = 0;
    for (auto const& subcatId : subcatIds)
        {
        DgnSubCategoryCP pCompareTo = nullptr;
        if (subcategory.GetSubCategoryId() == subcatId)
            pCompareTo = &subcategory;
        else if (subcategory2.GetSubCategoryId() == subcatId)
            pCompareTo = &subcategory2;
        else if (subcategory3.GetSubCategoryId() == subcatId)
            pCompareTo = &subcategory3;

        if (nullptr != pCompareTo)
            {
            ++nCompared;
            CompareSubCategories(subcatId, *pCompareTo);
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

    DgnSubCategory::Appearance app;
    DgnCategory cat1(DgnCategory::CreateParams(db, "Cat1", DgnCategory::Scope::Physical, DgnCategory::Rank::Domain));
    ASSERT_TRUE(cat1.Insert(app).IsValid());
    DgnCategory cat2(DgnCategory::CreateParams(db, "Cat2", DgnCategory::Scope::Physical, DgnCategory::Rank::Domain));
    ASSERT_TRUE(cat2.Insert(app).IsValid());
    DgnCategoryId cat1Id = cat1.GetCategoryId(),
                  cat2Id = cat2.GetCategoryId();

    // default sub-category exists with expected Code + ID
    DgnSubCategoryCPtr defaultSubCat1 = DgnSubCategory::QuerySubCategory(DgnCategory::GetDefaultSubCategoryId(cat1Id), db);
    ASSERT_TRUE(defaultSubCat1.IsValid());
    EXPECT_EQ(defaultSubCat1->GetCode().GetValue(), "Cat1");
    EXPECT_EQ(defaultSubCat1->GetSubCategoryId(), DgnCategory::GetDefaultSubCategoryId(cat1Id));

    // Code validation
    DgnSubCategoryPtr defaultSubCat1Edit = defaultSubCat1->MakeCopy<DgnSubCategory>();
    DgnCode code;    // invalid code
    EXPECT_EQ(DgnDbStatus::InvalidCodeAuthority, defaultSubCat1Edit->SetCode(code));
    code = DgnSubCategory::CreateSubCategoryCode(cat2Id, "Cat1"); // wrong category
    EXPECT_EQ(DgnDbStatus::InvalidName, defaultSubCat1Edit->SetCode(code));
    code = DgnSubCategory::CreateSubCategoryCode(cat2Id, "Cat2"); // wrong category
    EXPECT_EQ(DgnDbStatus::InvalidName, defaultSubCat1Edit->SetCode(code));
    code = DgnSubCategory::CreateSubCategoryCode(cat1Id, "NewName"); // sub-category name must equal category name
    EXPECT_EQ(DgnDbStatus::InvalidName, defaultSubCat1Edit->SetCode(code));

    // Cannot delete default sub-category
    EXPECT_EQ(DgnDbStatus::ParentBlockedChange, defaultSubCat1->Delete());

    // Cannot change parent category
    EXPECT_EQ(DgnDbStatus::InvalidParent, defaultSubCat1Edit->SetParentId(cat2Id));

    // require valid parent category
    DgnSubCategory noParent(DgnSubCategory::CreateParams(db, DgnCategoryId(), "NoParent", app, "Sub-category requires valid parent category"));
    DgnDbStatus status;
    EXPECT_TRUE(noParent.Insert(&status).IsNull());
    EXPECT_EQ(status, DgnDbStatus::InvalidName); // InvalidName because parent ID used to generate code.

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
    DgnSubCategoryPtr pSubcat2B = cpSubcat2B->MakeCopy<DgnSubCategory>();
    printf("\n%s, %s\n", pSubcat2B->GetCode().GetValue().c_str(), DgnSubCategory::CreateSubCategoryCode(cat2Id, "2A").GetValue().c_str());
    pSubcat2B->SetCode(DgnSubCategory::CreateSubCategoryCode(cat2Id, "2A"));
    EXPECT_TRUE(pSubcat2B->Update(&status).IsNull());
    EXPECT_EQ(DgnDbStatus::DuplicateCode, status);

    // Cannot change parent category
    EXPECT_EQ(DgnDbStatus::InvalidParent, pSubcat2B->SetParentId(cat1Id));

    // Code validation
    code = DgnSubCategory::CreateSubCategoryCode(cat1Id, "2B"); // wrong category
    EXPECT_EQ(DgnDbStatus::InvalidName, pSubcat2B->SetCode(code));
    code = DgnSubCategory::CreateSubCategoryCode(cat2Id, "NewName");
    EXPECT_EQ(DgnDbStatus::Success, pSubcat2B->SetCode(code));

    // Can rename non-default sub-category if no name collisions
    cpSubcat2B = pSubcat2B->Update(&status);
    EXPECT_EQ(DgnDbStatus::Success, status);
    EXPECT_EQ(cpSubcat2B->GetCode().GetValue().c_str(), "NewName");

    // Illegal characters in names
    pSubcat2B = cpSubcat2B->MakeCopy<DgnSubCategory>();
    Utf8String invalidChars = DgnCategory::GetIllegalCharacters();
    for (auto const& invalidChar : invalidChars)
        {
        Utf8String newName("SubCat");
        newName.append(1, invalidChar);
        code = DgnSubCategory::CreateSubCategoryCode(cat2Id, newName);
        EXPECT_EQ(DgnDbStatus::InvalidName, pSubcat2B->SetCode(code));
        }

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

    DgnCategory category(DgnCategory::CreateParams(*m_db, name, DgnCategory::Scope::Physical, DgnCategory::Rank::Domain, desc));

    //Inserts a category.
    DgnSubCategory::Appearance appearence;
    EXPECT_TRUE(category.Insert(appearence).IsValid());
    DgnCategoryId id = DgnCategory::QueryCategoryId(name, *m_db);
    EXPECT_TRUE (id.IsValid ());

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
    DgnSubCategoryId sub_id = DgnSubCategory::QuerySubCategoryId(id, sub_name, *m_db);
    EXPECT_FALSE (sub_id.IsValid ());
    }

//=======================================================================================
//! Test for Updating a subcategory.
// @bsiclass                                                     Maha Nasir      07/15
//=======================================================================================
TEST_F (CategoryTests, UpdateSubCategory)
    {
    SetupSeedProject();

    Utf8CP name = "TestCategory";
    Utf8CP desc = "This is a test category.";

    DgnCategory category(DgnCategory::CreateParams(*m_db, name, DgnCategory::Scope::Physical, DgnCategory::Rank::Domain, desc));

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

    //Insert category
    EXPECT_TRUE(category.Insert(appearence).IsValid());
    DgnCategoryId id = DgnCategory::QueryCategoryId(name,*m_db);
    EXPECT_TRUE (id.IsValid ());

    //Utf8CP u_name = "UpdatedSubCategory";
    Utf8CP u_desc = "This is the updated sub category.";

    //Updates category.
    DgnSubCategoryId subCategoryId = DgnCategory::GetDefaultSubCategoryId(id);
    DgnSubCategoryPtr subCat = m_db->Elements().GetForEdit<DgnSubCategory>(subCategoryId);
    DgnSubCategory::Appearance appearance2 = subCat->GetAppearance();
    appearance2.SetColor(ColorDef::Red());
    subCat->SetDescription(Utf8String(u_desc));
    DgnDbStatus updateStatus;
    subCat->Update(&updateStatus);
    EXPECT_TRUE(DgnDbStatus::Success == updateStatus);

    //Verification of category properties
    DgnSubCategoryCPtr updatedSubCat =DgnSubCategory::QuerySubCategory(subCategoryId, *m_db);
    EXPECT_TRUE(updatedSubCat.IsValid());
    EXPECT_STREQ(u_desc, updatedSubCat->GetDescription());
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

    DgnCategory category(DgnCategory::CreateParams(*m_db, name, DgnCategory::Scope::Physical, DgnCategory::Rank::Domain, desc));
    DgnSubCategory::Appearance appearence;

    //Inserts a category
    EXPECT_TRUE(category.Insert(appearence).IsValid());
    DgnCategoryId categoryId = DgnCategory::QueryCategoryId(name, *m_db);
    EXPECT_TRUE(categoryId.IsValid());

    DgnElementPtr el = TestElement::Create(*m_db, m_defaultModelId, categoryId, DgnCode());
    GeometrySourceP geomElem = el->ToGeometrySourceP();
    GeometryBuilderPtr builder = GeometryBuilder::Create(*m_defaultModelP, categoryId, DPoint3d::From(0.0, 0.0, 0.0));
    DEllipse3d ellipseData = DEllipse3d::From(1, 2, 3,
        0, 0, 2,
        0, 3, 0,
        0.0, Angle::TwoPi());
    ICurvePrimitivePtr ellipse = ICurvePrimitive::CreateArc(ellipseData);
    EXPECT_TRUE(builder->Append(*ellipse));

    EXPECT_EQ(SUCCESS, builder->SetGeometryStreamAndPlacement(*geomElem));
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
