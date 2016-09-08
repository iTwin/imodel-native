/*-------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/WebMercator.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

//=======================================================================================
// The ProgressiveTask for drawing WebMercator tiles as they arrive asynchronously.
// @bsiclass                                                    Keith.Bentley   05/16
//=======================================================================================
struct WebMercatorProgressive : ProgressiveTask
{
    MapRootR m_root;
    DrawArgs::MissingNodes m_missing;
    TimePoint m_nextShow;

    Completion _DoProgressive(ProgressiveContext& context, WantShow&) override;
    WebMercatorProgressive(MapRootR root, DrawArgs::MissingNodes& nodes) : m_root(root), m_missing(std::move(nodes)){}
};

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* WebMercator tile names are of the form: "level/column/row"
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String MapTile::_GetTileName() const
    {
    return Utf8PrintfString("%d/%d/%d", m_id.m_zoomLevel, m_id.m_column, m_id.m_row);
    }

/*---------------------------------------------------------------------------------**//**
* tile at maximum zoom level do not have children
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool MapTile::_HasChildren() const
    {
    return m_id.m_zoomLevel < m_mapRoot.m_maxZoom;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::ChildTiles const* MapTile::_GetChildren(bool create) const
    {
    if (!_HasChildren()) // is this is the highest resolution tile?
        return nullptr;

    if (create && m_children.empty())
        {
        // this Tile has children, but we haven't created them yet. Do so now
        uint8_t level = m_id.m_zoomLevel+1;
        uint32_t col = m_id.m_column*2;
        uint32_t row = m_id.m_row*2;
        for (int i=0; i<2; ++i)
            {
            for (int j=0; j<2; ++j)
                m_children.push_back(new MapTile(m_mapRoot, TileId(level, col+i, row+j), this));
            }
        }

    return &m_children;
    }

/*---------------------------------------------------------------------------------**//**
* we do not have any graphics for this tile, try its (lower resolution) parent, recursively.
* @bsimethod                                    Keith.Bentley                   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool MapTile::TryLowerRes(DrawArgsR args, int depth) const
    {
    MapTile* parent = (MapTile*) m_parent;
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

    return parent->TryLowerRes(args, depth-1); // recursion
    }

/*---------------------------------------------------------------------------------**//**
* We do not have any graphics for this tile, try its immediate children. Not recursive.
* @bsimethod                                    Keith.Bentley                   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void MapTile::TryHigherRes(DrawArgsR args) const
    {
    for (auto const& child : m_children)
        {
        MapTile* mapChild = (MapTile*) child.get();

        if (mapChild->HasGraphics())
            {
            //DEBUG_PRINTF("using higher res");
            args.m_substitutes.Add(*mapChild->m_graphic);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void MapTile::_DrawGraphics(DrawArgsR args, int depth) const
    {
    if (!m_reprojected)  // if we were unable to re-project this tile, don't draw it.
        return;

    if (!IsReady())
        {
        if (!IsNotFound())
            args.m_missing.Insert(depth, this);

        if (!TryLowerRes(args, 10))
            TryHigherRes(args);

        return;
        }

    if (m_graphic.IsValid())
        args.m_graphics.Add(*m_graphic);
    }

/*---------------------------------------------------------------------------------**//**
* This map tile just became available from some source (file, cache, http). Load its data and create
* a Render::Graphic to draw it. Only when finished, set the "ready" flag.
* @note this method can be called on many threads, simultaneously.
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MapTile::_LoadTile(StreamBuffer& data, RootR root)
    {
    MapRootR mapRoot = (MapRootR) root;

    auto graphic = root.GetRenderSystem()->_CreateGraphic(Graphic::CreateParams(nullptr));

    ImageSource source(mapRoot.m_format, std::move(data));
    Texture::CreateParams textureParams;
    textureParams.SetIsTileSection();
    auto texture = root.GetRenderSystem()->_CreateTexture(source, Image::Format::Rgb, Image::BottomUp::No, textureParams);
    data = std::move(source.GetByteStreamR()); // move the data back into this object. This is necessary since we need to keep to save it in the tile cache.

    graphic->SetSymbology(mapRoot.m_tileColor, mapRoot.m_tileColor, 0); // this is to set transparency
    graphic->AddTile(*texture, m_corners); // add the texture to the graphic, mapping to corners of tile (in BIM world coordinates)

    auto stat = graphic->Close(); // explicitly close the Graphic. This potentially blocks waiting for QV from other threads
    BeAssert(SUCCESS==stat);
    UNUSED_VARIABLE(stat);

    m_graphic = graphic;

    SetIsReady(); // OK, we're all done loading and the other thread may now use this data. Set the "ready" flag.
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
    if (m_id.m_zoomLevel < 1)   // top zoom level never re-project properly
        return ERROR;

    IGraphicBuilder::TileCorners corners;
    auto& units= m_mapRoot.GetDgnDb().Units();
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
MapTile::MapTile(MapRootR root, TileId id, MapTileCP parent) : Tile(parent), m_mapRoot(root), m_id(id)
    {
    // First, convert from tile coordinates to LatLong.
    double nTiles = (1 << id.m_zoomLevel);
    double east  = columnToLongitude(id.m_column, nTiles);
    double west  = columnToLongitude(id.m_column+1, nTiles);
    double north = rowToLatitude(id.m_row, nTiles);
    double south = rowToLatitude(id.m_row+1, nTiles);

    LatLongPoint llPts[4];             //    ----x----->
    llPts[0].Init(east, north, 0.0);   //  | [0]     [1]
    llPts[1].Init(west, north, 0.0);   //  y
    llPts[2].Init(east, south, 0.0);   //  | [2]     [3]
    llPts[3].Init(west, south, 0.0);   //  v

    // attempt tor reproject using BIM's GCS
    if (SUCCESS != ReprojectCorners(llPts))
        {
        // reprojection failed, use linear transform
        for (int i=0; i<4; ++i)
            m_corners.m_pts[i] = root.ToWorldPoint(llPts[i]);
        }

    m_range.InitFrom(m_corners.m_pts, 4);
    m_range.low.z = -1.0;
    m_range.high.z = 1.0;

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
Utf8String MapRoot::_ConstructTileName(TileCR tile)
    {
    return m_rootUrl + tile._GetTileName() + m_urlSuffix;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
MapRoot::MapRoot(DgnDbR db, TransformCR trans, Utf8CP realityCacheName, Utf8StringCR rootUrl, Utf8StringCR urlSuffix, Dgn::Render::SystemP system, Render::ImageSource::Format format, double transparency,
        uint8_t maxZoom, uint32_t maxSize) : Root(db, trans, realityCacheName, rootUrl.c_str(), system), m_format(format), m_urlSuffix(urlSuffix), m_maxZoom(maxZoom), m_maxPixelSize(maxSize)
    {
    m_tileColor = ColorDef::White();
    if (0.0 != transparency)
        m_tileColor.SetAlpha((Byte) (255.* transparency));

    AxisAlignedBox3d extents = db.Units().GetProjectExtents();
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
    status = db.Units().GetDgnGCS()->GetLocalTransform(&worldToMercator, center, &extents.low, true/*doRotate*/, true/*doScale*/, *wgs84);

    bool inv = m_mercatorToWorld.InverseOf(worldToMercator);
    if (!inv)
        {
        BeAssert(false);
        return;
        }

    CreateCache(MAX_DB_CACHE_SIZE);
    m_rootTile = new MapTile(*this, TileId(0,0,0), nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::_AddTerrainGraphics(TerrainContextR context) const
    {
    Load(&context.GetTargetR().GetSystem());

    if (!m_root.IsValid() || !m_root->GetRootTile().IsValid())
        {
        BeAssert(false);
        return;
        }

    auto now = std::chrono::steady_clock::now();
    DrawArgs args(context, m_root->GetLocation(), now, now-m_root->GetExpirationTime());
    m_root->Draw(args);
    DEBUG_PRINTF("Map draw %d graphics, %d total, %d missing ", args.m_graphics.m_entries.size(), m_root->GetRootTile()->CountTiles(), args.m_missing.size());

    args.DrawGraphics(context);

    // Do we still have missing tiles?
    if (!args.m_missing.empty())
        {
        // yes, request them and schedule a progressive task to draw them as they arrive.
        args.RequestMissingTiles(*m_root);
        context.GetViewport()->ScheduleTerrainProgressiveTask(*new WebMercatorProgressive(*m_root, args.m_missing));
        }
    }

/*---------------------------------------------------------------------------------**//**
* Called periodically (on a timer) on the main thread to check for arrival of missing tiles.
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
ProgressiveTask::Completion WebMercatorProgressive::_DoProgressive(ProgressiveContext& context, WantShow& wantShow)
    {
    auto now = std::chrono::steady_clock::now();
    DrawArgs args(context, m_root.GetLocation(), now, now-m_root.GetExpirationTime());

    DEBUG_PRINTF("Map progressive %d missing", m_missing.size());

    for (auto const& node: m_missing)
        {
        auto stat = node.second->GetLoadState();
        if (stat == Tile::LoadState::Ready)
            node.second->Draw(args, node.first);        // now ready, draw it (this potentially generates new missing nodes)
        else if (stat != Tile::LoadState::NotFound)
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String StreetMapModel::_GetRootUrl() const
    {
    Utf8String url("http://api.mapbox.com/v4/");
    url +=  m_properties.m_mapType == Properties::MapType::Map ? "mapbox.streets/" : "mapbox.satellite/";
    return url;
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String getStreetMapServerDescription(WebMercatorModel::Properties::MapType mapType)
    {
    Utf8String descr("Mapbox");   // *** WIP translate
    if (WebMercatorModel::Properties::MapType::Map == mapType)
        descr.append(" Map");   // *** WIP translate
    else
        descr.append(" Satellite Images"); // *** WIP translate

    return descr;
    }

DEFINE_REF_COUNTED_PTR(WebMercatorModel)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
WebMercatorModel::CreateParams::CreateParams(DgnDbR dgndb, Properties const& props) : T_Super::CreateParams(dgndb,
    DgnClassId(dgndb.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, "StreetMapModel")), 
    DgnElementId() /* WIP: which element? */,
    DgnModel::CreateModelCode(getStreetMapServerDescription(props.m_mapType))),
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

