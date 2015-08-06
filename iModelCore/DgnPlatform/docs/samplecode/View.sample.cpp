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
    DgnViews::View viewRow;
    DgnClassId classId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalView));
    viewRow.SetDgnViewType(classId, DgnViewType::Physical);
    viewRow.SetDgnViewSource(DgnViewSource::Generated);
    viewRow.SetName(name);
    viewRow.SetBaseModelId(baseModelId);

    if (BE_SQLITE_OK != db.Views().Insert(viewRow))
        return DgnViewId(); // error, return invalid DgnViewId

    PhysicalViewController viewController(db, viewRow.GetId());
    viewController.SetStandardViewRotation(StandardView::Iso);
    viewController.LookAtVolume(viewExtents);
    viewController.GetViewFlagsR().SetRenderMode(DgnRenderMode::SmoothShade);

    for (DgnCategoryId category : categories)
        viewController.ChangeCategoryDisplay(category, true);

    if (BE_SQLITE_OK != viewController.Save())
        return DgnViewId(); // error, return invalid DgnViewId

    return viewRow.GetId();
    }
//__PUBLISH_EXTRACT_END__
