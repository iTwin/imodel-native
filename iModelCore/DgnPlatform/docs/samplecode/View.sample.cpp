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
CategorySelectorCPtr createAndInsertCategorySelector(DefinitionModelR model, Utf8CP name, DgnCategoryIdSet const& categories)
    {
    // CategorySelector is a definition element that is potentially shared by many ViewDefinitions.
    DgnDbR db = model.GetDgnDb();
    CategorySelector categorySelector(model, name);
    categorySelector.GetCategoriesR() = categories;
    return db.Elements().Insert(categorySelector);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
ModelSelectorCPtr createAndInsertModelSelector(DefinitionModelR model, Utf8CP name, DgnModelIdSet const& modelsToSelect)
    {
    // ModelSelector is a definition element that is potentially shared by many ViewDefinitions.
    DgnDbR db = model.GetDgnDb();
    ModelSelector modelSelector(model, name); // model is the container for the ModelSelector definition element
    modelSelector.GetModelsR() = modelsToSelect; // modelsToSelect are the models that will be selected
    return db.Elements().Insert(modelSelector);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
DisplayStyle2dCPtr createAndInsertDisplayStyle(DefinitionModelR model, Utf8CP name)
    {
    // DisplayStyle is a definition element that is potentially shared by many ViewDefinitions.
    DgnDbR db = model.GetDgnDb();
    DisplayStyle2d displayStyle(model, name);
    Render::ViewFlags viewFlags = displayStyle.GetViewFlags();
    viewFlags.SetRenderMode(Render::RenderMode::SmoothShade);
    return db.Elements().Insert(displayStyle);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
DgnViewId createAndInsertView(DefinitionModelR model, Utf8CP name, DRange3dCR viewExtents, CategorySelectorR categorySelector, ModelSelectorR modelSelector, DisplayStyle3dR displayStyle)
    {
    DgnDbR db = model.GetDgnDb();

    // Construct the ViewDefinition
    // CategorySelector, ModelSelector, and DisplayStyle are definition elements that are potentially shared by many ViewDefinitions.
    // That is why they are inputs to this function. 
    SpatialViewDefinition view(model, name, categorySelector, displayStyle, modelSelector);

    view.SetStandardViewRotation(StandardView::Iso);
    view.LookAtVolume(db.GeoLocation().GetProjectExtents());

    // Write the ViewDefinition to the db
    return !view.Insert().IsValid() ? DgnViewId() : view.GetViewId();
    }
//__PUBLISH_EXTRACT_END__
