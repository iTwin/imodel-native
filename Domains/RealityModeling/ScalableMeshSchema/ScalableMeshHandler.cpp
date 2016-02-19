/*-------------------------------------------------------------------------------------+
|
|     $Source: ScalableMeshSchema/ScalableMeshHandler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ScalableMeshSchemaPCH.h"

#include <BeSQLite\BeSQLite.h>
#include <ScalableMeshSchema\ScalableMeshHandler.h>
#include "ScalableMeshDisplayCacheManager.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SCALABLEMESH_SCHEMA

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
    if (m_progressiveQueryEngine == nullptr)
        {
        m_displayNodesCache = new ScalableMeshDisplayCacheManager(context);
        m_progressiveQueryEngine = IScalableMeshProgressiveQueryEngine::Create(m_smPtr, m_displayNodesCache);
        }
    Transform curViewTransform;
    context.GetCurrLocalToWorldTrans(curViewTransform);
    DMatrix4d curViewTransformMat = DMatrix4d::From(curViewTransform);
   /* bvector<bool> clips;

    BentleyStatus status = m_progressiveQueryEngine->StartQuery(0,
                                                                viewDependentQueryParams,
                                                                m_meshNodes,
                                                                true,
                                                                clips,
                                                                &m_lastViewTransform,
                                                                &curViewTransformMat);

    assert(status == SUCCESS);

    if (m_progressiveQueryEngine->IsQueryComplete(0))
        {
        m_meshNodes.clear();
        status = m_progressiveQueryEngine->GetQueriedNodes(m_meshNodes, 0);
        m_overviewNodes.clear();

        assert(status == SUCCESS);
        }
    else
        {
        status = m_progressiveQueryEngine->GetOverviewNodes(m_overviewNodes, 0);
        m_overviewNodes.insert(m_overviewNodes.end(), m_meshNodes.begin(), m_meshNodes.end());
        m_meshNodes.clear();
        assert(m_overviewNodes.size() > 0);
        assert(status == SUCCESS);
        }
    m_lastViewTransform = curViewTransformMat;
    DrawCurrentNodeList(m_meshNodes, m_overviewNodes, context);*/
    }

//NEEDS_WORK_SM : Should be at application level
void GetScalableMeshTerrainFileName(BeFileName& smtFileName, const BeFileName& dgnDbFileName)
    {    
    //smtFileName = params.m_dgndb.GetFileName().GetDirectoryName();

    smtFileName = dgnDbFileName.GetDirectoryName();
    smtFileName.AppendToPath(dgnDbFileName.GetFileNameWithoutExtension().c_str());
    smtFileName.AppendString(L"\\terrain.smt");
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
void ScalableMeshModel::OpenFile(BeFileNameCR smFilename)
    {
    m_smPtr = IScalableMesh::GetFor(smFilename.GetWCharCP(), false, true);
    }

//=======================================================================================
//! Helper class used to kept pointers with a DgnDb in-memory
//=======================================================================================
struct ScalableMeshTerrainModelAppData : Db::AppData
    {
    static Key DataKey;

    ScalableMeshModel*  m_smTerrainPhysicalModelP;
    bool                m_modelSearched;

    ScalableMeshTerrainModelAppData () 
        {
        m_smTerrainPhysicalModelP = 0;
        m_modelSearched = false;
        }
    virtual ~ScalableMeshTerrainModelAppData () {}

     ScalableMeshModel* GetModel(DgnDbCR db)
        {
        if (m_modelSearched == false)
            {
            //NEEDS_WORK_SM : Not yet done. 
            /*
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
                */        

            m_modelSearched = true;
            }

        return m_smTerrainPhysicalModelP;
        }
    
    static ScalableMeshTerrainModelAppData* Get (DgnDbCR dgnDb)
        {
        ScalableMeshTerrainModelAppData* appData = dynamic_cast<ScalableMeshTerrainModelAppData*>(dgnDb.FindAppData (ScalableMeshTerrainModelAppData::DataKey));
        if (!appData)
            {
            appData = new ScalableMeshTerrainModelAppData ();
            dgnDb.AddAppData (ScalableMeshTerrainModelAppData::DataKey, appData);
            }

        return appData;
        }

    static void Delete (DgnDbCR dgnDb)
        {        
        dgnDb.DropAppData (ScalableMeshTerrainModelAppData::DataKey);        
        }    
    };

Db::AppData::Key ScalableMeshTerrainModelAppData::DataKey;


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
IMeshSpatialModelP ScalableMeshModel::GetTerrainModelP(BentleyApi::Dgn::DgnDbCR dgnDb)
    {   
    return ScalableMeshTerrainModelAppData::Get(dgnDb)->GetModel(dgnDb);
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
IScalableMesh* ScalableMeshModel::GetScalableMesh()
    {
    return m_smPtr.get();
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


IMeshSpatialModelP ScalableMeshModelHandler::AttachTerrainModel(DgnDbR db, Utf8StringCR modelName, BeFileNameCR smFilename)
    {    
    /*    
    BeFileName smtFileName;
    GetScalableMeshTerrainFileName(smtFileName, db.GetFileName());
    
    if (!smtFileName.GetDirectoryName().DoesPathExist())
        BeFileName::CreateNewDirectory(smtFileName.GetDirectoryName().c_str());
        */

    
    DgnClassId classId(db.Schemas().GetECClassId("ScalableMesh", "ScalableMeshModel"));
    BeAssert(classId.IsValid());        
         
    RefCountedPtr<ScalableMeshModel> model(new ScalableMeshModel(DgnModel::CreateParams(db, classId, DgnModel::CreateModelCode(modelName))));

    model->OpenFile(smFilename);

    //After Insert model pointer is handled by DgnModels.
    model->Insert();
                    
    ScalableMeshTerrainModelAppData* appData(ScalableMeshTerrainModelAppData::Get(db));        

    appData->m_smTerrainPhysicalModelP = model.get();
    appData->m_modelSearched = true;

    db.SaveChanges();
        
    return model.get();    
    }



HANDLER_DEFINE_MEMBERS(ScalableMeshModelHandler)