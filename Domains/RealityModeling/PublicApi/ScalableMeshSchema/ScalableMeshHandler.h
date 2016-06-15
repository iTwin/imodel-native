/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ScalableMeshSchema/ScalableMeshHandler.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <ScalableMeshSchema/ScalableMeshSchemaCommon.h>
#include <ScalableMeshSchema/ExportMacros.h>
#include <ScalableMesh/IScalableMesh.h>
#include <ScalableMeshSchema/IMeshSpatialModel.h>
#include <TerrainModel/TerrainModel.h>
#include <ScalableMesh\IScalableMeshProgressiveQuery.h>
#include <ScalableMesh\IScalableMeshQuery.h>


SCALABLEMESH_SCHEMA_TYPEDEFS(ScalableMeshModel)

USING_NAMESPACE_BENTLEY_SCALABLEMESH

BEGIN_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE

class ScalableMeshDrawingInfo;

typedef RefCountedPtr<ScalableMeshDrawingInfo> ScalableMeshDrawingInfoPtr;     

class ScalableMeshDrawingInfo : public RefCountedBase
    {
private : 

    DrawPurpose m_drawPurpose;            
    DMatrix4d   m_localToViewTransformation;    
    DPoint3d    m_low;
    DPoint3d    m_high;
public : 

    int m_currentQuery;
    bvector<BentleyG0601::ScalableMesh::IScalableMeshCachedDisplayNodePtr> m_meshNodes;
    bvector<BentleyG0601::ScalableMesh::IScalableMeshCachedDisplayNodePtr> m_overviewNodes;

public : 
        
    ScalableMeshDrawingInfo(ViewContextP viewContext)        
        {
        m_drawPurpose = viewContext->GetDrawPurpose();   
        const DMatrix4d localToView(viewContext->GetLocalToView());     
        memcpy(&m_localToViewTransformation, &localToView, sizeof(DMatrix4d));                
        viewContext->GetViewport()->GetViewCorners(m_low, m_high);
        }

    ~ScalableMeshDrawingInfo()
        {
        }    

    DrawPurpose GetDrawPurpose()
        {
        return m_drawPurpose;
        }
        
    int GetViewNumber()
        {
        //NEEDS_WORK_SM : Default to 0
        return 0;
        }    
    
    bool HasAppearanceChanged(const ScalableMeshDrawingInfoPtr& smDrawingInfoPtr)
        {                                                                  
        return (0 != memcmp(&smDrawingInfoPtr->m_localToViewTransformation, &m_localToViewTransformation, sizeof(DMatrix4d))) ||
               (0 != memcmp(&smDrawingInfoPtr->m_low, &m_low, sizeof(DPoint3d))) ||
               (0 != memcmp(&smDrawingInfoPtr->m_high, &m_high, sizeof(DPoint3d)));        
        }

    const DMatrix4d& GetLocalToViewTransform() {return m_localToViewTransformation;}   
    };


