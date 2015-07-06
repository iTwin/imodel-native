/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterQuadTree.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RasterSchemaInternal.h"
#include <DgnPlatform/DgnCore/ImageUtilities.h>
#include "RasterQuadTree.h"

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  5/2015
//----------------------------------------------------------------------------------------
static ReprojectStatus s_FilterGeocoordWarning(ReprojectStatus status)
    {
    if(REPROJECT_CSMAPERR_OutOfUsefulRange == status)   // This a warning
        return REPROJECT_Success;

    return status;   
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
struct RasterProgressiveDisplay : IProgressiveDisplay, NonCopyableClass
{
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS

protected:
    RasterQuadTreeR m_raster;

    bvector<RasterTilePtr>      m_visiblesTiles;    // all the tiles to display. 
    std::list<RasterTilePtr>    m_missingTiles;     // Tiles that need to be requested.
    std::list<RasterTilePtr>    m_pendingTiles;     // Tiles that were requested and we are waiting for them to arrive.     

    uint64_t m_nextRetryTime;                             //!< When to re-try m_pendingTiles. unix millis UTC
    uint64_t m_waitTime;                                  //!< How long to wait before re-trying m_pendingTiles. millis 

    struct SortByResolution
        {
        bool operator ()(const RasterTilePtr& lhs, const RasterTilePtr& rhs) const
            {
            if(lhs->GetId().resolution != rhs->GetId().resolution)
                return lhs->GetId().resolution < rhs->GetId().resolution;

            if(lhs->GetId().x != rhs->GetId().x)
                return lhs->GetId().x < rhs->GetId().x;

            return lhs->GetId().y < rhs->GetId().y;
            }
        };
    typedef bset<RasterTilePtr, SortByResolution> SortedTiles;

    RasterProgressiveDisplay (RasterQuadTreeR raster, ViewContextR context);
    virtual ~RasterProgressiveDisplay();

    //! Displays tiled rasters and schedules downloads. 
    virtual Completion _Process(ViewContextR) override;

    // set limit and returns true to cause caller to call EnableStopAfterTimout
    virtual bool _WantTimeoutSet(uint32_t& limit) override {return false;}

    bool ShouldDrawInConvext (ViewContextR context) const;

    void FindBackgroudTiles(SortedTiles& backgroundTiles, bvector<RasterTilePtr> const& visibleTiles, uint32_t resolutionDelta);
    
    void DrawLoadedChildren(RasterTileR tile, ViewContextR context, uint32_t resolutionDelta);

public:
    void Draw (ViewContextR context);

    static RefCountedPtr<RasterProgressiveDisplay> Create(RasterQuadTreeR raster, ViewContextR context);
};

//----------------------------------------------------------------------------------------
//-------------------------------  RasterTile --------------------------------------------
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
RasterTilePtr RasterTile::CreateRoot(RasterQuadTreeR tree)
    {
    if(tree.GetSource().GetResolutionCount() == 0)
        return NULL;

    TileId id(tree.GetSource().GetResolutionCount()-1,0,0);

    //&&MM TODO clip to gcs domain. For now, make sure we can compute the corners in UOR.
    DPoint3d srcCorners[4];
    DPoint3d uorCorners[4];
    tree.GetSource().ComputeTileCorners(srcCorners, id); 
    if(BSISUCCESS != ReprojectCorners(uorCorners, srcCorners, tree))
        return NULL;

    return new RasterTile(id, NULL, tree);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
RasterTile::RasterTile(TileId const& id, RasterTileP parent, RasterQuadTreeR tree)
    :m_tileId(id),
    m_tree(tree),
    m_pParent(parent),
    m_status(TileStatus::Unloaded)
    {
    BeAssert(m_tree.GetSource().IsValidTileId(id));

    RasterSourceR source = m_tree.GetSource();

    DPoint3d srcCorners[4];
    source.ComputeTileCorners(srcCorners, id); 

    ReprojectCorners(m_corners, srcCorners, tree);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  5/2015
//----------------------------------------------------------------------------------------
ReprojectStatus RasterTile::ReprojectCorners(DPoint3dP outUors, DPoint3dCP srcCartesian, RasterQuadTreeR tree)
    {
    GeoCoordinates::BaseGCSP pSourceGcs = tree.GetSource().GetGcsP();

    DgnGCSP pDgnGcs = tree.GetDgnDb().Units().GetDgnGCS();

    if(NULL == pSourceGcs || NULL == pDgnGcs)
        {
        // Assume raster to be coincident.
        memcpy(outUors, srcCartesian, sizeof(DPoint3d)*4);
        return REPROJECT_Success;
        }

    ReprojectStatus status = REPROJECT_Success;

    GeoPoint srcGeoCorners[4];
    for(size_t i=0; i < 4; ++i)
        {
        if(REPROJECT_Success != (status = s_FilterGeocoordWarning(pSourceGcs->LatLongFromCartesian(srcGeoCorners[i], srcCartesian[i]))))
            {
            BeAssert(!"A source should always be able to represent itself in its GCS."); // That operation cannot fail or can it?
            return status;
            }
        }
    
    // Source latlong to DgnDb latlong.
    GeoPoint dgnGeoCorners[4];
    for(size_t i=0; i < 4; ++i)
        {
        if(REPROJECT_Success != (status = s_FilterGeocoordWarning(pSourceGcs->LatLongFromLatLong(dgnGeoCorners[i], srcGeoCorners[i], *pDgnGcs))))
            return status;
        }

    //Finally to UOR
    for(uint32_t i=0; i < 4; ++i)
        {
        if(REPROJECT_Success != (status = s_FilterGeocoordWarning(pDgnGcs->UorsFromLatLong(outUors[i], dgnGeoCorners[i]))))
            return status;
        }

    return status;    
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
bool RasterTile::Draw(ViewContextR context)
    {
    // Corners are in this order:
    //  [0]  [1]
    //  [2]  [3]
    DPoint3d uvPts[4];
    memcpy(uvPts, m_corners, sizeof(uvPts));
    
    // Make sure the map displays beneath element graphics. Note that this policy is appropriate for the background map, which is always
    // "on the ground". It is not appropriate for other kinds of reality data, even some images display. It is up to the individual reality
    // data handler to use the "surface" that is appprpriate to the reality data.
    if (!context.GetViewport()->GetViewController().GetTargetModel()->Is3d())
        {
        for (auto& pt : uvPts)
            pt.z = -DgnViewport::GetDisplayPriorityFrontPlane();  // lowest possibly priority
        }
    else
        {
        BeSQLite::HighPriorityOperationBlock highPriority; //&&MM review.
        auto extents = context.GetViewport()->GetViewController().GetProjectExtents();
        for (auto& pt : uvPts)
            pt.z = extents.low.z - 1;
        }

    static bool s_DrawTileShape = true;    //&&MM for testing.
    if(m_pDisplayTile.IsNull() && s_DrawTileShape)
        {
        ElemMatSymb elemMatSymb;
        auto colorIdx = ColorDef(222, 0, 0, 128);
        elemMatSymb.SetLineColor (colorIdx);
        elemMatSymb.SetWidth (2);
        elemMatSymb.SetIsFilled(false);
        context.GetIDrawGeom().ActivateMatSymb (&elemMatSymb);

        DPoint3d box[5];
        box[0] = uvPts[0];
        box[1] = uvPts[1];
        box[2] = uvPts[3];
        box[3] = uvPts[2];
        box[4] = box[0];
        context.GetIDrawGeom().DrawLineString3d (_countof(box), box, NULL);

        DPoint3d centerPt = DPoint3d::FromInterpolate(uvPts[0], 0.5, uvPts[3]);
        double pixelSize = context.GetPixelSizeAtPoint(&centerPt);

        Utf8String tileText;
        tileText.Sprintf("%d:%d,%d", m_tileId.resolution, m_tileId.x, m_tileId.y);
         
        TextStringPtr textStr = TextString::Create();
        textStr->SetText(tileText.c_str());
        textStr->GetStyleR().SetFont(DgnFontManager::GetDecoratorFont());
        textStr->GetStyleR().SetSize(pixelSize*10);
        textStr->SetOriginFromJustificationOrigin(centerPt, TextString::HorizontalJustification::Center, TextString::VerticalJustification::Middle);

        elemMatSymb.SetWidth (0);
        colorIdx = ColorDef(222, 0, 222, 128);
        elemMatSymb.SetLineColor (colorIdx);
        context.GetIDrawGeom().ActivateMatSymb (&elemMatSymb);
        context.GetIViewDraw().DrawTextString (*textStr);
        }

    if(!m_pDisplayTile.IsValid())
        return false;

    static bool s_invert = false;    //&&MM I don't understand why I need to that. investigate.
    if (s_invert)
        {
        std::swap (uvPts[0], uvPts[2]);
        std::swap (uvPts[1], uvPts[3]);
        }

    uintptr_t textureId = m_pDisplayTile->GetTextureId();
    context.GetIViewDraw().DrawMosaic (1,1, &textureId, uvPts); 
    
    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
DisplayTileP RasterTile::GetDisplayTileP(bool request) 
    {
    if(!m_pDisplayTile.IsValid() && TileStatus::Unloaded == m_status)
        {
        // Load only if locally available. If not a request is made to the cache and NULL is returned.
        m_pDisplayTile = m_tree.GetSource().QueryTile(m_tileId, request);        

        //&&MM handle change in status. 
        }

    return m_pDisplayTile.get();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  5/2015
//----------------------------------------------------------------------------------------
RasterSource::Resolution const& RasterTile::GetResolution() const {return m_tree.GetSource().GetResolution(m_tileId.resolution);}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
void RasterTile::AllocateChildren()
    {
    if(IsLeaf() || m_pChilds[0].IsValid())  // no res or already allocated.
        return;

    RasterSource::Resolution const& childrenResolution = m_tree.GetSource().GetResolution(m_tileId.resolution-1);

    // Upper-Left child, we always have one 
    TileId childUpperLeft(GetId().resolution-1, GetId().x << 1, GetId().y << 1);
    m_pChilds[0] = new RasterTile(childUpperLeft, this, m_tree);
    
    // Upper-Right
    if(childUpperLeft.x + 1 < childrenResolution.GetTileCountX())
        {
        TileId childUpperRight(childUpperLeft.resolution, childUpperLeft.x+1, childUpperLeft.y);
        m_pChilds[1] = new RasterTile(childUpperRight, this, m_tree);
        }

    // Lower-left
    if(childUpperLeft.y + 1 < childrenResolution.GetTileCountY())
        {
        TileId childLowerLeft(childUpperLeft.resolution, childUpperLeft.x, childUpperLeft.y+1);
        m_pChilds[2] = new RasterTile(childLowerLeft, this, m_tree);
        }

    // Lower-Right
    if(childUpperLeft.x + 1 < childrenResolution.GetTileCountX() && 
       childUpperLeft.y + 1 < childrenResolution.GetTileCountY())
        {
        TileId childLowerRight(childUpperLeft.resolution, childUpperLeft.x+1, childUpperLeft.y+1);
        m_pChilds[3] = new RasterTile(childLowerRight, this, m_tree);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
void RasterTile::QueryVisible(bvector<RasterTilePtr>& visibles, ViewContextR context)
    { 
    double factor;
    if (IsVisible(context, factor))
        {
        if (!IsLeaf() && factor > m_tree.GetVisibleQualityFactor())
            {
            AllocateChildren(); 

            size_t visiblesIn = visibles.size();
            for(size_t i=0; i < 4; ++i)
                {
                RasterTileP pChild = GetChildP(i);
                if(pChild != NULL)
                    pChild->QueryVisible(visibles, context);
                }
                
            //&&MM investigate how to handle reprojection where parent is visible and the 4 children are not.
            // reason: parent and children do not encompass the same region.                
            if(visiblesIn == visibles.size())
                {
                visibles.size();
                }
            }
        else
            {
            visibles.push_back(this);
            }
        }    
    }

/*---------------------------------------------------------------------------------**//**
* Inspired from HIEMriGridDisplayable::IsVisible()
* @bsimethod                                                    Mathieu.Marchand  4/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterTile::IsVisible (ViewContextR viewContext, double& factor) const
    {
    DPoint3d npcCorners[4];
    DPoint3d frustCorners[4];

    viewContext.LocalToWorld(frustCorners, m_corners, 4);
    viewContext.GetWorldToNpc().M0.MultiplyAndRenormalize(npcCorners, frustCorners, 4);

    DPoint3d npcOrigin;
    npcOrigin.x = MIN(MIN(npcCorners[0].x, npcCorners[1].x), MIN(npcCorners[2].x, npcCorners[3].x));
    npcOrigin.y = MIN(MIN(npcCorners[0].y, npcCorners[1].y), MIN(npcCorners[2].y, npcCorners[3].y));
    npcOrigin.z = MIN(MIN(npcCorners[0].z, npcCorners[1].z), MIN(npcCorners[2].z, npcCorners[3].z));
    
    if (npcOrigin.x > 1.0 || npcOrigin.y > 1.0 || npcOrigin.z > 1.0)
        {
        return false;
        }

    DPoint3d npcCorner;
    npcCorner.x = MAX(MAX(npcCorners[0].x, npcCorners[1].x), MAX(npcCorners[2].x, npcCorners[3].x));
    npcCorner.y = MAX(MAX(npcCorners[0].y, npcCorners[1].y), MAX(npcCorners[2].y, npcCorners[3].y));
    npcCorner.z = MAX(MAX(npcCorners[0].z, npcCorners[1].z), MAX(npcCorners[2].z, npcCorners[3].z));
    
    if (npcCorner.x < 0.0 || npcCorner.y < 0.0 || npcCorner.z < 0.0)
        {
        return false;
        }

    // compute scale factor of this tile
    DPoint3d viewCorners[4];
    viewContext.WorldToView(viewCorners, frustCorners, 4);

    uint64_t physWidth = m_tree.GetSource().GetTileSizeX(m_tileId);
    uint64_t physHeight = m_tree.GetSource().GetTileSizeY(m_tileId);

    //&&MM need to change the factor interpretation. if we want to use the squared version
    //   -- visibles tiles are not always on the same resolution usually on border tiles.
    //          when reprojecting we need all tiles to have the same connection points to avoid gaps. that means use the highest resolution?
#if 0   
    double physicalDiagSquared = (double)(physWidth*physWidth + physHeight*physHeight);
    double viewDiag1Squared = viewCorners[3].DistanceSquaredXY(&viewCorners[0]);
    double viewDiag2Squared = viewCorners[2].DistanceSquaredXY(&viewCorners[1]);
    double averageViewDiagSquared = (viewDiag1Squared + viewDiag2Squared) / 2.0;
    factorSquared = averageViewDiagSquared / physicalDiagSquared;
#else
    double physicalDiag = sqrt(physWidth*physWidth + physHeight*physHeight);
    double viewDiag1 = viewCorners[3].DistanceXY(viewCorners[0]);
    double viewDiag2 = viewCorners[2].DistanceXY(viewCorners[1]);
    double averageViewDiag = (viewDiag1 + viewDiag2) / 2.0;
    factor = averageViewDiag / physicalDiag;

    DPoint3d centerPt = DPoint3d::FromInterpolate(m_corners[0], 0.5, m_corners[3]);centerPt;
    double pixelSize = viewContext.GetPixelSizeAtPoint(&centerPt);pixelSize;

    double widthView = viewCorners[1].DistanceXY(viewCorners[0]);widthView;
    double heightView = viewCorners[3].DistanceXY(viewCorners[2]);heightView;

    double uorDiag1 = m_corners[3].DistanceXY(m_corners[0]);uorDiag1;
    double uorDiag2 = m_corners[2].DistanceXY(m_corners[1]);uorDiag2;
    
#endif

    return true;
    }

//----------------------------------------------------------------------------------------
//-------------------------------  RasterQuadTree --------------------------------------------
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
RasterQuadTreePtr RasterQuadTree::Create(RasterSourceR source, DgnDbR dgnDb)
    {
    return new RasterQuadTree(source, dgnDb);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
RasterQuadTree::RasterQuadTree(RasterSourceR source, DgnDbR dgnDb)
:m_pSource(&source), // increment ref count.
 m_dgnDb(dgnDb)
    {    
    // Create the lowest LOD but do not load its data yet.
    m_pRoot = RasterTile::CreateRoot(*this);

    m_visibleQualityFactor = 1.25; 
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
void RasterQuadTree::QueryVisible(bvector<RasterTilePtr>& visibles, ViewContextR context)
    {
    visibles.clear();

    m_pRoot->QueryVisible(visibles, context);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void RasterQuadTree::Draw (ViewContextR context)
    {
    RefCountedPtr<RasterProgressiveDisplay> display = RasterProgressiveDisplay::Create(*this, context);
    display->Draw(context);
    }

//----------------------------------------------------------------------------------------
//-------------------------------  RasterProgressiveDisplay -----------------------------------------
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
RasterProgressiveDisplay::RasterProgressiveDisplay(RasterQuadTreeR raster, ViewContextR context)
:m_raster(raster)
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
RasterProgressiveDisplay::~RasterProgressiveDisplay()
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
RefCountedPtr<RasterProgressiveDisplay> RasterProgressiveDisplay::Create(RasterQuadTreeR rasterTree, ViewContextR context)
    {
    return new RasterProgressiveDisplay(rasterTree, context);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
bool RasterProgressiveDisplay::ShouldDrawInConvext (ViewContextR context) const
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
// @bsimethod                                                   Mathieu.Marchand  5/2015
//----------------------------------------------------------------------------------------
void RasterProgressiveDisplay::DrawLoadedChildren(RasterTileR tile, ViewContextR context, uint32_t resolutionDelta)
    {
    if(0 == resolutionDelta)
        return;

    for(uint32_t i=0; i < 4; ++i)
        {
        RasterTileP pChild = tile.GetChildP(i);
        if(NULL == pChild)
            continue;

        if(pChild->IsLoaded())
            {
            pChild->Draw(context);
            }
        else
            {
            DrawLoadedChildren(*pChild, context, resolutionDelta-1);
            }
        }
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  5/2015
//----------------------------------------------------------------------------------------
void RasterProgressiveDisplay::FindBackgroudTiles(SortedTiles& backgroundTiles, bvector<RasterTilePtr> const& visibleTiles, uint32_t resolutionDelta)
    {
    for(auto const& pTile : visibleTiles)
        {
        if(!pTile->IsLoaded())
            {
            uint32_t maxResolution = pTile->GetId().resolution + resolutionDelta;
            for(RasterTileP pParent = pTile->GetParentP(); NULL != pParent; pParent = pParent->GetParentP())
                {
                if(pParent->IsLoaded())
                    {
                    backgroundTiles.insert(pParent);
                    break;
                    }

                if(pParent->GetId().resolution >= maxResolution)
                    break;
                }
            }
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
void RasterProgressiveDisplay::Draw (ViewContextR context)
    {
    // **********************************
    // *** NB: This method must be fast. 
    // **********************************
    // Do not try to read from SQLite or allocate huge amounts of memory in here. 
    // Defer time-consuming tasks to progressive display
    //&&MM review how we draw. Right now, we only draw here when we have the qv tile.
    //     everything else including download, read from cache, qv tile creation is done in the _Process callback.
    // - Can we cancel what we scheduled from the previous draw? 
    //      **** We really need that functionality because when zooming in/out we will request all intermediate levels and that can 
    //           take some time and bandwidth before we finally receive what we need.
    //  
    // - Can we request missing tiles here so it starts background work right away?
    // - Need a timer in _process to give time to tile to arrive but I would prefer to have an async call from the DataCache when data as arrived. 
    //   Instead of querying every missing tiles again and again in hopes that they are now available.
    //   We could also decompress in the worker thread instead of the progressive display thread. N.B. we cannot call Qv or host stuff in the worker thread. 
    // - Downloading from the web is the slowest operation. We should be able to query the datacache for existence(without read) and schedule missing data before
    //   we try loading(from cache and qv tile) of what is locally available.
    // - What happen during a dynamic draw or multiple zoom in/out (wheel mouse)? show we scheduled?


    // First, determine if we can draw map tiles at all.
    if (!ShouldDrawInConvext (context) || NULL == context.GetViewport())
        return;

    m_raster.QueryVisible(m_visiblesTiles, context);

    // Draw background tiles if any, that gives feedback when zooming in.
    // Background tiles need to be draw from coarser resolution to finer resolution to make sure coarser tiles are not hiding finer tiles.
//&&MM not now. we have a Z fighting issue between coarser and real tiles. 
//     SortedTiles backgroundTiles;
//     FindBackgroudTiles(backgroundTiles, m_visiblesTiles, 4/*maxResDelta*/);
//     for(RasterTilePtr& pTile : backgroundTiles)
//         {
//         BeAssert(pTile->IsLoaded());
//         pTile->Draw(context);
//         }

    // Draw what we have.
    for(RasterTilePtr& pTile : m_visiblesTiles)
        {
        if(pTile->IsLoaded())
            {
            pTile->Draw(context);
            }
        else
            {
//&&MM not now. we have a Z fighting issue between coarser and real tiles. 
            // Draw finer resolution.
            //DrawLoadedChildren(*pTile, context, 2/*resolutionDelta*/);

            m_missingTiles.push_back(pTile);
            }
        }

    if(!m_missingTiles.empty())
        {
        context.GetViewport()->ScheduleProgressiveDisplay (*this);
        m_waitTime = 50;
        m_nextRetryTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis() + m_waitTime;
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
IProgressiveDisplay::Completion RasterProgressiveDisplay::_Process(ViewContextR context)
    {
    if (BeTimeUtilities::GetCurrentTimeAsUnixMillis() < m_nextRetryTime)
        {
       // LOG.tracev("Wait %lld until next retry", m_nextRetryTime - BeTimeUtilities::GetCurrentTimeAsUnixMillis());
        return Completion::Aborted;
        }

    while(!m_missingTiles.empty())
        {
        auto pTile = m_missingTiles.back();
        m_missingTiles.pop_back();

        DisplayTileP pDisplayTile = pTile->GetDisplayTileP(true);  // Read from cache or request it. Won't be requested twice by the reality data cache.
        if(NULL != pDisplayTile)
            {
            pTile->Draw(context);
            }
        else
            {
            pTile->Draw(context);   //&&MM debug purpose. it will draw the border if data is missing.
            // Tile was not available and was requested. We'll check for it at a later time.
            m_pendingTiles.push_back(pTile);
            }

        if (context.CheckStop())
            return Completion::Aborted;
        }

    IProgressiveDisplay::Completion completion = Completion::Finished;

    for (auto pTile = m_pendingTiles.begin(); pTile != m_pendingTiles.end();  /* incremented or erased in loop*/ )
        {
        if (context.CheckStop())
            {
            completion = Completion::Aborted;
            break;
            }

        DisplayTileP pDisplayTile = (*pTile)->GetDisplayTileP(false);  // Read from cache. do not request again.
        if(NULL != pDisplayTile)
            {
            (*pTile)->Draw(context);
            pTile = m_pendingTiles.erase(pTile);
            }
        else
            {
            ++pTile;    // Will retry later
            completion = Completion::Aborted;   // at least one did not finish
            }        
        }

    if (completion != Completion::Finished)
        {
        m_waitTime = MIN(1000, (uint64_t)(m_waitTime * 1.1));
        m_nextRetryTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis() + m_waitTime;  
        }

    return completion;    
    }

//----------------------------------------------------------------------------------------
//-------------------------------  TEMP TEMP TEMP ----------------------------------------
//----------------------------------------------------------------------------------------

#if 0 //&&MM WMS Example
//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
void WebMercatorModel::_AddGraphicsToScene (ViewContextR context)
    {
    static bool s_useMercator = false;

    if(s_useMercator)
        {
        RefCountedPtr<WebMercatorDisplay> display = new WebMercatorDisplay(*this, *context.GetViewport());
        display->DrawView(context);
        }
    else
        {
        if(s_rasterLOD.IsNull())
            {
            // Make sure GCS is initialized
            T_HOST.GetGeoCoordinationAdmin()._GetServices();

                // how to get a in latlong BBOX: http://boundingbox.klokantech.com/
            #if 0
                m_serverUrl = "http://giswebservices.massgis.state.ma.us/geoserver/wms";
                m_version = "1.1.1";
                m_layers = "massgis:GISDATA.TOWNS_POLYM,massgis:GISDATA.NAVTEQRDS_ARC,massgis:GISDATA.NAVTEQRDS_ARC_INT";
                //m_styles = "Black_Lines,GISDATA.NAVTEQRDS_ARC::ForOrtho`s,GISDATA.NAVTEQRDS_ARC_INT::Default";
                m_styles = "Black_Lines,GISDATA.NAVTEQRDS_ARC::ForOrthos,GISDATA.NAVTEQRDS_ARC_INT::Default";
                m_gcsType = "SRS";
                m_gcs = "EPSG:26986"; 
                m_format = "image/png";
                m_transparent = "TRUE";

                // WMS BBOX are in CRS units. i.e. cartesian.
                m_corners[0].x = m_corners[2].x = 898705.3447384972; 
                m_corners[1].x = m_corners[3].x = 903749.1401484597;  
                m_corners[0].y = m_corners[1].y = 232325.38526025353; 
                m_corners[2].y = m_corners[3].y = 238934.49648710093; 
                m_corners[0].z = m_corners[1].z = m_corners[2].z = m_corners[3].z = 0;

                m_vendorSpecific = "SERVICE=WMS";
            #elif 0.
            //http://giswebservices.massgis.state.ma.us/geoserver/wms?request=GetCapabilities&service=WMS
                m_serverUrl = "http://giswebservices.massgis.state.ma.us/geoserver/wms";
                m_version = "1.1.1";
                m_layers = "massgis:GISDATA.TOWNS_POLYM,massgis:GISDATA.NAVTEQRDS_ARC,massgis:GISDATA.NAVTEQRDS_ARC_INT";
                m_styles = "Black_Lines,GISDATA.NAVTEQRDS_ARC::ForOrthos,GISDATA.NAVTEQRDS_ARC_INT::Default";
                m_gcsType = "SRS";
                m_gcs = "EPSG:26986";
                m_format = "image/png";
                //m_format = "image/jpeg";
                m_transparent = "TRUE";

                m_vendorSpecific = "SERVICE=WMS";

                // WMS BBOX are in CRS units. i.e. cartesian.
                m_corners[0].x = m_corners[2].x = 232325.38526025353; 
                m_corners[1].x = m_corners[3].x = 238934.49648710093;  
                m_corners[0].y = m_corners[1].y = 898705.3447384972; 
                m_corners[2].y = m_corners[3].y = 903749.1401484597; 
                m_corners[0].z = m_corners[1].z = m_corners[2].z = m_corners[3].z = 0;
            #elif 1
                //http://basemap.nationalmap.gov/arcgis/services/USGSImageryTopo/MapServer/WMSServer?request=GetCapabilities&service=WMS

                Utf8String serverUrl = "http://basemap.nationalmap.gov/arcgis/services/USGSImageryTopo/MapServer/WmsServer";
                Utf8String version = "1.1.1";
                Utf8String layers = "0";
                Utf8String styles = "";
                Utf8String csType = "SRS";
                Utf8String csLabel = "EPSG:4269";
                Utf8String format = "image/png";
                Utf8String transparent = "TRUE";
           
                DRange2d bbox;
                bbox.low.x = -128.1;
                bbox.low.y = 24.7;
                bbox.high.x = -58.1;
                bbox.high.y = 49.8;   
            #elif 1
                //http://ows.geobase.ca/wms/geobase_en?service=wms&request=getCapabilities
                m_serverUrl = "http://ows.geobase.ca/wms/geobase_en";
                m_version = "1.1.1";
            #if 0 
                m_layers = "imagery:landsat7,reference:roads,reference:hydro,reference:boundaries";
                m_styles = ",,,";
            #else
                m_layers = "elevation:cded50k,reference:hydro,reference:roads,boundaries:geopolitical,reference:boundaries,nrwn:track,reference:placenames:capitals10m";
                m_styles = ",,,,,,";
            #endif    
                m_gcsType = "SRS";
                m_gcs = "EPSG:4269";
                m_format = "image/png";
                //m_format = "image/jpeg";
                m_transparent = "TRUE";

                m_vendorSpecific = "wms=WorldMap";

    
                //"[[-71.54944499999999,46.734076800000025],[-71.54944499999999,46.98193789999996],[-71.14270069999999,46.98193789999996],[-71.14270069999999,46.734076800000025]]"

                // WMS BBOX are in CRS units. i.e. cartesian.
                m_corners[0].x = m_corners[2].x = -78.18;
                m_corners[1].x = m_corners[3].x = -69.48;  
                m_corners[0].y = m_corners[1].y = 43.83; 
                m_corners[2].y = m_corners[3].y = 49.45; 
                m_corners[0].z = m_corners[1].z = m_corners[2].z = m_corners[3].z = 0;

            //     m_corners[0].x = m_corners[2].x = -94.605;
            //     m_corners[1].x = m_corners[3].x = -48.06;  
            //     m_corners[0].y = m_corners[1].y = 41.68; 
            //     m_corners[2].y = m_corners[3].y = 63.36; 
            //     m_corners[0].z = m_corners[1].z = m_corners[2].z = m_corners[3].z = 0;

                // Canada
            //     m_corners[0].x = m_corners[2].x = -141.15; 
            //     m_corners[1].x = m_corners[3].x = -48.06;  
            //     m_corners[0].y = m_corners[1].y = 41.68; 
            //     m_corners[2].y = m_corners[3].y = 85.05; 
            //     m_corners[0].z = m_corners[1].z = m_corners[2].z = m_corners[3].z = 0;

            #else
                //http://www2.demis.nl/wms/wms.asp?wms=WorldMap&request=GetCapabilities&version=1.1.1
                m_serverUrl = "http://www3.demis.nl/wms/wms.asp";
                m_version = "1.1.1";
                m_layers = "Topography,Rivers,Railroads,Roads,Borders,Cities";
                m_styles = ",,,,,";
                m_gcsType = "SRS";
                m_gcs = "EPSG:4326";
                m_format = "image/png";
                //m_format = "image/jpeg";
                m_transparent = "TRUE";

                m_vendorSpecific = "wms=WorldMap";

                // WMS BBOX are in CRS units. i.e. cartesian.
                m_corners[0].x = m_corners[2].x = -106; 
                m_corners[1].x = m_corners[3].x = -86;  
                m_corners[0].y = m_corners[1].y = 40; 
                m_corners[2].y = m_corners[3].y = 80; 
                m_corners[0].z = m_corners[1].z = m_corners[2].z = m_corners[3].z = 0;
            #endif

                
            // Keep the same cartesian to pixel ratio
            double xLength = bbox.high.x - bbox.low.x;
            double yLength = bbox.high.y - bbox.low.y;
                        
            uint32_t metaWidth = 2048*256;  // arbitrary size. 
            uint32_t metaHeight = (uint32_t)((yLength/xLength) * metaWidth);
                        
            s_rasterLOD = RasterLOD::Create(*WmsSource::Create(serverUrl.c_str(), version.c_str(), layers.c_str(), styles.c_str(), csType.c_str(), csLabel.c_str(), format.c_str(), bbox, metaWidth, metaHeight), context.GetDgnDb());
            }

        RefCountedPtr<RasterProgressiveDisplay> display = RasterProgressiveDisplay::Create(*s_rasterLOD, context);
        display->Draw(context);
        }
    }
#endif