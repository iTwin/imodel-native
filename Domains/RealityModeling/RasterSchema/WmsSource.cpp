/*-------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "RasterInternal.h"
#include "WmsSource.h"
#include "GcsUtils.h"
#include <Bentley/md5.h>

#define  CONTENT_TYPE_PNG       "image/png"
#define  CONTENT_TYPE_JPEG      "image/jpeg"


//----------------------------------------------------------------------------------------
//-------------------------------  WmsSource      ----------------------------------------
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
StatusInt WmsSource::ComputeLinearApproximation(TransformR cartesianToWorld)
    {
    DgnDbR db = GetModel().GetDgnDb();

    if (nullptr == GetGcsP() || GetModel().GetDgnDb().GeoLocation().GetDgnGCS() == nullptr)
        {
        cartesianToWorld.InitIdentity(); // assumed coincident
        return SUCCESS;
        }

    DPoint3d pointsPhysical[4];
    pointsPhysical[0].Init(0, 0, 0);
    pointsPhysical[1].Init(GetWidth(), 0, 0);
    pointsPhysical[2].Init(0, GetHeight(), 0);
    pointsPhysical[3].Init(GetWidth(), GetHeight(), 0);

    DPoint3d pointsCartesian[4];
    GetPhysicalToCartesian().Multiply(pointsCartesian, pointsPhysical, 4);

    double seed[] = {0.25, 0.5, 0.75};
    size_t seedCount = sizeof(seed) / sizeof(seed[0]);
    std::vector<DPoint3d> tiePoints;

    for (size_t y = 0; y < seedCount; ++y)
        {
        for (size_t x = 0; x < seedCount; ++x)
            {
            DPoint3d pointCartesian = DPoint3d::FromInterpolateBilinear(pointsCartesian[0], pointsCartesian[1], pointsCartesian[2], pointsCartesian[3], seed[x], seed[y]);

            DPoint3d pointWorld;
            if (SUCCESS == GcsUtils::Reproject(pointWorld, *db.GeoLocation().GetDgnGCS(), pointCartesian, *GetGcsP()))
                {
                tiePoints.push_back(pointCartesian);    // uncorrected
                tiePoints.push_back(pointWorld);        // corrected
                }
            }
        }
            
    DMatrix4d result;
    if (0 != ImagePP::HGF2DDCTransfoModel::GetAffineTransfoMatrixFromScaleAndTiePts(result.coff, (uint16_t) tiePoints.size() * 3, (double const*) tiePoints.data()))
        {
        cartesianToWorld.InitIdentity();
        return ERROR;
        }

    cartesianToWorld.InitFrom(result);

    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
/*static*/ GeoCoordinates::BaseGCSPtr WmsSource::CreateBaseGcsFromWmsGcs(Utf8StringCR gcsStr)
    {
    // WMS GCS ex:
    //  - EPSG:26986
    //  - CRS:83, CRS:27, CRS:84, or some Bentley WMS server geocoord keyname.
   
    //1) Attempt to build from EPSG code.   
    if (0 == BeStringUtilities::Strnicmp(gcsStr.c_str(), "EPSG:", 5))
        {
        Utf8String epsgCodeStr = gcsStr.substr(5);
        int epsgCode = atoi(epsgCodeStr.c_str());

        GeoCoordinates::BaseGCSPtr pGcs = GeoCoordinates::BaseGCS::CreateGCS();
        if (SUCCESS == pGcs->InitFromEPSGCode(NULL, NULL, epsgCode))
            return pGcs;       
        }

    //2) Attempt from keyname
    WString keyName(gcsStr.c_str(), true/*utf8*/);
    GeoCoordinates::BaseGCSPtr pGcs = GeoCoordinates::BaseGCS::CreateGCS(keyName.c_str());

    return pGcs->IsValid() ? pGcs : NULL;    
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  7/2015
//----------------------------------------------------------------------------------------
/*static*/ bool WmsSource::EvaluateReverseAxis(WmsMap const& mapInfo, GeoCoordinates::BaseGCSP pGcs)
    {
    switch (mapInfo.m_axisOrder)
        {
        case WmsMap::AxisOrder::Normal:
            return false;

        case WmsMap::AxisOrder::Reverse:
            return true;

        case WmsMap::AxisOrder::Default:
        default:
            // Evaluate below...
            break;
        }
    
    // Only CRS and version 1.3.0 as this non sense reverse axis.
    if (!(mapInfo.m_version.Equals("1.3.0") && mapInfo.m_csType.EqualsI("CRS")))
        return false;
       
    // Our coordinates and what is required by geocoord is:
    //  x = longitude(geographic) or easting(projected) 
    //  y = latitude(geographic) or northing(projected)
    // WMS 1.1.0 and 1.1.1. Same as geocoord x = longitude and y = latitude
    // Ordering for 1.3.0 is CRS dependant.

    // Map server has this strategy:
    //http://mapserver.org/development/rfc/ms-rfc-30.html
    // "EPSG codes: when advertising (such as BoundingBox for a layer element) or using a CRS element in a request such as GetMap/GetFeatureInfo, 
    //  elements using epsg code >=4000 and <5000 will be assumed to have a reverse axes."
    // >> According to AlainRobert, this wrong these days many CS have been created in the 4000-5000 range that do not need to be inverted and more are 
    // created outside that range that do not need to be inverted. Since geocoord cannot provide this information the best approach for now is 
    // to invert all geographic(lat/long) CS.
        
    if (mapInfo.m_csLabel.EqualsI("CRS:1")  ||     // pixels 
       mapInfo.m_csLabel.EqualsI("CRS:83") ||     // (long, lat)
       mapInfo.m_csLabel.EqualsI("CRS:84") ||     // (long, lat) 
       mapInfo.m_csLabel.EqualsI("CRS:27"))       // (long, lat) WMS spec are not clear about CRS:27, there is comment where x is latitude and y longitude but 
                                                    // everyplace else it says (long,lat). Found only one server with CRS:27 and it was (long,lat).
        {
        return false;
        }

    if (0 == BeStringUtilities::Strnicmp(mapInfo.m_csLabel.c_str(), "EPSG:", sizeof("EPSG:")-1/*skip '/n'*/))
        {
        // All Geographic EPSG are assumed: x = latitude, y = longitude
        if (NULL != pGcs && GeoCoordinates::BaseGCS::pcvUnity/*isGeographic*/ == pGcs->GetProjectionCode()) 
            {
            return true;
            }
        else // projected CS are assumed easting, northing.  Maybe some day we will have an axis order service from Gecoord.
            {
            return false;
            }
        }
    
    return false;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
WmsSourcePtr WmsSource::Create(WmsMap const& mapInfo, WmsModel& model, Dgn::Render::SystemP system)
    {
    return new WmsSource(mapInfo, model, system);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
WmsSource::WmsSource(WmsMap const& mapInfo, WmsModel& model, Dgn::Render::SystemP system)
 :RasterRoot(model, mapInfo.m_url.c_str(), system),
  m_mapInfo(mapInfo),
  m_reverseAxis(false),
  m_lastHttpError(Http::HttpStatus::None)
    {
    // for WMS we define a 256x256 multi-resolution image.
    GenerateResolution(m_resolution, m_mapInfo.m_metaWidth, m_mapInfo.m_metaHeight, 256, 256);

    m_gcs = CreateBaseGcsFromWmsGcs(m_mapInfo.m_csLabel);
    BeAssert(m_gcs.IsValid()); //Is it an error if we do not have a GCS? We will assume coincident.

    DPoint2d translation = m_mapInfo.m_boundingBox.low;
    DPoint2d scale = DPoint2d::From((m_mapInfo.m_boundingBox.high.x - m_mapInfo.m_boundingBox.low.x) / m_mapInfo.m_metaWidth,  
                                    (m_mapInfo.m_boundingBox.high.y - m_mapInfo.m_boundingBox.low.y) / m_mapInfo.m_metaHeight);                                         

    Transform mapTransfo = Transform::FromRowValues(scale.x, 0.0, 0.0, translation.x,
                                                    0.0, scale.y, 0.0, translation.y,
                                                    0.0, 0.0, 1.0, 0.0);


    // Data from server is upper-left(jpeg or png) and cartesians coordinate must have a lower-left origin, add a flip.
    Transform physicalToLowerLeft = Transform::FromRowValues(1.0, 0.0, 0.0, 0.0,
                                                             0.0, -1.0, 0.0, m_mapInfo.m_metaHeight,
                                                             0.0, 0.0, 1.0, 0.0);

    m_physicalToCartesian.InitProduct(mapTransfo, physicalToLowerLeft);

    m_reverseAxis = EvaluateReverseAxis(m_mapInfo, m_gcs.get());

    if (SUCCESS != ComputeLinearApproximation(m_cartesianToWorldApproximation))
        {
        BeAssert(!"Unable to compute reprojection approximation");
        m_cartesianToWorldApproximation.InitIdentity();
        }

    m_rootTile = new WmsTile(*this, TileId(GetResolutionCount() - 1, 0, 0), nullptr);

    // Cache name is a mix of model name and root tile url hash. If settings(ex: layers) change so is the tile url which will generate a new cache name.
    Utf8String rootTileUrl = _ConstructTileResource(*m_rootTile);
    MD5 hash;
    hash.Add(rootTileUrl.c_str(), rootTileUrl.length());
    Utf8String cacheName(model.GetName() + "_" + hash.GetHashString());

    CreateCache(cacheName.c_str(), 100 * 1024 * 1024); // 100 Mb
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
Utf8String WmsSource::_ConstructTileResource(Dgn::TileTree::TileCR tile) const
    {
    WmsTileCR wmsTile = static_cast<WmsTile const&>(tile);

    TileId tileId = wmsTile.GetTileId();

    // Get tile corners in this order, with a lower-left origin.
    // [0] [1]
    // [2] [3]
    DPoint3d tileCorners[4];
    wmsTile.GetCartesianCorners(tileCorners);

    double minX = tileCorners[2].x;
    double minY = tileCorners[2].y;
    double maxX = tileCorners[1].x;
    double maxY = tileCorners[1].y;

    if (m_reverseAxis)
        {
        std::swap(minX, minY);
        std::swap(maxX, maxY);
        }
    
    // Mandatory parameters
    Utf8String tileUrl;
    tileUrl.Sprintf("%s?VERSION=%s&REQUEST=GetMap&LAYERS=%s&STYLES=%s&%s=%s&BBOX=%f,%f,%f,%f&WIDTH=%d&HEIGHT=%d&FORMAT=%s", 
        m_mapInfo.m_url.c_str(), m_mapInfo.m_version.c_str(), m_mapInfo.m_layers.c_str(), m_mapInfo.m_styles.c_str(), 
        m_mapInfo.m_csType.c_str(), m_mapInfo.m_csLabel.c_str(), 
        minX, minY, maxX, maxY,
        GetTileSizeX(tileId), GetTileSizeY(tileId), m_mapInfo.m_format.c_str());
       
    // Optional parameters
    if (m_mapInfo.m_transparent)
        tileUrl.append("&TRANSPARENT=TRUE");
    else
        tileUrl.append("&TRANSPARENT=FALSE");

    if (!m_mapInfo.m_vendorSpecific.empty())
        {
        tileUrl.append("&");
        tileUrl.append(m_mapInfo.m_vendorSpecific);
        }
    
    return tileUrl;
    }

//----------------------------------------------------------------------------------------
// Attempt to reproject the cartesian corners of this tile through the non - linear GCS of the BIM file.
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
StatusInt WmsSource::ReprojectCorners(DPoint3dP destWorld, DPoint3dCP srcCartesian) const
    {
    GeoCoordinates::BaseGCSP pSrcGcs = GetGcsP();

    DgnGCSP pDgnGcs = m_model.GetDgnDb().GeoLocation().GetDgnGCS();

    if (NULL == pSrcGcs || NULL == pDgnGcs || !pSrcGcs->IsValid() || !pDgnGcs->IsValid())
        {
        // Assume raster to be coincident.
        memcpy(destWorld, srcCartesian, sizeof(DPoint3d) * 4);
        return SUCCESS;
        }

    for (uint32_t i = 0; i < 4; ++i)
        {
        if (SUCCESS != GcsUtils::Reproject(destWorld[i], *pDgnGcs, srcCartesian[i], *pSrcGcs))
            return ERROR;
        }

    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
//-------------------------------------  WmsTile  ----------------------------------------
//----------------------------------------------------------------------------------------

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.Marchand                9/2016
+---------------+---------------+---------------+---------------+---------------+------*/
WmsTile::WmsTile(WmsSourceR root, TileId id, WmsTileCP parent)
    : RasterTile(root, id, parent)
    {
    DPoint3d cartesianCorners[4];
    GetCartesianCorners(cartesianCorners);
    
    if (SUCCESS != root.ReprojectCorners(m_corners.m_pts, cartesianCorners))
        {
        // Reprojection failed! In that case, mark this tile as "not reprojected" and calculate its coordinates 
        // through a approximate linear transform for purposes of volume testing, but we will never display this tile.
        m_reprojected = false;
        root.GetCartesianToWorldApproximation().Multiply(m_corners.m_pts, cartesianCorners, 4);
        }

    m_range.InitFrom(m_corners.m_pts, 4);

    if (parent)
        parent->ExtendRange(m_range);

    // That max size is the radius and not the diagonal of the bounding sphere in pixels, this is why there is a /2.
    uint32_t tileSizeX = GetRoot().GetTileSizeX(GetTileId());
    uint32_t tileSizeY = GetRoot().GetTileSizeY(GetTileId());
    m_maxSize = sqrt(tileSizeX*tileSizeX + tileSizeY*tileSizeY) / 2;
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
void WmsTile::GetCartesianCorners(DPoint3dP pCorners) const
    {
    auto id = GetTileId();
    uint32_t xMinInRes = id.x * GetSource().GetResolution(id.resolution).GetTileSizeX();
    uint32_t yMinInRes = id.y * GetSource().GetResolution(id.resolution).GetTileSizeY();
    uint32_t xMaxInRes = xMinInRes + GetSource().GetResolution(id.resolution).GetTileSizeX();
    uint32_t yMaxInRes = yMinInRes + GetSource().GetResolution(id.resolution).GetTileSizeY();

    uint32_t xMin = xMinInRes << id.resolution;
    uint32_t yMin = yMinInRes << id.resolution;
    uint32_t xMax = xMaxInRes << id.resolution;
    uint32_t yMax = yMaxInRes << id.resolution;

    // Limit the tile extent to the raster physical size 
    if (xMax > GetSource().GetWidth())
        xMax = GetSource().GetWidth();
    if (yMax > GetSource().GetHeight())
        yMax = GetSource().GetHeight();

    BeAssert(xMax >= xMin);  // For a tile of one pixel, xMin == xMax
    BeAssert(yMax >= yMin);

    // Convert pixel to coordinates.
    DPoint3d physicalCorners[4];
    physicalCorners[0].x = physicalCorners[2].x = xMin;
    physicalCorners[1].x = physicalCorners[3].x = xMax;
    physicalCorners[0].y = physicalCorners[1].y = yMin;
    physicalCorners[2].y = physicalCorners[3].y = yMax;
    physicalCorners[0].z = physicalCorners[1].z = physicalCorners[2].z = physicalCorners[3].z = 0;

    GetSource().GetPhysicalToCartesian().Multiply(pCorners, physicalCorners, 4);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
TileTree::Tile::ChildTiles const* WmsTile::_GetChildren(bool load) const
    {
    if (!_HasChildren()) // is this is the highest resolution tile?
        return nullptr;

    if (load && m_children.empty())
        {
        // this Tile has children, but we haven't created them yet. Do so now
        RasterRoot::Resolution const& childrenResolution = ((root_type&)m_root).GetResolution(m_id.resolution - 1);

        // Upper-Left child, we always have one 
        TileId childUpperLeft(m_id.resolution - 1, m_id.x << 1, m_id.y << 1);
        m_children.push_back(new WmsTile((root_type&)m_root, childUpperLeft, this));

        // Upper-Right
        if (childUpperLeft.x + 1 < childrenResolution.GetTileCountX())
            {
            TileId childUpperRight(childUpperLeft.resolution, childUpperLeft.x + 1, childUpperLeft.y);
            m_children.push_back(new WmsTile((root_type&)m_root, childUpperRight, this));
            }

        // Lower-left
        if (childUpperLeft.y + 1 < childrenResolution.GetTileCountY())
            {
            TileId childLowerLeft(childUpperLeft.resolution, childUpperLeft.x, childUpperLeft.y + 1);
            m_children.push_back(new WmsTile((root_type&)m_root, childLowerLeft, this));
            }

        // Lower-Right
        if (childUpperLeft.x + 1 < childrenResolution.GetTileCountX() &&
            childUpperLeft.y + 1 < childrenResolution.GetTileCountY())
            {
            TileId childLowerRight(childUpperLeft.resolution, childUpperLeft.x + 1, childUpperLeft.y + 1);
            m_children.push_back(new WmsTile((root_type&)m_root, childLowerRight, this));
            }
        }

    return &m_children;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
folly::Future<BentleyStatus> WmsTile::WmsTileLoader::_GetFromSource()
    {
    auto query = std::make_shared<TileTree::HttpDataQuery>(m_resourceName, m_loads);

    if (m_credentials.IsValid())
        query->GetRequest().SetCredentials(m_credentials);

    if (m_proxyCredentials.IsValid())
        query->GetRequest().SetCredentials(m_proxyCredentials);

    RefCountedPtr<WmsTileLoader> me(this);

    return query->Perform().then([me, query] (Http::Response const& response)
        {
        if (!response.IsSuccess())
            {
            if (response.GetConnectionStatus() != Http::ConnectionStatus::Canceled &&
                Http::HttpStatus::OK != response.GetHttpStatus() && Http::HttpStatus::None != response.GetHttpStatus())
                me->GetWmsTile().GetSourceR().SetLastHttpError(response.GetHttpStatus());

            return ERROR;
            }

        me->m_contentType = response.GetHeaders().GetContentType();
        if (me->m_contentType.EqualsI("application/vnd.ogc.se_xml"))
            {
            // Report WMS error in the tile, root or model??
            return ERROR;
            }

        me->m_tileBytes = std::move(query->GetData());
        me->m_saveToCache = TileTree::HttpDataQuery::GetCacheControlExpirationDate(me->m_expirationDate, 0, response);

        return SUCCESS;
        });
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
BentleyStatus WmsTile::WmsTileLoader::_LoadTile()
    {
    WmsTile& rasterTile = static_cast<WmsTile&>(*m_tile);
    auto& root = rasterTile.GetRoot();

    Render::Image image;
    
    if (m_contentType.EqualsIAscii(CONTENT_TYPE_PNG) ||
        (m_contentType.empty() && rasterTile.GetSource().GetMapInfo().m_format.EqualsIAscii(CONTENT_TYPE_PNG)))
        {
        image = Render::Image::FromPng(m_tileBytes.GetData(), m_tileBytes.GetSize(), Render::Image::Format::Rgb);
        }
    else if (m_contentType.EqualsIAscii(CONTENT_TYPE_JPEG) ||
             (m_contentType.empty() && rasterTile.GetSource().GetMapInfo().m_format.EqualsIAscii(CONTENT_TYPE_JPEG)))
        {
        image = Render::Image::FromJpeg(m_tileBytes.GetData(), m_tileBytes.GetSize(), Render::Image::Format::Rgb);
        }

    if (!image.IsValid())
        return ERROR;

    Render::Texture::CreateParams textureParams;
    textureParams.SetIsTileSection();
    auto texture = GetRenderSystem()->_CreateTexture(image, root.GetDgnDb(), textureParams);

    // ###TODO: is this needed?  auto gfParams = Render::GraphicParams::FromSymbology(ColorDef::White(), ColorDef::White(), 0);
    Dgn::TileTree::TriMeshTree::TriMesh::CreateParams geomParams;
    FPoint3d fpts[4]; // local storage for floating point corners
    geomParams.FromTile(*texture, rasterTile.m_corners, fpts, root.GetDgnDb()); // ###TODO: gfParams?
    root.CreateGeometry(rasterTile.m_meshes, geomParams, GetRenderSystem());

    rasterTile.SetIsReady(); // OK, we're all done loading and the other thread may now use this data. Set the "ready" flag.
    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
TileTree::TileLoaderPtr WmsTile::_CreateTileLoader(TileTree::TileLoadStatePtr loads, Dgn::Render::SystemP renderSys)
    {
    auto status = GetSource().GetLastHttpError();
    if (Http::HttpStatus::Unauthorized == status || Http::HttpStatus::ProxyAuthenticationRequired == status)
        return nullptr; // Need to authenticate before we try again.

    RefCountedPtr<WmsTileLoader> tileLoader = new WmsTileLoader(GetRoot()._ConstructTileResource(*this), *this, loads, renderSys);
    tileLoader->m_credentials = GetSource().GetCredentials();
    tileLoader->m_proxyCredentials = GetSource().GetProxyCredentials();

    return tileLoader;
    }
