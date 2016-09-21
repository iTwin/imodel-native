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
#include "RasterTileTree.h"

RASTERSCHEMA_TYPEDEFS(WmsSource)
RASTERSCHEMA_TYPEDEFS(WmsTile)

BEGIN_BENTLEY_RASTERSCHEMA_NAMESPACE

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
struct WmsSource : public RasterRoot
{
private:
    DMatrix4d m_physicalToCartesian;    //&&MM todo reprojection

    WmsMap m_mapInfo;   //&&MM use the one from the model and remove this one?

    bool m_reverseAxis; // deduct form WmsMap::m_axisOrder at construction.

                        //uint32_t    m_metaTileSizeX;    //&&MM todo.
                        //uint32_t    m_metaTileSizeY;

    WmsSource(WmsMap const& mapInfo, WmsModel& model, Dgn::Render::SystemP system);

    virtual ~WmsSource() {};

    Utf8String BuildTileUrl(TileId const& tileId);

    static GeoCoordinates::BaseGCSPtr CreateBaseGcsFromWmsGcs(Utf8StringCR gcsStr);
    static bool EvaluateReverseAxis(WmsMap const& mapInfo, GeoCoordinates::BaseGCSP pGcs);

    Utf8String _ConstructTileName(Dgn::TileTree::TileCR tile) override;

    void ComputeTileCorners(DPoint3dP pCorners, TileId const& id) const;

    folly::Future<BentleyStatus> _RequestTile(Dgn::TileTree::TileCR tile, Dgn::TileTree::TileLoadsPtr loads) override;

public:
    static WmsSourcePtr Create(WmsMap const& mapInfo, WmsModel& model, Dgn::Render::SystemP system);

    WmsMap const& GetMapInfo() {return m_mapInfo;}    

    DMatrix4dCR GetPhysicalToWorld() const { return m_physicalToCartesian; }    //&&MM todo reproject
};

//=======================================================================================
//! A raster tile. May or may not have its graphics loaded.
// @bsiclass                                                    Mathieu.Marchand  9/2016
//=======================================================================================
struct WmsTile : RasterTile
{
protected:
    typedef WmsSource root_type;
    
    root_type& GetSource() { return (root_type&) GetRoot(); }

public:
    typedef WmsSource root_type;

    WmsTile(WmsSourceR root, TileId id, WmsTileCP parent);

    BentleyStatus _LoadTile(Dgn::TileTree::StreamBuffer&, Dgn::TileTree::RootR) override;

    TileTree::Tile::ChildTiles const* _GetChildren(bool load) const override;
};

END_BENTLEY_RASTERSCHEMA_NAMESPACE
