/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ScalableMeshSchema/IMeshSpatialModel.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <ScalableMeshSchema/ScalableMeshSchemaCommon.h>
#include <ScalableMeshSchema/ExportMacros.h>
#include <TerrainModel/TerrainModel.h>
#include <ScalableMesh/IScalableMesh.h>
SCALABLEMESH_SCHEMA_REF_COUNTED_PTR(ITerrainTileIterator)
SCALABLEMESH_SCHEMA_REF_COUNTED_PTR(ITerrainTexture)
SCALABLEMESH_SCHEMA_TYPEDEFS(IMeshSpatialModel)
BEGIN_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE

struct ITerrainTileChangedHandler
    {
    public:
        virtual void TilesChanged(const bvector<bpair<uint32_t, uint32_t>>& modifiedTiles, bool loadTextures) = 0;
    };


struct ITextureTileId
    {

    };
    
struct ITerrainTileIterator : public RefCountedBase
    {
    private : 

        virtual void _GetMeshPart(bvector<DPoint3d>& vertices, bvector<int32_t>& verticeIndexes) = 0;
        virtual bool _NextPart() = 0;

    public : 

        SCALABLEMESH_SCHEMA_EXPORT void GetMeshPart(bvector<DPoint3d>& vertices, bvector<int32_t>& verticeIndexes);
        SCALABLEMESH_SCHEMA_EXPORT bool NextPart();
    };

struct ITerrainTexture : public RefCountedBase
    {
    virtual BentleyStatus _LoadTexture(uint32_t& width, uint32_t& height, bvector<Byte>& dataRGBA) const = 0;
    virtual BentleyStatus _GetDimensions(uint32_t& width, uint32_t& height) const = 0;
    virtual BentleyStatus _GetMeshPartsIterator(ITerrainTileIteratorPtr& iterator) const = 0;
    virtual BentleyStatus _GetRange(DRange3dR range) const = 0;
    virtual ITextureTileId const& _GetId() const = 0;

    public:
        SCALABLEMESHSCHEMA_EXPORT BentleyStatus LoadTexture(uint32_t& width, uint32_t& height, bvector<Byte>& dataRGBA) const;
        SCALABLEMESHSCHEMA_EXPORT BentleyStatus GetDimensions(uint32_t& width, uint32_t& height) const;
        SCALABLEMESHSCHEMA_EXPORT BentleyStatus GetMeshPartsIterator(ITerrainTileIteratorPtr& iterator) const; 
        SCALABLEMESHSCHEMA_EXPORT BentleyStatus GetRange(DRange3dR range) const;
        SCALABLEMESHSCHEMA_EXPORT ITextureTileId const& GetId() const;
    };

struct IMeshSpatialModel : Dgn::SpatialModel
    {
    protected:
        IMeshSpatialModel(CreateParams const& params) : Dgn::SpatialModel(params){};
        virtual bool _IsMultiResolution() const = 0;
        virtual BentleyApi::Dgn::AxisAlignedBox3dCR _GetRange() const = 0;
        virtual BentleyStatus _QueryTexturesLod(bvector<ITerrainTexturePtr>& textures, size_t maxSizeBytes) const = 0;
        virtual BentleyStatus _QueryTexture(ITextureTileId const& tileId, ITerrainTexturePtr& texture) const = 0;
        //virtual BentleyStatus _GetMeshPartUnderClipMask(bvector<DPoint3d>& vertices,
        //                                 bvector<int32_t>&  verticeIndexes,
        //                                 BentleyApi::Dgn::DgnElementId&      clippedConceptualElementId) = 0;
        virtual BentleyStatus _ReloadClipMask(BentleyApi::Dgn::DgnElementId& clipMaskElementId, bool isNew) = 0;
        virtual BentleyStatus _ReloadAllClipMasks() = 0;
        virtual BentleyStatus _StartClipMaskBulkInsert() = 0;
        virtual BentleyStatus _StopClipMaskBulkInsert() = 0;
        virtual BentleyStatus _CreateIterator(ITerrainTileIteratorPtr& iterator) = 0;
        virtual TerrainModel::IDTM* _GetDTM(BentleyApi::ScalableMesh::DTMAnalysisType type) = 0;
        virtual void _RegisterTilesChangedEventListener(ITerrainTileChangedHandler* eventListener) = 0;
        virtual bool _UnregisterTilesChangedEventListener(ITerrainTileChangedHandler* eventListener) = 0;

    public:
        SCALABLEMESH_SCHEMA_EXPORT bool IsMultiResolution() const{ return _IsMultiResolution();}
        SCALABLEMESH_SCHEMA_EXPORT BentleyApi::Dgn::AxisAlignedBox3dCR GetRange() const;
        
        SCALABLEMESH_SCHEMA_EXPORT BentleyStatus QueryTexturesLod(bvector<ITerrainTexturePtr>& textures, size_t maxSizeBytes) const;

        SCALABLEMESH_SCHEMA_EXPORT BentleyStatus QueryTexture(ITextureTileId const& tileId, ITerrainTexturePtr& texture) const;

        //SCALABLEMESH_SCHEMA_EXPORT BentleyStatus GetMeshPartUnderClipMask(bvector<DPoint3d>& vertices,
        //                                                                bvector<int32_t>&  verticeIndexes,
        //                                                                BentleyApi::Dgn::DgnElementId&      clippedConceptualElementId);

        SCALABLEMESH_SCHEMA_EXPORT BentleyStatus ReloadClipMask(BentleyApi::Dgn::DgnElementId& clipMaskElementId, bool isNew);
        SCALABLEMESH_SCHEMA_EXPORT BentleyStatus ReloadAllClipMasks();

        SCALABLEMESH_SCHEMA_EXPORT BentleyStatus StartClipMaskBulkInsert();
        SCALABLEMESH_SCHEMA_EXPORT BentleyStatus StopClipMaskBulkInsert();
         

        SCALABLEMESH_SCHEMA_EXPORT BentleyStatus CreateIterator(ITerrainTileIteratorPtr& iterator);

        SCALABLEMESH_SCHEMA_EXPORT TerrainModel::IDTM* GetDTM(BentleyApi::ScalableMesh::DTMAnalysisType type = BentleyApi::ScalableMesh::DTMAnalysisType::Precise);

        SCALABLEMESH_SCHEMA_EXPORT void RegisterTilesChangedEventListener(ITerrainTileChangedHandler* eventListener);
        SCALABLEMESH_SCHEMA_EXPORT bool UnregisterTilesChangedEventListener(ITerrainTileChangedHandler* eventListener);
    };
    
END_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE