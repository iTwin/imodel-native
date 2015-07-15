/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/Category_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <ECDb/ECSqlBuilder.h>

#define TMTEST_SCHEMA_NAME                               "DgnPlatformTest"
#define TMTEST_SCHEMA_NAMEW                             L"DgnPlatformTest"
#define TMTEST_TEST_ELEMENT_CLASS_NAME                   "TestElement"

USING_NAMESPACE_BENTLEY_SQLITE

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      01/15
+===============+===============+===============+===============+===============+======*/
struct CategoryTests : public ::testing::Test
    {
    public:
        ScopedDgnHost m_host;
        DgnDbPtr      m_db;
        DgnModelId    m_defaultModelId;
        DgnCategoryId m_defaultCategoryId;
        BeFileName schemaFile1;

        CategoryTests ();
        ~CategoryTests ();
        void CloseDb ()
            {
            m_db->CloseDb ();
            }

        void Setup_Project (WCharCP projFile, WCharCP testFile, Db::OpenMode mode);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Maha Nasir      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
CategoryTests::CategoryTests ()
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Maha Nasir      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
CategoryTests::~CategoryTests ()
    {}

/*---------------------------------------------------------------------------------**//**
* set up method that opens an existing .dgndb project file after copying it to out
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void CategoryTests::Setup_Project (WCharCP projFile, WCharCP testFile, Db::OpenMode mode)
    {
    BeFileName outFileName;
    ASSERT_EQ (SUCCESS, DgnDbTestDgnManager::GetTestDataOut (outFileName, projFile, testFile, __FILE__));
    DbResult result;
    m_db = DgnDb::OpenDgnDb (&result, outFileName, DgnDb::OpenParams (mode));
    ASSERT_TRUE (m_db.IsValid ());
    ASSERT_TRUE (result == BE_SQLITE_OK);
    }

//=======================================================================================
//! Test for inserting categories and checking their properties
// @bsiclass                                                     Maha Nasir      07/15
//=======================================================================================
TEST_F (CategoryTests, InsertCategory)
    {
    Setup_Project (L"3dMetricGeneral.idgndb", L"CategoryTests.idgndb", Db::OpenMode::ReadWrite);

    //Category properties.
    Utf8CP cat_code = "Test Category";
    Utf8CP cat_desc = "This is a test category.";
    Utf8CP cat_label = "TestCategory";

    DgnCategories::Category category (cat_code, DgnCategories::Scope::Physical, cat_desc, cat_label, DgnCategories::Rank::Domain);
    category.SetCode (cat_code);
    category.SetDescription (cat_desc);
    category.SetLabel (cat_label);
    category.SetRank (DgnCategories::Rank::Domain);
    category.SetScope (DgnCategories::Scope::Physical);

    //Appearence properties.
    uint32_t weight = 10;
    double trans = 0.5;
    uint32_t dp = 1;

    DgnCategories::SubCategory::Appearance appearence;
    appearence.SetInvisible (false);
    appearence.SetColor (ColorDef::Maroon ());
    appearence.SetWeight (weight);
    appearence.SetTransparency (trans);
    appearence.SetDisplayPriority (dp);

    BeSQLite::DbResult insert = m_db->Categories ().Insert (category, appearence);
    EXPECT_EQ (BE_SQLITE_OK, insert);

    //Verifying category properties
    EXPECT_TRUE (category.GetCategoryId ().IsValid ());
    EXPECT_FALSE (category.IsSystemCategory ());
    EXPECT_FALSE (category.IsUserCategory ());

    DgnCategoryId id = m_db->Categories ().QueryCategoryId (cat_code);
    EXPECT_TRUE (id.IsValid ());

    DgnCategories::Category query = m_db->Categories ().Query (id);
    EXPECT_TRUE (query.IsValid ());

    DgnCategories::Category query_byCode = m_db->Categories ().QueryCategoryByCode (cat_code);
    EXPECT_TRUE (query_byCode.IsValid ());

    //Inserts Category 2
    Utf8CP cat2_code = "Test Category 2";
    Utf8CP cat2_desc = "This is test category 2.";
    Utf8CP cat2_label = "TestCategory2";

    DgnCategories::Category category2 (cat2_code, DgnCategories::Scope::Any, cat2_desc, cat2_label, DgnCategories::Rank::System);
    BeSQLite::DbResult insert_cat2 = m_db->Categories ().Insert (category2, appearence);
    EXPECT_EQ (BE_SQLITE_OK, insert_cat2);

    //Inserts Category 3
    Utf8CP cat3_code = "Test Category 3";
    Utf8CP cat3_desc = "This is test category 3.";
    Utf8CP cat3_label = "TestCategory3";

    DgnCategories::Category category3 (cat3_code, DgnCategories::Scope::Analytical, cat3_desc, cat3_label, DgnCategories::Rank::User);
    BeSQLite::DbResult insert_cat3 = m_db->Categories ().Insert (category3, appearence);
    EXPECT_EQ (BE_SQLITE_OK, insert_cat3);

    DgnCategoryId highest_id = m_db->Categories ().QueryHighestId ();
    EXPECT_EQ (4, highest_id.GetValue ());

    //Iterator for categories.
    DgnCategories& cat = m_db->Categories ();
    DgnCategories::Iterator iter = cat.MakeIterator ();

    EXPECT_EQ (4, iter.QueryCount ());
    DgnCategories::Iterator::Entry entry = iter.begin ();
    int i = 0;
    for (auto const& entry : iter)
        {
        if (entry.GetCategoryId ().GetValue () == 2)
            {
            EXPECT_TRUE (entry.GetCategoryId ().IsValid ());
            EXPECT_STREQ ("Test Category", entry.GetCode ());
            EXPECT_STREQ ("This is a test category.", entry.GetDescription ());
            EXPECT_STREQ ("TestCategory", entry.GetLabel ());
            EXPECT_EQ (DgnCategories::Rank::Domain, entry.GetRank ());
            EXPECT_EQ (DgnCategories::Scope::Physical, entry.GetScope ());
            }
        else if (entry.GetCategoryId ().GetValue () == 3)
            {
            EXPECT_TRUE (entry.GetCategoryId ().IsValid ());
            EXPECT_STREQ ("Test Category 2", entry.GetCode ());
            EXPECT_STREQ ("This is test category 2.", entry.GetDescription ());
            EXPECT_STREQ ("TestCategory2", entry.GetLabel ());
            EXPECT_EQ (DgnCategories::Rank::System, entry.GetRank ());
            EXPECT_EQ (DgnCategories::Scope::Any, entry.GetScope ());
            }
        else if (entry.GetCategoryId ().GetValue () == 4)
            {
            EXPECT_TRUE (entry.GetCategoryId ().IsValid ());
            EXPECT_STREQ ("Test Category 3", entry.GetCode ());
            EXPECT_STREQ ("This is test category 3.", entry.GetDescription ());
            EXPECT_STREQ ("TestCategory3", entry.GetLabel ());
            EXPECT_EQ (DgnCategories::Rank::User, entry.GetRank ());
            EXPECT_EQ (DgnCategories::Scope::Analytical, entry.GetScope ());
            }
        i++;
        }
    iter.end ();
    }

//=======================================================================================
//! Test for Deleting a category.
// @bsiclass                                                     Maha Nasir      07/15
//=======================================================================================
TEST_F (CategoryTests, DeleteCategory)
    {
    Setup_Project (L"3dMetricGeneral.idgndb", L"CategoryTests.idgndb", Db::OpenMode::ReadWrite);

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
    appearence.SetColor (ColorDef::Maroon ());
    appearence.SetWeight (weight);
    appearence.SetTransparency (trans);
    appearence.SetDisplayPriority (dp);

    //Inserts a category
    BeSQLite::DbResult insert = m_db->Categories ().Insert (category, appearence);
    EXPECT_EQ (BE_SQLITE_OK, insert);
    DgnCategoryId id = m_db->Categories ().QueryCategoryId (code);
    EXPECT_TRUE (id.IsValid ());

    //Deletes category.
    BeSQLite::DbResult dlt = m_db->Categories ().Delete (id);
    EXPECT_EQ (BE_SQLITE_OK, dlt);
    DgnCategoryId id1 = m_db->Categories ().QueryCategoryId (code);
    EXPECT_FALSE (id1.IsValid ());
    }

//=======================================================================================
//! Test for Updating a category.
// @bsiclass                                                     Maha Nasir      07/15
//=======================================================================================
TEST_F (CategoryTests, UpdateCategory)
    {
    Setup_Project (L"3dMetricGeneral.idgndb", L"CategoryTests.idgndb", Db::OpenMode::ReadWrite);

    //Category properties.
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
    appearence.SetColor (ColorDef::Maroon ());
    appearence.SetWeight (weight);
    appearence.SetTransparency (trans);
    appearence.SetDisplayPriority (dp);

    //Inserts a category
    BeSQLite::DbResult insert = m_db->Categories ().Insert (category, appearence);
    EXPECT_EQ (BE_SQLITE_OK, insert);
    DgnCategoryId id = m_db->Categories ().QueryCategoryId (code);
    EXPECT_TRUE (id.IsValid ());

    Utf8CP u_code = "UpdatedTestCategory";
    Utf8CP u_desc = "This is the updated test category.";
    Utf8CP u_label = "UpdatedTestCategory";

    //Updates category.
    DgnCategories::Category Updated_category (u_code, DgnCategories::Scope::Any, u_desc, u_label, DgnCategories::Rank::Application);
    BeSQLite::DbResult update = m_db->Categories ().Update (Updated_category);
    BeSQLite::DbResult insert1 = m_db->Categories ().Insert (Updated_category, appearence);
    EXPECT_EQ (BE_SQLITE_OK, insert1);

    EXPECT_TRUE (category.GetCategoryId ().IsValid ());
    EXPECT_STREQ ("UpdatedTestCategory", Updated_category.GetCode ());
    EXPECT_STREQ ("This is the updated test category.", Updated_category.GetDescription ());
    EXPECT_STREQ ("UpdatedTestCategory", Updated_category.GetLabel ());
    }

//=======================================================================================
//! Test for inserting SubCategories and checking their properties.
// @bsiclass                                                     Maha Nasir      07/15
//=======================================================================================
TEST_F (CategoryTests, InsertSubCategory)
    {
    Setup_Project (L"3dMetricGeneral.idgndb", L"CategoryTests.idgndb", Db::OpenMode::ReadWrite);

    Utf8CP code = "TestCategory";
    Utf8CP desc = "This is a test category.";
    Utf8CP label = "TestCategory";

    DgnCategories::Category category (code, DgnCategories::Scope::Physical, desc, label, DgnCategories::Rank::Domain);
    category.SetCode (code);
    category.SetDescription (desc);
    category.SetLabel (label);
    category.SetRank (DgnCategories::Rank::Domain);
    category.SetScope (DgnCategories::Scope::Physical);
    uint32_t weight = 10;

    //Appearence properties.
    double trans = 0.5;
    uint32_t dp = 1;

    DgnCategories::SubCategory::Appearance appearence;
    appearence.SetInvisible (false);
    appearence.SetColor (ColorDef::Maroon ());
    appearence.SetWeight (weight);
    appearence.SetTransparency (trans);
    appearence.SetDisplayPriority (dp);

    //Inserts a category
    BeSQLite::DbResult insert = m_db->Categories ().Insert (category, appearence);
    EXPECT_EQ (BE_SQLITE_OK, insert);
    DgnCategoryId id = m_db->Categories ().QueryCategoryId (code);
    EXPECT_TRUE (id.IsValid ());

    Utf8CP sub_code = "Test SubCategory";
    Utf8CP sub_desc = "This is a test subcategory";
    Utf8CP sub_label = "TestSubCategory";
    int64_t sub = 3;
    DgnCategories::SubCategory subcategory (id, (DgnSubCategoryId)sub, sub_code, appearence, sub_desc, sub_label);

    //Inserts a subcategory
    BeSQLite::DbResult insert_sub = m_db->Categories ().InsertSubCategory (subcategory);
    EXPECT_EQ (BE_SQLITE_OK, insert_sub);

    //Verifying appearence properties
    DgnCategories::SubCategory::Appearance app = subcategory.GetAppearance ();
    EXPECT_EQ (ColorDef::Maroon (), app.GetColor ());
    EXPECT_EQ (dp, app.GetDisplayPriority ());
    EXPECT_EQ (trans, app.GetTransparency ());
    EXPECT_EQ (weight, app.GetWeight ());
    EXPECT_FALSE (app.IsInvisible ());

    //Verifying subcategory properties
    DgnSubCategoryId subcat_id = m_db->Categories ().QuerySubCategoryId (id, sub_code);
    EXPECT_TRUE (subcat_id.IsValid ());

    DgnCategories::SubCategory query_sub = m_db->Categories ().QuerySubCategory (subcat_id);
    EXPECT_TRUE (query_sub.IsValid ());

    DgnCategories::SubCategory query_sub_bycode = m_db->Categories ().QuerySubCategoryByCode (id, sub_code);
    EXPECT_TRUE (query_sub_bycode.IsValid ());

    Utf8CP illegal_char = m_db->Categories ().GetIllegalCharacters ();
    EXPECT_TRUE (m_db->Categories ().IsValidCode (sub_code) == true);

    DgnSubCategoryId deafult_subId = m_db->Categories ().DefaultSubCategoryId (id);
    EXPECT_EQ (2, deafult_subId.GetValue ());

    //Inserts sub category 2
    Utf8CP sub2_code = "Test SubCategory 2";
    Utf8CP sub2_desc = "This is a test subcategory 2";
    Utf8CP sub2_label = "TestSubCategory2";
    int64_t sub2 = 4;

    DgnCategories::SubCategory subcategory2 (id, (DgnSubCategoryId)sub2, sub2_code, appearence, sub2_desc, sub2_label);
    BeSQLite::DbResult insert_sub2 = m_db->Categories ().InsertSubCategory (subcategory2);
    EXPECT_EQ (BE_SQLITE_OK, insert_sub2);

    //Inserts sub category 3
    Utf8CP sub3_code = "Test SubCategory 3";
    Utf8CP sub3_desc = "This is a test subcategory 3";
    Utf8CP sub3_label = "TestSubCategory3";
    int64_t sub3 = 5;

    DgnCategories::SubCategory subcategory3 (id, (DgnSubCategoryId)sub3, sub3_code, appearence, sub3_desc, sub3_label);
    BeSQLite::DbResult insert_sub3 = m_db->Categories ().InsertSubCategory (subcategory3);
    EXPECT_EQ (BE_SQLITE_OK, insert_sub3);

    //Iterator for subcategories.
    DgnCategories& sub_cat = m_db->Categories ();
    DgnCategories::SubCategoryIterator iter = sub_cat.MakeSubCategoryIterator ();
    DgnCategories::SubCategoryIterator::Entry entry = iter.begin ();

    int i = 0;
    for (auto const& entry : iter)
        {
        if (entry.GetSubCategoryId ().GetValue () == 3)
            {
            EXPECT_TRUE (entry.GetSubCategoryId ().IsValid ());
            EXPECT_STREQ ("Test SubCategory", entry.GetCode ());
            EXPECT_STREQ ("This is a test subcategory", entry.GetDescription ());
            EXPECT_EQ ("TestSubCategory", (Utf8String)entry.GetLabel ());

            }
        else if (entry.GetSubCategoryId ().GetValue () == 4)
            {
            EXPECT_TRUE (entry.GetSubCategoryId ().IsValid ());
            EXPECT_STREQ ("Test SubCategory 2", entry.GetCode ());
            EXPECT_STREQ ("This is a test subcategory 2", entry.GetDescription ());
            EXPECT_EQ ("TestSubCategory2", (Utf8String)entry.GetLabel ());
            }
        else if (entry.GetSubCategoryId ().GetValue () == 5)
            {
            EXPECT_TRUE (entry.GetSubCategoryId ().IsValid ());
            EXPECT_STREQ ("Test SubCategory 3", entry.GetCode ());
            EXPECT_STREQ ("This is a test subcategory 3", entry.GetDescription ());
            EXPECT_EQ ("TestSubCategory3", (Utf8String)entry.GetLabel ());
            }
        i++;
        }
    iter.end ();
    }

//=======================================================================================
//! Test for Deleting a subcategory.
// @bsiclass                                                     Maha Nasir      07/15
//=======================================================================================
TEST_F (CategoryTests, DeleteSubCategory)
    {
    Setup_Project (L"3dMetricGeneral.idgndb", L"CategoryTests.idgndb", Db::OpenMode::ReadWrite);

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
    appearence.SetColor (ColorDef::Maroon ());
    appearence.SetWeight (weight);
    appearence.SetTransparency (trans);
    appearence.SetDisplayPriority (dp);

    //Inserts a category.
    BeSQLite::DbResult insert = m_db->Categories ().Insert (category, appearence);
    EXPECT_EQ (BE_SQLITE_OK, insert);
    DgnCategoryId id = m_db->Categories ().QueryCategoryId (code);
    EXPECT_TRUE (id.IsValid ());

    Utf8CP sub_code = "TestSubCategory";
    Utf8CP sub_desc = "This is a test subcategory";
    Utf8CP sub_label = "TestSubCategory";
    int64_t sub = 3;
    DgnCategories::SubCategory subcategory (id, (DgnSubCategoryId)sub, sub_code, appearence, sub_desc, sub_label);

    //Inserts subcategory.
    BeSQLite::DbResult insert_sub = m_db->Categories ().InsertSubCategory (subcategory);
    EXPECT_EQ (BE_SQLITE_OK, insert_sub);

    //Verifying subcategory properties
    EXPECT_STREQ ("TestSubCategory", subcategory.GetCode ());
    EXPECT_STREQ ("This is a test subcategory", subcategory.GetDescription ());
    EXPECT_STREQ ("TestSubCategory", subcategory.GetLabel ());

    BeSQLite::DbResult delete_sub = m_db->Categories ().DeleteSubCategory ((DgnSubCategoryId)sub);
    EXPECT_EQ (BE_SQLITE_OK, delete_sub);
    DgnSubCategoryId sub_id = m_db->Categories ().QuerySubCategoryId (id, sub_code);
    EXPECT_FALSE (sub_id.IsValid ());
    }

//=======================================================================================
//! Test for Updating a subcategory.
// @bsiclass                                                     Maha Nasir      07/15
//=======================================================================================
TEST_F (CategoryTests, UpdateSubCategory)
    {
    Setup_Project (L"3dMetricGeneral.idgndb", L"CategoryTests.idgndb", Db::OpenMode::ReadWrite);

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
    appearence.SetColor (ColorDef::Maroon ());
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
    int64_t sub = 3;

    //Updates category.
    DgnCategories::SubCategory Updated_subcategory (id, (DgnSubCategoryId)sub, u_code, appearence, u_desc, u_label);
    BeSQLite::DbResult update = m_db->Categories ().UpdateSubCategory (Updated_subcategory);
    BeSQLite::DbResult insert_sub = m_db->Categories ().InsertSubCategory (Updated_subcategory);
    EXPECT_EQ (BE_SQLITE_OK, insert_sub);

    //Verification of category properties
    EXPECT_TRUE (category.GetCategoryId ().IsValid ());
    EXPECT_STREQ ("UpdatedSubCategory", Updated_subcategory.GetCode ());
    EXPECT_STREQ ("This is the updated sub category.", Updated_subcategory.GetDescription ());
    EXPECT_STREQ ("UpdatedSubCategory", Updated_subcategory.GetLabel ());
    }
