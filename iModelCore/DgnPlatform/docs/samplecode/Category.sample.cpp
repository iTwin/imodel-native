/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
DgnCategoryId createAndInsertCategory(DefinitionModelR definitionModel)
    {
    SpatialCategory category(definitionModel, "MyCategory", DgnCategory::Rank::Application);

    // configure the appearance for the Category's default SubCategory
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(ColorDef::Blue());

    if (!category.Insert(appearance).IsValid())
        return DgnCategoryId(); // return an invalid DgnCategoryId in case of an error

    return category.GetCategoryId(); // return the newly inserted category's Id (set by Insert method above)
    }
//__PUBLISH_EXTRACT_END__
