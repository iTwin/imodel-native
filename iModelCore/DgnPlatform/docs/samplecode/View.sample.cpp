/*--------------------------------------------------------------------------------------+
|
|     $Source: docs/samplecode/View.sample.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_EXTRACT_START__ View_Includes.sampleCode
// Primary header file for the DgnPlatform API 
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/ViewDefinition.h>

// helper macro for using the DgnPlatform namespace
USING_NAMESPACE_BENTLEY_DGN
//__PUBLISH_EXTRACT_END__

//__PUBLISH_EXTRACT_START__ View_CreateAndInsert.sampleCode
//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
CategorySelectorCPtr createAndInsertCategorySelector(DgnDbR db, Utf8CP name, DgnCategoryIdSet const& categories)
    {
    // CategorySelector is a definition element that is normally shared by many ViewDefinitions.
    CategorySelector categorySelector(db, name);
    categorySelector.GetCategoriesR() = categories;
    return db.Elements().Insert(categorySelector);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
ModelSelectorCPtr createAndInsertModelSelector(DgnDbR db, Utf8CP name, DgnModelIdSet const& models)
    {
    // ModelSelector is a definition element that is normally shared by many ViewDefinitions.
    ModelSelector modelSelector(db, name);
    modelSelector.GetModelsR() = models;
    return db.Elements().Insert(modelSelector);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
DisplayStyleCPtr createAndInsertDisplayStyle(DgnDbR db, Utf8CP name)
    {
    // DisplayStyle is a definition element that is normally shared by many ViewDefinitions.
    DisplayStyle displayStyle(db, name);
    Render::ViewFlags viewFlags = displayStyle.GetViewFlags();
    viewFlags.SetRenderMode(Render::RenderMode::SmoothShade);
    return db.Elements().Insert(displayStyle);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
DgnViewId createAndInsertView(DgnDbR db, Utf8CP name, DRange3dCR viewExtents, CategorySelectorR categorySelector, ModelSelectorR modelSelector, DisplayStyle3dR displayStyle)
    {
    // Construct the ViewDefinition
    // CategorySelector, ModelSelector, and DisplayStyle are definition elements that are normally shared by many ViewDefinitions.
    // That is why they are inputs to this function. 
    SpatialViewDefinition view(db.GetDictionaryModel(), name, categorySelector, displayStyle, modelSelector);

    view.SetStandardViewRotation(StandardView::Iso);
    view.LookAtVolume(db.GeoLocation().GetProjectExtents());

    // Write the ViewDefinition to the bim
    return !view.Insert().IsValid() ? DgnViewId() : view.GetViewId();
    }
//__PUBLISH_EXTRACT_END__
