/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterFileSource.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RasterInternal.h"
#include "RasterFileSource.h"

USING_NAMESPACE_TILETREE

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  2/2016
//----------------------------------------------------------------------------------------
static HFCPtr<HRPPixelType> getTileQueryPixelType(HRARaster const& raster, Render::Image::Format& format)
    {
    //&&MM TODO binary support.  We need to send RLE to QV so it uses nearest sampler instead on average.
    // Disabling assert because it pops during the build/run of converter tests.
    //BeAssertOnce(raster.GetPixelType()->CountIndexBits() != 1);       //TODO binary support.

    // if source holds alpha use Rgba
    if (raster.GetPixelType()->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE)
        {
        format = Render::Image::Format::Rgba;
        return new HRPPixelTypeV32R8G8B8A8();
        }

    format = Render::Image::Format::Rgb;
    return new HRPPixelTypeV24R8G8B8();
    }

//=======================================================================================
// @bsiclass                                                    Mathieu.Marchand  9/2016
//=======================================================================================
struct TileQuery
    {
    TileQuery(RasterFileTileR tile, TileLoadsPtr loads) : m_tile(&tile), m_loads(loads) {}

    RefCountedPtr<RasterFileTile> m_tile;
    mutable TileLoadsPtr m_loads;

    RasterFileSourceR GetFileSource() const { return static_cast<RasterFileSourceR>(m_tile->GetRoot()); }

    struct TileLoader
        {
        Root& m_root;
        TileLoader(Root& root) : m_root(root) { root.StartTileLoad(); }
        ~TileLoader() { m_root.DoneTileLoad(); }
        };

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  9/2016
    //----------------------------------------------------------------------------------------
    BentleyStatus DoRead() const
        {
        if (m_loads != nullptr && m_loads->IsCanceled())
            {
            if (m_tile.IsValid())
                m_tile->SetNotLoaded();

            return ERROR;
            }

        if (m_tile.IsValid() && !m_tile->IsQueued())
            return SUCCESS; // this node was abandoned.

        TileLoader loadFlag(m_tile->GetRoot());
        bool enableAlphaBlend = false;
        Render::Image image = GetFileSource().QueryTile(m_tile->GetTileId(), enableAlphaBlend);

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

        if (m_tile->IsAbandoned())
            return ERROR;

        auto graphic = m_tile->GetRoot().GetRenderSystem()->_CreateGraphic(Render::Graphic::CreateParams(nullptr));

        Render::Texture::CreateParams params;
        params.SetIsTileSection();  // tile section have clamp instead of warp mode for out of bound pixels. That help reduce seams between tiles when magnified.
        auto texture = m_tile->GetRoot().GetRenderSystem()->_CreateTexture(image, params);

        graphic->SetSymbology(ColorDef::White(), ColorDef::White(), 0);
        graphic->AddTile(*texture, m_tile->GetCorners());

        auto stat = graphic->Close(); // explicitly close the Graphic. This potentially blocks waiting for QV from other threads
        BeAssert(SUCCESS == stat);
        UNUSED_VARIABLE(stat);

        m_tile->m_graphic = graphic;
        m_tile->SetIsReady(); // OK, we're all done loading and the other thread may now use this data. Set the "ready" flag.

        if (m_loads != nullptr)
            m_loads->m_fromFile.IncrementAtomicPre(std::memory_order_relaxed);

        return SUCCESS;
        }
    };

