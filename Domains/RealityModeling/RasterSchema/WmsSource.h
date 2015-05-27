/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/WmsSource.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "RasterSource.h"

BEGIN_BENTLEY_RASTERSCHEMA_NAMESPACE

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
struct WmsSource : public RasterSource
{
public:
    static RasterSourcePtr Create(Utf8CP url, Utf8CP version, Utf8CP layers, Utf8CP styles, Utf8CP csType, Utf8CP csLabel, Utf8CP imageFormat, 
                                  DRange2dCR bbox, uint32_t metaWidth, uint32_t metaHeight);

    //! Server url for GetMap request. With scheme type(http, https) and everything before the '?'
    Utf8StringCR GetUrl() const;

    //! Wms server version.
    Utf8StringCR GetVersion() const;

    //! Comma-separated list of one or more map layers
    Utf8StringCR GetLayers() const;

    //! Comma-separated list of one rendering style per requested layer
    Utf8StringCR GetStyles() const;

    //! Type SRS for 1.1.1 and below. CRS for 1.3 and above.
    Utf8StringCR GetCoordinateSystemType() const;
    Utf8StringCR GetCoordinateSystem() const;

    //! Output format of map.
    Utf8StringCR GetFormat() const;

    //! Background transparency of map.
    bool IsTransparent() const;

    //! Hexadecimal red-green-blue color value for the background color (default=0xFFFFFF).
    Utf8StringCR GetBackgroundColor() const;
    
protected:
    virtual DisplayTilePtr _QueryTile(TileId const& id, bool request) override;

private:
    WmsSource(Utf8CP url, Utf8CP version, Utf8CP layers, Utf8CP styles, Utf8CP csType, Utf8CP cs, Utf8CP format, 
              DRange2dCR bbox, uint32_t metaWidth, uint32_t metaHeight);

    virtual ~WmsSource(){};

    Utf8String BuildTileUrl(TileId const& tileId);

    GeoCoordinates::BaseGCSPtr CreateBaseGcsFromWmsGcs(Utf8StringCR gcsStr);

    Utf8String  m_serverUrl;            // Server URL. Everything before the '?'
                                        // Request Parameter
    Utf8String  m_version;              // VERSION=
    Utf8String  m_layers;               // LAYERS=
    Utf8String  m_styles;               // STYLES=
    Utf8String  m_gcsType;              // 'SRS' version 1.1.1 and below. 'CRS' version 1.3.0 and above.
    Utf8String  m_gcs;                  // gcsType=
    Utf8String  m_format;               // FORMAT=
    Utf8String  m_transparent;          // TRANSPARENT=TRUE|FALSE
    Utf8String  m_backgroundColor;      // BGCOLOR= red-green-blue. default 0xFFFFFF.
    Utf8String  m_vendorSpecific;       // not interpreted. Append to the request.

    // data window parameter.    
    DRange2d m_boundingBox;
    uint32_t m_metaWidth;
    uint32_t m_metaHeight;
    
    bvector<Byte> m_decompressBuffer;

    //uint32_t    m_metaTileSizeX;    //&&MM todo.
    //uint32_t    m_metaTileSizeY;
};


END_BENTLEY_RASTERSCHEMA_NAMESPACE