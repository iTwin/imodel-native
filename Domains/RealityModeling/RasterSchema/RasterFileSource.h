/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "RasterTileTree.h"

BEGIN_BENTLEY_RASTER_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Mathieu.Marchand  9/2016
//=======================================================================================
struct RasterFileSource : RasterRoot
{
protected:
    RasterFilePtr       m_rasterFile;
    RasterFileModel&    m_model;
    Transform           m_physicalToSource;  //! Transformation from raster file true origin to a lower-left origin. In pixel unit.    
    DMap4d              m_physicalToWorld;   //! Including units change and reprojection approximation if any.
    Cesium::ChildTiles  m_children;

    RasterFileSource(RasterFileR rasterFile, RasterFileModel& model);
    ~RasterFileSource();

public:
    static RasterFileSourcePtr Create(Utf8StringCR resolvedName, RasterFileModel& model);

    Render::Image QueryTile(TileId const& id, bool& alphaBlend);

    DMatrix4dCR GetPhysicalToWorld() const { return m_physicalToWorld.M0; }
    DMatrix4dCR GetWorldToPhysical() const { return m_physicalToWorld.M1; }
};

//=======================================================================================
//! A raster tile. May or may not have its graphics loaded.
// @bsiclass                                                    Mathieu.Marchand  9/2016
//=======================================================================================
struct RasterFileTile : RasterTile
{
public:
    //=======================================================================================
    // @bsiclass                                                    Mathieu.Marchand  9/2016
    //=======================================================================================
    struct RasterTileLoader : Cesium::Loader
        {
        Render::Image m_image;  // filled by _ReadFromSource

        BentleyStatus DoGetFromSource();

        RasterTileLoader(Cesium::TileR tile, Cesium::LoadStateR loads, Cesium::OutputR output) : Cesium::Loader("", tile, output, loads) { }
            
        virtual ~RasterTileLoader(){}
        
        RasterFileSourceR GetFileSource() { return static_cast<RasterFileSourceR>(m_tile->GetRoot()); }

        folly::Future<BentleyStatus> _GetFromSource() override;        
        BentleyStatus _LoadTile() override;
        };
    

    RasterFileTile(RasterFileSourceR root, TileId id, RasterFileTileCP parent);

    Cesium::ChildTiles const* _GetChildren(bool load) const override;

    Cesium::LoaderPtr _CreateLoader(Cesium::LoadStateR loads, Cesium::OutputR output) override { return new RasterTileLoader(*this, loads, output); }
};

END_BENTLEY_RASTER_NAMESPACE
