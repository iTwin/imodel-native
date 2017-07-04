/*--------------------------------------------------------------------------------------+
|
|     $Source: DataCaptureSchema/Tests/BackDoor/DataCaptureTestFixture.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
        s_currentProject->SetAsBriefcase(BeBriefcaseId(1));
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
        s_currentProject->SetAsBriefcase(BeBriefcaseId(1));
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

    // Create CameraDevice
    auto cameraDevicePtr = CameraDevice::Create(*spatialModelP);
    cameraDevicePtr->SetLabel(cameraDeviceLable);
    cameraDevicePtr->SetFocalLength(4798.35);
    ImageDimensionType imgDimension(5456, 3632);
    cameraDevicePtr->SetImageDimension(imgDimension);
    DPoint2d principalPoint = { 2677.8,1772 };
    cameraDevicePtr->SetPrincipalPoint(principalPoint);
    CameraDeviceDistortionType distortion(1, 2, 3, 4, 5);
    cameraDevicePtr->SetDistortion(distortion);
    cameraDevicePtr->SetAspectRatio(1.0);
    cameraDevicePtr->SetSkew(1.0);
    cameraDevicePtr->Insert();
    CameraDeviceElementId cameraDeviceId = cameraDevicePtr->GetId();


    //Insert ten photos for this cameraDevice
    for (int photoNumber = 0; photoNumber < 10; photoNumber++)
        {
        // Create Photo for the cameraDevice
        auto ShotPtr = Shot::Create(*spatialModelP, cameraDeviceId);

        //Change Photo properties
        Utf8String photoLabel(Utf8PrintfString("BasicPhoto%d",photoNumber));
        ShotPtr->SetLabel(photoLabel.c_str());
        RotMatrix rotation(RotMatrix::FromIdentity());
        DPoint3d center = { 1.0,2.0,3.0 };
        Pose pose(center, rotation);
        ShotPtr->SetPose(pose);
        ShotPtr->SetPhotoId(photoNumber);

        //Insert Photo element
        ShotPtr->Insert();
        }

    //Save changes
    dgndb.SaveChanges("SamplePhotos");
    }

