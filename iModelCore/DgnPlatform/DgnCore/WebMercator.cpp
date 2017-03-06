/*-------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/WebMercator.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnGeoCoord.h>

using namespace WebMercator;
USING_NAMESPACE_TILETREE

BEGIN_UNNAMED_NAMESPACE

enum
{
    MAX_DB_CACHE_SIZE = 100*1024*1024, // 100 Mb
    WEB_MERCATOR_EPSG = 3857, // see: http://wiki.openstreetmap.org/wiki/EPSG:3857
};

/*---------------------------------------------------------------------------------**//**
* Conversions used in the Web Mercator projection and tiling system.
* See https://developers.google.com/maps/documentation/javascript/examples/map-coordinates for the formulas used here.
* "World" means pixel coordinates in the 256x256 tile that covers the entire map (i.e., tile #0).
* "Pixel" means "pixelCoordinate = worldCoordinate * 2^zoomLevel"
* See https://developers.google.com/maps/documentation/javascript/maptypes#WorldCoordinates
*
* Also see http://wiki.openstreetmap.org/wiki/Zoom_levels
*
* "The latitude and longitude are assumed to be on the WGS 84 datum."
* source: http://msdn.microsoft.com/en-us/library/bb259689.aspx
*
* "The longitude is assumed to range from -180 to +180 degrees,
* and the latitude must be clipped to range from -85.05112878 to 85.05112878. This avoids a
* singularity at the poles, and it causes the projected map to be square."
* source: http://msdn.microsoft.com/en-us/library/bb259689.aspx
* This also goes for Google maps and OpenStreetMaps.
*
* Useful to read:
* http://www.maptiler.org/google-maps-coordinates-tile-bounds-projection/
* http://www.codeproject.com/Articles/463434/GoogleMapsNet-GoogleMaps-Control-for-NET
*
+---------------+---------------+---------------+---------------+---------------+------*/
static double columnToLongitude(double column, double nTiles) {return column / nTiles * 360.0 - 180.;}
static double angleToLatitude(double mercatorAngle) {return msGeomConst_piOver2 - (2.0 * atan(exp(-mercatorAngle)));};
static double rowToLatitude(uint32_t row, double nTiles)
    {
    double n = msGeomConst_pi - msGeomConst_2pi * row / nTiles;
    return 180.0 / msGeomConst_pi * atan(0.5 * (exp(n) - exp(-n)));
    }

DEFINE_POINTER_SUFFIX_TYPEDEFS(LatLongPoint)

