/*--------------------------------------------------------------------------------------+
|
|     $Source: DataCaptureSchema/Tests/Published/DataCapture_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
    ASSERT_TRUE(cameraDeviceModelPtr.IsValid());

    SpatialModelP spatialModelP = spatialModelPtr->ToSpatialModelP();
    auto cameraDevicePtr = CameraDevice::Create(*spatialModelP,cameraDeviceModelPtr->GetId());
    ASSERT_TRUE(cameraDevicePtr.IsValid());

    //Change cameraDevice properties
    cameraDevicePtr->SetLabel("BasicCameraDevice1");
    cameraDevicePtr->SetFocalLength(0.00479835);
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
    cameraDevicePtr->SetSensorSize(1.0);

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
    ASSERT_DOUBLE_EQ(myCamPtr->GetFocalLength(),0.00479835);
    ASSERT_EQ(5456,myCamPtr->GetImageWidth());
    ASSERT_EQ(3632,myCamPtr->GetImageHeight());
    ASSERT_TRUE(principalPoint.IsEqual(myCamPtr->GetPrincipalPoint()));
    ASSERT_TRUE(nullptr != myCamPtr->GetRadialDistortion());
    ASSERT_TRUE(pRadialDistortion->IsEqual(*(myCamPtr->GetRadialDistortion())));
    ASSERT_TRUE(nullptr != myCamPtr->GetTangentialDistortion());
    ASSERT_TRUE(pTangentialDistortion->IsEqual(*(myCamPtr->GetTangentialDistortion())));
    ASSERT_DOUBLE_EQ(myCamPtr->GetAspectRatio(),1.0);
    ASSERT_DOUBLE_EQ(myCamPtr->GetSkew(),1.0);
    ASSERT_DOUBLE_EQ(myCamPtr->GetSensorSize(), 1.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataCaptureTests, ModifyCameraDevice)
    {
    DgnDbPtr projectPtr = CreateProject(L"ModifyCameraDevice.dgndb");
    ASSERT_TRUE(projectPtr.IsValid());
    Utf8String cameraDeviceLabel = "CameraDeviceToModify";
    CreateSampleShotProjectWithCameraDevice(*projectPtr, cameraDeviceLabel.c_str());


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
    cameraDevicePtr->SetSensorSize(1.0);

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
    ASSERT_DOUBLE_EQ(myCamPtr->GetSensorSize(), 1.0);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataCaptureTests, DeleteCameraDevice)
    {
    DgnDbPtr projectPtr = CreateProject(L"DeleteCameraDevice.dgndb");
    ASSERT_TRUE(projectPtr.IsValid());
    Utf8String cameraDeviceLabel = "CameraDeviceToDelete";
    CreateSampleShotProjectWithCameraDevice(*projectPtr, cameraDeviceLabel.c_str());

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
TEST_F(DataCaptureTests, CreateShot)
    {
    DgnDbPtr projectPtr = CreateProject(L"CreateShot.dgndb");
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
    cameraDevicePtr->SetFocalLength(0.00479835);
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
    cameraDevicePtr->SetSensorSize(1.0);
    auto cameraDeviceInsertedPtr = cameraDevicePtr->Insert();
    ASSERT_TRUE(cameraDeviceInsertedPtr.IsValid());
    CameraDeviceElementId cameraDeviceId = cameraDeviceInsertedPtr->GetId();
    ASSERT_TRUE(cameraDeviceId.IsValid());

    //Create a pose
    PosePtr pPose(Pose::Create(*spatialModelP));
    ASSERT_TRUE(pPose.IsValid());
    DPoint3d center = { 1.0,2.0,3.0 };
    pPose->SetCenter(center);
    Angle omega(Angle::FromDegrees(0));
    Angle phi(Angle::FromDegrees(0));
    Angle kappa(Angle::FromDegrees(0));
    pPose->SetOmega(omega);
    pPose->SetPhi(phi);
    pPose->SetKappa(kappa);
    auto poseInserted = pPose->Insert();
    ASSERT_TRUE(poseInserted.IsValid());
    PoseElementId poseId(pPose->GetId());

    // Create Photo for the cameraDevice
    auto ShotPtr = Shot::Create(*spatialModelP,cameraDeviceId, poseId);
    ASSERT_TRUE(ShotPtr.IsValid());

    //Change Photo properties
    ShotPtr->SetLabel("BasicPhoto1");
    //Don't set code - will test generate default code
//     DgnCode shotCode = Shot::CreateCode(*projectPtr, cameraDeviceInsertedPtr->GetCode().GetValue(), Utf8PrintfString("%d", 42));
//     ShotPtr->SetCode(shotCode);

    //Insert Shot element
    auto ShotInsertedPtr = ShotPtr->Insert();
    ASSERT_TRUE(ShotInsertedPtr.IsValid());
    ShotElementId ShotElementId = ShotInsertedPtr->GetId();
    ASSERT_TRUE(ShotElementId.IsValid());
    CameraDeviceElementId cameraDeviceIdRead = ShotInsertedPtr->GetCameraDeviceId();
    ASSERT_TRUE(cameraDeviceIdRead.IsValid());
    ASSERT_EQ(cameraDeviceIdRead, cameraDeviceId);

    //Save changes
    DbResult result = projectPtr->SaveChanges("BasicPhoto");
    EXPECT_EQ(BE_SQLITE_OK, result) << "Save Photo failed";

    //Close project to flush memory
    //release our element before closing project, otherwise we get an assert in closeDb.
    poseInserted=nullptr;
    pPose=nullptr;
    ShotPtr=nullptr;
    ShotInsertedPtr=nullptr;
    cameraDevicePtr=nullptr;
    cameraDeviceModelPtr = nullptr;
    cameraDeviceInsertedPtr=nullptr;
    CloseProject();

    //Reopen project
    DgnDbPtr projectReopenedPtr = OpenProject(L"CreateShot.dgndb");
    ASSERT_TRUE(projectReopenedPtr.IsValid());

    ASSERT_TRUE(projectReopenedPtr->Elements().GetElement(ShotElementId).IsValid());
    ShotCPtr myShotPtr = Shot::Get(*projectReopenedPtr,ShotElementId);
    ASSERT_TRUE(myShotPtr.IsValid());
    ASSERT_EQ(ShotElementId, myShotPtr->GetElementId());

    //Don't set code - will test generate default code
//     DgnCode myShotCode = myShotPtr->GetCode();
//     ASSERT_TRUE(myShotCode == shotCode);
    PoseCPtr myPosePtr = Pose::Get(*projectReopenedPtr, myShotPtr->GetPoseId());
    ASSERT_TRUE(myPosePtr.IsValid());
    ASSERT_EQ(poseId, myPosePtr->GetId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataCaptureTests, ModifyShot)
    {
    DgnDbPtr projectPtr = CreateProject(L"ModifyShot.dgndb");
    ASSERT_TRUE(projectPtr.IsValid());
    Utf8String cameraDeviceLabel = "BasicCameraDevice";
    CreateSampleShotProjectWithCameraDevice(*projectPtr, cameraDeviceLabel.c_str());


    DgnModelId spatialModelId = QueryFirstSpatialModelId(*projectPtr);
    DgnModelPtr spatialModelPtr =projectPtr->Models().GetModel(spatialModelId);
    ASSERT_TRUE(spatialModelPtr.IsValid());
    ASSERT_TRUE(spatialModelPtr->IsSpatialModel());


    // Query Shot element
    Utf8String photoLabel(Utf8PrintfString("BasicPhoto%d", 5));
    DgnElementId ShotId  = Shot::QueryForIdByLabel(*projectPtr,photoLabel.c_str());
    ASSERT_TRUE(ShotId.IsValid());
    ShotPtr shotPtr = Shot::GetForEdit(*projectPtr, ShotId);
    ASSERT_TRUE(shotPtr.IsValid());

    //Change Shot properties
    DPoint3d center = { 10.0,11.0,12.0 };
    PosePtr posePtr = Pose::GetForEdit(*projectPtr, shotPtr->GetPoseId());
    ASSERT_TRUE(posePtr.IsValid());

    posePtr->SetCenter(center);
    Angle omega(Angle::FromDegrees(10.0));
    Angle phi(Angle::FromDegrees(45.0));
    Angle kappa(Angle::FromDegrees(-20.0));
    posePtr->SetOmega(omega);
    posePtr->SetPhi(phi);
    posePtr->SetKappa(kappa);
    auto poseUpdatedPtr = posePtr->Update();
    ASSERT_TRUE(poseUpdatedPtr.IsValid());

    CameraDeviceElementId cameraDeviceId = shotPtr->GetCameraDeviceId();
    ASSERT_TRUE(cameraDeviceId.IsValid());
    CameraDeviceCPtr cameraDevicePtr = CameraDevice::Get(*projectPtr, cameraDeviceId);

    DgnCode shotCode = Shot::CreateCode(*projectPtr, cameraDevicePtr->GetCode().GetValue(), Utf8PrintfString("%d", 42));
    shotPtr->SetCode(shotCode);

    //Update Shot element
    auto ShotUpdatedPtr = shotPtr->Update();
    ASSERT_TRUE(ShotUpdatedPtr.IsValid());

    ShotElementId ShotUpdatedId = ShotUpdatedPtr->GetId();
    ASSERT_TRUE(ShotUpdatedId.IsValid());
    //Updating don't change id...
    ASSERT_TRUE(ShotUpdatedId == ShotId);

    //Save changes
    DbResult result = projectPtr->SaveChanges("BasicPhoto");
    EXPECT_EQ(BE_SQLITE_OK, result) << "Save Photo failed";

    //Close project to flush memory
    shotPtr = nullptr;//release our element before closing project, otherwise we get an assert in closeDb.
    ShotUpdatedPtr= nullptr;
    cameraDevicePtr=nullptr;
    poseUpdatedPtr=nullptr;
    posePtr=nullptr;
    CloseProject();

    //Reopen project
    DgnDbPtr projectReopenedPtr = OpenProject(L"ModifyShot.dgndb");
    ASSERT_TRUE(projectReopenedPtr.IsValid());

    ASSERT_TRUE(projectReopenedPtr->Elements().GetElement(ShotId).IsValid());
    ShotCPtr myShotPtr = Shot::Get(*projectReopenedPtr,ShotId);
    ASSERT_TRUE(myShotPtr.IsValid());
    ASSERT_EQ(ShotId, myShotPtr->GetElementId());
    CameraDeviceElementId cameraDeviceIdRead = myShotPtr->GetCameraDeviceId();
    ASSERT_TRUE(cameraDeviceIdRead.IsValid());
    ASSERT_EQ(cameraDeviceIdRead, cameraDeviceId);

    PoseCPtr myPosePtr = Pose::Get(*projectReopenedPtr, myShotPtr->GetPoseId());
    ASSERT_TRUE(myPosePtr.IsValid());



    //read back Shot properties and check if equal
    DgnCode myShotCode = myShotPtr->GetCode();
    ASSERT_TRUE(myShotCode == shotCode);
    ASSERT_TRUE(omega.Radians() == myPosePtr->GetOmega().Radians());
    ASSERT_TRUE(phi.Radians()== myPosePtr->GetPhi().Radians());
    ASSERT_TRUE(kappa.Radians() == myPosePtr->GetKappa().Radians());
    ASSERT_TRUE(myPosePtr->GetCenter() == center);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataCaptureTests, DeleteShot)
    {
    DgnDbPtr projectPtr = CreateProject(L"DeleteShot.dgndb");
    ASSERT_TRUE(projectPtr.IsValid());
    Utf8String cameraDeviceLabel = "BasicCameraDevice";
    CreateSampleShotProjectWithCameraDevice(*projectPtr, cameraDeviceLabel.c_str());


    DgnModelId spatialModelId = QueryFirstSpatialModelId(*projectPtr);
    DgnModelPtr spatialModelPtr =projectPtr->Models().GetModel(spatialModelId);
    ASSERT_TRUE(spatialModelPtr.IsValid());
    ASSERT_TRUE(spatialModelPtr->IsSpatialModel());

    // Query Shot element
    Utf8String photoLabel(Utf8PrintfString("BasicPhoto%d", 1));
    DgnElementId ShotId  = Shot::QueryForIdByLabel(*projectPtr,photoLabel.c_str());
    ASSERT_TRUE(ShotId.IsValid());

    //Delete edited Shot element - You CANNOT delete an edited Shot element because you get a copy of the original ...
    //Delete is merely a shortcut for el.GetDgnDb().Elements().Delete(el);
    ShotCPtr ShotEditedPtr = Shot::GetForEdit(*projectPtr, ShotId);
    ASSERT_TRUE(ShotEditedPtr.IsValid());
    DgnDbStatus status = ShotEditedPtr->Delete();
    ASSERT_FALSE(status==DgnDbStatus::Success);

    //Delete Shot element - You CAN delete a const Shot element because this is effectively the original element...
    //Delete is merely a shortcut for el.GetDgnDb().Elements().Delete(el);
    ShotCPtr shotPtr = Shot::Get(*projectPtr, ShotId);
    ASSERT_TRUE(shotPtr.IsValid());
    status = shotPtr->Delete();
    ASSERT_TRUE(status == DgnDbStatus::Success);


    //Save changes
    DbResult result = projectPtr->SaveChanges("BasicPhoto");
    EXPECT_EQ(BE_SQLITE_OK, result) << "Save Photo failed";

    //Close project to flush memory
    shotPtr = nullptr;//release our element before closing project, otherwise we get an assert in closeDb.
    ShotEditedPtr=nullptr;
    CloseProject();

    //Reopen project
    DgnDbPtr projectReopenedPtr = OpenProject(L"DeleteShot.dgndb");
    ASSERT_TRUE(projectReopenedPtr.IsValid());

    //Check that ShotId is not accessible anymore 
    ASSERT_FALSE(projectReopenedPtr->Elements().GetElement(ShotId).IsValid());
    ShotCPtr myShotPtr = Shot::Get(*projectReopenedPtr,ShotId);
    ASSERT_FALSE(myShotPtr.IsValid());

    // Check that query Shot element returns nothing
    DgnElementId deletedShotId = Shot::QueryForIdByLabel(*projectReopenedPtr, photoLabel.c_str());
    ASSERT_FALSE(deletedShotId.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataCaptureTests, QueryShotFromCameraDevice)
    {
    DgnDbPtr projectPtr = CreateProject(L"QueryShotsFromCameraDevice.dgndb");
    ASSERT_TRUE(projectPtr.IsValid());
    Utf8String cameraDeviceLabel = "SampleCameraDevice";
    CreateSampleShotProjectWithCameraDevice(*projectPtr,cameraDeviceLabel.c_str());

    // Query CameraDevice element
    CameraDeviceElementId cameraDeviceId = CameraDevice::QueryForIdByLabel(*projectPtr, cameraDeviceLabel.c_str());
    ASSERT_TRUE(cameraDeviceId.IsValid());
    CameraDeviceCPtr cameraDevicePtr = CameraDevice::Get(*projectPtr, cameraDeviceId);

    //Test iterator over all Shots from this cameraDevice
    int photoCount(0);
    for (CameraDevice::ShotEntry const& photo : CameraDevice::MakeShotIterator(*projectPtr, cameraDeviceId))
        {
        ShotCPtr myShotPtr = Shot::Get(*projectPtr,photo.GeShotElementId());
        ASSERT_TRUE(myShotPtr->GetCameraDeviceId()==cameraDeviceId);
        DgnCode myShotCode = myShotPtr->GetCode();
        DgnCode shotCode = Shot::CreateCode(*projectPtr, cameraDevicePtr->GetCode().GetValue(), Utf8PrintfString("%d", photoCount));

        ASSERT_TRUE(myShotCode ==shotCode);
        photoCount++;
        }
    ASSERT_EQ(photoCount,10);

    Utf8String cameraDeviceLabel2 = "SampleCameraDevice2";
    CreateSampleShotProjectWithCameraDevice(*projectPtr, cameraDeviceLabel2.c_str());
    // Query CameraDevice element
    CameraDeviceElementId cameraDevice2Id = CameraDevice::QueryForIdByLabel(*projectPtr, cameraDeviceLabel2.c_str());
    ASSERT_TRUE(cameraDevice2Id.IsValid());
    CameraDeviceCPtr cameraDevicePtr2 = CameraDevice::Get(*projectPtr, cameraDevice2Id);

    //Test iterator over all Shots from this second cameraDevice and make changes
    int photoCount2(0);
    for (CameraDevice::ShotEntry const& photo : CameraDevice::MakeShotIterator(*projectPtr, cameraDevice2Id))
        {
        ShotPtr myShotPtr = Shot::GetForEdit(*projectPtr,photo.GeShotElementId());
        ASSERT_TRUE(myShotPtr->GetCameraDeviceId()==cameraDevice2Id);
        DgnCode myShotCode = myShotPtr->GetCode();
        DgnCode shotCode = Shot::CreateCode(*projectPtr, cameraDevicePtr2->GetCode().GetValue(), Utf8PrintfString("%d", photoCount2));
        ASSERT_TRUE(myShotCode == shotCode);
        myShotPtr->SetCameraDeviceId(cameraDeviceId);
        myShotPtr->Update();
        photoCount2++;
        }
    ASSERT_EQ(photoCount2,10);


    // Query CameraDevice element
    //Test iterator over all Shots from this cameraDevice
    int photoCount3(0);
    for (CameraDevice::ShotEntry const& photo : CameraDevice::MakeShotIterator(*projectPtr, cameraDeviceId))
        {
        ShotCPtr myShotPtr = Shot::Get(*projectPtr, photo.GeShotElementId());
        ASSERT_TRUE(myShotPtr->GetCameraDeviceId() == cameraDeviceId);
        photoCount3++;
        }
    //All photos was changed to this cameraDevice, we should have now 10+10=20 Shots.
    ASSERT_EQ(photoCount3, 20);


    //Test iterator over all Shots from this cameraDevice
    int photoCount4(0);
    for (CameraDevice::ShotEntry const& photo : CameraDevice::MakeShotIterator(*projectPtr, cameraDeviceId))
        {
        ShotCPtr myShotPtr = Shot::Get(*projectPtr, photo.GeShotElementId());
        ASSERT_TRUE(myShotPtr->GetCameraDeviceId() == cameraDeviceId);
        //delete them all
        myShotPtr->Delete();
        photoCount4++;
        }
    //All photos was deleted from this cameraDevice
    ASSERT_EQ(photoCount4, 20);

    //Test iterator over all photos from this cameraDevice
    int photoCount5(0);
    for (CameraDevice::ShotEntry const& photo : CameraDevice::MakeShotIterator(*projectPtr, cameraDeviceId))
        {
        ShotCPtr myShotPtr = Shot::Get(*projectPtr, photo.GeShotElementId());
        ASSERT_TRUE(myShotPtr->GetCameraDeviceId() == cameraDeviceId);
        //delete them all
        myShotPtr->Delete();
        photoCount5++;
        }
    //All photos was deleted from this cameraDevice
    ASSERT_EQ(photoCount5, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataCaptureTests, ImportXMLFileFormat1)
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
TEST_F(DataCaptureTests, ImportXMLFileFormat2)
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataCaptureTests, CreateDrone)
    {
    DgnDbPtr projectPtr = CreateProject(L"CreateDrone.dgndb");
    ASSERT_TRUE(projectPtr.IsValid());

    DgnModelId spatialModelId = QueryFirstSpatialModelId(*projectPtr);
    DgnModelPtr spatialModelPtr = projectPtr->Models().GetModel(spatialModelId);
    ASSERT_TRUE(spatialModelPtr.IsValid());
    ASSERT_TRUE(spatialModelPtr->IsSpatialModel());
    SpatialModelP spatialModelP = spatialModelPtr->ToSpatialModelP();

    DgnModelId definitionModelId = QueryFirstDefinitionModelId(*projectPtr);
    DgnModelPtr definitonModelPtr = projectPtr->Models().GetModel(definitionModelId);
    DefinitionModelP definitonModelP = definitonModelPtr->ToDefinitionModelP();
    ASSERT_TRUE(definitonModelPtr.IsValid());
    ASSERT_TRUE(definitonModelPtr->IsDefinitionModel());

    // Create GimbalAngleRange
    auto gimbalAngleRangePtr = GimbalAngleRange::Create(*spatialModelP);
    ASSERT_TRUE(gimbalAngleRangePtr.IsValid());

    gimbalAngleRangePtr->SetMinimumAngle(Angle::FromDegrees(20));
    gimbalAngleRangePtr->SetMaximumAngle(Angle::FromDegrees(40));

    auto gimbalAngleRangeInsertedPtr = gimbalAngleRangePtr->Insert();
    GimbalAngleRangeElementId gimbalAngleRangeElmId = gimbalAngleRangeInsertedPtr->GetId();

    // Create CameraDevice Model
    auto cameraDeviceModelPtr = CameraDeviceModel::Create(*definitonModelP);
    cameraDeviceModelPtr->Insert();
    ASSERT_TRUE(cameraDeviceModelPtr.IsValid());

    // Create Camera
    auto cameraDevicePtr = CameraDevice::Create(*spatialModelP, cameraDeviceModelPtr->GetId());
    ASSERT_TRUE(cameraDevicePtr.IsValid());

    cameraDevicePtr->SetLabel("BasicCameraDevice1");
    cameraDevicePtr->SetFocalLength(0.00479835);
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
    cameraDevicePtr->SetSensorSize(1.0);

    auto cameraDeviceInsertedPtr = cameraDevicePtr->Insert();
    ASSERT_TRUE(cameraDeviceInsertedPtr.IsValid());
    CameraDeviceElementId cameraDeviceId = cameraDeviceInsertedPtr->GetId();
    ASSERT_TRUE(cameraDeviceId.IsValid());

    // Create Gimbal
    auto gimbalPtr = Gimbal::Create(*spatialModelP);
    ASSERT_TRUE(gimbalPtr.IsValid());

    DgnElementIdSet gimbalAngleRangeSet;
    gimbalAngleRangeSet.insert(gimbalAngleRangeElmId);
    gimbalPtr->SetGimbalAngleRangeElementIdSet(gimbalAngleRangeSet);

    DgnElementIdSet cameraDeviceSet;
    cameraDeviceSet.insert(cameraDeviceId);
    gimbalPtr->SetCameraElementIdSet(cameraDeviceSet);

    auto gimbalInsertedPtr = gimbalPtr->Insert();
    GimbalElementId gimbalElmId = gimbalInsertedPtr->GetId();

    // Create Drone
    auto dronePtr = Drone::Create(*spatialModelP, gimbalElmId);
    ASSERT_TRUE(dronePtr.IsValid());

    auto droneInsertedPtr = dronePtr->Insert();
    DroneElementId droneElmId = droneInsertedPtr->GetId();

    //Save changes
    DbResult result = projectPtr->SaveChanges("BasicDrone");
    EXPECT_EQ(BE_SQLITE_OK, result) << "Save Drone failed";

    //Close project to flush memory
    cameraDevicePtr = nullptr;//release our element before closing project, otherwise we get an assert in closeDb.
    cameraDeviceModelPtr = nullptr;
    cameraDeviceInsertedPtr = nullptr;
    gimbalAngleRangePtr = nullptr;
    gimbalAngleRangeInsertedPtr = nullptr;
    gimbalPtr = nullptr;
    gimbalInsertedPtr = nullptr;
    dronePtr = nullptr;
    droneInsertedPtr = nullptr;

    CloseProject();

    //Reopen project
    DgnDbPtr projectReopenedPtr = OpenProject(L"CreateDrone.dgndb");
    ASSERT_TRUE(projectReopenedPtr.IsValid());

    ASSERT_TRUE(projectReopenedPtr->Elements().GetElement(droneElmId).IsValid());
    DroneCPtr myDronePtr = Drone::Get(*projectReopenedPtr, droneElmId);
    ASSERT_TRUE(myDronePtr.IsValid());
    ASSERT_EQ(droneElmId, myDronePtr->GetId());

    GimbalCPtr myGimbalPtr = Gimbal::Get(*projectReopenedPtr, myDronePtr->GetGimbalElementId());
    ASSERT_TRUE(myGimbalPtr.IsValid());
    ASSERT_EQ(gimbalElmId, myGimbalPtr->GetId());

    DgnElementIdSet cameraElementIdSet = myGimbalPtr->GetCameraElementIdSet();
    CameraDeviceCPtr myCameraDevicePtr = CameraDevice::Get(*projectReopenedPtr, *(cameraElementIdSet.begin()));
    ASSERT_TRUE(myCameraDevicePtr.IsValid());
    ASSERT_EQ(cameraDeviceId, myCameraDevicePtr->GetId());

    DgnElementIdSet gimbalAngleRangeIdSet = myGimbalPtr->GetGimbalAngleRangeElementIdSet();
    GimbalAngleRangeCPtr myGimbalAngleRangePtr = GimbalAngleRange::Get(*projectReopenedPtr, *(gimbalAngleRangeIdSet.begin()));
    ASSERT_TRUE(myGimbalAngleRangePtr.IsValid());
    ASSERT_EQ(gimbalAngleRangeElmId, myGimbalAngleRangePtr->GetId());

    //read back cameraDevice properties and check if equal
    ASSERT_DOUBLE_EQ(myCameraDevicePtr->GetFocalLength(), 0.00479835);
    ASSERT_EQ(5456, myCameraDevicePtr->GetImageWidth());
    ASSERT_EQ(3632, myCameraDevicePtr->GetImageHeight());
    ASSERT_TRUE(principalPoint.IsEqual(myCameraDevicePtr->GetPrincipalPoint()));
    ASSERT_TRUE(nullptr != myCameraDevicePtr->GetRadialDistortion());
    ASSERT_TRUE(pRadialDistortion->IsEqual(*(myCameraDevicePtr->GetRadialDistortion())));
    ASSERT_TRUE(nullptr != myCameraDevicePtr->GetTangentialDistortion());
    ASSERT_TRUE(pTangentialDistortion->IsEqual(*(myCameraDevicePtr->GetTangentialDistortion())));
    ASSERT_DOUBLE_EQ(myCameraDevicePtr->GetAspectRatio(), 1.0);
    ASSERT_DOUBLE_EQ(myCameraDevicePtr->GetSkew(), 1.0);
    ASSERT_DOUBLE_EQ(myCameraDevicePtr->GetSensorSize(), 1.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataCaptureTests, ModifyDrone)
    {
    DgnDbPtr projectPtr = CreateProject(L"ModifyDrone.dgndb");
    ASSERT_TRUE(projectPtr.IsValid());
    Utf8String cameraDeviceLabel = "BasicCameraDevice";
    CreateSampleDroneProjectWithCameraDevice(*projectPtr, cameraDeviceLabel.c_str());

    DgnModelId spatialModelId = QueryFirstSpatialModelId(*projectPtr);
    DgnModelPtr spatialModelPtr = projectPtr->Models().GetModel(spatialModelId);
    ASSERT_TRUE(spatialModelPtr.IsValid());
    ASSERT_TRUE(spatialModelPtr->IsSpatialModel());

    // Query Drone element
    Utf8String droneLabel(Utf8PrintfString("Drone1"));
    DgnElementId droneId = Drone::QueryForIdByLabel(*projectPtr, droneLabel.c_str());
    ASSERT_TRUE(droneId.IsValid());
    DronePtr dronePtr = Drone::GetForEdit(*projectPtr, droneId);
    ASSERT_TRUE(dronePtr.IsValid());

    // Change Drone properties
    DgnElementId gimbalId = dronePtr->GetGimbalElementId();
    GimbalPtr gimbalPtr = Gimbal::GetForEdit(*projectPtr, gimbalId);
    ASSERT_TRUE(gimbalPtr.IsValid());

    DgnElementIdSet gimbalAngleRangeSet = gimbalPtr->GetGimbalAngleRangeElementIdSet();
    DgnElementId gimbalAngleRangeId = *gimbalAngleRangeSet.begin();
    GimbalAngleRangePtr gimbalAngleRangePtr = GimbalAngleRange::GetForEdit(*projectPtr, gimbalAngleRangeId);
    ASSERT_TRUE(gimbalAngleRangePtr.IsValid());
    DgnCode gimbalAngleRangeCode = GimbalAngleRange::CreateCode(*projectPtr, Utf8PrintfString("%d", 42));
    gimbalAngleRangePtr->SetCode(gimbalAngleRangeCode);
    Angle minAngle(Angle::FromDegrees(10.0));
    gimbalAngleRangePtr->SetMinimumAngle(minAngle);
    Angle maxAngle(Angle::FromDegrees(20.0));
    gimbalAngleRangePtr->SetMaximumAngle(maxAngle);
    auto gimbalAngleRangeUpdatedPtr = gimbalAngleRangePtr->Update();
    ASSERT_TRUE(gimbalAngleRangeUpdatedPtr.IsValid());
    GimbalAngleRangeElementId gimbalAngleRangeUpdatedId = gimbalAngleRangeUpdatedPtr->GetId();
    ASSERT_TRUE(gimbalAngleRangeUpdatedId.IsValid());
    //Updating don't change id...
    ASSERT_TRUE(gimbalAngleRangeUpdatedId == gimbalAngleRangeId);

    DgnCode gimbalCode = Gimbal::CreateCode(*projectPtr, Utf8PrintfString("%d", 42));
    gimbalPtr->SetCode(gimbalCode);
    auto gimbalUpdatedPtr = gimbalPtr->Update();
    ASSERT_TRUE(gimbalUpdatedPtr.IsValid());
    GimbalElementId gimbalUpdatedId = gimbalUpdatedPtr->GetId();
    ASSERT_TRUE(gimbalUpdatedId.IsValid());
    //Updating don't change id...
    ASSERT_TRUE(gimbalUpdatedId == gimbalId);

    DgnCode droneCode = Drone::CreateCode(*projectPtr, Utf8PrintfString("%d", 42));
    dronePtr->SetCode(droneCode);
    auto droneUpdatedPtr = dronePtr->Update();
    ASSERT_TRUE(droneUpdatedPtr.IsValid());
    DroneElementId droneUpdatedId = droneUpdatedPtr->GetId();
    ASSERT_TRUE(droneUpdatedId.IsValid());
    //Updating don't change id...
    ASSERT_TRUE(droneUpdatedId == droneId);

    //Save changes
    DbResult result = projectPtr->SaveChanges("BasicDrone");
    EXPECT_EQ(BE_SQLITE_OK, result) << "Save Drone failed";

    //Close project to flush memory
    dronePtr = nullptr;//release our element before closing project, otherwise we get an assert in closeDb.
    droneUpdatedPtr = nullptr;
    gimbalPtr = nullptr;
    gimbalUpdatedPtr = nullptr;
    gimbalAngleRangePtr = nullptr;
    gimbalAngleRangeUpdatedPtr = nullptr;
    CloseProject();

    //Reopen project
    DgnDbPtr projectReopenedPtr = OpenProject(L"ModifyDrone.dgndb");
    ASSERT_TRUE(projectReopenedPtr.IsValid());

    ASSERT_TRUE(projectReopenedPtr->Elements().GetElement(droneId).IsValid());
    DroneCPtr myDronePtr = Drone::Get(*projectReopenedPtr, droneId);
    ASSERT_TRUE(myDronePtr.IsValid());
    ASSERT_EQ(droneId, myDronePtr->GetElementId());
    GimbalElementId gimbalIdRead = myDronePtr->GetGimbalElementId();
    ASSERT_TRUE(gimbalIdRead.IsValid());
    ASSERT_EQ(gimbalIdRead, gimbalId);
    GimbalCPtr myGimbalPtr = Gimbal::Get(*projectReopenedPtr, gimbalIdRead);

    DgnElementIdSet newGimbalAngleRangeSet = myGimbalPtr->GetGimbalAngleRangeElementIdSet();
    GimbalAngleRangeCPtr myGimbalAngleRangePtr = GimbalAngleRange::Get(*projectReopenedPtr, *(newGimbalAngleRangeSet.begin()));
    ASSERT_TRUE(myGimbalAngleRangePtr.IsValid());

    //read back Drone properties and check if equal
    DgnCode myDroneCode = myDronePtr->GetCode();
    ASSERT_TRUE(myDroneCode == droneCode);

    DgnCode myGimbalCode = myGimbalPtr->GetCode();
    ASSERT_TRUE(myGimbalCode == gimbalCode);
    DgnCode myGimbalAngleRangeCode = myGimbalAngleRangePtr->GetCode();
    ASSERT_TRUE(myGimbalAngleRangeCode == gimbalAngleRangeCode);
    ASSERT_TRUE(minAngle.Radians() == myGimbalAngleRangePtr->GetMinimumAngle().Radians());
    ASSERT_TRUE(maxAngle.Radians() == myGimbalAngleRangePtr->GetMaximumAngle().Radians());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataCaptureTests, DeleteDrone)
    {
    DgnDbPtr projectPtr = CreateProject(L"DeleteDrone.dgndb");
    ASSERT_TRUE(projectPtr.IsValid());
    Utf8String cameraDeviceLabel = "BasicCameraDevice";
    CreateSampleDroneProjectWithCameraDevice(*projectPtr, cameraDeviceLabel.c_str());

    DgnModelId spatialModelId = QueryFirstSpatialModelId(*projectPtr);
    DgnModelPtr spatialModelPtr = projectPtr->Models().GetModel(spatialModelId);
    ASSERT_TRUE(spatialModelPtr.IsValid());
    ASSERT_TRUE(spatialModelPtr->IsSpatialModel());

    // Query Drone element
    Utf8String droneLabel(Utf8PrintfString("Drone1"));
    DgnElementId droneId = Drone::QueryForIdByLabel(*projectPtr, droneLabel.c_str());
    ASSERT_TRUE(droneId.IsValid());

    //Delete edited Drone element - You CANNOT delete an edited Drone element because you get a copy of the original ...
    //Delete is merely a shortcut for el.GetDgnDb().Elements().Delete(el);
    DroneCPtr droneEditedPtr = Drone::GetForEdit(*projectPtr, droneId);
    ASSERT_TRUE(droneEditedPtr.IsValid());
    DgnDbStatus status = droneEditedPtr->Delete();
    ASSERT_FALSE(status == DgnDbStatus::Success);

    //Delete Drone element - You CAN delete a const Drone element because this is effectively the original element...
    //Delete is merely a shortcut for el.GetDgnDb().Elements().Delete(el);
    DroneCPtr dronePtr = Drone::Get(*projectPtr, droneId);
    ASSERT_TRUE(dronePtr.IsValid());
    status = dronePtr->Delete();
    ASSERT_TRUE(status == DgnDbStatus::Success);

    //Save changes
    DbResult result = projectPtr->SaveChanges("BasicDrone");
    EXPECT_EQ(BE_SQLITE_OK, result) << "Save Drone failed";

    //Close project to flush memory
    dronePtr = nullptr;//release our element before closing project, otherwise we get an assert in closeDb.
    droneEditedPtr = nullptr;
    CloseProject();

    //Reopen project
    DgnDbPtr projectReopenedPtr = OpenProject(L"DeleteDrone.dgndb");
    ASSERT_TRUE(projectReopenedPtr.IsValid());

    //Check that DroneId is not accessible anymore 
    ASSERT_FALSE(projectReopenedPtr->Elements().GetElement(droneId).IsValid());
    DroneCPtr myDronePtr = Drone::Get(*projectReopenedPtr, droneId);
    ASSERT_FALSE(myDronePtr.IsValid());

    // Check that query Drone element returns nothing
    DgnElementId deletedDroneId = Drone::QueryForIdByLabel(*projectReopenedPtr, droneLabel.c_str());
    ASSERT_FALSE(deletedDroneId.IsValid());
    }   