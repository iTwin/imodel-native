/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterTileTree.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RasterSchemaInternal.h"
#include "RasterTileTree.h"

//static const uint32_t DRAW_FINER_DELTA = 2;
static const uint32_t DRAW_COARSER_DELTA = 6;

//=======================================================================================
//! &&MM review the need for this class.
// @bsiclass                                                    Mathieu.Marchand  9/2016
//=======================================================================================
struct TileQuery
    {
    TileQuery(RasterTile& tile) : m_tile(&tile) {}

    RefCountedPtr<RasterTile> m_tile;

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  9/2016
    //----------------------------------------------------------------------------------------
    BentleyStatus DoRead() const
        {
        if (m_tile.IsValid() && !m_tile->IsQueued())
            return SUCCESS; // this node was abandoned.

        bool enableAlphaBlend = false;
        Render::Image image = m_tile->GetRoot().GetSource().QueryTile(m_tile->GetTileId(), enableAlphaBlend);
#ifndef NDEBUG  // debug build only.
        static bool s_missingTilesInRed = false;
        if (s_missingTilesInRed && !image.IsValid())
            {
            ByteStream data(256 * 256 * 3);
            Byte red[3] = {255,0,0};
            for (uint32_t pixel = 0; pixel < 256 * 256; ++pixel)
                memcpy(data.GetDataP() + pixel * 3, red, 3);

            image = Render::Image(256, 256, std::move(data), Render::Image::Format::Rgb);
            enableAlphaBlend = false;
            }
#endif  

        if (!image.IsValid())
            {
            m_tile->SetNotFound();
            return ERROR;
            }

        auto graphic = m_tile->GetRoot().GetRenderSystem()->_CreateGraphic(Render::Graphic::CreateParams(nullptr));

        Render::Texture::CreateParams params;
        params.SetIsTileSection();  // tile section have clamp instead of warp mode for out of bound pixels. That help reduce seams between tiles when magnified.
        auto texture = m_tile->GetRoot().GetRenderSystem()->_CreateTexture(image, params);

        graphic->SetSymbology(ColorDef::White(), ColorDef::White(), 0);
        graphic->AddTile(*texture, m_tile->GetCorners());

        auto stat = graphic->Close();
        BeAssert(SUCCESS == stat);
        UNUSED_VARIABLE(stat);

        m_tile->m_graphic = graphic;
        m_tile->SetIsReady();

        return SUCCESS;
        }
    };

