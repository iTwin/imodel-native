/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/RealityModelTilesets.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include <RealityPlatformTools/SimpleRDSApi.h>
#include "DgnPlatform/WebMercator.h"


USING_NAMESPACE_BENTLEY_REALITYPLATFORM

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE


BE_JSON_NAME(tilesetUrl)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Converter::GenerateWebMercatorModel()
    {
    LinkModelPtr rdsModel = GetDgnDb().GetRealityDataSourcesModel();
    Utf8String name("Bing Aerial");
    DgnCode code = CodeSpec::CreateCode(BIS_CODESPEC_LinkElement, *rdsModel->GetModeledElement(), name);
    DgnElementId existing = GetDgnDb().Elements().QueryElementIdByCode(code);
    if (existing.IsValid())
        return SUCCESS;

    RepositoryLinkPtr aerialLink = RepositoryLink::Create(*rdsModel, nullptr, "Bing Aerial");
    if (!aerialLink.IsValid() ||  !aerialLink->Insert().IsValid())
        return ERROR;

    // set up the Bing Aerial map properties Json.
    BentleyApi::Json::Value jsonParameters;
    jsonParameters[WebMercator::WebMercatorModel::json_providerName()] = WebMercator::BingImageryProvider::prop_BingProvider();
    jsonParameters[WebMercator::WebMercatorModel::json_groundBias()] = -1.0;
    jsonParameters[WebMercator::WebMercatorModel::json_transparency()] = 0.0;
    BentleyApi::Json::Value& bingAerialJson = jsonParameters[WebMercator::WebMercatorModel::json_providerData()];

    bingAerialJson[WebMercator::WebMercatorModel::json_mapType()] = (int) WebMercator::MapType::Aerial;
    WebMercator::WebMercatorModel::CreateParams createParams (GetDgnDb(), aerialLink->GetElementId(), jsonParameters);

    WebMercator::WebMercatorModelPtr model = new WebMercator::WebMercatorModel (createParams);
    DgnDbStatus insertStatus = model->Insert();
    BeAssert (DgnDbStatus::Success == insertStatus);
    return DgnDbStatus::Success == insertStatus ? SUCCESS : ERROR;
    }

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
        static double   s_leafTolerance = .01;      // TBD. make this a setting.
        TileTree::IO::ICesiumPublisher::WriteCesiumTileset(rootJsonFile, modelDir, *geometricModel, dbToEcefTransform, s_leafTolerance);
        
        Utf8String url;
        if (doUpload)
            {
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

            Utf8String identifier = crd.GetIdentifier();
            RealityDataByIdRequest rd = RealityDataByIdRequest(identifier);
            url = rd.GetHttpRequestString();
            BeFileName::EmptyAndRemoveDirectory(modelDir);
            }
        else
            {
            url = Utf8String("http://localhost:8080/") + Utf8String(dbFileName).c_str() + "/" + model->GetModelId().ToString() + Utf8String("/TileRoot.json");
            }

        model->SetJsonProperties(json_tilesetUrl(), url);
        model->Update();
        }

    return BSISUCCESS;
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
