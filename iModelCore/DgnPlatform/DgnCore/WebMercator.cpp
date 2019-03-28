/*-------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/WebMercator.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
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
    static double RadiusOfEarth() {return 6378137.0;}
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
        longitude = Angle::RadiansToDegrees(mercator.x  / WebMercatorPoint::RadiusOfEarth());
        latitude  = Angle::RadiansToDegrees(angleToLatitude(mercator.y / WebMercatorPoint::RadiusOfEarth()));
        elevation = 0.0;
        };
};

/*---------------------------------------------------------------------------------**//**
* convert a LatLong coordinate to web Mercator meters
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
WebMercatorPoint::WebMercatorPoint(GeoPoint latLong)
    {
    x = Angle::DegreesToRadians(latLong.longitude) * RadiusOfEarth();
    y = LatitudeToAngle(Angle::DegreesToRadians(latLong.latitude)) * RadiusOfEarth();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
static DRange3d computeAuxRange(DrawArgsCR args)
    {
    // calculate a range in lat/long to use for the the first 4 layers of tiles.
    DRange3d latLongRange = DRange3d::NullRange();
    DgnGCSP dgnGCS = args.GetDgnDb().GeoLocation().GetDgnGCS();
    if (nullptr == dgnGCS)
        {
        BeAssert(false);
        return latLongRange;
        }

    Frustum viewBox = args.GetFrustum();
    GeoPoint latLongBox[8];
    for (int iPoint=0; iPoint < 8; ++iPoint)
        {
        GeoPointP latLongP = &latLongBox[iPoint];
        if (SUCCESS != (StatusInt)dgnGCS->LatLongFromUors (*latLongP, viewBox.m_pts[iPoint]))
            {
            BeAssert (false);
            return DRange3d::NullRange();
            }

        if (latLongP->longitude < latLongRange.low.x)
            latLongRange.low.x = latLongP->longitude;
        if (latLongP->longitude > latLongRange.high.x)
            latLongRange.high.x = latLongP->longitude;

        if (latLongP->latitude < latLongRange.low.y)
            latLongRange.low.y = latLongP->latitude;
        if (latLongP->latitude > latLongRange.high.y)
            latLongRange.high.y = latLongP->latitude;

        latLongRange.low.z = latLongRange.high.z = 0;
        }

    return latLongRange;
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

    // don't accept tiles that match the missing tile data.
    if (mapRoot.m_imageryProvider->_MatchesMissingTile (m_tileBytes))
        return ERROR;

    // some tile servers (for example Bing) start returning PNG tiles at a certain zoom level, even if you request Jpeg.
    ImageSource::Format format = mapRoot.m_format;
    if (0 == m_contentType.CompareTo("image/png"))
        format = ImageSource::Format::Png;
    if (0 == m_contentType.CompareTo("image/jpeg"))
        format = ImageSource::Format::Jpeg;

    ImageSource source(format, std::move(m_tileBytes));
    Texture::CreateParams textureParams;
    textureParams.SetIsTileSection();
    auto texture = GetRenderSystem()->_CreateTexture(source, Image::BottomUp::No, mapRoot.GetDgnDb(), textureParams);

    m_tileBytes = std::move(source.GetByteStreamR()); // move the data back into this object. This is necessary since we need to keep to save it in the tile cache.

    GraphicParams gfParams = GraphicParams::FromSymbology(mapRoot.m_tileColor, mapRoot.m_tileColor, 0); // this is to set transparency
    tile.m_graphic = GetRenderSystem()->_CreateTile(*texture, tile.m_corners, mapRoot.GetDgnDb(), gfParams, true);

    BeAssert(tile.m_graphic.IsValid());

    tile.SetIsReady(); // OK, we're all done loading and the other thread may now use this data. Set the "ready" flag.
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Returns the default default duration, which it gets from the imagery provider
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t MapTile::Loader::_GetMaxValidDuration() const
    {
    MapTileR tile = static_cast<MapTileR>(*m_tile);
    MapRootR mapRoot = tile.GetMapRoot();
    return mapRoot.m_imageryProvider->_GetMaxValidDuration();
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
        if (SUCCESS != units.XyzFromLatLongWGS84(corners.m_pts[i], llPts[i]))
            return ERROR; // only use re-projection if all 4 corners re-project properly
        }

    // All points were successfully reprojected. Save reprojected corners and mark tile as displayble.
    m_reprojected = true;
    m_corners = corners;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/18
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::SelectParent MapTile::_SelectTiles(bvector<TileCPtr>& selected, DrawArgsR args) const
    {
    bool isRoot = (0 == GetDepth());
    if (isRoot)
        args.m_auxRange = computeAuxRange(args);

    Tile::SelectParent result = T_Super::_SelectTiles(selected, args);

    // Once all tiles are selected, allow the imageryProvider to see them in case it needs them for its copyright message (Bing does).
    if (isRoot)
        GetMapRoot().m_imageryProvider->_OnSelectTiles (selected, args);

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::Visibility MapTile::_GetVisibility(DrawArgsCR args) const
    {
    // For levels less than or equal to 4, we accept them if their lat long overlaps our view boundary's lat long.
    // That is because the conversion from lat/long -> cartesian coordinates is problematic for lat/longs far out 
    // of the effective range of our coordinate system (and the CS-Map code does not give us an error when it happens)
    static constexpr uint32_t maxZoomLevelForLatLongCheck = 5;
    if (m_id.m_level <= maxZoomLevelForLatLongCheck)
        {
        // get the lat/long for this tile.
        BeAssert(!args.m_auxRange.IsNull());

        double nTiles = (1 << m_id.m_level);
        double tileWest  = columnToLongitude(m_id.m_column, nTiles);
        double tileEast  = columnToLongitude(m_id.m_column+1, nTiles);
        double tileNorth = rowToLatitude(m_id.m_row, nTiles);
        double tileSouth = rowToLatitude(m_id.m_row+1, nTiles);

        // if the tile overlaps the range, we must use it.
        if (tileWest > args.m_auxRange.high.x)
            return Visibility::OutsideFrustum;
        if (tileEast < args.m_auxRange.low.x)
            return Visibility::OutsideFrustum;
        if (tileSouth > args.m_auxRange.high.y)
            return Visibility::OutsideFrustum;
        if (tileNorth < args.m_auxRange.low.y)
            return Visibility::OutsideFrustum;
        }

    return T_Super::_GetVisibility(args);
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
    double west  = columnToLongitude(id.m_column, nTiles);
    double east  = columnToLongitude(id.m_column+1, nTiles);
    double north = rowToLatitude(id.m_row, nTiles);
    double south = rowToLatitude(id.m_row+1, nTiles);

    LatLongPoint llPts[4];             //    ----x----->
    llPts[0].Init(west, north, 0.0);   //  | [0]     [1]
    llPts[1].Init(east, north, 0.0);   //  y
    llPts[2].Init(west, south, 0.0);   //  | [2]     [3]
    llPts[3].Init(east, south, 0.0);   //  v

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
Utf8String MapRoot::_ConstructTileResource(TileCR tile) const
    {
    QuadTree::Tile const* quadTile = dynamic_cast <QuadTree::Tile const*>(&tile);
    BeAssert(nullptr != quadTile);
    return m_imageryProvider->_ConstructUrl(*quadTile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
MapRoot::MapRoot(DgnDbR db, DgnModelId modelId, TransformCR trans, ImageryProviderR imageryProvider, Dgn::Render::SystemP system, Render::ImageSource::Format format, double transparency,
        uint32_t maxSize) : QuadTree::Root(db, modelId, trans, nullptr, system, imageryProvider._GetMaximumZoomLevel(false), maxSize, transparency), m_format(format), m_imageryProvider(&imageryProvider)
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

    CreateCache(imageryProvider._GetCacheFileName().c_str(), MAX_DB_CACHE_SIZE);
    m_rootTile = new MapTile(*this, QuadTree::TileId(0,0,0), nullptr);
    m_rootTile->SetIsReady();

    // get the copyright sprite from the imagery provider.
    m_copyrightSprite = imageryProvider._GetCopyrightSprite();
    }


namespace WebMercatorStrings
{
// top level identifier of WebMercator subfolder.
BE_JSON_NAME(webMercatorModel)

// property names common to all providers
};

using namespace WebMercatorStrings;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::ToJson(Json::Value& value) const
    {
    value[json_providerName()] = m_provider->_GetProviderName();
    value[json_groundBias()]   = m_groundBias;
    value[json_transparency()] = m_transparency;

    m_provider->_ToJson(value[json_providerData()]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::FromJson(Json::Value const& value)
    {
    m_groundBias = value[json_groundBias()].asDouble(-1.0);
    m_transparency = value[json_transparency()].asDouble(0.0);
    LIMIT_RANGE(0.0, .95, m_transparency);

    Utf8String providerName = value[json_providerName()].asString();
    Json::Value const& providerDataValue = value[json_providerData()];

    if (0 == providerName.CompareToI(WebMercator::MapBoxImageryProvider::prop_MapBoxProvider()))
        {
        m_provider = MapBoxImageryProvider::Create(providerDataValue);
        }
    else if (0 == providerName.CompareToI(WebMercator::BingImageryProvider::prop_BingProvider()))
        {
        m_provider = BingImageryProvider::Create(providerDataValue);
        }
    else if (0 == providerName.CompareToI(WebMercator::HereImageryProvider::prop_HereProvider()))
        {
        m_provider = HereImageryProvider::Create(providerDataValue);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::_OnSaveJsonProperties()
    {
    Json::Value value;
    ToJson(value);
    SetJsonProperties(json_webMercatorModel(), value);
    T_Super::_OnSaveJsonProperties();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::_OnLoadedJsonProperties()
    {
    ECN::AdHocJsonValueCR value = GetJsonProperties(json_webMercatorModel());

    FromJson(value);
    T_Super::_OnLoadedJsonProperties();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WebMercatorModel::_GetCopyrightMessage(ViewController& viewController) const
    {
    return m_provider.IsValid() ? m_provider->_GetCopyrightMessage(viewController) : "";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::RgbaSpriteP WebMercatorModel::_GetCopyrightSprite(ViewController& viewController) const
    { 
    // make sure we have a root.
    MapRoot* root = dynamic_cast<MapRoot*> (m_root.get());
    if (nullptr != root)
        return root->m_copyrightSprite.get();
    else
        return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::RootPtr WebMercatorModel::LoadTileTree(SystemP renderSys) const
    {
    if (m_provider.IsNull())
        {
        BeAssert(false);
        return nullptr;
        }

    // Here we would like to create the TileTree root (MapRoot), but we might not be ready to do that for some ImageryProviders.
    // For example, the Bing provider isn't ready to go until it has fetched the template URL.
    ImageryProvider::TemplateUrlLoadStatus templateStatus = m_provider->_GetTemplateUrlLoadStatus();
    if (ImageryProvider::TemplateUrlLoadStatus::Received == templateStatus)
        {
        Transform biasTrans;
        biasTrans.InitFrom(DPoint3d::From(0.0, 0.0, m_groundBias));


        uint32_t maxSize = 362; // the maximum pixel size for a tile. Approximately sqrt(256^2 + 256^2).
        return new MapRoot(this->GetDgnDb(), this->GetModelId(), biasTrans, *m_provider.get(), renderSys, ImageSource::Format::Jpeg, m_transparency, maxSize);
        }

    // Here we do not yet have the TemplateUrl. _FetchTemplateUrl can be called multiple times - it takes care of checking the status, etc.
    m_provider->_FetchTemplateUrl();
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
WebMercatorModel::WebMercatorModel(CreateParams const& params) : T_Super(params)
    {
    // if the jsonParameters aren't filled in, that's because this is creating an existing the model from the DgnDb.
    // We will get the json parameters later when _OnLoadedJsonProperties() is called.
    if (params.m_jsonParameters.isNull())
        return;

    // if not null, this is a new model creation. Get the parameters from those passed in to the constructor of CreateParams.
    FromJson(params.m_jsonParameters);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
MapBoxImageryProvider*  MapBoxImageryProvider::Create (Json::Value const& providerDataValue)
    {
    MapBoxImageryProvider*    imageryProvider = new MapBoxImageryProvider();
    imageryProvider->_FromJson (providerDataValue);

    // if the mapType is None, return null.
    if (MapType::None == imageryProvider->m_mapType)
        return nullptr;

    return imageryProvider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String MapBoxImageryProvider::_ConstructUrl(TileTree::QuadTree::Tile const& tile) const
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

    return Utf8PrintfString("%s%d/%d/%d%s", m_baseUrl.c_str(), tile.GetZoomLevel(), tile.GetColumn(), tile.GetRow(), ".jpg80" "?access_token=" "pk%2EeyJ1IjoibWFwYm94YmVudGxleSIsImEiOiJjaWZvN2xpcW00ZWN2czZrcXdreGg2eTJ0In0%2Ef7c9GAxz6j10kZvL%5F2DBHg");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String MapBoxImageryProvider::_GetCopyrightMessage(ViewController&) const
    {
    return "(c) Mapbox, (c) OpenStreetMap contributors";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String MapBoxImageryProvider::_GetCacheFileName() const
    {
    switch (m_mapType)
        {
        case MapType::Street:
            return "MapBoxStreets";

        case MapType::Aerial:
            return "MapBoxSatellite";

        case MapType::Hybrid:
            return "MapBoxHybrid";
        }
    BeAssert(false);
    return "MapBoxUnknown";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MapBoxImageryProvider::_FromJson(Json::Value const& value)
    {
    // the only thing currently stored in the MapBoxImageryProvider Json is the MapType.
    m_mapType = (MapType) value[WebMercatorModel::json_mapType()].asInt((int)MapType::Street);

    switch (m_mapType)
        {
        case MapType::Street:
            m_baseUrl = "http://api.mapbox.com/v4/mapbox.streets/";
            break;

        case MapType::Aerial:
            m_baseUrl = "http://api.mapbox.com/v4/mapbox.satellite/";
            break;

        case MapType::Hybrid:
            m_baseUrl = "http://api.mapbox.com/v4/mapbox.streets-satellite/";
            break;

        default:
            BeAssert(false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MapBoxImageryProvider::_ToJson(Json::Value& value) const
    {
    // the only thing currently stored in the MapBoxImageryProvider Json is the MapType.
    value[WebMercatorModel::json_mapType()] = (int)m_mapType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String tileXYToQuadKey(int tileX, int tileY, int levelOfDetail)
    {
    // blatantly ripped off from C# example in bing documentation https://msdn.microsoft.com/en-us/library/bb259689.aspx
    Utf8String  quadKey;

    // Root tile is not displayable. Returns 0 for _GetMaximumSize(). Should not end up here.
    BeAssert(0 != levelOfDetail);

    for (int i = levelOfDetail; i > 0; i--)
        {
        char digit = '0';
        int mask = 1 << (i - 1);
        if ((tileX & mask) != 0)
            {
            digit++;
            }
        if ((tileY & mask) != 0)
            {
            digit++;
            digit++;
            }
        quadKey.append(1, digit);
        }
    return quadKey;
    }

BingImageryProviderPtr  BingImageryProvider::s_streetMapProvider;
BingImageryProviderPtr  BingImageryProvider::s_aerialMapProvider;
BingImageryProviderPtr  BingImageryProvider::s_hybridMapProvider;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BingImageryProvider*  BingImageryProvider::Create (Json::Value const& providerDataValue)
    {
    // the only thing currently stored in the BingImageryProvider Json is the MapType.
    MapType mapType = (MapType) providerDataValue[WebMercatorModel::json_mapType()].asInt((int)MapType::Hybrid);

    // if the background tupe is None, return nullptr.
    if (MapType::None == mapType)
        return nullptr;

    // we keep one imagery provider for each MapType of map. That way we don't have to query
    // again for the URL to ask, and Bing's accounting might be based off those queries, so we use
    // as few as possible.

    BingImageryProviderPtr* providerForTypeP = nullptr;
    if (MapType::Street == mapType)
        providerForTypeP = &s_streetMapProvider;
    else if (MapType::Aerial == mapType)
        providerForTypeP = &s_aerialMapProvider;
    else if (MapType::Hybrid == mapType)
        providerForTypeP = &s_hybridMapProvider;

    if (nullptr == providerForTypeP)
        {
        BeAssert (false);
        providerForTypeP = &s_streetMapProvider;
        }

    if ((*providerForTypeP).IsNull())
        {
        (*providerForTypeP) = new BingImageryProvider();
        (*providerForTypeP)->_FromJson (providerDataValue);
        }

    return (*providerForTypeP).get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BingImageryProvider::_ConstructUrl(TileTree::QuadTree::Tile const& tile) const
    {
    // From the tile, get a "quadkey" the Microsoft way.
    int x = tile.GetColumn();
    int y = tile.GetRow();
    Utf8String  quadKey = tileXYToQuadKey(x, y, tile.GetZoomLevel());
    int subdomain = (x + y) % 4;

    // from the template url, construct the tile url.
    Utf8String url;
    url.Sprintf(m_urlTemplate.c_str(), subdomain, quadKey.c_str());

    return url;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BingImageryProvider::_GetCacheFileName() const
    {
    switch (m_mapType)
        {
        case MapType::Street:
            return "BingRoad";

        case MapType::Aerial:
            return "BingAerial";

        case MapType::Hybrid:
            return "BingHybrid";
        }
    BeAssert(false);
    return "BingUnknown";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    BingImageryProvider::_FromJson(Json::Value const& value)
    {
    // the only thing currently stored in the BingImageryProvider Json is the MapType.
    m_mapType = (MapType) value[WebMercatorModel::json_mapType()].asInt((int)MapType::Street);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    BingImageryProvider::_ToJson(Json::Value& value) const
    {
    // the only thing currently stored in the BingImageryProvider Json is the MapType.
    value[WebMercatorModel::json_mapType()] = (int)m_mapType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BingImageryProvider::ReadAttributionsFromJson (Json::Value const& response)
    {
    Json::Value const& imageryProvidersJson = response["imageryProviders"];
    for (Json::ArrayIndex iAttribution = 0; iAttribution < 1000; iAttribution++)
        {
        BingImageryProvider::Attribution    thisAttribution;

        if (!imageryProvidersJson.isValidIndex (iAttribution))
            break;

        Json::Value const& attributionJson = imageryProvidersJson[iAttribution];
        thisAttribution.m_copyrightMessage = attributionJson["attribution"].asString();

        Json::Value const& coverageAreas = attributionJson["coverageAreas"];
        for (Json::ArrayIndex iCoverage = 0; iCoverage < 1000; iCoverage++)
            {
            BingImageryProvider::Coverage   thisCoverage;

            if (!coverageAreas.isValidIndex (iCoverage))
                break;

            Json::Value const& coverageJson     = coverageAreas[iCoverage];
            Json::Value const& boundingBox      = coverageJson["bbox"];
            thisCoverage.m_lowerLeftLatitude    = boundingBox[0].asDouble();
            thisCoverage.m_lowerLeftLongitude   = boundingBox[1].asDouble();
            thisCoverage.m_upperRightLatitude   = boundingBox[2].asDouble();
            thisCoverage.m_upperRightLongitude  = boundingBox[3].asDouble();
            thisCoverage.m_minimumZoomLevel     = (uint8_t) coverageJson["zoomMin"].asInt();
            thisCoverage.m_maximumZoomLevel     = (uint8_t) coverageJson["zoomMax"].asInt();
            thisAttribution.m_coverageList.push_back (thisCoverage);
            }

        m_attributions.push_back (thisAttribution);
        }

    }

struct Extent
    {
    GeoPoint    m_lowerLeft;
    GeoPoint    m_upperRight;
    uint8_t     m_lowestZoomLevel;
    uint8_t     m_highestZoomLevel;

    Extent() 
        { 
        Init();
        }

    void Init ()
        {
        m_lowerLeft.longitude = 180.0;
        m_lowerLeft.latitude = 90.0;
        m_lowerLeft.elevation = 0;

        m_upperRight.longitude = -180.0;
        m_upperRight.latitude = -90.0;
        m_upperRight.elevation = 0;

        m_lowestZoomLevel = 255;
        m_highestZoomLevel = 0;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void UnionTileExtent (Extent& extent, MapTileCR tile)
    {
    // first get the lat/long range from the tile.
    QuadTree::TileId id = tile.GetTileId();
    double nTiles = (1 << id.m_level);
    double west  = columnToLongitude(id.m_column, nTiles);
    double east  = columnToLongitude(id.m_column+1, nTiles);
    double north = rowToLatitude(id.m_row, nTiles);
    double south = rowToLatitude(id.m_row+1, nTiles);

    if (west < extent.m_lowerLeft.longitude)
        extent.m_lowerLeft.longitude = west;
    if (south < extent.m_lowerLeft.latitude)
        extent.m_lowerLeft.latitude = south;

    if (east > extent.m_upperRight.longitude)
        extent.m_upperRight.longitude = east;
    if (north > extent.m_upperRight.latitude)
        extent.m_upperRight.latitude = north;

    if (id.m_level < extent.m_lowestZoomLevel)
        extent.m_lowestZoomLevel = id.m_level;

    if (id.m_level > extent.m_highestZoomLevel)
        extent.m_highestZoomLevel = id.m_level;
    }


struct AttributionAppData : ViewController::AppData 
    {
    StopWatch   m_stopWatch;
    Extent      m_extent;

    AttributionAppData() : m_stopWatch(true) 
        {

        }
    };

static AttributionAppData::Key    s_attributionAppDataKey;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/18
+---------------+---------------+---------------+---------------+---------------+------*/
void BingImageryProvider::_OnSelectTiles (bvector<TileCPtr>& selected, DrawArgsR args) const
    {
    ViewController& viewController = args.GetContext().GetViewportR().GetViewControllerR();

    // get our AppData from the viewController.
    bool                newView = false;
    AttributionAppData* attributionAppData;
    if (nullptr == (attributionAppData = dynamic_cast <AttributionAppData*>(viewController.FindAppData(s_attributionAppDataKey))))
        {
        attributionAppData = new AttributionAppData ();
        viewController.AddAppData (s_attributionAppDataKey, attributionAppData);
        newView = true;
        }

    // There is no point in doing this every frame. So we set up a timer and do it every second.
    if (newView || (attributionAppData->m_stopWatch.GetCurrentSeconds() > 1.0))
        {
        attributionAppData->m_extent.Init();

        // calculate the extent of all the tiles and store it.
        for (auto tile : selected)
            {
            if (!tile.IsValid())
                continue;

            UnionTileExtent (attributionAppData->m_extent, *(static_cast<MapTileCP>(tile.get())));
            }

        // reset stopwatch.
        attributionAppData->m_stopWatch.Start();
        SpatialViewController* svc;
        if (nullptr != (svc = dynamic_cast<SpatialViewController*>(&viewController)))
            svc->InvalidateCopyrightInfo();
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void     AddUniqueString (T_Utf8StringVectorR messages, Utf8StringCR thisMessage)
    {
    for (auto message : messages)
        {
        if (0 == thisMessage.CompareTo (message))
            return;
        }
    messages.push_back (thisMessage);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void AttributionCopyrightMessagesFromExtent (std::vector<BingImageryProvider::Attribution>const & allAttributions, T_Utf8StringVectorR copyrightMessages, Extent& extent)
    {
    for (auto attribution : allAttributions)
        {
        for (auto coverage : attribution.m_coverageList)
            {
            // does the range overlap with the mapRoots range?
            if (coverage.m_minimumZoomLevel > extent.m_lowestZoomLevel)
                continue;
            if (coverage.m_maximumZoomLevel < extent.m_highestZoomLevel)
                continue;

            if (coverage.m_upperRightLongitude < extent.m_lowerLeft.longitude)
                continue;

            if (coverage.m_lowerLeftLongitude > extent.m_upperRight.longitude)
                continue;

            if (coverage.m_upperRightLatitude < extent.m_lowerLeft.latitude)
                continue;

            if (coverage.m_lowerLeftLatitude > extent.m_upperRight.latitude)
                continue;

            // if we get here, the data is used, and there is no need to look through the rest of the coverages.
            AddUniqueString (copyrightMessages, attribution.m_copyrightMessage);
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BingImageryProvider::_GetCopyrightMessage(ViewController& viewController) const
    {
    // get our AppData from the viewController.
    Utf8String          copyrightMsg;

    AttributionAppData* attributionAppData;
    if (nullptr != (attributionAppData = dynamic_cast <AttributionAppData*>(viewController.FindAppData(s_attributionAppDataKey))))
        {
        T_Utf8StringVector  copyrightMessages;

        AttributionCopyrightMessagesFromExtent (m_attributions, copyrightMessages, attributionAppData->m_extent);
        // concatentate copyright messages.

        for (auto thisString: copyrightMessages)
            {
            copyrightMsg.append (thisString);
            copyrightMsg.append (" ");
            }
        }
    return copyrightMsg;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ImageryProvider::TemplateUrlLoadStatus> BingImageryProvider::_FetchTemplateUrl()
    {
    // make a request to the following URL to get the http://dev.virtualearth.net/REST/v1/Imagery/Metadata/<imagerySet>?o=json&key= Metadata information
    // where <imagerySet> is Aerial, AerialWithLabels, or Road. There's an "OrdnanceSurvey" value but it's only valid for the London area.
    if (ImageryProvider::TemplateUrlLoadStatus::NotFetched != m_templateUrlLoadStatus)
        return m_templateUrlLoadStatus;

    // If the request has not yet been made, make it here:
    m_templateUrlLoadStatus.store(ImageryProvider::TemplateUrlLoadStatus::Requested);

    // base the request on the imagery type we want.
    Utf8String imagerySetName;
    switch (m_mapType)
        {
        case MapType::Street:
            imagerySetName.assign("Road");
            break;

        case MapType::Aerial:
            imagerySetName.assign("Aerial");
            break;

        case MapType::Hybrid:
            imagerySetName.assign("AerialWithLabels");
            break;

        default:
            BeAssert(false);
        }

    // prepare the url.
    Utf8String  url;

#if defined (NEEDSWORK_Bing_FetchKeyFromURL)
    // The key from our Microsoft agreement.
    Utf8String  bingKey = fetchKeyFromURL;
#else
    Utf8String  bingKey = "AtaeI3QDNG7Bpv1L53cSfDBgBKXIgLq3q-xmn_Y2UyzvF-68rdVxwAuje49syGZt";
#endif

    // this is hardcoding the Bentley key.
    url.Sprintf("http://dev.virtualearth.net/REST/v1/Imagery/Metadata/%s?o=json&incl=ImageryProviders&key=%s", imagerySetName.c_str(), bingKey.c_str());

    // make the URL request.
    Http::Request request(url);
    BingImageryProviderPtr me(this);

    return request.Perform().then([me] (Http::Response response)
        {
        // we got the response from the server.
        if (Http::ConnectionStatus::OK == response.GetConnectionStatus() && Http::HttpStatus::OK == response.GetHttpStatus())
            {
            // typical reponse, (LF's added for clarity)
            // {"authenticationResultCode":"ValidCredentials",
            // "brandLogoUri":"http:\/\/dev.virtualearth.net\/Branding\/logo_powered_by.png",
            // "copyright":"Copyright � 2017 Microsoft and its suppliers. All rights reserved. This API cannot be accessed and the content and any results may not be used, reproduced or transmitted in any manner without express written permission from Microsoft Corporation.",
            // "resourceSets":
            //    [{"estimatedTotal":1,
            //      "resources":
            //        [{"__type":
            //          "ImageryMetadata:http:\/\/schemas.microsoft.com\/search\/local\/ws\/rest\/v1",
            //          "imageHeight":256,
            //          "imageUrl":"http:\/\/ecn.{subdomain}.tiles.virtualearth.net\/tiles\/r{quadkey}.jpeg?g=5677&mkt={culture}&shading=hill",
            //          "imageUrlSubdomains":["t0","t1","t2","t3"],
            //          "imageWidth":256,
            //          "imageryProviders":null,
            //          "vintageEnd":null,
            //          "vintageStart":null,
            //          "zoomMax":21,
            //          "zoomMin":1}
            //        ]}
            //    ],
            //  "statusCode":200,
            //  "statusDescription":"OK",
            //  "traceId":"5ad3ec719ca34a168d3bd29e33e147fe|BN20130431|7.7.0.0|"
            //  }
            //

            Http::HttpResponseContentPtr    content = response.GetContent();
            Http::HttpBodyPtr               body = content->GetBody();
            Utf8String                      responseString = body->AsString();
            Json::Value                     responseJson;
            Json::Reader::Parse(responseString.c_str(), responseJson);
            if (responseJson.isNull())
                {
                me->m_templateUrlLoadStatus.store(ImageryProvider::TemplateUrlLoadStatus::Failed);
                return ImageryProvider::TemplateUrlLoadStatus::Failed;
                }

            // get the url for the bing logo.
            me->m_creditUrl             = responseJson["brandLogoUri"].asString();

            JsonValueCR resourceUrl     = responseJson["resourceSets"][0]["resources"][0];
            me->m_minimumZoomLevel      = resourceUrl["zoomMin"].asInt();
            me->m_maximumZoomLevel      = resourceUrl["zoomMax"].asInt();
            me->m_tileHeight            = resourceUrl["imageHeight"].asInt();
            me->m_tileWidth             = resourceUrl["imageWidth"].asInt();

            me->ReadAttributionsFromJson (resourceUrl);

            Utf8String rawTemplate      = resourceUrl["imageUrl"].asString();

            size_t  subdomain           = rawTemplate.find("{subdomain}");
            BeAssert(Utf8String::npos != subdomain);
            rawTemplate.replace(subdomain, 11, "t%d");    // Note:: Depends on imageUrlSubdomains returning "t0", "t1", "t2", "t3"  !!

            size_t quadkey              = rawTemplate.find("{quadkey}");
            BeAssert(Utf8String::npos != quadkey);
            rawTemplate.replace(quadkey, 9, "%s");

            // NEEDSWORK_Culture
            size_t culture              = rawTemplate.find("{culture}");
            if (Utf8String::npos != culture)
                rawTemplate.replace(culture, 9, "en-US");

            me->m_urlTemplate = rawTemplate;

            // if we got a creditUrl from the response, download it also.
            if (!me->m_creditUrl.empty ())
                {
                Http::Request logoRequest (me->m_creditUrl);
                Http::HttpByteStreamBodyPtr byteStream = new Http::HttpByteStreamBody();
                logoRequest.SetResponseBody (byteStream);
                logoRequest.Perform().then ([me, byteStream] (Http::Response logoResponse)
                    {
                    if (Http::ConnectionStatus::OK == logoResponse.GetConnectionStatus() && Http::HttpStatus::OK == logoResponse.GetHttpStatus())
                        {
                        Http::HttpResponseContentPtr    logoContent = logoResponse.GetContent();
                        me->m_logoByteStream = std::move (byteStream->GetByteStream());
                        me->m_logoContentType = logoResponse.GetHeaders().GetContentType();
                        me->m_logoValid.store(true);
                        }
                    });
                }

            // download a tile that we know is the worthless tile that Bing map sends out when they don't have data.
            if (true)
                {
                // get the key of a tile we know is missing 
                Utf8String missingTileKey = tileXYToQuadKey (0, 0, me->m_maximumZoomLevel-1);

                // from the template url, construct the tile url.
                Utf8String url;
                url.Sprintf (me->m_urlTemplate.c_str(), 0, missingTileKey.c_str());

                // make the URL request.
                Http::Request request (url);
                Http::HttpByteStreamBodyPtr byteStream = new Http::HttpByteStreamBody();
                request.SetResponseBody (byteStream);

                request.Perform().then([me, byteStream] (Http::Response response)
                    {
                    // we got the response from the server.
                    if (Http::ConnectionStatus::OK == response.GetConnectionStatus() && Http::HttpStatus::OK == response.GetHttpStatus())
                        {
                        Http::HttpResponseContentPtr    content = response.GetContent();
                        StreamBuffer                    tileBytes   = std::move(byteStream->GetByteStream());
                        // use  the middle 1/3 of the data.
                        uint32_t                        dataSize = tileBytes.GetSize() / 3;   

                        me->m_missingTileData = (uint8_t*) malloc (dataSize);
                        memcpy (me->m_missingTileData, (tileBytes.GetData() + dataSize), dataSize);
                        me->m_missingTileDataSize.store (tileBytes.GetSize());
                        }
                    });
                }

            me->m_templateUrlLoadStatus.store(ImageryProvider::TemplateUrlLoadStatus::Received);
            return ImageryProvider::TemplateUrlLoadStatus::Received;
            }

        me->m_templateUrlLoadStatus.store(ImageryProvider::TemplateUrlLoadStatus::Failed);
        return ImageryProvider::TemplateUrlLoadStatus::Failed;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool BingImageryProvider::_MatchesMissingTile (ByteStream& tileBytes) const
    {
    // missing tile data might not have come in yet.
    uint32_t missingTileDataSize;
    if (0 == (missingTileDataSize = m_missingTileDataSize.load()))
        return false;

    // If it's not the same size, it doesn't match.
    uint32_t dataSize;
    if ((dataSize = tileBytes.GetSize()) != missingTileDataSize)
        return false;

    // start 1/3 of the way in, look at 1/3 of the data.
    uint32_t compareSize = dataSize/3;
    uint8_t const* thisData = tileBytes.GetData() + compareSize;
    return 0 == memcmp (thisData, m_missingTileData, compareSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::RgbaSpriteP   BingImageryProvider::_GetCopyrightSprite ()
    {
    if (!m_logoValid.load())
        return nullptr;

    if (m_copyrightSprite.IsValid())
        return m_copyrightSprite.get();

    // we have a byte stream, create the logo image
    Render::ImageSource::Format format;
    if (0 == m_logoContentType.CompareTo ("image/png"))
        format = Render::ImageSource::Format::Png;
    else if (0 == m_logoContentType.CompareTo ("image/jpeg"))
        format = Render::ImageSource::Format::Jpeg;
    else
        {
        BeAssert (false);
        return nullptr;
        }

    Render::ImageSource source (format, std::move(m_logoByteStream));
    m_copyrightSprite = Render::RgbaSprite::CreateFrom(source);

    return m_copyrightSprite.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/17
* Our Bing Maps provider limits the tile lifetime to three days.
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t    BingImageryProvider::_GetMaxValidDuration () const
    {
    // testing - 3 minute
    // return 3.0 * 60.0 * 1000.0;

    //     days  hours/day    seconds/Hour  milli/second
    return 3.0 * 24.0       * 3600          * 1000;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
HereImageryProvider*  HereImageryProvider::Create (Json::Value const& providerDataValue)
    {
    HereImageryProvider*    imageryProvider = new HereImageryProvider();
    imageryProvider->_FromJson (providerDataValue);

    // if the mapType is None, return null.
    if (MapType::None == imageryProvider->m_mapType)
        return nullptr;

    return imageryProvider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
HereImageryProvider::HereImageryProvider()
    {
    // Trial period credentials, good only until July 9, 2017.
    m_appId.assign("Eieg0LYRqg5cyHQdUPCf");
    m_appCode.assign("scZCSrR56QXuGU4_EwzQGQ");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HereImageryProvider::_ConstructUrl(TileTree::QuadTree::Tile const& tile) const
    {
    // The general format (from Here documentation: https://developer.here.com/rest-apis/documentation/enterprise-map-tile/topics/request-constructing.html
    // {Base URL}{Path}{resource (tile type)}/{map id}/{scheme}/{zoom}/{column}/{row}/{size}/{format}
    // ?app_id={YOUR_APP_ID}
    // &app_code={YOUR_APP_CODE}
    // &{param}={value}
    // 
    // Base URL for Map types:      https://{1-4}.base.maps.api.here.com
    // Base URL for Aerial type:    https://{1-4}.aerial.maps.api.here.com
    // They also have traffic tiles, but I don't think they are relevant for us.

    int x = tile.GetColumn();
    int y = tile.GetRow();
    int subdomain = 1 + ((x + y) % 4);

    Utf8String  url;
    url.Sprintf(m_urlTemplate.c_str(), subdomain, tile.GetZoomLevel(), x, y, m_appId.c_str(), m_appCode.c_str());

    return url;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HereImageryProvider::_GetCopyrightMessage(ViewController&) const
    {
    return "(c) HERE";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HereImageryProvider::_GetCacheFileName() const
    {
    switch (m_mapType)
        {
        case MapType::Street:
            return "HereRoad";

        case MapType::Aerial:
            return "HereAerial";

        case MapType::Hybrid:
            return "HereHybrid";
        }
    BeAssert(false);
    return "HereUnknown";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    HereImageryProvider::_FromJson(Json::Value const& value)
    {
    // the only thing currently stored in the HereImageryProvider Json is the MapType.
    m_mapType = (MapType) value[WebMercatorModel::json_mapType()].asInt((int)MapType::Street);

    switch (m_mapType)
        {
        case MapType::Street:
            m_urlTemplate = "https://%d.base.maps.api.here.com/maptile/2.1/maptile/newest/normal.day/%d/%d/%d/256/jpg?app_id=%s&app_code=%s";
            break;

        case MapType::Aerial:
            m_urlTemplate = "https://%d.aerial.maps.api.here.com/maptile/2.1/maptile/newest/satellite.day/%d/%d/%d/256/jpg?app_id=%s&app_code=%s";
            break;

        case MapType::Hybrid:
            m_urlTemplate = "https://%d.aerial.maps.api.here.com/maptile/2.1/maptile/newest/hybrid.day/%d/%d/%d/256/jpg?app_id=%s&app_code=%s";
            break;

        default:
            BeAssert(false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    HereImageryProvider::_ToJson(Json::Value& value) const
    {
    // the only thing currently stored in the HereImageryProvider Json is the MapType.
    value[WebMercatorModel::json_mapType()] = (int)m_mapType;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::RootPtr WebMercatorModel::_GetTileTree(RenderContextR context)
    {
    return GetTileTree(context.GetRenderSystem());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::RootPtr WebMercatorModel::_CreateTileTree(Render::SystemP system)
    {
    return LoadTileTree(system);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
WebMercatorDisplayHandler::WebMercatorDisplayHandler(Json::Value const& settings) 
    { 
    _Initialize(settings);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorDisplayHandler::_Initialize(Json::Value const& settings)
    {
    Utf8String providerName = settings[WebMercatorModel::json_providerName()].asString(WebMercator::BingImageryProvider::prop_BingProvider());
    Json::Value const& providerData = settings[WebMercatorModel::json_providerData()];

    if (0 == providerName.CompareToI(WebMercator::MapBoxImageryProvider::prop_MapBoxProvider()))
        {
        m_provider = MapBoxImageryProvider::Create(providerData);
        }
    else if (0 == providerName.CompareToI(WebMercator::BingImageryProvider::prop_BingProvider()))
        {
        m_provider = BingImageryProvider::Create(providerData);
        }
    else if (0 == providerName.CompareToI(WebMercator::HereImageryProvider::prop_HereProvider()))
        {
        m_provider = HereImageryProvider::Create(providerData);
        }
    m_settings = settings;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::RootPtr WebMercatorDisplayHandler::_GetTileTree(SceneContextR sceneContext) 
    {
    if (m_provider.IsNull())
        {
        BeAssert(false);
        return nullptr;
        }

    // Here we would like to create the TileTree root (MapRoot), but we might not be ready to do that for some ImageryProviders.
    // For example, the Bing provider isn't ready to go until it has fetched the template URL.
    ImageryProvider::TemplateUrlLoadStatus templateStatus = m_provider->_GetTemplateUrlLoadStatus();
    if (ImageryProvider::TemplateUrlLoadStatus::Received == templateStatus)
        {
        Transform biasTrans;
        biasTrans.InitFrom(DPoint3d::From(0.0, 0.0, m_settings[WebMercatorModel::json_groundBias()].asDouble(-1.0)));


        uint32_t maxSize = 362; // the maximum pixel size for a tile. Approximately sqrt(256^2 + 256^2).
        return new MapRoot(sceneContext.GetDgnDb(), DgnModelId((uint64_t) 0), biasTrans, *m_provider.get(), sceneContext.GetRenderSystem(), ImageSource::Format::Jpeg, 0.0, maxSize);
        }

    // Here we do not yet have the TemplateUrl. _FetchTemplateUrl can be called multiple times - it takes care of checking the status, etc.
    m_provider->_FetchTemplateUrl();
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool WebMercatorDisplayHandler::_GetDisplayPlane(DPlane3dR plane)
    {
    if (m_provider.IsNull())
        {
        BeAssert(false);
        return false;
        }

    double z = m_settings[WebMercatorModel::json_groundBias()].asDouble(-1.0);
    plane.InitFromOriginAndNormal(0.0, 0.0, z, 0.0, 0.0, 1.0);
    return true;
    }

