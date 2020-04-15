/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../BackDoor/PublicApi/BackDoor/DataCapture/BackDoor.h"


DataCaptureProjectHost* DataCaptureTestsFixture::m_host = nullptr;
DgnDbPtr DataCaptureTestsFixture::s_currentProject = DgnDbPtr(nullptr);
//---------------------------------------------------------------------------------------
// Automatically called by gTest framework before running every test
//---------------------------------------------------------------------------------------
void DataCaptureTestsFixture::SetUpTestCase()
    {
    m_host = new DataCaptureProjectHost();
    }
//---------------------------------------------------------------------------------------
// Automatically called by gTest framework after running every test
//---------------------------------------------------------------------------------------
void DataCaptureTestsFixture::TearDownTestCase()
    {
    if (s_currentProject.IsValid())
        s_currentProject->SaveChanges();

    s_currentProject = nullptr;

    delete m_host;
    m_host = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
DgnModelId DataCaptureTestsFixture::QueryFirstSpatialModelId(DgnDbR db)
    {
    for (auto const& modelEntry : db.Models().MakeIterator())
        {
        if ((DgnModel::DictionaryId() == modelEntry.GetModelId()))
            continue;

        DgnModelPtr model = db.Models().GetModel(modelEntry.GetModelId());
        if (model->IsSpatialModel() && dynamic_cast<SpatialModelP>(model.get()))
            return modelEntry.GetModelId();
        }

    BeAssert(false && "No SpatialModel found");
    return DgnModelId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId DataCaptureTestsFixture::QueryFirstDefinitionModelId(DgnDbR db)
    {
    for (auto const& modelEntry : db.Models().MakeIterator())
        {
        if ((DgnModel::DictionaryId() == modelEntry.GetModelId()))
            continue;

        DgnModelPtr model = db.Models().GetModel(modelEntry.GetModelId());
        if (model->IsDefinitionModel() && dynamic_cast<DefinitionModelP>(model.get()))
            return modelEntry.GetModelId();
        }

    //Create a definition model
    auto& handler = dgn_ModelHandler::Definition::GetHandler();
    DgnClassId classId = db.Domains().GetClassId(handler);
    DgnCode modelCode = DgnModel::CreateModelCode("DataCaptureDefinition");
    DefinitionModelPtr model = DefinitionModel::Create(DefinitionModel::CreateParams(db, classId, modelCode));
    if (DgnDbStatus::Success == model->Insert())
        {
        return model->GetModelId();
        }

    BeAssert(false && "No DefinitionModelP found");
    return DgnModelId();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr DataCaptureTestsFixture::CreateProject(WCharCP baseName, bool needsSetBriefcase)
    {
    BeAssert(nullptr != m_host);

    const WCharCP testSeedName = L"TestSeed.dgndb";
    const BeFileName testSeedPath = m_host->BuildProjectFileName(testSeedName);
    const BeFileName projectName = m_host->BuildProjectFileName(baseName);

    //! Create seed
    if (!testSeedPath.DoesPathExist())
        {
        DgnDbPtr seedProject = m_host->CreateProject(testSeedName);

        //! Error
        if (seedProject.IsNull())
            return nullptr;

        seedProject->CloseDb();
        }

    if (s_currentProject.IsValid())
        {
        s_currentProject->SaveChanges();
        s_currentProject->CloseDb();
        s_currentProject = nullptr;
        }

    if (BeFileNameStatus::Success != BeFileName::BeCopyFile(testSeedPath.c_str(), projectName.c_str(), false))
        return nullptr;

    s_currentProject = m_host->OpenProject(baseName);
    if (s_currentProject.IsNull())
        return nullptr;

    if (needsSetBriefcase)
        {
        s_currentProject->ResetBriefcaseId(BeBriefcaseId(1));
        s_currentProject->CloseDb();
        s_currentProject = m_host->OpenProject(baseName);
        }

    return s_currentProject;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbPtr DataCaptureTestsFixture::OpenProject(WCharCP baseName, bool needsSetBriefcase)
    {
    BeAssert(nullptr != m_host);

    const BeFileName projectName = m_host->BuildProjectFileName(baseName);

    bool wantsCurrentProject = false;

    if (s_currentProject.IsValid())
        {
        Utf8String projectNameUtf(projectName.c_str());
        if (0 == projectNameUtf.CompareTo(s_currentProject->GetDbFileName()))
            wantsCurrentProject = true;
        }

    if (!wantsCurrentProject)
        s_currentProject = m_host->OpenProject(baseName);

    if (needsSetBriefcase && s_currentProject.IsValid())
        {
        s_currentProject->ResetBriefcaseId(BeBriefcaseId(1));
        s_currentProject->SaveChanges();
        s_currentProject->CloseDb();
        s_currentProject = m_host->OpenProject(baseName);
        }

    return s_currentProject;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void DataCaptureTestsFixture::CloseProject()
    {
    BeAssert(nullptr != m_host);

    if (s_currentProject.IsValid())
        {
        s_currentProject->SaveChanges();
        s_currentProject->CloseDb();
        }

    s_currentProject = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void DataCaptureTestsFixture::CreateSampleShotProjectWithCameraDevice(Dgn::DgnDbR dgndb, Utf8CP cameraDeviceLable)
    {
    DgnModelId spatialModelId = QueryFirstSpatialModelId(dgndb);
    DgnModelId definitionModelId = QueryFirstDefinitionModelId(dgndb);
    DgnModelPtr spatialModelPtr = dgndb.Models().GetModel(spatialModelId);
    SpatialModelP spatialModelP = spatialModelPtr->ToSpatialModelP();

    DgnModelPtr definitonModelPtr = dgndb.Models().GetModel(definitionModelId);
    DefinitionModelP definitonModelP = definitonModelPtr->ToDefinitionModelP();
    ASSERT_TRUE(definitonModelPtr.IsValid());
    ASSERT_TRUE(definitonModelPtr->IsDefinitionModel());

    // Create CameraDevice Model
    auto cameraDeviceModelPtr = CameraDeviceModel::Create(*definitonModelP);
    cameraDeviceModelPtr->Insert();
    ASSERT_TRUE(cameraDeviceModelPtr.IsValid());

    // Create CameraDevice
    auto cameraDevicePtr = CameraDevice::Create(*spatialModelP, cameraDeviceModelPtr->GetId());
    cameraDevicePtr->SetLabel(cameraDeviceLable);
    cameraDevicePtr->SetFocalLength(0.00479835);
    DPoint2d principalPoint = { 2677.8,1772 };
    cameraDevicePtr->SetPrincipalPoint(principalPoint);
    RadialDistortionPtr  pRadialDistortion = RadialDistortion::Create(1, 2, 3);
    TangentialDistortionPtr  pTangentialDistortion = TangentialDistortion::Create(4, 5);
    cameraDevicePtr->SetRadialDistortion(pRadialDistortion.get());
    cameraDevicePtr->SetTangentialDistortion(pTangentialDistortion.get());
    cameraDevicePtr->SetAspectRatio(1.0);
    cameraDevicePtr->SetSkew(1.0);
    cameraDevicePtr->SetSensorSize(1.0);
    cameraDevicePtr->Insert();
    CameraDeviceElementId cameraDeviceId = cameraDevicePtr->GetId();


    //Insert ten photos for this cameraDevice
    for (int photoNumber = 0; photoNumber < 10; photoNumber++)
        {
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
        auto ShotPtr = Shot::Create(*spatialModelP, cameraDeviceId, poseId);

        //Change Photo properties
        Utf8String photoLabel(Utf8PrintfString("BasicPhoto%d",photoNumber));
        ShotPtr->SetLabel(photoLabel.c_str());

        //Insert Photo element
        ShotPtr->Insert();
        }

    //Save changes
    dgndb.SaveChanges("SamplePhotos");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                    04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void DataCaptureTestsFixture::CreateSampleDroneProjectWithCameraDevice(Dgn::DgnDbR dgndb, Utf8CP cameraDeviceLable)
    {
    DgnModelId spatialModelId = QueryFirstSpatialModelId(dgndb);
    DgnModelPtr spatialModelPtr = dgndb.Models().GetModel(spatialModelId);
    SpatialModelP spatialModelP = spatialModelPtr->ToSpatialModelP();

    DgnModelId definitionModelId = QueryFirstDefinitionModelId(dgndb);
    DgnModelPtr definitonModelPtr = dgndb.Models().GetModel(definitionModelId);
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

    cameraDevicePtr->SetLabel(cameraDeviceLable);
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
    dronePtr->SetLabel("Drone1");

    ASSERT_TRUE(dronePtr.IsValid());

    auto droneInsertedPtr = dronePtr->Insert();
    DroneElementId droneElmId = droneInsertedPtr->GetId();

    //Save changes
    dgndb.SaveChanges("SampleDrone");
	}