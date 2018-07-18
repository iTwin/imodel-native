/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/RealityModelTilesets.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include <RealityPlatformTools/SimpleRDSApi.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE


BE_JSON_NAME(tilesetUrl)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Converter::GenerateRealityModelTilesets()
    {
    Bentley::WString uploadConfigVar;
    if (SUCCESS != DgnV8Api::ConfigurationManager::GetVariable(uploadConfigVar, L"DGNDB_REALITY_MODEL_UPLOAD"))
        return SUCCESS;

    if (m_modelsRequiringRealityTiles.empty())
        return SUCCESS;

    RDSRequestManager::Setup();
    if (!RealityDataService::AreParametersSet())
        {
        ReportIssue(Converter::IssueSeverity::Error, IssueCategory::Unknown(), Issue::RDSUninitialized(), "");
        return ERROR;
        }

    BeFileName              outputDirectory;
    if (BentleyStatus::ERROR == T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory(outputDirectory, L"Bentley\\RealityModelTileSets"))
        {
        ReportIssueV(Converter::IssueSeverity::Error, IssueCategory::DiskIO(), Issue::TemporaryDirectoryNotFound(), Utf8String(outputDirectory.c_str()).c_str());
        return ERROR;
        }

    WString      dbFileName = GetDgnDb().GetFileName().GetFileNameWithoutExtension();

    outputDirectory.AppendToPath(dbFileName.c_str());
    if (!outputDirectory.IsDirectory() && BeFileNameStatus::Success != BeFileName::CreateNewDirectory(outputDirectory))
        {
        ReportIssueV(Converter::IssueSeverity::Error, IssueCategory::DiskIO(), Issue::TemporaryDirectoryNotFound(), Utf8String(outputDirectory.c_str()).c_str());
        BeAssert(false && "unable to create output directory");
        return ERROR;
        }
    
    SetStepName(ProgressMessage::STEP_CREATE_REALITY_MODEL_TILES());
    AddTasks((int32_t)m_modelsRequiringRealityTiles.size());

    auto    ecefLocation = m_dgndb->GeoLocation().GetEcefLocation();
    auto    dbToEcefTransform = ecefLocation.m_isValid ? Transform::From(ecefLocation.m_angles.ToRotMatrix(), ecefLocation.m_origin) : Transform::FromIdentity();

    for (auto const& modelId : m_modelsRequiringRealityTiles)
        {
        auto model = m_dgndb->Models().GetModel(modelId);
        auto geometricModel = model->ToGeometricModel();

        if (nullptr == geometricModel)
            {
            BeAssert(false && "Reality model requested for non-geometric model");
            return ERROR;
            }

        BeFileName tempDir = outputDirectory;
        tempDir.AppendToPath(WString(model->GetModelId().ToString().c_str()).c_str());
        if (tempDir.DoesPathExist())
            {
            if (!tempDir.IsDirectory())
                {
                if (BeFileNameStatus::Success != tempDir.BeDeleteFile())
                    {
                    return ERROR;
                    }
                if (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(tempDir))
                    {
                    return ERROR;
                    }
                }
            else if (BeFileNameStatus::Success != BeFileName::EmptyDirectory(tempDir))
                {
                return ERROR;
                }
            }

        BeFileName  rootJsonFile(nullptr, tempDir.c_str(), L"TileRoot", L"json");

        static double   s_leafTolerance = .01;      // TBD. make this a setting.

        TileTree::IO::ICesiumPublisher::WriteCesiumTileset(rootJsonFile, tempDir, *geometricModel, dbToEcefTransform, s_leafTolerance);

        ConnectedRealityData crd = ConnectedRealityData();
        crd.SetName(Utf8String(dbFileName).c_str());
        Utf8PrintfString description(ConverterDataStrings::RDS_Description(), Utf8String(dbFileName).c_str());
        crd.SetDescription(description.c_str());
        Utf8String rootDoc = Utf8String(dbFileName).c_str() + Utf8String("/TileRoot.json");
        crd.SetRootDocument(rootDoc.c_str());
        crd.SetClassification(RealityDataBase::MODEL);
        crd.SetVisibility(RealityDataBase::Visibility::PERMISSION);
        crd.SetDataset(model->GetName().c_str());
        crd.SetRealityDataType("RealityMesh3DTiles");
        RealityDataService::SetProjectId("fb1696c8-c074-4c76-a539-a5546e048cc6"); // This is the project id  used for testing on qa.

        Utf8String empty = "";
        ConnectedResponse response = crd.Upload(tempDir, empty);

        if (!response.simpleSuccess)
            {
            ReportIssue(Converter::IssueSeverity::Error, IssueCategory::DiskIO(), Issue::RDSUploadFailed(), "");
            return ERROR;
            }

        Utf8String identifier = crd.GetIdentifier();
        RealityDataByIdRequest rd = RealityDataByIdRequest(identifier);
        Utf8String url = rd.GetHttpRequestString();
        model->SetJsonProperties(json_tilesetUrl(), url);
        model->Update();
        BeFileName::EmptyAndRemoveDirectory(tempDir);
        }

    return BSISUCCESS;
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