//----------------------------------------------------------------------------------------
//-------------------------------   RasterFileSource   -----------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
RasterFileSourcePtr RasterFileSource::Create(Utf8StringCR resolvedName, RasterFileModel& model, Dgn::Render::SystemP system)
    {
    // Open raster file
    auto rasterFile = RasterFile::Create(resolvedName);
    if (rasterFile.IsNull())
        return nullptr;  // Can't create model; probably that file name is invalid.
        
    return new RasterFileSource(*rasterFile, model, system);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
RasterFileSource::~RasterFileSource()
    {
    ClearAllTiles();        // wait for pending tasks that use the raster before we destroy it.
    m_rasterFile = nullptr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
RasterFileSource::RasterFileSource(RasterFileR rasterFile, RasterFileModel& model, Dgn::Render::SystemP system)
    :RasterRoot(model, model.GetName().c_str(), ""/*rootUrl*/, system),
     m_rasterFile(&rasterFile),
     m_model(model)
    {  
    Point2d sizePixels;
    m_rasterFile->GetSize(&sizePixels);

    // Compute transformation to a LOWER_LEFT_HORIZONTAL origin 
    switch (m_rasterFile->GetHRFRasterFile().GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetScanlineOrientation().m_ScanlineOrientation)
        {
        case HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL: // SLO 6
            m_physicalToSource.InitIdentity();
            break;

        case HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL: // SLO 4
            m_physicalToSource = Transform::FromScaleFactors(1, -1, 1);
            m_physicalToSource.SetTranslation(DPoint2d::From(0, sizePixels.y));
            break;

            // Adapt everything else like a SLO4? binaries have their SLO transform to SLO 4(and/or SLO 6?) added to the page transfo model.
        default:
            m_physicalToSource = Transform::FromScaleFactors(1, -1, 1);
            m_physicalToSource.SetTranslation(DPoint2d::From(0, sizePixels.y));
            break;
        }

    // Raster width and height come from the raster file.
    // And resolution definition should come from the raster resolution descriptor.
    GenerateResolution(m_resolution, sizePixels.x, sizePixels.y, 256, 256);     // Tile size. We always use 256 for display

    // Init physicalToWorld transformation.
    DMatrix4d physicalToWorld;
    physicalToWorld.InitProduct(model.GetSourceToWorld(), DMatrix4d::From(m_physicalToSource));
    DMatrix4d worldToPhysical;
    worldToPhysical.QrInverseOf(physicalToWorld);
    m_physicalToWorld.InitFrom(physicalToWorld, worldToPhysical);

    // not required for non http. CreateCache(100 * 1024 * 1024); // 100 Mb
    m_rootTile = new RasterFileTile(*this, TileId(GetResolutionCount() - 1, 0, 0), nullptr);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
Render::Image RasterFileSource::QueryTile(TileId const& id, bool& alphaBlend)
    {
    // Imagepp CopyFrom is not thread safe. sequentialize the queries.
    static std::mutex s_ippMutex;
    std::unique_lock<std::mutex> __ippLock(s_ippMutex);

    // Must be done within lock because GetRasterP might load the raster.
    if (m_rasterFile == nullptr || m_rasterFile->GetRasterP() == nullptr)
        return Render::Image();

    Render::Image::Format imageFormat = Render::Image::Format::Rgb;
    HFCPtr<HRPPixelType> pPixelType = getTileQueryPixelType(*m_rasterFile->GetRasterP(), imageFormat);

    uint32_t effectiveTileSizeX = GetTileSizeX(id);
    uint32_t effectiveTileSizeY = GetTileSizeY(id);

    Resolution const& resInfo = GetResolution(0);

    // Use integer type to avoid floating-points precision errors during origin multiplication.
    uint32_t scale = 1 << id.resolution;
    HGF2DStretch stretch(HGF2DDisplacement((id.x * resInfo.GetTileSizeX()) * scale, (id.y * resInfo.GetTileSizeY()) * scale), scale, scale);

    HFCPtr<HRABitmap> pTileBitmap;
    pTileBitmap = HRABitmap::Create(effectiveTileSizeX, effectiveTileSizeY, &stretch, m_rasterFile->GetPhysicalCoordSys(), pPixelType);

    ByteStream dataStream((uint32_t)(effectiveTileSizeY*pTileBitmap->ComputeBytesPerWidth()));

    HFCPtr<HCDPacket> pPacket(new HCDPacket(dataStream.GetDataP(), dataStream.GetSize(), dataStream.GetSize()));
    pTileBitmap->SetPacket(pPacket);

    HRACopyFromOptions opts;
    opts.SetResamplingMode(HGSResampling::BILINEAR);
    if (ImagePPStatus::IMAGEPP_STATUS_Success != pTileBitmap->CopyFrom(*m_rasterFile->GetRasterP(), opts))
        return Render::Image();

    __ippLock.unlock(); // done with imagepp...

#if defined (NEEDS_WORK_READ_IMAGE)
    alphaBlend = (Render::Image::Format::Rgba == imageFormat || Render::Image::Format::Bgra == imageFormat);
#endif
    alphaBlend = (Render::Image::Format::Rgba == imageFormat);
    return Render::Image(effectiveTileSizeX, effectiveTileSizeY, std::move(dataStream), imageFormat);
    }

      
//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
folly::Future<BentleyStatus> RasterFileSource::_RequestTile(TileTree::TileCR tile, TileTree::TileLoadsPtr loads)
    {
    DgnDb::VerifyClientThread();

    if (!tile.IsNotLoaded()) // this should only be called when the tile is in the "not loaded" state.
        {
        BeAssert(false);
        return ERROR;
        }

    if (loads)
        loads->m_requested.IncrementAtomicPre(std::memory_order_relaxed);

    tile.SetIsQueued(); // mark as queued so we don't request it again.

    TileQuery query(const_cast<RasterFileTileR>(static_cast<RasterFileTileCR>(tile)), loads);
    return folly::via(&BeFolly::IOThreadPool::GetPool(), [=] () { return query.DoRead(); });
    }

//----------------------------------------------------------------------------------------
//-------------------------------  RasterFileTile  ---------------------------------------
//----------------------------------------------------------------------------------------

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.Marchand                9/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RasterFileTile::RasterFileTile(RasterFileSourceR root, TileId id, RasterFileTileCP parent)
    : RasterTile(root, id, parent)
    {
    uint32_t xMinInRes = id.x * GetRoot().GetResolution(id.resolution).GetTileSizeX();
    uint32_t yMinInRes = id.y * GetRoot().GetResolution(id.resolution).GetTileSizeY();
    uint32_t xMaxInRes = xMinInRes + GetRoot().GetResolution(id.resolution).GetTileSizeX();
    uint32_t yMaxInRes = yMinInRes + GetRoot().GetResolution(id.resolution).GetTileSizeY();

    uint32_t xMin = xMinInRes << id.resolution;
    uint32_t yMin = yMinInRes << id.resolution;
    uint32_t xMax = xMaxInRes << id.resolution;
    uint32_t yMax = yMaxInRes << id.resolution;

    // Limit the tile extent to the raster physical size 
    if (xMax > GetRoot().GetWidth())
        xMax = GetRoot().GetWidth();
    if (yMax > GetRoot().GetHeight())
        yMax = GetRoot().GetHeight();

    DPoint3d physicalCorners[4];
    physicalCorners[0].x = physicalCorners[2].x = xMin;
    physicalCorners[1].x = physicalCorners[3].x = xMax;
    physicalCorners[0].y = physicalCorners[1].y = yMin;
    physicalCorners[2].y = physicalCorners[3].y = yMax;
    physicalCorners[0].z = physicalCorners[1].z = physicalCorners[2].z = physicalCorners[3].z = 0;
    root.GetPhysicalToWorld().MultiplyAndRenormalize(m_corners.m_pts, physicalCorners, 4);

    m_range.InitFrom(m_corners.m_pts, 4);

    if (parent)
        parent->ExtendRange(m_range);

    // That max size is the radius and not the diameter of the bounding sphere in pixels, this is why there is a /2.
    uint32_t tileSizeX = GetRoot().GetTileSizeX(GetTileId());
    uint32_t tileSizeY = GetRoot().GetTileSizeY(GetTileId());
    m_maxSize = sqrt(tileSizeX*tileSizeX + tileSizeY*tileSizeY) / 2;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
TileTree::Tile::ChildTiles const* RasterFileTile::_GetChildren(bool load) const
    {
    if (!_HasChildren()) // is this is the highest resolution tile?
        return nullptr;

    if (load && m_children.empty())
        {
        // this Tile has children, but we haven't created them yet. Do so now
        RasterRoot::Resolution const& childrenResolution = m_root.GetResolution(m_id.resolution - 1);

        // Upper-Left child, we always have one 
        TileId childUpperLeft(m_id.resolution - 1, m_id.x << 1, m_id.y << 1);
        m_children.push_back(new RasterFileTile((root_type&)m_root, childUpperLeft, this));

        // Upper-Right
        if (childUpperLeft.x + 1 < childrenResolution.GetTileCountX())
            {
            TileId childUpperRight(childUpperLeft.resolution, childUpperLeft.x + 1, childUpperLeft.y);
            m_children.push_back(new RasterFileTile((root_type&)m_root, childUpperRight, this));
            }

        // Lower-left
        if (childUpperLeft.y + 1 < childrenResolution.GetTileCountY())
            {
            TileId childLowerLeft(childUpperLeft.resolution, childUpperLeft.x, childUpperLeft.y + 1);
            m_children.push_back(new RasterFileTile((root_type&)m_root, childLowerLeft, this));
            }

        // Lower-Right
        if (childUpperLeft.x + 1 < childrenResolution.GetTileCountX() &&
            childUpperLeft.y + 1 < childrenResolution.GetTileCountY())
            {
            TileId childLowerRight(childUpperLeft.resolution, childUpperLeft.x + 1, childUpperLeft.y + 1);
            m_children.push_back(new RasterFileTile((root_type&)m_root, childLowerRight, this));
            }
        }

    return &m_children;
    }
