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
    Transform m_physicalToCartesian;    //! from physical pixel to Wms Gcs coordinate.

    Transform m_cartesianToWorldApproximation;  //! linear transform from Wms gcs to Bim world. Only used when reprojection fails.

    WmsMap m_mapInfo; 

    bool m_reverseAxis; // deduct form WmsMap::m_axisOrder at construction.

    GeoCoordinates::BaseGCSPtr m_gcs;   //! WMS Gcs. Might be NULL if we cannot create one.

    WmsSource(WmsMap const& mapInfo, WmsModel& model, Dgn::Render::SystemP system);

    virtual ~WmsSource() {};

    Utf8String BuildTileUrl(TileId const& tileId);

    static GeoCoordinates::BaseGCSPtr CreateBaseGcsFromWmsGcs(Utf8StringCR gcsStr);
    static bool EvaluateReverseAxis(WmsMap const& mapInfo, GeoCoordinates::BaseGCSP pGcs);

    Utf8String _ConstructTileName(Dgn::TileTree::TileCR tile) override;

    folly::Future<BentleyStatus> _RequestTile(Dgn::TileTree::TileCR tile, Dgn::TileTree::TileLoadsPtr loads) override;

    StatusInt ComputeLinearApproximation(TransformR cartesianToWorld);

public:
    static WmsSourcePtr Create(WmsMap const& mapInfo, WmsModel& model, Dgn::Render::SystemP system);

    WmsMap const& GetMapInfo() const {return m_mapInfo;}    

    TransformCR GetCartesianToWorldApproximation() const { return m_cartesianToWorldApproximation; }

    TransformCR GetPhysicalToCartesian() const { return m_physicalToCartesian; }

    GeoCoordinates::BaseGCSP GetGcsP() const { return m_gcs.get(); }

    StatusInt ReprojectCorners(DPoint3dP destWorld, DPoint3dCP srcCartesian) const;
};

//=======================================================================================
//! A raster tile. May or may not have its graphics loaded.
// @bsiclass                                                    Mathieu.Marchand  9/2016
//=======================================================================================
struct WmsTile : RasterTile
{
public:
    typedef WmsSource root_type;

protected:
    root_type const& GetSource() const { return (root_type&) m_root; }

public:
    WmsTile(WmsSourceR root, TileId id, WmsTileCP parent);

    BentleyStatus _LoadTile(Dgn::TileTree::StreamBuffer&, Dgn::TileTree::RootR) override;

    TileTree::Tile::ChildTiles const* _GetChildren(bool load) const override;

    //! Cartesian(in WMS gcs) corners of this tile. 
    void GetCartesianCorners(DPoint3dP pCorners) const;
};

END_BENTLEY_RASTERSCHEMA_NAMESPACE
