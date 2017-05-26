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

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void MapTile::_GetGraphics(DrawGraphicsR args, int depth) const
    {
    if (m_reprojected)  // if we were unable to re-project this tile, don't draw it.
        T_Super::_GetGraphics(args, depth);
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

    auto graphic = mapRoot.GetRenderSystem()->_CreateGraphic(Graphic::CreateParams(nullptr));

    // some tile servers (for example Bing) start returning PNG tiles at a certain zoom level, even if you request Jpeg.
    ImageSource::Format format = mapRoot.m_format;
    if (0 == m_contentType.CompareTo("image/png"))
        format = ImageSource::Format::Png;
    if (0 == m_contentType.CompareTo("image/jpeg"))
        format = ImageSource::Format::Jpeg;

    ImageSource source(format, std::move(m_tileBytes));
    Texture::CreateParams textureParams;
    textureParams.SetIsTileSection();
    auto texture = mapRoot.GetRenderSystem()->_CreateTexture(source, Image::Format::Rgb, Image::BottomUp::No, textureParams);
    m_tileBytes = std::move(source.GetByteStreamR()); // move the data back into this object. This is necessary since we need to keep to save it in the tile cache.

    graphic->SetSymbology(mapRoot.m_tileColor, mapRoot.m_tileColor, 0); // this is to set transparency
    graphic->AddTile(*texture, tile.m_corners); // add the texture to the graphic, mapping to corners of tile (in BIM world coordinates)

    auto stat = graphic->Close(); // explicitly close the Graphic. This potentially blocks waiting for QV from other threads
    BeAssert(SUCCESS==stat);
    UNUSED_VARIABLE(stat);

    tile.m_graphic = graphic;

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

    IGraphicBuilder::TileCorners corners;
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
Utf8String MapRoot::_ConstructTileResource(TileCR tile) const
    {
    QuadTree::Tile const* quadTile = dynamic_cast <QuadTree::Tile const*>(&tile);
    BeAssert(nullptr != quadTile);
    return m_imageryProvider->_ConstructUrl(*quadTile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/

MapRoot::MapRoot(DgnDbR db, TransformCR trans, ImageryProviderR imageryProvider, Dgn::Render::SystemP system, Render::ImageSource::Format format, double transparency,
        uint32_t maxSize) : QuadTree::Root(db, trans, nullptr, system, imageryProvider._GetMaximumZoomLevel(false), maxSize, transparency), m_format(format), m_imageryProvider(&imageryProvider)
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
    LIMIT_RANGE(0.0, .9, m_transparency);

    Utf8String providerName = value[json_providerName()].asString();

    if (0 == providerName.CompareToI(WebMercator::MapBoxImageryProvider::prop_MapBoxProvider()))
        {
        m_provider = new MapBoxImageryProvider();
        }
    else if (0 == providerName.CompareToI(WebMercator::BingImageryProvider::prop_BingProvider()))
        {
        m_provider = new BingImageryProvider();
        }
    else if (0 == providerName.CompareToI(WebMercator::HereImageryProvider::prop_HereProvider()))
        {
        m_provider = new HereImageryProvider();
        }

    if (m_provider.IsValid())
        {
        BeAssert(value.isMember(json_providerData()));
        m_provider->_FromJson(value[json_providerData()]);
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
Utf8String WebMercatorModel::_GetCopyrightMessage() const
    {
    return m_provider.IsValid() ? m_provider->_GetCreditMessage() : "";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::Load(SystemP renderSys) const
    {
    if (m_provider.IsNull())
        {
        BeAssert(false);
        return;
        }

    if (m_root.IsValid() && (nullptr==renderSys || m_root->GetRenderSystem()==renderSys))
        return;

    Transform biasTrans;
    biasTrans.InitFrom(DPoint3d::From(0.0, 0.0, m_groundBias));

    uint32_t maxSize = 362; // the maximum pixel size for a tile. Approximately sqrt(256^2 + 256^2).
    m_root = new MapRoot(m_dgndb, biasTrans, *m_provider.get(), renderSys, ImageSource::Format::Jpeg, m_transparency, maxSize);
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
Utf8String MapBoxImageryProvider::_GetCreditMessage() const
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
        case MapBoxImageryProvider::MapType::StreetMap:
            return "MapBoxStreets";

        case MapBoxImageryProvider::MapType::Satellite:
            return "MapBoxSatellite";

        case MapBoxImageryProvider::MapType::StreetsAndSatellite:
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
    m_mapType = (MapBoxImageryProvider::MapType) value[json_mapType()].asInt((int)MapBoxImageryProvider::MapType::StreetMap);

    switch (m_mapType)
        {
        case MapBoxImageryProvider::MapType::StreetMap:
            m_baseUrl = "http://api.mapbox.com/v4/mapbox.streets/";
            break;

        case MapBoxImageryProvider::MapType::Satellite:
            m_baseUrl = "http://api.mapbox.com/v4/mapbox.satellite/";
            break;

        case MapBoxImageryProvider::MapType::StreetsAndSatellite:
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
    value[json_mapType()] = (int)m_mapType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String MapBoxImageryProvider::_GetCreditUrl() const
    {
    // NEEDSWORK_MapBox
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String tileXYToQuadKey(int tileX, int tileY, int levelOfDetail)
    {
    // blatantly ripped off from C# example in bing documentation https://msdn.microsoft.com/en-us/library/bb259689.aspx
    Utf8String  quadKey;

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
Utf8String BingImageryProvider::_GetCreditMessage() const
    {
    return "(c) Microsoft";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BingImageryProvider::_GetCacheFileName() const
    {
    switch (m_mapType)
        {
        case BingImageryProvider::MapType::Road:
            return "BingRoad";

        case BingImageryProvider::MapType::Aerial:
            return "BingAerial";

        case BingImageryProvider::MapType::AerialWithLabels:
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
    m_mapType = (BingImageryProvider::MapType) value[json_mapType()].asInt((int)BingImageryProvider::MapType::Road);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    BingImageryProvider::_ToJson(Json::Value& value) const
    {
    // the only thing currently stored in the BingImageryProvider Json is the MapType.
    value[json_mapType()] = (int)m_mapType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String  BingImageryProvider::_GetCreditUrl() const
    {
    // NEEDSWORK_MapBox
    return nullptr;
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
        case BingImageryProvider::MapType::Road:
            imagerySetName.assign("Road");
            break;

        case BingImageryProvider::MapType::Aerial:
            imagerySetName.assign("Aerial");
            break;

        case BingImageryProvider::MapType::AerialWithLabels:
            imagerySetName.assign("AerialWithLabels");
            break;

        default:
            BeAssert(false);
        }

    // prepare the url.
    Utf8String url;
    url.Sprintf("http://dev.virtualearth.net/REST/v1/Imagery/Metadata/%s?o=json&key=Am-FIomxQ8COwv6zeuMNoc9xx3rMoeNYo8prPUJysZeQSuGLHQ9VbrHa9hNaO23z", imagerySetName.c_str());

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
                return ImageryProvider::TemplateUrlLoadStatus::Failed;

            // get the url for the bing logo.
            me->m_creditUrl             = responseJson["brandLogoUri"].asString();

            JsonValueCR resourceUrl     = responseJson["resourceSets"][0]["resources"][0];
            me->m_minimumZoomLevel      = resourceUrl["zoomMin"].asInt();
            me->m_maximumZoomLevel      = resourceUrl["zoomMax"].asInt();
            me->m_tileHeight            = resourceUrl["imageHeight"].asInt();
            me->m_tileWidth             = resourceUrl["imageWidth"].asInt();

            Utf8String rawTemplate      = resourceUrl["imageUrl"].asString();

            size_t  subdomain           = rawTemplate.find("{subdomain}");
            BeAssert(Utf8String::npos != subdomain);
            rawTemplate.replace(subdomain, 11, "t%d");    // Note:: Depends on imageUrlSubdomains returning "t0", "t1", "t2", "t3"  !!

            size_t quadkey              = rawTemplate.find("{quadkey}");
            BeAssert(Utf8String::npos != quadkey);
            rawTemplate.replace(quadkey, 9, "%s");

            // NEEDSWORK_Culture
            size_t culture              = rawTemplate.find("{culture}");
            BeAssert(Utf8String::npos != culture);
            rawTemplate.replace(culture, 9, "en-US");

            me->m_urlTemplate = rawTemplate;

            return ImageryProvider::TemplateUrlLoadStatus::Received;
            }

        return ImageryProvider::TemplateUrlLoadStatus::Failed;
        });
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
Utf8String HereImageryProvider::_GetCreditMessage() const
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
        case HereImageryProvider::MapType::Map:
            return "HereRoad";

        case HereImageryProvider::MapType::Aerial:
            return "HereAerial";

        case HereImageryProvider::MapType::Combined:
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
    m_mapType = (HereImageryProvider::MapType) value[json_mapType()].asInt((int)HereImageryProvider::MapType::Map);

    switch (m_mapType)
        {
        case HereImageryProvider::MapType::Map:
            m_urlTemplate = "https://%d.base.maps.api.here.com/maptile/2.1/maptile/newest/normal.day/%d/%d/%d/256/jpg?app_id=%s&app_code=%s";
            break;

        case HereImageryProvider::MapType::Aerial:
            m_urlTemplate = "https://%d.aerial.maps.api.here.com/maptile/2.1/maptile/newest/satellite.day/%d/%d/%d/256/jpg?app_id=%s&app_code=%s";
            break;

        case HereImageryProvider::MapType::Combined:
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
    value[json_mapType()] = (int)m_mapType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String  HereImageryProvider::_GetCreditUrl() const
    {
    // NEEDSWORK_MapBox
    return nullptr;
    }

BEGIN_BENTLEY_DGN_NAMESPACE

namespace WebMercator
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
struct FetchTemplateUrlProgressiveTask : ProgressiveTask 
    {
    folly::Future<ImageryProvider::TemplateUrlLoadStatus>   m_future;
    WebMercatorModelCPtr                                    m_model;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Barry.Bentley                   04/17
    +---------------+---------------+---------------+---------------+---------------+------*/
    FetchTemplateUrlProgressiveTask(folly::Future<ImageryProvider::TemplateUrlLoadStatus>&& future, WebMercatorModel const* model) : m_future(std::move(future)), m_model(model) {}

    ~FetchTemplateUrlProgressiveTask() 
        {
        // The progressive display is deleted if the view is closed.
        // If the request is still outstanding, then set it back to "NotFetched"
        ImageryProvider::TemplateUrlLoadStatus status = m_model->m_provider->_GetTemplateUrlLoadStatus();
        if (ImageryProvider::TemplateUrlLoadStatus::Requested == status)
            m_model->m_provider->_SetTemplateUrlLoadStatus(ImageryProvider::TemplateUrlLoadStatus::NotFetched);
        }

    /*---------------------------------------------------------------------------------**//**
    * Called periodically (on a timer) on the client thread to check for arrival of the template Url.
    * @bsimethod                                    Keith.Bentley                   08/16
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ProgressiveTask::Completion _DoProgressive(RenderListContext& context, WantShow& wantShow) override
        {
        // won't affect the screen.
        wantShow = WantShow::No;

        // if we haven't gotten a response yet, go around another progressive cycle.
        if (!m_future.isReady())
            return Completion::Aborted;

        // now we have the response from the server.
        m_model->m_provider->_SetTemplateUrlLoadStatus(m_future.get());

        switch (m_future.get())
            {
            case ImageryProvider::TemplateUrlLoadStatus::Received:
                {
                // got it - go on to the next task.
                m_model->Load(&context.GetTargetR().GetSystem());

                if (m_model->m_root.IsValid())
                    m_model->m_root->DrawInView(context, m_model->m_root->GetLocation(), m_model->m_root->m_clip.get());
                return Completion::Finished;
                }

            case ImageryProvider::TemplateUrlLoadStatus::Requested:
                {
                // still waiting.
                return Completion::Aborted;
                }

            case ImageryProvider::TemplateUrlLoadStatus::Failed:
            case ImageryProvider::TemplateUrlLoadStatus::Abandoned:
                {
                return Completion::Finished;
                }
            }

        return Completion::Aborted;
        }
    };
};

END_BENTLEY_DGN_NAMESPACE
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::_AddTerrainGraphics(TerrainContextR context) const
    {
    // need a provider to get the tiles.
    if (m_provider.IsNull())
        return;

    folly::Future<ImageryProvider::TemplateUrlLoadStatus> future = m_provider->_FetchTemplateUrl();
    if (!future.isReady())
        {
        for (;;)
            {
            if (!context.GetUpdatePlan().GetQuitTime().IsInFuture()) // do we want to wait for them? This is really just for thumbnails
                {
                // don't have the tile template yet, schedule a progressive pass to get it.
                context.GetViewport()->ScheduleProgressiveTask(*new FetchTemplateUrlProgressiveTask(std::move(future), this));
                return;
                }
            else
                {
                // this is for the thumbnail case - wait for the fetch to finish.
                BeDuration::FromMilliseconds(20).Sleep(); // we want to wait. Give tiles some time to arrive
                if (ImageryProvider::TemplateUrlLoadStatus::Received == m_provider->_GetTemplateUrlLoadStatus())
                    break;
                }
            }
        }

    // don't need or already have TemplateUrl - go on to load and display the model.
    Load(&context.GetTargetR().GetSystem());

    if (m_root.IsValid())
        m_root->DrawInView(context, m_root->GetLocation(), m_root->m_clip.get());
    }
