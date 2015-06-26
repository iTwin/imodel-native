/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RasterSchema/WmsHandler.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <RasterSchema/RasterHandler.h>

BEGIN_BENTLEY_RASTERSCHEMA_NAMESPACE

struct WmsModelHandler;

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
struct WmsMap
    {
    WmsMap();
    RASTERSCHEMA_EXPORT WmsMap(Utf8CP url, DRange2dCR bbox, Utf8CP version, Utf8CP layers, Utf8CP csType, Utf8CP csLabel);

    //! Return true if mandatory parameters are set. Does not validate with server.
    bool HasValidParameters() const;

    //! Compute and set metaWidth and metaHeight using the number of requested resolution. bbox aspect ration is preserved.
    RASTERSCHEMA_EXPORT void SetMetaSizeByResolutionCount(uint32_t count);

    //! Set metaWidth or metaHeight using the largest side of the bounding box to pixelCount and compute the other meta size that preserves bbox aspect ratio.
    RASTERSCHEMA_EXPORT void SetMetaSizeByLargestBoundingBoxSide(uint32_t pixelCount);

    Utf8String m_url;               //! Get map url. Up to but excluding the query char '?'

    // Map window
    DRange2d m_boundingBox;         //! Bounding box corners (minx,miny,maxx,maxy) in 'CoordinateSystem' unit (aka. cartesian or reprojected unit)
    uint32_t m_metaWidth;           //! Width of the window in pixels. The pixel ratio should be equal to the 'BoundingBox' ratio to avoid any distortion. Sub-Resolutions will be generated from this width.
    uint32_t m_metaHeight;          //! Height of the window in pixels. The pixel ratio should be equal to the 'BoundingBox' ratio to avoid any distortion. Sub-Resolutions will be generated from this Height.


    // Mandatory GetMap parameters
    Utf8String m_version;           //! Wms server version
    Utf8String m_layers;            //! Comma-separated list of one or more map layers.
    Utf8String m_styles;            //! Comma-separated list of one rendering style per requested layer.
    Utf8String m_csType;            //! Usually, 'SRS' for 1.1.1 and below. 'CRS' for 1.3 and above.
    Utf8String m_csLabel;           //! Coordinate system label. ex: "EPSG:4326"
    Utf8String m_format;            //! Output format of map default is 'image/png'

    // Optional GetMap parameters
    Utf8String m_vendorSpecific;    //! [optional] Unparsed, vendor specific parameters that will be appended to the request. 
    bool m_transparent;             //! [optional] Background is transparent?

    void ToJson(Json::Value&) const;
    void FromJson(Json::Value const&);                
    };

//=======================================================================================
// @bsiclass                                                     Mathieu.Marchand  6/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE WmsModel : RasterModel
{
    DEFINE_T_SUPER(RasterModel)

private:
    WmsMap m_map;     //&&MM need a way to avoid duplication of information between RasterModel/WmsModel/WmsSource.

protected:
    friend struct WmsModelHandler;
    
    //! Destruct a WmsModel object.
    ~WmsModel();

    virtual void _ToPropertiesJson(Json::Value&) const override;
    virtual void _FromPropertiesJson(Json::Value const&) override;
    virtual Dgn::AxisAlignedBox3d _QueryModelRange() const override;

    virtual BentleyStatus _LoadQuadTree() override;

    //! Create a WmsModel object, in preparation for loading it from the DgnDb. Called by MODELHANDLER_DECLARE_MEMBERS. 
    WmsModel(CreateParams const& params);

    //! Create a new WmsModel object to be stored in the DgnDb.
    WmsModel(CreateParams const& params, WmsMap const& prop);
public:
    
};

//=======================================================================================
// Model handler for raster
// Instances of WmsModel must be able to assume that their handler is a WmsModelHandler.
// @bsiclass                                                     Mathieu.Marchand  6/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE WmsModelHandler : RasterModelHandler
{
    RASTERMODELHANDLER_DECLARE_MEMBERS (RASTER_CLASSNAME_WmsModel, WmsModel, WmsModelHandler, RasterModelHandler, RASTERSCHEMA_EXPORT)

public:
    RASTERSCHEMA_EXPORT static Dgn::DgnModelId CreateWmsModel(DgnDbR db, Utf8CP modelName, WmsMap const& prop);
};

END_BENTLEY_RASTERSCHEMA_NAMESPACE