//=======================================================================================
// A point (in meters) on the web Mercator projection of the earth.
// @bsiclass                                                    Keith.Bentley   08/16
//=======================================================================================
struct WebMercatorPoint : DPoint2d
{
    WebMercatorPoint() {}
    explicit WebMercatorPoint(GeoPoint);
    static double EarthRadius() {return 6378137.0;}
    static double LatitudeToAngle(double latitude)
        {
        double sinLatitude = sin(latitude);
        return 0.5 * log((1.0 + sinLatitude) / (1.0 - sinLatitude));
        };
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   03/16
//=======================================================================================
struct LatLongPoint : GeoPoint
{
    LatLongPoint() {}
    explicit LatLongPoint(WebMercatorPoint mercator)
        {
        longitude = Angle::RadiansToDegrees(mercator.x  / WebMercatorPoint::EarthRadius());
        latitude  = Angle::RadiansToDegrees(angleToLatitude(mercator.y / WebMercatorPoint::EarthRadius()));
        elevation = 0.0;
        };
};

/*---------------------------------------------------------------------------------**//**
* convert a LatLong coordinate to web Mercator meters
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
WebMercatorPoint::WebMercatorPoint(GeoPoint latLong)
    {
    x = Angle::DegreesToRadians(latLong.longitude) * EarthRadius();
    y = LatitudeToAngle(Angle::DegreesToRadians(latLong.latitude)) * EarthRadius();
    }

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void MapTile::_DrawGraphics(DrawArgsR args) const
    {
    if (m_reprojected)  // if we were unable to re-project this tile, don't draw it.
        T_Super::_DrawGraphics(args);
    }

/*---------------------------------------------------------------------------------**//**
* This map tile just became available from some source (file, cache, http). Load its data and create
* a Render::Graphic to draw it. Only when finished, set the "ready" flag.
* @note this method can be called on many threads, simultaneously.
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MapTile::Loader::_LoadTile()
    {
    MapTileR tile = static_cast<MapTileR>(*m_tile);
    MapRootR mapRoot = tile.GetMapRoot();

    auto graphic = mapRoot.GetRenderSystem()->_CreateGraphic(Graphic::CreateParams(mapRoot.GetDgnDb()));

    ImageSource source(mapRoot.m_format, std::move(m_tileBytes));
    Texture::CreateParams textureParams;
    textureParams.SetIsTileSection();
    auto texture = mapRoot.GetRenderSystem()->_CreateTexture(source, Image::BottomUp::No, textureParams);
    m_tileBytes = std::move(source.GetByteStreamR()); // move the data back into this object. This is necessary since we need to keep to save it in the tile cache.

    graphic->SetSymbology(mapRoot.m_tileColor, mapRoot.m_tileColor, 0); // this is to set transparency
    graphic->AddTile(*texture, tile.m_corners); // add the texture to the graphic, mapping to corners of tile (in BIM world coordinates)

    tile.m_graphic = graphic->Finish();
    BeAssert(tile.m_graphic.IsValid());

    tile.SetIsReady(); // OK, we're all done loading and the other thread may now use this data. Set the "ready" flag.
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Attempt to reproject the corners of this tile through the non-linear GCS of the BIM file. For LatLong points far
* away from the center of the BIM, this reprojection may fail. In that case, mark this tile as "not reprojected". Its
* coordinates will be calculated through the approximate linear transform for purposes of volume testing, but we will
* never display this tile.
* @bsimethod                                    Keith.Bentley                   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt MapTile::ReprojectCorners(GeoPoint* llPts)
    {
    if (m_id.m_level < 1) // level 0 tile never re-projects properly
        return ERROR;

    GraphicBuilder::TileCorners corners;
    auto& units= m_root.GetDgnDb().GeoLocation();
    for (int i=0; i<4; ++i)
        {
        if (SUCCESS != units.XyzFromLatLong(corners.m_pts[i], llPts[i]))
            return ERROR; // only use re-projection if all 4 corners re-project properly
        }

    // All points were successfully reprojected. Save reprojected corners and mark tile as displayble.
    m_reprojected = true;
    m_corners = corners;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Construct a new MapTile by TileId. First convert tileid -> LatLong, and then LatLong -> BIM world.
* If the projection fails, the tile will not be displayable (but we still need an approximate range for
* frustum testing).
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
MapTile::MapTile(MapRootR root, QuadTree::TileId id, MapTileCP parent) : QuadTree::Tile(root, id, parent)
    {
    // First, convert from tile coordinates to LatLong.
    double nTiles = (1 << id.m_level);
    double east  = columnToLongitude(id.m_column, nTiles);
    double west  = columnToLongitude(id.m_column+1, nTiles);
    double north = rowToLatitude(id.m_row, nTiles);
    double south = rowToLatitude(id.m_row+1, nTiles);

    LatLongPoint llPts[4];             //    ----x----->
    llPts[0].Init(east, north, 0.0);   //  | [0]     [1]
    llPts[1].Init(west, north, 0.0);   //  y
    llPts[2].Init(east, south, 0.0);   //  | [2]     [3]
    llPts[3].Init(west, south, 0.0);   //  v

    // attempt to reproject using BIM's GCS
    if (SUCCESS != ReprojectCorners(llPts))
        {
        // reprojection failed, use linear transform
        for (int i=0; i<4; ++i)
            m_corners.m_pts[i] = root.ToWorldPoint(llPts[i]);
        }

    m_range.InitFrom(m_corners.m_pts, 4);

    if (parent)
        parent->ExtendRange(m_range);
    }

/*---------------------------------------------------------------------------------**//**
* Convert a LatLongPoint to a "BIM world" point by transforming from LL -> WebMercator -> world. This is only useful
* for points far away from the GCS of the BIM file. Otherwise we use GCS projection routines.
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d MapRoot::ToWorldPoint(GeoPoint geoPt)
    {
    WebMercatorPoint mercator(geoPt);
    DPoint3d pt = {mercator.x, mercator.y, 0.0};
    m_mercatorToWorld.Multiply(pt);
    return pt;
    }

/*---------------------------------------------------------------------------------**//**
* Combine the three parts of the full tile URL: rootUrl + tileName + urlSuffix.
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String MapRoot::_ConstructTileName(TileCR tile) const
    {
    return m_rootUrl + tile._GetTileName() + m_urlSuffix;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
MapRoot::MapRoot(DgnDbR db, TransformCR trans, Utf8CP realityCacheName, Utf8StringCR rootUrl, Utf8StringCR urlSuffix, Dgn::Render::SystemP system, Render::ImageSource::Format format, double transparency,
        uint8_t maxZoom, uint32_t maxSize) : QuadTree::Root(db, trans, rootUrl.c_str(), system, maxZoom, maxSize, transparency), m_format(format), m_urlSuffix(urlSuffix)
    {
    AxisAlignedBox3d extents = db.GeoLocation().GetProjectExtents();
    DPoint3d center = extents.GetCenter();
    center.z = 0.0;

    // We need a linear transform for the topmost tiles that are out of range for the BIM's reprojection system. To do that, create a GCS for
    // the web Mercator projection system used by map servers, and then get the transform at the center of the BIM's project extents.
    WString warningMsg;
    StatusInt status, warning;
    DgnGCSPtr wgs84 = DgnGCS::CreateGCS(db);
    status = wgs84->InitFromEPSGCode(&warning, &warningMsg, WEB_MERCATOR_EPSG);
    if (SUCCESS != status)
        {
        BeAssert(false);
        return;
        }

    Transform worldToMercator;
    status = db.GeoLocation().GetDgnGCS()->GetLocalTransform(&worldToMercator, center, &extents.low, true/*doRotate*/, true/*doScale*/, *wgs84);

    bool inv = m_mercatorToWorld.InverseOf(worldToMercator);
    if (!inv)
        {
        BeAssert(false);
        return;
        }

    CreateCache(realityCacheName, MAX_DB_CACHE_SIZE);
    m_rootTile = new MapTile(*this, QuadTree::TileId(0,0,0), nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::RootPtr WebMercatorModel::_CreateTileTree(RenderContextR context, ViewControllerCR view)
    {
    Load(&context.GetTargetR().GetSystem());
    return m_root;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String StreetMapModel::_GetRootUrl() const
    {
    return m_properties.m_mapType == Properties::MapType::Map ? GetMapboxStreetsUrl() : GetMapboxSatelliteUrl();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String StreetMapModel::_GetUrlSuffix() const
    {
    /*
    @2x.png     2x scale(retina)
    png32       32 color indexed PNG
    png64       64 color indexed PNG
    png128      128 color indexed PNG
    png256      256 color indexed PNG
    jpg70       70 % quality JPG
    jpg80       80 % quality JPG
    jpg90       90 % quality JPG
    */

    return ".jpg80" "?access_token=" "pk%2EeyJ1IjoibWFwYm94YmVudGxleSIsImEiOiJjaWZvN2xpcW00ZWN2czZrcXdreGg2eTJ0In0%2Ef7c9GAxz6j10kZvL%5F2DBHg";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP StreetMapModel::_GetCopyrightMessage() const
    {
    return "(c) Mapbox, (c) OpenStreetMap contributors";
    }

DEFINE_REF_COUNTED_PTR(WebMercatorModel)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
WebMercatorModel::CreateParams::CreateParams(DgnDbR dgndb, DgnElementId modeledElementId, Properties const& props) : 
    T_Super::CreateParams(dgndb, DgnClassId(dgndb.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_StreetMapModel)), modeledElementId),
    m_properties(props)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId StreetMapHandler::CreateStreetMapModel(StreetMapModel::CreateParams const& params)
    {
    WebMercatorModelPtr model = new StreetMapModel(params);
    model->Insert();
    return model->GetModelId();
    }

namespace WebMercatorStrings
{
static constexpr Utf8CP str_WebMercatorModel() {return "WebMercatorModel";}
static constexpr Utf8CP str_MapService() {return "service";}
static constexpr Utf8CP str_MapType() {return "map_type";}
static constexpr Utf8CP str_GroundBias() {return "groundBias";}
static constexpr Utf8CP str_Transparency() {return "transparency";}
};

using namespace WebMercatorStrings;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::Properties::ToJson(Json::Value& value) const
    {
    value[Json::StaticString(str_MapService())] = (uint32_t) m_mapService;
    value[Json::StaticString(str_MapType())] = (uint32_t) m_mapType;
    value[Json::StaticString(str_GroundBias())] = m_groundBias;
    value[Json::StaticString(str_Transparency())] = m_transparency;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::Properties::FromJson(Json::Value const& value)
    {
    if (value.isMember(str_MapService()))
        m_mapService = (MapService) value[str_MapService()].asInt();

    if (value.isMember(str_MapType()))
        m_mapType = (MapType) value[str_MapType()].asInt();

    m_groundBias = value.isMember(str_GroundBias()) ? value[str_GroundBias()].asDouble() : -1.0;
    m_transparency = value.isMember(str_Transparency()) ? value[str_Transparency()].asDouble() : 0.0;

    LIMIT_RANGE(0.0, .9, m_transparency);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::_WriteJsonProperties(Json::Value& val) const
    {
    m_properties.ToJson(val[str_WebMercatorModel()]);
    T_Super::_WriteJsonProperties(val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::_ReadJsonProperties(Json::Value const& val)
    {
    BeAssert(val.isMember(str_WebMercatorModel()));
    m_properties.FromJson(val[Json::StaticString(str_WebMercatorModel())]);
    T_Super::_ReadJsonProperties(val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::Load(SystemP renderSys) const
    {
    if (m_root.IsValid() && (nullptr==renderSys || m_root->GetRenderSystem()==renderSys))
        return;

    Transform biasTrans;
    biasTrans.InitFrom(DPoint3d::From(0.0, 0.0, m_properties.m_groundBias));

    uint32_t maxSize = 362; // the maximum pixel size for a tile. Approximately sqrt(256^2 + 256^2).
    m_root = new MapRoot(m_dgndb, biasTrans, GetName().c_str(), _GetRootUrl(), _GetUrlSuffix(), renderSys, ImageSource::Format::Jpeg, m_properties.m_transparency, 19, maxSize);
    }
