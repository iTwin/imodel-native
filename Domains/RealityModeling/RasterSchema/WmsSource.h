/*-------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <Raster/WmsHandler.h>
#include <DgnPlatform/RealityDataCache.h>
#include "RasterTileTree.h"

RASTER_TYPEDEFS(WmsSource)
RASTER_TYPEDEFS(WmsTile)

BEGIN_BENTLEY_RASTER_NAMESPACE

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
struct WmsSource : public RasterRoot
{
private:
    Transform m_physicalToCartesian;    //! from physical pixel to Wms Gcs coordinate.

    Transform m_cartesianToWorldApproximation;  //! linear transform from Wms gcs to Bim world. Only used when reprojection fails.

    WmsMap m_mapInfo;

    Http::Credentials m_credentials;
    Http::Credentials m_proxyCredentials;

    bool m_reverseAxis; // deduct form WmsMap::m_axisOrder at construction.

    std::atomic<Http::HttpStatus> m_lastHttpError;

    GeoCoordinates::BaseGCSPtr m_gcs;   //! WMS Gcs. Might be NULL if we cannot create one.

    WmsSource(WmsMap const& mapInfo, WmsModel& model, Dgn::Render::SystemP system);

    virtual ~WmsSource() {};
    
    static GeoCoordinates::BaseGCSPtr CreateBaseGcsFromWmsGcs(Utf8StringCR gcsStr);
    static bool EvaluateReverseAxis(WmsMap const& mapInfo, GeoCoordinates::BaseGCSP pGcs);
    Utf8CP _GetName() const override {return "WmsSource";}
    Utf8String _ConstructTileResource(Dgn::TileTree::TileCR tile) const override;

    StatusInt ComputeLinearApproximation(TransformR cartesianToWorld);

public:
    static WmsSourcePtr Create(WmsMap const& mapInfo, WmsModel& model, Dgn::Render::SystemP system);

    WmsMap const& GetMapInfo() const {return m_mapInfo;}    

    TransformCR GetCartesianToWorldApproximation() const { return m_cartesianToWorldApproximation; }

    TransformCR GetPhysicalToCartesian() const { return m_physicalToCartesian; }

    GeoCoordinates::BaseGCSP GetGcsP() const { return m_gcs.get(); }

    bool IsReverseAxis() const { return m_reverseAxis; }

    StatusInt ReprojectCorners(DPoint3dP destWorld, DPoint3dCP srcCartesian) const;

    Http::HttpStatus GetLastHttpError() const { return m_lastHttpError; }
    void SetLastHttpError(Http::HttpStatus const& lastError) { m_lastHttpError = lastError; }

    Http::CredentialsCR GetCredentials() const { return m_credentials; }
    void SetCredentials(Http::Credentials const& credentials) { m_credentials = credentials; }
    
    Http::CredentialsCR GetProxyCredentials() const { return m_proxyCredentials; }
    void SetProxyCredentials(Http::Credentials const& credentials) { m_proxyCredentials = credentials; }
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
    //=======================================================================================
    // @bsiclass                                                    Mathieu.Marchand  9/2016
    //=======================================================================================
    struct WmsTileLoader : TileTree::TileLoader
        {
        Http::Credentials m_credentials;
        Http::Credentials m_proxyCredentials;

        WmsTileLoader(Utf8StringCR url, TileTree::TileR tile, TileTree::TileLoadStatePtr loads, Dgn::Render::SystemP renderSys) : TileLoader(url, tile, loads, tile._GetTileCacheKey(), renderSys) {}
        virtual ~WmsTileLoader() {}

        folly::Future<BentleyStatus> _GetFromSource() override;
        BentleyStatus _LoadTile() override;

        WmsTile& GetWmsTile() { return static_cast<WmsTile&>(*m_tile); }
        };

    Dgn::TileTree::TileLoaderPtr _CreateTileLoader(Dgn::TileTree::TileLoadStatePtr, Dgn::Render::SystemP renderSys) override;

    root_type const& GetSource() const { return static_cast<root_type const&>(m_root); }
    root_type& GetSourceR() { return static_cast<root_type&>(m_root); }

public:
    WmsTile(WmsSourceR root, TileId id, WmsTileCP parent);

    TileTree::Tile::ChildTiles const* _GetChildren(bool load) const override;

    //! Cartesian(in WMS gcs) corners of this tile. 
    void GetCartesianCorners(DPoint3dP pCorners) const;

    
};

END_BENTLEY_RASTER_NAMESPACE
