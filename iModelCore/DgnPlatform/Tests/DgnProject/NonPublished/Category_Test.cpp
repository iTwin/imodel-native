/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/Category_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../TestFixture/DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_DPTEST

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      01/15
+===============+===============+===============+===============+===============+======*/
struct CategoryTests : public DgnDbTestFixture
    {
    void CompareCategories(DgnCategoryId catId, Utf8CP code, DgnCategory::Scope scope, DgnCategory::Rank rank, Utf8CP descr)
        {
        DgnCategoryCPtr cat = DgnCategory::QueryCategory(catId, *m_db);
        EXPECT_TRUE(cat.IsValid());
        if (cat.IsValid())
            CompareCategories(*cat, code, scope, rank, descr);
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

    void CompareCategories(DgnCategoryCR cat, Utf8CP code, DgnCategory::Scope scope, DgnCategory::Rank rank, Utf8CP descr)
        {
        EXPECT_STREQ(code, cat.GetCategoryName().c_str());
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
            EXPECT_EQ(subcat->GetCode().GetNameSpace(), other.GetCode().GetNameSpace());
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
    SetupProject (L"3dMetricGeneral.idgndb", L"CategoryTests.idgndb", Db::OpenMode::ReadWrite);
    ASSERT_TRUE(m_db.IsValid());
        
    //Category properties.
    Utf8CP cat_code = "Test Category";
    Utf8CP cat_desc = "This is a test category.";

    DgnCategory category(DgnCategory::CreateParams(*m_db, cat_code, DgnCategory::Scope::Physical, DgnCategory::Rank::Domain, cat_desc));

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
    CompareCategories(category, cat_code, DgnCategory::Scope::Physical, DgnCategory::Rank::Domain, cat_desc);
    EXPECT_TRUE (category.GetCategoryId ().IsValid ());
    EXPECT_FALSE (pCategory->IsSystemCategory ());
    EXPECT_FALSE (pCategory->IsUserCategory ());
    CompareCategories(*pCategory, category);

    DgnCategoryId id = DgnCategory::QueryCategoryId(cat_code, *m_db);
    EXPECT_TRUE (id.IsValid ());
    EXPECT_EQ(id, category.GetCategoryId());
    EXPECT_EQ(id, pCategory->GetCategoryId());

    DgnCategoryCPtr query = DgnCategory::QueryCategory(id, *m_db);
    EXPECT_TRUE (query.IsValid ());

    DgnCategoryCPtr query_byCode = DgnCategory::QueryCategory(cat_code, *m_db);
    EXPECT_TRUE (query_byCode.IsValid ());

    //Inserts Category 2
    Utf8CP cat2_code = "Test Category 2";
    Utf8CP cat2_desc = "This is test category 2.";

    DgnCategory category2(DgnCategory::CreateParams(*m_db, cat2_code, DgnCategory::Scope::Any, DgnCategory::Rank::System, cat2_desc));
    DgnCategoryCPtr pCategory2 = category2.Insert(appearence);
    ASSERT_TRUE(pCategory2.IsValid());

    //Inserts Category 3
    Utf8CP cat3_code = "Test Category 3";
    Utf8CP cat3_desc = "This is test category 3.";

    DgnCategory category3(DgnCategory::CreateParams(*m_db, cat3_code, DgnCategory::Scope::Analytical, DgnCategory::Rank::User, cat3_desc));
    DgnCategoryCPtr pCategory3 = category3.Insert(appearence);
    ASSERT_TRUE(pCategory3.IsValid());

    //Inserts Category 4
    Utf8CP cat4_code = "Test Category 4";
    Utf8CP cat4_desc = "This is test category 4.";

    DgnCategory category4(DgnCategory::CreateParams(*m_db, cat4_code, DgnCategory::Scope::Annotation, DgnCategory::Rank::User, cat4_desc));
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
    }

//=======================================================================================
//! Test for Deleting a category.
// @bsiclass                                                     Maha Nasir      07/15
//=======================================================================================
TEST_F (CategoryTests, DeleteCategory)
    {
    SetupProject (L"3dMetricGeneral.idgndb", L"CategoryTests.idgndb", Db::OpenMode::ReadWrite);

    Utf8CP code = "TestCategory";
    Utf8CP desc = "This is a test category.";

    DgnCategory category(DgnCategory::CreateParams(*m_db, code, DgnCategory::Scope::Physical, DgnCategory::Rank::Domain, desc));

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
    DgnCategoryId id = DgnCategory::QueryCategoryId(code, *m_db);
    EXPECT_TRUE (id.IsValid ());

    // Deletion of a category is not supported.
    DgnDbStatus dlt = pCat->Delete();
    EXPECT_EQ(DgnDbStatus::DeletionProhibited, dlt);
    DgnCategoryId id1 = DgnCategory::QueryCategoryId(code, *m_db);
    EXPECT_TRUE (id1.IsValid ());
    }

//=======================================================================================
//! Test for Updating a category.
// @bsiclass                                                     Maha Nasir      07/15
//=======================================================================================
TEST_F (CategoryTests, UpdateCategory)
    {
    SetupProject (L"3dMetricGeneral.idgndb", L"CategoryTests.idgndb", Db::OpenMode::ReadWrite);

    //Category properties.
    Utf8CP code = "TestCategory";
    Utf8CP desc = "This is a test category.";

    DgnCategory category(DgnCategory::CreateParams(*m_db, code, DgnCategory::Scope::Physical, DgnCategory::Rank::Domain, desc));

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
    DgnCategoryId id = DgnCategory::QueryCategoryId (code, *m_db);
    EXPECT_TRUE (id.IsValid ());

#ifdef NEEDSWORK_THIS_CODE_DOESNT_EVEN_TEST_UPDATE
    Utf8CP u_code = "UpdatedTestCategory";
    Utf8CP u_desc = "This is the updated test category.";

    //Updates category.
    DgnCategories::Category Updated_category (u_code, DgnCategories::Scope::Any, u_desc, u_label, DgnCategories::Rank::Application);
    m_db->Categories ().Update (Updated_category);
    BeSQLite::DbResult insert1 = m_db->Categories ().Insert (Updated_category, appearence);
    EXPECT_EQ (BE_SQLITE_OK, insert1);

    EXPECT_TRUE (category.GetCategoryId ().IsValid ());
    EXPECT_STREQ ("UpdatedTestCategory", Updated_category.GetCode ());
    EXPECT_STREQ ("This is the updated test category.", Updated_category.GetDescription ());
    EXPECT_STREQ ("UpdatedTestCategory", Updated_category.GetLabel ());
#endif
    }

//=======================================================================================
//! Test for inserting SubCategories and checking their properties.
// @bsiclass                                                     Maha Nasir      07/15
//=======================================================================================
TEST_F (CategoryTests, InsertSubCategory)
    {
    SetupProject (L"3dMetricGeneral.idgndb", L"CategoryTests.idgndb", Db::OpenMode::ReadWrite);

    Utf8CP code = "TestCategory";
    Utf8CP desc = "This is a test category.";

    DgnCategory category(DgnCategory::CreateParams(*m_db, code, DgnCategory::Scope::Physical, DgnCategory::Rank::Domain, desc));


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
    DgnCategoryId id = DgnCategory::QueryCategoryId(code, *m_db);
    EXPECT_TRUE (id.IsValid ());

    Utf8CP sub_code = "Test SubCategory";
    Utf8CP sub_desc = "This is a test subcategory";
    DgnSubCategory subcategory(DgnSubCategory::CreateParams(*m_db, id, sub_code, appearence, sub_desc));

    //Inserts a subcategory
    EXPECT_TRUE(subcategory.Insert().IsValid());

    //Verifying appearence properties
    DgnSubCategory::Appearance app = subcategory.GetAppearance ();
    EXPECT_EQ (ColorDef::DarkRed (), app.GetColor ());
    EXPECT_EQ (dp, app.GetDisplayPriority ());
    EXPECT_EQ (trans, app.GetTransparency ());
    EXPECT_EQ (weight, app.GetWeight ());
    EXPECT_FALSE (app.IsInvisible ());
    EXPECT_TRUE(app.IsEqual(appearence));

    //Verifying subcategory properties
    DgnSubCategoryId subcat_id = DgnSubCategory::QuerySubCategoryId(id, sub_code, *m_db);
    EXPECT_TRUE (subcat_id.IsValid ());

    DgnSubCategoryCPtr query_sub = DgnSubCategory::QuerySubCategory(subcat_id, *m_db);
    EXPECT_TRUE (query_sub.IsValid ());

    DgnSubCategoryId default_subId = DgnCategory::GetDefaultSubCategoryId(id);
    EXPECT_EQ(id.GetValue()+1, default_subId.GetValue());

    //Inserts sub category 2
    Utf8CP sub2_code = "Test SubCategory 2";
    Utf8CP sub2_desc = "This is a test subcategory 2";

    DgnSubCategory subcategory2(DgnSubCategory::CreateParams(*m_db, id, sub2_code, appearence, sub2_desc));
    EXPECT_TRUE(subcategory2.Insert().IsValid());

    //Inserts sub category 3
    Utf8CP sub3_code = "Test SubCategory 3";
    Utf8CP sub3_desc = "This is a test subcategory 3";

    DgnSubCategory subcategory3(DgnSubCategory::CreateParams(*m_db, id, sub3_code, appearence, sub3_desc));
    EXPECT_TRUE(subcategory3.Insert().IsValid());

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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CategoryTests, SubCategoryInvariants)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"SubCategoryInvaraints.idgndb", Db::OpenMode::ReadWrite);
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
    EXPECT_EQ(defaultSubCat1->GetCode().GetNameSpace(), "Cat1");
    EXPECT_EQ(defaultSubCat1->GetCode().GetValue(), "Cat1");
    EXPECT_EQ(defaultSubCat1->GetSubCategoryId(), DgnCategory::GetDefaultSubCategoryId(cat1Id));

    // Code validation
    DgnSubCategoryPtr defaultSubCat1Edit = defaultSubCat1->MakeCopy<DgnSubCategory>();
    DgnAuthority::Code code;    // invalid code
    EXPECT_EQ(DgnDbStatus::InvalidName, defaultSubCat1Edit->SetCode(code));
    code = DgnSubCategory::CreateSubCategoryCode(cat2Id, "Cat1", db); // wrong category
    EXPECT_EQ(DgnDbStatus::InvalidName, defaultSubCat1Edit->SetCode(code));
    code = DgnSubCategory::CreateSubCategoryCode(cat2Id, "Cat2", db); // wrong category
    EXPECT_EQ(DgnDbStatus::InvalidName, defaultSubCat1Edit->SetCode(code));
    code = DgnSubCategory::CreateSubCategoryCode(cat1Id, "NewName", db); // sub-category name must equal category name
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

    DgnSubCategoryPtr pSubcat2B = cpSubcat2B->MakeCopy<DgnSubCategory>();
    pSubcat2B->SetCode(DgnSubCategory::CreateSubCategoryCode(cat2Id, "2A", db));
    EXPECT_TRUE(pSubcat2B->Update(&status).IsNull());
    EXPECT_EQ(DgnDbStatus::DuplicateCode, status);

    // Cannot change parent category
    EXPECT_EQ(DgnDbStatus::InvalidParent, pSubcat2B->SetParentId(cat1Id));

    // Code validation
    code = DgnSubCategory::CreateSubCategoryCode(cat1Id, "2B", db); // wrong category
    EXPECT_EQ(DgnDbStatus::InvalidName, pSubcat2B->SetCode(code));
    code = DgnSubCategory::CreateSubCategoryCode(cat2Id, "NewName", db);
    EXPECT_EQ(DgnDbStatus::Success, pSubcat2B->SetCode(code));

    // Can rename non-default sub-category if no name collisions
    cpSubcat2B = pSubcat2B->Update(&status);
    EXPECT_EQ(DgnDbStatus::Success, status);
    EXPECT_EQ(cpSubcat2B->GetCode().GetValue(), "NewName");

    // Illegal characters in names
    pSubcat2B = cpSubcat2B->MakeCopy<DgnSubCategory>();
    Utf8String invalidChars = DgnCategory::GetIllegalCharacters();
    for (auto const& invalidChar : invalidChars)
        {
        Utf8String newName("SubCat");
        newName.append(1, invalidChar);
        code = DgnSubCategory::CreateSubCategoryCode(cat2Id, newName, db);
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
    SetupProject (L"3dMetricGeneral.idgndb", L"CategoryTests.idgndb", Db::OpenMode::ReadWrite);

    Utf8CP code = "TestCategory";
    Utf8CP desc = "This is a test category.";

    DgnCategory category(DgnCategory::CreateParams(*m_db, code, DgnCategory::Scope::Physical, DgnCategory::Rank::Domain, desc));

    //Inserts a category.
    DgnSubCategory::Appearance appearence;
    EXPECT_TRUE(category.Insert(appearence).IsValid());
    DgnCategoryId id = DgnCategory::QueryCategoryId(code, *m_db);
    EXPECT_TRUE (id.IsValid ());

    Utf8CP sub_code = "TestSubCategory";
    Utf8CP sub_desc = "This is a test subcategory";
    DgnSubCategory subcategory(DgnSubCategory::CreateParams(*m_db, id, sub_code, appearence, sub_desc));

    //Inserts subcategory.
    DgnSubCategoryCPtr pSubCat = subcategory.Insert();
    ASSERT_TRUE(pSubCat.IsValid());

    //Verifying subcategory properties
    EXPECT_STREQ ("TestSubCategory", subcategory.GetSubCategoryName().c_str());
    EXPECT_STREQ ("This is a test subcategory", subcategory.GetDescription ());

    EXPECT_EQ(DgnDbStatus::Success, pSubCat->Delete());
    DgnSubCategoryId sub_id = DgnSubCategory::QuerySubCategoryId(id, sub_code, *m_db);
    EXPECT_FALSE (sub_id.IsValid ());
    }

#ifdef NEEDSWORK_THIS_CODE_DOESNT_EVEN_TEST_UPDATE
//=======================================================================================
//! Test for Updating a subcategory.
// @bsiclass                                                     Maha Nasir      07/15
//=======================================================================================
TEST_F (CategoryTests, UpdateSubCategory)
    {
    SetupProject (L"3dMetricGeneral.idgndb", L"CategoryTests.idgndb", Db::OpenMode::ReadWrite);

    Utf8CP code = "TestCategory";
    Utf8CP desc = "This is a test category.";
    Utf8CP label = "TestCategory";

    DgnCategories::Category category (code, DgnCategories::Scope::Physical, desc, label, DgnCategories::Rank::Domain);
    category.SetCode (code);
    category.SetDescription (desc);
    category.SetLabel (label);
    category.SetRank (DgnCategories::Rank::Domain);
    category.SetScope (DgnCategories::Scope::Physical);

    //Appearence properties.
    uint32_t weight = 10;
    double trans = 0.5;
    uint32_t dp = 1;

    DgnCategories::SubCategory::Appearance appearence;
    appearence.SetInvisible (false);
    appearence.SetColor (ColorDef::DarkRed ());
    appearence.SetWeight (weight);
    appearence.SetTransparency (trans);
    appearence.SetDisplayPriority (dp);

    //Inserts a category
    BeSQLite::DbResult insert = m_db->Categories ().Insert (category, appearence);
    EXPECT_EQ (BE_SQLITE_OK, insert);
    DgnCategoryId id = m_db->Categories ().QueryCategoryId (code);
    EXPECT_TRUE (id.IsValid ());

    Utf8CP u_code = "UpdatedSubCategory";
    Utf8CP u_desc = "This is the updated sub category.";
    Utf8CP u_label = "UpdatedSubCategory";
    uint64_t sub = 3;

    //Updates category.
    DgnCategories::SubCategory Updated_subcategory (id, (DgnSubCategoryId)sub, u_code, appearence, u_desc, u_label);
    m_db->Categories ().UpdateSubCategory (Updated_subcategory);
    BeSQLite::DbResult insert_sub = m_db->Categories ().InsertSubCategory (Updated_subcategory);
    EXPECT_EQ (BE_SQLITE_OK, insert_sub);

    //Verification of category properties
    EXPECT_TRUE (category.GetCategoryId ().IsValid ());
    EXPECT_STREQ ("UpdatedSubCategory", Updated_subcategory.GetCode ());
    EXPECT_STREQ ("This is the updated sub category.", Updated_subcategory.GetDescription ());
    EXPECT_STREQ ("UpdatedSubCategory", Updated_subcategory.GetLabel ());
    }
#endif

//=======================================================================================
//! Test for Quering a category.using elementID
// @bsiclass                                                     Umar.Hayat      09/15
//=======================================================================================
TEST_F (CategoryTests, QueryByElementId)
    {
    SetupProject (L"3dMetricGeneral.idgndb", L"CategoryTests.idgndb", Db::OpenMode::ReadWrite);

    //Category properties.
    Utf8CP code = "TestCategory";
    Utf8CP desc = "This is a test category.";

    DgnCategory category(DgnCategory::CreateParams(*m_db, code, DgnCategory::Scope::Physical, DgnCategory::Rank::Domain, desc));
    DgnSubCategory::Appearance appearence;

    //Inserts a category
    EXPECT_TRUE(category.Insert(appearence).IsValid());
    DgnCategoryId categoryId = DgnCategory::QueryCategoryId(code, *m_db);
    EXPECT_TRUE(categoryId.IsValid());

    DgnElementPtr el = TestElement::Create(*m_db, m_defaultModelId, categoryId, DgnElement::Code());
    GeometricElementP geomElem = const_cast<GeometricElementP>(el->ToGeometricElement());
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*m_defaultModelP, categoryId, DPoint3d::From(0.0, 0.0, 0.0));
    DEllipse3d ellipseData = DEllipse3d::From(1, 2, 3,
        0, 0, 2,
        0, 3, 0,
        0.0, Angle::TwoPi());
    ICurvePrimitivePtr ellipse = ICurvePrimitive::CreateArc(ellipseData);
    EXPECT_TRUE(builder->Append(*ellipse));

    EXPECT_EQ(SUCCESS, builder->SetGeomStreamAndPlacement(*geomElem));
    auto elem = m_db->Elements().Insert(*el);
    EXPECT_TRUE(elem.IsValid());

    // Search category by element Id
    DgnCategoryId tofind = DgnCategory::QueryElementCategoryId(el->GetElementId(), *m_db);
    EXPECT_TRUE(tofind.IsValid());
    EXPECT_TRUE(tofind == categoryId);
    }