//=======================================================================================
// @bsiclass                                                  
//=======================================================================================
struct ScalableMeshModel : IMeshSpatialModel
    {
        DGNMODEL_DECLARE_MEMBERS("ScalableMeshModel", IMeshSpatialModel)

        private:

        IScalableMeshPtr                        m_smPtr;
        bool                                    m_tryOpen; 
        BentleyApi::Dgn::AxisAlignedBox3d       m_range;

        IScalableMeshDisplayCacheManagerPtr     m_displayNodesCache;
        IScalableMeshProgressiveQueryEnginePtr  m_progressiveQueryEngine;        
        ScalableMeshDrawingInfoPtr              m_currentDrawingInfoPtr;
        DMatrix4d                               m_storageToUorsTransfo; 
        bool m_forceRedraw;
        bset<uint64_t>                          m_activeClips;
                       
    protected:

        struct Properties
            {
            Utf8String          m_fileId;    

            void ToJson(Json::Value&) const;
            void FromJson(Json::Value const&);
            };

        Properties      m_properties;

        virtual void _WriteJsonProperties(Json::Value&) const override;
        virtual void _ReadJsonProperties(Json::Value const&) override;
     
        virtual bool _IsMultiResolution() const { return true; };
        virtual BentleyApi::Dgn::AxisAlignedBox3dCR _GetRange() const override;
        virtual BentleyStatus _QueryTexturesLod(bvector<ITerrainTexturePtr>& textures, size_t maxSizeBytes) const override;
        virtual BentleyStatus _QueryTexture(ITextureTileId const& tileId, ITerrainTexturePtr& texture) const override;

        virtual BentleyStatus _ReloadClipMask(BentleyApi::Dgn::DgnElementId& clipMaskElementId, bool isNew) override;
        virtual BentleyStatus _ReloadAllClipMasks() override;
        virtual BentleyStatus _StartClipMaskBulkInsert() override;
        virtual BentleyStatus _StopClipMaskBulkInsert() override;
        virtual BentleyStatus _CreateIterator(ITerrainTileIteratorPtr& iterator) override;
        virtual TerrainModel::IDTM* _GetDTM(ScalableMesh::DTMAnalysisType type) override;
        virtual void _RegisterTilesChangedEventListener(ITerrainTileChangedHandler* eventListener) override;
        virtual bool _UnregisterTilesChangedEventListener(ITerrainTileChangedHandler* eventListener) override;

        SCALABLEMESH_SCHEMA_EXPORT virtual void _AddGraphicsToScene(BentleyApi::Dgn::ViewContextR context) override;
    public:

        //! Create a new TerrainPhysicalModel object, in preparation for loading it from the DgnDb.
        ScalableMeshModel(BentleyApi::Dgn::DgnModel::CreateParams const& params);

        virtual ~ScalableMeshModel();

        SCALABLEMESH_SCHEMA_EXPORT static ScalableMeshModelP CreateModel(BentleyApi::Dgn::DgnDbR dgnDb);
                
        void OpenFile(BeFileNameCR smFilename, DgnDbR dgnProject);

        //! A DgnDb can have only one terrain. 
        SCALABLEMESH_SCHEMA_EXPORT static IMeshSpatialModelP GetTerrainModelP(BentleyApi::Dgn::DgnDbCR dgnDb);

        SCALABLEMESH_SCHEMA_EXPORT static void GetAllScalableMeshes(BentleyApi::Dgn::DgnDbCR dgnDb, bvector<IMeshSpatialModelP>& models);

        SCALABLEMESH_SCHEMA_EXPORT static WString GetTerrainModelPath(BentleyApi::Dgn::DgnDbCR dgnDb);

        SCALABLEMESH_SCHEMA_EXPORT IScalableMesh* GetScalableMesh();

        SCALABLEMESH_SCHEMA_EXPORT Transform GetUorsToStorage();

        SCALABLEMESH_SCHEMA_EXPORT void SetActiveClipSets(bset<uint64_t>& activeClips, bset<uint64_t>& previouslyActiveClips);

        SCALABLEMESH_SCHEMA_EXPORT void GetClipSetIds(bvector<uint64_t>& allShownIds);

        SCALABLEMESH_SCHEMA_EXPORT bool IsTerrain();


    };

struct EXPORT_VTABLE_ATTRIBUTE ScalableMeshModelHandler : Dgn::dgn_ModelHandler::Spatial
    {
    MODELHANDLER_DECLARE_MEMBERS("ScalableMeshModel", ScalableMeshModel, ScalableMeshModelHandler, Dgn::dgn_ModelHandler::Spatial, SCALABLEMESH_SCHEMA_EXPORT)

    public : 
                     
        //NEEDS_WORK_SM : Currently for testing only
        SCALABLEMESH_SCHEMA_EXPORT static IMeshSpatialModelP AttachTerrainModel(DgnDb& db, Utf8StringCR modelName, BeFileNameCR smFilename);
    };
END_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE