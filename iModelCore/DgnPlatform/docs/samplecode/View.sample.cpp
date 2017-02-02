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
    catSel.GetCategoriesR() = categories;
    return db.Elements().Insert(catSel);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
ModelSelectorCPtr createAndInsertModelSelector(DgnDbR db, Utf8CP name, DgnModelIdSet const& models)
    {
    // ModelSelector is a definition element that is normally shared by many ViewDefinitions.
    ModelSelector modSel(db, name);
    modSel.GetModelsR() = models;
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
DgnViewId createAndInsertView(DgnDbR db, Utf8CP name, DRange3dCR viewExtents, CategorySelectorR catSel, ModelSelectorR modSel, DisplayStyle3dR dstyle)
    {
    // Construct the ViewDefinition
    // CategorySelector, ModelSelector, and DisplayStyle are definition elements that are normally shared by many ViewDefinitions.
    // That is why they are inputs to this function. 
    CameraViewDefinition view(db, name, catSel, dstyle, modSel);

    view.SetStandardViewRotation(StandardView::Iso);
    view.LookAtVolume(db.GeoLocation().GetProjectExtents());

    // Write the ViewDefinition to the bim
    if (!view.Insert().IsValid())
        return DgnViewId();

    return view.GetViewId();
    }
//__PUBLISH_EXTRACT_END__
