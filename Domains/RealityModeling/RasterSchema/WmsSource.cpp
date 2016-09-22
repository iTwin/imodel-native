/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/WmsSource.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RasterSchemaInternal.h"
#include "WmsSource.h"

#define  CONTENT_TYPE_PNG       "image/png"
#define  CONTENT_TYPE_JPEG      "image/jpeg"

USING_NAMESPACE_BENTLEY_SQLITE

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  5/2015
//----------------------------------------------------------------------------------------
static ReprojectStatus s_FilterGeocoordWarning(ReprojectStatus status)  //&&MM create shared Gcs services.
    {
    if ((REPROJECT_CSMAPERR_OutOfUsefulRange == status) || (REPROJECT_CSMAPERR_VerticalDatumConversionError == status))   // These are warnings
        return REPROJECT_Success;

    return status;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
static StatusInt reproject(DPoint3dR dest, DgnGCSCR destGcs, DPoint3dCR srcCartesian, GeoCoordinates::BaseGCSCR sourceGcs)
    {
    GeoPoint srcGeo;
    if (REPROJECT_Success != s_FilterGeocoordWarning(sourceGcs.LatLongFromCartesian(srcGeo, srcCartesian)))
        {
        BeAssert(!"A source should always be able to represent itself in its GCS."); // That operation cannot fail or can it?
        return ERROR;
        }

    // Source latlong to BIM latlong.
    GeoPoint bimGeo;
    if (REPROJECT_Success != s_FilterGeocoordWarning(sourceGcs.LatLongFromLatLong(bimGeo, srcGeo, destGcs)))
        return ERROR;

    // Finally to UOR/BIM
    if (REPROJECT_Success != s_FilterGeocoordWarning(destGcs.UorsFromLatLong(dest, bimGeo)))
        return ERROR;

    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
//-------------------------------  WmsSource      ----------------------------------------
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
StatusInt WmsSource::ComputeLinearApproximation(TransformR cartesianToWorld)
    {
    DgnDbR db = GetModel().GetDgnDb();

    if (nullptr == GetGcsP() || GetModel().GetDgnDb().Units().GetDgnGCS() == nullptr)
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
            if (SUCCESS == reproject(pointWorld, *db.Units().GetDgnGCS(), pointCartesian, *GetGcsP()))
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
 :RasterRoot(model, model.GetName().c_str(), mapInfo.m_url.c_str(), system),
  m_mapInfo(mapInfo),
  m_reverseAxis(false)
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

    CreateCache(100 * 1024 * 1024); // 100 Mb
    m_rootTile = new WmsTile(*this, TileId(GetResolutionCount() - 1, 0, 0), nullptr);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
Utf8String WmsSource::_ConstructTileName(Dgn::TileTree::TileCR tile)
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
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
folly::Future<BentleyStatus> WmsSource::_RequestTile(Dgn::TileTree::TileCR tile, Dgn::TileTree::TileLoadsPtr loads)
    {
    DEBUG_PRINTF("RequestTile r=%d (%d,%d) %d", ((WmsTileR) tile).GetTileId().resolution, ((WmsTileR) tile).GetTileId().x, ((WmsTileR) tile).GetTileId().y, (uintptr_t)&tile);
    return RasterRoot::_RequestTile(tile, loads);
    }

//----------------------------------------------------------------------------------------
// Attempt to reproject the cartesian corners of this tile through the non - linear GCS of the BIM file.
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
StatusInt WmsSource::ReprojectCorners(DPoint3dP destWorld, DPoint3dCP srcCartesian) const
    {
    GeoCoordinates::BaseGCSP pSrcGcs = GetGcsP();

    DgnGCSP pDgnGcs = m_model.GetDgnDb().Units().GetDgnGCS();

    if (NULL == pSrcGcs || NULL == pDgnGcs || !pSrcGcs->IsValid() || !pDgnGcs->IsValid())
        {
        // Assume raster to be coincident.
        memcpy(destWorld, srcCartesian, sizeof(DPoint3d) * 4);
        return SUCCESS;
        }

    for (uint32_t i = 0; i < 4; ++i)
        {
        if (SUCCESS != reproject(destWorld[i], *pDgnGcs, srcCartesian[i], *pSrcGcs))
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
    //&&MM review. doesn't work with GCS in lat/long. Why we need that?
    //             also problematic in DrawArgs::DrawGraphics where substitute are offseted.
//     m_range.low.z = -1.0;
//     m_range.high.z = 1.0;

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
        RasterRoot::Resolution const& childrenResolution = m_root.GetResolution(m_id.resolution - 1);

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
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
BentleyStatus WmsTile::_LoadTile(Dgn::TileTree::StreamBuffer& data, Dgn::TileTree::RootR root)
    {
    Render::ImageSource::Format format;
    if (GetSource().GetMapInfo().m_format.EqualsI(CONTENT_TYPE_PNG))
        {
        format = Render::ImageSource::Format::Png;
        }
    else if (GetSource().GetMapInfo().m_format.EqualsI(CONTENT_TYPE_JPEG))
        {
        format = Render::ImageSource::Format::Jpeg;
        }
    else
        {
        SetNotFound();
        return ERROR;
        }

    Render::ImageSource source(format, std::move(data));
    Render::Image image(source, Render::Image::Format::Rgb, Render::Image::BottomUp::No);

    if (!image.IsValid())
        {
        // We might have receive an error message from the server in the form of an XML stream.
        // the html field "Content-Type" have that info but we do not have access to it. >> Content-Type=application/vnd.ogc.se_xml
        SetNotFound();
        ERROR_PRINTF("invalid tile data r=%d (%d,%d)", GetTileId().resolution, GetTileId().x, GetTileId().y);
        return ERROR;
        }

    Render::Texture::CreateParams textureParams;
    textureParams.SetIsTileSection();
    auto texture = root.GetRenderSystem()->_CreateTexture(image, textureParams);

    data = std::move(source.GetByteStreamR()); // move the data back into this object. This is necessary since we need to keep to save it in the tile cache.

    auto graphic = root.GetRenderSystem()->_CreateGraphic(Render::Graphic::CreateParams(nullptr));
    graphic->SetSymbology(ColorDef::White(), ColorDef::White(), 0); // this is to set transparency
    graphic->AddTile(*texture, m_corners); // add the texture to the graphic, mapping to corners of tile (in BIM world coordinates)

    auto stat = graphic->Close(); // explicitly close the Graphic. This potentially blocks waiting for QV from other threads
    BeAssert(SUCCESS == stat);
    UNUSED_VARIABLE(stat);

    m_graphic = graphic;

    SetIsReady(); // OK, we're all done loading and the other thread may now use this data. Set the "ready" flag.
    return SUCCESS;
    }