static Utf8CP JSON_WebMercatorModel() {return "WebMercatorModel";}
static Utf8CP PROPERTYJSON_MapService() {return "service";}
static Utf8CP PROPERTYJSON_MapType() {return "map_type";}
static Utf8CP PROPERTYJSON_GroundBias() {return "groundBias";}
static Utf8CP PROPERTYJSON_Transparency() {return "transparency";}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::Properties::ToJson(Json::Value& value) const
    {
    value[PROPERTYJSON_MapService()] = (uint32_t) m_mapService;
    value[PROPERTYJSON_MapType()] = (uint32_t) m_mapType;
    value[PROPERTYJSON_GroundBias()] = m_groundBias;
    value[PROPERTYJSON_Transparency()] = m_transparency;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::Properties::FromJson(Json::Value const& value)
    {
    if (value.isMember(PROPERTYJSON_MapService()))
        m_mapService = (MapService) value[PROPERTYJSON_MapService()].asInt();

    if (value.isMember(PROPERTYJSON_MapType()))
        m_mapType = (MapType) value[PROPERTYJSON_MapType()].asInt();

    m_groundBias = value.isMember(PROPERTYJSON_GroundBias()) ? value[PROPERTYJSON_GroundBias()].asDouble() : -1.0;
    m_transparency = value.isMember(PROPERTYJSON_Transparency()) ? value[PROPERTYJSON_Transparency()].asDouble() : 0.0;

    LIMIT_RANGE(0.0, .9, m_transparency);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::_WriteJsonProperties(Json::Value& val) const
    {
    m_properties.ToJson(val[JSON_WebMercatorModel()]);
    T_Super::_WriteJsonProperties(val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::_ReadJsonProperties(Json::Value const& val)
    {
    BeAssert(val.isMember(JSON_WebMercatorModel()));
    m_properties.FromJson(val[JSON_WebMercatorModel()]);
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
