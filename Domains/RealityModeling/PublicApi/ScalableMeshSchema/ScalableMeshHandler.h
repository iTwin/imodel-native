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

SCALABLEMESH_SCHEMA_TYPEDEFS(ScalableMeshModel)

USING_NAMESPACE_BENTLEY_SCALABLEMESH

BEGIN_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE

//=======================================================================================
// @bsiclass                                                  
//=======================================================================================
struct ScalableMeshModel : IMeshSpatialModel
    {

    DGNMODEL_DECLARE_MEMBERS("ScalableMeshModel", IMeshSpatialModel)
        private:

        IScalableMeshPtr m_smPtr;
        BentleyApi::Dgn::AxisAlignedBox3d m_range;
    protected:
 

        virtual bool _IsMultiResolution() const { return true; };
        virtual BentleyApi::Dgn::AxisAlignedBox3dCR _GetRange() const override;
        virtual BentleyStatus _QueryTexturesLod(bvector<ITerrainTexturePtr>& textures, size_t maxSizeBytes) const override;
        virtual BentleyStatus _QueryTexture(ITextureTileId const& tileId, ITerrainTexturePtr& texture) const override;

        virtual BentleyStatus _ReloadClipMask(BentleyApi::Dgn::DgnElementId& clipMaskElementId, bool isNew) override;
        virtual BentleyStatus _ReloadAllClipMasks() override;
        virtual BentleyStatus _StartClipMaskBulkInsert() override;
        virtual BentleyStatus _StopClipMaskBulkInsert() override;
        virtual BentleyStatus _CreateIterator(ITerrainTileIteratorPtr& iterator) override;
        virtual TerrainModel::IDTM* _GetDTM() override;
        virtual void _RegisterTilesChangedEventListener(ITerrainTileChangedHandler* eventListener) override;
        virtual bool _UnregisterTilesChangedEventListener(ITerrainTileChangedHandler* eventListener) override;

        SCALABLEMESH_SCHEMA_EXPORT virtual void _AddGraphicsToScene(BentleyApi::Dgn::ViewContextR context) override;
    public:

        //! Create a new TerrainPhysicalModel object, in preparation for loading it from the DgnDb.
        ScalableMeshModel(BentleyApi::Dgn::DgnModel::CreateParams const& params);

        virtual ~ScalableMeshModel();

        void OpenFile(BeFileNameCR smFilename);

        //! A DgnDb can have only one terrain. 
        SCALABLEMESH_SCHEMA_EXPORT static IMeshSpatialModelP GetTerrainModelP(BentleyApi::Dgn::DgnDbCR dgnDb);


    };

struct EXPORT_VTABLE_ATTRIBUTE ScalableMeshModelHandler : Dgn::dgn_ModelHandler::Spatial
    {
    MODELHANDLER_DECLARE_MEMBERS("ScalableMeshModel", ScalableMeshModel, ScalableMeshModelHandler, Dgn::dgn_ModelHandler::Spatial, SCALABLEMESH_SCHEMA_EXPORT)

    public : 
                     
        //NEEDS_WORK_SM : Currently for testing only
        SCALABLEMESH_SCHEMA_EXPORT static IMeshSpatialModelP AttachTerrainModel(DgnDb& db, Utf8StringCR modelName, BeFileNameCR smFilename);
    };
END_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE