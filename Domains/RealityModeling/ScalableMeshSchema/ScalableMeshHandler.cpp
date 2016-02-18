/*-------------------------------------------------------------------------------------+
|
|     $Source: ScalableMeshSchema/ScalableMeshHandler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ScalableMeshSchemaPCH.h"
USING_NAMESPACE_BENTLEY_DGNPLATFORM
#include <ScalableMeshSchema\ScalableMeshHandler.h>

USING_NAMESPACE_BENTLEY_SCALABLEMESHSCHEMA

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
AxisAlignedBox3dCR ScalableMeshModel::_GetRange() const
    {
    return m_range;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
BentleyStatus ScalableMeshModel::_QueryTexturesLod(bvector<ITerrainTexturePtr>& textures, size_t maxSizeBytes) const
    {
    return ERROR;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
BentleyStatus ScalableMeshModel::_QueryTexture(ITextureTileId const& tileId, ITerrainTexturePtr& texture) const
    {
    return ERROR;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
BentleyStatus ScalableMeshModel::_ReloadClipMask(BentleyApi::Dgn::DgnElementId& clipMaskElementId, bool isNew)
    {
    return ERROR;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
BentleyStatus ScalableMeshModel::_ReloadAllClipMasks()
    {
    return ERROR;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
BentleyStatus ScalableMeshModel::_StartClipMaskBulkInsert()
    {
    return ERROR;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
BentleyStatus ScalableMeshModel::_StopClipMaskBulkInsert()
    {
    return ERROR;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
BentleyStatus ScalableMeshModel::_CreateIterator(ITerrainTileIteratorPtr& iterator)
    {
    return ERROR;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
TerrainModel::IDTM* ScalableMeshModel::_GetDTM()
    {
    if (nullptr == m_smPtr.get()) return nullptr;
    return m_smPtr->GetDTMInterface();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
void ScalableMeshModel::_RegisterTilesChangedEventListener(ITerrainTileChangedHandler* eventListener)
    {

    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
bool ScalableMeshModel::_UnregisterTilesChangedEventListener(ITerrainTileChangedHandler* eventListener)
    {
    return false;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
void ScalableMeshModel::_AddGraphicsToScene(ViewContextR context)
    {

    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
ScalableMeshModel::ScalableMeshModel(BentleyApi::Dgn::DgnModel::CreateParams const& params)
    : T_Super(params)
    {
    BeFileName tmFileName;
    tmFileName = params.m_dgndb.GetFileName().GetDirectoryName();
    tmFileName.AppendToPath(params.m_dgndb.GetFileName().GetFileNameWithoutExtension().c_str());
    tmFileName.AppendString(L"\\terrain.stm");
    m_smPtr = IScalableMesh::GetFor(tmFileName.GetWCharCP(), false, true);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
ScalableMeshModel::~ScalableMeshModel()
    {

    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
IMeshSpatialModelP ScalableMeshModel::GetTerrainModelP(BentleyApi::Dgn::DgnDbCR dgnDb)
    {
    BeSQLite::EC::ECSqlStatement stmt;
    if (BeSQLite::EC::ECSqlStatus::Success != stmt.Prepare(dgnDb, "SELECT ECInstanceId FROM ScalableMeshModel.ScalableMesh;"))
        return nullptr;

    if (BeSQLite::BE_SQLITE_ROW != stmt.Step()) return nullptr;
    DgnModelId smModelID = DgnModelId(stmt.GetValueUInt64(0));
    DgnModelPtr dgnModel = dgnDb.Models().FindModel(smModelID);

    if (dgnModel.get() == 0)
        {
        dgnModel = dgnDb.Models().GetModel(smModelID);
        }

    if (dgnModel.get() != 0)
        {
        assert(dynamic_cast<ScalableMeshModel*>(dgnModel.get()) != 0);

        return static_cast<ScalableMeshModel*>(dgnModel.get());
        }
    return nullptr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
ScalableMeshModelP ScalableMeshModel::CreateModel(BentleyApi::Dgn::DgnDbR dgnDb)
    {
    DgnClassId classId(dgnDb.Schemas().GetECClassId("ScalableMesh","ScalableMeshModel"));
    BeAssert(classId.IsValid());

    ScalableMeshModelP model = new ScalableMeshModel(DgnModel::CreateParams(dgnDb, classId, DgnModel::CreateModelCode("scalableTerrain")));

    model->Insert();
    dgnDb.SaveChanges();
    return model;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
WString ScalableMeshModel::GetTerrainModelPath(BentleyApi::Dgn::DgnDbCR dgnDb)
    {
    BeFileName tmFileName;
    tmFileName = dgnDb.GetFileName().GetDirectoryName();
    tmFileName.AppendToPath(dgnDb.GetFileName().GetFileNameWithoutExtension().c_str());
    if (!tmFileName.DoesPathExist())
        BeFileName::CreateNewDirectory(tmFileName.c_str());
    tmFileName.AppendString(L"\\terrain.stm");
    return tmFileName;
    }

HANDLER_DEFINE_MEMBERS(ScalableMeshModelHandler)