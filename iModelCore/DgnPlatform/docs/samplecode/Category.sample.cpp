/*--------------------------------------------------------------------------------------+
|
|     $Source: docs/samplecode/Category.sample.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_EXTRACT_START__ Category_Includes.sampleCode
// Primary header file for the DgnPlatform API 
#include <DgnPlatform/DgnPlatformApi.h>

// helper macro for using the BeSQLite namespace
USING_NAMESPACE_BENTLEY_SQLITE

// helper macro for using the DgnPlatform namespace
USING_NAMESPACE_BENTLEY_DGN
//__PUBLISH_EXTRACT_END__

//__PUBLISH_EXTRACT_START__ Category_CreateAndInsert.sampleCode
//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
DgnCategoryId createAndInsertCategory(DgnDbR db)
    {
    // configure the Category
    DgnCategory::CreateParams params(db,
            "MyApp_MyCategory",                         // Set the code for this category.
            DgnCategory::Scope::Physical,               // Set the scope for this category (which will only be valid for physical models)
            DgnCategory::Rank::Application,             // Indicate that this category was inserted by an application
            "MyCategory");                              // Friendly description for this category

    DgnCategory category(params);

    // configure the appearance for the Category's default SubCategory
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(ColorDef::Blue());

    if (!category.Insert(appearance).IsValid())
        return DgnCategoryId(); // return an invalid DgnCategoryId in case of an error

    return category.GetCategoryId(); // return the newly inserted category's Id (set by Insert method above)
    }
//__PUBLISH_EXTRACT_END__
