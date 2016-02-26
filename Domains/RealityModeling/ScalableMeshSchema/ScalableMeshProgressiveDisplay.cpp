/*-------------------------------------------------------------------------------------+
|
|     $Source: ScalableMeshSchema/ScalableMeshProgressiveDisplay.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ScalableMeshSchemaPCH.h"

BEGIN_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
ScalableMeshProgressiveDisplay::ScalableMeshProgressiveDisplay(TexturedMeshR mesh, ViewContextR context)
:m_mesh(mesh),
 m_wasUpToDate(m_mesh.GetTerrain().IsUpToDate())
    {
    //TODO validate that we actually need texture in current purpose. for some purpose a fill color is fine.
    m_wantTexture = !m_mesh.GetTextureDirectory().empty();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
ScalableMeshProgressiveDisplay::~ScalableMeshProgressiveDisplay()
    {
    for (auto& pTile : m_pendingTiles)
        {
        pTile->CancelRequestTexture((uintptr_t)this);        
        }

    PRINT_MSG_IF(!m_pendingTiles.empty() || !m_missingTiles.empty(), 
                 "[%d]> Draw Aborted....Missing tiles %I64u...Pending tiles = %I64u\n", m_startTime, m_missingTiles.size(), m_pendingTiles.size())
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
RefCountedPtr<ScalableMeshProgressiveDisplay> ScalableMeshProgressiveDisplay::Create(TexturedMeshR mesh, ViewContextR context)
    {
    return new ScalableMeshProgressiveDisplay(mesh, context);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
bool ScalableMeshProgressiveDisplay::ShouldDrawInConvext (ViewContextR context) const
    {
    switch (context.GetDrawPurpose())
        {
        case DrawPurpose::Hilite:
        case DrawPurpose::Unhilite:
        case DrawPurpose::ChangedPre:       // Erase, rely on Healing.
        case DrawPurpose::RestoredPre:      // Erase, rely on Healing.
        case DrawPurpose::Pick:
        case DrawPurpose::Flash:
        case DrawPurpose::CaptureGeometry:
        case DrawPurpose::FenceAccept:
        case DrawPurpose::RegionFlood:
        case DrawPurpose::FitView:
        case DrawPurpose::ExportVisibleEdges:
        case DrawPurpose::ModelFacet:
            return false;
        }

    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2015
//----------------------------------------------------------------------------------------
void ScalableMeshProgressiveDisplay::DrawLoadedChildren(ViewContextR context, TextureTileR tile, uint32_t resolutionDelta)
    {
    if(0 == resolutionDelta)
        return;

    for(uint32_t i=0; i < 4; ++i)
        {
        TextureTileP pChild = tile.GetChildP(i);
        if(NULL == pChild)
            continue;

        TileRenderMaterialP materialP = pChild->GetMaterialP();
        if(nullptr != materialP && materialP->IsQvCached())
            {
            pChild->Draw(context, materialP, true/*cachedOnly*/);
            }
        else
            {
            DrawLoadedChildren(context, *pChild, resolutionDelta-1);
            }
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
void ScalableMeshProgressiveDisplay::Draw(ViewContextR context)
    {
    BeAssert(!m_mesh.GetTerrain().IsEmpty());

    // **********************************
    // *** NB: This method must be fast. 
    // **********************************
    // Do not try to read from SQLite or allocate huge amounts of memory in here. 
    // Defer time-consuming tasks to progressive display

    if (DrawPurpose::FitView == context.GetDrawPurpose())
        {
        DPoint3d rangeBox[8];
        m_mesh.GetRange().Get8Corners(rangeBox);
        context.GetIDrawGeom().DrawPointString3d(8, rangeBox, NULL);
        return;
        }

    if (!ScalableMeshProgressiveDisplay::ShouldDrawInConvext(context) || NULL == context.GetViewport())
        return;

    m_startTime = BeTimeUtilities::QueryMillisecondsCounterUInt32();

    m_mesh.QueryVisible(m_visiblesTiles, context);
    if(m_visiblesTiles.empty())
        {
        PRINT_MSG("[%d]<Terrain nothing visible>\n", m_startTime);
        return;
        }

    // Needed because we somehow inherit from the last draw element(ex: hilite)
    //context.ResetContextOverrides();
    //Suggested by Brien, same as ResetContextOverrides() but without a call to _AddContextOverrides(), we don't want context overrides...I think.
    context.OnPreDrawTransient();   // Reinitialize stuff that normally happens when visiting an element (ex. GeomDetail, OverrideMatSymb)

#ifdef TERRAIN_IGNORE_LIGHT
    std::unique_ptr<AutoDisableLighting> __disableLighting(WantTexture() ? new AutoDisableLighting(context) : nullptr);
#endif

    // Our Draw logic, the most important aspect is to always have something on the screen and avoid blank/flickering tiles.
    // (1) We draw only what we have in cache. 
    //      - If a mesh is not in cache, we draw nothing here and defer to _process callback.
    //      - If a texture is not in cache, we look for a coarser and finer resolution texture and display that. Defer texture loading in _process callback.
    //      - we are not doing any CheckStop here, we assumed drawing cached stuff is fast enough. If we do, we will have missing tiles and flickering.
    //
    // (2) _process callback.  
    //      - We request any missing textures and we redraw them on top of the substitutes if any. That should have lead to z-fighting issues but I
    //        found some information suggesting that drawing the exact same geometries will give the exact same math and thus the same depth value. 
    //        Apparently this is how multi-pass forward lighting works. Anyways, if z-fighting happens we will have to review the technique 
    //        and probably end up with a healed required that will unfortunately slow down the display.
    //      - We also look for terrain upToDate.  That occurs during road clipping.  When that happen, we have new geometries so we have no choice but
    //        to do a full heal.
    //      
    // Future:
    //      (1)Coarser vs finer.  We have a bias on coarser since a parent always cover its children area. But we should attempt children first 
    //      and if all the children are loaded draw them, if not fall back to coarser and then if we did not find one fall back to whatever children we have.
    //      (2) Load textures that are closer to the view center first.
    for (auto& visiblePair : m_visiblesTiles)
        {
        TextureTilePtr& pTile = visiblePair.second;

        TileRenderMaterialP pMaterial = WantTexture() ? pTile->GetMaterialP() : nullptr;

        if (WantTexture() && (nullptr == pMaterial || !pMaterial->IsQvCached()))
            {
            // Draw at a coarser resolution
            TileRenderMaterialP coarserMaterialP = pTile->FindCachedMaterialP(DRAW_COARSER_DELTA);

            if (nullptr == coarserMaterialP)
                {
                DrawLoadedChildren(context, *pTile, DRAW_CHILDREN_DELTA);
                }
            else
                {
                // Draw with a coarser resolution
                pTile->Draw(context, coarserMaterialP, true/*cachedOnly*/);
                }

            m_missingTiles.push_back(pTile.get());
            }
        else
            {
            if (!pTile->Draw(context, pMaterial, true/*cachedOnly*/))
                m_missingTiles.push_back(pTile.get());
            }
        }

    if (!m_missingTiles.empty() || !m_wasUpToDate)
        {
        context.GetViewport()->ScheduleProgressiveDisplay(*this);
        m_waitTime = 0;
        m_nextRetryTime = m_startTime + m_waitTime;
        PRINT_MSG("[%d]< visible tiles = %I64u...Missing tiles %I64u\n", m_startTime, m_visiblesTiles.size(), m_missingTiles.size());
        }
    else
        {
        PRINT_MSG("[%d]< visible tiles = %I64u all in cache\n", m_startTime, m_visiblesTiles.size());
        PRINT_MSG("[%d]> Completed in %d ms\n", m_startTime, BeTimeUtilities::QueryMillisecondsCounterUInt32() - m_startTime);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
IProgressiveDisplay::Completion ScalableMeshProgressiveDisplay::_Process(ViewContextR context)
    {
    //&&MM make sure we actually need this.  calling QueryMillisecondsCounterUInt32 as a cost.
    // and everything comes really fast.  Maybe not that fast when we have not uptodate tiles.
//     if (BeTimeUtilities::QueryMillisecondsCounterUInt32() < m_nextRetryTime)
//         {
//         PRINT_MSG("Terrain: Wait %d ms until next retry\n", m_nextRetryTime - BeTimeUtilities::QueryMillisecondsCounterUInt32());
//         return Completion::Aborted;
//         }

    if(!m_wasUpToDate && m_mesh.GetTerrain().IsUpToDate())
        {
        PRINT_MSG("[%d] Healed Required\n", m_startTime);
        return Completion::HealRequired;
        }

    while(!m_missingTiles.empty())
        {
        auto pTile = m_missingTiles.front();    // Front to request them in the order they were added to the list.
        m_missingTiles.pop_front();

        if(WantTexture())
            {
            pTile->RequestTexture((uintptr_t)this/*requestorId*/);
            m_pendingTiles.push_back(pTile);    // was requested check back later.
            //PRINT_MSG("[%d]> Requested Tile res=%d (%d, %d) \n", m_startTime, pTile->GetId().resolution, pTile->GetId().x, pTile->GetId().y);

            // RequestTexture is super fast so avoid calling CheckStop in this case, it is surprisingly slow and come up during profiling.
            }
        else
            {
            // Only do checkStop when we draw something, RequestTexture above is fast enough and doesn't require it.
            if (!pTile->Draw(context, nullptr, false/*cachedOnly*/) || context.CheckStop())   
                {
                PRINT_MSG("[%d] Aborting... pending(%I64u) next retry in %d ms\n", m_startTime, m_pendingTiles.size(), m_waitTime)
                return Completion::Aborted;
                }
            }
        }

    // Can only finish when terrain is upToDate.
    IProgressiveDisplay::Completion completion = m_wasUpToDate ? Completion::Finished : Completion::Aborted;

    if(!m_pendingTiles.empty())
        {
        BeAssert(WantTexture());

#ifdef TERRAIN_IGNORE_LIGHT
        AutoDisableLighting __disableLighting(context);
#endif
 
        for (auto pTile = m_pendingTiles.begin(); pTile != m_pendingTiles.end();  /* incremented or erased in loop*/ )
            {
            if (context.CheckStop())
                {
                PRINT_MSG("[%d] CheckStop reached\n", m_startTime);
                completion = Completion::Aborted;
                break;
                }

            TextureTile::MaterialStatus status;
            TileRenderMaterialP materialP = (*pTile)->LoadMaterial(status);

            if(TextureTile::MaterialStatus::Pending == status)
                {
                ++pTile;    // Will retry later
                completion = Completion::Aborted;   // at least one tile did not finish
                }
            else
                {
                if((*pTile)->Draw(context, materialP, false/*cachedOnly*/))
                    {
                    //PRINT_MSG("[%d]> Draw Tile res=%d (%d, %d) \n", m_startTime, (*pTile)->GetId().resolution, (*pTile)->GetId().x, (*pTile)->GetId().y);
                    pTile = m_pendingTiles.erase(pTile);
                    }
                else
                    { 
                    // We may failed because we reach checkStop. Will catch it and react in the next iteration.
                    // This usually happen only the first time when we create the qvcache.
                    ++pTile;   // Draw did not complete because we reached CheckStop. will retry later.
                    completion = Completion::Aborted;   // at least one tile did not finish
                    }
                }
            }
        }

    if(completion == Completion::Finished)
        {
        BeAssert(m_wasUpToDate);

        PRINT_MSG("[%d]> Completed in %d ms\n", m_startTime, BeTimeUtilities::QueryMillisecondsCounterUInt32() - m_startTime);
        return Completion::Finished;
        }

#ifdef PRINT_TERRAIN_MSG
    if(m_wasUpToDate)
        PRINT_MSG("[%d] progressing... pending(%I64u) next retry in %d ms\n", m_startTime, m_pendingTiles.size(), m_waitTime)
    else
        PRINT_MSG("[%d] waiting ... meshes not up to date retry in %d ms\n", m_startTime, m_waitTime)
#endif

   // m_nextRetryTime = BeTimeUtilities::QueryMillisecondsCounterUInt32() + m_waitTime;

    return completion;    
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2015
//----------------------------------------------------------------------------------------
    BentleyStatus TerrainPhysicalModel::_QueryTexturesLod(bvector<ITerrainTexturePtr>& textures, size_t maxSizeBytes) const
    {
    uint32_t selectedResolution;
    if(SUCCESS != CalculateTextureResolution(selectedResolution, maxSizeBytes))
        return ERROR;

    for(uint32_t tileY=0; tileY < m_pTexturedMesh->GetResolution(selectedResolution).GetTileCountY(); ++tileY)
        {
        for(uint32_t tileX=0; tileX < m_pTexturedMesh->GetResolution(selectedResolution).GetTileCountX(); ++tileX)
            {
            TextureTileId tileId(selectedResolution, tileX, tileY);
            BeFileName    filename = TextureTileGenerator::BuildTextureFilename(m_pTexturedMesh->GetTextureDirectory(), tileId, false/*createDir*/);

            textures.push_back(new TerrainTexture(*this, filename, tileId));
            }        
        }

    return BSISUCCESS;
    }
    

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Nicholas.Woodfield  10/2015
//----------------------------------------------------------------------------------------
BentleyStatus TerrainPhysicalModel::_QueryTexture(ITextureTileId const& tileId, ITerrainTexturePtr& texture) const
	{
    TextureTileId const& texTileId = static_cast<TextureTileId const&>(tileId);
    if (!m_pTexturedMesh.IsValid() || texTileId.resolution >= m_pTexturedMesh->GetResolutionCount())
		return BSIERROR;
    	
    BeFileName filename = TextureTileGenerator::BuildTextureFilename(m_pTexturedMesh->GetTextureDirectory(), texTileId, false/*createDir*/);
    texture = new TerrainTexture(*this, filename, texTileId);

	return BSISUCCESS;
	}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Nicholas.Woodfield  10/2015
//----------------------------------------------------------------------------------------
BentleyStatus TerrainPhysicalModel::CalculateTextureResolution(uint32_t& resolution, size_t maxSizeBytes) const
    {
    if (!m_pTexturedMesh.IsValid())
        return BSIERROR;

    resolution = m_pTexturedMesh->GetResolutionCount() - 1;
    for (uint32_t res = 0; res < m_pTexturedMesh->GetResolutionCount(); ++res)
        {
        TextureResolution const& resInfo = m_pTexturedMesh->GetResolution(res);

        // Do not care about actual size of border tiles. They will get aligned to a pow of 2 anyways.
        size_t resSize = TILE_TEXTURE_SIZE * TILE_TEXTURE_SIZE * 4/*RGBA*/ * resInfo.GetTileCountX() * resInfo.GetTileCountY();

        // allow a 5% difference.
        if (resSize < (maxSizeBytes*1.05))
            {
			resolution = res;
            break;
            }
        }

    return BSISUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2015
//----------------------------------------------------------------------------------------
TerrainTexture::TerrainTexture(TerrainPhysicalModel const& terrain, BeFileNameCR filename, TextureTileId const& id)
:m_terrainP(&terrain),
 m_filename(filename),
 m_id(id)
    {    
    uint32_t xMin = m_id.x << m_id.resolution;
    uint32_t yMin = m_id.y << m_id.resolution;
    uint32_t xMax = (m_id.x+1) << m_id.resolution;
    uint32_t yMax = (m_id.y+1) << m_id.resolution;

    xMax = MIN(xMax, m_terrainP->GetTerrainCache().GetTileCountX());
    yMax = MIN(yMax, m_terrainP->GetTerrainCache().GetTileCountY());

    m_meshCountX = xMax - xMin;
    m_meshCountY = yMax - yMin;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2015
//----------------------------------------------------------------------------------------
BentleyStatus TerrainTexture::_LoadTexture(uint32_t& width, uint32_t& height, bvector<Byte>& dataRGBA) const
    {
    BeFile textureFile;
    if (BeFileStatus::Success != textureFile.Open(m_filename, BeFileAccess::Read))
        return BSIERROR;

    bvector<Byte> jpegData;
    if (BeFileStatus::Success != textureFile.ReadEntireFile(jpegData))
        return BSIERROR;

    BeJpegDecompressor decomp;                             
    if(SUCCESS != decomp.ReadHeader(width, height, jpegData.data(), jpegData.size()))
        return BSIERROR;

    dataRGBA.resize(width*height*4);

    if(SUCCESS != decomp.Decompress(dataRGBA.data(), dataRGBA.size(), jpegData.data(), jpegData.size(), BeJpegPixelType::BE_JPEG_PIXELTYPE_RgbA))
        return BSIERROR;

    return BSISUCCESS;
    }

BentleyStatus TerrainTexture::_GetMeshPartsIterator(ITerrainTileIteratorPtr& iterator) const
    {
    uint32_t xMin = m_id.x << m_id.resolution;
    uint32_t yMin = m_id.y << m_id.resolution;
    iterator = new TextureTileIterator(xMin, yMin, GetMeshCountX(), GetMeshCountY(), m_terrainP->GetTerrainCache().GetTileCountX(), m_terrainP->GetTerrainCache().GetMeshes());
    return BSISUCCESS;
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Nicholas.Woodfield  10/2015
//----------------------------------------------------------------------------------------
BentleyStatus TerrainTexture::_GetRange(DRange3dR range) const
    {
    range = DRange3d::NullRange();
    bvector<CachedMeshPtr> const& meshes = m_terrainP->GetTerrainCache().GetMeshes();

    uint32_t xMin = m_id.x << m_id.resolution;
    uint32_t yMin = m_id.y << m_id.resolution;

    for (uint32_t tileY = 0; tileY < m_meshCountY; tileY++)
        {
        for (uint32_t tileX = 0; tileX < m_meshCountX; tileX++)
            {
            CachedMesh& cachedMesh = *meshes[(yMin + tileY)*m_terrainP->GetTerrainCache().GetTileCountX() + xMin + tileX];

            range.Extend(cachedMesh.GetRange());
            }
        }

    return range.IsNull() ? BSIERROR : BSISUCCESS;
    }


END_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE