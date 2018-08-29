/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/RealityModelTilesets.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include <RealityPlatformTools/SimpleRDSApi.h>
#include <ScalableMeshSchema/ScalableMeshHandler.h>
#include "DgnPlatform/WebMercator.h"


USING_NAMESPACE_BENTLEY_REALITYPLATFORM
USING_NAMESPACE_BENTLEY_SCALABLEMESH_SCHEMA

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE


BE_JSON_NAME(tilesetUrl)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Converter::GenerateRealityModelTilesets()
    {
    bool doUpload = false, doLocal = false;

    Bentley::WString uploadConfigVar;
    Bentley::WString serverConfigVar;
    if (SUCCESS == DgnV8Api::ConfigurationManager::GetVariable(uploadConfigVar, L"DGNDB_REALITY_MODEL_UPLOAD"))                          
        doUpload = true;
    else
        {
        if (SUCCESS == DgnV8Api::ConfigurationManager::GetVariable(serverConfigVar, L"DGNDB_REALITY_MODEL_TEMPDIR"))
            doLocal = true;
        }

    if (!doUpload && !doLocal)
        return SUCCESS;

    if (m_modelsRequiringRealityTiles.empty())
        return SUCCESS;

    if (doUpload)
        {
        RDSRequestManager::Setup();
        if (!RealityDataService::AreParametersSet())
            {
            ReportIssue(Converter::IssueSeverity::Error, IssueCategory::Unknown(), Issue::RDSUninitialized(), "");
            return ERROR;
            }
        }

    BeFileName              outputDirectory;
    if (doUpload)
        {
        if (BentleyStatus::ERROR == T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory(outputDirectory, L"Bentley\\RealityModelTileSets"))
            {
            ReportIssueV(Converter::IssueSeverity::Error, IssueCategory::DiskIO(), Issue::TemporaryDirectoryNotFound(), Utf8String(outputDirectory.c_str()).c_str());
            return ERROR;
            }
        }
    else
        {
        outputDirectory.SetName(serverConfigVar.c_str());
        if (!outputDirectory.IsDirectory())
            {
            BeAssert(false && "output directory does not exist");
            return ERROR;
            }
        }

    WString      dbFileName = GetDgnDb().GetFileName().GetFileNameWithoutExtension();

    outputDirectory.AppendToPath(dbFileName.c_str());
    if (outputDirectory.IsDirectory())
        {
        BeFileName::EmptyDirectory(outputDirectory);
        }
    else if (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(outputDirectory))
        {
        ReportIssueV(Converter::IssueSeverity::Error, IssueCategory::DiskIO(), Issue::TemporaryDirectoryNotFound(), Utf8String(outputDirectory.c_str()).c_str());
        BeAssert(false && "unable to create output directory");
        return ERROR;
        }
    
    SetStepName(ProgressMessage::STEP_CREATE_REALITY_MODEL_TILES());
    AddTasks((int32_t)m_modelsRequiringRealityTiles.size());

    auto    ecefLocation = m_dgndb->GeoLocation().GetEcefLocation();
    auto    dbToEcefTransform = ecefLocation.m_isValid ? Transform::From(ecefLocation.m_angles.ToRotMatrix(), ecefLocation.m_origin) : Transform::FromIdentity();

    for (auto const& curr : m_modelsRequiringRealityTiles)
        {
        auto model = m_dgndb->Models().GetModel(curr.first);
        bpair<Utf8String, SyncInfo::V8FileSyncInfoId> tpair = curr.second;
        Utf8String fileName = tpair.first;
        SyncInfo::V8FileSyncInfoId fileId = tpair.second;
        auto geometricModel = model->ToGeometricModel();

        if (nullptr == geometricModel)
            {
            BeAssert(false && "Reality model requested for non-geometric model");
            return ERROR;
            }

        BeFileName file(fileName.c_str());
        uint64_t currentLastModifiedTime;
        uint64_t currentFileSize;
        Utf8String currentEtag;
        GetSyncInfo().GetCurrentImageryInfo(fileName, currentLastModifiedTime, currentFileSize, currentEtag);

        uint64_t existingLastModifiedTime;
        uint64_t existingFileSize;
        Utf8String existingEtag;
        Utf8String rdsId;
        bool isUpdate = false;
        if (GetSyncInfo().TryFindImageryFile(model->GetModeledElementId(), fileName, existingLastModifiedTime, existingFileSize, existingEtag, rdsId))
            {
            if (!existingEtag.empty())
                {
                if (existingEtag.Equals(currentEtag))
                    continue;
                }
            else if (currentLastModifiedTime == existingLastModifiedTime && currentFileSize == existingFileSize)
                continue;
            isUpdate = true;
            }

        // Only get to this point if it is a new image or if it is an existing image that has been modified
        BeFileName modelDir = outputDirectory;
        modelDir.AppendToPath(WString(model->GetModelId().ToString().c_str()).c_str());
        if (modelDir.DoesPathExist())
            {
            if (!modelDir.IsDirectory())
                {
                if (BeFileNameStatus::Success != modelDir.BeDeleteFile())
                    {
                    return ERROR;
                    }
                if (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(modelDir))
                    {
                    return ERROR;
                    }
                }
            else if (BeFileNameStatus::Success != BeFileName::EmptyDirectory(modelDir))
                {
                return ERROR;
                }
            }
        BeFileName  rootJsonFile(nullptr, modelDir.c_str(), L"TileRoot", L"json");
        auto smModel = dynamic_cast<ScalableMeshModelCP>(geometricModel);
        if (smModel != nullptr)
            {
            smModel->WriteCesiumTileset(rootJsonFile, modelDir);
            }
        else
            {
            static double   s_leafTolerance = 0.0;      // Use the tolerance of the input tileset.
            TileTree::IO::ICesiumPublisher::WriteCesiumTileset(rootJsonFile, modelDir, *geometricModel, dbToEcefTransform, s_leafTolerance);
            }

        Utf8String url;
        Utf8String identifier = "";
        if (doUpload)
            {
            // if it is an update, then first we need to delete the current data
            if (isUpdate)
                {
                bvector<ConnectedRealityDataRelationshipPtr> relationshipVector;
                ConnectedRealityDataRelationship::RetrieveAllForRDId(relationshipVector, rdsId);
                for (ConnectedRealityDataRelationshipPtr relationship : relationshipVector)
                    relationship->Delete();
                ConnectedRealityData deleter = ConnectedRealityData(rdsId);
                deleter.Delete();
                }
            ConnectedRealityData crd = ConnectedRealityData();
            crd.SetName(Utf8String(dbFileName).c_str());
            Utf8PrintfString description(ConverterDataStrings::RDS_Description(), Utf8String(dbFileName).c_str());
            crd.SetDescription(description.c_str());
            Utf8String rootDoc("TileRoot.json");
            crd.SetRootDocument(rootDoc.c_str());
            crd.SetClassification(RealityDataBase::MODEL);
            crd.SetVisibility(RealityDataBase::Visibility::PERMISSION);
            crd.SetDataset(model->GetName().c_str());
            crd.SetRealityDataType("RealityMesh3DTiles");
            RealityDataService::SetProjectId("fb1696c8-c074-4c76-a539-a5546e048cc6"); // This is the project id  used for testing on qa.
                                              
            Utf8String empty = "";
            ConnectedResponse response = crd.Upload(modelDir, empty);

            if (!response.simpleSuccess)
                {
                ReportIssue(Converter::IssueSeverity::Error, IssueCategory::DiskIO(), Issue::RDSUploadFailed(), "");
                return ERROR;
                }

            identifier = crd.GetIdentifier();
            RealityDataByIdRequest rd = RealityDataByIdRequest(identifier);
            url = BeStringUtilities::UriDecode(rd.GetHttpRequestString().c_str());
            BeFileName::EmptyAndRemoveDirectory(modelDir);
            }
        else
            {
            url = Utf8String("http://localhost:8080/") + Utf8String(dbFileName).c_str() + "/" + model->GetModelId().ToString() + Utf8String("/TileRoot.json");
            }

        model->SetJsonProperties(json_tilesetUrl(), url);
        model->Update();

        if (isUpdate)
            m_syncInfo.UpdateImageryFile(model->GetModeledElementId(), currentLastModifiedTime, currentFileSize, currentEtag.c_str(), identifier.c_str());
        else
            m_syncInfo.InsertImageryFile(model->GetModeledElementId(), fileId, fileName.c_str(), currentLastModifiedTime, currentFileSize, currentEtag.c_str(), identifier.c_str());

        }

    return BSISUCCESS;
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
