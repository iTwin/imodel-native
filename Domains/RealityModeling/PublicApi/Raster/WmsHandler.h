/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <Raster/RasterHandler.h>

BEGIN_BENTLEY_RASTER_NAMESPACE

struct WmsModelHandler;

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
struct WmsMap
    {
    //! Identify axis order for GetMap queries. Might be required for WMS server 1.3.0. 
    enum class AxisOrder : uint32_t
        {
        Default     = 0,   //!< Use default axis order logic. Order is deducted from CS label for 1.3.0 servers. 
        Normal      = 1,   //!< Use normal axis order.  (minX,minY,maxX,maxY) -> (minX,minY,maxX,maxY)
        Reverse     = 2,   //!< Use reverse axis order. (minX,minY,maxX,maxY) -> (minY,minX,maxY,maxX)
        };

    WmsMap();

    //! Construct a a new WmsMap.  csType is deduce from version and format is set to "image/png".
    //! By default a meta raster of 10 resolutions is generated.
    RASTER_EXPORT WmsMap(Utf8CP url, DRange2dCR bbox, Utf8CP version, Utf8CP layers, Utf8CP csLabel);

    //! Return true if mandatory parameters are set. Does not validate with server.
    bool HasValidParameters() const;

    //! Compute and set metaWidth and metaHeight using the number of requested resolution. Bounding box aspect ratio is preserved.
    RASTER_EXPORT void SetMetaSizeByResolutionCount(uint32_t count);

    //! Set metaWidth and metaHeight. The largest side of the bounding box is set to pixelCount and the other meta size is computed to preserves bounding box aspect ratio.
    RASTER_EXPORT void SetMetaSizeByLargestBoundingBoxSide(uint32_t pixelCount);

    Utf8String m_url;               //! Get map url. Up to but excluding the query char '?'

    // Map window
    DRange2d m_boundingBox;         //! Bounding box corners (minx,miny,maxx,maxy) in 'Coordinate System' unit. long, lat for geographic CS and easting, northing for projected CS.
    uint32_t m_metaWidth;           //! Width of the window in pixels. The pixel ratio should be equal to the 'BoundingBox' ratio to avoid any distortion. Sub-Resolutions will be generated from this width.
    uint32_t m_metaHeight;          //! Height of the window in pixels. The pixel ratio should be equal to the 'BoundingBox' ratio to avoid any distortion. Sub-Resolutions will be generated from this Height.

    // Mandatory GetMap parameters
    Utf8String m_version;           //! Wms server version
    Utf8String m_layers;            //! Comma-separated list of one or more map layers.
    Utf8String m_styles;            //! Comma-separated list of one rendering style per requested layer.
    Utf8String m_csType;            //! Default to 'SRS' for 1.1.1 and below. 'CRS' for 1.3.
    Utf8String m_csLabel;           //! Coordinate System label. ex: "EPSG:4326"
    Utf8String m_format;            //! Output format MIME type. Default is 'image/png'

    // Optional GetMap parameters
    Utf8String m_vendorSpecific;    //! [optional] Unparsed, server specific parameters that will be appended to the request. 
    bool       m_transparent;       //! [optional] Background is transparent? Default is false. A format that support transparency is required. ex: image/png.
    AxisOrder  m_axisOrder;         //! [optional] Identify axis order for GetMap queries. Might be required for WMS server 1.3.0.

    void ToJson(Json::Value&) const;
    void FromJson(Json::Value const&);                
    };

//=======================================================================================
// @bsiclass                                                     Mathieu.Marchand  6/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE WmsModel : RasterModel
{
DGNMODEL_DECLARE_MEMBERS(RASTER_CLASSNAME_WmsModel, RasterModel)

    BE_JSON_NAME(wms)
private:
    WmsMap m_map;

    Http::Credentials m_credentials;
    Http::Credentials m_proxyCredentials;

protected:
    friend struct WmsModelHandler;
    
    //! Destruct a WmsModel object.
    ~WmsModel();

    void _OnSaveJsonProperties() override;
    void _OnLoadedJsonProperties() override;

    bool _IsParallelToGround() const override { return true; }

    Dgn::Cesium::RootPtr _CreateCesiumTileTree(Dgn::Cesium::OutputR) override { return nullptr; } // publishing not permitted...

    //! Create a WmsModel object, in preparation for loading it from the DgnDb. Called by MODELHANDLER_DECLARE_MEMBERS. 
    WmsModel(CreateParams const& params);

    //! Create a new WmsModel object to be stored in the DgnDb.
    WmsModel(CreateParams const& params, WmsMap const& prop);
public:
    RASTER_EXPORT WmsMap const& GetMap() const;    

    //! Return the last http error. If HttpStatus::Unauthorized or HttpStatus::ProxyAuthenticationRequired then 
    //! the connection must be authenticate before we try to contact the server again.
    RASTER_EXPORT Http::HttpStatus GetLastHttpError() const;

    //! Authenticate the connection. If successful, the credentials are saved for the session.
    RASTER_EXPORT Http::HttpStatus Authenticate(Http::Credentials const& credentials, Http::Credentials const& proxyCredentials);
};

//=======================================================================================
// Model handler for raster
// Instances of WmsModel must be able to assume that their handler is a WmsModelHandler.
// @bsiclass                                                     Mathieu.Marchand  6/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE WmsModelHandler : RasterModelHandler
{
    RASTERMODELHANDLER_DECLARE_MEMBERS (RASTER_CLASSNAME_WmsModel, WmsModel, WmsModelHandler, RasterModelHandler, RASTER_EXPORT)

public:
    RASTER_EXPORT static WmsModelPtr CreateWmsModel(DgnDbR db, Dgn::RepositoryLinkCR link, WmsMap const& prop);
};

END_BENTLEY_RASTER_NAMESPACE
