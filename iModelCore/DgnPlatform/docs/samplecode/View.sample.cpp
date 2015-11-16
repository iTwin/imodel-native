/*--------------------------------------------------------------------------------------+
|
|     $Source: docs/samplecode/View.sample.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_EXTRACT_START__ View_Includes.sampleCode
// Primary header file for the DgnPlatform API 
#include <DgnPlatform/DgnPlatformApi.h>

// helper macro for using the BeSQLite namespace
USING_NAMESPACE_BENTLEY_SQLITE

// helper macro for using the DgnPlatform namespace
USING_NAMESPACE_BENTLEY_DGN
//__PUBLISH_EXTRACT_END__

//__PUBLISH_EXTRACT_START__ View_CreateAndInsert.sampleCode
//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
DgnViewId createAndInsertView(DgnDbR db, Utf8CP name, DgnModelId baseModelId, DgnCategoryIdSet categories, DRange3dCR viewExtents)
    {
    CameraViewDefinition view(CameraViewDefinition::CreateParams(db, name, ViewDefinition::Data(baseModelId, DgnViewSource::Generated)));
    view.Insert();
    DgnViewId viewId = view.GetViewId();
    if (viewId.IsValid())
        {
        PhysicalViewController viewController(db, viewId);
        viewController.SetStandardViewRotation(StandardView::Iso);
        viewController.LookAtVolume(viewExtents);
        viewController.GetViewFlagsR().SetRenderMode(DgnRenderMode::SmoothShade);

        for (DgnCategoryId category : categories)
            viewController.ChangeCategoryDisplay(category, true);

        if (BE_SQLITE_OK != viewController.Save())
            viewId.Invalidate();
        }

    return viewId;
    }
//__PUBLISH_EXTRACT_END__
