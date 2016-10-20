/*--------------------------------------------------------------------------------------+
|
|     $Source: DataCaptureSchema/Tests/Published/DataCapture_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../BackDoor/PublicApi/BackDoor/DataCapture/BackDoor.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataCaptureTests, CreateCamera)
    {
    DgnDbPtr projectPtr = CreateProject(L"PhotoPlanningTest.dgndb");
    ASSERT_TRUE(projectPtr.IsValid());

    DgnModelId spatialModelId = QueryFirstSpatialModelId(*projectPtr);
    DgnModelPtr spatialModelPtr =projectPtr->Models().GetModel(spatialModelId);
    ASSERT_TRUE(spatialModelPtr.IsValid());
    ASSERT_TRUE(spatialModelPtr->IsSpatialModel());

    // Create Camera
    SpatialModelP spatialModelP = spatialModelPtr->ToSpatialModelP();
    auto cameraPtr = Camera::Create(*spatialModelP);
    ASSERT_TRUE(cameraPtr.IsValid());

    //Change camera properties
    cameraPtr->SetLabel("BasicCamera1");
    cameraPtr->SetFocalLenghtPixels(4798.35);
    ImageDimensionType imgDimension(5456,3632);
    cameraPtr->SetImageDimension(imgDimension); 
    DPoint2d principalPoint={2677.8,1772};
    cameraPtr->SetPrincipalPoint(principalPoint);
    CameraDistortionType distortion(1,2,3,4,5);
    cameraPtr->SetDistortion(distortion);
    cameraPtr->SetAspectRatio(1.0);
    cameraPtr->SetSkew(1.0);

    //Insert camera element
    auto cameraInsertedPtr = cameraPtr->Insert();
    ASSERT_TRUE(cameraInsertedPtr.IsValid());
    CameraElementId cameraId = cameraInsertedPtr->GetId();
    ASSERT_TRUE(cameraId.IsValid());

    //Save changes
    DbResult result = projectPtr->SaveChanges("BasicCamera");
    EXPECT_EQ(BE_SQLITE_OK, result) << "Save Camera failed";

    //Close project to flush memory
    cameraPtr=nullptr;//release our element before closing project, otherwise we get an assert in closeDb.
    cameraInsertedPtr=nullptr;
    CloseProject();

    //Reopen project
    DgnDbPtr projectReopenedPtr = OpenProject(L"PhotoPlanningTest.dgndb");
    ASSERT_TRUE(projectReopenedPtr.IsValid());

    ASSERT_TRUE(projectReopenedPtr->Elements().GetElement(cameraId).IsValid());
    CameraCPtr myCamPtr = Camera::Get(*projectReopenedPtr,cameraId);
    ASSERT_TRUE(myCamPtr.IsValid());
    ASSERT_EQ(cameraId, myCamPtr->GetElementId());

    //read back camera properties and check if equal
    ASSERT_DOUBLE_EQ(myCamPtr->GetFocalLenghtPixels(),4798.35);
    ASSERT_TRUE(imgDimension.IsEqual(myCamPtr->GetImageDimension()));
    ASSERT_TRUE(principalPoint.IsEqual(myCamPtr->GetPrincipalPoint()));
    ASSERT_TRUE(distortion.IsEqual(myCamPtr->GetDistortion()));
    ASSERT_DOUBLE_EQ(myCamPtr->GetAspectRatio(),1.0);
    ASSERT_DOUBLE_EQ(myCamPtr->GetSkew(),1.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataCaptureTests, ModifyCamera)
    {
    DgnDbPtr projectPtr = OpenProject(L"PhotoPlanningTest.dgndb");
    ASSERT_TRUE(projectPtr.IsValid());

    DgnModelId spatialModelId = QueryFirstSpatialModelId(*projectPtr);
    DgnModelPtr spatialModelPtr =projectPtr->Models().GetModel(spatialModelId);
    ASSERT_TRUE(spatialModelPtr.IsValid());
    ASSERT_TRUE(spatialModelPtr->IsSpatialModel());

    // Query Camera element
    DgnElementId cameraId  = Camera::QueryForIdByLabel(*projectPtr,"BasicCamera1");
    ASSERT_TRUE(cameraId.IsValid());
    CameraPtr cameraPtr = Camera::GetForEdit(*projectPtr, cameraId);
    ASSERT_TRUE(cameraPtr.IsValid());

    //Change camera properties
    cameraPtr->SetFocalLenghtPixels(12);
    ImageDimensionType imgDimension(13,14);
    cameraPtr->SetImageDimension(imgDimension); 
    DPoint2d principalPoint={15,16};
    cameraPtr->SetPrincipalPoint(principalPoint);
    CameraDistortionType distortion(11,12,13,14,15);
    cameraPtr->SetDistortion(distortion);
    cameraPtr->SetAspectRatio(2.0);
    cameraPtr->SetSkew(3.0);

    //Update camera element
    auto cameraUpdatedPtr = cameraPtr->Update();
    ASSERT_TRUE(cameraUpdatedPtr.IsValid());

    CameraElementId cameraUpdatedId = cameraUpdatedPtr->GetId();
    ASSERT_TRUE(cameraUpdatedId.IsValid());
    //Updating don't change id...
    ASSERT_TRUE(cameraUpdatedId == cameraId);

    //Save changes
    DbResult result = projectPtr->SaveChanges("BasicCamera");
    EXPECT_EQ(BE_SQLITE_OK, result) << "Save Camera failed";

    //Close project to flush memory
    cameraPtr = nullptr;//release our element before closing project, otherwise we get an assert in closeDb.
    cameraUpdatedPtr= nullptr;
    CloseProject();

    //Reopen project
    DgnDbPtr projectReopenedPtr = OpenProject(L"PhotoPlanningTest.dgndb");
    ASSERT_TRUE(projectReopenedPtr.IsValid());

    ASSERT_TRUE(projectReopenedPtr->Elements().GetElement(cameraId).IsValid());
    CameraCPtr myCamPtr = Camera::Get(*projectReopenedPtr,cameraId);
    ASSERT_TRUE(myCamPtr.IsValid());
    ASSERT_EQ(cameraId, myCamPtr->GetElementId());

    //read back camera properties and check if equal
    ASSERT_DOUBLE_EQ(myCamPtr->GetFocalLenghtPixels(),12);
    ASSERT_TRUE(imgDimension.IsEqual(myCamPtr->GetImageDimension()));
    ASSERT_TRUE(principalPoint.IsEqual(myCamPtr->GetPrincipalPoint()));
    ASSERT_TRUE(distortion.IsEqual(myCamPtr->GetDistortion()));
    ASSERT_DOUBLE_EQ(myCamPtr->GetAspectRatio(),2.0);
    ASSERT_DOUBLE_EQ(myCamPtr->GetSkew(),3.0);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataCaptureTests, DeleteCamera)
    {
    DgnDbPtr projectPtr = OpenProject(L"PhotoPlanningTest.dgndb");
    ASSERT_TRUE(projectPtr.IsValid());

    DgnModelId spatialModelId = QueryFirstSpatialModelId(*projectPtr);
    DgnModelPtr spatialModelPtr =projectPtr->Models().GetModel(spatialModelId);
    ASSERT_TRUE(spatialModelPtr.IsValid());
    ASSERT_TRUE(spatialModelPtr->IsSpatialModel());

    // Query Camera element
    DgnElementId cameraId  = Camera::QueryForIdByLabel(*projectPtr,"BasicCamera1");
    ASSERT_TRUE(cameraId.IsValid());

    //Delete edited camera element - You CANNOT delete an edited camera element because you get a copy of the original ...
    //Delete is merely a shortcut for el.GetDgnDb().Elements().Delete(el);
    CameraCPtr cameraEditedPtr = Camera::GetForEdit(*projectPtr, cameraId);
    ASSERT_TRUE(cameraEditedPtr.IsValid());
    DgnDbStatus status = cameraEditedPtr->Delete();
    ASSERT_FALSE(status==DgnDbStatus::Success);

    //Delete camera element - You CAN delete a const camera element because this is effectively the original element...
    //Delete is merely a shortcut for el.GetDgnDb().Elements().Delete(el);
    CameraCPtr cameraPtr = Camera::Get(*projectPtr, cameraId);
    ASSERT_TRUE(cameraPtr.IsValid());
    status = cameraPtr->Delete();
    ASSERT_TRUE(status == DgnDbStatus::Success);


    //Save changes
    DbResult result = projectPtr->SaveChanges("BasicCamera");
    EXPECT_EQ(BE_SQLITE_OK, result) << "Save Camera failed";

    //Close project to flush memory
    cameraPtr = nullptr;//release our element before closing project, otherwise we get an assert in closeDb.
    cameraEditedPtr=nullptr;
    CloseProject();

    //Reopen project
    DgnDbPtr projectReopenedPtr = OpenProject(L"PhotoPlanningTest.dgndb");
    ASSERT_TRUE(projectReopenedPtr.IsValid());

    //Check that cameraId is not accessible anymore 
    ASSERT_FALSE(projectReopenedPtr->Elements().GetElement(cameraId).IsValid());
    CameraCPtr myCamPtr = Camera::Get(*projectReopenedPtr,cameraId);
    ASSERT_FALSE(myCamPtr.IsValid());

    // Check that query Camera element returns nothing
    DgnElementId deletedCameraId = Camera::QueryForIdByLabel(*projectReopenedPtr, "BasicCamera1");
    ASSERT_FALSE(deletedCameraId.IsValid());
    }
