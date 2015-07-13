/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/WmsSource.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RasterSchemaInternal.h"
#include <DgnPlatform/DgnCore/ImageUtilities.h>
#include "WmsSource.h"

#define TABLE_NAME_WmsTileData "WmsTileData"

#define  CONTENT_TYPE_PNG       "image/png"
#define  CONTENT_TYPE_JPEG      "image/jpeg"

USING_NAMESPACE_BENTLEY_SQLITE

//=======================================================================================
// @bsimethod                                                   Mathieu.Marchand  6/2015
//=======================================================================================
struct WmsTileData : IRealityData<WmsTileData, BeSQLiteRealityDataStorage, HttpRealityDataSource>
    {
    //===================================================================================
    // @bsimethod                                               Mathieu.Marchand  6/2015
    //===================================================================================
    struct RequestOptions : RealityDataCacheOptions, IRealityData::RequestOptions
    {
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS
    private:
        RequestOptions(bool allowExpired, bool requestFromSource) :RealityDataCacheOptions(allowExpired, requestFromSource) {DEFINE_BENTLEY_REF_COUNTED_MEMBER_INIT}        
    public:
        ~RequestOptions() {}
        //! Creates the options object.
        //! @param[in] returnExpired        Should the cache return reality data even if it's expired.
        //! @param[in] requestFromSource    Should the cache request for fresh data if it's expired or doesn't exist.
        static RefCountedPtr<RequestOptions> Create(bool returnExpired, bool requestFromSource) {return new RequestOptions(returnExpired, requestFromSource);}        
    };

private:
    Utf8String      m_url;
    bvector<Byte>   m_data;
    DateTime        m_creationDate;
    Utf8String      m_contentType;

private:
    WmsTileData(){}

    bool IsSupportedContent(Utf8StringCR contentType) const;
        
protected:
    virtual Utf8CP _GetId() const override {return m_url.c_str();}
    virtual bool _IsExpired() const override;
    virtual BentleyStatus _InitFrom(Utf8CP url, bmap<Utf8String, Utf8String> const& header, bvector<Byte> const& body, HttpRealityDataSource::RequestOptions const& options) override;
    virtual BentleyStatus _InitFrom(BeSQLite::Db& db, Utf8CP key, BeSQLiteRealityDataStorage::SelectOptions const& options) override;
    virtual BentleyStatus _Persist(BeSQLite::Db& db) const override;
    virtual BeSQLiteRealityDataStorage::DatabasePrepareAndCleanupHandlerPtr _GetDatabasePrepareAndCleanupHandler() const override;

public:
    static RefCountedPtr<WmsTileData> Create() {return new WmsTileData();}    
    bvector<Byte> const& GetData() const  {return m_data;}
    DateTime GetCreationDate() const  {return m_creationDate;}
    Utf8String GetContentType() const  {return m_contentType;}
    };

