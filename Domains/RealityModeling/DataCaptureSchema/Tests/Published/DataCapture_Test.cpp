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
TEST_F(DataCaptureTests, CreateCameraDevice)
    {
    DgnDbPtr projectPtr = CreateProject(L"CreateCameraDevice.dgndb");
    ASSERT_TRUE(projectPtr.IsValid());

    DgnModelId spatialModelId = QueryFirstSpatialModelId(*projectPtr);
    DgnModelPtr spatialModelPtr =projectPtr->Models().GetModel(spatialModelId);
    ASSERT_TRUE(spatialModelPtr.IsValid());
    ASSERT_TRUE(spatialModelPtr->IsSpatialModel());
    DgnModelId definitionModelId = QueryFirstDefinitionModelId(*projectPtr);
    DgnModelPtr definitonModelPtr = projectPtr->Models().GetModel(definitionModelId);
    DefinitionModelP definitonModelP = definitonModelPtr->ToDefinitionModelP();
    ASSERT_TRUE(definitonModelPtr.IsValid());
    ASSERT_TRUE(definitonModelPtr->IsDefinitionModel());

    // Create CameraDevice
    auto cameraDeviceModelPtr = CameraDeviceModel::Create(*definitonModelP);
    cameraDeviceModelPtr->Insert();

    SpatialModelP spatialModelP = spatialModelPtr->ToSpatialModelP();
    auto cameraDevicePtr = CameraDevice::Create(*spatialModelP,cameraDeviceModelPtr->GetId());
    ASSERT_TRUE(cameraDevicePtr.IsValid());

    //Change cameraDevice properties
    cameraDevicePtr->SetLabel("BasicCameraDevice1");
    cameraDevicePtr->SetFocalLength(4798.35);
    cameraDevicePtr->SetImageWidth(5456);
    cameraDevicePtr->SetImageHeight(3632);
    DPoint2d principalPoint={2677.8,1772};
    cameraDevicePtr->SetPrincipalPoint(principalPoint);
    RadialDistortionPtr  pRadialDistortion = RadialDistortion::Create(1, 2, 3);
    TangentialDistortionPtr  pTangentialDistortion = TangentialDistortion::Create(4, 5);
    cameraDevicePtr->SetRadialDistortion(pRadialDistortion.get());
    cameraDevicePtr->SetTangentialDistortion(pTangentialDistortion.get());
    cameraDevicePtr->SetAspectRatio(1.0);
    cameraDevicePtr->SetSkew(1.0);

    //Insert cameraDevice element
    auto cameraDeviceInsertedPtr = cameraDevicePtr->Insert();
    ASSERT_TRUE(cameraDeviceInsertedPtr.IsValid());
    CameraDeviceElementId cameraDeviceId = cameraDeviceInsertedPtr->GetId();
    ASSERT_TRUE(cameraDeviceId.IsValid());

    //Save changes
    DbResult result = projectPtr->SaveChanges("BasicCameraDevice");
    EXPECT_EQ(BE_SQLITE_OK, result) << "Save CameraDevice failed";

    //Close project to flush memory
    cameraDevicePtr = nullptr;//release our element before closing project, otherwise we get an assert in closeDb.
    cameraDeviceModelPtr=nullptr;
    cameraDeviceInsertedPtr=nullptr;
    CloseProject();

    //Reopen project
    DgnDbPtr projectReopenedPtr = OpenProject(L"CreateCameraDevice.dgndb");
    ASSERT_TRUE(projectReopenedPtr.IsValid());

    ASSERT_TRUE(projectReopenedPtr->Elements().GetElement(cameraDeviceId).IsValid());
    CameraDeviceCPtr myCamPtr = CameraDevice::Get(*projectReopenedPtr,cameraDeviceId);
    ASSERT_TRUE(myCamPtr.IsValid());
    ASSERT_EQ(cameraDeviceId, myCamPtr->GetElementId());

    //read back cameraDevice properties and check if equal
    ASSERT_DOUBLE_EQ(myCamPtr->GetFocalLength(),4798.35);
    ASSERT_EQ(5456,myCamPtr->GetImageWidth());
    ASSERT_EQ(3632,myCamPtr->GetImageHeight());
    ASSERT_TRUE(principalPoint.IsEqual(myCamPtr->GetPrincipalPoint()));
    ASSERT_TRUE(nullptr != myCamPtr->GetRadialDistortion());
    ASSERT_TRUE(pRadialDistortion->IsEqual(*(myCamPtr->GetRadialDistortion())));
    ASSERT_TRUE(nullptr != myCamPtr->GetTangentialDistortion());
    ASSERT_TRUE(pTangentialDistortion->IsEqual(*(myCamPtr->GetTangentialDistortion())));
    ASSERT_DOUBLE_EQ(myCamPtr->GetAspectRatio(),1.0);
    ASSERT_DOUBLE_EQ(myCamPtr->GetSkew(),1.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataCaptureTests, ModifyCameraDevice)
    {
    DgnDbPtr projectPtr = CreateProject(L"ModifyCameraDevice.dgndb");
    ASSERT_TRUE(projectPtr.IsValid());
    Utf8String cameraDeviceLabel = "CameraDeviceToModify";
    CreateSamplePhotoProjectWithCameraDevice(*projectPtr, cameraDeviceLabel.c_str());


    DgnModelId spatialModelId = QueryFirstSpatialModelId(*projectPtr);
    DgnModelPtr spatialModelPtr =projectPtr->Models().GetModel(spatialModelId);
    ASSERT_TRUE(spatialModelPtr.IsValid());
    ASSERT_TRUE(spatialModelPtr->IsSpatialModel());
    DgnModelId definitionModelId = QueryFirstDefinitionModelId(*projectPtr);
    DgnModelPtr definitonModelPtr = projectPtr->Models().GetModel(definitionModelId);
    ASSERT_TRUE(definitonModelPtr.IsValid());
    ASSERT_TRUE(definitonModelPtr->IsDefinitionModel());

    // Query CameraDevice element
    DgnElementId cameraDeviceId  = CameraDevice::QueryForIdByLabel(*projectPtr,cameraDeviceLabel.c_str());
    ASSERT_TRUE(cameraDeviceId.IsValid());
    CameraDevicePtr cameraDevicePtr = CameraDevice::GetForEdit(*projectPtr, cameraDeviceId);
    ASSERT_TRUE(cameraDevicePtr.IsValid());

    //Change cameraDevice properties
    cameraDevicePtr->SetFocalLength(12);
    cameraDevicePtr->SetImageWidth(13);
    cameraDevicePtr->SetImageHeight(14);
    DPoint2d principalPoint={15,16};
    cameraDevicePtr->SetPrincipalPoint(principalPoint);
    RadialDistortionPtr  pRadialDistortion = RadialDistortion::Create(11,12,13);
    TangentialDistortionPtr  pTangentialDistortion = TangentialDistortion::Create(14,15);
    cameraDevicePtr->SetRadialDistortion(pRadialDistortion.get());
    cameraDevicePtr->SetTangentialDistortion(pTangentialDistortion.get());
    cameraDevicePtr->SetAspectRatio(2.0);
    cameraDevicePtr->SetSkew(3.0);

    //Update cameraDevice element
    auto cameraDeviceUpdatedPtr = cameraDevicePtr->Update();
    ASSERT_TRUE(cameraDeviceUpdatedPtr.IsValid());

    CameraDeviceElementId cameraDeviceUpdatedId = cameraDeviceUpdatedPtr->GetId();
    ASSERT_TRUE(cameraDeviceUpdatedId.IsValid());
    //Updating don't change id...
    ASSERT_TRUE(cameraDeviceUpdatedId == cameraDeviceId);

    //Save changes
    DbResult result = projectPtr->SaveChanges("BasicCameraDevice");
    EXPECT_EQ(BE_SQLITE_OK, result) << "Save CameraDevice failed";

    //Close project to flush memory
    cameraDevicePtr = nullptr;//release our element before closing project, otherwise we get an assert in closeDb.
    cameraDeviceUpdatedPtr= nullptr;
    CloseProject();

    //Reopen project
    DgnDbPtr projectReopenedPtr = OpenProject(L"ModifyCameraDevice.dgndb");
    ASSERT_TRUE(projectReopenedPtr.IsValid());

    ASSERT_TRUE(projectReopenedPtr->Elements().GetElement(cameraDeviceId).IsValid());
    CameraDeviceCPtr myCamPtr = CameraDevice::Get(*projectReopenedPtr,cameraDeviceId);
    ASSERT_TRUE(myCamPtr.IsValid());
    ASSERT_EQ(cameraDeviceId, myCamPtr->GetElementId());

    //read back cameraDevice properties and check if equal
    ASSERT_DOUBLE_EQ(myCamPtr->GetFocalLength(),12);
    ASSERT_EQ(13, myCamPtr->GetImageWidth());
    ASSERT_EQ(14, myCamPtr->GetImageHeight());
    ASSERT_TRUE(principalPoint.IsEqual(myCamPtr->GetPrincipalPoint()));
    ASSERT_TRUE(nullptr != myCamPtr->GetRadialDistortion());
    ASSERT_TRUE(pRadialDistortion->IsEqual(*(myCamPtr->GetRadialDistortion())));
    ASSERT_TRUE(nullptr != myCamPtr->GetTangentialDistortion());
    ASSERT_TRUE(pTangentialDistortion->IsEqual(*(myCamPtr->GetTangentialDistortion())));
    ASSERT_DOUBLE_EQ(myCamPtr->GetAspectRatio(),2.0);
    ASSERT_DOUBLE_EQ(myCamPtr->GetSkew(),3.0);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataCaptureTests, DeleteCameraDevice)
    {
    DgnDbPtr projectPtr = CreateProject(L"DeleteCameraDevice.dgndb");
    ASSERT_TRUE(projectPtr.IsValid());
    Utf8String cameraDeviceLabel = "CameraDeviceToDelete";
    CreateSamplePhotoProjectWithCameraDevice(*projectPtr, cameraDeviceLabel.c_str());

    // Query CameraDevice element
    CameraDeviceElementId cameraDeviceId = CameraDevice::QueryForIdByLabel(*projectPtr, cameraDeviceLabel.c_str());
    ASSERT_TRUE(cameraDeviceId.IsValid());


    //Delete edited cameraDevice element - You CANNOT delete an edited cameraDevice element because you get a copy of the original ...
    //Delete is merely a shortcut for el.GetDgnDb().Elements().Delete(el);
    CameraDeviceCPtr cameraDeviceEditedPtr = CameraDevice::GetForEdit(*projectPtr, cameraDeviceId);
    ASSERT_TRUE(cameraDeviceEditedPtr.IsValid());
    DgnDbStatus status = cameraDeviceEditedPtr->Delete();
    ASSERT_FALSE(status==DgnDbStatus::Success);

    //Delete cameraDevice element - You CAN delete a const cameraDevice element because this is effectively the original element...
    //Delete is merely a shortcut for el.GetDgnDb().Elements().Delete(el);
    CameraDeviceCPtr cameraDevicePtr = CameraDevice::Get(*projectPtr, cameraDeviceId);
    ASSERT_TRUE(cameraDevicePtr.IsValid());
    status = cameraDevicePtr->Delete();
    ASSERT_TRUE(status == DgnDbStatus::Success);


    //Save changes
    DbResult result = projectPtr->SaveChanges("BasicCameraDevice");
    EXPECT_EQ(BE_SQLITE_OK, result) << "Save CameraDevice failed";

    //Close project to flush memory
    cameraDevicePtr = nullptr;//release our element before closing project, otherwise we get an assert in closeDb.
    cameraDeviceEditedPtr=nullptr;
    CloseProject();

    //Reopen project
    DgnDbPtr projectReopenedPtr = OpenProject(L"DeleteCameraDevice.dgndb");
    ASSERT_TRUE(projectReopenedPtr.IsValid());

    //Check that cameraDeviceId is not accessible anymore 
    ASSERT_FALSE(projectReopenedPtr->Elements().GetElement(cameraDeviceId).IsValid());
    CameraDeviceCPtr myCamPtr = CameraDevice::Get(*projectReopenedPtr,cameraDeviceId);
    ASSERT_FALSE(myCamPtr.IsValid());

    // Check that query CameraDevice element returns nothing
    DgnElementId deletedCameraDeviceId = CameraDevice::QueryForIdByLabel(*projectReopenedPtr, cameraDeviceLabel.c_str());
    ASSERT_FALSE(deletedCameraDeviceId.IsValid());
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
    DgnModelId definitionModelId = QueryFirstDefinitionModelId(*projectPtr);
    DgnModelPtr definitonModelPtr = projectPtr->Models().GetModel(definitionModelId);
    DefinitionModelP definitonModelP = definitonModelPtr->ToDefinitionModelP();
    ASSERT_TRUE(definitonModelPtr.IsValid());
    ASSERT_TRUE(definitonModelPtr->IsDefinitionModel());

    // Create CameraDevice
    auto cameraDeviceModelPtr = CameraDeviceModel::Create(*definitonModelP);
    cameraDeviceModelPtr->Insert();

    auto cameraDevicePtr = CameraDevice::Create(*spatialModelP,cameraDeviceModelPtr->GetId());
    ASSERT_TRUE(cameraDevicePtr.IsValid());
    cameraDevicePtr->SetLabel("BasicCameraDevice1");
    cameraDevicePtr->SetFocalLength(4798.35);
    cameraDevicePtr->SetImageWidth(5456);
    cameraDevicePtr->SetImageHeight(3632);
    DPoint2d principalPoint = { 2677.8,1772 };
    cameraDevicePtr->SetPrincipalPoint(principalPoint);
    RadialDistortionPtr  pRadialDistortion = RadialDistortion::Create(1, 2, 3);
    TangentialDistortionPtr  pTangentialDistortion = TangentialDistortion::Create(4, 5);
    cameraDevicePtr->SetRadialDistortion(pRadialDistortion.get());
    cameraDevicePtr->SetTangentialDistortion(pTangentialDistortion.get());
    cameraDevicePtr->SetAspectRatio(1.0);
    cameraDevicePtr->SetSkew(1.0);
    auto cameraDeviceInsertedPtr = cameraDevicePtr->Insert();
    ASSERT_TRUE(cameraDeviceInsertedPtr.IsValid());
    CameraDeviceElementId cameraDeviceId = cameraDeviceInsertedPtr->GetId();
    ASSERT_TRUE(cameraDeviceId.IsValid());


    // Create Photo for the cameraDevice
    auto PhotoPtr = Photo::Create(*spatialModelP,cameraDeviceId);
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
    CameraDeviceElementId cameraDeviceIdRead = PhotoInsertedPtr->GetCameraDeviceId();
    ASSERT_TRUE(cameraDeviceIdRead.IsValid());
    ASSERT_EQ(cameraDeviceIdRead, cameraDeviceId);

    //Save changes
    DbResult result = projectPtr->SaveChanges("BasicPhoto");
    EXPECT_EQ(BE_SQLITE_OK, result) << "Save Photo failed";

    //Close project to flush memory
    PhotoPtr=nullptr;//release our element before closing project, otherwise we get an assert in closeDb.
    PhotoInsertedPtr=nullptr;
    cameraDevicePtr=nullptr;
    cameraDeviceModelPtr = nullptr;
    cameraDeviceInsertedPtr=nullptr;
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
    Utf8String cameraDeviceLabel = "BasicCameraDevice";
    CreateSamplePhotoProjectWithCameraDevice(*projectPtr, cameraDeviceLabel.c_str());


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
    CameraDeviceElementId cameraDeviceId = PhotoPtr->GetCameraDeviceId();
    ASSERT_TRUE(cameraDeviceId.IsValid());


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
    CameraDeviceElementId cameraDeviceIdRead = myPhotoPtr->GetCameraDeviceId();
    ASSERT_TRUE(cameraDeviceIdRead.IsValid());
    ASSERT_EQ(cameraDeviceIdRead, cameraDeviceId);


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
    Utf8String cameraDeviceLabel = "BasicCameraDevice";
    CreateSamplePhotoProjectWithCameraDevice(*projectPtr, cameraDeviceLabel.c_str());


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
TEST_F(DataCaptureTests, QueryPhotosFromCameraDevice)
    {
    DgnDbPtr projectPtr = CreateProject(L"QueryPhotosFromCameraDevice.dgndb");
    ASSERT_TRUE(projectPtr.IsValid());
    Utf8String cameraDeviceLabel = "SampleCameraDevice";
    CreateSamplePhotoProjectWithCameraDevice(*projectPtr,cameraDeviceLabel.c_str());

    // Query CameraDevice element
    CameraDeviceElementId cameraDeviceId = CameraDevice::QueryForIdByLabel(*projectPtr, cameraDeviceLabel.c_str());
    ASSERT_TRUE(cameraDeviceId.IsValid());
    //Test iterator over all photos from this cameraDevice
    int photoCount(0);
    for (CameraDevice::PhotoEntry const& photo : CameraDevice::MakePhotoIterator(*projectPtr, cameraDeviceId))
        {
        PhotoCPtr myPhotoPtr = Photo::Get(*projectPtr,photo.GePhotoElementId());
        ASSERT_TRUE(myPhotoPtr->GetCameraDeviceId()==cameraDeviceId);
        ASSERT_EQ(myPhotoPtr->GetPhotoId(), photoCount);
        photoCount++;
        }
    ASSERT_EQ(photoCount,10);

    Utf8String cameraDeviceLabel2 = "SampleCameraDevice2";
    CreateSamplePhotoProjectWithCameraDevice(*projectPtr, cameraDeviceLabel2.c_str());
    // Query CameraDevice element
    CameraDeviceElementId cameraDevice2Id = CameraDevice::QueryForIdByLabel(*projectPtr, cameraDeviceLabel2.c_str());
    ASSERT_TRUE(cameraDevice2Id.IsValid());
    //Test iterator over all photos from this second cameraDevice and make changes
    int photoCount2(0);
    for (CameraDevice::PhotoEntry const& photo : CameraDevice::MakePhotoIterator(*projectPtr, cameraDevice2Id))
        {
        PhotoPtr myPhotoPtr = Photo::GetForEdit(*projectPtr,photo.GePhotoElementId());
        ASSERT_TRUE(myPhotoPtr->GetCameraDeviceId()==cameraDevice2Id);
        ASSERT_EQ(myPhotoPtr->GetPhotoId(), photoCount2);
        myPhotoPtr->SetCameraDeviceId(cameraDeviceId);
        myPhotoPtr->Update();
        photoCount2++;
        }
    ASSERT_EQ(photoCount2,10);


    // Query CameraDevice element
    //Test iterator over all photos from this cameraDevice
    int photoCount3(0);
    for (CameraDevice::PhotoEntry const& photo : CameraDevice::MakePhotoIterator(*projectPtr, cameraDeviceId))
        {
        PhotoCPtr myPhotoPtr = Photo::Get(*projectPtr, photo.GePhotoElementId());
        ASSERT_TRUE(myPhotoPtr->GetCameraDeviceId() == cameraDeviceId);
        photoCount3++;
        }
    //All photos was changed to this cameraDevice, we should have now 10+10=20 photos.
    ASSERT_EQ(photoCount3, 20);


    //Test iterator over all photos from this cameraDevice
    int photoCount4(0);
    for (CameraDevice::PhotoEntry const& photo : CameraDevice::MakePhotoIterator(*projectPtr, cameraDeviceId))
        {
        PhotoCPtr myPhotoPtr = Photo::Get(*projectPtr, photo.GePhotoElementId());
        ASSERT_TRUE(myPhotoPtr->GetCameraDeviceId() == cameraDeviceId);
        //delete them all
        myPhotoPtr->Delete();
        photoCount4++;
        }
    //All photos was deleted from this cameraDevice
    ASSERT_EQ(photoCount4, 20);

    //Test iterator over all photos from this cameraDevice
    int photoCount5(0);
    for (CameraDevice::PhotoEntry const& photo : CameraDevice::MakePhotoIterator(*projectPtr, cameraDeviceId))
        {
        PhotoCPtr myPhotoPtr = Photo::Get(*projectPtr, photo.GePhotoElementId());
        ASSERT_TRUE(myPhotoPtr->GetCameraDeviceId() == cameraDeviceId);
        //delete them all
        myPhotoPtr->Delete();
        photoCount5++;
        }
    //All photos was deleted from this cameraDevice
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
    DgnModelId definitionModelId = QueryFirstDefinitionModelId(*projectPtr);
    DgnModelPtr definitonModelPtr = projectPtr->Models().GetModel(definitionModelId);
    ASSERT_TRUE(definitonModelPtr.IsValid());
    ASSERT_TRUE(definitonModelPtr->IsDefinitionModel());


    BeFileName assetsDirectory = GetHost().GetDgnPlatformAssetsDirectory();
    BeFileName xmlFileName = assetsDirectory;
    xmlFileName.AppendToPath(L"TestFiles/BLOCK_DEF_SOL_AERIEN.xml");

    XmlReader myReader(*spatialModelPtr, *(definitonModelPtr->ToDefinitionModelP()));
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
    DgnModelId definitionModelId = QueryFirstDefinitionModelId(*projectPtr);
    DgnModelPtr definitonModelPtr = projectPtr->Models().GetModel(definitionModelId);
    ASSERT_TRUE(definitonModelPtr.IsValid());
    ASSERT_TRUE(definitonModelPtr->IsDefinitionModel());


    BeFileName assetsDirectory = GetHost().GetDgnPlatformAssetsDirectory();
    BeFileName xmlFileName = assetsDirectory;
    xmlFileName.AppendToPath(L"TestFiles/myBlock.xml");

    XmlReader myReader(*spatialModelPtr,*(definitonModelPtr->ToDefinitionModelP()));
    ASSERT_EQ(SUCCESS, myReader.ReadXml(xmlFileName));

    }