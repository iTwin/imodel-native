/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/RealityModelTilesets.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE


BE_JSON_NAME(tilesetUrl)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Converter::GenerateRealityModelTilesets()
    {
    Bentley::WString        serverConfigVar;

    if (SUCCESS != DgnV8Api::ConfigurationManager::GetVariable(serverConfigVar, L"DGNDB_REALITY_MODEL_TEMPDIR"))
        return SUCCESS;

    if (m_modelsRequiringRealityTiles.empty())
        return SUCCESS;

    BeFileName              outputDirectory(serverConfigVar.c_str());
    if (!outputDirectory.IsDirectory())
        {
        BeAssert(false && "output directory does not exist");
        return ERROR;
        }

    WString      dbFileName = GetDgnDb().GetFileName().GetFileNameWithoutExtension();

    outputDirectory.AppendToPath(dbFileName.c_str());
    if (!outputDirectory.IsDirectory() && BeFileNameStatus::Success != BeFileName::CreateNewDirectory(outputDirectory))
        {
        BeAssert(false && "unable to create output directory");
        return ERROR;
        }
    
    BeFileName  rootJsonFile(nullptr, outputDirectory.c_str(), L"TileRoot", L"json");

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
        static double   s_leafTolerance = .01;      // TBD. make this a setting.

        // TBD... Carole M.   Provide an (empty) scratch directory and a filename (in that directory) for the root/JSON file.
         TileTree::IO::ICesiumPublisher::WriteCesiumTileset(rootJsonFile, outputDirectory, *geometricModel, dbToEcefTransform, s_leafTolerance);

        // TBD... Carole M.   Upload the contents of the output directory to Reality Data Server and return URL for the root/JSON file.
        // Add the URL to the reality model (in the modeled element or a JSON property???) - delete the temporary output directory and its contents.
        Utf8String      urlValue = Utf8String("http://localhost:8080/") + Utf8String(dbFileName).c_str() + Utf8String("/TileRoot.json");

        model->SetJsonProperties(json_tilesetUrl(), urlValue);
        model->Update();
        }

    return BSISUCCESS;
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