//---------------------------------------------------------------------------------------------
//-------------------------------- RasterRoot -------------------------------------------------
//---------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
RasterRoot::RasterRoot(RasterSourceR source, RasterModel& model, Dgn::Render::SystemP system)
    :TileTree::Root(model.GetDgnDb(), Transform::FromIdentity(), model.GetName().c_str(), "", system),
    m_source(&source), m_model(model)
    {
    //&&MM need to reproject.
    // Init m_physicalToWorld transformation
    DMatrix4d physicalToWorld;
    physicalToWorld.InitProduct(model.GetSourceToWorld(), DMatrix4d::From(source._PhysicalToSource()));
    DMatrix4d worldToPhysical;
    worldToPhysical.QrInverseOf(physicalToWorld);
    m_physicalToWorld.InitFrom(physicalToWorld, worldToPhysical);

    // not required for non http. CreateCache(100 * 1024 * 1024); // 100 Mb
    m_rootTile = new RasterTile(*this, TileId(m_source->GetResolutionCount() - 1, 0, 0), nullptr);
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
RasterRoot::~RasterRoot()
    {
    m_rootTile = nullptr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
folly::Future<BentleyStatus> RasterRoot::_RequestTile(TileTree::TileCR tile)
    {
    DgnDb::VerifyClientThread();

    if (!tile.IsNotLoaded()) // this should only be called when the tile is in the "not loaded" state.
        {
        BeAssert(false);
        return ERROR;
        }

    tile.SetIsQueued();

    TileQuery query((RasterTile&) tile);
    return folly::via(&BeFolly::IOThreadPool::GetPool(), [=] () { return query.DoRead(); });
    }

//---------------------------------------------------------------------------------------------
//-------------------------------- RasterTile -------------------------------------------------
//---------------------------------------------------------------------------------------------

/*---------------------------------------------------------------------------------**//**
* Construct a new MapTile by TileId.
* @bsimethod                                    Mathieu.Marchand                9/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RasterTile::RasterTile(RasterRootR root, TileId id, RasterTileCP parent)
    : Tile(parent), m_root(root), m_id(id)
    {
    uint32_t xMinInRes = id.x * m_root.GetSource().GetResolution(id.resolution).GetTileSizeX();
    uint32_t yMinInRes = id.y * m_root.GetSource().GetResolution(id.resolution).GetTileSizeY();
    uint32_t xMaxInRes = xMinInRes + m_root.GetSource().GetResolution(id.resolution).GetTileSizeX();
    uint32_t yMaxInRes = yMinInRes + m_root.GetSource().GetResolution(id.resolution).GetTileSizeY();

    uint32_t xMin = xMinInRes << id.resolution;
    uint32_t yMin = yMinInRes << id.resolution;
    uint32_t xMax = xMaxInRes << id.resolution;
    uint32_t yMax = yMaxInRes << id.resolution;

    // Limit the tile extent to the raster physical size 
    if (xMax > m_root.GetSource().GetWidth())
        xMax = m_root.GetSource().GetWidth();
    if (yMax > m_root.GetSource().GetHeight())
        yMax = m_root.GetSource().GetHeight();

    DPoint3d physicalCorners[4];
    physicalCorners[0].x = physicalCorners[2].x = xMin;
    physicalCorners[1].x = physicalCorners[3].x = xMax;
    physicalCorners[0].y = physicalCorners[1].y = yMin;
    physicalCorners[2].y = physicalCorners[3].y = yMax;
    physicalCorners[0].z = physicalCorners[1].z = physicalCorners[2].z = physicalCorners[3].z = 0;
    m_root.GetPhysicalToWorld().MultiplyAndRenormalize(m_corners.m_pts, physicalCorners, 4);

    m_range.InitFrom(m_corners.m_pts, 4);
    m_range.low.z = -1.0;
    m_range.high.z = 1.0;
    m_reprojected = true;

    if (parent)
        parent->ExtendRange(m_range);

    // That max size is the radius and not the diagonal of the bounding sphere in pixels, this is why there is a /2.
    uint32_t tileSizeX = m_root.GetSource().GetTileSizeX(GetTileId());
    uint32_t tileSizeY = m_root.GetSource().GetTileSizeY(GetTileId());
    m_maxSize = sqrt(tileSizeX*tileSizeX + tileSizeY*tileSizeY) / 2;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
Utf8String RasterTile::_GetTileName() const
    {
    return Utf8PrintfString("%d/%d/%d", m_id.resolution, m_id.x, m_id.y);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
TileTree::Tile::ChildTiles const* RasterTile::_GetChildren(bool load) const
    {
    if (!_HasChildren()) // is this is the highest resolution tile?
        return nullptr;

    if (load && m_children.empty())
        {
        // this Tile has children, but we haven't created them yet. Do so now
        RasterSource::Resolution const& childrenResolution = m_root.GetSource().GetResolution(m_id.resolution - 1);

        // Upper-Left child, we always have one 
        TileId childUpperLeft(m_id.resolution - 1, m_id.x << 1, m_id.y << 1);
        m_children.push_back(new RasterTile(m_root, childUpperLeft, this));

        // Upper-Right
        if (childUpperLeft.x + 1 < childrenResolution.GetTileCountX())
            {
            TileId childUpperRight(childUpperLeft.resolution, childUpperLeft.x + 1, childUpperLeft.y);
            m_children.push_back(new RasterTile(m_root, childUpperRight, this));
            }

        // Lower-left
        if (childUpperLeft.y + 1 < childrenResolution.GetTileCountY())
            {
            TileId childLowerLeft(childUpperLeft.resolution, childUpperLeft.x, childUpperLeft.y + 1);
            m_children.push_back(new RasterTile(m_root, childLowerLeft, this));
            }

        // Lower-Right
        if (childUpperLeft.x + 1 < childrenResolution.GetTileCountX() &&
            childUpperLeft.y + 1 < childrenResolution.GetTileCountY())
            {
            TileId childLowerRight(childUpperLeft.resolution, childUpperLeft.x + 1, childUpperLeft.y + 1);
            m_children.push_back(new RasterTile(m_root, childLowerRight, this));
            }
        }

    return &m_children;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
bool RasterTile::TryLowerRes(TileTree::DrawArgsR args, int depth) const
    {
    RasterTile* parent = (RasterTile*) m_parent;
    if (depth <= 0 || nullptr == parent)
        {
        // DEBUG_PRINTF("no lower res");
        return false;
        }

    if (parent->HasGraphics())
        {
        //DEBUG_PRINTF("using lower res %d", depth);
        args.m_substitutes.Add(*parent->m_graphic);
        return true;
        }

    return parent->TryLowerRes(args, depth - 1); // recursion
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
void RasterTile::TryHigherRes(TileTree::DrawArgsR args) const
    {
    for (auto const& child : m_children)
        {
        RasterTile* mapChild = (RasterTile*) child.get();

        if (mapChild->HasGraphics())
            {
            //DEBUG_PRINTF("using higher res");
            args.m_substitutes.Add(*mapChild->m_graphic);
            }
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
void RasterTile::_DrawGraphics(TileTree::DrawArgsR args, int depth) const
    {
    if (!m_reprojected)     // if we were unable to reproject this tile, don't try to draw it.
        return;

    if (!IsReady())
        {
        if (!IsNotFound())
            args.m_missing.Insert(depth, this);

        if (!TryLowerRes(args, DRAW_COARSER_DELTA))
            TryHigherRes(args);

        return;
        }

    if (m_graphic.IsValid())
        args.m_graphics.Add(*m_graphic);
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------                                                                                     +---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RasterTile::_LoadTile(TileTree::StreamBuffer& data, TileTree::RootR root)
    {
    BeAssert(!"not expected we load from _RequestTile");

    return ERROR;
    }

//---------------------------------------------------------------------------------------------
//-------------------------------- RasterProgressive ------------------------------------------
//---------------------------------------------------------------------------------------------

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.Marchand                9/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ProgressiveTask::Completion RasterProgressive::_DoProgressive(ProgressiveContext& context, WantShow& wantShow)
    {
    auto now = std::chrono::steady_clock::now();
    TileTree::DrawArgs args(context, m_root.GetLocation(), now, now - m_root.GetExpirationTime());
    args.SetClip(m_root.GetModel().GetClip().GetClipVector());

    DEBUG_PRINTF("Map progressive %d missing", m_missing.size());

    for (auto const& node : m_missing)
        {
        auto stat = node.second->GetLoadState();
        if (stat == TileTree::Tile::LoadState::Ready)
            node.second->Draw(args, node.first);        // now ready, draw it (this potentially generates new missing nodes)
        else if (stat != TileTree::Tile::LoadState::NotFound)
            args.m_missing.Insert(node.first, node.second);     // still not ready, put into new missing list
        }

    args.RequestMissingTiles(m_root);
    args.DrawGraphics(context);  // the nodes that newly arrived are in the GraphicBranch in the DrawArgs. Add them to the context 

    m_missing.swap(args.m_missing); // swap the list of missing tiles we were waiting for with those that are still missing.

    DEBUG_PRINTF("Map after progressive still %d missing", m_missing.size());
    if (m_missing.empty()) // when we have no missing tiles, the progressive task is done.
        {
        context.GetViewport()->SetNeedsHeal(); // unfortunately the newly drawn tiles may be obscured by lower resolution ones
        return Completion::Finished;
        }

    if (now > m_nextShow)
        {
        m_nextShow = now + std::chrono::seconds(1); // once per second
        wantShow = WantShow::Yes;
        }

    return Completion::Aborted;
    }



#if 0 //&&MM this is how we reproject 4 corners. 
//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  5/2015
//----------------------------------------------------------------------------------------
ReprojectStatus RasterTile::ReprojectCorners(DPoint3dP outUors, DPoint3dCP srcCartesian, RasterQuadTreeR tree)
    {
    //&&MM WIP reproject
    tree.GetSourceToWorld().MultiplyAndRenormalize(outUors, srcCartesian, 4);
    
    ReprojectStatus status = REPROJECT_Success;

    static bool s_doExactTransform = false;  // for debugging

    if (s_doExactTransform)
        {
        DPoint3d outUors2[4];
        GeoCoordinates::BaseGCSP pSourceGcs = tree.GetSource().GetGcsP();

        DgnGCSP pDgnGcs = tree.GetDgnDb().Units().GetDgnGCS();

        if (NULL == pSourceGcs || NULL == pDgnGcs || !pSourceGcs->IsValid() || !pDgnGcs->IsValid())
            {
            // Assume raster to be coincident.
            memcpy(outUors, srcCartesian, sizeof(DPoint3d) * 4);
            return REPROJECT_Success;
            }

        GeoPoint srcGeoCorners[4];
        for (size_t i = 0; i < 4; ++i)
            {
            if (REPROJECT_Success != (status = s_FilterGeocoordWarning(pSourceGcs->LatLongFromCartesian(srcGeoCorners[i], srcCartesian[i]))))
                {
                BeAssert(!"A source should always be able to represent itself in its GCS."); // That operation cannot fail or can it?
                return status;
                }
            }

        // Source latlong to DgnDb latlong.
        GeoPoint dgnGeoCorners[4];
        for (size_t i = 0; i < 4; ++i)
            {
            if (REPROJECT_Success != (status = s_FilterGeocoordWarning(pSourceGcs->LatLongFromLatLong(dgnGeoCorners[i], srcGeoCorners[i], *pDgnGcs))))
                return status;
            }

        //Finally to UOR
        for (uint32_t i = 0; i < 4; ++i)
            {
            if (REPROJECT_Success != (status = s_FilterGeocoordWarning(pDgnGcs->UorsFromLatLong(outUors2[i], dgnGeoCorners[i]))))
                return status;
            }
        }

    return status;    
#endif

#if 0
//----------------------------------------------------------------------------------------
// @bsiclass                                                   Mathieu.Marchand  7/2016
//----------------------------------------------------------------------------------------
struct ReprojectionApproximater
    {

    ReprojectionApproximater(GeoCoordinates::BaseGCSR srcGcs, DgnGCSR dgnGcs) :m_sourceGCS(srcGcs), m_destGCS(dgnGcs)
        {
//         HFCPtr<HGF2DPolygonOfSegments> pSrcDomainLL = GetDomainShape(*pSourceGcs);
//         HFCPtr<HGF2DPolygonOfSegments> pDgnDomainLL = GetDomainShape(*pDgnGcs);
//         m_pDomainInLatLong  = pSrcDomainLL->IntersectShape(*pDgnDomainLL);       
        }

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  7/2016
    //----------------------------------------------------------------------------------------
    StatusInt AddPointUOR(DPoint3dCR uorPt)
        {
        DPoint3d srcCartesianPt;

        ReprojectStatus status;

        GeoPoint dgnGeoPt;
        if (REPROJECT_Success != (status = s_FilterGeocoordWarning(m_destGCS.LatLongFromUors(dgnGeoPt, uorPt))))
            return ERROR;

        GeoPoint srcGeoPt;
        if (REPROJECT_Success != (status = s_FilterGeocoordWarning(m_destGCS.LatLongFromLatLong(srcGeoPt, dgnGeoPt, m_sourceGCS))))
            return ERROR;
        
        if (REPROJECT_Success != (status = s_FilterGeocoordWarning(m_sourceGCS.CartesianFromLatLong(srcCartesianPt, srcGeoPt))))
            return ERROR;

       
        AddTiePoints(srcCartesianPt, uorPt);

        return SUCCESS;
        }

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  7/2016
    //----------------------------------------------------------------------------------------
    StatusInt ReprojectPoint(DPoint3dR uorPt, DPoint3dCR srcCartesianPt)
        {
        ReprojectStatus status;

        // Source cartesian to lat/long.
        GeoPoint srcGeoPt;
        if (REPROJECT_Success != (status = s_FilterGeocoordWarning(m_sourceGCS.LatLongFromCartesian(srcGeoPt, srcCartesianPt))))
            return ERROR;

        // Source lat/long to DgnDb lat/long.
        GeoPoint dgnGeoPt;
        if (REPROJECT_Success != (status = s_FilterGeocoordWarning(m_sourceGCS.LatLongFromLatLong(dgnGeoPt, srcGeoPt, m_destGCS))))
            return ERROR;

        // Finally to UOR
        if (REPROJECT_Success != (status = s_FilterGeocoordWarning(m_destGCS.UorsFromLatLong(uorPt, dgnGeoPt))))
            return ERROR;

        return SUCCESS;
        }

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  7/2016
    //----------------------------------------------------------------------------------------
    StatusInt AddPoint(DPoint3dCR srcCartesianPt)
        {
        DPoint3d uorPt;
        if (SUCCESS != ReprojectPoint(uorPt, srcCartesianPt))
            return ERROR;            

        AddTiePoints(srcCartesianPt, uorPt);
        return SUCCESS;
        }

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  7/2016
    //----------------------------------------------------------------------------------------
    void AddTiePoints(DPoint3dCR badPt, DPoint3dCR goodPt)
        {
        m_tiePoints.push_back(badPt);
        m_tiePoints.push_back(goodPt);
        }

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  7/2016
    //----------------------------------------------------------------------------------------
    StatusInt ComputeProjective(DMatrix4dR out)
        {
        if (0 != HGF2DDCTransfoModel::GetProjectiveTransfoMatrixFromScaleAndTiePts(out.coff, (uint16_t)m_tiePoints.size() * 3, (double const*) m_tiePoints.data()))
            return ERROR;
        
        return SUCCESS;
        }

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  7/2016
    //----------------------------------------------------------------------------------------
    static HFCPtr<HGF2DPolygonOfSegments> GetDomainShape(GeoCoordinates::BaseGCSR geoCs)
        {
        bvector<GeoPoint> geoDomainLL;  // in lat/long
        geoCs.GetMathematicalDomain(geoDomainLL);

        HGF2DCoordCollection<double> geoPoints;
        for (GeoPoint const& geoPt : geoDomainLL)
            geoPoints.push_back(HGF2DCoord<double>(geoPt.longitude, geoPt.latitude));

        HFCPtr<HGF2DPolygonOfSegments> pDomainShape(new HGF2DPolygonOfSegments(geoPoints));

        return pDomainShape;
        }

    void ClearAll() { m_tiePoints.clear(); }
    size_t GetTiePointCount() {return m_tiePoints.size()/2;}

    //HFCPtr<HGF2DShape>    m_pDomainInLatLong;
    std::vector<DPoint3d> m_tiePoints;

    GeoCoordinates::BaseGCSR m_sourceGCS;
    DgnGCSR                  m_destGCS;
    };
#endif

#if 0 //&&MM This is how to compute a projection approx.
//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  7/2016
//----------------------------------------------------------------------------------------
StatusInt RasterQuadTree::ComputeSourceToWorldTransform(DMatrix4dR sourceToWorld)
    {
    sourceToWorld.InitIdentity();

    GeoCoordinates::BaseGCSP pSourceGcs = GetSource().GetGcsP();
    DgnGCSP pDgnGcs = GetDgnDb().Units().GetDgnGCS();

    if (nullptr == pSourceGcs || nullptr == pDgnGcs || !pSourceGcs->IsValid() || !pDgnGcs->IsValid())
        return SUCCESS; // Assume raster to be coincident.
        
    //&&MM test for same GCS but also make sure units or UOR factor are applied
    //!m_pDgnGCS->IsEquivalent(*pRasterGcs)

    // Create a shapeCartesian in with src corners.
    DPoint3dCP sourceCorners = &GetSource().GetCorners();
    
    ReprojectionApproximater approx(*pSourceGcs, *pDgnGcs);

    double seed[] = {0.15, 0.5, 0.85};
    size_t  seedCount = sizeof(seed) / sizeof(seed[0]);

    bool hadFailure = false;
    for (size_t y = 0; y < seedCount; ++y)
        {
        for (size_t x = 0; x < seedCount; ++x)
            {
            if (SUCCESS != approx.AddPoint(DPoint3d::FromInterpolateBilinear(sourceCorners[0], sourceCorners[1], sourceCorners[2], sourceCorners[3], seed[x], seed[y])))
                {
                //&&MM and do what? move toward the center of the raster or domain or project extent.
                hadFailure = true;
                x = y = seedCount;
                }
            }
        }

    //&&MM we should test that we have enough pt if not do something else?
    // even if we have enough the distribution might not be good enough.
#if 0
    if (hadFailure)
        {
        approx.ClearAll();

        // Project contains extent of geometries only. ->> not used right now.
        AxisAlignedBox3d projectExtent = GetDgnDb().Units().GetProjectExtents();

        DPoint3d box[8];
        projectExtent.Get8Corners(box);
        for (size_t y = 0; y < seedCount; ++y)
            {
            for (size_t x = 0; x < seedCount; ++x)
                {
                if (SUCCESS != approx.AddPointUOR(DPoint3d::FromInterpolateBilinear(box[0], box[1], box[2], box[3], seed[x], seed[y])))
                    {
                    //&&MM and do what? move toward the center of the raster or domain or project extent.
//                     hadFailure = true;
//                     x = y = seedCount;
                    }
                }
            }
        }
#endif
    
    return approx.ComputeProjective(sourceToWorld);    
    }
#endif
