/*-------------------------------------------------------------------------------------+
|
|     $Source: ScalableMeshSchema/IMeshSpatialModel.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ScalableMeshSchemaPCH.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SCALABLEMESHSCHEMA

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2016
//----------------------------------------------------------------------------------------
BentleyStatus ITerrainTexture::LoadTexture(uint32_t& width, uint32_t& height, bvector<Byte>& dataRGBA) const
    {
    return _LoadTexture(width, height, dataRGBA);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Nicholas.Woodfield     1/2016
//----------------------------------------------------------------------------------------
BentleyStatus ITerrainTexture::GetDimensions(uint32_t& width, uint32_t& height) const
    {
    return _GetDimensions(width, height);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2016
//----------------------------------------------------------------------------------------
ITextureTileId const& ITerrainTexture::GetId() const
    {
    return _GetId();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2016
//----------------------------------------------------------------------------------------
BentleyStatus ITerrainTexture::GetMeshPartsIterator(ITerrainTileIteratorPtr& iterator) const
    {
    return _GetMeshPartsIterator(iterator);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2016
//----------------------------------------------------------------------------------------
BentleyStatus ITerrainTexture::GetRange(DRange3dR range) const
    {
    return _GetRange(range);
    }

void ITerrainTileIterator::GetMeshPart(bvector<DPoint3d>& vertices, bvector<int32_t>& verticeIndexes)
    {
    _GetMeshPart(vertices, verticeIndexes);
    }

bool ITerrainTileIterator::NextPart()
    {
    return _NextPart();
    }   


//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2016
//----------------------------------------------------------------------------------------
void IMeshSpatialModel::RegisterTilesChangedEventListener(ITerrainTileChangedHandler* eventListener) 
    {
    return _RegisterTilesChangedEventListener(eventListener); 
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2016
//----------------------------------------------------------------------------------------
bool IMeshSpatialModel::UnregisterTilesChangedEventListener(ITerrainTileChangedHandler* eventListener) 
    {
    return _UnregisterTilesChangedEventListener(eventListener);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2016
//----------------------------------------------------------------------------------------
AxisAlignedBox3dCR IMeshSpatialModel::GetRange() const
{
return _GetRange();
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2016
//----------------------------------------------------------------------------------------
TerrainModel::IDTM* IMeshSpatialModel::GetDTM() 
    {
    return _GetDTM();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2016
//----------------------------------------------------------------------------------------
BentleyStatus IMeshSpatialModel::QueryTexturesLod(bvector<ITerrainTexturePtr>& textures, size_t maxSizeBytes) const
{
return _QueryTexturesLod(textures, maxSizeBytes);
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2016
//----------------------------------------------------------------------------------------
BentleyStatus IMeshSpatialModel::QueryTexture(ITextureTileId const& tileId, ITerrainTexturePtr& texture) const
{
return _QueryTexture(tileId, texture);
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2016
//----------------------------------------------------------------------------------------
/*BentleyStatus IMeshSpatialModel::GetMeshPartUnderClipMask(bvector<DPoint3d>& vertices,
                                                            bvector<int32_t>&  verticeIndexes,
                                                            BentleyApi::Dgn::DgnElementId&      clippedConceptualElementId)
{
return _GetMeshPartUnderClipMask(vertices,verticeIndexes,clippedConceptualElementId);
}*/

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2016
//----------------------------------------------------------------------------------------
BentleyStatus IMeshSpatialModel::ReloadClipMask(BentleyApi::Dgn::DgnElementId& clipMaskElementId, bool isNew)
{
return _ReloadClipMask(clipMaskElementId, isNew);
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2016
//----------------------------------------------------------------------------------------
BentleyStatus IMeshSpatialModel::ReloadAllClipMasks()
{
return _ReloadAllClipMasks();
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2016
//----------------------------------------------------------------------------------------
BentleyStatus IMeshSpatialModel::StartClipMaskBulkInsert()
{
return _StartClipMaskBulkInsert();
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2016
//----------------------------------------------------------------------------------------
BentleyStatus IMeshSpatialModel::StopClipMaskBulkInsert()
{
return _StopClipMaskBulkInsert();
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2016
//----------------------------------------------------------------------------------------
BentleyStatus IMeshSpatialModel::CreateIterator(ITerrainTileIteratorPtr& iterator)
{
return _CreateIterator(iterator);
}