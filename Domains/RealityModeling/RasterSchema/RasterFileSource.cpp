/*-------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "RasterInternal.h"
#include "RasterFileSource.h"

USING_NAMESPACE_DGN_CESIUM

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
// Since we can only create one tile at a time, we serialize all requests through a single thread.
// This avoids all of the requests blocking in the IoPool.
// @bsiclass                                                    Mathieu.Marchand 12/16
//=======================================================================================
struct RasterFileThread : BeFolly::ThreadPool
    {
    RasterFileThread() : ThreadPool(1, "RasterFile") {}
    static RasterFileThread& Get() { static folly::Singleton<RasterFileThread> s_pool; return *s_pool.try_get_fast(); }
    };

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
folly::Future<BentleyStatus> RasterFileTile::RasterTileLoader::_GetFromSource()
    {
    RefCountedPtr<RasterTileLoader> me(this);
    return folly::via(&RasterFileThread::Get(), [me] ()
        {
        if (me->IsCanceledOrAbandoned())
            return ERROR;

        return me->DoGetFromSource();
        });
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
BentleyStatus RasterFileTile::RasterTileLoader::DoGetFromSource()
    {
    RasterFileTile& rasterTile = static_cast<RasterFileTile&>(*m_tile.get());

    bool enableAlphaBlend = false;
    m_image = GetFileSource().QueryTile(rasterTile.GetTileId(), enableAlphaBlend); //&&MM validate that we are not copying bytes for nothing.
    
    return m_image.IsValid() ? SUCCESS : ERROR;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
BentleyStatus RasterFileTile::RasterTileLoader::_LoadTile()
    {
    if (!m_image.IsValid())
        return ERROR;

    RasterFileTile& rasterTile = static_cast<RasterFileTile&>(*m_tile.get());
    auto& root = rasterTile.GetRoot();

    auto texture = GetOutput().CreateTexture(m_image);

    Cesium::TriMeshTree::TriMesh::CreateParams geomParams;
    FPoint3d fpts[4];
    geomParams.FromTile(*texture, rasterTile.GetCorners(), fpts);
    root.CreateGeometry(rasterTile.m_meshes, geomParams, GetOutput());

    return SUCCESS;
    };

//----------------------------------------------------------------------------------------
//-------------------------------   RasterFileSource   -----------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
RasterFileSourcePtr RasterFileSource::Create(Utf8StringCR resolvedName, RasterFileModel& model)
    {
    // Open raster file
    auto rasterFile = RasterFile::Create(resolvedName);
    if (rasterFile.IsNull())
        return nullptr;  // Can't create model; probably that file name is invalid.
        
    return new RasterFileSource(*rasterFile, model);
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
RasterFileSource::RasterFileSource(RasterFileR rasterFile, RasterFileModel& model)
    :RasterRoot(model, ""/*rootUrl*/), m_rasterFile(&rasterFile), m_model(model)
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
    // >>> We now run all qeury thru a single thread pool 'RasterFileThread' so no more than one request will run at the same time.
//     static std::mutex s_ippMutex;
//     std::unique_lock<std::mutex> __ippLock(s_ippMutex);

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

    //__ippLock.unlock(); // done with imagepp...

#if defined (NEEDS_WORK_READ_IMAGE)
    alphaBlend = (Render::Image::Format::Rgba == imageFormat || Render::Image::Format::Bgra == imageFormat);
#endif
    alphaBlend = (Render::Image::Format::Rgba == imageFormat);
    return Render::Image(effectiveTileSizeX, effectiveTileSizeY, std::move(dataStream), imageFormat);
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
Cesium::ChildTiles const* RasterFileTile::_GetChildren(bool load) const
    {
    if (!HasChildren()) // is this is the highest resolution tile?
        return nullptr;

    if (load && m_children.empty())
        {
        auto& root = (RasterFileSource&)m_root;

        // this Tile has children, but we haven't created them yet. Do so now
        RasterRoot::Resolution const& childrenResolution = root.GetResolution(m_id.resolution - 1);

        // Upper-Left child, we always have one 
        TileId childUpperLeft(m_id.resolution - 1, m_id.x << 1, m_id.y << 1);
        m_children.push_back(new RasterFileTile(root, childUpperLeft, this));

        // Upper-Right
        if (childUpperLeft.x + 1 < childrenResolution.GetTileCountX())
            {
            TileId childUpperRight(childUpperLeft.resolution, childUpperLeft.x + 1, childUpperLeft.y);
            m_children.push_back(new RasterFileTile(root, childUpperRight, this));
            }

        // Lower-left
        if (childUpperLeft.y + 1 < childrenResolution.GetTileCountY())
            {
            TileId childLowerLeft(childUpperLeft.resolution, childUpperLeft.x, childUpperLeft.y + 1);
            m_children.push_back(new RasterFileTile(root, childLowerLeft, this));
            }

        // Lower-Right
        if (childUpperLeft.x + 1 < childrenResolution.GetTileCountX() &&
            childUpperLeft.y + 1 < childrenResolution.GetTileCountY())
            {
            TileId childLowerRight(childUpperLeft.resolution, childUpperLeft.x + 1, childUpperLeft.y + 1);
            m_children.push_back(new RasterFileTile(root, childLowerRight, this));
            }
        }

    return &m_children;
    }
