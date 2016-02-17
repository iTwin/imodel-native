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

SCALABLEMESHSCHEMA_TYPEDEFS(ScalableMeshModel)
BEGIN_BENTLEY_SCALABLEMESHSCHEMA_NAMESPACE
USING_NAMESPACE_BENTLEY_SCALABLEMESH
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

        SCALABLEMESHSCHEMA_EXPORT virtual void _AddGraphicsToScene(BentleyApi::Dgn::ViewContextR context) override;
    public:

        //! Create a new TerrainPhysicalModel object, in preparation for loading it from the DgnDb.
        ScalableMeshModel(BentleyApi::Dgn::DgnModel::CreateParams const& params);

        virtual ~ScalableMeshModel();


        //! A DgnDb can have only one terrain. 
        SCALABLEMESHSCHEMA_EXPORT static IMeshSpatialModelP GetTerrainModelP(BentleyApi::Dgn::DgnDbCR dgnDb);


    };

struct EXPORT_VTABLE_ATTRIBUTE ScalableMeshModelHandler : Dgn::dgn_ModelHandler::Spatial
    {
    MODELHANDLER_DECLARE_MEMBERS("ScalableMeshModel", ScalableMeshModel, ScalableMeshModelHandler, Dgn::dgn_ModelHandler::Spatial, SCALABLEMESHSCHEMA_EXPORT)

    };
END_BENTLEY_SCALABLEMESHSCHEMA_NAMESPACE