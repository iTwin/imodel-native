/*-------------------------------------------------------------------------------------+
|
|     $Source: ScalableMeshSchema/TerrainHandler.cpp $
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
/*
//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2016
//----------------------------------------------------------------------------------------
BentleyStatus ITerrainTexture::GetMeshPart(DRange3dR range, bvector<DPoint3d>& vertices, bvector<int32_t>& verticeIndexes) const
{
return _GetMeshPart(range, vertices, verticeIndexes);
}
*/

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
AxisAlignedBox3dCR TerrainSpatialModel::GetRange() const
{
return _GetRange();
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2016
//----------------------------------------------------------------------------------------
BentleyStatus TerrainSpatialModel::QueryTexturesLod(bvector<ITerrainTexturePtr>& textures, size_t maxSizeBytes) const
{
return _QueryTexturesLod(textures, maxSizeBytes);
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2016
//----------------------------------------------------------------------------------------
BentleyStatus TerrainSpatialModel::QueryTexture(ITextureTileId const& tileId, ITerrainTexturePtr& texture) const
{
return _QueryTexture(tileId, texture);
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2016
//----------------------------------------------------------------------------------------
BentleyStatus TerrainSpatialModel::GetMeshPartUnderClipMask(bvector<DPoint3d>& vertices,
                                                            bvector<int32_t>&  verticeIndexes,
                                                            BentleyApi::Dgn::DgnElementId&      clippedConceptualElementId)
{
return _GetMeshPartUnderClipMask(vertices,verticeIndexes,clippedConceptualElementId);
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2016
//----------------------------------------------------------------------------------------
BentleyStatus TerrainSpatialModel::ReloadClipMask(BentleyApi::Dgn::DgnElementId& clipMaskElementId, bool isNew)
{
return _ReloadClipMask(clipMaskElementId, isNew);
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2016
//----------------------------------------------------------------------------------------
BentleyStatus TerrainSpatialModel::ReloadAllClipMasks()
{
return _ReloadAllClipMasks();
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2016
//----------------------------------------------------------------------------------------
BentleyStatus TerrainSpatialModel::StartClipMaskBulkInsert()
{
return _StartClipMaskBulkInsert();
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2016
//----------------------------------------------------------------------------------------
BentleyStatus TerrainSpatialModel::StopClipMaskBulkInsert()
{
return _StopClipMaskBulkInsert();
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2016
//----------------------------------------------------------------------------------------
BentleyStatus TerrainSpatialModel::CreateIterator(ITerrainTileIteratorPtr& iterator)
{
return _CreateIterator(iterator);
}