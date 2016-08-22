/*--------------------------------------------------------------------------------------+
|
|     $Source: docs/samplecode/View.sample.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_EXTRACT_START__ View_Includes.sampleCode
// Primary header file for the DgnPlatform API 
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnView.h>

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
    CategorySelector catSel(db, name);
    catSel.SetCategoryIds(categories);
    return db.Elements().Insert(catSel);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
ModelSelectorCPtr createAndInsertModelSelector(DgnDbR db, Utf8CP name, DgnModelIdSet const& models)
    {
    // ModelSelector is a definition element that is normally shared by many ViewDefinitions.
    ModelSelector modSel(db, name);
    modSel.SetModelIds(models);
    return db.Elements().Insert(modSel);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
DisplayStyleCPtr createAndInsertDisplayStyle(DgnDbR db, Utf8CP name)
    {
    // DisplayStyle is a definition element that is normally shared by many ViewDefinitions.
    DisplayStyle dstyle(db, name);
    Render::ViewFlags viewFlags = dstyle.GetViewFlags();
    viewFlags.SetRenderMode(Render::RenderMode::SmoothShade);
    return db.Elements().Insert(dstyle);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
DgnViewId createAndInsertView(DgnDbR db, Utf8CP name, DRange3dCR viewExtents, CategorySelectorCR catSel, ModelSelectorCR modSel, DisplayStyleCR dstyle)
    {
    // Construct the ViewDefinition
    CameraViewDefinition view(db, name);

    // CategorySelector, ModelSelector, and DisplayStyle are definition elements that are normally shared by many ViewDefinitions.
    // That is why they are inputs to this function. 
    // See createAndInsertCategorySelector, createAndInsertModelSelector, and createAndInsertDisplayStyle for examples of how to create these inputs.

    view.SetModelSelector(modSel);
    view.SetCategorySelector(catSel);
    view.SetDisplayStyleId(dstyle.GetElementId());

    view.SetStandardViewDirection(StandardView::Iso);

    // Write the ViewDefinition to the bim
    if (!view.Insert().IsValid())
        return DgnViewId();

    // We could have set up the view direction and camera while defining the CameraViewDefinition in the step above.
    // The most convenient way to do this, however, is to use one of the CamerViewController's editing functions.
    // We do that after inserting the ViewDefinition.
    ViewControllerPtr viewController = view.LoadViewController(ViewDefinition::FillModels::No);
    viewController->LookAtVolume(db.Units().GetProjectExtents());
    viewController->Save(); // This writes to the ViewDefinition

    return view.GetViewId();
    }
//__PUBLISH_EXTRACT_END__
