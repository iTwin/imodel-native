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
    DgnDbPtr projectPtr = CreateProject(L"CreateCamera.dgndb");
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
    DgnDbPtr projectReopenedPtr = OpenProject(L"CreateCamera.dgndb");
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
    DgnDbPtr projectPtr = CreateProject(L"ModifyCamera.dgndb");
    ASSERT_TRUE(projectPtr.IsValid());
    Utf8String cameraLabel = "CameraToModify";
    CreateSamplePhotoProjectWithCamera(*projectPtr, cameraLabel.c_str());


    DgnModelId spatialModelId = QueryFirstSpatialModelId(*projectPtr);
    DgnModelPtr spatialModelPtr =projectPtr->Models().GetModel(spatialModelId);
    ASSERT_TRUE(spatialModelPtr.IsValid());
    ASSERT_TRUE(spatialModelPtr->IsSpatialModel());

    // Query Camera element
    DgnElementId cameraId  = Camera::QueryForIdByLabel(*projectPtr,cameraLabel.c_str());
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
    DgnDbPtr projectReopenedPtr = OpenProject(L"ModifyCamera.dgndb");
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
    DgnDbPtr projectPtr = CreateProject(L"DeleteCamera.dgndb");
    ASSERT_TRUE(projectPtr.IsValid());
    Utf8String cameraLabel = "CameraToDelete";
    CreateSamplePhotoProjectWithCamera(*projectPtr, cameraLabel.c_str());

    // Query Camera element
    CameraElementId cameraId = Camera::QueryForIdByLabel(*projectPtr, cameraLabel.c_str());
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
    DgnDbPtr projectReopenedPtr = OpenProject(L"DeleteCamera.dgndb");
    ASSERT_TRUE(projectReopenedPtr.IsValid());

    //Check that cameraId is not accessible anymore 
    ASSERT_FALSE(projectReopenedPtr->Elements().GetElement(cameraId).IsValid());
    CameraCPtr myCamPtr = Camera::Get(*projectReopenedPtr,cameraId);
    ASSERT_FALSE(myCamPtr.IsValid());

    // Check that query Camera element returns nothing
    DgnElementId deletedCameraId = Camera::QueryForIdByLabel(*projectReopenedPtr, cameraLabel.c_str());
    ASSERT_FALSE(deletedCameraId.IsValid());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataCaptureTests, CreatePhoto)
    {
    DgnDbPtr projectPtr = CreateProject(L"CreatePhoto.dgndb");
    ASSERT_TRUE(projectPtr.IsValid());

    DgnModelId spatialModelId = QueryFirstSpatialModelId(*projectPtr);
    DgnModelPtr spatialModelPtr =projectPtr->Models().GetModel(spatialModelId);
    ASSERT_TRUE(spatialModelPtr.IsValid());
    ASSERT_TRUE(spatialModelPtr->IsSpatialModel());
    SpatialModelP spatialModelP = spatialModelPtr->ToSpatialModelP();

    // Create Camera
    auto cameraPtr = Camera::Create(*spatialModelP);
    ASSERT_TRUE(cameraPtr.IsValid());
    cameraPtr->SetLabel("BasicCamera1");
    cameraPtr->SetFocalLenghtPixels(4798.35);
    ImageDimensionType imgDimension(5456, 3632);
    cameraPtr->SetImageDimension(imgDimension);
    DPoint2d principalPoint = { 2677.8,1772 };
    cameraPtr->SetPrincipalPoint(principalPoint);
    CameraDistortionType distortion(1, 2, 3, 4, 5);
    cameraPtr->SetDistortion(distortion);
    cameraPtr->SetAspectRatio(1.0);
    cameraPtr->SetSkew(1.0);
    auto cameraInsertedPtr = cameraPtr->Insert();
    ASSERT_TRUE(cameraInsertedPtr.IsValid());
    CameraElementId cameraId = cameraInsertedPtr->GetId();
    ASSERT_TRUE(cameraId.IsValid());


    // Create Photo for the camera
    auto PhotoPtr = Photo::Create(*spatialModelP,cameraId);
    ASSERT_TRUE(PhotoPtr.IsValid());

    //Change Photo properties
    PhotoPtr->SetLabel("BasicPhoto1");
    RotationMatrixType rotation(RotationMatrixType::FromIdentity());
    DPoint3d center = {1.0,2.0,3.0};
    PoseType pose(center,rotation);
    PhotoPtr->SetPose(pose);
    PhotoPtr->SetPhotoId(42);

    //Insert Photo element
    auto PhotoInsertedPtr = PhotoPtr->Insert();
    ASSERT_TRUE(PhotoInsertedPtr.IsValid());
    PhotoElementId PhotoElementId = PhotoInsertedPtr->GetId();
    ASSERT_TRUE(PhotoElementId.IsValid());
    CameraElementId cameraIdRead = PhotoInsertedPtr->GetCameraId();
    ASSERT_TRUE(cameraIdRead.IsValid());
    ASSERT_EQ(cameraIdRead, cameraId);

    //Save changes
    DbResult result = projectPtr->SaveChanges("BasicPhoto");
    EXPECT_EQ(BE_SQLITE_OK, result) << "Save Photo failed";

    //Close project to flush memory
    PhotoPtr=nullptr;//release our element before closing project, otherwise we get an assert in closeDb.
    PhotoInsertedPtr=nullptr;
    cameraPtr=nullptr;
    cameraInsertedPtr=nullptr;
    CloseProject();

    //Reopen project
    DgnDbPtr projectReopenedPtr = OpenProject(L"CreatePhoto.dgndb");
    ASSERT_TRUE(projectReopenedPtr.IsValid());

    ASSERT_TRUE(projectReopenedPtr->Elements().GetElement(PhotoElementId).IsValid());
    PhotoCPtr myPhotoPtr = Photo::Get(*projectReopenedPtr,PhotoElementId);
    ASSERT_TRUE(myPhotoPtr.IsValid());
    ASSERT_EQ(PhotoElementId, myPhotoPtr->GetElementId());

    //read back Photo properties and check if equal
    ASSERT_DOUBLE_EQ(myPhotoPtr->GetPhotoId(),42);
    ASSERT_TRUE(pose.IsEqual(myPhotoPtr->GetPose()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataCaptureTests, ModifyPhoto)
    {
    DgnDbPtr projectPtr = CreateProject(L"ModifyPhoto.dgndb");
    ASSERT_TRUE(projectPtr.IsValid());
    Utf8String cameraLabel = "BasicCamera";
    CreateSamplePhotoProjectWithCamera(*projectPtr, cameraLabel.c_str());


    DgnModelId spatialModelId = QueryFirstSpatialModelId(*projectPtr);
    DgnModelPtr spatialModelPtr =projectPtr->Models().GetModel(spatialModelId);
    ASSERT_TRUE(spatialModelPtr.IsValid());
    ASSERT_TRUE(spatialModelPtr->IsSpatialModel());


    // Query Photo element
    Utf8String photoLabel(Utf8PrintfString("BasicPhoto%d", 5));
    DgnElementId PhotoId  = Photo::QueryForIdByLabel(*projectPtr,photoLabel.c_str());
    ASSERT_TRUE(PhotoId.IsValid());
    PhotoPtr PhotoPtr = Photo::GetForEdit(*projectPtr, PhotoId);
    ASSERT_TRUE(PhotoPtr.IsValid());

    //Change Photo properties
    RotationMatrixType rotation(RotationMatrixType::FromRowValues(1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0));
    DPoint3d center = { 10.0,11.0,12.0 };
    PoseType pose(center, rotation);
    PhotoPtr->SetPose(pose);
    PhotoPtr->SetPhotoId(42);
    CameraElementId cameraId = PhotoPtr->GetCameraId();
    ASSERT_TRUE(cameraId.IsValid());


    //Update Photo element
    auto PhotoUpdatedPtr = PhotoPtr->Update();
    ASSERT_TRUE(PhotoUpdatedPtr.IsValid());

    PhotoElementId PhotoUpdatedId = PhotoUpdatedPtr->GetId();
    ASSERT_TRUE(PhotoUpdatedId.IsValid());
    //Updating don't change id...
    ASSERT_TRUE(PhotoUpdatedId == PhotoId);

    //Save changes
    DbResult result = projectPtr->SaveChanges("BasicPhoto");
    EXPECT_EQ(BE_SQLITE_OK, result) << "Save Photo failed";

    //Close project to flush memory
    PhotoPtr = nullptr;//release our element before closing project, otherwise we get an assert in closeDb.
    PhotoUpdatedPtr= nullptr;
    CloseProject();

    //Reopen project
    DgnDbPtr projectReopenedPtr = OpenProject(L"ModifyPhoto.dgndb");
    ASSERT_TRUE(projectReopenedPtr.IsValid());

    ASSERT_TRUE(projectReopenedPtr->Elements().GetElement(PhotoId).IsValid());
    PhotoCPtr myPhotoPtr = Photo::Get(*projectReopenedPtr,PhotoId);
    ASSERT_TRUE(myPhotoPtr.IsValid());
    ASSERT_EQ(PhotoId, myPhotoPtr->GetElementId());
    CameraElementId cameraIdRead = myPhotoPtr->GetCameraId();
    ASSERT_TRUE(cameraIdRead.IsValid());
    ASSERT_EQ(cameraIdRead, cameraId);


    //read back Photo properties and check if equal
    ASSERT_DOUBLE_EQ(myPhotoPtr->GetPhotoId(), 42);
    ASSERT_TRUE(pose.IsEqual(myPhotoPtr->GetPose()));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataCaptureTests, DeletePhoto)
    {
    DgnDbPtr projectPtr = CreateProject(L"DeletePhoto.dgndb");
    ASSERT_TRUE(projectPtr.IsValid());
    Utf8String cameraLabel = "BasicCamera";
    CreateSamplePhotoProjectWithCamera(*projectPtr, cameraLabel.c_str());


    DgnModelId spatialModelId = QueryFirstSpatialModelId(*projectPtr);
    DgnModelPtr spatialModelPtr =projectPtr->Models().GetModel(spatialModelId);
    ASSERT_TRUE(spatialModelPtr.IsValid());
    ASSERT_TRUE(spatialModelPtr->IsSpatialModel());

    // Query Photo element
    Utf8String photoLabel(Utf8PrintfString("BasicPhoto%d", 1));
    DgnElementId PhotoId  = Photo::QueryForIdByLabel(*projectPtr,photoLabel.c_str());
    ASSERT_TRUE(PhotoId.IsValid());

    //Delete edited Photo element - You CANNOT delete an edited Photo element because you get a copy of the original ...
    //Delete is merely a shortcut for el.GetDgnDb().Elements().Delete(el);
    PhotoCPtr PhotoEditedPtr = Photo::GetForEdit(*projectPtr, PhotoId);
    ASSERT_TRUE(PhotoEditedPtr.IsValid());
    DgnDbStatus status = PhotoEditedPtr->Delete();
    ASSERT_FALSE(status==DgnDbStatus::Success);

    //Delete Photo element - You CAN delete a const Photo element because this is effectively the original element...
    //Delete is merely a shortcut for el.GetDgnDb().Elements().Delete(el);
    PhotoCPtr PhotoPtr = Photo::Get(*projectPtr, PhotoId);
    ASSERT_TRUE(PhotoPtr.IsValid());
    status = PhotoPtr->Delete();
    ASSERT_TRUE(status == DgnDbStatus::Success);


    //Save changes
    DbResult result = projectPtr->SaveChanges("BasicPhoto");
    EXPECT_EQ(BE_SQLITE_OK, result) << "Save Photo failed";

    //Close project to flush memory
    PhotoPtr = nullptr;//release our element before closing project, otherwise we get an assert in closeDb.
    PhotoEditedPtr=nullptr;
    CloseProject();

    //Reopen project
    DgnDbPtr projectReopenedPtr = OpenProject(L"DeletePhoto.dgndb");
    ASSERT_TRUE(projectReopenedPtr.IsValid());

    //Check that PhotoId is not accessible anymore 
    ASSERT_FALSE(projectReopenedPtr->Elements().GetElement(PhotoId).IsValid());
    PhotoCPtr myPhotoPtr = Photo::Get(*projectReopenedPtr,PhotoId);
    ASSERT_FALSE(myPhotoPtr.IsValid());

    // Check that query Photo element returns nothing
    DgnElementId deletedPhotoId = Photo::QueryForIdByLabel(*projectReopenedPtr, photoLabel.c_str());
    ASSERT_FALSE(deletedPhotoId.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataCaptureTests, QueryPhotosFromCamera)
    {
    DgnDbPtr projectPtr = CreateProject(L"QueryPhotosFromCamera.dgndb");
    ASSERT_TRUE(projectPtr.IsValid());
    Utf8String cameraLabel = "SampleCamera";
    CreateSamplePhotoProjectWithCamera(*projectPtr,cameraLabel.c_str());

    // Query Camera element
    CameraElementId cameraId = Camera::QueryForIdByLabel(*projectPtr, cameraLabel.c_str());
    ASSERT_TRUE(cameraId.IsValid());
    //Test iterator over all photos from this camera
    int photoCount(0);
    for (Camera::PhotoEntry const& photo : Camera::MakePhotoIterator(*projectPtr, cameraId))
        {
        PhotoCPtr myPhotoPtr = Photo::Get(*projectPtr,photo.GePhotoElementId());
        ASSERT_TRUE(myPhotoPtr->GetCameraId()==cameraId);
        ASSERT_EQ(myPhotoPtr->GetPhotoId(), photoCount);
        photoCount++;
        }
    ASSERT_EQ(photoCount,10);

    Utf8String cameraLabel2 = "SampleCamera2";
    CreateSamplePhotoProjectWithCamera(*projectPtr, cameraLabel2.c_str());
    // Query Camera element
    CameraElementId camera2Id = Camera::QueryForIdByLabel(*projectPtr, cameraLabel2.c_str());
    ASSERT_TRUE(camera2Id.IsValid());
    //Test iterator over all photos from this second camera and make changes
    int photoCount2(0);
    for (Camera::PhotoEntry const& photo : Camera::MakePhotoIterator(*projectPtr, camera2Id))
        {
        PhotoPtr myPhotoPtr = Photo::GetForEdit(*projectPtr,photo.GePhotoElementId());
        ASSERT_TRUE(myPhotoPtr->GetCameraId()==camera2Id);
        ASSERT_EQ(myPhotoPtr->GetPhotoId(), photoCount2);
        myPhotoPtr->SetCameraId(cameraId);
        myPhotoPtr->Update();
        photoCount2++;
        }
    ASSERT_EQ(photoCount2,10);


    // Query Camera element
    //Test iterator over all photos from this camera
    int photoCount3(0);
    for (Camera::PhotoEntry const& photo : Camera::MakePhotoIterator(*projectPtr, cameraId))
        {
        PhotoCPtr myPhotoPtr = Photo::Get(*projectPtr, photo.GePhotoElementId());
        ASSERT_TRUE(myPhotoPtr->GetCameraId() == cameraId);
        photoCount3++;
        }
    //All photos was changed to this camera, we should have now 10+10=20 photos.
    ASSERT_EQ(photoCount3, 20);


    //Test iterator over all photos from this camera
    int photoCount4(0);
    for (Camera::PhotoEntry const& photo : Camera::MakePhotoIterator(*projectPtr, cameraId))
        {
        PhotoCPtr myPhotoPtr = Photo::Get(*projectPtr, photo.GePhotoElementId());
        ASSERT_TRUE(myPhotoPtr->GetCameraId() == cameraId);
        //delete them all
        myPhotoPtr->Delete();
        photoCount4++;
        }
    //All photos was deleted from this camera
    ASSERT_EQ(photoCount4, 20);

    //Test iterator over all photos from this camera
    int photoCount5(0);
    for (Camera::PhotoEntry const& photo : Camera::MakePhotoIterator(*projectPtr, cameraId))
        {
        PhotoCPtr myPhotoPtr = Photo::Get(*projectPtr, photo.GePhotoElementId());
        ASSERT_TRUE(myPhotoPtr->GetCameraId() == cameraId);
        //delete them all
        myPhotoPtr->Delete();
        photoCount5++;
        }
    //All photos was deleted from this camera
    ASSERT_EQ(photoCount5, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataCaptureTests, ImportXMLFile)
    {
    DgnDbPtr projectPtr = CreateProject(L"ImportXMLFile.dgndb");
    ASSERT_TRUE(projectPtr.IsValid());

    DgnModelId spatialModelId = QueryFirstSpatialModelId(*projectPtr);
    DgnModelPtr modelPtr = projectPtr->Models().GetModel(spatialModelId);
    ASSERT_TRUE(modelPtr.IsValid());
    ASSERT_TRUE(modelPtr->IsSpatialModel());
    SpatialModelPtr spatialModelPtr =  modelPtr->ToSpatialModelP();


    BeFileName assetsDirectory = GetHost().GetDgnPlatformAssetsDirectory();
    BeFileName xmlFileName = assetsDirectory;
    xmlFileName.AppendToPath(L"TestFiles/BLOCK_DEF_SOL_AERIEN.xml");

    XmlReader myReader(*spatialModelPtr);
    ASSERT_EQ(SUCCESS, myReader.ReadXml(xmlFileName));

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CCPhotoPlannerTests, ImportXMLFileFormat2)
    {
    DgnDbPtr projectPtr = CreateProject(L"ImportXMLFile.dgndb");
    ASSERT_TRUE(projectPtr.IsValid());

    DgnModelId spatialModelId = QueryFirstSpatialModelId(*projectPtr);
    DgnModelPtr modelPtr = projectPtr->Models().GetModel(spatialModelId);
    ASSERT_TRUE(modelPtr.IsValid());
    ASSERT_TRUE(modelPtr->IsSpatialModel());
    SpatialModelPtr spatialModelPtr =  modelPtr->ToSpatialModelP();


    BeFileName assetsDirectory = GetHost().GetDgnPlatformAssetsDirectory();
    BeFileName xmlFileName = assetsDirectory;
    xmlFileName.AppendToPath(L"TestFiles/myBlock.xml");

    XmlReader myReader(*spatialModelPtr);
    ASSERT_EQ(SUCCESS, myReader.ReadXml(xmlFileName));

    }