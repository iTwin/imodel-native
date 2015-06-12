/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/WmsSource.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <RasterSchema/WmsHandler.h>
#include "RasterSource.h"

BEGIN_BENTLEY_RASTERSCHEMA_NAMESPACE

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
struct WmsSource : public RasterSource
{
public:
    static WmsSourcePtr Create(WmsMap const& mapInfo);

    WmsMap const& GetProperties() {return m_mapInfo;}

protected:
    virtual DisplayTilePtr _QueryTile(TileId const& id, bool request) override;

private:
    WmsSource(WmsMap const& mapInfo);

    virtual ~WmsSource(){};

    Utf8String BuildTileUrl(TileId const& tileId);

    GeoCoordinates::BaseGCSPtr CreateBaseGcsFromWmsGcs(Utf8StringCR gcsStr);

    WmsMap m_mapInfo;

    bvector<Byte> m_decompressBuffer;

    //uint32_t    m_metaTileSizeX;    //&&MM todo.
    //uint32_t    m_metaTileSizeY;
};


END_BENTLEY_RASTERSCHEMA_NAMESPACE