//----------------------------------------------------------------------------------------
//-------------------------------  WmsTileData      ----------------------------------------
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
bool WmsTileData::_IsExpired() const 
    {
    return DateTime::CompareResult::EarlierThan == DateTime::Compare(GetExpirationDate(), DateTime::GetCurrentTime());
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.Marchand  6/2015
//=======================================================================================
struct WmsTileDataPrepareAndCleanupHandler : BeSQLiteRealityDataStorage::DatabasePrepareAndCleanupHandler
    {
    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  6/2015
    //----------------------------------------------------------------------------------------
    virtual BentleyStatus _PrepareDatabase(BeSQLite::Db& db) const override
        {
        if (db.TableExists(TABLE_NAME_WmsTileData))
            return SUCCESS;
    
        Utf8CP ddl = "Url CHAR PRIMARY KEY, \
                        Raster BLOB,          \
                        RasterSize INT,       \
                        ContentType CHAR,     \
                        Created BIGINT,       \
                        Expires BIGINT,       \
                        ETag CHAR";
        return BeSQLite::BE_SQLITE_OK == db.CreateTable(TABLE_NAME_WmsTileData, ddl) ? SUCCESS : ERROR;
        }
    
    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  6/2015
    //----------------------------------------------------------------------------------------
    virtual BentleyStatus _CleanupDatabase(BeSQLite::Db& db, double percentage) const override
        {
        CachedStatementPtr sumStatement;
        if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement(sumStatement, "select sum(RasterSize) from " TABLE_NAME_WmsTileData))
            return ERROR;

        if (BeSQLite::BE_SQLITE_ROW != sumStatement->Step()) 
            return ERROR;
            
        auto rasterSize = sumStatement->GetValueInt(0);         
        uint64_t overHead = (uint64_t)(rasterSize * percentage) / 100;
    
        CachedStatementPtr selectStatement;
        if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement(selectStatement, "select RasterSize, Created from " TABLE_NAME_WmsTileData " ORDER BY Created ASC"))
            return ERROR;
    
        uint64_t runningSum = 0;
        while ((runningSum < overHead) &&(BeSQLite::BE_SQLITE_ROW == selectStatement->Step()))
            runningSum += selectStatement->GetValueInt(0);
            
        CachedStatementPtr deleteStatement;
        uint64_t creationDate = selectStatement->GetValueInt64(1);
        if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement(deleteStatement, "DELETE FROM " TABLE_NAME_WmsTileData " WHERE Created  <= ? "))
            return ERROR;

        deleteStatement->BindInt64(1, creationDate);
        if (BeSQLite::BE_SQLITE_DONE != deleteStatement->Step())
            return ERROR;
            
        return SUCCESS;
        }
    };

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
BeSQLiteRealityDataStorage::DatabasePrepareAndCleanupHandlerPtr WmsTileData::_GetDatabasePrepareAndCleanupHandler() const
    {
    return new WmsTileDataPrepareAndCleanupHandler();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
bool WmsTileData::IsSupportedContent(Utf8StringCR contentType) const
    {
    // Only jpeg and png for now.
    // "application/vnd.ogc.se_xml" would be a Wms exception. In the future, we should report that to the user.
    if(contentType.EqualsI (CONTENT_TYPE_PNG) || contentType.EqualsI (CONTENT_TYPE_JPEG))
        return true;
    
    return false;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
BentleyStatus WmsTileData::_InitFrom(Utf8CP url, bmap<Utf8String, Utf8String> const& header, bvector<Byte> const& body, HttpRealityDataSource::RequestOptions const& requestOptions) 
    {
    BeAssert(nullptr != dynamic_cast<RequestOptions const*>(&requestOptions));
   // RequestOptions const& options = static_cast<RequestOptions const&>(requestOptions);
    
    m_url.AssignOrClear(url);
    m_creationDate = DateTime::GetCurrentTime();

    auto contentTypeIter = header.find("Content-Type");
    if (contentTypeIter == header.end())
        return ERROR;

    // Reject and don't cache what we can't consumed.
    if(!IsSupportedContent(contentTypeIter->second))
        return BSIERROR;

    m_contentType = contentTypeIter->second.c_str();
    m_data = body;

    return BSISUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
BentleyStatus WmsTileData::_InitFrom(BeSQLite::Db& db, Utf8CP key, BeSQLiteRealityDataStorage::SelectOptions const& options)
    {
    HighPriorityOperationBlock highPriority;

    CachedStatementPtr stmt;
    if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT Raster, RasterSize, Created, Expires, ETag, ContentType from " TABLE_NAME_WmsTileData " WHERE Url=?"))
        return ERROR;

    stmt->ClearBindings();
    stmt->BindText(1, key, BeSQLite::Statement::MakeCopy::Yes);
    if (BeSQLite::BE_SQLITE_ROW == stmt->Step())
        {
        m_url = key;

        auto raster     = static_cast<Byte const*>(stmt->GetValueBlob(0));
        auto rasterSize = stmt->GetValueInt(1);
        m_data.assign(raster, raster + rasterSize);

        DateTime::FromUnixMilliseconds(m_creationDate,(uint64_t) stmt->GetValueInt64(2));
        m_contentType = stmt->GetValueText(5);

        DateTime expirationDate;
        DateTime::FromUnixMilliseconds(expirationDate,(uint64_t) stmt->GetValueInt64(3));
        SetExpirationDate(expirationDate);
        SetEntityTag(stmt->GetValueText(4));

        return SUCCESS;
        }
    return ERROR;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
BentleyStatus WmsTileData::_Persist(BeSQLite::Db& db) const
    {
    int bufferSize = (int) GetData().size();

    int64_t creationTime = 0;
    if (SUCCESS != GetCreationDate().ToUnixMilliseconds(creationTime))
        return ERROR;

    int64_t expirationDate = 0;
    if (SUCCESS != GetExpirationDate().ToUnixMilliseconds(expirationDate))
        return ERROR;

    HighPriorityOperationBlock highPriority;

    CachedStatementPtr selectStatement;
    if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement(selectStatement, "SELECT Url from " TABLE_NAME_WmsTileData " WHERE Url=?"))
        return ERROR;

    selectStatement->ClearBindings();
    selectStatement->BindText(1, GetId(), BeSQLite::Statement::MakeCopy::Yes);
    if (BeSQLite::BE_SQLITE_ROW == selectStatement->Step())
        {
        // update
        CachedStatementPtr stmt;
        if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement(stmt, "UPDATE " TABLE_NAME_WmsTileData " SET Expires=?, ETag=? WHERE Url=?"))
            return ERROR;

        stmt->ClearBindings();
        stmt->BindInt64(1, expirationDate);
        stmt->BindText (2, GetEntityTag(), BeSQLite::Statement::MakeCopy::Yes);
        stmt->BindText (3, GetId(), BeSQLite::Statement::MakeCopy::Yes);
        if (BeSQLite::BE_SQLITE_DONE != stmt->Step())
            return ERROR;
        }
    else
        {
        // insert
        CachedStatementPtr stmt;
        if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement(stmt, "INSERT INTO " TABLE_NAME_WmsTileData "(Url, Raster, RasterSize, Created, Expires, ETag, ContentType) VALUES(?,?,?,?,?,?,?)"))
            return ERROR;

        stmt->ClearBindings();
        stmt->BindText (1, GetId(), BeSQLite::Statement::MakeCopy::Yes);
        stmt->BindBlob (2, GetData().data(), bufferSize, BeSQLite::Statement::MakeCopy::No);
        stmt->BindInt  (3, bufferSize);
        stmt->BindInt64(4, creationTime);
        stmt->BindInt64(5, expirationDate);
        stmt->BindText (6, GetEntityTag(), BeSQLite::Statement::MakeCopy::Yes);
        stmt->BindText (7, GetContentType(), BeSQLite::Statement::MakeCopy::Yes);
        if (BeSQLite::BE_SQLITE_DONE != stmt->Step())
            return ERROR;
        }

    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
//-------------------------------  WmsSource      ----------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
/*static*/ GeoCoordinates::BaseGCSPtr WmsSource::CreateBaseGcsFromWmsGcs(Utf8StringCR gcsStr)
    {
    // WMS GCS ex:
    //  - EPSG:26986
    //  - CRS:83, CRS:27, CRS:84, or some Bentley WMS server geocoord keyname.
   
    //1) Attempt to build from EPSG code.   
    if(0 == BeStringUtilities::Strnicmp(gcsStr.c_str(), "EPSG:", 5))
        {
        Utf8String epsgCodeStr = gcsStr.substr(5);
        int epsgCode = atoi(epsgCodeStr.c_str());

        GeoCoordinates::BaseGCSPtr pGcs = GeoCoordinates::BaseGCS::CreateGCS();
        if(SUCCESS == pGcs->InitFromEPSGCode(NULL, NULL, epsgCode))
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
    switch(mapInfo.m_axisOrder)
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
    if(!(mapInfo.m_version.Equals("1.3.0") && mapInfo.m_csType.EqualsI("CRS")))
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
        
    if(mapInfo.m_csLabel.EqualsI("CRS:1")  ||     // pixels 
       mapInfo.m_csLabel.EqualsI("CRS:83") ||     // (long, lat)
       mapInfo.m_csLabel.EqualsI("CRS:84") ||     // (long, lat) 
       mapInfo.m_csLabel.EqualsI("CRS:27"))       // (long, lat) WMS spec are not clear about CRS:27, there is comment where x is latitude and y longitude but 
                                                    // everyplace else it says (long,lat). Found only one server with CRS:27 and it was (long,lat).
        {
        return false;
        }

    if(0 == BeStringUtilities::Strnicmp (mapInfo.m_csLabel.c_str(), "EPSG:", sizeof("EPSG:")-1/*skip '/n'*/))
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
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
WmsSourcePtr WmsSource::Create(WmsMap const& mapInfo)
    {
    return new WmsSource(mapInfo);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
WmsSource::WmsSource(WmsMap const& mapInfo)
 :m_mapInfo(mapInfo),
  m_reverseAxis(false)
    {
    // for WMS we define a 256x256 multi-resolution image.
    bvector<Resolution> resolution;
    RasterSource::GenerateResolution(resolution, m_mapInfo.m_metaWidth, m_mapInfo.m_metaHeight, 256, 256);

    GeoCoordinates::BaseGCSPtr pGcs = CreateBaseGcsFromWmsGcs(m_mapInfo.m_csLabel);
    BeAssert(pGcs.IsValid()); //Is it an error if we do not have a GCS? We will assume coincident.

    DPoint3d translation = DPoint3d::From(m_mapInfo.m_boundingBox.low); // z == 0
    DPoint3d scale = DPoint3d::From((m_mapInfo.m_boundingBox.high.x - m_mapInfo.m_boundingBox.low.x) / m_mapInfo.m_metaWidth,  
                                    (m_mapInfo.m_boundingBox.high.y - m_mapInfo.m_boundingBox.low.y) / m_mapInfo.m_metaHeight, 
                                    0);

    DMatrix4d mapTransfo = DMatrix4d::FromScaleAndTranslation(scale, translation);

    // Data from server is upper-left(jpeg or png) and cartesians coordinate must have a lower-left origin, add a flip.
    DMatrix4d physicalToLowerLeft = DMatrix4d::FromRowValues(1.0, 0.0, 0.0, 0.0,
                                                             0.0, -1.0, 0.0, m_mapInfo.m_metaHeight,
                                                             0.0, 0.0, 1.0, 0.0,
                                                             0.0, 0.0, 0.0, 1.0);

    DMatrix4d physicalToCartesian;
    physicalToCartesian.InitProduct(mapTransfo, physicalToLowerLeft);

    m_reverseAxis = EvaluateReverseAxis(m_mapInfo, pGcs.get());
           
    Initialize(resolution, physicalToCartesian, pGcs.get());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
DisplayTilePtr WmsSource::_QueryTile(TileId const& id, bool request)
    {
    Utf8String tileUrl = BuildTileUrl(id);

    //      We are using tiledRaster but it should be extended to support more pixeltype and compression or have a new type?
    //      Maybe we should create a better Image object than RgbImageInfo and use that.
    RefCountedPtr<WmsTileData::RequestOptions> pOptions = WmsTileData::RequestOptions::Create(true, request);
        
    //&&MM for WMS it looks like I will need another kind of tiledRaster to handle exception response from the server.
    //     for example, a badly formated request generate an XML response. This is badly interpreted as a valid response(HttpRealityDataSourceRequest::_Handle) and 
    //     stored into the dataCache.  contentType equals "application/vnd.ogc.se_xml" (required by wms spec).
    //     Poss sol:  reject in TiledRaster::_InitFrom >>> did that for now.
    // *** maybe we could use the expiration date to handle errors. e.g. set a short expiration date for errors, timeout.
    //     that way the caller would receive the WmsTileData and we would retry at a later time.
    // *** as the database get bigger(I guess 500MB) the first call is very very slow. sorting by string is probably not a good idea either
    //     Maybe one table per server?  and use TileId or hash the url ?
    //     BeSQLiteRealityDataStorage::wt_Prepare call to "VACCUUM" is the reason why we have such a big slowdown.
    RefCountedPtr<WmsTileData> pWmsTileData = T_HOST.GetRealityDataAdmin().GetCache().Get<WmsTileData>(tileUrl.c_str(), *pOptions);

    if(!pWmsTileData.IsValid())
        return NULL;

    auto const& data = pWmsTileData->GetData();
    ImageUtilities::RgbImageInfo actualImageInfo;
    Utf8StringCR contentType = pWmsTileData->GetContentType();
    
    BentleyStatus status;

    m_decompressBuffer.clear(); // reuse the same buffer, in order to minimize mallocs

    if (contentType.EqualsI (CONTENT_TYPE_PNG))
        {
        status = ImageUtilities::ReadImageFromPngBuffer (m_decompressBuffer, actualImageInfo, data.data(), data.size());
        }
    else if (contentType.EqualsI (CONTENT_TYPE_JPEG))
        {
        //&&MM I don't understand this concept of expected image info. The fileRead should output that info.
        ImageUtilities::RgbImageInfo expectedImageInfo;
        expectedImageInfo.width = GetTileSizeX(id);
        expectedImageInfo.height = GetTileSizeY(id);
        expectedImageInfo.hasAlpha = false;
        expectedImageInfo.isBGR = false;
        expectedImageInfo.isTopDown = true;

        status = ImageUtilities::ReadImageFromJpgBuffer (m_decompressBuffer, actualImageInfo, data.data(), data.size(), expectedImageInfo);
        }
    else
        {
        BeAssertOnce (false && "Unsupported image type");
        return NULL;
        }

    //&&MM we probably need a way to handle errors and avoid trying over and over again.
    if (SUCCESS != status)
        return NULL;
    
    BeAssert (!actualImageInfo.isBGR);    //&&MM todo 
    DisplayTile::PixelType pixelType = actualImageInfo.hasAlpha ? DisplayTile::PixelType::Rgba : DisplayTile::PixelType::Rgb;
    DisplayTilePtr pDisplayTile = DisplayTile::Create(actualImageInfo.width, actualImageInfo.height, pixelType, m_mapInfo.m_transparent && actualImageInfo.hasAlpha, m_decompressBuffer.data(), 0/*notPadded*/);

    return pDisplayTile;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
Utf8String WmsSource::BuildTileUrl(TileId const& tileId)
    {
    // Get tile corners in this order, with a lower-left origin.
    // [0] [1]
    // [2] [3]
    DPoint3d tileCorners[4];
    ComputeTileCorners(tileCorners, tileId);

    double minX = tileCorners[2].x;
    double minY = tileCorners[2].y;
    double maxX = tileCorners[1].x;
    double maxY = tileCorners[1].y;

    if(m_reverseAxis)
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
    if(m_mapInfo.m_transparent)
        tileUrl.append("&TRANSPARENT=TRUE");
    else
        tileUrl.append("&TRANSPARENT=FALSE");

    if(!m_mapInfo.m_vendorSpecific.empty())
        {
        tileUrl.append("&");
        tileUrl.append(m_mapInfo.m_vendorSpecific);
        }
    
    return tileUrl;   
    }