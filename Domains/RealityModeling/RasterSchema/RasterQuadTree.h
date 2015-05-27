/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterQuadTree.h $
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
struct RasterTile : public RefCountedBase
{
public:

    enum ChildIndex
        {
        UpperLeft   = 0,
        UpperRight  = 1,
        LowerLeft   = 2,
        LowerRight  = 3,
        };

    //&&MM TODO status. The goal is to avoid requesting over and over when errors occurs.
    // Also when a request timeout we should somehow retry a number of time.
    // Maybe we need flags? or 2 status. 
    enum class TileStatus
        {
        Unloaded,
        Requested,
        Loaded,
        ConnectionError,
        //Error,
        //Expired,
        OutOfDomain,    // reprojection error  
        };

    static RasterTilePtr CreateRoot(RasterQuadTreeR tree);

    //! return NULL if not available.
    DisplayTileP GetDisplayTileP(bool request);

    // Raster 4 corners(UOR) in this order:
    //  [0]  [1]
    //  [2]  [3]
    DPoint3dCR GetCorners() const; 

    //! Draw this tile in the view. Tile might not be loaded. Return true if successful.
    bool Draw(ViewContextR context);    

    TileId const& GetId() const {return m_tileId;}

    RasterTileP GetParentP() {return m_pParent;}

    //! Return allocated child only. May be NULL.
    RasterTileP GetChildP(size_t index) {return m_pChilds[index].get();}

    bool QueryVisible(bvector<RasterTilePtr>& visibles, ViewContextR context);

    bool IsLoaded() const {return m_pDisplayTile.IsValid();}

private:
    RasterTile(TileId const& id, RasterTileP parent, RasterQuadTreeR tree);

    bool IsLeaf() const {return 0 == m_tileId.resolution;}

    bool IsVisible (ViewContextR viewContext, double& factor) const;

    bool IsBorderTile() const{return GetResolution().GetTileCountX() == m_tileId.tileX +1 || GetResolution().GetTileCountY() == m_tileId.tileY + 1;}

    RasterSource::Resolution const& GetResolution() const;

    DisplayTilePtr m_pDisplayTile;      // Can be NULL, will be loaded when needed. 

    //! Allocate children according to the resolution description.
    void AllocateChildren();

    bool ReprojectCorners(DPoint3dP outUors, DPoint3dCP srcCartesian);

    TileId m_tileId; 
    RasterQuadTreeR m_tree;          // Hold a ref only.

    DPoint3d m_corners[4];      // Corners in uor.

    TileStatus m_status;

    RasterTileP m_pParent;      // NULL for root node.
    RasterTilePtr m_pChilds[4];    
};


//----------------------------------------------------------------------------------------
// Quad-tree that hold raster tiles.
// The tree structure use the rasterSource resolutions and tile to define the nodes.
// Pixels are not resampled by this class it use the 'source' object resolution and tiles size
// to define the tree nodes.
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
struct RasterQuadTree : public RefCountedBase
{
public:
    static RasterQuadTreePtr Create(RasterSourceR source, DgnDbR dgnDb);

    RasterTileR GetTileR(TileId const& id);

    RasterTileR GetRoot() {return *m_pRoot;}

    RasterSourceR GetSource() {return *m_pSource;}

    bool QueryVisible(bvector<RasterTilePtr>& visibles, ViewContextR context);

    DgnDbR GetDgnDb() {return m_dgnDb;}
private:
    
    RasterQuadTree(RasterSourceR source, DgnDbR dgnDb);

    //&&MM need to have 4 valid corners. Need to intersect with domain.
    DgnDbR m_dgnDb;                 
    RasterSourcePtr m_pSource; 
    RasterTilePtr m_pRoot;          // The lowest resolution. 
};


END_BENTLEY_RASTERSCHEMA_NAMESPACE