/*---------------------------------------------------------------------------------**//**
* Category code = { Value : CategoryName; Namespace : blank }
* SubCategory code = { Value : SubCategoryName; Namespace : CategoryName }
* Default sub-category name is same as category name
* Therefore when we rename a category we must update sub-category codes.
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CategoryTests, RenameCategory)
    {
    SetupProject (L"3dMetricGeneral.idgndb", L"CategoryTests.idgndb", Db::OpenMode::ReadWrite);

    DgnCategory cat(DgnCategory::CreateParams(*m_db, "MyCat", DgnCategory::Scope::Physical));
    DgnSubCategory::Appearance app;
    DgnCategoryCPtr cpCat = cat.Insert(app);
    ASSERT_TRUE(cpCat.IsValid());

    DgnSubCategory subcat(DgnSubCategory::CreateParams(*m_db, cat.GetCategoryId(), "SubCat", app));
    EXPECT_TRUE(subcat.Insert().IsValid());

    DgnCategoryPtr pCat = cpCat->MakeCopy<DgnCategory>();
    EXPECT_EQ(DgnDbStatus::Success, pCat->SetCode(DgnCategory::CreateCategoryCode("NewCat")));
    EXPECT_TRUE(pCat->Update().IsValid());

    auto cpDefaultSubcat = DgnSubCategory::QuerySubCategory(pCat->GetDefaultSubCategoryId(), *m_db);
    ASSERT_TRUE(cpDefaultSubcat.IsValid());
    EXPECT_EQ(cpDefaultSubcat->GetCode().GetValue(), "NewCat");
    EXPECT_EQ(cpDefaultSubcat->GetCode().GetNameSpace(), "NewCat");

    auto cpSubcat = DgnSubCategory::QuerySubCategory(subcat.GetSubCategoryId(), *m_db);
    ASSERT_TRUE(cpSubcat.IsValid());
    EXPECT_EQ(cpSubcat->GetCode().GetValue(), "SubCat");
    EXPECT_EQ(cpSubcat->GetCode().GetNameSpace(), "NewCat");
    }


