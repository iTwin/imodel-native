/*--------------------------------------------------------------------------------------+
|
|     $Source: docs/samplecode/ViewController.sample.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_EXTRACT_START__ ViewController_Includes.sampleCode
// Primary header file for the DgnPlatform API 
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/ViewController.h>

// helper macro for using the DgnPlatform namespace
USING_NAMESPACE_BENTLEY_DGN
//__PUBLISH_EXTRACT_END__

//__PUBLISH_EXTRACT_START__ ViewController.sampleCode
//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
DgnDbStatus viewControllerEdit(DgnViewportR viewport, DgnModelId newModel, DgnCategoryId newCategory)
    {
    // A ViewController is used to control an existing ViewDefinition in the course of a session.

    ViewControllerR controller = viewport.GetViewControllerR();
    
    // You can turn display of a catetory on or off
    controller.ChangeCategoryDisplay(newCategory, true);

    // You can turn display of a model on or off
    controller.ChangeModelDisplay(newModel, true);

    // There are ViewController functions to set the view geometry. 
    // In this example, we set up the view to look at the entire project.
    // Note that this example applies only to a spatial view.
    controller.LookAtVolume(controller.GetDgnDb().Units().GetProjectExtents());
    
    // If this is a camera view, then you can also use specialized Camera functions
    CameraViewControllerP cameraController = controller.ToCameraViewP();
    if (nullptr != cameraController)
        {
        DVec3d offset = DVec3d::From(0, 0, 1); // Move the camera in 1 meter
        ViewportStatus status = cameraController->MoveCameraLocal(offset);
        if (ViewportStatus::Success != status)
            return DgnDbStatus::BadRequest;
        }

    // If you don't want these changes to be persistent, then don't save them.

    // If you do want these changes to be persistent, then you must call the controller's Save function.
    // That causes it to write to the ViewDefinition and the shared definition elements that it uses.
    return cameraController->Save();
    }
//__PUBLISH_EXTRACT_END__
