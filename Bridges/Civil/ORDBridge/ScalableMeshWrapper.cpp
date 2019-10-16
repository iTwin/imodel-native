/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/ScalableMeshWrapper.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ScalableMeshWrapper.h"
#include <DgnPlatform/DgnPlatformApi.h>
#include <ScalableMeshSchema/ScalableMeshSchemaApi.h>

USING_NAMESPACE_BENTLEY_DGN

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ScalableMeshWrapper::RegisterDomain()
    {
    DgnDomains::RegisterDomain(ScalableMeshSchema::ScalableMeshDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ScalableMeshWrapper::AddTerrainClassifiers(DgnDbR dgnDb, DgnModelId const& clippingsModelId)
    {
    auto linkModelPtr = dgnDb.GetRealityDataSourcesModel();

    BeSQLite::EC::ECSqlStatement stmt;
    stmt.Prepare(dgnDb, "SELECT sm.ECInstanceId FROM ScalableMesh.ScalableMeshModel sm, " BIS_SCHEMA(BIS_CLASS_RepositoryLink) " rl "
                 "WHERE rl.ECInstanceId = sm.ModeledElement.Id AND rl.Model.Id = ?");
    BeAssert(stmt.IsPrepared());

    stmt.BindId(1, linkModelPtr->GetModelId());

    ModelSpatialClassifiers classifiers;
    while (BeSQLite::DbResult::BE_SQLITE_ROW == stmt.Step())
        {
        if (auto spatialModelP = dynamic_cast<ScalableMeshSchema::ScalableMeshModelP>(dgnDb.Models().GetModel(stmt.GetValueId<DgnModelId>(0)).get()))
            {
            if (BentleyStatus::SUCCESS != spatialModelP->GetSpatialClassifiers(classifiers))
                continue;

            Dgn::ModelSpatialClassifier::Flags terrainClassifierFlags;
            terrainClassifierFlags.m_insideDisplay = Dgn::ModelSpatialClassifier::Display::DISPLAY_ElementColor;
            terrainClassifierFlags.m_outsideDisplay = Dgn::ModelSpatialClassifier::Display::DISPLAY_On;
            terrainClassifierFlags.m_type = Dgn::ModelSpatialClassifier::Type::TYPE_Model;

            Dgn::ModelSpatialClassifier terrainClassifier(clippingsModelId, Dgn::DgnCategoryId(), Dgn::DgnElementId(),
                                                          terrainClassifierFlags, "Terrain Clippings", 0.0, true, false);
            classifiers.push_back(terrainClassifier);
            spatialModelP->SetClassifiers(classifiers);
            spatialModelP->Update();
            }
        }
    }