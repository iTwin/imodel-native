/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/WmsSource.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <RasterSchema/WmsHandler.h>
#include <DgnPlatform/RealityDataCache.h>
#include "RasterSource.h"

BEGIN_BENTLEY_RASTERSCHEMA_NAMESPACE

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
struct WmsSource : public RasterSource
{
private:
    mutable RealityData::CachePtr m_realityDataCache;

public:
    static WmsSourcePtr Create(WmsMap const& mapInfo);

    WmsMap const& GetMapInfo() {return m_mapInfo;}

    virtual TransformCR _PhysicalToSource() const override
        {
        BeAssert(!"todo"); //&&MM
        static Transform m_trans;
        return m_trans;
        }

protected:
    virtual Render::Image _QueryTile(TileId const& id, bool& alphaBlend) override;
    RealityData::Cache& GetRealityDataCache() const;

private:
    WmsSource(WmsMap const& mapInfo);

    virtual ~WmsSource(){};

    Utf8String BuildTileUrl(TileId const& tileId);

    static GeoCoordinates::BaseGCSPtr CreateBaseGcsFromWmsGcs(Utf8StringCR gcsStr);
    static bool EvaluateReverseAxis(WmsMap const& mapInfo, GeoCoordinates::BaseGCSP pGcs);

    WmsMap m_mapInfo;

    bool m_reverseAxis; // deduct form WmsMap::m_axisOrder at construction.

    //uint32_t    m_metaTileSizeX;    //&&MM todo.
    //uint32_t    m_metaTileSizeY;
};

END_BENTLEY_RASTERSCHEMA_NAMESPACE
